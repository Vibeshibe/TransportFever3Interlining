-- Passive diagnostics: never mutate an event payload or return a routing override.
local probe = {}
local events = {
	OnModifyPathfinding = "SimPersonSystem",
	OnStartedLineUsage = "SimPersonSystem",
	OnCompletedLineUsage = "SimPersonSystem",
	OnArriveAtStop = "TransportVehicleSystem",
}

local function tableSummary(value)
	if type(value) ~= "table" then return type(value) end
	local keys = {}
	local truncated = false
	for key, item in pairs(value) do
		if #keys >= 16 then
			truncated = true
			break
		end
		keys[#keys + 1] = tostring(key) .. ":" .. type(item)
	end
	table.sort(keys)
	return "{" .. table.concat(keys, ",") .. (truncated and ",..." or "") .. "}"
end

local function boundedCount(value)
	if type(value) ~= "table" then return type(value) end
	local count = 0
	for _ in pairs(value) do
		count = count + 1
		if count > 64 then return "64+" end
	end
	return tostring(count)
end

local function timestamp()
	local ok, time = pcall(function()
		return api.engine.getComponent(api.engine.util.getWorld(), api.type.ComponentType.GAME_TIME)
	end)
	if not ok or time == nil then return "time=unavailable" end
	return "gameTime=" .. tostring(time.gameTime)
		.. " tick=" .. tostring(time.tickCount) .. " update=" .. tostring(time.updateCount)
end

probe.update = function(_userParams, state, _dt)
	if not state:hasEventSubscriptions() then
		for name in pairs(events) do state:subscribeToEvent(name) end
		log.message("[TPF3InterliningProbe] subscribed; passive mode")
	end
end

probe.handleEvent = function(_userParams, state, _src, id, name, param)
	if events[name] ~= id then return nil end
	local saved = state:get() or {}
	saved.counts = saved.counts or {}
	local count = (saved.counts[name] or 0) + 1
	saved.counts[name] = count
	state:set(saved)
	-- Sample early events and then sparsely to keep logging cost bounded.
	if count <= 20 or count % 100 == 0 then
		local details = ""
		if type(param) == "table" then
			if name == "OnModifyPathfinding" then
				details = " townMap=" .. tableSummary(param.town2lineCostFactor)
					.. " townEntries=" .. boundedCount(param.town2lineCostFactor)
			elseif name == "OnArriveAtStop" then
				details = " line=" .. tostring(param.lineEntity)
					.. " vehicle=" .. tostring(param.vehicleEntity)
					.. " stop=" .. tostring(param.stopIndex)
					.. " previousStop=" .. tostring(param.lastStopIndex)
			else
				details = " batchSize=" .. boundedCount(param.entities)
				if type(param.entities) == "table" then
					local first = param.entities[1]
					if type(first) == "table" then
						details = details .. " samplePerson=" .. tostring(first[1])
							.. " sampleLine=" .. tostring(first[2])
					elseif first ~= nil then
						details = details .. " samplePerson=" .. tostring(first)
					end
				end
			end
		end
		log.message("[TPF3InterliningProbe] " .. id .. "/" .. name
			.. " count=" .. count .. " " .. timestamp()
			.. " payload=" .. tableSummary(param) .. details)
	end
	-- The native dispatcher only aggregates non-nil callback returns.
	-- Returning nil avoids replacing other mods' pathfinding modifications.
	return nil
end

-- Lua resource scripts expose fields through data(); returning the module also
-- allows the same implementation to be exercised by the offline test harness.
function data()
	return probe
end

return probe

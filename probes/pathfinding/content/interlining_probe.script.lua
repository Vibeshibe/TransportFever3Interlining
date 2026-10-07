-- Passive diagnostics: never mutate an event payload or return a routing override.
local probe = {}
local REVISION = 2
local MAX_QUEUE_STOPS = 16
local MAX_PEOPLE_PER_QUEUE = 8
local MAX_PEOPLE_PER_ARRIVAL = 32
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

-- Keep engine values local to the callback; never persist component objects.
local function component(entity, kind)
	if type(entity) ~= "number" or entity < 0 then return nil end
	local ok, result = pcall(function()
		return api.engine.getComponent(entity, api.type.ComponentType[kind])
	end)
	if ok then return result end
	return nil
end

local function entityName(entity)
	local value = component(entity, "NAME")
	return string.format("%q", tostring(value and value.name or "unknown"):sub(1, 96))
end

local function stopAt(line, index)
	if not line or type(index) ~= "number" or index < 0 or index % 1 ~= 0 then return nil end
	return line.stops[index + 1] -- engine stop indices are zero-based
end

local function stopDetails(stop)
	if not stop then return "group=unknown station=unknown terminal=unknown" end
	return "group=" .. tostring(stop.stationGroup)
		.. " station=" .. tostring(stop.station) .. " terminal=" .. tostring(stop.terminal)
end

local function emit(message)
	log.message("[TPF3InterliningProbe] " .. message)
end

local function arrivalSnapshot(param, count)
	local line = component(param.lineEntity, "LINE")
	local stop = stopAt(line, param.stopIndex)
	local prefix = "arrival=" .. count .. " vehicle=" .. tostring(param.vehicleEntity)
	if not stop then
		emit("ArrivalSnapshot " .. prefix .. " status=missing_line_or_stop")
		return
	end
	local cargo = api.res.cargoTypeRep.getPassengerCargoTypeId()
	if type(cargo) ~= "number" or cargo < 0 then error("Passenger cargo ID unavailable") end
	local vehicle = component(param.vehicleEntity, "TRANSPORT_VEHICLE")
	local capacity = vehicle and vehicle.config and vehicle.config.capacities[cargo + 1]
	local occupied = api.engine.system.simEntityAtVehicleSystem
		.getVehicleSimEntitiesCountForCargoType(param.vehicleEntity, cargo)
	local terminal = vehicle and vehicle.arrivalStationTerminal
	local config = stop.stopConfig
	emit("ArrivalSnapshot " .. prefix .. " " .. timestamp()
		.. " line=" .. tostring(param.lineEntity) .. " lineName=" .. entityName(param.lineEntity)
		.. " stop=" .. tostring(param.stopIndex) .. " " .. stopDetails(stop)
		.. " groupName=" .. entityName(stop.stationGroup)
		.. " actualStation=" .. tostring(terminal and terminal.station)
		.. " actualTerminal=" .. tostring(terminal and terminal.terminal)
		.. " capacity=" .. tostring(capacity) .. " occupied=" .. tostring(occupied)
		.. " loadAllowed=" .. tostring(config and config.load[cargo + 1])
		.. " maxLoad=" .. tostring(config and config.maxLoad[cargo + 1])
		.. " loadState=" .. tostring(vehicle and vehicle.loadState))
	-- Inspect every line occurrence at this station group, not just the arriving
	-- line. Platform and direction differences remain visible, not assumed valid.
	local queues = api.engine.system.lineSystem.getLineStops(stop.stationGroup)
	local peopleLogged = 0
	local queuesLogged = 0
	for _, entry in ipairs(queues) do
		if queuesLogged >= MAX_QUEUE_STOPS or peopleLogged >= MAX_PEOPLE_PER_ARRIVAL then
			emit("SnapshotLimit " .. prefix .. " queues=" .. queuesLogged .. " people=" .. peopleLogged)
			break
		end
		queuesLogged = queuesLogged + 1
		local queueLine, queueStop = entry[1], entry[2]
		local queueComponent = component(queueLine, "LINE")
		local people = api.engine.system.simEntityAtTerminalSystem
			.getLineStopSimEntities(queueLine, queueStop, cargo)
		emit("QueueSnapshot " .. prefix .. " plannedLine=" .. tostring(queueLine)
			.. " lineName=" .. entityName(queueLine) .. " boardingStop=" .. tostring(queueStop)
			.. " " .. stopDetails(stopAt(queueComponent, queueStop))
			.. " waiting=" .. tostring(#people))
		local inQueue = 0
		for _, person in ipairs(people) do
			if inQueue >= MAX_PEOPLE_PER_QUEUE or peopleLogged >= MAX_PEOPLE_PER_ARRIVAL then
				emit("QueueSampleLimit " .. prefix .. " plannedLine=" .. tostring(queueLine)
					.. " boardingStop=" .. tostring(queueStop) .. " sampled=" .. inQueue)
				break
			end
			inQueue = inQueue + 1
			peopleLogged = peopleLogged + 1
			local waiting = component(person, "SIM_ENTITY_AT_TERMINAL")
			if waiting then
				local planned = waiting.line == queueLine and queueComponent or component(waiting.line, "LINE")
				local alighting = stopAt(planned, waiting.lineStop1)
				emit("WaitingPerson " .. prefix .. " person=" .. tostring(person)
					.. " plannedLine=" .. tostring(waiting.line)
					.. " boardingStop=" .. tostring(waiting.lineStop0)
					.. " alightingStop=" .. tostring(waiting.lineStop1)
					.. " alightingGroup=" .. tostring(alighting and alighting.stationGroup)
					.. " alightingGroupName=" .. entityName(alighting and alighting.stationGroup)
					.. " arrivalTime=" .. tostring(waiting.arrivalTime)
					.. " assignedVehicle=" .. tostring(waiting.vehicle))
			else
				emit("WaitingPerson " .. prefix .. " person=" .. tostring(person) .. " status=no_waiting_component")
			end
		end
	end
end

probe.update = function(_userParams, state, _dt)
	local saved = state:get() or {}
	if saved.probeRevision ~= REVISION then
		saved.probeRevision = REVISION
		saved.counts = {} -- restart sampling when upgrading an experimental save
		state:set(saved)
		emit("revision=" .. REVISION .. "; passive mode; arrival snapshots enabled")
	end
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
	if name == "OnArriveAtStop" and type(param) == "table"
		and (count <= 60 or count % 100 == 0) then
		local ok, err = pcall(arrivalSnapshot, param, count)
		if not ok then
			emit("SnapshotError arrival=" .. count .. " reason=" .. string.format("%q", tostring(err):sub(1, 240)))
		end
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

-- Capability inventory plus an inert native module loading experiment.
local probe = {}
local emitted = {}

local function inspect(label, getter)
	local ok, value = pcall(getter)
	return label .. "=" .. (ok and type(value) or "inaccessible")
end

local function report(phase)
	if emitted[phase] then return end
	emitted[phase] = true
	local fields = {
		"[TPF3RuntimeProbe] revision=2 phase=" .. phase,
		inspect("package", function() return package end),
		inspect("package.loadlib", function() return package.loadlib end),
		inspect("package.cpath", function() return package.cpath end),
		inspect("package.searchers", function() return package.searchers end),
		inspect("require", function() return require end),
		inspect("ug_require", function() return ug_require end),
		inspect("ffi", function() return ffi end),
		inspect("readComponent", function() return api.engine.getComponent end),
		inspect("sendCommand", function() return api.cmd.sendCommand end),
		inspect("debugExchange", function() return api.cmd.debug.makeComponentExchangeCmd end),
		inspect("queueQuery", function() return api.engine.system.simEntityAtTerminalSystem.getLineStopSimEntities end),
	}
	local ok, build = pcall(function() return getBuildVersion() end)
	if ok then fields[#fields + 1] = "build=" .. tostring(build) end
	log.message(table.concat(fields, " "))
end

function probe.preRun() report("preRun") end
function probe.run() report("run") end
function probe.postRun() report("postRun") end
local attempted = false
local function handshake()
	if attempted then return end
	attempted = true
	-- This library only prints a marker and returns; it cannot modify the engine.
	local ok, err = pcall(function()
		if type(package) ~= "table" or type(package.searchers) ~= "table" then
			log.message("[TPF3RuntimeProbe] native_handshake=no_searchers")
			return
		end
		local candidates = {"./mods/tpf3_interlining_runtime_probe_1/content/native/?.so"}
		local resolved, path = pcall(function() return resolve("native/?.so") end)
		if resolved and type(path) == "string" then
			table.insert(candidates, 1, path)
			log.message("[TPF3RuntimeProbe] resolved_native_path=" .. path)
		end
		local previous = package.cpath
		local loaded = false
		local success, failure = pcall(function()
			for _, candidate in ipairs(candidates) do
				package.cpath = candidate
				for index = 3, 4 do
					local searcher = package.searchers[index]
					log.message("[TPF3RuntimeProbe] searcher=" .. index .. " type=" .. type(searcher))
					if type(searcher) == "function" then
						local found, loader = pcall(searcher, "tpf3_runtime_handshake")
						if found and type(loader) == "function" then
							loader("tpf3_runtime_handshake")
							loaded = true
							log.message("[TPF3RuntimeProbe] native_handshake=loaded searcher=" .. index)
							return
						end
						log.message("[TPF3RuntimeProbe] searcher_result=" .. tostring(loader):sub(1, 300))
					end
				end
			end
		end)
		package.cpath = previous
		if not success then error(failure) end
		if not loaded then log.message("[TPF3RuntimeProbe] native_handshake=unavailable") end
	end)
	if not ok then log.message("[TPF3RuntimeProbe] handshake_error=" .. tostring(err):sub(1, 300)) end
end

function probe.update(_, state)
	-- TF3 may recreate the resource module for later callbacks. Keep the latch in
	-- this diagnostic mod's private state, not in the transient Lua module.
	if state then
		local saved = state:get() or {}
		if saved.runtimeProbeRevision == 2 then return end
		saved.runtimeProbeRevision = 2
		state:set(saved)
	end
	report("update")
	handshake()
end

function data() return probe end
return probe

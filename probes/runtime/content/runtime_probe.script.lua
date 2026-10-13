-- Read-only capability inventory. Never call a loader, command or native function.
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
		"[TPF3RuntimeProbe] revision=1 phase=" .. phase,
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
function probe.update() report("update") end

function data() return probe end
return probe

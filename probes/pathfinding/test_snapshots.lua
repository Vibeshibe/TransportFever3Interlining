-- Engine-shaped fixtures for bounded, read-only arrival diagnostics.
local messages = {}
log = {message = function(s) messages[#messages + 1] = s end}
local function contains(s)
	for _, message in ipairs(messages) do
		if message:find(s, 1, true) then return true end
	end
	return false
end
local function countLines(s)
	local count = 0
	for _, message in ipairs(messages) do
		if message:find(s, 1, true) then count = count + 1 end
	end
	return count
end
local function stop(group, terminal)
	return {stationGroup = group, station = 0, terminal = terminal,
		stopConfig = {load = {[6] = true}, maxLoad = {[6] = 0.75}}}
end
local components = {
	[10] = {NAME = {name = "S1"}, LINE = {stops = {stop(100, 2), stop(200, 0)}}},
	[20] = {NAME = {name = "S2"}, LINE = {stops = {stop(100, 3), stop(300, 0), stop(200, 1)}}},
	[100] = {NAME = {name = "Shared station"}},
	[200] = {NAME = {name = "Alighting station"}},
	[50] = {TRANSPORT_VEHICLE = {config = {capacities = {[6] = 40}},
		arrivalStationTerminal = {station = 0, terminal = 4}, loadState = 2}},
	[70] = {SIM_ENTITY_AT_TERMINAL = {line = 20, lineStop0 = 0, lineStop1 = 2,
		arrivalTime = 123, vehicle = -1}},
}
local queues = {{10, 0}, {20, 0}}
local people = {70, 71} -- 71 has disappeared from the component snapshot
local queryCalls = 0
api = {
	type = {ComponentType = setmetatable({}, {__index = function(_, k) return k end})},
	res = {cargoTypeRep = {getPassengerCargoTypeId = function() return 5 end}},
	engine = {
		util = {getWorld = function() return 0 end},
		getComponent = function(entity, kind)
			if kind == "GAME_TIME" then return {gameTime = 1000, tickCount = 50, updateCount = 40} end
			return components[entity] and components[entity][kind]
		end,
		system = {
			lineSystem = {getLineStops = function(group) assert(group == 100); return queues end},
			simEntityAtTerminalSystem = {getLineStopSimEntities = function(line, index, cargo)
				queryCalls = queryCalls + 1
				assert(index == 0 and cargo == 5)
				return line == 10 and {} or people
			end},
			simEntityAtVehicleSystem = {getVehicleSimEntitiesCountForCargoType = function(vehicle, cargo)
				assert(vehicle == 50 and cargo == 5); return 12
			end},
		},
	},
}
local state = {saved = {probeRevision = 1, counts = {OnArriveAtStop = 999}}, subscribed = true}
function state:get() return self.saved end
function state:set(v) self.saved = v end
function state:hasEventSubscriptions() return self.subscribed end
function state:subscribeToEvent() error("Upgrade must preserve subscriptions") end
local probe = dofile("probes/pathfinding/content/interlining_probe.script.lua")
probe.update({}, state, 0)
assert(state.saved.probeRevision == 2 and next(state.saved.counts) == nil)
assert(contains("revision=2"))
local event = {lineEntity = 10, vehicleEntity = 50, stopIndex = 0, lastStopIndex = 1}
local function arrive()
	assert(probe.handleEvent({}, state, "", "TransportVehicleSystem", "OnArriveAtStop", event) == nil)
end
arrive()
assert(contains('line=10 lineName="S1" stop=0 group=100 station=0 terminal=2'))
assert(contains('plannedLine=20 lineName="S2" boardingStop=0 group=100 station=0 terminal=3'))
assert(contains("actualStation=0 actualTerminal=4 capacity=40 occupied=12 loadAllowed=true maxLoad=0.75"))
assert(contains("person=70 plannedLine=20 boardingStop=0 alightingStop=2 alightingGroup=200"))
assert(contains("person=71 status=no_waiting_component"))
assert(not contains("SnapshotError"))
assert(components[70].SIM_ENTITY_AT_TERMINAL.line == 20 and event.lineEntity == 10)

-- Bounded iteration across large queues; no global entity enumeration.
messages = {}
people = {}
for i = 1, 1000 do people[i] = 70 end
queues = {}
for i = 1, 100 do queues[i] = {20, 0} end
queryCalls = 0
arrive()
assert(countLines("WaitingPerson ") == 32 and queryCalls == 4)
assert(countLines("QueueSampleLimit ") == 4 and contains("SnapshotLimit"))
messages = {}
people = {}
queryCalls = 0
arrive()
assert(queryCalls == 16 and contains("SnapshotLimit"))

-- Exceptions and stale stops cannot escape to the return-bearing dispatcher.
messages = {}
api.engine.system.lineSystem.getLineStops = function() error("fixture failure") end
arrive()
assert(contains("SnapshotError") and contains("fixture failure"))
messages = {}
event.stopIndex = 999
arrive()
assert(contains("status=missing_line_or_stop"))

-- Saved counters retain their values on repeated updates, and sampling resumes
-- at 100; upgrading alone resets counters, not every simulation update.
state.saved.counts.OnArriveAtStop = 60
probe.update({}, state, 0)
messages = {}
arrive()
assert(#messages == 0 and state.saved.counts.OnArriveAtStop == 61)
state.saved.counts.OnArriveAtStop = 99
arrive()
assert(contains("arrival=100"))
print("PASS: line/terminal mapping, zero-based indices, cargo capacity, stale entities, queue limits, error isolation, revision migration, and sampling")

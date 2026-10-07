-- Offline checks for observer neutrality and log sampling; not an engine test.
local messages = {}
log = { message = function(message) messages[#messages + 1] = message end }
api = {
	type = { ComponentType = { GAME_TIME = 1 } },
	engine = {
		util = { getWorld = function() return 0 end },
		getComponent = function() return {gameTime = 1000, tickCount = 10, updateCount = 8} end,
	},
}

local state = { saved = {}, subscriptions = {} }
function state:hasEventSubscriptions() return next(self.subscriptions) ~= nil end
function state:subscribeToEvent(name) self.subscriptions[name] = true end
function state:get() return self.saved end
function state:set(value) self.saved = value end

local probe = dofile("probes/pathfinding/content/interlining_probe.script.lua")
assert(data() == probe)
probe.update({}, state, 0)
assert(state.subscriptions.OnModifyPathfinding and state.subscriptions.OnArriveAtStop)
local originalCount = #messages
probe.update({}, state, 1)
assert(#messages == originalCount, "Subscriptions should initialize once")

local map = {[123] = 0.75}
local payload = {town2lineCostFactor = map, extra = {sentinel = true}}
for _ = 1, 100 do
	assert(probe.handleEvent({}, state, "", "SimPersonSystem", "OnModifyPathfinding", payload) == nil)
end
assert(payload.town2lineCostFactor == map and map[123] == 0.75 and payload.extra.sentinel)
assert(next(map, 123) == nil, "Probe must not add routing entries")
assert(state.saved.counts.OnModifyPathfinding == 100)
assert(#messages == originalCount + 21, "Log sampling must stay bounded")
assert(messages[#messages]:find("count=100", 1, true))

probe.handleEvent({}, state, "", "TransportVehicleSystem", "OnArriveAtStop",
	{lineEntity = 12, vehicleEntity = 34, stopIndex = 2, lastStopIndex = 1})
assert(messages[#messages]:find("line=12 vehicle=34 stop=2", 1, true))
probe.handleEvent({}, state, "", "SimPersonSystem", "OnStartedLineUsage", {entities = {{5, 12}}})
assert(messages[#messages]:find("samplePerson=5 sampleLine=12", 1, true))
probe.handleEvent({}, state, "", "SimPersonSystem", "OnCompletedLineUsage", {entities = {5}})
assert(messages[#messages]:find("samplePerson=5", 1, true))
local count = state.saved.counts.OnModifyPathfinding
probe.handleEvent({}, state, "", "WrongSystem", "OnModifyPathfinding", payload)
assert(state.saved.counts.OnModifyPathfinding == count)
probe.handleEvent({}, state, "", "SimPersonSystem", "OnModifyPathfinding", nil)
api.engine.getComponent = function() error("No game time") end
probe.handleEvent({}, state, "", "TransportVehicleSystem", "OnArriveAtStop", {})
assert(messages[#messages]:find("time=unavailable", 1, true))
print("PASS: subscription initialization, nil returns, unchanged routing payload, bounded logging, event summaries, and missing game-time handling")

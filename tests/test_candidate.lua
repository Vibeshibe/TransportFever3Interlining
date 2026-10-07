local candidate = dofile("src/interlining/candidate.lua")
local function stop(index, group, terminal)
	return {index = index, group = group, station = 0, terminal = terminal}
end
local function fixtures()
	-- Test 2 arrival 1. S3's next stop and alighting terminal are corroborated
	-- by later arrival 14 and the station's queue snapshots, not this instant alone.
	return {
		person = 75625, line = 98278, assigned_vehicle = -1, tick = 17011,
		boarding = stop(1, 97291, 1), alighting = stop(2, 97928, 0),
	}, {
		line = 97743, vehicle = 97959, tick = 17011,
		boarding = stop(1, 97291, 1), alighting = stop(2, 97928, 0),
		actual_station = 0, actual_terminal = 1,
		capacity = 40, occupied = 3, load_allowed = true, max_load_fraction = 1,
	}
end
local function digest(value)
	if type(value) ~= "table" then return tostring(value) end
	local keys, parts = {}, {}
	for key in pairs(value) do keys[#keys + 1] = key end
	table.sort(keys)
	for _, key in ipairs(keys) do parts[#parts + 1] = key .. "=" .. digest(value[key]) end
	return "{" .. table.concat(parts, ",") .. "}"
end
local waiting, arriving = fixtures()
local before = digest(waiting) .. digest(arriving)
local proposal = candidate.evaluate(waiting, arriving)
assert(proposal.status == "candidate" and proposal.requires_native_validation)
assert(proposal.proposed_line == 97743 and proposal.expected_line == 98278)
assert(proposal.proposed_vehicle == 97959 and proposal.observed_unused_capacity == 37)
assert(before == digest(waiting) .. digest(arriving), "Policy must not mutate snapshots")
proposal.proposed_line = 0
assert(arriving.line == 97743, "Proposal must not alias engine-derived input")

local function reject(reason, change)
	local w, a = fixtures()
	change(w, a)
	local result = candidate.evaluate(w, a)
	assert(result.status == "keep_current" and result.reason == reason,
		reason .. ": got " .. result.status .. "/" .. result.reason)
end
reject("different_boarding_terminal", function(_, a)
	a.line = 97742; a.boarding.terminal = 2; a.actual_terminal = 2
end) -- S2 geometry from Test 2; not a fabricated S2 arrival event
reject("different_alighting_terminal", function(_, a) a.alighting.terminal = 1 end)
reject("different_alighting_terminal", function(_, a) a.alighting.group = 98125 end)
reject("different_boarding_terminal", function(_, a) a.boarding.station = 1 end)
reject("unsupported_segment", function(_, a) a.alighting.index = 0 end)
reject("unsupported_segment", function(_, a) a.alighting.index = 4 end)
reject("unsupported_segment", function(w, a) w.alighting.group = w.boarding.group; a.alighting.group = w.boarding.group end)
reject("already_assigned_or_unknown", function(w) w.assigned_vehicle = 97959 end)
reject("already_assigned_or_unknown", function(w) w.assigned_vehicle = -2 end)
reject("already_assigned_or_unknown", function(w) w.assigned_vehicle = nil end)
reject("already_planned_line", function(w, a) a.line = w.line end)
reject("different_snapshot_ticks", function(_, a) a.tick = a.tick + 1 end)
reject("different_snapshot_ticks", function(w) w.tick = nil end)
reject("actual_terminal_differs_or_unknown", function(_, a) a.actual_terminal = 0 end)
reject("actual_terminal_differs_or_unknown", function(_, a) a.actual_station = nil end)
reject("loading_disabled_or_unknown", function(_, a) a.load_allowed = false end)
reject("loading_disabled_or_unknown", function(_, a) a.load_allowed = nil end)
reject("no_observed_capacity", function(_, a) a.occupied = 40 end)
reject("no_observed_capacity", function(_, a) a.max_load_fraction = 0.075 end)
reject("invalid_capacity_snapshot", function(_, a) a.capacity = math.huge end)
reject("invalid_capacity_snapshot", function(_, a) a.occupied = -1 end)
reject("invalid_capacity_snapshot", function(_, a) a.max_load_fraction = 0 / 0 end)
reject("invalid_capacity_snapshot", function(_, a) a.max_load_fraction = 1.1 end)
reject("missing_stop_mapping", function(_, a) a.alighting = nil end)
reject("invalid_entity", function(_, a) a.vehicle = -1 end)
assert(candidate.evaluate(nil, arriving).reason == "missing_snapshot")
print("PASS: Test2 S1/S3 proposal; 25 rejection cases; immutable inputs; no engine dependencies")

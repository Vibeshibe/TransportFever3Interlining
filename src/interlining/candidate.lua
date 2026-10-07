-- Pure policy prototype. Evaluates copied snapshots; never calls engine APIs.
-- A candidate is a proposal for native validation, not permission to board.
local candidate = {}

local function integer(value)
	return type(value) == "number" and value >= 0 and value < math.huge and value % 1 == 0
end

local function finite(value)
	return type(value) == "number" and value == value and math.abs(value) < math.huge
end

local function validStop(stop)
	return type(stop) == "table" and integer(stop.index) and integer(stop.group)
		and integer(stop.station) and integer(stop.terminal)
end

local function sameTerminal(a, b)
	return a.group == b.group and a.station == b.station and a.terminal == b.terminal
end

local function keep(reason)
	return {status = "keep_current", reason = reason}
end

function candidate.evaluate(waiting, arriving)
	if type(waiting) ~= "table" or type(arriving) ~= "table" then
		return keep("missing_snapshot")
	end
	if not integer(waiting.person) or not integer(waiting.line)
		or not integer(arriving.line) or not integer(arriving.vehicle) then
		return keep("invalid_entity")
	end
	if not integer(waiting.tick) or waiting.tick ~= arriving.tick then
		return keep("different_snapshot_ticks")
	end
	if waiting.assigned_vehicle ~= -1 then
		return keep("already_assigned_or_unknown")
	end
	if waiting.line == arriving.line then return keep("already_planned_line") end
	if not validStop(waiting.boarding) or not validStop(waiting.alighting)
		or not validStop(arriving.boarding) or not validStop(arriving.alighting) then
		return keep("missing_stop_mapping")
	end
	-- First prototype covers adjacent forward stops only. Loop wrap, repeated
	-- station occurrences, express stops, and longer routes need separate proofs.
	if waiting.alighting.index ~= waiting.boarding.index + 1
		or arriving.alighting.index ~= arriving.boarding.index + 1
		or waiting.boarding.group == waiting.alighting.group then
		return keep("unsupported_segment")
	end
	if not sameTerminal(waiting.boarding, arriving.boarding) then
		return keep("different_boarding_terminal")
	end
	if not sameTerminal(waiting.alighting, arriving.alighting) then
		return keep("different_alighting_terminal")
	end
	if arriving.actual_station ~= arriving.boarding.station
		or arriving.actual_terminal ~= arriving.boarding.terminal then
		return keep("actual_terminal_differs_or_unknown")
	end
	if arriving.load_allowed ~= true then return keep("loading_disabled_or_unknown") end
	if not integer(arriving.capacity) or not integer(arriving.occupied)
		or not finite(arriving.max_load_fraction)
		or arriving.max_load_fraction < 0 or arriving.max_load_fraction > 1 then
		return keep("invalid_capacity_snapshot")
	end
	local unused = math.floor(arriving.capacity * arriving.max_load_fraction) - arriving.occupied
	if unused < 1 then return keep("no_observed_capacity") end
	return {
		status = "candidate",
		reason = "same_terminal_adjacent_segment",
		person = waiting.person,
		observed_tick = waiting.tick,
		expected_line = waiting.line,
		expected_boarding_stop = waiting.boarding.index,
		expected_alighting_stop = waiting.alighting.index,
		expected_vehicle = waiting.assigned_vehicle,
		proposed_line = arriving.line,
		proposed_boarding_stop = arriving.boarding.index,
		proposed_alighting_stop = arriving.alighting.index,
		proposed_vehicle = arriving.vehicle,
		observed_unused_capacity = unused,
		requires_native_validation = true,
	}
end

return candidate

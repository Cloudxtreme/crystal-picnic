function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

local NFRAMES = 11
local TICKS_EACH = 3

function logic(pid)
	local entity = get_particle_blackboard(pid, 0)
	local offset_x = get_particle_blackboard(pid, 1)
	local offset_y = get_particle_blackboard(pid, 2)
	local ticks = get_particle_blackboard(pid, 3)

	local x, y = get_entity_position(entity)

	if (not (x == nil)) then
		set_particle_position(pid, x + offset_x, y + offset_y)
	end

	ticks = ticks + 1

	if (ticks >= NFRAMES*TICKS_EACH) then
		remove_particle(pid)
		return
	end

	local frame = math.floor(ticks / TICKS_EACH)
	set_particle_bitmap_index(pid, frame)

	set_particle_blackboard(pid, 3, ticks)
end


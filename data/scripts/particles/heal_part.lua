function start(this_pgid)
	pgid = this_pgid
end

function logic(pid)
	local entity_start_x = get_particle_blackboard(pid, 0)
	local entity_start_y = get_particle_blackboard(pid, 1)
	local entity_id = get_particle_blackboard(pid, 2)
	local ticks = get_particle_blackboard(pid, 3)
	local nframes = get_particle_blackboard(pid, 4)
	local ticks_per_frame = get_particle_blackboard(pid, 5)
	local frame = get_particle_blackboard(pid, 6)

	local x, y = get_entity_position(entity_id)
	set_particle_blackboard(pid, 0, x)
	set_particle_blackboard(pid, 1, y)
	local dx = x - entity_start_x
	local dy = y - entity_start_y
	local x2, y2 = get_particle_position(pid)
	set_particle_position(pid, x2+dx, y2+dy)

	ticks = ticks + 1
	if (ticks == ticks_per_frame) then
		ticks = 0
		frame = frame + 1
	end

	if (frame >= nframes) then
		remove_particle(pid)
		return
	end

	set_particle_bitmap_index(pid, frame)

	set_particle_blackboard(pid, 3, ticks)
	set_particle_blackboard(pid, 6, frame)
end

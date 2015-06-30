function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function logic(pid)
	local nframes = get_particle_blackboard(pid, 0)
	local ticks_each = get_particle_blackboard(pid, 1)
	local ticks = get_particle_blackboard(pid, 2)

	local x, y = get_particle_position(pid)

	ticks = ticks + 1

	if (ticks >= nframes*ticks_each) then
		remove_particle(pid)
		return
	end

	local frame = math.floor(ticks / ticks_each)
	set_particle_bitmap_index(pid, frame)

	set_particle_blackboard(pid, 2, ticks)
end


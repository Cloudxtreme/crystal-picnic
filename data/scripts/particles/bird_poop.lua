function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	remove_particle(pid)
end

function logic(pid)
	local ticks = get_particle_blackboard(pid, 0)
	ticks = ticks + 1
	if (ticks >= 15) then
		remove_particle(pid)
		return
	end
	set_particle_blackboard(pid, 0, ticks)
	set_particle_bitmap_index(pid, ticks / 5)
end

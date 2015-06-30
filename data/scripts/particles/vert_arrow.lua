function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	play_sample("sfx/hit.ogg", 1, 0, 1)
	remove_particle(pid)
end

function logic(pid)
	local up = not (get_particle_blackboard(pid, 0) == 0)
	local inc
	if (up) then
		inc = -5
	else
		inc = 5
	end
	local x, y = get_particle_position(pid)
	y = y + inc
	if (y < 0 or y >= get_battle_height()) then
		remove_particle(pid)
		return
	end
	set_particle_position(pid, x, y)
end

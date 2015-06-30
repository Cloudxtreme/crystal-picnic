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
	local right = not (get_particle_blackboard(pid, 0) == 0)
	local inc
	if (right) then
		inc = 5
	else
		inc = -5
	end
	local x, y = get_particle_position(pid)
	x = x + inc
	if (x < 0 or x >= get_battle_width()) then
		remove_particle(pid)
		return
	end
	set_particle_position(pid, x, y)
end

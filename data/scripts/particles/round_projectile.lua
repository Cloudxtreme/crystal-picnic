function start(this_pgid)
	pgid = this_pgid
end

function collide(pid, entity_id)
	play_sample("sfx/hit.ogg", 1, 0, 1)
	remove_particle(pid)
end

function logic(pid)
	local x, y = get_particle_position(pid)
	local dx = get_particle_blackboard(pid, 0)
	local dy = get_particle_blackboard(pid, 1)
	
	x = x + dx
	y = y + dy

	if (x < 0 or y < 0 or x >= get_battle_width() or y >= get_battle_height()) then
		remove_particle(pid)
	end

	set_particle_position(pid, x, y)
end

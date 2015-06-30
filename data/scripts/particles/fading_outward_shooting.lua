function start(this_pgid)
	pgid = this_pgid
end

function logic(pid)
	local x = get_particle_blackboard(pid, 0)
	local y = get_particle_blackboard(pid, 1)
	local vx = get_particle_blackboard(pid, 2)
	local vy = get_particle_blackboard(pid, 3)
	local ticks = get_particle_blackboard(pid, 4)
	local life = get_particle_blackboard(pid, 5)

	x = x + vx * life
	y = y + vy * life
	
	set_particle_position(pid, x, y)
	
	local a = 1 - (life / ticks)
	set_particle_tint(pid, a, a, a, a)

	life = life + 1
	
	if (life >= ticks) then
		remove_particle(pid)
	else
		set_particle_blackboard(pid, 5, life)
	end
end

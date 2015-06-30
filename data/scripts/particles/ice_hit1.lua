function start(this_pgid)
	pgid = this_pgid
end

function start_particle(pid)
	local a = rand(1000)/1000 * math.pi * 2
	local vx = math.cos(a) * 1.2
	local vy = math.sin(a) * 1.2
	set_particle_blackboard(pid, 2, vx)
	set_particle_blackboard(pid, 3, vy)
	set_particle_blackboard(pid, 4, 0)
end

local LIFE = 25

function logic(pid)
	local angle = get_particle_blackboard(pid, 0)
	local angle_delta = get_particle_blackboard(pid, 1)
	local vx = get_particle_blackboard(pid, 2)
	local vy = get_particle_blackboard(pid, 3)
	local ticks = get_particle_blackboard(pid, 4)

	local x, y = get_particle_position(pid)
	
	x = x + vx
	y = y + vy
	
	set_particle_position(pid, x, y)
	
	local r, g, b, a = get_particle_tint(pid)
	a = a - 1 / LIFE;
	set_particle_tint(pid, r, g, b, a)

	angle = angle + angle_delta
	set_particle_angle(pid, angle)
	set_particle_blackboard(pid, 0, angle)

	ticks = ticks + 1
	
	if (ticks >= LIFE) then
		remove_particle(pid)
	else
		set_particle_blackboard(pid, 4, ticks)
	end
end

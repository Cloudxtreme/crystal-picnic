function start(this_pgid)
	pgid = this_pgid
end

function start_particle(pid)
	local a = rand(1000)/1000 * math.pi * 2
	local vx = math.cos(a) * 1.2
	local vy = math.sin(a) * 1.2
	set_particle_blackboard(pid, 0, vx)
	set_particle_blackboard(pid, 1, vy)
	set_particle_blackboard(pid, 2, 0)
end

local LIFE = 40

function logic(pid)
	local vx = get_particle_blackboard(pid, 0)
	local vy = get_particle_blackboard(pid, 1)
	local ticks = get_particle_blackboard(pid, 2)

	local x, y = get_particle_position(pid)
	
	x = x + vx
	y = y + vy
	
	set_particle_position(pid, x, y)
	
	local r, g, b, a = get_particle_tint(pid)
	r = r - 0.25 / LIFE;
	if (r < 0) then r = 0 end
	g = g - 0.25 / LIFE;
	if (g < 0) then g = 0 end
	b = b - 0.25 / LIFE;
	if (b < 0) then b = 0 end
	a = a - 1 / LIFE;
	set_particle_tint(pid, r, g, b, a)

	ticks = ticks + 1
	
	if (ticks >= LIFE) then
		remove_particle(pid)
	else
		set_particle_blackboard(pid, 2, ticks)
	end
end

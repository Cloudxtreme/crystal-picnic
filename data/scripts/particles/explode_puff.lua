function start(this_pgid)
	pgid = this_pgid
end

function start_particle(pid)
	local vx = ((rand(100) / 100) - 0.5) * 3
	local vy = -(0.5 + rand(100) / 200)
	local start_x, start_y = get_particle_position(pid)
	set_particle_blackboard(pid, 0, vx)
	set_particle_blackboard(pid, 1, vy)
end

function logic(pid)
	local vx = get_particle_blackboard(pid, 0)
	local vy = get_particle_blackboard(pid, 1)
	local end_y = get_particle_blackboard(pid, 2)

	local x, y = get_particle_position(pid)
	
	x = x + vx
	y = y + vy
	
	set_particle_position(pid, x, y)

	local sub = 0.05

	if (vx == 0) then
		-- do nothing
	elseif (math.abs(vx) <= sub) then
		vx = 0
	elseif (vx > 0) then
		vx = vx - sub
	else
		vx = vx + sub
	end
	
	set_particle_blackboard(pid, 0, vx)
	
	local dist = y - end_y
	
	if (dist >= 0 and dist < 15) then
		local start_a = get_particle_blackboard(pid, 3)
		local a = 1.0 - (dist / 15)
		a = a * start_a
		local r, g, b = get_particle_tint(pid)
		set_particle_tint(pid, r, g, b, a)
	end

	if (dist < 0) then
		remove_particle(pid)
	end
end

function start(this_pgid)
	pgid = this_pgid
end

function start_particle(pid)
end

function logic(pid)
	local sx = get_particle_blackboard(pid, 0)
	local sy = get_particle_blackboard(pid, 1)

	local r = get_particle_blackboard(pid, 2)

	local dx = get_particle_blackboard(pid, 3)
	local dy = get_particle_blackboard(pid, 4)

	local lifetime = get_particle_blackboard(pid, 5)
	local count = get_particle_blackboard(pid, 6)
	--local popup_time = get_particle_blackboard(pid, 7)
	--local popped_up = get_particle_blackboard(pid, 8)

	count = count + LOGIC_MILLIS / 1000

	if (count >= lifetime) then
		remove_particle(pid)
		return
	--elseif (popped_up == 0 and count > popup_time) then
		--push_entity_to_back(pid)
		--set_particle_blackboard(pid, 8, 1)
	end

	set_particle_blackboard(pid, 6, count)

	local x, y = get_particle_position(pid)
	local distx = x - sx
	local disty = y - sy
	local dist = math.sqrt(distx*distx + disty*disty)
	if (count < 1.75) then
		if (dist < r) then
			x = x + dx
			y = y + dy
			set_particle_position(pid, x, y)
		end
	end

	set_particle_angle(pid, count / lifetime * math.pi * 2 * 2)

	local r, g, b, a
	if (count >= 1.75) then
		local v2 = (count-1.75) / (lifetime-1.75)
		local v = 1 - v2
		r = v
		g = v
		b = v
		a = v
	else
		r = 1
		g = 1
		b = 1
		a = 1
	end
	d = 1 - count / lifetime
	set_particle_tint(pid, r*d, g*d, b*d, a*d)

	if (count < 0.25) then
		local scx = count / 0.25
		local scy = count / 0.25
		set_particle_scale(pid, scx, scy)
	elseif (count > 1.75) then
		local scx = 1 - (count - 1.75) / 0.75
		local scy = 1 - (count - 1.75) / 0.75
		set_particle_scale(pid, scx, scy)
	else
		set_particle_scale(pid, 1, 1)
	end
end


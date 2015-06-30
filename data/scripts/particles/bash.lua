function start(this_pgid)
	pgid = this_pgid
end

function start_particle(pid)
end

function logic(pid)
	local start_x = get_particle_blackboard(pid, 0)
	local start_y = get_particle_blackboard(pid, 1)

	set_particle_position(pid, start_x+(rand(3)-1), start_y+(rand(3)-1))

	local r, g, b, a = get_particle_tint(pid)

	r = r - 0.01
	g = g - 0.01
	b = b - 0.01
	a = a - 0.01

	if (a <= 0) then
		remove_particle(pid)
		return
	end

	set_particle_tint(pid, r, g, b, a)
end


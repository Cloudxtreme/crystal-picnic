function add_explosion(x, y)
	local NFIRE = 30
	local NSMOKE = 30
	-- fire
	local pgid3 = add_particle_group("explode_fire", 0, PARTICLE_HURT_NONE, "fire16-1", "fire16-2", "fire16-3")
	local i
	for i=1,NFIRE do
		local val = rand(100)/400+0.25
		local r = 0.75
		local g = val
		local b = 0
		local a = 1
		local bmp_idx = rand(3)
		local pid3 = add_particle(
			pgid3,
			0, 0,
			r, g, b, a,
			bmp_idx,
			0,
			false,
			false
		)
		set_particle_position(pid3, x, y)
	end
	-- smoke
	local pgid2 = add_particle_group("explode_puff", 0, PARTICLE_HURT_NONE, "smoke16-1", "smoke16-2", "smoke16-3")
	local i
	for i=1,NSMOKE do
		local val = rand(255)/255
		local r = val
		local g = val
		local b = val
		local a = rand(100)/200 + 0.5
		local bmp_idx = rand(3)
		local pid2 = add_particle(
			pgid2,
			0, 0,
			r, g, b, a,
			bmp_idx,
			0,
			false,
			false
		)
		set_particle_position(pid2, x, y)
		set_particle_blackboard(pid2, 2, y - (rand(30)+40))
		set_particle_blackboard(pid2, 3, a)
	end
end


function start(this_pgid)
	pgid = this_pgid
end

function collide(pid, entity_id)
	local NPARTICLES = 20
	local x, y = get_particle_position(pid)
	local pgid2 = add_particle_group("ice_hit1", 0, PARTICLE_HURT_NONE, "ice_shard1", "ice_shard2", "ice_shard3")
	local i
	for i=1,NPARTICLES do
		local r = 1
		local g = 1
		local b = 1
		local a = 1
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
		set_particle_blackboard(pid2, 0, rand(1000)/1000*math.pi*2);
		set_particle_blackboard(pid2, 1, (rand(3) - 1) * (0.1 + ((rand(1000)/1000)*0.1)))
	end
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

function start(this_pgid)
	pgid = this_pgid
end

function collide(pid, entity_id)
	play_sample("sfx/fire_hit.ogg", 1, 0, 1)
	remove_particle(pid)
	local pgid2 = add_particle_group("fire_particle", 0, PARTICLE_HURT_NONE,
		"fire_particle1",
		"fire_particle2",
		"fire_particle3",
		"fire_particle4",
		"fire_particle5",
		"fire_particle6",
		"fire_particle7",
		"fire_particle8",
		"fire_particle9",
		"fire_particle10",
		"fire_particle11"
	)
	local w, h = get_entity_bone_size(entity_id)
	local x, y = get_entity_position(entity_id)
	for i=1,3 do
		local right
		if (rand(2) == 0) then
			right = true
		else
			right = false
		end
		local pid2 = add_particle(
			pgid2,
			1, 1,
			1, 1, 1, 1,
			0,
			0,
			right,
			false
		)
		set_particle_blackboard(pid2, 0, entity_id)
		set_particle_blackboard(pid2, 1, rand(w-7)-(w-7)/2)
		set_particle_blackboard(pid2, 2, -(rand(10)+h-10))
		set_particle_blackboard(pid2, 3, 0)

		set_particle_position(pid2, x+get_particle_blackboard(pid2, 1))
		set_particle_position(pid2, y+get_particle_blackboard(pid2, 2))
	end
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

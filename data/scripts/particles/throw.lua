function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	if (get_particle_group_alignment(pgid) == PARTICLE_HURT_ENEMY) then
		play_sample("sfx/hit.ogg", 1, 0, 1)
		set_particle_group_alignment(pgid, PARTICLE_HURT_NONE)
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
	local falling = get_particle_blackboard(pid, 3)
	local x, y = get_particle_position(pid)
	x = x + inc
	if (x < 0) then
		x = 0
		falling = 1
	elseif (x >= get_battle_width()) then
		x = get_battle_width()
		falling = 1
	end

	local start_x = get_particle_blackboard(pid, 2)
	local diff = math.abs(start_x-x)
	if (diff >= 80) then
		falling = 1
	end

	if (not (falling == 0)) then
		local vel_y = get_particle_blackboard(pid, 4)
		vel_y = vel_y + 0.05
		local max = 2.5
		if (vel_y > 2.5) then
			vel_y = 2.5
		end
		set_particle_blackboard(pid, 4, vel_y)
		y = y + vel_y
	end

	if (particle_is_colliding_with_area(pid)) then
		local pgid2 = add_particle_group("egbert_weapon", 0, PARTICLE_HURT_EGBERT, get_player_weapon_name("egbert"))
		local pid2 = add_particle(
			pgid2,
			5, 5,
			1, 1, 1, 1,
			0,
			0,
			true,
			false
		)
		set_particle_position(pid2, x, y-15)
		
		remove_particle(pid)
	end

	set_particle_position(pid, x, y)

	local count = get_particle_blackboard(pid, 1)
	count = count + 0.3333334;
	local frame = count % 8;
	set_particle_bitmap_index(pid, math.floor(frame));
	set_particle_blackboard(pid, 1, count)
	set_particle_blackboard(pid, 3, falling)
end

function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function mushroom(pid)
	local x, y = get_particle_position(pid)

	local pgid2 = add_particle_group("anim", 0, PARTICLE_HURT_PLAYER,
		"mushroom_cloud/1",
		"mushroom_cloud/2",
		"mushroom_cloud/3",
		"mushroom_cloud/4",
		"mushroom_cloud/5",
		"mushroom_cloud/6",
		"mushroom_cloud/7",
		"mushroom_cloud/8",
		"mushroom_cloud/9",
		"mushroom_cloud/10",
		"mushroom_cloud/11",
		"mushroom_cloud/12",
		"mushroom_cloud/13"
	)
	local pid2 = add_particle(
		pgid2,
		80, 80,
		1, 1, 1, 1,
		0,
		HIT_UP,
		false,
		true
	)
	set_particle_damage(pid2, 3)

	set_particle_position(pid2, x, y)
	set_particle_draw_offset(pid2, 0, -32)
	set_particle_blackboard(pid2, 0, 13)
	set_particle_blackboard(pid2, 1, 3)
	set_particle_blackboard(pid2, 2, 0)

	play_sample("sfx/bomb.ogg", 1, 0, 1)
	for i=1,3 do
		local r = rand(1000)
		r = r / 1000 * 2
		r = r - 1
		local speed = rand(1000) / 2000
		play_sample("sfx/bomb.ogg", 1, r, speed)
	end

	remove_particle(pid)
end

function collide(pid, entity_id)
	local num_bounces = get_particle_blackboard(pid, 2)

	if (num_bounces >= 3) then
		mushroom(pid)
	else
		local x, y = get_particle_position(pid)
		add_explosion(x, y)
		play_sample("sfx/bomb.ogg", 1, 0, 1)
		remove_particle(pid)
	end
end

function logic(pid)
	local x, y = get_particle_position(pid)
	local vx = get_particle_blackboard(pid, 0)
	local vy = get_particle_blackboard(pid, 1)
	local dir = get_particle_blackboard(pid, 5)

	if (y >= get_battle_height()) then
		remove_particle(pid)
	end

	if (x < 0) then
		play_sample("sfx/grenade_tink.ogg", 1, 0, 1)
		x = 0
		vx = -vx
		dir = -dir
		set_particle_position(pid, x, y)
		set_particle_blackboard(pid, 0, vx)
		set_particle_blackboard(pid, 1, vy)
		set_particle_blackboard(pid, 5, dir)
		return
	elseif (x >= get_battle_width()) then
		play_sample("sfx/grenade_tink.ogg", 1, 0, 1)
		x = get_battle_width()-1
		vx = -vx
		dir = -dir
		set_particle_position(pid, x, y)
		set_particle_blackboard(pid, 0, vx)
		set_particle_blackboard(pid, 1, vy)
		set_particle_blackboard(pid, 5, dir)
		return
	end

	local num_bounces = get_particle_blackboard(pid, 2)
	local time_left = get_particle_blackboard(pid, 3)
	local angle = get_particle_blackboard(pid, 4)

	if (num_bounces >= 3) then
		time_left = time_left - (1.0/LOGIC_RATE)
		set_particle_blackboard(pid, 3, time_left)
		if (time_left <= 0) then
			mushroom(pid)
		end
		return
	end

	if (num_bounces == 0) then
		angle = angle + 0.25 * dir
	else
		angle = angle + 0.5 * dir
	end

	local x1, y1, x2, y2, percent = get_level_collision(x, y, 2)

	if (x1 == nil) then
		x = x + vx
		y = y + vy
		if (vx < 0) then
			vx = vx + 0.05
		elseif (vx > 0) then
			vx = vx - 0.05
		end
		vy = vy + 0.1
	else
		play_sample("sfx/grenade_tink.ogg", 1, 0, 1)
		num_bounces = num_bounces + 1
		y = y - vy
		vy = -1.5
		vx = dir * 2.5
	end

	set_particle_position(pid, x, y)
	set_particle_angle(pid, angle)

	set_particle_blackboard(pid, 0, vx)
	set_particle_blackboard(pid, 1, vy)
	set_particle_blackboard(pid, 2, num_bounces)
	set_particle_blackboard(pid, 4, angle)
end

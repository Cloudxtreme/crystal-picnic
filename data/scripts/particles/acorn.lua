local SPEED = 4
local ROLL_SPEED = 1

function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	play_sample("sfx/knock.ogg", 1, 0, 1)
	remove_particle(pid)
end

function logic(pid)
	local frames = get_particle_blackboard(pid, 0)
	local frame = get_particle_blackboard(pid, 1)
	local tick = get_particle_blackboard(pid, 2)
	local rotate_right = get_particle_blackboard(pid, 3)

	local x, y = get_particle_position(pid)
	local x1, y1, x2, y2, percent = get_level_collision(x, y, 2)

	if (x1 == nil or math.abs(y1-y2) < 3) then
		y = y + SPEED
		set_particle_blackboard(pid, 4, 0)
	else
		if (get_particle_blackboard(pid, 4) == 0) then
			play_sample("sfx/knock.ogg", 1, 0, 1)
		end
		set_particle_blackboard(pid, 4, 1)
		if (y1 < y2) then
			rotate_right = 1
		else
			rotate_right = 0
		end
		if (y2 > y1) then
			local tmpx = x1
			local tmpy = y1
			x1 = x2
			y1 = y2
			x2 = tmpx
			y2 = tmpy
		end
		local dx = x1 - x2
		local dy = y1 - y2
		local a = math.atan2(dy, dx)
		x = x + math.cos(a) * ROLL_SPEED
		y = y + math.sin(a) * ROLL_SPEED
	end

	if (y >= get_battle_height()) then
		remove_particle(pid)
		return
	end

	set_particle_position(pid, x, y)

	tick = tick + 1
	if (tick >= 2) then
		tick = 0
		if (rotate_right == 1) then
			frame = frame + 1
			if (frame >= frames) then
				frame = 0
			end
		else
			frame = frame - 1
			if (frame < 0) then
				frame = frames - 1
			end
		end
	end

	set_particle_bitmap_index(pid, frame)

	set_particle_blackboard(pid, 1, frame)
	set_particle_blackboard(pid, 2, tick)
	set_particle_blackboard(pid, 3, rotate_right)
end

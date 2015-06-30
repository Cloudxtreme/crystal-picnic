local VISION_SPEED = 0.025
local SIGHT_DIST = 50
local SPIT_SPEED = 4

local next_attack = rand(5) -- each 1 is 0.25 seconds, see below
local max_angle = math.pi -- 180
local min_angle = max_angle - math.pi/3 -- 120
local angle = math.random()*(max_angle-min_angle) + min_angle
local da
if (rand(2) == 0) then
	da = -VISION_SPEED
else
	da = VISION_SPEED
end

function start(this_id)
	id = this_id
	local x, y = get_random_start_platform(32, 16)
	set_entity_position(id, x, y)
	local right
	if (rand(2) == 0) then
		right = true
	else
		right = false
	end
	set_entity_right(id, right)
	set_entity_immovable(id, true)
end

function get_attack_sound()
	return ""
end

function decide()
	angle = angle + da
	if (da < 0) then
		if (angle < min_angle) then
			da = -da
		end
	else
		if (angle > max_angle) then
			da = -da
		end
	end

	local a
	if (get_entity_right(id)) then
		a = -(angle + math.pi)
	else
		a = angle
	end

	local dx = math.cos(a)
	local dy = math.sin(a)

	local x, y = get_entity_position(id)
	-- get position of face (-16) plus offset out of face (6)
	x = x + dx * 4
	y = y - 22 + dy * 4

	local x2 = x + dx * SIGHT_DIST
	local y2 = y + dy * SIGHT_DIST

	if (next_attack <= 0) then
		if (checkcoll_line_player(x, y, x2, y2)) then
			next_attack = rand(4) + 1

			local pgid = add_particle_group("round_projectile", 0, PARTICLE_HURT_PLAYER, "sunflower_seed")
			local seed = add_particle(
				pgid,
				3, 3,
				1, 1, 1, 1,
				0,
				HIT_SIDE,
				true,
				false
			)
			set_particle_position(seed, x, y)

			set_particle_blackboard(seed, 0, dx * SPIT_SPEED)
			set_particle_blackboard(seed, 1, dy * SPIT_SPEED)

			return "attack"
		else
			next_attack = 0
			return "rest 0 nostop"
		end
	else
		next_attack = next_attack - 1
	end

	return "rest 0.25 nostop"
end

function get_should_auto_attack()
	return false
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function logic()
	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

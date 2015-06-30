function start(this_id)
	id = this_id
	load_sample("sfx/bomb.ogg", false)
	load_sample("sfx/equip_gun.ogg", false)
	load_sample("sfx/fire_bazooka.ogg", false)
end

function stop()
	destroy_sample("sfx/bomb.ogg")
	destroy_sample("sfx/equip_gun.ogg")
	destroy_sample("sfx/fire_bazooka.ogg")
end

function get_attack_sound()
	return ""
end

function get_should_auto_attack()
	return false
end

local MIN_DIST = 50
local SHOOT_DIST = 110
local MAX_Y_DIST = 12

local ready_count = 0
local readying_attack = false

function logic()
	local hp = ai_get(id, "hp " .. id)
	
	if (readying_attack) then
		if (hp <= 0) then
			readying_attack = false
			return
		end
		local anim_len = get_entity_animation_length(id, "attack")
		ready_count = ready_count + LOGIC_MILLIS
		if (ready_count >= anim_len) then	
			play_sample("sfx/fire_bazooka.ogg", 1, 0, 1)
			
			local x, y = ai_get(id, "entity_positions_by_id " .. id)
			
			-- add rocket "particle"
			local pgid = add_particle_group("rocket", 0, PARTICLE_HURT_PLAYER, "rocket")
			local rocket = add_particle(
				pgid,
				8, 8,
				0, 0, 0, 1,
				0,
				HIT_UP,
				right,
				true
			)
			set_particle_bullet_time(rocket, true)
			local offs
			if (right) then
				offs = 20
			else
				offs = -20
			end
			set_particle_position(rocket, x + offs, y - 12)
			local value
			if (right) then
				value = 1
			else
				value = 0
			end
			set_particle_blackboard(rocket, 0, value)
			
			ready_count = 0
			readying_attack = false
		end
	end

	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

function decide()
	if (readying_attack) then
		return "rest " .. (LOGIC_MILLIS/1000) .. " nostop"
	end

	local player_id = ai_get(id, "A_PLAYER")

	if (player_id == nil) then
		return "rest " .. (LOGIC_MILLIS/1000) .. " nostop"
	end

	local dist = ai_get(id, "entity_distance " .. player_id)
	local x, y, px, py =
		ai_get(id, "entity_positions_by_id " .. id .. " " .. player_id)
	local ydist = math.abs(y-py)
	local hp = ai_get(id, "hp " .. id)

	if (dist < MIN_DIST and ydist < MAX_Y_DIST) then
		local ret = "seek pos "
		if (px < x) then
			return ret .. (px + SHOOT_DIST) .. " " .. y
		else
			return ret .. (px - SHOOT_DIST) .. " " .. y
		end
	elseif (hp > 0 and dist < SHOOT_DIST and ydist < MAX_Y_DIST) then		
		local player_id = ai_get(id, "A_PLAYER")

		if (player_id == nil) then
			return "rest " .. (LOGIC_MILLIS/1000) .. " nostop"
		end

		local x, y, px, py = ai_get(id, "entity_positions_by_id " .. id .. " " .. player_id)
		if (x < px) then
			right = true
		else
			right = false
		end

		set_entity_right(id, right)

		play_sample("sfx/equip_gun.ogg", 1, 0, 1)
			
		readying_attack = true
		return "attack"
	else
		local r = (rand(1000)/1000)
		return "seek within " ..
			SHOOT_DIST .. " 20 A_PLAYER nostop rest " ..
			r .. " nostop"
	end
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

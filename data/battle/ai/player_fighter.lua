function start(this_id)
	id = this_id
	is_bisou = get_entity_name(id) == "bisou"
end

function get_should_auto_attack()
	local name = get_entity_name(id)
	local melee_attack_name
	if (name == "egbert") then
		melee_attack_name = "SLASH"
	elseif (name == "frogbert") then
		melee_attack_name = "KICK"
	else
		melee_attack_name = "ROLL"
	end
	local have_basic_attack = not (player_ability_button(id, "ATTACK") == nil)
	if (player_ability_button(id, melee_attack_name) == nil and (not have_basic_attack) and (not (name == "egbert" and player_ability_button(id, "THROW")))) then
		return false
	end
	local min, max
	local name = get_entity_name(id)
	if (name == "egbert") then
		local have_throw = not (player_ability_button(id, "THROW") == nil)
		local have_slash = not (player_ability_button(id, "SLASH") == nil)
		if (have_throw and (not have_slash or rand(5) == 0)) then
			min = 0
			max = 1
			melee_name = "THROW"
			melee_type = ABILITY_THROW
		else
			min = 1
			max = 2
			melee_name = "SLASH"
			melee_type = ABILITY_SLASH
		end
	elseif (name == "frogbert") then
		min = 4
		max = 5
		melee_name = "KICK"
		melee_type = ABILITY_KICK
	else
		min = 7
		max = 9
		melee_name = "ROLL"
		melee_type = ABILITY_ROLL
	end
	local mod = get_time() % 10
	if (not have_basic_attack) then
		min = 0
		if (is_bisou) then
			max = 7.5
		else
			max = 5.0
		end
	end
	if (not (player_ability_button(id, melee_name) == nil) and mod >= min and mod <= max) then
		melee_ability_in_use = true
	else
		melee_ability_in_use = false
	end
	return true
end

local MAX_Y_DIST = 12

function decide()
	local do_long_range
	local name = get_entity_name(id)
	local do_burrow = false
	local do_heal = -1
	if (is_bisou) then
		if (get_player_weapon_name("bisou") == "") then
			do_long_range = false
		else
			local t = get_time() % 10
			if (t >= 8.0 and t <= 9.0 and rand(5) == 0 and get_mp(id) >= get_ability_cost(id, "BURROW")) then
				do_long_range = false
				do_burrow = true
			elseif (t >= 9.0 and t <= 10.0 and rand(5) == 0 and get_mp(id) >= get_ability_cost(id, "HEAL")) then
				do_long_range = true
				local n = get_num_players()
				for i=0,n do
					local _id = get_player_id(i)
					local hp = get_hp(_id)
					if (hp < 10) then
						do_long_range = false
						do_heal = _id
						break
					end
				end
			else
				do_long_range = not (player_ability_button(id, "ATTACK") == nil)
			end
		end
	else
		local min, max
		if (name == "egbert") then
			min = 5
			max = 7
			long_range_name = "ICE"
			long_range_type = ABILITY_ICE
		else
			min = 7
			max = 9
			long_range_name = "FIRE"
			long_range_type = ABILITY_FIRE
		end
		local mod = get_time() % 10
		if (not (player_ability_button(id, long_range_name) == nil) and mod >= min and mod <= max) then
			long_range_ability_in_use = true
			do_long_range = true
		else
			long_range_ability_in_use = false
			do_long_range = false
		end
	end
	if (do_long_range) then
		local enemy_id = ai_get(id, "nearest_enemy")

		if (enemy_id == nil) then
			return "rest " .. (LOGIC_MILLIS/1000) .. " nostop"
		end

		local x, y, px, py =
			ai_get(id, "entity_positions_by_id " .. id .. " " .. enemy_id)

		local ydiff = y - py
		if (math.abs(ydiff) > MAX_Y_DIST or get_hp(enemy_id) <= 0) then
			return "rest " .. (LOGIC_MILLIS/1000) .. " nostop"
		end

		if (x < px) then
			right = true
		else
			right = false
		end

		set_entity_right(id, right)

		return "attack nostop rest 0.3 nostop"
	else
		if (is_bisou) then
			if (do_burrow and not (player_ability_button(id, "BURROW") == nil)) then
				return "do_ability " .. ABILITY_BURROW
			elseif (not (do_heal == -1) and not (player_ability_button(id, "HEAL") == nil)) then
				return "heal " .. do_heal
			end
		elseif (name == "frogbert") then
			local t = get_time() % 10
			if (t >= 5 and t <= 6 and rand(2) == 0 and get_mp(id) >= get_ability_cost(id, "PLANT") and not (player_ability_button(id, "PLANT") == nil)) then
				return "do_ability " .. ABILITY_PLANT
			end
		end
		local r = (rand(1000)/1000) * 0.5
		return "seek A_ENEMY nostop rest " .. (r+0.5) .. " nostop"
	end
end

function get_melee_distance()
	if (melee_ability_in_use) then
		if (melee_type == ABILITY_THROW) then
			return 150
		else
			return 75
		end
	else
		return nil
	end
end

function get_attack_type()
	if (melee_ability_in_use and get_mp(id) >= get_ability_cost(id, melee_name)) then
		return melee_type
	elseif (long_range_ability_in_use and get_mp(id) >= get_ability_cost(id, long_range_name)) then
			return long_range_type
	end
	return ABILITY_ATTACK
end

function got_hit(hitter)
	if (get_entity_name(hitter) == "antboss") then
		call_on_battle_script("launch_player")
	end
end


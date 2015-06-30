function start(this_id)
	id = this_id
	set_hp(id, 8)
	set_battle_entity_attack(id, 2)
end

function get_attack_sound()
	return ""
end

local SEARCH_ANGLE = -math.pi/9
local SEARCH_DIST = 60
local JUMP_FORCE_X = 7
local JUMP_FORCE_Y = 0.6
local WIDTH = 64
local HEIGHT = 64

local next_attack = 0
local should_attack = false
local attack_timeout = 0

function logic()
	if (get_hp(id) <= 0) then
		return
	end

	if (should_attack) then
		if (get_height_from_ground(id) > 2) then
			set_should_attack(id, true)
			should_attack = false
		else
			attack_timeout = attack_timeout - 1
			if (attack_timeout <= 0) then
				should_attack = false
			end
		end
		return
	end

	if (next_attack > 0) then
		next_attack = next_attack - 1
		return
	end

	local x, y = get_entity_position(id)
	y = y - 4

	local a
	if (get_entity_right(id)) then
		a = SEARCH_ANGLE
	else
		a = SEARCH_ANGLE + math.pi
	end

	local x2 = x + math.cos(a) * SEARCH_DIST
	local y2 = y + math.sin(a) * SEARCH_DIST

	if (checkcoll_line_player(x, y, x2, y2)) then
		play_sample("sfx/enemy_jump.ogg", 1, 0, 1)
		apply_force(id, HIT_UP, get_entity_right(id), JUMP_FORCE_X, JUMP_FORCE_Y)
		set_battle_entity_jumping(id)
		set_entity_animation(id, "jump-start")
		should_attack = true
		next_attack = 15 -- 15 = quarter second
		attack_timeout = 5
	end

	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

function decide()
	if (next_attack > 0 or get_hp(id) <= 0) then
		return "nil nostop"
	else
		local r = rand(2)
		if (r == 0) then
			set_battle_entity_speed_multiplier(id, 0.5)
		else
			set_battle_entity_speed_multiplier(id, 1.0)
		end
		return "seek within 0 20 A_PLAYER nostop rest 0.25 nostop"
	end
end

function get_should_auto_attack()
	return false
end

function stop()
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

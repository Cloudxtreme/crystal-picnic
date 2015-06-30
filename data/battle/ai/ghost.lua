local player
local prev_positions = {}

local TRAIL_LEN = 20

function start(this_id)
	id = this_id
	local x = 10 + rand(get_battle_width()-20)
	local y = 10 + rand(get_battle_height()-20)
	set_entity_position(id, x, y)
	set_battle_entity_flying(id, true)
	local right = rand(2)
	if (right == 1) then
		set_entity_right(id, true)
	else
		set_entity_right(id, false)
	end
	set_hp(id, 10)
	set_battle_entity_attack(id, 1)
	set_enemy_aggressiveness(id, LOGIC_RATE * 20)

	player = ai_get(id, "A_PLAYER")

	for i=1,TRAIL_LEN do
		prev_positions[i] = { x, y }
	end
	
	set_battle_entity_speed_multiplier(id, 0.5)
end

function get_attack_sound()
	return ""
end

local turn_attack_off = false
local attack_off_count = 0
local laughing = false

function dist()
	local x, y = get_entity_position(id)
	local px, py = get_entity_position(player)

	local dx = x - px
	local dy = y - py

	local dist = math.sqrt(dx*dx + dy*dy)

	return dist
end

function decide()
	if (get_hp(player) <= 0) then
		player = ai_get(id, "A_PLAYER")
		return "rest 0.01 nostop"
	end

	if (dist() < 5) then
		local w = get_battle_width()
		local x = get_entity_position(id)
		local m = math.fmod(get_time(), 2)
		
		if ((x < 100 or x > (w-100)) and m < 1) then
			if (not laughing) then
				laughing = true
				set_entity_animation(id, "laugh")
			end
		else
			if (laughing) then
				laughing = false
				set_entity_animation(id, "battle-idle")
			end
			set_battle_entity_attacking(id, true)
			turn_attack_off = true
		end

		return "rest 0.01 nostop"
	else
		if (laughing) then
			laughing = false
			set_entity_animation(id, "battle-idle")
		end

		local x, y = get_entity_position(player)
		local xx, yy = get_entity_position(id)

		local dx = x - xx
		local dy = y - yy

		local a = math.atan2(dy, dx)

		local dest_x = xx + math.cos(a) * 10
		local dest_y = yy + math.sin(a) * 10

		return "direct_move " .. dest_x .. " " .. dest_y .. " nostop"
	end
end

function get_should_auto_attack()
	return false
end

function die()
	if (rand(5) == 0) then
		local coin = add_battle_enemy("coin1")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	elseif (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function stop()
end

function logic()
	local x, y = get_entity_position(id)
	for i=TRAIL_LEN,2,-1 do
		prev_positions[i] = prev_positions[i-1]
	end
	prev_positions[1] = { x, y }

	if (turn_attack_off) then
		attack_off_count = attack_off_count + 1
		if (attack_off_count >= 2) then
			turn_attack_off = false
			attack_off_count = 0
			set_battle_entity_attacking(id, false)
		end
	end
end

function pre_draw()
	local top_x, top_y = get_battle_top()

	local bmp = get_entity_bitmap(id)
	local w, h = get_bitmap_size(bmp)

	local flags
	if (get_entity_right(id)) then
		flags = 0
	else
		flags = ALLEGRO_FLIP_HORIZONTAL
	end

	for i=TRAIL_LEN,1,-1 do
		draw_tinted_rotated_bitmap(
			bmp,
			(TRAIL_LEN-i)/TRAIL_LEN,
			(TRAIL_LEN-i)/TRAIL_LEN,
			(TRAIL_LEN-i)/TRAIL_LEN,
			(TRAIL_LEN-i)/TRAIL_LEN,
			w/2,
			h/2,
			prev_positions[i][1]-top_x,
			prev_positions[i][2]-h/2-top_y+BOTTOM_SPRITE_PADDING,
			0,
			flags
		)
	end
end

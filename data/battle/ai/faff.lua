local colliding_with_faff_data = false
local following_faff_data = false
local faff_dx, faff_dy, faff_direction
local legs = {}
local ZONE = 5

local counted_faffs = false
local multiple_faffs
local destroy_quiet = true

function start(this_id)
	id = this_id

	local x, y = get_random_start_platform(5, 32)
	set_entity_position(id, x, y)
	set_battle_entity_flying(id, true)
	local right = rand(2)
	if (right == 1) then
		set_entity_right(id, true)
	else
		set_entity_right(id, false)
	end
	set_battle_entity_speed_multiplier(id, 0.75)
	set_hp(id, 20)
	set_battle_entity_attack(id, 2)
	set_enemy_aggressiveness(id, LOGIC_RATE * 20)

	for i=1,6 do
		legs[i] = load_bitmap("battle/misc_graphics/faff_legs/" .. i .. ".png")
	end

	load_sample("sfx/faff.ogg", true)
	load_sample("sfx/faff_quiet.ogg", true)
end

function get_attack_sound()
	return ""
end

function decide()
	if (not counted_faffs) then
		multiple_faffs = count_battle_entities("faff") > 1
		if (multiple_faffs) then
			play_sample("sfx/faff_quiet.ogg", 1, 0, 1)
		else
			play_sample("sfx/faff.ogg", 1, 0, 1)
		end
	end

	if (multiple_faffs) then
		if (count_battle_entities("faff") <= 1) then
			destroy_sample("sfx/faff_quiet.ogg")
			destroy_quiet = false
			multiple_faffs = false
			play_sample("sfx/faff.ogg", 1, 0, 1)
		end
	end

	if (not following_faff_data) then
		local x, y = get_entity_position(id)
		if (x < 20) then
			set_entity_position(id, 20, y)
			set_entity_right(id, true)
		elseif (x > get_battle_width()-20) then
			set_entity_position(id, get_battle_width()-20, y)
			set_entity_right(id, false)
		else
			-- get dist to nearest faff data
			local size = get_battle_data_size("faff")
			local nelem = size / 6
			local closest = -1
			local closest_dist = 1000000
			local closest_x, closest_y
			for i=1,nelem do
				local x = get_battle_data("faff", (i-1)*6+0)
				local y = get_battle_data("faff", (i-1)*6+1)
				local faff_x, faff_y = get_entity_position(id)
				local dx = x - faff_x
				local dy = y - faff_y
				local dist = math.sqrt(dx*dx + dy*dy)
				if (dist < closest_dist) then
					closest = i
					closest_dist = dist
					closest_x = x
					closest_y = y
				end
			end
			if (closest >= 0) then
				if (not colliding_with_faff_data and closest_dist <= ZONE) then
					colliding_with_faff_data = true
					if (get_battle_data("faff", (closest-1)*6+4) == 1 or rand(2) == 0) then
						following_faff_data = true
						faff_dx = get_battle_data("faff", (closest-1)*6+2)
						faff_dy = get_battle_data("faff", (closest-1)*6+3)
						faff_direction = get_battle_data("faff", (closest-1)*6+5)
						set_entity_position(id, closest_x, closest_y)
					end
				elseif (colliding_with_faff_data and closest_dist > ZONE) then
					colliding_with_faff_data = false
				end
			end
		end
		if (not following_faff_data) then
			local x, y = get_entity_position(id)
			if (get_entity_right(id)) then
				x = x + 2
			else
				x = x - 2
			end
			return "direct_move " .. x .. " -1 nostop"
		end
	else
		local x, y = get_entity_position(id)
		local dx = x - faff_dx
		local dy = y - faff_dy
		local dist = math.sqrt(dx*dx + dy*dy)
		if (dist <= ZONE) then
			following_faff_data = false
			set_entity_position(id, faff_dx, faff_dy)
			if (faff_direction == 0) then
				set_entity_right(id, false)
			elseif (faff_direction == 1) then
				set_entity_right(id, true)
			end
		else
			if (y < faff_dy) then
				return "direct_move -1 " .. (y+2) .. " nostop"
			else
				return "direct_move -1 " .. (y-2) .. " nostop"
			end
		end
	end
	return "rest 0.01 nostop"
end

function get_should_auto_attack()
	return true
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
	for i=1,6 do
		destroy_bitmap("battle/misc_graphics/faff_legs/" .. i .. ".png")
	end

	destroy_sample("sfx/faff.ogg")
	if (destroy_quiet) then
		destroy_sample("sfx/faff_quiet.ogg")
	end
end

local frame = 1
local count = 0

function logic()
	count = count + 1
	if (count >= 2) then
		count = 0
		frame = frame + 1
		if (frame > 6) then
			frame = 1
		end
	end
end

function pre_draw()
	if (legs == nil) then
		return
	end

	local x, y = get_entity_position(id)
	local w, h = get_bitmap_size(legs[1])
	local flip
	if (get_entity_right(id)) then
		flip = 0
	else
		flip = ALLEGRO_FLIP_HORIZONTAL
	end
	local top_x, top_y = get_battle_top()
	draw_bitmap(legs[frame], x-w/2-top_x, y-h-top_y+BOTTOM_SPRITE_PADDING, flip)
end

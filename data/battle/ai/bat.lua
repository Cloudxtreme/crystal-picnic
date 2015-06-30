function get_stalactite_point()
	local num_stalactites = get_battle_data_size("stalactite") / 6
	local num = rand(num_stalactites)
	local tip_x = get_battle_data("stalactite", num*6)
	local tip_y = get_battle_data("stalactite", num*6+1)
	local side = rand(2)
	local end_x = get_battle_data("stalactite", num*6+2+side*2)
	local end_y = get_battle_data("stalactite", num*6+2+side*2+1)
	local x = (tip_x - end_x) * 0.75 + end_x
	local y = (tip_y - end_y) * 0.75 + end_y
	return x, y
end

function set_dir(move_x, move_y)
       if (move_x == 0) then
	       if (move_y < 0) then
		       set_entity_animation(id, "fly-up")
	       else
		       set_entity_animation(id, "fly-down")
	       end
       elseif (move_y == 0) then
	       set_entity_animation(id, "fly")
	       if (move_x < 0) then
		       set_entity_right(id, false)
	       else
		       set_entity_right(id, true)
	       end
       else
	       if (move_x < 0) then
		       set_entity_right(id, false)
	       else
		       set_entity_right(id, true)
	       end
	       if (move_x > move_y) then
		       set_entity_animation(id, "fly")
	       else
		       if (move_y < 0) then
			       set_entity_animation(id, "fly-up")
		       else
			       set_entity_animation(id, "fly-down")
		       end
	       end
       end
end

local STEP = 25

local SITTING = 0
local GOING_IN = 1
local GOING_OUT = 2

local stage
local sit_time
local sit_start

local played_sample = false

local attacking = 0

local old_player_x
local old_player_y

function next_sit()
	stage = SITTING
	sit_time = (rand(1000)/1000) * 2.5 + 2.5
	sit_start = get_time()
	set_entity_animation(id, "battle-idle")
	played_sample = false
end

function start(this_id)
	id = this_id
	local x, y = get_stalactite_point()
	set_entity_position(id, x, y)
	set_battle_entity_flying(id, true)
	set_battle_entity_layer(id, 1)
	local right = rand(2)
	if (right == 1) then
		set_entity_right(id, true)
	else
		set_entity_right(id, false)
	end
	set_hp(id, 1)
	set_battle_entity_attack(id, 5)

	load_sample("sfx/flap.ogg", false)
	load_sample("sfx/magic_drain.ogg", false)

	next_sit()
end

function get_attack_sound()
	return ""
end

function decide()
	if (stage == SITTING) then
		local now = get_time()
		if (now >= sit_start+sit_time) then
			stage = GOING_IN
			attacking_player = ai_get(id, "A_PLAYER")
			old_player_x = -1
			old_player_y = -1
			play_sample("sfx/flap.ogg", 1, 0, 1)
		end
	elseif (stage == GOING_IN) then
		local x, y = get_entity_position(id)
		local px, py = get_entity_position(attacking_player)
		py = py - 5

		local player_pos_changed

		if (not (px == old_player_x) or not (py == old_player_y)) then
			player_pos_changed = true
		else
			player_pos_changed = false
		end

		old_player_x = px
		old_player_y = py

		local dx = px - x
		local dy = py - y
		local dist = math.sqrt(dx*dx + dy*dy)

		local burrowing = is_burrowing(attacking_player)

		if (dist < 3 or burrowing) then
			stage = GOING_OUT
			going_to_x, going_to_y = get_stalactite_point()
			set_dir(going_to_x - x, going_to_y - y)
			play_sample("sfx/flap.ogg", 1, 0, 1)
			set_battle_entity_attacking(id, true)
			attacking = 2
		else
			local angle = math.atan2(dy, dx)

			local s
			if (STEP > dist) then
				s = dist
			else
				s = STEP
			end

			local old_x = x
			local old_y = y

			x = x + math.cos(angle) * s
			y = y + math.sin(angle) * s

			set_dir(x - old_x, y - old_y)

			return "direct_move " .. x .. " " .. y .. " nostop"
		end
	else
		local x, y = get_entity_position(id)
		local dx = going_to_x - x
		local dy = going_to_y - y
		local dist = math.sqrt(dx*dx + dy*dy)
		if (dist < 3) then
			set_entity_position(id, going_to_x, going_to_y)

			next_sit()
		else
			local angle = math.atan2(dy, dx)

			local s
			if (STEP > dist) then
				s = dist
			else
				s = STEP
			end

			x = x + math.cos(angle) * s
			y = y + math.sin(angle) * s

			return "direct_move " .. x .. " " .. y .. " nostop"
		end
	end
	
	return "rest 0.01 nostop"
end

function get_should_auto_attack()
	return false
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin1")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function stop()
	destroy_sample("sfx/flap.ogg")
	destroy_sample("sfx/magic_drain.ogg")
end

function logic()
	if (attacking > 0) then
		attacking = attacking - 1
		if (attacking == 0) then
			set_battle_entity_attacking(id, false)
		end
	end
end

function collide(with, me)
	if (not played_sample) then
		played_sample = true
		play_sample("sfx/magic_drain.ogg", 1, 0, 1)
		drain_magic(with, 5)
	end
end

local MAIN_MINIGAME_MILESTONE = "whack_a_skunk_sock"

local water_wheel_x = 1168
local water_wheel_y = 664

local flea_milestone_name = "flea_intro"
local river_town_intro = "river_town_intro"
local spoke = false

function done_intro(tween, id, elapsed)
	set_character_role(1, "follow", 0)
	set_entity_input_disabled(0, false)
	set_milestone_complete(river_town_intro, true)
	return true
end

function get_vol()
	local px, py = get_entity_position(0)
	local dx = px - water_wheel_x
	local dy = py - water_wheel_y
	local dist = math.sqrt(dx*dx + dy*dy)
	if (dist >= 400) then
		return 0
	else
		return (400 - dist) / 400
	end
end

function adjust_water_wheel_sound()
	adjust_sample("sfx/water_wheel.ogg", get_vol(), 0.0, 1.0)
end

function change_r3_male_direction(tween, id, elapsed)
	local n = rand(4)
	if (n == 0) then
		set_entity_direction(r3_male, DIR_S)
	elseif (n == 1) then
		set_entity_direction(r3_male, DIR_N)
	elseif (n == 2) then
		set_entity_direction(r3_male, DIR_E)
	else
		set_entity_direction(r3_male, DIR_W)
	end
	return true
end

function start()
	play_music("music/river_town.mid")

	process_outline()

	if (not milestone_is_complete(river_town_intro)) then
		set_entity_input_disabled(0, true)
		local px, py = get_entity_position(0)
		set_entity_position(1, px-32, py)
		set_entity_direction(0, DIR_W)
		set_entity_direction(1, DIR_E)
	end

	-- Mario bushes
	add_wall_group(2, 25, 16, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 28, 15, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 33, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 37, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 56, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 25, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 55, 11, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 68, 26, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 68, 29, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 18, 2, 2, TILE_GROUP_BUSHES)

	-- SOM bushes
	add_wall_group(2, 11, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 14, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 19, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 17, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 17, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 29, 59, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 31, 2, 2, TILE_GROUP_BUSHES)

	-- Trees (4x6 tiles)
	add_wall_group(2, 9, 1, 4, 6, 0)
	add_wall_group(2, 13, 2, 4, 6, 0)
	add_wall_group(2, 31, 25, 4, 6, 0)
	add_wall_group(2, 21, 46, 4, 6, 0)
	add_wall_group(2, 69, 44, 4, 6, 0)
	add_wall_group(2, 10, 39, 4, 6, 0)

	-- Trees (3x6)
	add_wall_group(2, 77, 32, 3, 6, 0)

	-- Flea market
	add_wall_group(2, 21, 25, 9, 7, 0)

	-- Cafe posts
	add_wall_group(2, 42, 10, 2, 4, 0)
	add_wall_group(2, 46, 10, 2, 4, 0)

	-- Cafe door
	cafe_door = Door:new{entity_name="river_town_ext_door2", layer=2, x=45*TILE_SIZE, y=11*TILE_SIZE, sound=true}

	-- Other doors
	frogbert_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=17*TILE_SIZE+TILE_SIZE/2, y=17*TILE_SIZE, sound=true}
	egbert_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=28*TILE_SIZE+TILE_SIZE/2, y=52*TILE_SIZE, sound=true}
	mill_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=66*TILE_SIZE+TILE_SIZE/2, y=42*TILE_SIZE, sound=true}
	r1_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=71*TILE_SIZE+TILE_SIZE/2, y=13*TILE_SIZE, sound=true}
	r2_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=60*TILE_SIZE+TILE_SIZE/2, y=30*TILE_SIZE, sound=true}
	r3_door = Door:new{entity_name="river_town_ext_door1", layer=2, x=50*TILE_SIZE+TILE_SIZE/2, y=62*TILE_SIZE, sound=true}

	-- Portals
	frogbert_entrance = add_polygon_entity(
		2,
		256, 267,
		303, 267,
		303, 277,
		256, 277
	)
	egbert_entrance = add_polygon_entity(
		2,
		432, 827,
		479, 827,
		479, 837,
		432, 837
	)
	inn_entrance = add_polygon_entity(
		2,
		704, 173,
		735, 173,
		735, 183,
		704, 183
	)
	r1_entrance = add_polygon_entity(
		2,
		1120, 203,
		1167, 203,
		1167, 213,
		1120, 213
	)
	r2_entrance = add_polygon_entity(
		2,
		944, 475,
		991, 475,
		991, 485,
		944, 485
	)
	r3_entrance = add_polygon_entity(
		2,
		784, 987,
		831, 987,
		831, 997,
		784, 997
	)
	mill_entrance = add_polygon_entity(
		2,
		1041, 667,
		1087, 667,
		1087, 677,
		1041, 677
	)

	east_exit = Active_Block:new{x=79, y=13, width=1, height=7}
	r3_from_roof = Active_Block:new{layer=4, x=49, y=55, width=1, height=2}
	south_exit = Active_Block:new{x=38, y=73, width=7, height=1}

	add_entity("water_wheel", 2, water_wheel_x, water_wheel_y)
	add_entity("water_wheel_trail", 1, water_wheel_x, water_wheel_y+45)

	turtle1 = add_npc("turtle", 2, 1127, 712)
	set_character_role(turtle1, "wander", 96, 1, 4)

	if (done_oldoak()) then
		r3_male = add_npc("cat", 2, 42.5*TILE_SIZE, 65.5*TILE_SIZE)
		set_character_role(r3_male, "wander", 192, 2, 4)
	else
		r3_male = add_entity("cat", 4, 774, 941)
		local t = {}
		t.run = change_r3_male_direction
		t.started = false
		t.next_tween = create_idle_tween(5)
		t.next_tween.next_tween = t
		new_tween(t)
	end

	npc1 = add_npc("pig_female", 2, 151, 200)
	set_character_role(npc1, "wander", 192, 2, 4)
	npc2 = add_npc("squirrel", 2, 399, 568)
	set_character_role(npc2, "wander", 64, 3, 5)
	npc3 = add_npc("squirrel_female", 2, 712, 728)
	set_character_role(npc3, "wander", 256, 1, 3)
	npc4 = add_entity("squirrel", 2, 280, 984)
	set_entity_direction(npc4, DIR_E)
	npc5 = add_entity("turtle", 2, 1159, 1006)
	set_entity_direction(npc5, DIR_N)

	flea_minigames = add_entity("flea_minigames", 2, 365, 471)
	set_entity_direction(flea_minigames, DIR_S)
	flea_items = add_entity("flea_items", 2, 400, 471)
	set_entity_direction(flea_items, DIR_S)
	flea_equipment = add_entity("flea_equipment", 2, 435, 471)
	set_entity_direction(flea_equipment, DIR_S)

	dog = add_npc("weiner_dog", 2, 584, 393)
	set_character_role(dog, "wander", 256, 0, 0)
	set_wander_minimum_move_distance(dog, 64)

	load_sample("sfx/water_wheel.ogg", true)
	play_sample("sfx/water_wheel.ogg", get_vol(), 0.0, 1.0)
	
	load_sample("sfx/flea_minigames.ogg", false)
	load_sample("sfx/flea_items.ogg", false)
	load_sample("sfx/flea_equipment.ogg", false)
end

local count = 0
local speak_count = 0

function logic()
	if (whack_a_skunk_was_played()) then
		local score = get_whack_a_skunk_score()
		set_whack_a_skunk_played(false)
		if (score >= 60) then
			if (not milestone_is_complete(MAIN_MINIGAME_MILESTONE)) then
				achieve("whack");
				speak(false, false, true, t("WON_DIRTYSOCK"), t("FLOYD"), flea_minigames, flea_minigames)
				play_sample("sfx/item_found.ogg", 1, 0, 1)
				give_items("DIRTYSOCK", 1)
				set_milestone_complete(MAIN_MINIGAME_MILESTONE, true)
				speak(
					true,
					false,
					true,
					string.format(t("FOUND_ONE"), t("DIRTYSOCK")),
					"",
					0
				)
			else
				speak(false, false, true, t("WON_HEALTHVIAL"), t("FLOYD"), flea_minigames, flea_minigames)
				play_sample("sfx/item_found.ogg", 1, 0, 1)
				give_items("HEALTHVIAL", 1)
				speak(
					true,
					false,
					true,
					string.format(t("FOUND_ONE"), t("HEALTHVIAL")),
					"",
					0
				)
			end
		else
			speak(false, false, true, t("WON_NOTHING"), t("FLOYD"), flea_minigames, flea_minigames)
		end
	end

	if (not milestone_is_complete(river_town_intro)) then
		speak_count = speak_count + 1
		if (not spoke and speak_count >= 50) then
			spoke = true

			local egbert = 0
			local frogbert = 1

			simple_speak{
				true,
				"SCN4_EGBERT_1", "", egbert,
				"SCN4_FROGBERT_1", "", frogbert,
				"SCN4_EGBERT_2", "gesture-anim", egbert,
				"SCN4_FROGBERT_2", "", frogbert,
				"SCN4_FROGBERT_3", "point", frogbert
			}

			set_character_role(1, "astar")
			
			local px, py = get_entity_position(0)
			local t = create_astar_tween(1, px, py)
			append_tween(t, {run=done_intro, started=false})
			new_tween(t)
		end
		return
	end

	count = count + 1
	if (count == 15) then
		count = 0
		adjust_water_wheel_sound()
	end

	if (east_exit:entity_is_colliding(0)) then
		start_map("RIVER_TOWN")
	elseif (r3_from_roof:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town_r3", DIR_S, 95, 71)
	elseif (south_exit:entity_is_colliding(0)) then
		set_milestone_complete("exited_river_town_south", true);
		start_map("RIVER_TOWN")
	end

	if (speak_1 == true) then
		speak_1 = false
		set_entity_animation(flea_items, "idle-down")
		speak(false, false, true, t("FLEA_INTRO_1"), t("FLYNN"), flea_items, flea_items)
		local t = create_gesture_tween(flea_equipment, "gesture")
		append_tween(t, create_idle_tween(0.7));
		append_tween(t, {run = flea_2, started = false});
		new_tween(t)
	elseif (speak_2 == true) then
		speak_2 = false
		set_entity_animation(flea_equipment, "idle-down")
		speak(false, false, true, t("FLEA_INTRO_2"), t("FABIAN"), flea_equipment, flea_equipment)
		local t = create_gesture_tween(flea_minigames, "gesture")
		append_tween(t, create_idle_tween(0.9));
		append_tween(t, {run = flea_3, started = false});
		new_tween(t)
	elseif (speak_3 == true) then
		speak_3 = false
		set_entity_animation(flea_minigames, "idle-down")
		speak(false, false, true, t("FLEA_INTRO_3"), t("FLOYD"), flea_minigames, flea_minigames)
		set_milestone_complete(flea_milestone_name, true)
		set_entity_input_disabled(0, false)
	end
end

function flea_1(tween, id, elapsed)
	speak_1 = true
	return true
end

function flea_2(tween, id, elapsed)
	speak_2 = true
	return true
end

function flea_3(tween, id, elapsed)
	speak_3 = true
	return true
end

function enable_input(tween, id, elapsed)
	set_entity_input_disabled(0, false)
	return true
end

function activate(activator, activated)
	if (activated == turtle1) then
		if (done_amaysa1()) then
			speak(false, false, true, t("MILL_TURTLE2_2"), "", turtle1, turtle1)
		else
			speak(false, false, true, t("MILL_TURTLE2_1"), "", turtle1, turtle1)
		end
	elseif (activated == r3_male) then
		if (done_oldoak()) then
			speak(false, false, true, t("RIVER_TOWN_R3_MALE_3"), "", r3_male, r3_male)
		elseif (done_pyou1()) then
			speak(false, false, true, t("RIVER_TOWN_R3_MALE_2"), "", r3_male, r3_male)
		else
			speak(false, false, true, t("RIVER_TOWN_R3_MALE_1"), "", r3_male, r3_male)
		end
	elseif (activated == npc1) then
		if (done_oldoak()) then
			speak(false, false, true, t("RIVER_TOWN_NPC1_3"), "", npc1, npc1)
		elseif (done_pyou1()) then
			speak(false, false, true, t("RIVER_TOWN_NPC1_2"), "", npc1, npc1)
		else
			speak(false, false, true, t("RIVER_TOWN_NPC1_1"), "", npc1, npc1)
		end
	elseif (activated == npc2) then
		if (done_oldoak()) then
			speak(false, false, true, t("RIVER_TOWN_NPC2_3"), "", npc2, npc2)
		elseif (done_pyou1()) then
			speak(false, false, true, t("RIVER_TOWN_NPC2_2"), "", npc2, npc2)
		else
			speak(false, false, true, t("RIVER_TOWN_NPC2_1"), "", npc2, npc2)
		end
	elseif (activated == npc3) then
		if (done_oldoak()) then
			speak(false, false, true, t("RIVER_TOWN_NPC3_3"), "", npc3, npc3)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("RIVER_TOWN_NPC3_2"), "", npc3, npc3)
		else
			speak(false, false, true, t("RIVER_TOWN_NPC3_1"), "", npc3, npc3)
		end
	elseif (activated == npc4) then
		speak(false, false, true, t("RIVER_TOWN_NPC4_1"), "", npc4, npc4)
	elseif (activated == npc5) then
		if (done_amaysa1()) then
			speak(false, false, true, t("RIVER_TOWN_NPC5_2"), "", npc5, npc5)
		else
			speak(false, false, true, t("RIVER_TOWN_NPC5_1"), "", npc5, npc5)
		end
	elseif (activated == flea_minigames or activated == flea_equipment or activated == flea_items) then
		if (milestone_is_complete(flea_milestone_name)) then
			if (activated == flea_minigames) then
				play_sample("sfx/flea_minigames.ogg", 1, 0, 1)
				set_entity_input_disabled(0, true)
				speak(false, false, true, t("FLEA_RIVER_TOWN_2"), "", flea_minigames, flea_minigames)
				local tbl = create_idle_tween(0.5)
				append_tween(tbl, { run = enable_input })
				new_tween(tbl)
				do_whack_a_skunk()
			elseif (activated == flea_equipment) then
				play_sample("sfx/flea_equipment.ogg", 1, 0, 1)
				set_entity_input_disabled(0, true)
				speak(false, false, true, t("FLEA_RIVER_TOWN_1"), "", flea_equipment, flea_equipment)
				local tbl = create_idle_tween(0.5)
				append_tween(tbl, { run = enable_input })
				new_tween(tbl)
				do_equipment_shop(
					"BAT", 50,
					"SHOVEL", 50
				)
			else
				play_sample("sfx/flea_items.ogg", 1, 0, 1)
				set_entity_input_disabled(0, true)
				speak(false, false, true, t("FLEA_RIVER_TOWN_1"), "", flea_items, flea_items)
				local tbl = create_idle_tween(0.5)
				append_tween(tbl, { run = enable_input })
				new_tween(tbl)
				do_item_shop(
					"ANTIDOTE", 10,
					"HEALTHVIAL", 5,
					"HEALTHJAR", 20,
					"MAGICVIAL", 15,
					"MAGICJAR", 50,
					"DIRTYSOCK", 75
				)
			end
		else
			stop_entity(get_player_id(0))
			set_entity_input_disabled(get_player_id(0), true)
			local t = create_gesture_tween(flea_items, "gesture")
			append_tween(t, create_idle_tween(0.7));
			append_tween(t, {run = flea_1, started = false});
			new_tween(t)
		end
	end
end

function collide(id1, id2)
	if (colliding(id1, id2, 0, cafe_door.id)) then
		cafe_door:collide()
	elseif (colliding(id1, id2, 0, frogbert_door.id)) then
		frogbert_door:collide()
	elseif (colliding(id1, id2, 0, egbert_door.id)) then
		egbert_door:collide()
	elseif (colliding(id1, id2, 0, mill_door.id)) then
		mill_door:collide()
	elseif (colliding(id1, id2, 0, r1_door.id)) then
		r1_door:collide()
	elseif (colliding(id1, id2, 0, r2_door.id)) then
		r2_door:collide()
	elseif (colliding(id1, id2, 0, r3_door.id)) then
		r3_door:collide()
	elseif (colliding(id1, id2, 0, frogbert_entrance)) then
		next_player_layer = 2
		change_areas("river_town_frogbert", DIR_N, 160, 136)
	elseif (colliding(id1, id2, 0, egbert_entrance)) then
		next_player_layer = 2
		change_areas("river_town_egbert", DIR_N, 48, 197)
	elseif (colliding(id1, id2, 0, inn_entrance)) then
		next_player_layer = 3
		change_areas("river_town_inn_lower", DIR_N, 64, 183)
	elseif (colliding(id1, id2, 0, r1_entrance)) then
		next_player_layer = 2
		change_areas("river_town_r1", DIR_N, 176, 151)
	elseif (colliding(id1, id2, 0, r2_entrance)) then
		next_player_layer = 2
		change_areas("river_town_r2", DIR_N, 64, 135)
	elseif (colliding(id1, id2, 0, r3_entrance)) then
		next_player_layer = 2
		change_areas("river_town_r3", DIR_N, 144, 166)
	elseif (colliding(id1, id2, 0, mill_entrance)) then
		next_player_layer = 2
		change_areas("river_town_mill", DIR_N, 80, 186)
	end
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
end

function stop()
	destroy_sample("sfx/water_wheel.ogg")
	destroy_sample("sfx/flea_minigames.ogg")
	destroy_sample("sfx/flea_items.ogg")
	destroy_sample("sfx/flea_equipment.ogg")
end

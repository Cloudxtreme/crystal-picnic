is_dungeon = true

MILESTONE_NAME = "pyou_intro"
CRYSTAL1_MILESTONE_NAME = "pyou_crystal"
CRYSTAL2_MILESTONE_NAME = "bisou_crystal"
LEFT_AREA = "left_pyou_area"

function start(game_just_loaded)
	play_music("music/abw2.mid");

	load_sample("sfx/blush.ogg", false)
	load_sample("sfx/crystal_glow.ogg", true)
	glow_loaded = true
	load_sample("sfx/spend_crystal.ogg", false)

	process_outline()

	if (milestone_is_complete(MILESTONE_NAME)) then
		if (not milestone_is_complete(CRYSTAL1_MILESTONE_NAME)) then
			crystal1 = add_entity("crystal", 2, 1496, 318)
		end
		if (not milestone_is_complete(CRYSTAL2_MILESTONE_NAME)) then
			crystal2 = add_entity("crystal", 2, 1432, 296)
		end
		if (not milestone_is_complete(LEFT_AREA)) then
			pyou = add_npc("pyou", 2, 93*TILE_SIZE+TILE_SIZE/2+32, 19*TILE_SIZE+TILE_SIZE/2-10)
		end
	end
	
	ring = load_bitmap("misc_graphics/crystal_ring.png")

	-- apple trees
	add_wall_group(2, 57, 23, 5, 7, 0)
	add_wall_group(2, 68, 50, 5, 7, 0)
	add_wall_group(2, 106, 51, 5, 7, 0)
	add_wall_group(2, 104, 26, 5, 7, 0)
	-- smaller trees
	add_wall_group(2, 73, 73, 4, 6, 0)
	add_wall_group(2, 97, 21, 4, 6, 0)
	add_wall_group(2, 71, 57, 4, 6, 0)
	add_wall_group(2, 61, 38, 4, 6, 0)
	add_wall_group(2, 57, 33, 4, 6, 0)
	add_wall_group(2, 51, 19, 4, 6, 0)
	add_wall_group(2, 40, 24, 4, 6, 0)
	add_wall_group(2, 30, 28, 4, 6, 0)
	add_wall_group(2, 18, 30, 4, 6, 0)
	add_wall_group(2, 14, 34, 4, 6, 0)
	add_wall_group(2, 23, 43, 4, 6, 0)
	add_wall_group(2, 23, 43, 4, 6, 0)
	add_wall_group(2, 30, 47, 4, 6, 0)
	add_wall_group(2, 35, 49, 4, 6, 0)
	add_wall_group(2, 26, 24, 4, 4, 0) -- part
	-- som bushes
	add_wall_group(2, 1, 25, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 4, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 9, 25, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 10, 35, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 26, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 21, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 35, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 29, 21, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 33, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 37, 29, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 40, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 42, 32, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 22, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 49, 25, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 56, 20, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 26, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 62, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 55, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 63, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 63, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 39, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 42, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 49, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 42, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 48, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 45, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 62, 45, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 59, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 53, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 59, 63, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 62, 61, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 64, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 70, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 69, 72, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 74, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 70, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 73, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 85, 78, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 71, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 73, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 94, 63, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 104, 72, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 71, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 109, 61, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 86, 53, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 88, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 86, 48, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 46, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 89, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 49, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 108, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 92, 29, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 23, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 76, 13, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 78, 15, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 83, 17, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 75, 9, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 76, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 72, 5, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 75, 5, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 78, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 1, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 0, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 1, 2, 2, TILE_GROUP_BUSHES)
	-- mario bushes
	add_wall_group(2, 29, 45, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 5, 25, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 11, 25, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 31, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 18, 41, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 23, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 33, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 25, 21, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 32, 21, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 34 ,21, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 48, 19, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 47, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 50, 41, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 38, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 50, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 47, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 53, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 60, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 65, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 76, 62, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 78, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 71, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 77, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 93, 80, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 89, 67, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 91, 63, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 64, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 92, 58, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 91, 49, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 100, 47, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 109, 36, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 111, 33, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 109, 26, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 96, 28, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 2, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 84, 0, 2, 2, TILE_GROUP_BUSHES)
	-- bridge stuff
	add_wall_group(2, 96, 70, 2, 1, 0)
	add_wall_group(2, 102, 70, 2, 1, 0)
	add_wall_group(2, 96, 36, 2, 1, 0)
	add_wall_group(2, 102, 36, 2, 1, 0)
	add_wall_group(2, 96, 37, 8, 2, 0)
	add_wall_group(2, 96, 71, 8, 2, 0)
	-- porch railings
	add_wall_group(2, 90, 13, 4, 2, 0)
	add_wall_group(2, 96, 13, 2, 2, 0)

	add_no_enemy_zone(68, 0, 114, 60)
	
	pyou_door = Door:new{entity_name="pyou_door", layer=2, x=92*TILE_SIZE+TILE_SIZE/2, y=13*TILE_SIZE, sound=true}
	pyou_entrance = add_polygon_entity(2, 92*TILE_SIZE, 12*TILE_SIZE, 93*TILE_SIZE, 12*TILE_SIZE, 93*TILE_SIZE, 13*TILE_SIZE-6, 92*TILE_SIZE, 13*TILE_SIZE-6)

	if (not game_just_loaded) then
		add_enemies(2, { "ant", "wolf" }, 8, 3, 5, "abw", { "ant", "bazooka_ant", "tough_ant", "bird", "wolf" })
		choppable_array = {}
		choppable_array[1] = { name="bush", layer=2, x=1*TILE_SIZE+8, y=27*TILE_SIZE+8, dead=false }
		choppable_array[2] = { name="bush", layer=2, x=7*TILE_SIZE+8, y=28*TILE_SIZE+8, dead=false }
		choppable_array[3] = { name="bush", layer=2, x=31*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[4] = { name="bush", layer=2, x=48*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[5] = { name="bush", layer=2, x=56*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[6] = { name="bush", layer=2, x=63*TILE_SIZE+8, y=25*TILE_SIZE+8, dead=false }
		choppable_array[7] = { name="bush", layer=2, x=63*TILE_SIZE+8, y=35*TILE_SIZE+8, dead=false }
		choppable_array[8] = { name="bush", layer=2, x=67*TILE_SIZE+8, y=50*TILE_SIZE+8, dead=false }
		choppable_array[9] = { name="bush", layer=2, x=67*TILE_SIZE+8, y=52*TILE_SIZE+8, dead=false }
		choppable_array[10] = { name="bush", layer=2, x=62*TILE_SIZE+8, y=55*TILE_SIZE+8, dead=false }
		choppable_array[11] = { name="bush", layer=2, x=64*TILE_SIZE+8, y=64*TILE_SIZE+8, dead=false }
		choppable_array[12] = { name="bush", layer=2, x=86*TILE_SIZE+8, y=75*TILE_SIZE+8, dead=false }
		choppable_array[13] = { name="bush", layer=2, x=96*TILE_SIZE+8, y=68*TILE_SIZE+8, dead=false }
		choppable_array[14] = { name="bush", layer=2, x=103*TILE_SIZE+8, y=65*TILE_SIZE+8, dead=false }
		choppable_array[15] = { name="bush", layer=2, x=96*TILE_SIZE+8, y=57*TILE_SIZE+8, dead=false }
		choppable_array[16] = { name="bush", layer=2, x=95*TILE_SIZE+8, y=55*TILE_SIZE+8, dead=false }
		choppable_array[17] = { name="bush", layer=2, x=104*TILE_SIZE+8, y=45*TILE_SIZE+8, dead=false }
		choppable_array[18] = { name="bush", layer=2, x=90*TILE_SIZE+8, y=16*TILE_SIZE+8, dead=false }
		choppable_array[19] = { name="bush", layer=2, x=91*TILE_SIZE+8, y=16*TILE_SIZE+8, dead=false }
		choppable_array[20] = { name="bush", layer=2, x=96*TILE_SIZE+8, y=16*TILE_SIZE+8, dead=false }
		choppable_array[21] = { name="bush", layer=2, x=97*TILE_SIZE+8, y=16*TILE_SIZE+8, dead=false }
	end
	spawn_choppable(choppable_array)

	to_meeting = Active_Block:new{x=0, y=23, width=1, height=16}

	scene_start_block = Active_Block:new{x=100, y=42, width=11, height=1}

	to_pyou2 = Active_Block:new{x=114, y=27, width=1, height=10}
end

function activate(activator, activated)
	if (not (crystal1 == nil) and activated == crystal1) then
		remove_entity(crystal1)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL1_MILESTONE_NAME, true)
	elseif (not (crystal2 == nil) and activated == crystal2) then
		remove_entity(crystal2)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL2_MILESTONE_NAME, true)
	elseif (not (pyou == nil) and activated == pyou) then
		simple_speak{
			true,
			"SCN6_PYOU_19", "greet", pyou
		}
	end
end

function do_start_speech(tween, id, elapsed)
	start_speech = true
	return true
end

function do_pyou_greet(tween, id, elapsed)
	pyou_greet = true
	return true
end

function do_egbert_interrupt(tween, id, elapsed)
	egbert_interrupt = true
	return true
end

function do_speak_to_egbert(tween, id, elapsed)
	speak_to_egbert = true
	return true
end

function do_cute(tween, id, elapsed)
	cute = true
	return true
end

function do_crystal_walk(tween, id, elapsed)
	crystal_walk = true
	return true
end

function do_bisou_talk(tween, id, elapsed)
	bisou_talk = true
	return true
end

function inc_counter(tween, id, elapsed)
	counter = counter + 1
	return true
end

function inc_counter2(tween, id, elapsed)
	counter2 = counter2 + 1
	return true
end

function logic()
	update_choppable()

	if (to_meeting:entity_is_colliding(get_player_id(0))) then
		if (milestone_is_complete(MILESTONE_NAME)) then
			set_milestone_complete(LEFT_AREA, "true")
		end
		next_player_layer = 2
		change_areas("meeting", DIR_W, 840, 560)
	elseif (to_pyou2:entity_is_colliding(get_player_id(0))) then
		set_milestone_complete(LEFT_AREA, "true")
		next_player_layer = 2
		change_areas("pyou2", DIR_E, 55, 120)
	end


	if (counter2 == 2 and not finished_scene) then
		finished_scene = true

		set_milestone_complete(MILESTONE_NAME, true)

		remove_entity(bisou)
		
		add_bisou()

		if (get_entity_name(get_player_id(0)) == "frogbert") then
			bisou = get_player_id(1)
			egbert = get_player_id(2)
			set_character_role(egbert, "follow", bisou)
		else
			frogbert = get_player_id(1)
			bisou = get_player_id(2)
			set_character_role(frogbert, "follow", get_player_id(0))
		end

		set_entity_animation(pyou, "idle-down")

		set_character_role(get_player_id(0), "")
		
		local t = create_center_view_tween()
		new_tween(t)

		set_entity_solid_with_area(get_player_id(0), true)
		set_entity_solid_with_entities(get_player_id(0), true)

		set_entity_direction(get_player_id(0), DIR_E)
	elseif (battle_started and not battle_ended) then
		battle_ended = true
		
		play_music("music/abw2.mid")

		remove_entity(ant1)
		remove_entity(ant2)

		set_entity_right(egbert, true)
		set_entity_right(frogbert, true)
		set_entity_right(pyou, false)
		set_entity_animation(pyou, "idle")
		set_entity_animation(bisou, "idle-down")

		simple_speak{
			true,
			"SCN6_EGBERT_14", "facepalm", egbert
		}

		speak_force_top(false, false, true, t("SCN6_FROGBERT_13"), "", frogbert)

		simple_speak{
			true,
			"SCN6_BISOU_3", "", bisou,
			"SCN6_PYOU_17", "", pyou,
			"SCN6_BISOU_4", "", bisou
		}

		speak_force_top(false, false, true, t("SCN6_FROGBERT_14"), "thinking", frogbert)

		simple_speak{
			true,
			"SCN6_BISOU_5", "", bisou
		}

		local x, y = get_entity_position(bisou)

		crystal2 = add_entity("crystal", 2, x, y+16)

		simple_speak{
			true,
			"SCN6_PYOU_18", "", pyou,
			"SCN6_BISOU_6", "", bisou,
			"SCN6_EGBERT_15", "", egbert
		}

		speak_force_top(false, false, true, t("SCN6_FROGBERT_15"), "", frogbert)

		simple_speak{
			true,
			"SCN6_EGBERT_16", "gesture-anim", egbert,
			"SCN6_BISOU_7", "", bisou,
			"SCN6_PYOU_19", "", pyou,
			"SCN6_EGBERT_17", "", egbert,
			"SCN6_PYOU_20", "", pyou
		}

		speak_force_top(false, false, true, t("SCN6_FROGBERT_16"), "blush", frogbert)

		counter2 = 0

		local main_x, main_y = get_entity_position(get_player_id(0))

		local t = create_astar_tween(get_player_id(1), main_x, main_y, false)
		append_tween(t, { run = inc_counter2 })
		new_tween(t)

		set_entity_solid_with_entities(bisou, false)

		t = create_astar_tween(bisou, main_x, main_y, false)
		append_tween(t, { run = inc_counter2 })
		new_tween(t)
	elseif (counter == 4 and not counter_done) then
		counter_done = true

		play_music("music/boss.mid")

		start_battle("abw1", "abw", true, "machete_ant", "machete_ant")

		battle_started = true
	elseif (bisou_talk and not bisou_talked) then
		bisou_talked = true
		
		simple_speak{
			true,
			"SCN6_BISOU_2", "scared", bisou
		}

		set_entity_animation_set_prefix(bisou, "")

		ant1 = add_npc("ant", 2, 89*TILE_SIZE, 10*TILE_SIZE)
		set_character_role(ant1, "astar")
		ant2 = add_npc("ant", 2, 83*TILE_SIZE+TILE_SIZE/2, 11*TILE_SIZE)
		set_character_role(ant2, "astar")

		set_entity_solid_with_entities(get_player_id(0), false)

		counter = 0

		local t = create_astar_tween(ant1, 89*TILE_SIZE+TILE_SIZE/2, 16*TILE_SIZE+TILE_SIZE/2, false)
		append_tween(t, { run = inc_counter })
		new_tween(t)

		t = create_astar_tween(ant2, 87*TILE_SIZE+TILE_SIZE/2, 16*TILE_SIZE+TILE_SIZE/2, false)
		append_tween(t, { run = inc_counter })
		new_tween(t)

		t = create_astar_tween(frogbert, 87*TILE_SIZE+TILE_SIZE/2, 17*TILE_SIZE+TILE_SIZE/2, false)
		append_tween(t, { run = inc_counter })
		new_tween(t)

		t = create_astar_tween(egbert, 88*TILE_SIZE+TILE_SIZE/2, 18*TILE_SIZE+TILE_SIZE/2+2, false)
		append_tween(t, { run = inc_counter })
		new_tween(t)
	elseif (wow and not wowed) then
		wowed = true

		simple_speak{
			true,
			"SCN6_EGBERT_12", "", egbert,
			"SCN6_PYOU_16", "", pyou
		}

		play_music("music/boss_encounter.mid")

		speak_force_top(false, false, true, t("SCN6_BISOU_1"), "", -1)

		set_entity_animation(egbert, "idle-up")
		set_entity_animation(frogbert, "idle-up")
		set_entity_animation(pyou, "idle-up")
		
		simple_speak{
			true,
			"SCN6_EGBERT_13", "gesture-anim", egbert,
			"SCN6_FROGBERT_12", "thinking", frogbert
		}

		bisou = add_npc("bisou", 2, 82*TILE_SIZE+TILE_SIZE/2, 12*TILE_SIZE)
		set_entity_animation_set_prefix(bisou, "rollfall-")
		set_character_role(bisou, "astar")

		local t = create_astar_tween(bisou, 89*TILE_SIZE+TILE_SIZE/2, 17*TILE_SIZE+TILE_SIZE/2, true)
		append_tween(t, { run = do_bisou_talk })
		new_tween(t)
	elseif (drawing_ring) then
		ring_angle1 = ring_angle1 + 0.05
		ring_angle2 = ring_angle2 - 0.05
		draw_ring_count = draw_ring_count + 1
		if (draw_ring_count >= 60*5) then -- 5 seconds
			set_entity_animation(pyou, "idle")
			drawing_ring = false
			destroy_sample("sfx/crystal_glow.ogg")
			glow_loaded = false
			play_sample("sfx/spend_crystal.ogg", 1, 0, 1)
			wow = true
		end
	elseif (crystal_walk and not crystal_walked) then
		crystal_walked = true

		simple_speak{
			true,
			"SCN6_PYOU_8", "", pyou,
			"SCN6_EGBERT_7", "", egbert,
			"SCN6_PYOU9", "", pyou,
			"SCN6_FROGBERT_7", "thinking", frogbert,
			"SCN6_PYOU_10", "", pyou,
			"SCN6_EGBERT_8", "", egbert,
			"SCN6_PYOU_11", "hmph", pyou,
			"SCN6_FROGBERT_8", "", frogbert,
			"SCN6_PYOU_12", "", pyou,
			"SCN6_EGBERT_9", "snooty-anim", egbert,
			"SCN6_FROGBERT_9", "snooty", frogbert,
			"SCN6_EGBERT_10", "", egbert,
			"SCN6_FROGBERT_10", "", frogbert,
			"SCN6_PYOU_13", "hmph", pyou,
			"SCN6_EGBERT_11", "", egbert,
			"SCN6_PYOU_14", "", pyou,
			"SCN6_FROGBERT_11", "hand-gesture", frogbert,
			"SCN6_PYOU_15", "", pyou
		}

		set_character_role(pyou, "")
		set_entity_animation(pyou, "chant")

		ring_angle1 = 0.0
		ring_angle2 = math.pi
		drawing_ring = true
		draw_ring_count = 0

		play_sample("sfx/crystal_glow.ogg", 1, 0, 1)
	elseif (cute and not said_cute) then
		said_cute = true

		set_entity_animation(pyou, "idle")
		set_entity_right(pyou, false)

		simple_speak{
			true,
			"SCN6_PYOU_7", "", pyou,
			"SCN6_FROGBERT_6", "blush", frogbert,
			"SCN6_EGBERT_5", "", egbert,
			"SCN6_EGBERT_6", "", egbert
		}

		local x, y = get_entity_position(egbert)

		crystal1 = add_entity("crystal", 2, x+16, y)
		add_crystals(-1)

		local t = create_astar_tween(pyou, x+32, y, false)
		append_tween(t, { run = do_crystal_walk })
		new_tween(t)
	elseif (speak_to_egbert and not spoke_to_egbert) then
		spoke_to_egbert = true

		set_entity_animation(pyou, "idle")
		set_entity_right(pyou, false)

		set_entity_animation(egbert, "idle")
		set_entity_right(egbert, true)
		
		set_entity_animation(frogbert, "idle")
		set_entity_right(frogbert, true)

		simple_speak{
			true,
			"SCN6_PYOU_6", "", pyou
		}

		local x, tmp = get_entity_position(pyou)
		local tmp, y = get_entity_position(frogbert)

		local t = create_astar_tween(pyou, x, y, false)
		append_tween(t, create_idle_tween(1))
		append_tween(t, create_astar_tween(pyou, x-8, y, false))
		append_tween(t, { run = do_cute })
		new_tween(t)
	elseif (egbert_interrupt and not egbert_interrupted) then
		egbert_interrupted = true

		simple_speak{
			true,
			"SCN6_EGBERT_4", "gesture", egbert
		}

		set_entity_animation(egbert, "idle-up")

		speak_force_bottom(false, false, true, t("SCN6_PYOU_3"), "", pyou)
		simple_speak{
			true,
			"SCN6_FROGBERT_3", "", frogbert
		}
		speak_force_bottom(false, false, true, t("SCN6_PYOU_4"), "hmph", pyou)
		simple_speak{
			true,
			"SCN6_FROGBERT_4", "thinking", frogbert
		}
		speak_force_bottom(false, false, true, t("SCN6_PYOU_5"), "", pyou)
		simple_speak{
			true,
			"SCN6_FROGBERT_5", "", frogbert
		}

		local x, y = get_entity_position(egbert)
		local x2, y2 = get_entity_position(pyou)

		local t = create_astar_tween(pyou, x2, y-12, false)
		append_tween(t, create_astar_tween(pyou, x+50, y, false))
		append_tween(t, { run = do_speak_to_egbert })
		new_tween(t)
	elseif (pyou_greet and not pyou_greeted) then
		pyou_greeted = true

		simple_speak{
			true,
			"SCN6_EGBERT_3", "", egbert
		}
		speak_force_bottom(false, false, true, t("SCN6_PYOU_2"), "greet", pyou)
		simple_speak{
			true,
			"SCN6_FROGBERT_2", "point", frogbert
		}

		local x, y = get_entity_position(frogbert)

		local t = create_astar_tween(egbert, x, y-10, true)
		append_tween(t, { run = do_egbert_interrupt })
		new_tween(t)
	elseif (start_speech and not started_speech) then
		started_speech = true
		
		pyou = add_npc("pyou", 2, 92*TILE_SIZE+TILE_SIZE/2, 13*TILE_SIZE-1)

		set_entity_right(frogbert, true)

		simple_speak{
			true,
			"SCN6_EGBERT_2", "", egbert,
			"SCN6_FROGBERT_1", "hand-gesture", frogbert,
			"SCN6_PYOU_1", "", pyou
		}

		local x, y = get_entity_position(pyou)
		set_entity_position(pyou, x, y+2)
		set_character_role(pyou, "astar")
		set_entity_animation(pyou, "idle-down")
		pyou_door:collide()

		local t = create_idle_tween(1)
		append_tween(t, create_astar_tween(pyou, 95*TILE_SIZE, 15*TILE_SIZE-1, false))
		append_tween(t, create_gesture_tween(pyou, "idle-down"))
		append_tween(t, { run = do_pyou_greet })
		new_tween(t)
	elseif (not milestone_is_complete(MILESTONE_NAME) and not scene_started and scene_start_block:entity_is_colliding(get_player_id(0))) then
		save_boss_save()

		scene_started = true

		local name = get_entity_name(get_player_id(0))
		if (name == "egbert") then
			egbert = get_player_id(0)
			frogbert = get_player_id(1)
		else
			egbert = get_player_id(1)
			frogbert = get_player_id(0)
		end

		simple_speak{
			true,
			"SCN6_EGBERT_1", "", egbert
		}
		
		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")

		local t = create_astar_tween(egbert, 95*TILE_SIZE+TILE_SIZE/2, 19*TILE_SIZE+TILE_SIZE/2, true)
		if (egbert == get_player_id(0)) then
			append_tween(t, create_center_camera_tween(0, 94*TILE_SIZE+TILE_SIZE, 15*TILE_SIZE+TILE_SIZE, 50))
		end
		new_tween(t)
		
		t = create_idle_tween(1)
		append_tween(t, create_astar_tween(frogbert, 93*TILE_SIZE+TILE_SIZE/2, 19*TILE_SIZE+TILE_SIZE/2, true))
		if (frogbert == get_player_id(0)) then
			append_tween(t, create_center_camera_tween(0, 94*TILE_SIZE+TILE_SIZE, 15*TILE_SIZE+TILE_SIZE, 50))
		end
		append_tween(t, { run = do_start_speech })
		new_tween(t)
	end
end

function collide(id1, id2)
	collide_with_coins(id1, id2)

	if (colliding(id1, id2, get_player_id(0), pyou_door.id)) then
		pyou_door:collide()
	elseif (colliding(id1, id2, get_player_id(0), pyou_entrance)) then
		next_player_layer = 2
		change_areas("pyou_cabin", DIR_N, 64, 184)
	end
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
	destroy_bitmap("misc_graphics/crystal_ring.png")
	destroy_sample("sfx/blush.ogg")
	if (glow_loaded) then
		destroy_sample("sfx/crystal_glow.ogg")
		glow_loaded = false
	end
	destroy_sample("sfx/spend_crystal.ogg")
end

function post_draw_layer(layer)
	if (layer == 2 and drawing_ring) then
		local x, y = get_entity_position(crystal1)
		x = x
		y = y - 8
		local top_x, top_y = get_area_top()
		local w, h = get_bitmap_size(ring)
		local true_w, true_h = get_bitmap_texture_size(ring)
		local time = get_time()
		local f = math.fmod(time, 1)
		if (f >= 0.5) then
			f = 0.5 - (f - 0.5)
		end
		f = f * 0.4
		draw_tinted_rotated_bitmap(ring, 0.2, 0.2, 0.2, 0.2, w/2, h/2, x-top_x, y-top_y, ring_angle2, 0)
		draw_tinted_rotated_bitmap_additive(ring, f, f, f, f, w/2, h/2, x-top_x, y-top_y, ring_angle2, 0)
		draw_tinted_rotated_bitmap(ring, 0.3*0.25, 0.3, 0.3, 0.3, w/2, h/2, x-top_x, y-top_y, ring_angle1, 0)
	end
end

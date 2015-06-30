local MILESTONE_NAME="old_oak_battle"

local player_count = 0

local CRYSTAL1_MILESTONE_NAME = "oldoak_crystal1"
local CRYSTAL2_MILESTONE_NAME = "oldoak_crystal2"
local CRYSTAL3_MILESTONE_NAME = "oldoak_crystal3"
local CRYSTAL4_MILESTONE_NAME = "oldoak_crystal4"
local CRYSTAL5_MILESTONE_NAME = "oldoak_crystal5"
local CRYSTAL6_MILESTONE_NAME = "oldoak_crystal6"
local CRYSTAL7_MILESTONE_NAME = "oldoak_crystal7"
local CRYSTAL8_MILESTONE_NAME = "oldoak_crystal8"

local CRYSTAL1_X = 222
local CRYSTAL1_Y = 376
local CRYSTAL2_X = 240
local CRYSTAL2_Y = 427
local CRYSTAL3_X = 373
local CRYSTAL3_Y = 442
local CRYSTAL4_X = 151
local CRYSTAL4_Y = 297
local CRYSTAL5_X = 124
local CRYSTAL5_Y = 252
local CRYSTAL6_X = 344
local CRYSTAL6_Y = 358
local CRYSTAL7_X = 386
local CRYSTAL7_Y = 360
local CRYSTAL8_X = 471
local CRYSTAL8_Y = 458

function get_players()
	local name = get_entity_name(get_player_id(0))

	if (name == "egbert") then
		egbert = get_player_id(0)
		frogbert = get_player_id(1)
		bisou = get_player_id(2)
	elseif (name == "frogbert") then
		frogbert = get_player_id(0)
		bisou = get_player_id(1)
		egbert = get_player_id(2)
	else
		bisou = get_player_id(0)
		egbert = get_player_id(1)
		frogbert = get_player_id(2)
	end
end

function start(game_just_loaded)
	if (milestone_is_complete(MILESTONE_NAME)) then
		play_music("music/old_forest.mid");
	else
		play_music("music/boss_encounter.mid");
	end

	process_outline()

	tree_crystals = load_bitmap("misc_graphics/oldoak_crystals.png")
	tree = load_bitmap("misc_graphics/oldoak.png")

	-- som bushes
	add_wall_group(2, 15, 5, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 12, 6, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 10, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 4, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 12, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 36, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 30, 20, 2, 2, TILE_GROUP_BUSHES)
	-- mario bushes
	add_wall_group(2, 27, 7, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 27, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 30, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 37, 2, 2, TILE_GROUP_BUSHES)

	add_wall_group(2, 16, 23, 5, 2, 0) -- Old Oak
	
	choppable_array = {}
	choppable_array[1] = { name="of_bush", layer=2, x=12*TILE_SIZE+8, y=33*TILE_SIZE+8, dead=false }
	choppable_array[2] = { name="of_bush", layer=2, x=24*TILE_SIZE+8, y=35*TILE_SIZE+8, dead=false }
	spawn_choppable(choppable_array)

	to_of2 = Active_Block:new{x=15, y=50, width=9, height=1}
	to_map = Active_Block:new{x=35, y=7, width=1, height=5}

	crystals = {
		{ n=1, x=266, y=397 },
		{ n=2, x=250, y=388 },
		{ n=3, x=272, y=417 },
		{ n=4, x=260, y=405 },
		{ n=5, x=309, y=426 },
		{ n=6, x=300, y=420 },
		{ n=7, x=329, y=417 },
		{ n=8, x=321, y=400 },
		{ n=1, x=338, y=398 },
		{ n=3, x=331, y=406 },
		{ n=5, x=304, y=434 },
		{ n=7, x=268, y=428 }
	}

	if (milestone_is_complete(MILESTONE_NAME)) then
		if (not milestone_is_complete(CRYSTAL1_MILESTONE_NAME)) then
			crystal1 = add_entity("crystal", 2, CRYSTAL1_X, CRYSTAL1_Y)
		end
		if (not milestone_is_complete(CRYSTAL2_MILESTONE_NAME)) then
			crystal2 = add_entity("crystal", 2, CRYSTAL2_X, CRYSTAL2_Y)
		end
		if (not milestone_is_complete(CRYSTAL3_MILESTONE_NAME)) then
			crystal3 = add_entity("crystal", 2, CRYSTAL3_X, CRYSTAL3_Y)
		end
		if (not milestone_is_complete(CRYSTAL4_MILESTONE_NAME)) then
			crystal4 = add_entity("crystal", 2, CRYSTAL4_X, CRYSTAL4_Y)
		end
		if (not milestone_is_complete(CRYSTAL5_MILESTONE_NAME)) then
			crystal5 = add_entity("crystal", 2, CRYSTAL5_X, CRYSTAL5_Y)
		end
		if (not milestone_is_complete(CRYSTAL6_MILESTONE_NAME)) then
			crystal6 = add_entity("crystal", 2, CRYSTAL6_X, CRYSTAL6_Y)
		end
		if (not milestone_is_complete(CRYSTAL7_MILESTONE_NAME)) then
			crystal7 = add_entity("crystal", 2, CRYSTAL7_X, CRYSTAL7_Y)
		end
		if (not milestone_is_complete(CRYSTAL8_MILESTONE_NAME)) then
			crystal8 = add_entity("crystal", 2, CRYSTAL8_X, CRYSTAL8_Y)
		end
	end
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
	elseif (not (crystal3 == nil) and activated == crystal3) then
		remove_entity(crystal3)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL3_MILESTONE_NAME, true)
	elseif (not (crystal4 == nil) and activated == crystal4) then
		remove_entity(crystal4)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL4_MILESTONE_NAME, true)
	elseif (not (crystal5 == nil) and activated == crystal5) then
		remove_entity(crystal5)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL5_MILESTONE_NAME, true)
	elseif (not (crystal6 == nil) and activated == crystal6) then
		remove_entity(crystal6)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL6_MILESTONE_NAME, true)
	elseif (not (crystal7 == nil) and activated == crystal7) then
		remove_entity(crystal7)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL7_MILESTONE_NAME, true)
	elseif (not (crystal8 == nil) and activated == crystal8) then
		remove_entity(crystal8)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL8_MILESTONE_NAME, true)
	end
end

function count_players(tween, id, elapsed)
	player_count = player_count + 1
	return true
end

function start_speak_tween(tween, id, elapsed)
	start_speak = true
	return true
end

function start_battle_tween(tween, id, elapsed)
	_start_battle = true
	return true
end

function change_bitmaps_tween(tween, id, elapsed)
	change_bitmaps = true
	return true
end

function old_oak_better_tween(tween, id, elapsed)
	old_oak_better = true
	return true
end

function flash(tween, id, elasped)
	add_flash(0.0, 1.0, 3.0, 0.5, 255, 255, 255, 255)
	return true
end


local first_run = true

function logic()
	if (not milestone_is_complete(MILESTONE_NAME) and first_run) then
		first_run = false

		save_boss_save()

		for i=1,#crystals do
			crystals[i].id = add_entity("crystals", 2, crystals[i].x, crystals[i].y)
			set_entity_animation(crystals[i].id, "" .. crystals[i].n)
		end
	
		get_players()

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		local t
		
		t = create_astar_tween(egbert, 16*TILE_SIZE+TILE_SIZE/2, 30*TILE_SIZE, true)
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_astar_tween(frogbert, 18*TILE_SIZE+TILE_SIZE/2, 30*TILE_SIZE, true)
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_astar_tween(bisou, 20*TILE_SIZE+TILE_SIZE/2, 30*TILE_SIZE, true)
		append_tween(t, { run = count_players })
		new_tween(t)
	end

	update_choppable()

	if (to_of2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("oldforest2", DIR_S, 1079, 56)
	elseif (to_map:entity_is_colliding(0)) then
		start_map("OLD_FOREST")
	end
	
	if (not milestone_is_complete(MILESTONE_NAME)) then
		if (player_count == 2 and old_oak_bettered) then
			set_character_role(get_player_id(1), "follow", get_player_id(0))
			set_character_role(get_player_id(2), "follow", get_player_id(1))

			set_character_role(get_player_id(0), "")

			local t = create_reset_camera_tween(0.0, 50)
			new_tween(t)
			
			set_milestone_complete(MILESTONE_NAME, true)
		elseif (old_oak_better and not old_oak_bettered) then
			old_oak_bettered = true

			simple_speak{
				true,
				"OLDOAK_BISOU_6", "", bisou,
				"OLDOAK_1", "", -1,
				"OLDOAK_BISOU_7", "", bisou,
				"OLDOAK_FROGBERT_6", "", frogbert,
				"OLDOAK_2", "", -1,
				"OLDOAK_BISOU_8", "idle", bisou,
				"OLDOAK_3", "", -1,
				"OLDOAK_EGBERT_2", "", egbert,
				"OLDOAK_4", "", -1,
				"OLDOAK_FROGBERT_7", "", frogbert,
				"OLDOAK_5", "", -1,
				"OLDOAK_6", "", -1,
				"OLDOAK_FROGBERT_8", "", frogbert,
				"OLDOAK_EGBERT_3", "idle", egbert
			}

			-- FIXME: add a few new crystals

			fade_out()

			set_entity_layer(egbert, 2)
			set_entity_layer(frogbert, 2)
			set_entity_layer(bisou, 2)

			draw_tree = false

			fade_in()

			player_count = 0

			local x, y = get_entity_position(get_player_id(0))

			local t
			
			t = create_astar_tween(get_player_id(1), x, y, false)
			append_tween(t, { run = count_players })
			new_tween(t)

			t = create_astar_tween(get_player_id(2), x, y, false)
			append_tween(t, { run = count_players })
			new_tween(t)
		elseif (change_bitmaps and not changed_bitmaps) then
			changed_bitmaps = true

			draw_tree_crystals = false
			draw_tree = true

			local t = create_idle_tween(2)
			append_tween(t, { run = old_oak_better_tween })
			new_tween(t)
		elseif (really_started_battle and not ended_battle) then
			ended_battle = true

			set_entity_direction(egbert, DIR_N)
			set_entity_direction(frogbert, DIR_N)
			set_entity_direction(bisou, DIR_N)

			crystal1 = add_entity("crystal", 2, CRYSTAL1_X, CRYSTAL1_Y)
			crystal2 = add_entity("crystal", 2, CRYSTAL2_X, CRYSTAL2_Y)
			crystal3 = add_entity("crystal", 2, CRYSTAL3_X, CRYSTAL3_Y)
			crystal4 = add_entity("crystal", 2, CRYSTAL4_X, CRYSTAL4_Y)
			crystal5 = add_entity("crystal", 2, CRYSTAL5_X, CRYSTAL5_Y)
			crystal6 = add_entity("crystal", 2, CRYSTAL6_X, CRYSTAL6_Y)
			crystal7 = add_entity("crystal", 2, CRYSTAL7_X, CRYSTAL7_Y)
			crystal8 = add_entity("crystal", 2, CRYSTAL8_X, CRYSTAL8_Y)

			play_music("music/old_forest.mid");

			local t = create_idle_tween(1)
			append_tween(t, { run = flash })
			append_tween(t, create_idle_tween(1))
			append_tween(t, { run = change_bitmaps_tween })
			new_tween(t)
		elseif (_start_battle and not started_battle) then
			started_battle = true

			draw_yellow_glow = true

			play_sample("sfx/ribbit.ogg", 1, 0, 1)
			speak(false, false, true, "<b><color 255 255 255>" .. t("FROGBERT") .. ":" .. "</color></b>" .. " " .. t("OLDOAK_FROGBERT_5"), "", -1)
			
			play_music("music/boss.mid");
			start_battle("oldoak", "oldoak", true, "oldoak")
			
			really_started_battle = true
		elseif (speak2 and not spoke2) then
			spoke2 = true

			play_sample("sfx/bisou.ogg", 1, 0, 1)
			speak(false, false, true, "<b><color 255 255 255>" .. t("BISOU") .. ":" .. "</color></b>" .. " " .. t("OLDOAK_BISOU_5"), "", -1)

			add_flash(0.0, 0.5, 1.0, 0.5, 0, 255, 255, 255)

			local t = create_idle_tween(1.0)
			append_tween(t, { run = start_battle_tween })
			new_tween(t)
		elseif (start_speak and not started_speak) then
			started_speak = true

			set_entity_right(egbert, true)
			set_entity_right(frogbert, true)
			set_entity_right(bisou, false)

			simple_speak{
				true,
				"OLDOAK_EGBERT_1", "", egbert,
				"OLDOAK_BISOU_1", "", bisou,
				"OLDOAK_FROGBERT_1", "", frogbert,
				"OLDOAK_BISOU_2", "", bisou,
				"OLDOAK_FROGBERT_2", "", frogbert,
				"OLDOAK_BISOU_3", "", bisou,
				"OLDOAK_FROGBERT_3", "thinking", frogbert,
				"OLDOAK_BISOU_4", "laugh", bisou,
				"OLDOAK_FROGBERT_4", "", frogbert
			}

			fade_out()

			set_entity_layer(egbert, 4)
			set_entity_layer(frogbert, 4)
			set_entity_layer(bisou, 4)

			for i=1,#crystals do
				remove_entity(crystals[i].id)
			end

			draw_tree_crystals = true

			fade_in()

			speak2 = true
		elseif (player_count == 3 and not arrived) then
			arrived = true

			set_entity_direction(egbert, DIR_N)
			set_entity_direction(frogbert, DIR_N)
			set_entity_direction(bisou, DIR_N)

			local t = create_center_camera_tween(0.0, 18*TILE_SIZE+TILE_SIZE/2, 26*TILE_SIZE+TILE_SIZE/2, 50)
			append_tween(t, { run = start_speak_tween })
			new_tween(t)
		end
	end
end

function collide(id1, id2)
	collide_with_coins(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
	destroy_bitmap("misc_graphics/oldoak_crystals.png")
	destroy_bitmap("misc_graphics/oldoak.png")
end

function mid_draw_layer(layer)
	if (layer == 4) then
		local scr_w, scr_h = get_screen_size()
		local bmp_w, bmp_h = get_bitmap_size(tree_crystals)
		if (draw_tree_crystals) then
			if (draw_yellow_glow) then
				draw_bitmap_yellow_glow(tree_crystals, scr_w/2-bmp_w/2, scr_h/2-bmp_h/2, 0, 183, 207, 243, 183, 207, 243)
			else
				draw_bitmap(tree_crystals, scr_w/2-bmp_w/2, scr_h/2-bmp_h/2, 0)
			end
		elseif (draw_tree) then
			draw_bitmap(tree, scr_w/2-bmp_w/2, scr_h/2-bmp_h/2, 0)
		end
	end
end

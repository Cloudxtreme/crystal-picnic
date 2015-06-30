local GUARDS_MILESTONE_NAME = "heading_west_guards"
local ran_to_frogbert = false
local picked_up_stick = false
local start_battle_guards = false
local after_battle_guards = false
local change_camera = false
local last_thoughts = false
local camera_offset = 0

function run_to_frogbert(tween, id, elapsed)
	ran_to_frogbert = true
	return true
end

function pickup_stick(tween, id, elapsed)
	picked_up_stick = true
	return true
end

function battle_guards(tween, id, elapsed)
	start_battle_guards = true
	return true
end

function remove_egbert(tween, id, elapsed)
	remove_entity(egbert)
	return true
end

function remove_frogbert(tween, id, elapsed)
	-- no need to keep entities, but maybe backup stats? FIXME

	set_milestone_complete(GUARDS_MILESTONE_NAME, true)
	hold_milestones(false)

	remove_entity(frogbert)
	
	start_map("CRYSTAL_CASTLE_ENTRANCE")

	return true
end

function start()
	play_music("music/abw.mid");

	process_outline()

	-- Mario bushes
	add_wall_group(3, 2, 2, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 5, 6, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 4, 9, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 5, 11, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 2, 18, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 1, 21, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 6, 22, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 8, 19, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 12, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 14, 18, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 14, 21, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 18, 18, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 19, 21, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 25, 19, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 28, 21, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 45, 18, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 48, 20, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 52, 19, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 46, 7, 4, 2, TILE_GROUP_BUSHES)

	-- SOM bushes
	add_wall_group(3, 17, 9, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 19, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 21, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 22, 9, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 26, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 30, 10, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 28, 1, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 50, 2, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 57, 6, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 58, 0, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 60, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 67, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 28, 24, 2, 2, TILE_GROUP_BUSHES)

	-- Trees
	add_wall_group(3, 60, 14, 4, 6, 0)
	add_wall_group(3, 65, 16, 4, 6, 0)
	add_wall_group(3, 52, 6, 4, 6, 0)
	add_wall_group(3, 62, 5, 4, 6, 0)
	add_wall_group(3, 0, 5, 4, 6, 0)

	-- Partial trees
	add_wall_group(3, 15, 0, 4, 4, 0)
	add_wall_group(3, 44, 0, 4, 3, 0)
	add_wall_group(3, 66, 0, 4, 4, 0)

	-- Bridge
	add_wall_group(3, 32, 6, 11, 2, 0)

	choppable = {}
	choppable[1] = new_choppable("bush", 3, 60*TILE_SIZE+8, 20*TILE_SIZE+8)
	choppable[2] = new_choppable("bush", 3, 41*TILE_SIZE+8, 13*TILE_SIZE+8)

	if (not milestone_is_complete(GUARDS_MILESTONE_NAME)) then
		ant1 = add_npc("ant", 3, 700, 100)
		ant2 = add_npc("ant", 3, 700, 115)

		frogbert = add_npc("frogbert", 3, 724, 107)
		set_entity_solid_with_entities(frogbert, false)
		set_entity_direction(frogbert, DIR_W)

		egbert = add_npc("egbert", 3, 1000, 50)
		set_entity_direction(egbert, DIR_S)
		set_entity_solid_with_entities(egbert, false)
		set_character_role(egbert, "astar")

		camera_entity = egbert

		local t = create_idle_tween(1)
		t.next_tween = create_astar_tween(egbert, 755, 107, true)
		t.next_tween.next_tween = { run = run_to_frogbert, started = false }
		new_tween(t)
	end

	west_exit = Active_Block:new{x=0, y=3, width=1, height=6}
	east_exit = Active_Block:new{x=61, y=0, width=5, height=2}
end

function logic()
	if (not milestone_is_complete(GUARDS_MILESTONE_NAME)) then
		if (ran_to_frogbert) then
			ran_to_frogbert = false

			simple_speak{
				true,
				"SCN3_FROGBERT_1", "", frogbert
			}

			local t = create_astar_tween(egbert, 755, 127, false)
			t.next_tween = { run = pickup_stick, started = false }
			new_tween(t)

		elseif (picked_up_stick) then
			picked_up_stick = false

			play_sample("sfx/item_found.ogg", 1, 0, 1)

			speak(
				true,
				false,
				true,
				t("FOUND_A_STICK"),
				"",
				0
			)

			local t = create_astar_tween(egbert, 724, 122, false)
			t.next_tween = { run = battle_guards, started = false }
			new_tween(t)
		elseif (start_battle_guards) then
			start_battle_guards = false
			start_battle("abw2", "abw", false, "ant", "ant")
			remove_entity(ant1)
			remove_entity(ant2)
			after_battle_guards = true
		elseif (after_battle_guards) then
			after_battle_guards = false

			set_character_role(egbert, "none")

			simple_speak{
				true,
				"SCN3_EGBERT_1", "", egbert,
				"SCN3_FROGBERT_2", "", frogbert,
				"SCN3_EGBERT_2", "", egbert,
				"SCN3_FROGBERT_3", "", frogbert,
				"SCN3_EGBERT_3", "", egbert,
				"SCN3_FROGBERT_4", "", frogbert,
				"SCN3_EGBERT_4", "", egbert,
				"SCN3_FROGBERT_5", "point", frogbert,
				"SCN3_EGBERT_5", "facepalm", egbert,
				"SCN3_FROGBERT_6", "", frogbert,
				"SCN3_EGBERT_6", "", egbert
			}

			set_character_role(egbert, "astar")

			local t = create_astar_tween(egbert, 17, 99, true)
			t.next_tween = { run = remove_egbert, started = false }
			new_tween(t)

			camera_offset = 15
			camera_entity = frogbert
			change_camera = true
		elseif (last_thoughts) then
			last_thoughts = false

			simple_speak{
				true,
				"SCN3_FROGBERT_7", "thinking", frogbert
			}

			set_character_role(frogbert, "astar")
			t = create_astar_tween(frogbert, 17, 99, true)
			t.next_tween = { run = remove_frogbert, started = false }
			new_tween(t)
		end

		set_camera_to(camera_entity)
		if (change_camera) then
			camera_offset = camera_offset - (15 / LOGIC_RATE)
			if (camera_offset <= 0) then
				change_camera = false
				camera_offset = 0
				last_thoughts = true
			end
		end
		local x, y = get_camera_offset()
		y = y + camera_offset
		set_camera_offset(x, y)
	end

	update_choppable()

	if (milestone_is_complete(GUARDS_MILESTONE_NAME)) then
		if (west_exit:entity_is_colliding(0)) then
			start_map("CRYSTAL_CASTLE_ENTRANCE")
		elseif (east_exit:entity_is_colliding(0)) then
			next_player_layer = 4
			change_areas("crystal_castle_entrance", DIR_N, 215, 1034)
		end
	end
end

function collide(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
end

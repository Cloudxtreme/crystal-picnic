local ANT_TRACKS_MILESTONE_NAME="ant_tracks"
local ant_tracks_speak1 = false
local ant_tracks_speak2 = false
local exit_x = 13.5*TILE_SIZE
local exit_y = 68*TILE_SIZE-6

function end_ant_tracks(tween, id, elapsed)
	remove_entity(egbert)

	set_milestone_complete(ANT_TRACKS_MILESTONE_NAME, true)

	next_player_layer = 3

	change_areas("heading_west", DIR_S, 0, 0)

	return true
end

function wait_up_frogbert(tween, id, elapsed)
	ant_tracks_speak2 = true
	return true
end

function remove_frogbert(tween, id, elapsed)
	remove_entity(frogbert)
	return true
end

function end_first_run(tween, id, elapsed)
	ant_tracks_speak1 = true
	return true
end

function start()
	play_music("music/abw.mid");

	process_outline()

	-- SOM shrubs
	add_wall_group(4, 1, 0, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 5, 1, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 0, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 31, 3, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 35, 5, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 34, 10, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 31, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 3, 64, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 18, 58, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 31, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 33, 61, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 29, 64, 2, 2, TILE_GROUP_BUSHES)

	-- Mario shrubs
	add_wall_group(4, 0, 15, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 5, 14, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 1, 17, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 23, 14, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 21, 17, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 25, 17, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 29, 17, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 20, 20, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 25, 21, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 19, 53, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 18, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 33, 58, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 19, 60, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 17, 62, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 33, 63, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 2, 66, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 8, 66, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 20, 64, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 28, 66, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(4, 31, 65, 4, 2, TILE_GROUP_BUSHES)

	-- Apple blossom trees
	add_wall_group(4, 6, 7, 5, 7, 0)
	add_wall_group(4, 15, 9, 5, 7, 0)
	add_wall_group(4, 6, 17, 5, 7, 0)
	add_wall_group(4, 15, 16, 5, 7, 0)
	add_wall_group(4, 20, 22, 5, 7, 0)
	add_wall_group(4, 13, 27, 5, 7, 0)

	-- Small trees
	add_wall_group(4, 8, 59, 4, 5, 0)
	add_wall_group(4, 29, 58, 4, 5, 0)

	-- Arch
	add_wall_group(4, 15, 45, 6, 8, 0)

	-- Add destructable bushes
	choppable = {}
	choppable[#choppable+1] = new_choppable("bush", 4, 34*TILE_SIZE+8, 2*TILE_SIZE+8)
	choppable[#choppable+1] = new_choppable("bush", 4, 26*TILE_SIZE+8, 14*TILE_SIZE+8)
	choppable[#choppable+1] = new_choppable("bush", 4, 24*TILE_SIZE+8, 21*TILE_SIZE+8)
	choppable[#choppable+1] = new_choppable("bush", 4, 5*TILE_SIZE+8, 17*TILE_SIZE+8)
	choppable[#choppable+1] = new_choppable("bush", 4, 7*TILE_SIZE+8, 67*TILE_SIZE+8)
	choppable[#choppable+1] = new_choppable("bush", 4, 17*TILE_SIZE+8, 60*TILE_SIZE+8)

	if (not milestone_is_complete(ANT_TRACKS_MILESTONE_NAME)) then
		local ex = 230
		local ey = 40
		
		egbert = add_npc("egbert", 4, ex, ey)
		set_entity_direction(egbert, DIR_S)
		set_entity_solid_with_entities(egbert, false)
		set_character_role(egbert, "astar")
		set_camera_to(egbert)
		
		frogbert = add_npc("frogbert", 4, ex, ey + 50)
		set_entity_direction(frogbert, DIR_S)
		set_entity_solid_with_entities(frogbert, false)
		set_character_role(frogbert, "astar")

		local t = create_idle_tween(1)
		t.next_tween = create_astar_tween(frogbert, 282, 587, true)
		new_tween(t)

		t = create_idle_tween(2)
		t.next_tween = create_astar_tween(egbert, 282, 537, true)
		t.next_tween.next_tween = { run = end_first_run, started = false }
		new_tween(t)
	end

	south_exit = Active_Block:new{x=11, y=67, width=5, height=1}
	north_exit = Active_Block:new{x=8, y=0, width=22, height=2}
end

function logic()
	if (not milestone_is_complete(ANT_TRACKS_MILESTONE_NAME)) then
		set_camera_to(egbert)
		if (ant_tracks_speak1) then
			ant_tracks_speak1 = false

			set_character_role(egbert, "none")
			set_character_role(frogbert, "none")

			set_entity_direction(egbert, DIR_S)
			set_entity_direction(frogbert, DIR_N)

			simple_speak{
				true,
				"SCN2_FROGBERT_1", "", frogbert
			}
			speak_force_bottom(false, false, true, t("SCN2_EGBERT_1"), "facepalm", egbert)
			simple_speak{
				true,
				"SCN2_FROGBERT_2", "", frogbert
			}
			speak_force_bottom(false, false, true, t("SCN2_EGBERT_2"), "", egbert)
			simple_speak{
				true,
				"SCN2_FROGBERT_3", "snooty", frogbert
			}
			speak_force_bottom(false, false, true, t("SCN2_EGBERT_3"), "snooty-anim", egbert)
			simple_speak{
				true,
				"SCN2_FROGBERT_4", "", frogbert
			}

			set_character_role(egbert, "astar")
			set_character_role(frogbert, "astar")

			local t = create_astar_tween(frogbert, exit_x, exit_y, true)
			t.next_tween = { run = remove_frogbert, started = false }
			new_tween(t)

			t = create_idle_tween(1)
			t.next_tween = { run = wait_up_frogbert, started = false }
			new_tween(t)
		elseif (ant_tracks_speak2) then
			ant_tracks_speak2 = false

			speak_force_bottom(false, false, true, t("SCN2_EGBERT_4"), "", egbert)
			
			local t = create_astar_tween(egbert, exit_x, exit_y, true)
			t.next_tween = { run = end_ant_tracks, started = false }
			new_tween(t)
		end
	end

	update_choppable()
	
	if (milestone_is_complete(ANT_TRACKS_MILESTONE_NAME)) then
		if (south_exit:entity_is_colliding(0)) then
			next_player_layer = 3
			change_areas("heading_west", DIR_S, 1015, 43)
		elseif (north_exit:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle", DIR_N, 576, 1427)
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

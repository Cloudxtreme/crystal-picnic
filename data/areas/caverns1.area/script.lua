local MILESTONE_NAME = "CAVERNS_INTRO"

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
	play_music("music/caverns.mid");

	process_outline()

	add_wall_group(2, 3, 14, 2, 2, 0)

	to_map = Active_Block:new{x=7, y=8, width=3, height=1}
	to_caverns2 = Active_Block:new{x=27, y=0, width=5, height=1}

	if (not milestone_is_complete(MILESTONE_NAME)) then
		get_players()

		do_scene = true
	end

	add_entity("large_boulder", 2, 32*TILE_SIZE, 14*TILE_SIZE)
end

function activate(activator, activated)
end

local player_count = 0

function count_players(tween, id, elapsed)
	player_count = player_count + 1
	return true
end

function start_talk(tween, id, elapsed)
	do_start_talk = true
	return true
end

function logic()
	if (to_map:entity_is_colliding(0)) then
		start_map("CAVERNS")
	elseif (to_caverns2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns2", DIR_N, 62.5*TILE_SIZE, 72.5*TILE_SIZE)
	end

	if (player_count == 2 and not done) then
		done = true

		set_entity_input_disabled(get_player_id(0), false)
		set_character_role(get_player_id(1), "follow", get_player_id(0))
		set_character_role(get_player_id(2), "follow", get_player_id(1))

		set_milestone_complete(MILESTONE_NAME, true)
	elseif (do_start_talk and not started_talk) then
		started_talk = true

		set_entity_right(frogbert, true)

		set_entity_animation(egbert, "idle-up")
		set_entity_animation(frogbert, "idle")
		set_entity_animation(bisou, "idle-down")

		simple_speak{
			true,
			"SCAV_ENTRANCE_EGBERT_1", "", egbert,
			"SCAV_ENTRANCE_FROGBERT_1", "", frogbert,
			"SCAV_ENTRANCE_EGBERT_2", "", egbert,
			"SCAV_ENTRANCE_BISOU_1", "laugh", bisou,
			"SCAV_ENTRANCE_EGBERT_3", "", egbert,
			"SCAV_ENTRANCE_FROGBERT_2", "", frogbert
		}

		set_character_role(get_player_id(1), "astar")
		set_character_role(get_player_id(2), "astar")

		local x, y = get_entity_position(get_player_id(0))

		local t

		t = create_astar_tween(get_player_id(1), x, y, false)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(get_player_id(2), x, y, false)
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (do_scene and not scene_started) then
		scene_started = true

		local t = create_idle_tween(2)
		append_tween(t, { run = start_talk })
		new_tween(t)
	end
end

function collide(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
end

function stop()
end

function pre_draw_layer(layer)
	if (layer == 2 and not milestone_is_complete(MILESTONE_NAME) and not set_on_ground) then
		set_on_ground = true

		set_entity_visible(egbert, true)
		set_entity_visible(frogbert, true)
		set_entity_visible(bisou, true)

		set_entity_position(frogbert, 25.5*TILE_SIZE, 10.5*TILE_SIZE)
		set_entity_position(bisou, 31.5*TILE_SIZE, 9.5*TILE_SIZE)

		set_entity_animation(egbert, "on-ground")
		set_entity_animation(frogbert, "on-ground")
		set_entity_animation(bisou, "on-ground")
	end
end

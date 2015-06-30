is_dungeon = true

local MS_NAME = "smashed_caverns7_wall"

function hide_wall()
	local w, h = get_area_tile_size()
	local x, y
	for y=1,h-1 do
		for x=1,w-1 do
			set_tile(4, x, y, -1, -1, false)
		end
	end
	set_tile(1, 29, 44, -1, -1, false)
	set_tile(1, 29, 45, -1, -1, false)
	remove_entity(wall_blocker1)
	remove_entity(wall_blocker2)
end

function start(game_just_loaded)
	play_music("music/caverns.mid")

	process_outline()

	-- stalactites
	add_wall_group(3, 9, 16, 2, 6, 0)
	add_wall_group(3, 19, 28, 2, 6, 0)
	add_wall_group(3, 10, 31, 2, 6, 0)

	-- stuff on carts
	add_wall_group(3, 27, 28, 3, 2, 0)
	add_wall_group(3, 27, 63, 3, 2, 0)
	add_wall_group(3, 30, 63, 3, 2, 0)

	add_enemies(3, nil, 0, 0, 0, "")

	chest = Chest:new{x=46.5*TILE_SIZE, y=11.5*TILE_SIZE, layer=3, contains_gold=true, contains="CASH", quantity=100, milestone="caverns7_chest1"}

	to_caverns5 = Active_Block:new{x=6, y=0, width=6, height=1}

	wall_blocker1 = add_polygon_entity(3, 330, 676, 312, 731, 370, 704)
	wall_blocker2 = add_polygon_entity(3, 476, 696, 473, 750, 440, 709)

	if (milestone_is_complete(MS_NAME)) then
		hide_wall()
	end

	scene_activate_block = Active_Block:new{x=33, y=56, width=7, height=1}
	
	load_sample("sfx/rumble.ogg", true)
	load_sample("sfx/bomb.ogg", false)
	load_sample("sfx/boing.ogg", false)
end

function activate(activator, activated)
	if (activated == chest.id) then
		chest:open()
	end
end

local player_count = 0

function count_players(tween, id, elapsed)
	player_count = player_count + 1
	return true
end

function after_attack(tween, id, elapsed)
	remove_entity(egbert_clone)
	set_entity_visible(egbert, true)
	after_attacked = true
	return true
end

function after_shake(tween, id, elapsed)
	after_shook = true
	return true
end

function after_shake2(tween, id, elapsed)
	after_shook2 = true
	return true
end

function shake_tween(tween, id, elapsed)
	if (not tween.started) then
		shake(0, tween.duration, tween.amount)
		play_sample("sfx/rumble.ogg", 1, 0, 1)
		tween.started = true
	end
	local ret
	if (elapsed >= tween.duration) then
		ret = true
		stop_sample("sfx/rumble.ogg")
	else
		ret = false
	end
	if (tween.playbig) then
		play_sample("sfx/giant_footsteps.ogg", 1, 0, 1)
		tween.playbig = false
	end
	return ret
end

function bomb_tween(tween, id, elapsed)
	add_bomb_puff(3, tween.x + rand(11) - 5, tween.y + rand(11) - 5, 15)
	play_sample("sfx/bomb.ogg", 1, 0, 1)

	return true
end

function bigant_idle(tween, id, elapsed)
	set_entity_animation(bigant, "idle-sign")
	end_shake()
	stop_sample("sfx/rumble.ogg")
	return true
end

function hide_wall_tween(tween, id, elapsed)
	hide_wall()
	set_milestone_complete(MS_NAME, true)
	return true
end

function start_run_away(tween, id, elapsed)
	ran_away = true
	return true
end

function do_center(tween, id, elapsed)
	centered = true
	return true
end

function logic()
	if (to_caverns5:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns5", DIR_N, 39*TILE_SIZE, 16*TILE_SIZE)
	elseif (not milestone_is_complete(MS_NAME) and scene_activate_block:entity_is_colliding(0) and not scene_started) then
		save_boss_save()

		scene_started = true

		if (get_entity_name(get_player_id(0)) == "egbert") then
			egbert = get_player_id(0)
			frogbert = get_player_id(1)
			bisou = get_player_id(2)
		elseif (get_entity_name(get_player_id(1)) == "egbert") then
			egbert = get_player_id(1)
			frogbert = get_player_id(2)
			bisou = get_player_id(0)
		else
			egbert = get_player_id(2)
			frogbert = get_player_id(0)
			bisou = get_player_id(1)
		end

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		set_entity_solid_with_entities(egbert, false)
		set_entity_solid_with_entities(frogbert, false)
		set_entity_solid_with_entities(bisou, false)

		local t = {}

		t = create_astar_tween(egbert, 35.5*TILE_SIZE, 45.5*TILE_SIZE, false)
		append_tween(t, create_change_direction_tween(egbert, DIR_E))
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(frogbert, 37.5*TILE_SIZE, 45.5*TILE_SIZE, false)
		append_tween(t, create_change_direction_tween(frogbert, DIR_W))
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(bisou, 39.5*TILE_SIZE, 46.5*TILE_SIZE, false)
		append_tween(t, create_change_direction_tween(bisou, DIR_W))
		append_tween(t, { run = count_players })
		new_tween(t)
	end

	if (battle_started and not battle_ended) then
		battle_ended = true

		revive_everyone()

		set_milestone_complete("beat_antboss", true)

		next_player_layer = 2
		change_areas("junglevalley1", DIR_N, 410, 777)
	elseif (final_speak and not changed_to_battle and player_count == 3) then
		changed_to_battle = true
		fade_out()

		set_entity_visible(egbert, false)
		set_entity_visible(frogbert, false)
		set_entity_visible(bisou, false)

		battle_started = true

		revive_everyone()
	
		start_battle("cart", "cart", true)
	elseif (ran_away and last_speak and not finished_run_away) then
		finished_run_away = true

		set_character_role(egbert, "astar")
		set_character_role(bisou, "astar")
		
		t = create_astar_tween(egbert, 51.5*TILE_SIZE, 706, true)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(frogbert, 52.5*TILE_SIZE, 706, true)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(bisou, 53.5*TILE_SIZE, 706, true)
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (shook2 and not final_speak and player_count == 3) then
		player_count = 0
		final_speak = true

		play_sample("sfx/boing.ogg", 1, 0, 1)
		set_character_role(egbert, "")
		set_character_role(bisou, "")
		set_entity_animation(egbert, "surprised")
		set_entity_animation(bisou, "surprised")

		local t = create_center_camera_tween(0, 33.5*TILE_SIZE, 43.5*TILE_SIZE, 50)
		append_tween(t, { run = start_run_away })
		new_tween(t)

		simple_speak{
			true,
			"CAVERNS7_FROGBERT_4", "surprised", frogbert
		}

		last_speak = true

		t = create_reset_camera_tween(0, 250)
		new_tween(t)
	elseif (shook and not shook2 and after_shook2) then
		player_count = 0
		shook2 = true

		simple_speak{
			true,
			"CAVERNS7_EGBERT_4", "", egbert,
			"CAVERNS7_FROGBERT_3", "", frogbert,
			"CAVERNS7_EGBERT_5", "", egbert
		}

		play_music("music/boss_encounter.mid")
		
		new_tween({ run = shake_tween, duration = 1000000, amount = 5, playbig=false })

		bigant = add_npc("fieldantboss", 3, 18.5*TILE_SIZE, 45.5*TILE_SIZE)
		redraw()

		set_character_role(bigant, "")
		set_entity_animation(bigant, "charge")
		set_entity_solid_with_area(bigant, false)
		set_entity_solid_with_entities(bigant, false)

		local t = { run = bomb_tween, x = 20*TILE_SIZE, y=44*TILE_SIZE }
		append_tween(t, create_idle_tween(0.1))
		append_tween(t, { run = bomb_tween, x = 22*TILE_SIZE, y=44*TILE_SIZE })
		append_tween(t, create_idle_tween(0.1))
		append_tween(t, { run = bomb_tween, x = 24*TILE_SIZE, y=44*TILE_SIZE })
		append_tween(t, create_idle_tween(0.1))
		append_tween(t, { run = bomb_tween, x = 26*TILE_SIZE, y=44*TILE_SIZE })
		append_tween(t, create_idle_tween(0.1))
		append_tween(t, { run = bomb_tween, x = 28*TILE_SIZE, y=44*TILE_SIZE })
		append_tween(t, { run = hide_wall_tween })
		append_tween(t, create_idle_tween(0.1))
		append_tween(t, { run = bomb_tween, x = 30*TILE_SIZE, y=44*TILE_SIZE })
		new_tween(t)

		t = create_direct_move_xyz_tween(bigant, 1, 31.5*TILE_SIZE, 45.5*TILE_SIZE, 0)
		append_tween(t, { run = bigant_idle })
		new_tween(t)
		
		set_character_role(egbert, "astar")

		t = create_idle_tween(0.5)
		append_tween(t, create_astar_tween(egbert, 36.5*TILE_SIZE, 45.5*TILE_SIZE, true))
		append_tween(t, create_change_direction_tween(egbert, DIR_W))
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_idle_tween(0.5)
		append_tween(create_astar_tween(frogbert, 37.5*TILE_SIZE, 45.5*TILE_SIZE, true))
		append_tween(t, create_change_direction_tween(frogbert, DIR_W))
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_idle_tween(0.5)
		append_tween(create_astar_tween(bisou, 38.5*TILE_SIZE, 45.5*TILE_SIZE, true))
		append_tween(t, create_change_direction_tween(bisou, DIR_W))
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (done_attack and not shook and after_shook) then
		shook = true

		simple_speak{
			true,
			"CAVERNS7_EGBERT_3", "", egbert,
			"CAVERNS7_BISOU_1", "", bisou
		}

		new_tween({ run = shake_tween, duration = 1, amount = 5, playbig=false })

		local t = create_idle_tween(1)
		append_tween(t, { run = after_shake2 })
		new_tween(t)
	elseif (hit_wall and not done_attack and after_attacked) then
		done_attack = true

		simple_speak{
			true,
			"CAVERNS7_FROGBERT_2", "", frogbert
		}

		new_tween({ run = shake_tween, duration = 1, amount = 5, playbig=false })


		local t = create_idle_tween(1)
		append_tween(t, { run = after_shake })
		new_tween(t)
	elseif (begin_speech and not hit_wall and player_count == 1) then
		hit_wall = true

		local ex, ey = get_entity_position(egbert)
		egbert_clone = add_entity("egbert", 3, ex, ey)
		set_entity_visible(egbert, false)
		set_entity_right(egbert_clone, false)
		set_entity_animation(egbert_clone, "attack")
		play_sample("sfx/swing_weapon.ogg", 1, 0, 1)

		local t = create_idle_tween(0.5)
		append_tween(t, create_change_direction_tween(egbert, DIR_E))
		append_tween(t, { run = after_attack })
		new_tween(t)
	elseif (centered and not begin_speech) then
		begin_speech = true
		player_count = 0

		simple_speak{
			true,
			"CAVERNS7_EGBERT_1", "", egbert
		}

		set_entity_right(frogbert, true)

		simple_speak{
			true,
			"CAVERNS7_FROGBERT_1", "", frogbert
		}
		
		set_entity_right(frogbert, false)

		simple_speak{
			true,
			"CAVERNS7_EGBERT_2", "snooty-anim", egbert
		}

		local t = create_astar_tween(egbert, 31*TILE_SIZE, 45.5*TILE_SIZE, false)
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (scene_started and not centering and player_count == 3) then
		centering = true
		local x, y = get_entity_position(egbert)
		local t = create_center_camera_tween(0, x, y, 50)
		append_tween(t, { run = do_center })
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
	destroy_sample("sfx/rumble.ogg")
	destroy_sample("sfx/bomb.ogg")
	destroy_sample("sfx/boing.ogg")
end

function post_draw_layer(layer)
	if (layer == 4 and battle_started) then
		clear_to_color(0, 0, 0, 1)
	end
end

local MILESTONE_NAME = "STONECRATER_BIGHOLE"

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
	play_music("music/stonecrater.mid");

	process_outline()

	add_wall_group(2, 18, 8, 1, 2, 0)
	add_wall_group(2, 7, 7, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 12, 6, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 13, 5, 3, 2, TILE_GROUP_BUSHES)

	to_stonecrater5 = Active_Block:new{x=9, y=21, width=6, height=1}

	bmps = {}
	for i=1,21 do
		bmps[i] = load_bitmap("misc_graphics/stonecrater_hole/" .. i .. ".png")
	end

	if (not milestone_is_complete(MILESTONE_NAME)) then
		frame = 1
		large_boulder = add_entity("large_boulder", 1, 12.5*TILE_SIZE, 10.5*TILE_SIZE)
		start_scene = true
	else
		add_polygon_entity(
			2,
			176, 208,
			158, 191,
			157, 158,
			177, 141,
			221, 139,
			240, 157,
			241, 191,
			224, 208
		)
	end
	
	load_sample("sfx/boing.ogg", false)
	load_sample("sfx/throw.ogg", false)
	load_sample("sfx/ground_cracking.ogg", false)
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	end
end

local player_count = 0

function count_players(tween, id, elapsed)
	player_count = player_count + 1
	return true
end

local crack_count = 0

function switch_tile(layer, x, y)
	local sheet, num, solid = get_tile(layer, x, y)
	set_tile(3, x, y, sheet, num, solid)
end

function switch_tile_layers()
	switch_tile(1, 10, 12)
	switch_tile(1, 14, 12)
	for i=10,14 do
		switch_tile(1, i, 13)
		switch_tile(1, i, 14)
		switch_tile(1, i, 15)
	end
end

local num_fell = 0

function inc_fell(tween, id, elapsed)
	num_fell = num_fell + 1
	return true
end

function hide_entity(tween, id, elapsed)
	set_entity_visible(tween.entity, false)
	return true
end

function drop(ent)
	local x, y = get_entity_position(ent)

	play_sample("sfx/throw.ogg", 1, 0, 1)

	local t = create_direct_move_tween(ent, x, 14.5*TILE_SIZE, 250)
	append_tween(t, { run = hide_entity, entity = ent })
	append_tween(t, { run = inc_fell })
	new_tween(t)
end

function logic()
	if (to_stonecrater5:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater5", DIR_S, 24*TILE_SIZE, 3.5*TILE_SIZE)
	end
	
	if (not (next_area == nil)) then
		next_area = next_area + 1
		if (next_area == 10) then
			set_milestone_complete(MILESTONE_NAME, true)
			next_player_layer = 2
			change_areas("caverns1", DIR_S, 27.5*TILE_SIZE, 10.5*TILE_SIZE)
		end
	elseif (num_fell == 4 and not fell5) then
		next_area = 1
		fell5 = true
	elseif (num_fell == 3 and not fell4) then
		fell4 = true
		drop(bisou)
	elseif (num_fell == 2 and not fell3) then
		fell3 = true
		drop(egbert)
	elseif (num_fell == 1 and not fell2) then
		fell2 = true
		drop(frogbert)
	elseif (finished_cracking_ground and not fell1) then
		fell1 = true
		drop(large_boulder)
	elseif (crack_ground and not finished_cracking_ground) then
		crack_count = crack_count + 1
		if (crack_count == 4) then
			crack_count = 0
			frame = frame + 1
			if (frame == 5) then
				play_sample("sfx/boing.ogg", 1, 0, 1)
				set_entity_input_disabled(0, true)
				set_entity_animation(egbert, "surprised")
				set_entity_animation(frogbert, "surprised")
				set_entity_animation(bisou, "surprised")
			elseif (frame == 21) then
				finished_cracking_ground = true
				switch_tile_layers()
			end
		end
	elseif (player_count == 3 and not talked) then
		talked = true

		set_entity_right(frogbert, true)
		set_entity_right(bisou, false)

		set_character_role(egbert, "")
		set_character_role(frogbert, "")
		set_character_role(bisou, "")

		simple_speak{
			true,
			"SC5_FROGBERT_1", "idle", frogbert,
			"SC5_BISOU_1", "idle", bisou
		}

		crack_ground = true

		play_sample("sfx/ground_cracking.ogg", 1, 0, 1)
	elseif (start_scene and not scene_started) then
		scene_started = true

		get_players()

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		player_count = 0

		local t

		t = create_astar_tween(egbert, 12.5*TILE_SIZE, 12*TILE_SIZE, true)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(frogbert, 11.5*TILE_SIZE, 12*TILE_SIZE, true)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(bisou, 13.5*TILE_SIZE, 12*TILE_SIZE, true)
		append_tween(t, { run = count_players })
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
	for i=1,20 do
		destroy_bitmap("misc_graphics/stonecrater_hole/" .. i .. ".png")
	end
	destroy_sample("sfx/boing.ogg")
	destroy_sample("sfx/throw.ogg")
	destroy_sample("sfx/ground_cracking.ogg")
end

function mid_draw_layer(layer)
	if (not milestone_is_complete(MILESTONE_NAME)) then
		if (layer == 1) then
			local top_x, top_y = get_area_top()
			draw_bitmap(bmps[frame], 9*TILE_SIZE-top_x, 8*TILE_SIZE-top_y, 0)
		end
	end
end

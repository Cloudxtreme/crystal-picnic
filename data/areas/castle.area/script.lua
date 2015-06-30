local MAX_SPEED = 15
local speed = MAX_SPEED
local draw_done = false
local draw_hole = false
local hole_x
local hole_y
local BIRD_MILESTONE_NAME = "castle_bird"
local frogbert_intro_start_x = 652
local frogbert_intro_start_y = 1265
local prev_video_frame = 0

local add_to_layer_two_HACK = {
	982, 1222+16,
	996, 1231+16,
	998, 1248+16,
	983, 1258+16,
	967, 1246+16,
	966, 1234+16,
	-1, -1
}

function make_hole_solid()
	for i=1,#add_to_layer_two_HACK do
		outline1[#outline1+1] = add_to_layer_two_HACK[i]
	end
end

function start()
	local x = frogbert_intro_start_x
	local y = frogbert_intro_start_y

	hole_x = x + 270 + 60 - 38/2
	hole_y = (y + 1230) / 2 - 38/2

	if (milestone_is_complete(BIRD_MILESTONE_NAME)) then
		play_music("music/castle.mid");
		make_hole_solid()
		add_floating_image(
			"hole_in_grass.png",
			2,
			hole_x, hole_y,
			true,
			false
		)
		draw_hole = true
		draw_done = true
	end

	-- Must be called after make_hole_solid
	process_outline()

	add_wall_group(3, 12, 22, 1, 7, 0)
	add_wall_group(3, 13, 28, 1, 14, 0)
	
	add_wall_group(3, 59, 22, 1, 7, 0)
	add_wall_group(3, 58, 28, 1, 14, 0)
	
	-- center tower
	add_wall_group(3, 33, 19, 1, 12, 0)
	add_wall_group(3, 38, 19, 1, 12, 0)
	
	add_wall_group(3, 3, 44, 1, 6, 0)
	add_wall_group(3, 68, 44, 1, 6, 0)

	------
	
	add_wall_group(2, 28, 86, 1, 4, 0)
	add_wall_group(2, 43, 86, 1, 4, 0)
	add_wall_group(2, 9, 69, 1, 5, 0)
	add_wall_group(2, 62, 69, 1, 5, 0)

	-- bridges

	add_tile_group(2, 544, 1048, 62, 6, 0,
		34, 64,
		35, 64,
		36, 64,
		37, 64,
		34, 65,
		35, 65,
		36, 65,
		37, 65
	)

	add_tile_group(2, 288, 1160, 44, 6, 0,
		18, 71,
		19, 71,
		20, 71,
		18, 72,
		19, 72,
		20, 72
	)

	add_tile_group(2, 818, 1160, 44, 6, 0,
		51, 71,
		52, 71,
		53, 71,
		51, 72,
		52, 72,
		53, 72
	)

	-- bushes

	add_wall_group(2, 11, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 56, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 56, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 50, 56, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 55, 56, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 59, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 61, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 75, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 77, 1, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 60, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 62, 75, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 63, 77, 1, 2, TILE_GROUP_BUSHES)

	courtyard_entrance = Active_Block:new{x=34, y=54, width=4, height=2}

	if (not milestone_is_complete(BIRD_MILESTONE_NAME)) then
		set_entity_input_disabled(0, true)
		set_move_entity_cameras_while_input_disabled(0, false)
		set_entity_visible(0, false)
		set_entity_position(0, frogbert_intro_start_x, frogbert_intro_start_y)
		
		set_entity_visible(1, false)

		set_area_swiping_in(false)

		hold_milestones(true)

		frogbert = add_npc("frogbert", 2, x, y)
		set_entity_direction(frogbert, DIR_E)
		push_entity_to_back(frogbert)
		set_entity_solid_with_entities(frogbert, false)

		egbert = add_npc("egbert", 2, x + 50, 1240)
		set_entity_direction(egbert, DIR_W)
		set_entity_solid_with_entities(egbert, false)

		grass_bmp = load_bitmap("misc_graphics/grass_hole_patch.png")

		local w, h = get_screen_size()
		area_offset = -(y-h/2-40)
		video_offset = 0
		set_camera_offset(0, area_offset)

		load_sample("sfx/bomb.ogg", false)
		load_sample("sfx/bird_tweet.ogg", false)
		load_sample("sfx/bird_wing_flap.ogg", false)
		load_sample("sfx/boing.ogg", false)
		load_sample("sfx/rumble.ogg", true)
		rumbling_loaded = true

		vp = load_video("videos/bird")
		play_music("music/castle.mid");
		start_video(vp)
	else
		pig1 = add_npc("pig_guard", 6, 303, 406)
		set_character_role(pig1, "wander", 96, 0.25, 1.0)
		
		pig2 = add_npc("pig_guard", 6, 848, 407)
		set_character_role(pig2, "wander", 96, 0.25, 1.0)

		pig3 = add_npc("pig_guard", 3, 575, 542)
		set_character_role(pig3, "astar")
		local t = create_astar_tween(pig3, 448, 542)
		append_tween(t, create_character_role_change_tween(pig3, "null"))
		append_tween(t, create_change_direction_tween(pig3, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig3, "astar"))
		append_tween(t, create_astar_tween(pig3, 702, 542))
		append_tween(t, create_character_role_change_tween(pig3, "null"))
		append_tween(t, create_change_direction_tween(pig3, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig3, "astar"))
		append_tween(t, t)
		new_tween(t)

		pig4 = add_npc("pig_guard", 3, 247, 1327)
		set_character_role(pig4, "astar")
		local t = create_astar_tween(pig4, 102, 1327)
		append_tween(t, create_character_role_change_tween(pig4, "null"))
		append_tween(t, create_change_direction_tween(pig4, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig4, "astar"))
		append_tween(t, create_astar_tween(pig4, 408, 1327))
		append_tween(t, create_character_role_change_tween(pig4, "null"))
		append_tween(t, create_change_direction_tween(pig4, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig4, "astar"))
		append_tween(t, t)
		new_tween(t)

		pig5 = add_npc("pig_guard", 3, 896, 1327)
		set_character_role(pig5, "astar")
		local t = create_astar_tween(pig5, 1057, 1327)
		append_tween(t, create_character_role_change_tween(pig5, "null"))
		append_tween(t, create_change_direction_tween(pig5, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig5, "astar"))
		append_tween(t, create_astar_tween(pig5, 751, 1327))
		append_tween(t, create_character_role_change_tween(pig5, "null"))
		append_tween(t, create_change_direction_tween(pig5, DIR_S))
		append_tween(t, create_idle_tween(5))
		append_tween(t, create_character_role_change_tween(pig5, "astar"))
		append_tween(t, t)
		new_tween(t)
	end

	add_ladder(3, 11*TILE_SIZE, 46*TILE_SIZE+10, 2*TILE_SIZE, 3*TILE_SIZE-20)
	add_ladder(3, 59*TILE_SIZE, 46*TILE_SIZE+10, 2*TILE_SIZE, 3*TILE_SIZE-20)
	add_ladder(3, 27*TILE_SIZE, 36*TILE_SIZE+10, 2*TILE_SIZE, 7*TILE_SIZE-20)
	add_ladder(3, 43*TILE_SIZE, 36*TILE_SIZE+10, 2*TILE_SIZE, 7*TILE_SIZE-20)

	south_exit = Active_Block:new{x=27, y=90, width=18, height=1}

	-- Entrances going to main hall, left to right
	to_hall1 = Active_Block:new{x=15, y=40, width=2, height=2}
	to_hall2 = Active_Block:new{x=18, y=47, width=2, height=2}
	to_hall3 = Active_Block:new{x=35, y=40, width=2, height=2}
	to_hall4 = Active_Block:new{x=52, y=47, width=2, height=2}
	to_hall5 = Active_Block:new{x=55, y=40, width=2, height=2}

	in_topleft = Active_Block:new{x=19, y=21, width=2, height=2, layer=6}
	in_topright = Active_Block:new{x=51, y=21, width=2, height=2, layer=6}

	to_tower = Active_Block:new{x=35, y=29, width=2, height=2}
end

function bomb_tween(tween, id, elapsed)
	add_bomb_puff(2, tween.x + rand(11) - 5, tween.y + rand(11) - 5, 15)
	play_sample("sfx/bomb.ogg", 1, 0, 1)
	if (tween.add_hole == true) then
		add_floating_image(
			"hole_in_grass.png",
			2,
			hole_x, hole_y,
			true,
			false
		)
	end

	return true
end

function delete_ant_tween(tween, id, elapsed)
	remove_entity(tween.ant)
	return true
end

function end_frogbert(tween, id, elapsed)
	remove_entity(frogbert)
	
	remove_entity(egbert)
	make_hole_solid()
	-- Must be called after make_hole_solid (reset on > 1 process)
	reset_outline();
	process_outline()

	destroy_sample("sfx/bomb.ogg")
	destroy_sample("sfx/bird_tweet.ogg")
	destroy_sample("sfx/bird_wing_flap.ogg")
	destroy_sample("sfx/boing.ogg")

	set_milestone_complete(BIRD_MILESTONE_NAME, true)

	next_player_layer = 4
	change_areas("crystal_castle_entrance", DIR_S, 0, 0)

	return true
end

function do_leave(tween, id, elapsed)
	set_character_role(egbert, "follow", frogbert)

	local t = create_astar_tween(frogbert, 576, 1450, false)
	t.next_tween = {}
	t.next_tween.run = end_frogbert
	t.next_tween.started = false
	new_tween(t)

	return true
end

function setup_leave(tween, id, elapsed)
	set_character_role(egbert, "astar")
	set_character_role(frogbert, "astar")

	do_final_intro_speech = true

	return true
end

function new_ant(tween, id, elapsed)
	local ant = add_npc("ant-crystal", 2, tween.x, tween.y)
	set_entity_right(ant, false)
	set_entity_solid_with_entities(ant, false)
	set_character_role(ant, "astar")
	local t = create_astar_tween(ant, 576, 1450, false)
	t.next_tween = { run = delete_ant_tween, started = false, ant = ant }
	new_tween(t)
	
	if (tween.last == true) then
		local t = {}
		t.run = setup_leave
		t.started = false
		new_tween(t)
	end

	return true
end

function destroy_rumble(tween, id, elapsed)
	destroy_sample("sfx/rumble.ogg")
	rumbling_loaded = false
	return true
end

function ran_to_wall(tween, id, elapsed)
	local x, y = get_entity_position(frogbert)
	local ex, ey = get_entity_position(egbert)

	local puffx = x + 60
	local puffy = (y + ey) / 2

	local t = create_idle_tween(0)
	local tween = t
	for i=1,6 do
		t.next_tween = {}
		t.next_tween.run = bomb_tween
		t.next_tween.started = false
		t.next_tween.x = puffx
		t.next_tween.y = puffy
		if (i == 3) then
			t.next_tween.add_hole = true
		end
		t = t.next_tween
		t.next_tween = create_idle_tween(0.05)
		t = t.next_tween
	end

	new_tween(tween)
	
	t = create_idle_tween(2)
	t.next_tween = {}
	t.next_tween.run = destroy_rumble
	t.next_tween.started = false
	new_tween(t)

	t = create_idle_tween(2)
	tween = t
	for i=1,5 do
		t.next_tween = {}
		t.next_tween.run = new_ant
		t.next_tween.started = false
		t.next_tween.x = hole_x + 38/2
		t.next_tween.y = hole_y + 38/1.5
		if (i == 5) then
			t.next_tween.last = true
		end
		t = t.next_tween
		t.next_tween = create_idle_tween(1)
		t = t.next_tween
	end

	new_tween(tween)

	draw_hole = true

	return true
end

function look_surprised(tween, id, elapsed)
	play_sample("sfx/boing.ogg", 1, 0, 1)
	set_entity_animation(tween.ent, "surprised")
	return true
end

function think_frogbert_think(tween, id, elapsed)
	set_character_role(frogbert, "none")
	set_entity_animation(frogbert, "thinking")
	return true
end

function video_sfx_callback(frame)
	if (frame == 47 or frame == 90) then
		play_sample("sfx/bird_tweet.ogg", 1, 0, 1)
	elseif (frame == 12 or frame == 16 or frame == 20 or frame == 24 or frame == 27 or frame == 30 or frame == 42 or frame == 69 or frame == 73 or frame == 77 or frame == 81 or frame == 85 or frame == 88 or frame == 105 or frame == 109 or frame == 115) then
		play_sample("sfx/bird_wing_flap.ogg", 1, 0, 1)
	end
end

function logic()
	if (not milestone_is_complete(BIRD_MILESTONE_NAME)) then
		if (do_final_intro_speech == true) then
			do_final_intro_speech = false
			simple_speak{
				true,
				"SCN1_EGBERT_5", "", egbert,
				"SCN1_FROGBERT_5", "", frogbert
			}

			local fx, fy = get_entity_position(frogbert)
			local t = create_astar_tween(egbert, fx, fy, false)
			t.next_tween = { run = do_leave, started = false }
			new_tween(t)
			
			return
		end
		if (draw_done and not (area_offset == 0)) then
			local left = -area_offset
			if (left < 300) then
				speed = (left / 300) * MAX_SPEED
				if (speed <= 1) then
					speed = 1
				end
			end
			video_offset = video_offset - speed
			area_offset = area_offset + speed
			if (area_offset >= 0) then
				area_offset = 0
				destroy_video(vp)

				simple_speak{
					true,
					"SCN1_EGBERT_1", "", egbert,
					"SCN1_FROGBERT_1", "", frogbert,
					"SCN1_EGBERT_2", "facepalm", egbert,
					"SCN1_FROGBERT_2", "snooty", frogbert,
					"SCN1_EGBERT_3", "", egbert,
					"SCN1_FROGBERT_3", "point", frogbert,
					"SCN1_EGBERT_4", "snooty-anim", egbert,
					"SCN1_FROGBERT_4", "", frogbert
				}

				play_sample("sfx/rumble.ogg", 1, 0, 1)
				shake(0, 7.5, 5)
			
				local ex, ey = get_entity_position(egbert)
				local fx, fy = get_entity_position(frogbert)

				fx = fx + 270

				local tween = create_idle_tween(0.5)
				tween.next_tween = {}
				tween.next_tween.run = look_surprised
				tween.next_tween.ent = frogbert
				tween.next_tween.started = false
				tween.next_tween.next_tween = create_idle_tween(1.5)
				tween.next_tween.next_tween.next_tween = create_character_role_change_tween(frogbert, "astar")
				tween.next_tween.next_tween.next_tween.next_tween = create_astar_tween(frogbert, fx, fy, false)
				tween.next_tween.next_tween.next_tween.next_tween.next_tween = {
					run = ran_to_wall, started = false
				}
				tween.next_tween.next_tween.next_tween.next_tween.next_tween.next_tween = {
					run=think_frogbert_think, started=false
				}
				new_tween(tween)
				
				tween = create_idle_tween(0.5)
				tween.next_tween = {}
				tween.next_tween.run = look_surprised
				tween.next_tween.ent = egbert
				tween.next_tween.started = false
				tween.next_tween.next_tween = create_idle_tween(1.5)
				tween.next_tween.next_tween.next_tween = create_character_role_change_tween(egbert, "astar")
				tween.next_tween.next_tween.next_tween.next_tween = create_astar_tween(egbert, fx, ey, false)
				new_tween(tween)
			else
				set_video_offset(vp, 0, video_offset)
			end
			set_camera_offset(0, area_offset)
		elseif (draw_done) then
			set_camera_to(frogbert)
		else
			update_video(vp)
			prev_video_frame = add_video_sfx(vp, video_sfx_callback, prev_video_frame)
		end
	end

	if (milestone_is_complete(BIRD_MILESTONE_NAME)) then
		if (courtyard_entrance:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_main_entrance", DIR_N, 192, 320)
		elseif (south_exit:entity_is_colliding(0)) then
			next_player_layer = 4
			change_areas("crystal_castle_entrance", DIR_S, 231, 39)
		elseif (to_hall1:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_hall", DIR_N, 103, 143)
		elseif (to_hall2:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_hall", DIR_N, 311, 686)
		elseif (to_hall3:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_hall", DIR_N, 967, 799)
		elseif (to_hall4:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_hall", DIR_N, 1704, 686)
		elseif (to_hall5:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_hall", DIR_N, 1911, 143)
		elseif (in_topleft:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_knight_left2", DIR_N, 103, 198)
		elseif (in_topright:entity_is_colliding(0)) then
			next_player_layer = 3
			change_areas("castle_knight_right2", DIR_N, 103, 198)
		elseif (to_tower:entity_is_colliding(0)) then
			next_player_layer = 2
			change_areas("castle_tower1", DIR_N, 127, 192)
		end
	end
end

function post_draw_layer(layer)
	if (layer == 6) then
		if (not draw_hole) then
			draw_bitmap(grass_bmp, hole_x, hole_y, 0)
		end
		if (not draw_done) then
			draw_done = draw_video(vp)
		end
	end
end

function activate(activator, activated)
	if (activated == pig1) then
		if (done_pyou1()) then
			speak(false, false, true, t("PIG_TOWER_1_2"), "", pig1, pig1)
		else
			speak(false, false, true, t("PIG_TOWER_1"), "", pig1, pig1)
		end
	elseif (activated == pig2) then
		speak(false, false, true, t("PIG_TOWER_2"), "", pig2, pig2)
	elseif (activated == pig3) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_UPPER_2"), "", pig3, pig3)
		else
			speak(false, false, true, t("PIG_UPPER_1"), "", pig3, pig3)
		end
	elseif (activated == pig4) then
		speak(false, false, true, t("PIG_BATTLE_1"), "", pig4, pig4)
	elseif (activated == pig5) then
		speak(false, false, true, t("PIG_BATTLE_2"), "", pig5, pig5)
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
	if (rumbling_loaded) then
		destroy_sample("sfx/rumble.ogg")
	end
	destroy_bitmap("misc_graphics/grass_hole_patch.png")
end

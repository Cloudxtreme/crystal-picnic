local jumping = false
local MILESTONE_NAME = "stone_crater_amaysa_scene"
local MILESTONE2_NAME = "stone_crater_boulder_pushed"
local AMAYSA_FLY_SPEED = 400

local BOULDER_MILESTONE = "sc_boulder"
local BOULDER_SIZE = 16

local boulder = {}
local BOULDER_START_X = {}
local BOULDER_START_Y = {}
local BOULDER_X = {}
local BOULDER_Y = {}
local BOULDER_LAYERS = { 3, 4, 2, 5, 5 }
	
BOULDER_START_X[1] = 12*TILE_SIZE+TILE_SIZE/2
BOULDER_START_Y[1] = 35*TILE_SIZE+TILE_SIZE/2
BOULDER_START_X[2] = 5*TILE_SIZE
BOULDER_START_Y[2] = 57*TILE_SIZE
BOULDER_START_X[3] = 33*TILE_SIZE
BOULDER_START_Y[3] = 30*TILE_SIZE+TILE_SIZE/2
BOULDER_START_X[4] = 29*TILE_SIZE
BOULDER_START_Y[4] = 58*TILE_SIZE

BOULDER_X[1] = 23*TILE_SIZE - TILE_SIZE/4
BOULDER_Y[1] = 28*TILE_SIZE + TILE_SIZE/2
BOULDER_X[2] = 23*TILE_SIZE - TILE_SIZE/4
BOULDER_Y[2] = 40*TILE_SIZE + TILE_SIZE/2
BOULDER_X[3] = 24*TILE_SIZE + TILE_SIZE/4
BOULDER_Y[3] = 28*TILE_SIZE + TILE_SIZE/2
BOULDER_X[4] = 24*TILE_SIZE + TILE_SIZE/4
BOULDER_Y[4] = 40*TILE_SIZE + TILE_SIZE/2

local bigant_step = 1

local PUSH_SOUND_TIME = 0.1
local pushing_boulder = false
local stop_push_sound_time = get_time() + PUSH_SOUND_TIME

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

function start()
	play_music("music/stonecrater.mid")

	process_outline()

	add_wall_group(2, 9, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 14, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 17, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 10, 16, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 16, 8, 1, 2, 0)
	add_wall_group(2, 16, 14, 1, 2, 0)
	add_wall_group(2, 35, 7, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 8, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 37, 9, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 10, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 40, 9, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 5, 2, 2, 0)

	add_wall_group(3, 4, 21, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 5, 20, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 3, 22, 3, 2, TILE_GROUP_BUSHES)

	add_wall_group(5, 28, 53, 3, 2, TILE_GROUP_BUSHES)
	
	jump_block1 = Active_Block:new{x=17, y=23, width=2, height=2}
	local jump_entity1 = add_entity("jump_bubble", 2, 18*TILE_SIZE, 24*TILE_SIZE)
	set_show_entity_shadow(jump_entity1, true)
	jump_block2 = Active_Block:new{x=12, y=23, width=2, height=2}
	local jump_entity2 = add_entity("jump_bubble", 3, 13*TILE_SIZE, 24*TILE_SIZE)
	set_show_entity_shadow(jump_entity2, true)
	
	jump_block3 = Active_Block:new{x=9, y=40, width=3, height=2}
	local jump_entity3 = add_entity("jump_bubble", 3, 10*TILE_SIZE, 41*TILE_SIZE)
	set_show_entity_shadow(jump_entity3, true)
	jump_block4 = Active_Block:new{x=9, y=44, width=3, height=2}
	local jump_entity4 = add_entity("jump_bubble", 4, 10*TILE_SIZE, 45*TILE_SIZE)
	set_show_entity_shadow(jump_entity4, true)

	jump_block5 = Active_Block:new{x=25, y=33, width=2, height=2}
	local jump_entity5 = add_entity("jump_bubble", 2, 26*TILE_SIZE, 34*TILE_SIZE)
	set_show_entity_shadow(jump_entity5, true)
	jump_block6 = Active_Block:new{x=25, y=37, width=2, height=2}
	local jump_entity6 = add_entity("jump_bubble", 5, 26*TILE_SIZE, 38*TILE_SIZE)
	set_show_entity_shadow(jump_entity6, true)

	if (not milestone_is_complete(MILESTONE_NAME)) then
		hide_all_rocks()

		start_player_name = get_entity_name(get_player_id(0))

		while (not (get_entity_name(get_player_id(0)) == "egbert")) do
			switch_characters(false)
		end

		add_blocking_polygon()
		boulder[5] = add_entity("large_boulder", 5, 23*TILE_SIZE+TILE_SIZE/2, 49*TILE_SIZE)
		set_entity_solid_with_area(boulder[5], false)
		set_entities_slide_on_entity(boulder[5], false)
	elseif (milestone_is_complete(MILESTONE2_NAME)) then
		hide_some_rocks()
	else
		add_blocking_polygon()
		boulder[5] = add_entity("large_boulder", 5, 23*TILE_SIZE+TILE_SIZE/2, 49*TILE_SIZE)
		set_entity_solid_with_area(boulder[5], false)
		set_entities_slide_on_entity(boulder[5], false)
	end
	
	load_sample("sfx/rumble.ogg", true)
	load_sample("sfx/bomb.ogg", false)
	load_sample("sfx/amaysa_fly_r_l.ogg", false)
	load_sample("sfx/amaysa.ogg", false)
	load_sample("sfx/throw.ogg", false)
	load_sample("sfx/push_small_boulder.ogg", true)
	load_sample("sfx/push_large_boulder.ogg", true)
	load_sample("sfx/boulder_thud.ogg", false)
	load_sample("sfx/giant_footsteps.ogg", false)
	load_sample("sfx/dizzy.ogg", false)
	load_sample("sfx/boulder_roll.ogg", true)

	for i=1,4 do
		if (not milestone_is_complete(BOULDER_MILESTONE .. i)) then
			boulder[i] = add_entity("small_boulder", BOULDER_LAYERS[i], BOULDER_START_X[i], BOULDER_START_Y[i])
			set_entity_solid_with_area(boulder[i], false)
			set_entities_slide_on_entity(boulder[i], false)
		end
	end

	boulder_img = load_bitmap("misc_graphics/small_boulder.png")

	to_stonecrater4 = Active_Block:new{x=0, y=12, width=1, height=4}
	to_stonecrater6 = Active_Block:new{x=20, y=0, width=7, height=1}
	
	add_ladder(5, 22*TILE_SIZE, 54*TILE_SIZE+10, 2*TILE_SIZE, 3*TILE_SIZE-20)
end

local rocks = {}

function hide_all_rocks()
	local count = 1
	for y=1,7 do
		for x=21,26 do
			rocks[count] = {get_tile(1, x, y)}
			count = count + 1
			set_tile(1, x, y, -1, -1, false)
		end
	end
	for y=1,7 do
		for x=21,26 do
			rocks[count] = {get_tile(2, x, y)}
			count = count + 1
			if (x >= 21 and x <= 22 and y >= 5 and y <= 6) then
				set_tile(2, x, y, 0, 0, false)
			else
				set_tile(2, x, y, -1, -1, false)
			end
		end
	end
end

function show_all_rocks()
	local count = 1
	for y=1,7 do
		for x=21,26 do
			set_tile(1, x, y, rocks[count][1], rocks[count][2], rocks[count][3])
			count = count + 1
		end
	end
	for y=1,7 do
		for x=21,26 do
			set_tile(2, x, y, rocks[count][1], rocks[count][2], rocks[count][3])
			count = count + 1
		end
	end
end

function hide_some_rocks()
	for y=2,7 do
		for x=23,24 do
			set_tile(1, x, y, -1, -1, false)
		end
	end
	for y=2,4 do
		for x=21,26 do
			set_tile(2, x, y, -1, -1, false)
		end
	end
	for y=5,5 do
		for x=23,26 do
			set_tile(2, x, y, -1, -1, false)
		end
	end
end

function add_blocking_polygon()
	blocking_poly = add_polygon_entity(2, 23*TILE_SIZE, 6*TILE_SIZE, 25*TILE_SIZE, 6*TILE_SIZE, 25*TILE_SIZE, 8*TILE_SIZE, 23*TILE_SIZE, 8*TILE_SIZE)
end

function activate(activator, activated)
end

function add_rock_poof(x, y)
	local pgid = add_particle_group(
		"anim",
		5,
		PARTICLE_HURT_NONE,
		"rock_poof/1",
		"rock_poof/2",
		"rock_poof/3",
		"rock_poof/4",
		"rock_poof/5",
		"rock_poof/6",
		"rock_poof/7"
	)

	local pid = add_particle(
		pgid,
		1,
		1,
		1, 1, 1, 1,
		0,
		0,
		true,
		false
	)

	set_particle_position(pid, x, y)
	set_particle_blackboard(pid, 0, 7)
	set_particle_blackboard(pid, 1, 3)
	set_particle_blackboard(pid, 2, 0)
end

local first_run = true
local player_count = 0

function count_players(tween, id, elapsed)
	player_count = player_count + 1
	return true
end

function end_egbert_step(tween, id, elapsed)
	egbert_stepped = true
	return true
end

function end_frogbert_step(tween, id, elapsed)
	frogbert_stepped = true
	return true
end

function bigant_done_walking(tween, id, elapsed)
	bigant_is_done_walking = true
	return true
end

function amaysa_jump_off(tween, id, elapsed)
	amaysa_jumped_off = true
	return true
end

function dramatic_pause(tween, id, elapsed)
	dramatic_paused = true
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

function five_bombs()
	for i=1,5 do
		local x = 384
		local r = rand(128) - 64
		x = x + r
		local y = 80
		r = rand(96) - 48
		y = y + r
		add_bomb_puff(2, x, y, 15)
		play_sample("sfx/bomb.ogg", 1, 0, 1)
	end
end

function one_bomb(tween, id, elapsed)
	add_bomb_puff(tween.layer, tween.x, tween.y, 15)
	play_sample("sfx/bomb.ogg", 1, 0, 1)
	return true
end

function remove_bigant(tween, id, elapsed)
	remove_entity(bigant)
	bigant = nil
	bigant_removed = true

	five_bombs()

	return true
end

function more_bombs_tween(tween, id, elapsed)
	five_bombs()
	return true
end

function block_path(tween, id, elapsed)
	show_all_rocks()
	return true
end

function amaysa_speak(tween, id, elapsed)
	amaysa_spoke = true
	return true
end

function remove_amaysa(tween, id, elapsed)
	remove_entity(amaysa)
	removed_amaysa = true
	return true
end

function bisou_check_rocks(tween, id, elapsed)
	bisou_checked_rocks = true
	return true
end

local falling_rocks = {}

function remove_falling_rocks(tween, id, elapsed)
	for i=1,#falling_rocks do
		remove_entity(falling_rocks[i])
	end
	return true
end

function show_shadow(tween, id, elapsed)
	set_show_entity_shadow(tween.entity, true)
	return true
end

function frogbert_off_boulder(tween, id, elapsed)
	frogbert_popped_off = true
	return true
end

function boulder_tween1(tween, id, elapsed)
	boulder_tween1ed = true
	return true
end

function open_path(tween, id, elapsed)
	path_open = true
	return true
end

function remove_boulder5(tween, id, elapsed)
	remove_entity(boulder[5])
	boulder[5] = nil
	stop_sample("sfx/boulder_roll.ogg")
	return true
end

function remove_poly(tween, id, elapsed)
	remove_entity(blocking_poly)
	hide_some_rocks()
	pan_to_frogbert = true
	return true
end

function frogbert_rejoin(tween, id, elapsed)
	rejoin_frogbert = true
	return true
end

function frogbert_jump(tween, id, elapsed)
	jump_frogbert = true
	return true
end

function final_tween(tween, id, elapsed)
	finale = true
	return true
end

function logic()
	if (to_stonecrater4:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater4", DIR_W, 32.5*TILE_SIZE, 11.5*TILE_SIZE)
	elseif (to_stonecrater6:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater6", DIR_N, 12*TILE_SIZE, 19.5*TILE_SIZE)
	end

	if (not milestone_is_complete(MILESTONE_NAME) and first_run) then
		first_run = false

		get_players()

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		local t = create_idle_tween(0.2)
		append_tween(t, create_astar_tween(egbert, 384, 220, true))
		append_tween(t, create_change_direction_tween(egbert, DIR_N))
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_idle_tween(0.4)
		append_tween(t, create_astar_tween(frogbert, 352, 220, true))
		append_tween(t, create_change_direction_tween(frogbert, DIR_N))
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_astar_tween(bisou, 416, 220, true)
		append_tween(t, create_change_direction_tween(bisou, DIR_N))
		append_tween(t, { run = count_players })
		new_tween(t)
	end


	if (waiting_for_join and player_count == 2) then
		waiting_for_join = false

		set_character_role(egbert, "")
		set_character_role(frogbert, "follow", egbert)
		set_character_role(bisou, "follow", frogbert)
			
		while (not (get_entity_name(get_player_id(0)) == start_player_name)) do
			switch_characters(false)
		end

		local t = create_reset_camera_tween(0.0, 50)
		new_tween(t)
		
		set_milestone_complete(MILESTONE_NAME, true)
	elseif (not finished_bisou_checked_rocks and bisou_checked_rocks) then
		finished_bisou_checked_rocks = true

		set_entity_animation(bisou, "idle-down")
		set_entity_right(egbert, false)

		simple_speak{
			true,
			"SCSCN2_BISOU_4", "", bisou,
			"SCSCN2_FROGBERT_5", "", frogbert,
			"SCSCN2_EGBERT_2", "", egbert,
			"SCSCN2_FROGBERT_6", "", frogbert,
			"SCSCN2_EGBERT_3", "idle", egbert
		}

		local x, y = get_entity_position(egbert)
		player_count = 0

		local t = create_astar_tween(frogbert, x, y, false)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(bisou, x, y, true)
		append_tween(t, { run = count_players })
		new_tween(t)

		waiting_for_join = true
	elseif (not finished_remove_amaysa and removed_amaysa) then
		finished_remove_amaysa = true

		set_entity_right(frogbert, true)
		set_entity_right(bisou, false)
		set_entity_animation(frogbert, "idle")
		set_entity_animation(bisou, "idle")

		simple_speak{
			true,
			"SCSCN2_BISOU_3", "", bisou,
			"SCSCN2_FROGBERT_4", "", frogbert
		}
		
		set_entity_animation(frogbert, "idle-up")

		local x, y = get_entity_position(bisou)
		local t = create_astar_tween(bisou, x, y-50, false)
		append_tween(t, { run = bisou_check_rocks })
		new_tween(t)
	elseif (not finished_amaysa_speak and amaysa_spoke) then
		finished_amaysa_speak = true

		set_entity_right(bisou, false)

		speak_force_bottom(false, false, true, t("SCSCN2_AMAYSA_1"), "", amaysa)

		simple_speak{
			true,
			"SCSCN2_FROGBERT_1", "hand-gesture", frogbert,
			"SCSCN2_BISOU_1", "idle", bisou
		}

		speak_force_bottom(false, false, true, t("SCSCN2_AMAYSA_2"), "", amaysa)

		simple_speak{
			true,
			"SCSCN2_EGBERT_1", "", egbert
		}

		speak_force_bottom(false, false, true, t("SCSCN2_AMAYSA_3"), "", amaysa)

		simple_speak{
			true,
			"SCSCN2_FROGBERT_2", "", frogbert,
			"SCSCN2_BISOU_2", "", bisou
		}

		speak_force_bottom(false, false, true, t("SCSCN2_AMAYSA_4"), "", amaysa)

		simple_speak{
			true,
			"SCSCN2_FROGBERT_3", "", frogbert
		}

		speak_force_bottom(false, false, true, t("SCSCN2_AMAYSA_5"), "impudence", amaysa)

		set_entity_right(amaysa, false)
		set_entity_animation(amaysa, "left-right")
		local x, y = get_entity_position(amaysa)
		local t = create_direct_move_tween(amaysa, 180, y, AMAYSA_FLY_SPEED)
		append_tween(t, { run = remove_amaysa })
		new_tween(t)

		play_sample("sfx/amaysa_fly_r_l.ogg", 1, 0, 1)
		play_music("music/stonecrater.mid")
	elseif (not finished_amaysa_jumped_off and amaysa_jumped_off) then
		finished_amaysa_jumped_off = true

		set_character_role(bigant, "")

		set_entity_animation(bigant, "jump")

		local t = create_idle_tween(0.5)
		append_tween(t, create_direct_move_xyz_tween(bigant, 0.8, 384, 136, 400))
		append_tween(t, create_direct_move_xyz_tween(bigant, 0.8, 384, 106, 0))
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, create_direct_move_xyz_tween(bigant, 0.8, 384, 56, 400))
		append_tween(t, create_direct_move_xyz_tween(bigant, 0.8, 384, 0, 0))
		append_tween(t, { run = shake_tween, duration = 5, amount = 10, playbig=true })
		new_tween(t)
		
		t = create_idle_tween(2)
		append_tween(t, { run = shake_tween, duration = 1, amount = 10, playbig=true })
		append_tween(t, create_idle_tween(1))
		append_tween(t, create_center_camera_tween(0.0, 384, 150, 60))
		append_tween(t, create_idle_tween(2))
		append_tween(t, { run = remove_bigant })
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, { run = more_bombs_tween })
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, { run = more_bombs_tween })
		append_tween(t, { run = block_path })
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, { run = more_bombs_tween })
		append_tween(t, create_astar_tween(bisou, 416, 210, false))
		append_tween(t, { run = remove_falling_rocks })
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, { run = more_bombs_tween })
		append_tween(t, { run = amaysa_speak })
		new_tween(t)

		for i=1,15 do
			local x = (21+rand(6)) * TILE_SIZE + TILE_SIZE/2
			local y = (1+rand(6)) * TILE_SIZE + TILE_SIZE/2
			local z = rand(50) + 100
			falling_rocks[i] = add_entity("falling_rocks", 2, x, y)
			set_show_entity_shadow(falling_rocks[i], false)
			set_entity_z(falling_rocks[i], z)
			set_entity_animation(falling_rocks[i], "" .. (1+rand(2)))
			local time = 4.5 + 1/rand(4)
			t = create_idle_tween(time)
			append_tween(t, create_play_sample_tween("sfx/throw.ogg"))
			append_tween(t, create_direct_move_xyz_tween(falling_rocks[i], 1/2, x, y, 0))
			new_tween(t)
			t = create_idle_tween(time+0.25)
			append_tween(t, { run = show_shadow, entity = falling_rocks[i]})
			new_tween(t)
		end

	elseif (not finished_bigant_done_walking and bigant_is_done_walking) then
		finished_bigant_done_walking = true

		amaysa_jumped_off = true

		set_show_entity_shadow(amaysa, true)
		set_entity_z(amaysa, 40)
		local x, y = get_entity_position(amaysa)
		y = y + 40
		set_entity_position(amaysa, x, y)
		set_entity_animation_set_prefix(amaysa, "fly-")
		set_entity_animation(amaysa, "idle")

		local t = create_direct_move_tween(amaysa, 350, y+20, AMAYSA_FLY_SPEED)
		append_tween(t, { run = amaysa_jump_off })
		new_tween(t)
	elseif (not finished_frogbert_step and frogbert_stepped) then
		finished_frogbert_step = true

		set_entity_right(frogbert, true)

		simple_speak{
			true,
			"SCSCN_FROGBERT_4", "", frogbert,
			"SCSCN_FROGBERT_5", "", frogbert,
			"SCSCN_FROGBERT_6", "idle", frogbert
		}

		play_music("music/boss_encounter.mid")

		new_tween({ run = shake_tween, duration = 10, amount = 5 })

		local t = create_center_camera_tween(0.0, 384, 150, 50)
		new_tween(t)

		bigant_added = true

		bigant = add_npc("fieldantboss", 2, 88, 222)
		set_character_role(bigant, "astar")

		t = create_astar_tween(bigant, 480, 176, false)
		append_tween(t, create_astar_tween(bigant, 272, 176, false))
		append_tween(t, create_center_camera_tween(0.0, 384, 130, 50))
		append_tween(t, create_astar_tween(bigant, 384, 176, false))
		append_tween(t, { run = bigant_done_walking })
		new_tween(t)

		amaysa = add_entity("amaysa", 3, 0, 0)
		set_show_entity_shadow(amaysa, false)
		set_entity_animation(amaysa, "riding-ant")
		-- her position is set in logic elsewhere
	elseif (not finished_egbert_step and egbert_stepped) then
		finished_egbert_step = true

		simple_speak{
			true,
			"SCSCN_EGBERT_2", "", egbert,
			"SCSCN_FROGBERT_2", "", frogbert,
			"SCSCN_EGBERT_3", "", egbert,
			"SCSCN_FROGBERT_3", "", frogbert,
			"SCSCN_EGBERT_4", "", egbert
		}
			
		local t = create_astar_tween(frogbert, 352, 210, false)
		append_tween(t, { run = end_frogbert_step })
		new_tween(t)
	elseif (not dramatic_pause_finished and dramatic_paused) then
		dramatic_pause_finished = true

		set_entity_direction(frogbert, DIR_E)
		set_entity_direction(bisou, DIR_W)

		simple_speak{
			true,
			"SCSCN_EGBERT_1", "", egbert,
			"SCSCN_BISOU_1", "", bisou,
			"SCSCN_FROGBERT_1", "thinking", frogbert
		}

		new_tween({ run = shake_tween, duration = 1, amount = 5 })

		local t = create_astar_tween(egbert, 384, 210, false)
		append_tween(t, { run = end_egbert_step })
		new_tween(t)

		set_entity_direction(frogbert, DIR_N)
		set_entity_direction(bisou, DIR_N)
	elseif (not first_run and player_count == 3 and not started_talking) then
		started_talking = true

		new_tween({ run = shake_tween, duration = 1, amount = 5 })

		set_entity_direction(egbert, DIR_N)
		set_entity_direction(frogbert, DIR_N)
		set_entity_direction(bisou, DIR_N)

		local t = create_idle_tween(1)
		append_tween(t, { run = dramatic_pause })
		new_tween(t)
	end

	-- NOTE: always needed for jump levels
	if (jumping) then
		local all_done = true
		for i=1,#jump_table do
			if (jump_table[i].done == false) then
				all_done = false
				local x, y = get_entity_position(jump_table[i].tween.entity)
				if (math.abs(x-jump_table[i].x) <= 5 and math.abs(y-jump_table[i].y) <= 5) then
					local t = create_character_role_change_tween(jump_table[i].tween.entity, "")
					append_tween(t, jump_table[i].tween)
					local t2 = create_character_role_change_tween(jump_table[i].tween.entity, "follow")
					t2.role_parameters[1] = jump_table[i].follow
					append_tween(t, t2)
					new_tween(t)
					jump_table[i].done = true
				end
			end
		end
		if (all_done) then
			jumping = false
		end
	end

	if (bigant_added and not amaysa_jumped_off) then
		local x, y = get_entity_position(bigant)
		local right = get_entity_right(bigant)
		local xo
		if (right) then
			xo = -15
		else
			xo = 15
		end
		set_entity_position(amaysa, x+xo, y-65)
		set_entity_right(amaysa, right)
	end

	if (pushing_boulder) then
		if (stop_push_sound_time < get_time()) then
			stop_sample("sfx/push_small_boulder.ogg")
			pushing_boulder = false
		end
	end

	if (bigant) then
		if (get_entity_animation(bigant) == "walk") then
			local frame = get_entity_animation_frame(bigant)
			if (bigant_step == -1) then
				if (frame == 0) then
					bigant_step = 1
				end
			elseif (frame >= bigant_step) then
				play_sample("sfx/giant_footsteps.ogg", 1, 0, 1)
				if (bigant_step == 1) then
					bigant_step = 4
				elseif (bigant_step == 4) then
					bigant_step = -1
				end
			end
		end
	end

	if (totally_finished and player_count == 2 and not final) then
		final = true

		set_entity_input_disabled(0, false)

		set_character_role(get_player_id(1), "follow", 0)
		set_character_role(get_player_id(2), "follow", 1)

		local t = create_reset_camera_tween(0.0, 50)
		new_tween(t)

		set_milestone_complete(MILESTONE2_NAME, true)
	elseif (finale and not totally_finished) then
		totally_finished = true

		set_entity_input_disabled(0, true)

		set_character_role(get_player_id(1), "astar")
		set_character_role(get_player_id(2), "astar")

		player_count = 0

		local t = create_astar_tween(get_player_id(1), p0_x1, p0_y1, false)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(get_player_id(2), p0_x1, p0_y1, false)
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (jump_frogbert and not frogbert_jumped) then
		frogbert_jumped = true
		jump_table = {}
		local tween = create_jump_tween{
			entity = frogbert,
			time = 1,
			height = 32,
			end_x = frogbert_x,
			end_y = frogbert_y,
			end_layer = 5,
			right = true,
			updown = true
		}
		append_tween(tween, { run = final_tween })
		new_tween(tween)
		jumping = true
	elseif (rejoin_frogbert and not frogbert_rejoined) then
		frogbert_rejoined = true

		set_entity_animation(frogbert, "walk-down")

		local t = create_direct_move_tween(frogbert, 23.5*TILE_SIZE, 46*TILE_SIZE, 40)
		append_tween(t, { run = frogbert_jump })
		new_tween(t)

		if (not (frogbert == get_player_id(0))) then
			t = create_center_camera_tween(0.0, 23.5*TILE_SIZE, 46*TILE_SIZE, 40)
			new_tween(t)
		end
	elseif (pan_to_frogbert and not panned_to_frogbert) then
		panned_to_frogbert = true

		local x, y = get_entity_position(frogbert)

		local t = create_center_camera_tween(0.0, x, y, 250)
		append_tween(t, create_play_sample_tween("sfx/dizzy.ogg"))
		append_tween(t, create_idle_tween(3))
		append_tween(t, { run = frogbert_rejoin })
		new_tween(t)
	elseif (path_open and not path_opened) then
		path_opened = true
		local j = 0
		for i=7,3,-1 do
			j = j + 1
			local t = create_idle_tween(0.1*j)
			append_tween(t, { run = one_bomb, layer = 5, x = 24*TILE_SIZE, y=i*TILE_SIZE })
			new_tween(t)
		end

		local t = create_idle_tween(1)
		append_tween(t, { run = remove_poly })
		new_tween(t)
	end

	if (boulder_tween1ed and not boulder_tween1_finished) then
		boulder_tween1_finished = true
		
		play_sample("sfx/boulder_thud.ogg", 1, 0, 1)

		local t = create_direct_move_tween(boulder[5], 23.5*TILE_SIZE, 19*TILE_SIZE, 125)
		append_tween(t, create_play_sample_tween("sfx/boulder_thud.ogg"))
		append_tween(t, create_direct_move_xyz_tween(boulder[5], 0.1, 23.5*TILE_SIZE, 18.5*TILE_SIZE, 20))
		append_tween(t, create_direct_move_xyz_tween(boulder[5], 0.1, 23.5*TILE_SIZE, 18*TILE_SIZE, 0))
		append_tween(t, create_direct_move_tween(boulder[5], 24*TILE_SIZE, 8*TILE_SIZE, 125))
		append_tween(t, { run = open_path })
		append_tween(t, create_direct_move_tween(boulder[5], 24*TILE_SIZE, 0, 125))
		append_tween(t, { run = remove_boulder5 })
		new_tween(t)

		t = create_center_camera_tween(0.0, 23.5*TILE_SIZE, 19*TILE_SIZE, 125)
		append_tween(t, create_center_camera_tween(0.0, 24*TILE_SIZE, 7*TILE_SIZE, 125))
		new_tween(t)
	elseif (frogbert_popped_off and not finished_frogbert_popped_off) then
		finished_frogbert_popped_off = true

		play_sample("sfx/boulder_thud.ogg", 1, 0, 1)

		set_entity_position(frogbert, 23.5*TILE_SIZE, 42*TILE_SIZE)
		set_entity_z(frogbert, 30)
		set_entity_visible(frogbert, true)
		set_entity_animation(frogbert, "dizzy")

		local p0_x2, p0_y2 = get_entity_position(0)
		local ox, oy = get_camera_offset()
		ox = ox - (p0_x2 - p0_x1)
		oy = oy - (p0_y2 - p0_y1)
		set_camera_offset(ox, oy)

		play_sample("sfx/throw.ogg", 1, 0, 1)

		local t = create_direct_move_xyz_tween(frogbert, 0.25, 23.5*TILE_SIZE, 42*TILE_SIZE, 0)
		new_tween(t)

			local x, y = get_entity_position(boulder[5])
		remove_entity(boulder[5])

		boulder[5] = add_entity("freerolling_large_boulder", 5, x, y)

		t = create_direct_move_tween(boulder[5], 23.5*TILE_SIZE, 28*TILE_SIZE, 125)
		append_tween(t, { run = boulder_tween1 })
		new_tween(t)

		t = create_center_camera_tween(0.0, 23.5*TILE_SIZE, 28*TILE_SIZE, 125)
		new_tween(t)
	elseif (lined_up and not fully_pushed and player_count == 4) then
		fully_pushed = true

		p0_x1, p0_y1 = get_entity_position(0)
		frogbert_x, frogbert_y = get_entity_position(frogbert)

		stop_sample("sfx/push_large_boulder.ogg")
		play_sample("sfx/boulder_thud.ogg", 1, 0, 1)
		play_sample("sfx/boulder_roll.ogg", 1, 0, 1)

		remove_entity(boulder[5])
		boulder[5] = add_entity("large_boulder_frogbert", 5, 23.5*TILE_SIZE, 46.5*TILE_SIZE)

		set_entity_visible(frogbert, false)

		set_entity_animation(egbert, "idle-up")
		set_entity_animation(bisou, "idle-up")
		
		t = create_direct_move_tween(boulder[5], 23.5*TILE_SIZE, 40*TILE_SIZE, 100)
		append_tween(t, { run = frogbert_off_boulder })
		new_tween(t)

		t = create_center_camera_tween(0.0, 23.5*TILE_SIZE, 40*TILE_SIZE, 100)
		new_tween(t)
	elseif (started_big_push and not lined_up and player_count == 3) then
		lined_up = true

		player_count = 0

		set_entity_input_disabled(0, true)

		set_character_role(egbert, "")
		set_character_role(frogbert, "")
		set_character_role(bisou, "")

		set_entity_animation(egbert, "push-up")
		set_entity_animation(frogbert, "push-up")
		set_entity_animation(bisou, "push-up")

		local t
		
		play_sample("sfx/push_large_boulder.ogg", 1, 0, 1)

		t = create_direct_move_tween(frogbert, 23.5*TILE_SIZE, 47.5*TILE_SIZE, 50)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_direct_move_tween(egbert, 23*TILE_SIZE, 47.5*TILE_SIZE, 50)
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_direct_move_tween(bisou, 24*TILE_SIZE, 47.5*TILE_SIZE, 50)
		append_tween(t, { run = count_players })
		new_tween(t)
		
		local x, y = get_entity_position(boulder[5])
		remove_entity(boulder[5])

		boulder[5] = add_entity("freerolling_large_boulder", 5, x, y)
		set_entity_solid_with_area(boulder[5], false)

		t = create_direct_move_tween(boulder[5], 23.5*TILE_SIZE, 46.5*TILE_SIZE, 50)
		append_tween(t, { run = count_players })
		new_tween(t)
	elseif (start_big_push and not started_big_push) then
		started_big_push = true

		get_players()

		set_character_role(egbert, "astar", true)
		set_character_role(frogbert, "astar", true)
		set_character_role(bisou, "astar", true)

		player_count = 0

		local t

		t = create_astar_tween(frogbert, 23.5*TILE_SIZE, 49.75*TILE_SIZE, false)
		append_tween(t, { run = count_players })
		new_tween(t)

		t = create_astar_tween(egbert, 23*TILE_SIZE, 49.75*TILE_SIZE, false)
		append_tween(t, { run = count_players })
		new_tween(t)
		
		t = create_astar_tween(bisou, 24*TILE_SIZE, 49.75*TILE_SIZE, false)
		append_tween(t, { run = count_players })
		new_tween(t)
	end
end

local push_counts = {
	0, 0, 0, 0, 0
}

function handle_boulder_collision(n)
	if (not pushing_boulder) then
		pushing_boulder = true
		play_sample("sfx/push_small_boulder.ogg", 1, 0, 1)
	end
	stop_push_sound_time = get_time() + PUSH_SOUND_TIME

	local dir = get_entity_direction(get_player_id(0))

	if (dir == DIR_N) then
		set_entity_animation(get_player_id(0), "push-up")
	elseif (dir == DIR_S) then
		set_entity_animation(get_player_id(0), "push-down")
	else
		set_entity_animation(get_player_id(0), "push")
	end

	local x, y = get_entity_inputs(get_player_id(0))
	local speed = get_entity_speed(get_player_id(0))
	x = x * speed * 2
	y = y * speed * 2
	local x2, y2 = get_entity_position(boulder[n])
	x2 = x2 + x
	y2 = y2 + y
	set_entity_position(boulder[n], x2, y2)
	push_counts[n] = push_counts[n] + math.sqrt(x * x + y * y)
	if (push_counts[n] > 5) then
		push_counts[n] = push_counts[n] - 5
		local f = tonumber(get_entity_animation(boulder[n])) + 1
		if (f > 4) then
			f = 1
		end
		set_entity_animation(boulder[n], "" .. f)
	end
	y2 = y2 - BOULDER_SIZE/2
	if (not point_in_area_bounds(BOULDER_LAYERS[n], x2, y2)) then
		local offset = { -TILE_SIZE/4, -TILE_SIZE/4, TILE_SIZE/4, TILE_SIZE/4 }
		local dx = x2 - (BOULDER_X[n]+offset[n])
		local dy = y2 - BOULDER_Y[n]
		local dist = math.sqrt(dx*dx + dy*dy)
		if (dist < TILE_SIZE) then
			play_sample("sfx/item_found.ogg", 1, 0, 1)
			set_milestone_complete(BOULDER_MILESTONE .. n, true)
			remove_entity(boulder[n])
			boulder[n] = nil
		else
			play_sample("sfx/error.ogg", 1, 0, 1)
			local x, y = get_entity_position(boulder[n])
			add_rock_poof(x, y-BOULDER_SIZE/2)
			set_entity_position(boulder[n], BOULDER_START_X[n], BOULDER_START_Y[n])
		end
	end
end

function collide(id1, id2)
	for i=1,4 do
		if ((not (boulder[i] == nil)) and colliding(id1, id2, get_player_id(0), boulder[i])) then
			handle_boulder_collision(i)
		end
	end
	if (not milestone_is_complete(MILESTONE2_NAME) and colliding(id1, id2, get_player_id(0), boulder[5])) then
		local ready = true
		for i=1,4 do
			if (not milestone_is_complete(BOULDER_MILESTONE .. i)) then
				ready = false
				break
			end
		end
		if (ready) then
			if (not started_big_push) then
				start_big_push = true
				stop_entity(get_player_id(0))
			end
		else
			get_players()
			simple_speak{
				true,
				"BOULDER_NOT_READY", "snooty-anim", egbert
			}
		end
	end
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
	destroy_sample("sfx/amaysa_fly_r_l.ogg")
	destroy_sample("sfx/amaysa.ogg")
	destroy_sample("sfx/throw.ogg")
	destroy_sample("sfx/push_small_boulder.ogg")
	destroy_sample("sfx/giant_footsteps.ogg")
	destroy_sample("sfx/push_large_boulder.ogg")
	destroy_sample("sfx/boulder_thud.ogg")
	destroy_sample("sfx/dizzy.ogg")
	destroy_sample("sfx/boulder_roll.ogg")

	destroy_bitmap("misc_graphics/small_boulder.png")
end

-- NOTE: always needed for jump levels
function jump()
	local tween
	if (jump_block1:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 13*TILE_SIZE,
			end_y = 24*TILE_SIZE,
			end_layer = 3,
			right = false
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block2:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 18*TILE_SIZE,
			end_y = 24*TILE_SIZE,
			end_layer = 2,
			right = true
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block3:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 10*TILE_SIZE,
			end_y = 45*TILE_SIZE,
			end_layer = 4,
			right = false,
			updown = true
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block4:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 10*TILE_SIZE,
			end_y = 41*TILE_SIZE,
			end_layer = 3,
			right = true,
			updown = true
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block5:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 26*TILE_SIZE,
			end_y = 38*TILE_SIZE,
			end_layer = 5,
			right = true,
			updown = true
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block6:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 26*TILE_SIZE,
			end_y = 34*TILE_SIZE,
			end_layer = 2,
			right = true,
			updown = true
		}
		new_tween(tween)
		jumping = true
	end
	if (jumping) then
		jump_table = {}
		for i=1,get_num_players()-1 do
			jump_table[i] = {}
			jump_table[i].entity = get_player_id(i)
			jump_table[i].follow = get_player_id(i-1)
			jump_table[i].x, jump_table[i].y = get_entity_position(0)
			jump_table[i].done = false
			jump_table[i].tween = create_jump_tween{
				entity = get_player_id(i),
				time = tween.time,
				height = tween.height,
				end_x = tween.end_x,
				end_y = tween.end_y,
				end_layer = tween.end_layer,
				right = tween.right,
				updown = tween.updown
			}
		end
	end
end

function post_draw_layer(layer)
	if (layer == 2) then
		local top_x, top_y = get_area_top()
		for i=1,4 do
			if (milestone_is_complete(BOULDER_MILESTONE .. i)) then
				draw_bitmap(boulder_img, BOULDER_X[i]-BOULDER_SIZE/2-top_x, BOULDER_Y[i]-BOULDER_SIZE/2-top_y, 0)
			end
		end
	end
end

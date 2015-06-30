MILESTONE_NAME = "beat_game"

function start()
	play_music("music/jungle_flea_furrrr.mid");

	process_outline()

	set_character_role(get_player_id(0), "")
	set_character_role(get_player_id(1), "follow", get_player_id(0))
	set_character_role(get_player_id(2), "follow", get_player_id(1))

	while (not (get_entity_name(get_player_id(0)) == "egbert")) do
		switch_characters(false)
	end

	egbert = get_player_id(0)
	frogbert = get_player_id(1)
	bisou = get_player_id(2)
		
	set_entity_visible(egbert, true)
	set_entity_visible(frogbert, true)
	set_entity_visible(bisou, true)

	set_entity_input_disabled(egbert, true)

	set_character_role(egbert, "")
	set_character_role(frogbert, "")
	set_character_role(bisou, "")

	crystals = {
		{ n=1, x=440, y=780 },
		{ n=2, x=444, y=730 },
		{ n=3, x=448, y=768 },
		{ n=4, x=452, y=712 },
		{ n=5, x=455, y=730 },
		{ n=6, x=460, y=737 },
		{ n=7, x=464, y=750 },
		{ n=8, x=470, y=711 },
		{ n=1, x=473, y=712 },
		{ n=2, x=477, y=725 },
		{ n=3, x=481, y=734 },
		{ n=4, x=486, y=754 },
		{ n=5, x=441, y=774 },
		{ n=6, x=445, y=772 },
		{ n=7, x=531, y=762 },
		{ n=8, x=535, y=757 },
		{ n=1, x=540, y=725 },
		{ n=2, x=532, y=721 },
		{ n=3, x=490, y=720 },
		{ n=4, x=450, y=745 },
		{ n=5, x=535, y=750 },
		{ n=6, x=460, y=755 },
		{ n=7, x=530, y=765 },
		{ n=8, x=490, y=770 }
	}

	for i=1,#crystals do
		crystals[i].id = add_entity("crystals", 2, crystals[i].x, crystals[i].y)
		set_entity_animation(crystals[i].id, "" .. crystals[i].n)
	end

	local t = create_idle_tween(5)
	append_tween(t, { run = start_scene })
	new_tween(t)

	egbert_start_x, egbert_start_y = get_entity_position(egbert)
	t = create_center_camera_tween(1, egbert_start_x + 40, egbert_start_y - 50, 50)
	new_tween(t)

	load_sample("sfx/thud.ogg", false)
	load_sample("sfx/amaysa_fly_l_r.ogg", false)
	load_sample("sfx/amaysa_fly_l_r_short.ogg", false)
	load_sample("sfx/amaysa_fly_r_l.ogg", false)
	load_sample("sfx/amaysa.ogg", false)
	load_sample("sfx/dizzy.ogg", false)
	load_sample("sfx/blush.ogg", false)
	load_sample("sfx/kiss.ogg", false)
end

function activate(activator, activated)
end

function start_scene(tween, id, elapsed)
	scene_started = true
	return true
end

function amaysa_fly_in(tween, id, elapsed)
	amaysa_flew_in = true
	return true
end

function crystal_tossed(tween, id, elapsed)
	tossed_crystal = true
	set_entity_direction(egbert, DIR_N)
	set_entity_animation(egbert, "idle-up")
	return true
end

function amaysa_fall(tween, id, elapsed)
	amaysa_fell = true
	return true
end

function kiss(tween, id, elapsed)
	play_sample("sfx/kiss.ogg", 1, 0, 1)
	set_entity_animation(amaysa, "kiss")
	return true
end

function amaysa_gone(tween, id, elapsed)
	amaysa_is_gone = true
	return true
end

function final_speech(tween, id, elapsed)
	final_spoke = true
	return true
end

function amaysa_fly_left(tween, id, elapsed)
	set_entity_right(amaysa, false)
	set_entity_animation(amaysa, "fly-left-right")
	return true
end

function frogbert_step_out(tween, id, elapsed)
	frogbert_stepped_out = true
	return true
end

function run_credits(tween, id, elapsed)
	should_run_credits = true
	return true
end

local AMAYSA_FLY_SPEED = 400

function logic()
	if (should_run_credits and not _should_run_credits) then
		_should_run_credits = true
		fade(0, 0, 0, 0, 0, 0)
		credits()
	elseif (final_spoke and not _final_spoke) then
		_final_spoke = true

		speak_force_top(false, false, true, t("END_FROGBERT_8"), "", frogbert)
		speak_force_top(false, false, true, t("END_EGBERT_3"), "", egbert)
		simple_speak{
			true,
			"END_BISOU_5", "", bisou
		}
		speak_force_top(false, false, true, t("END_EGBERT_4"), "", egbert)
		simple_speak{
			true,
			"END_BISOU_6", "", bisou
		}
		speak_force_top(false, false, true, t("END_EGBERT_5"), "", egbert)
		simple_speak{
			true,
			"END_BISOU_7", "", bisou
		}
		speak_force_top(false, false, true, t("END_FROGBERT_9"), "", frogbert)

		set_entity_input_disabled(egbert, true)
		set_character_role(egbert, "")
		set_entity_animation(egbert, "gesture")

		set_character_role(bisou, "")
		play_sample("sfx/bisou.ogg", 1, 0, 1)
		set_entity_animation(bisou, "laugh")

		fade(0, 5, 0, 0, 0, 255)

		local t = create_idle_tween(5)
		append_tween(t, { run = run_credits })
		new_tween(t)
	elseif (amaysa_is_gone and not _amaysa_is_gone) then
		_amaysa_is_gone = true

		set_entity_direction(egbert, DIR_S)
		set_entity_direction(frogbert, DIR_S)
		set_entity_direction(bisou, DIR_W)

		simple_speak{
			true,
			"END_FROGBERT_6", "blush", frogbert,
			"END_BISOU_3", "laugh", bisou,
			"END_FROGBERT_7", "", frogbert,
			"END_EGBERT_2", "", egbert,
			"END_BISOU_4", "", bisou
		}

		set_character_role(egbert, "astar")
		set_entity_input_disabled(egbert, false)

		local t = create_astar_tween(egbert, 443, 749, false)
		new_tween(t)

		t = create_astar_tween(frogbert, 450, 732, false)
		new_tween(t)

		t = create_astar_tween(bisou, 449, 768, false)
		new_tween(t)

		t = create_idle_tween(2)
		append_tween(t, { run = final_speech })
		new_tween(t)
	elseif (frogbert_stepped_out and not _frogbert_stepped_out) then
		_frogbert_stepped_out = true

		set_entity_right(frogbert, true)

		speak_force_top(false, false, true, t("END_FROGBERT_2"), "", frogbert)

		set_milestone_complete(MILESTONE_NAME, true)

		speak_force_bottom(false, false, true, t("END_AMAYSA_2"), "", amaysa)

		simple_speak{
			true,
			"END_BISOU_2", "", bisou
		}

		speak_force_bottom(false, false, true, t("END_AMAYSA_3"), "", amaysa)

		speak_force_top(false, false, true, t("END_FROGBERT_3"), "", frogbert)
		speak_force_bottom(false, false, true, t("END_AMAYSA_4"), "", amaysa)
		speak_force_top(false, false, true, t("END_FROGBERT_4"), "", frogbert)
		speak_force_bottom(false, false, true, t("END_AMAYSA_5"), "", amaysa)
		speak_force_top(false, false, true, t("END_FROGBERT_5"), "hand-gesture", frogbert)
		speak_force_top(false, false, true, t("END_EGBERT_1"), "snooty-anim", egbert)
		speak_force_bottom(false, false, true, t("END_AMAYSA_6"), "", amaysa)
		
		set_entity_z(amaysa, 10)
		local frogbert_x, frogbert_y = get_entity_position(frogbert)
		local t = create_direct_move_tween(amaysa, frogbert_x, frogbert_y - 10, AMAYSA_FLY_SPEED)
		append_tween(t, { run = kiss })
		append_tween(t, create_idle_tween(0.5))
		append_tween(t, create_play_sample_tween("sfx/amaysa_fly_r_l.ogg"))
		append_tween(t, { run = amaysa_fly_left })
		append_tween(t, create_direct_move_tween(amaysa, frogbert_x - 200, frogbert_y - 50, AMAYSA_FLY_SPEED))
		append_tween(t, { run = amaysa_gone })
		new_tween(t)
	elseif (amaysa_fell and not _amaysa_fell) then
		_amaysa_fell = true

		local x, y = get_entity_position(frogbert)
		local t = create_astar_tween(frogbert, x - 25, y - 25, false)
		append_tween(t, { run = frogbert_step_out })
		new_tween(t)
	elseif (tossed_crystal and not _tossed_crystal) then
		_tossed_crystal = true

		local amaysa_x, amaysa_y = get_entity_position(amaysa)
		set_entity_animation(amaysa, "dizzy")
		play_sample("sfx/dizzy.ogg", 1, 0, 1)
		local t = create_direct_move_xyz_tween(amaysa, 0.5, amaysa_x, amaysa_y, 0)
		append_tween(t, { run = amaysa_fall })
		new_tween(t)
	elseif (amaysa_flew_in and not _amaysa_flew_in) then
		_amaysa_flew_in = true

		set_entity_animation(amaysa, "fly-idle")

		set_entity_direction(egbert, DIR_N)
		set_entity_direction(frogbert, DIR_N)
		set_entity_direction(bisou, DIR_N)

		speak_force_bottom(false, false, true, t("END_AMAYSA_1"), "fly-idle", amaysa)
		
		set_entity_animation(amaysa, "fly-idle")

		local t = create_direct_move_tween(amaysa, 460, 730, AMAYSA_FLY_SPEED)
		new_tween(t)

		crystal = add_entity("crystal", 3, egbert_start_x + 3, egbert_start_y - 10)
		local crystal_x, crystal_y = get_entity_position(crystal)
		play_sample("sfx/throw.ogg", 1, 0, 1)
		set_entity_right(egbert, true)
		set_entity_animation(egbert, "throw")
		reset_entity_animation(egbert)
		t = create_toss_tween(crystal, crystal_x, crystal_y, 460, 685, 1)
		append_tween(t, { run = crystal_tossed })
		append_tween(t, create_play_sample_tween("sfx/thud.ogg"))
		append_tween(t, create_toss_tween(crystal, 460, 710, 480, 740, 0.3))
		append_tween(t, create_play_sample_tween("sfx/thud.ogg"))
		new_tween(t)
	elseif (scene_started and not _scene_started) then
		_scene_started = true

		set_entity_right(egbert, true)
		set_entity_animation(egbert, "idle-down")
		set_entity_right(frogbert, true)
		set_entity_animation(frogbert, "idle-down")
		set_entity_right(bisou, false)
		set_entity_animation(bisou, "idle")

		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		simple_speak{
			true,
			"END_BISOU_1", "", bisou
		}

		speak_force_top(false, false, true, t("END_FROGBERT_1"), "", frogbert)

		AMAYSA_START_Y = egbert_start_y - 50

		amaysa = add_entity("amaysa", 2, egbert_start_x - 200, AMAYSA_START_Y)
		set_entity_z(amaysa, 40)
		set_show_entity_shadow(amaysa, true)
		set_entity_animation(amaysa, "fly-left-right")
		set_entity_right(amaysa, true)
		set_entity_solid_with_area(amaysa, false)

		play_sample("sfx/amaysa_fly_l_r.ogg", 1, 0, 1)
		local t = create_direct_move_tween(amaysa, egbert_start_x, AMAYSA_START_Y, AMAYSA_FLY_SPEED)
		append_tween(t, { run = amaysa_fly_in })
		new_tween(t)

		local bisou_x, bisou_y = get_entity_position(bisou)
		t = create_idle_tween(0.5)
		append_tween(t, create_astar_tween(frogbert, bisou_x, bisou_y + 10, true))
		append_tween(t, create_change_direction_tween(frogbert, DIR_N))
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
	destroy_sample("sfx/thud.ogg")
	destroy_sample("sfx/amaysa_fly_l_r.ogg")
	destroy_sample("sfx/amaysa_fly_l_r_short.ogg")
	destroy_sample("sfx/amaysa_fly_r_l.ogg")
	destroy_sample("sfx/amaysa.ogg")
	destroy_sample("sfx/dizzy.ogg")
	destroy_sample("sfx/blush.ogg")
	destroy_sample("sfx/kiss.ogg")
end

function pre_draw_layer(layer)
	if (layer == 2 and not set_on_ground) then
		set_on_ground = true

		set_entity_position(frogbert, 390, 760)
		set_entity_position(bisou, 420, 785)

		set_entity_animation(egbert, "on-ground")
		set_entity_animation(frogbert, "on-ground")
		set_entity_animation(bisou, "on-ground")
	end
end


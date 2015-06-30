MILESTONE_NAME = "pyoudock_intro"

local first_talk = false
local num_in_canoe = 0
local num_off_canoe = 0

local plank_x = 327
local plank_y = 123
local plank_a = 0.0
local plank_sx = plank_x
local plank_sy = plank_y
local plank_sa = plank_a
local plank_dx = 357
local plank_dy = 140
local plank_da = math.pi * 1.5
local plank_count = 0
local plank_frames = 25

local top_x, top_y

function start()
	play_music("music/abw2.mid");

	process_outline()

	load_sample("sfx/rowing.ogg", false)

	add_wall_group(2, 9, 4, 3, 2, TILE_GROUP_BUSHES)

	to_pyou2 = Active_Block:new{x=0, y=6, width=1, height=5}

	plank = load_bitmap("misc_graphics/canoe_plank.png")

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

	if (milestone_is_complete("CANOEING")) then -- already started canoeing in from the west
		canoeing = true
		dont_draw_plank = true

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		set_entity_visible(pyou, false)
		set_entity_visible(egbert, false)
		set_entity_visible(frogbert, false)
		set_entity_visible(bisou, false)

		set_entity_position(egbert, 352, 111)
		set_entity_position(frogbert, 332, 111)
		set_entity_position(bisou, 310, 111)

		local x = 327 + 300
		local y = 129

		canoe = add_entity("canoe", 1, x, y)
		set_entity_animation(canoe, "west")

		local t = create_slowing_row_tween(canoe, x, y, x-300, y, 0, -3)
		append_tween(t, { run = get_off })
		new_tween(t)

	else
		empty_canoe = add_entity("empty_canoe", 1, 327, 129)
		pyou = add_npc("pyou", 2, 291, 109)
	end

	if (not milestone_is_complete(MILESTONE_NAME)) then
		set_character_role(get_player_id(0), "astar")

		local t = create_idle_tween(1)
		append_tween(t, create_astar_tween(get_player_id(0), 224, 144, true))
		append_tween(t, { run = do_first_talk })
		new_tween(t)
	end

	canoe_activate = Active_Block:new{x=16, y=8, width=1, height=2}
end

function do_first_talk(tween, id, elapsed)
	first_talk = true
	return true
end

function in_canoe(tween, id, elapsed)
	num_in_canoe = num_in_canoe + 1
	return true
end

function next_area(tween, id, elapsed)
	set_milestone_complete("CANOEING", true)
	next_player_layer = 2
	change_areas("oldforestdock", DIR_E, 150, 160)
	return true
end

function get_off(tween, id, elapsed)
	remove_entity(canoe)
	empty_canoe = add_entity("empty_canoe", 1, 327, 129)
	pyou = add_npc("pyou", 2, 291, 109)
	set_entity_visible(egbert, true)
	set_entity_visible(frogbert, true)
	set_entity_visible(bisou, true)
	getting_off = true
	return true
end

function off_boat(tween, id, elapsed)
	num_off_canoe = num_off_canoe + 1
	return true
end

function activate(activator, activated)
end

function logic()
	if (to_pyou2:entity_is_colliding(0)) then
		set_milestone_complete("CANOEING", false)
		next_player_layer = 2
		change_areas("pyou2", DIR_W, 1862, 1158)
	elseif (canoe_activate:entity_is_colliding(0) and not getting_in_canoe and not canoeing) then
		getting_in_canoe = true
		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		t = create_astar_tween(egbert, 327, 144, false)
		append_tween(t, create_astar_tween(egbert, 327, 111, false))
		append_tween(t, create_astar_tween(egbert, 352, 111, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)

		t = create_astar_tween(frogbert, 327, 144, false)
		append_tween(t, create_astar_tween(frogbert, 327, 111, false))
		append_tween(t, create_astar_tween(frogbert, 332, 111, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)

		t = create_astar_tween(bisou, 327, 144, false)
		append_tween(t, create_astar_tween(bisou, 327, 111, false))
		append_tween(t, create_astar_tween(bisou, 310, 111, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)
	end

	if (getting_off and not got_off) then
		dont_draw_plank = false

		got_off = true

		set_entity_direction(pyou, DIR_W)
		set_entity_direction(egbert, DIR_E)
		set_entity_direction(frogbert, DIR_E)
		set_entity_direction(bisou, DIR_E)

		local t

		t = create_astar_tween(get_player_id(0), 327, 111, false)
		append_tween(t, create_astar_tween(get_player_id(0), 327, 144, false))
		append_tween(t, create_astar_tween(get_player_id(0), 224, 144, false))
		append_tween(t, { run = off_boat })
		new_tween(t)

		t = create_astar_tween(get_player_id(1), 327, 111, false)
		append_tween(t, create_astar_tween(get_player_id(1), 327, 144, false))
		append_tween(t, create_astar_tween(get_player_id(1), 224, 144, false))
		append_tween(t, { run = off_boat })
		new_tween(t)

		t = create_astar_tween(get_player_id(2), 327, 111, false)
		append_tween(t, create_astar_tween(get_player_id(2), 327, 144, false))
		append_tween(t, create_astar_tween(get_player_id(2), 224, 144, false))
		append_tween(t, { run = off_boat })
		new_tween(t)
	end

	if (not milestone_is_complete(MILESTONE_NAME)) then
		if (first_talk and not first_talked) then
			first_talked = true
			simple_speak{
				true,
				"PYOUDOCK_EGBERT_1", "gesture", egbert,
				"PYOUDOCK_FROGBERT_1", "thinking", frogbert
			}

			speak_force_bottom(false, false, true, t("PYOUDOCK_PYOU_1"), "", pyou)

			simple_speak{
				true,
				"PYOUDOCK_EGBERT_2", "", egbert
			}


			speak_force_bottom(false, false, true, t("PYOUDOCK_PYOU_2"), "", pyou)

			set_character_role(get_player_id(0), "")

			set_milestone_complete(MILESTONE_NAME, true)
		end
	end

	if (not all_off_boat and num_off_canoe == 3) then
		canoeing = false
		all_off_boat = true
		set_character_role(get_player_id(0), "")
		set_character_role(get_player_id(1), "follow", get_player_id(0))
		set_character_role(get_player_id(2), "follow", get_player_id(1))
	end
	
	if (num_in_canoe == 3 and not all_in_canoe) then
		all_in_canoe = true

		remove_plank = true

		set_entity_visible(pyou, false)
		set_entity_visible(egbert, false)
		set_entity_visible(frogbert, false)
		set_entity_visible(bisou, false)

		local x, y = get_entity_position(empty_canoe)
		remove_entity(empty_canoe)

		canoe = add_entity("canoe", 1, x, y)

		local t = create_row_tween(canoe, x, y, x+300, y, 4.0, -15, -3)
		append_tween(t, { run = next_area })
		new_tween(t)
	end

	if (remove_plank and plank_count < plank_frames) then
		plank_count = plank_count + 1
		local p = plank_count / plank_frames
		plank_x = plank_sx + p * (plank_dx - plank_sx)
		plank_y = plank_sy + p * (plank_dy - plank_sy)
		plank_a = plank_sa + p * (plank_da - plank_sa)
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
	destroy_sample("sfx/rowing.ogg")
	destroy_bitmap("misc_graphics/canoe_plank.png")
end

function post_draw_layer(layer)
	if (layer == 1) then
		if (dont_draw_plank) then
			return
		end
		local x, y = get_area_top()
		if (not (x == nil)) then
			top_x = x
			top_y = y
		end
		draw_tinted_rotated_bitmap(
			plank,
			0.8,
			0.8,
			0.8,
			1.0,
			8,
			11,
			plank_x - top_x,
			plank_y - top_y,
			plank_a,
			0
		)
	end
end

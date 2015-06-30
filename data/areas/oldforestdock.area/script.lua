local MILESTONE_NAME = "canoe_outro"

local num_in_canoe = 0
local num_off_canoe = 0

local plank_x = 150
local plank_y = 154
local plank_a = 0.0
local plank_sx = plank_x
local plank_sy = plank_y
local plank_sa = plank_a
local plank_dx = 120
local plank_dy = 171
local plank_da = -math.pi * 1.5
local plank_count = 0
local plank_frames = 25

local top_x, top_y

function start()
	play_music("music/old_forest.mid");

	process_outline()
	
	load_sample("sfx/rowing.ogg", false)

	if (not milestone_is_complete("entered_old_forest")) then
		set_milestone_complete("entered_old_forest", true)
	end

	-- mario bushes
	add_wall_group(2, 17, 13, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 11, 2, 2, TILE_GROUP_BUSHES)
	-- SOM bushes
	add_wall_group(2, 21, 6, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 7, 2, 2, TILE_GROUP_BUSHES)

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

		set_entity_position(egbert, 175, 142)
		set_entity_position(frogbert, 155, 142)
		set_entity_position(bisou, 133, 142)

		local x = -150
		local y = 160

		canoe = add_entity("canoe", 1, x, y)

		local t = create_slowing_row_tween(canoe, x, y, x+300, y, -15, -3)
		append_tween(t, { run = get_off })
		new_tween(t)
	else
		empty_canoe = add_entity("empty_canoe", 1, 150, 160)
		pyou = add_npc("pyou", 2, 114, 140)
	end

	canoe_activate = Active_Block:new{x=12, y=10, width=1, height=2}

	exit_right = Active_Block:new{x=27, y=8, width=1, height=7}
end

function get_off(tween, id, elapsed)
	remove_entity(canoe)
	empty_canoe = add_entity("empty_canoe", 1, 150, 160)
	pyou = add_npc("pyou", 2, 114, 140)
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

function in_canoe(tween, id, elapsed)
	num_in_canoe = num_in_canoe + 1
	return true
end

function next_area(tween, id, elapsed)
	set_milestone_complete("CANOEING", true)
	next_player_layer = 2
	change_areas("pyoudock", DIR_E, 327, 129)
	return true
end

function activate(activator, activated)
end

function logic()
	if (canoe_activate:entity_is_colliding(0) and not getting_in_canoe and not canoeing) then
		getting_in_canoe = true

		set_character_role(egbert, "astar")
		set_character_role(frogbert, "astar")
		set_character_role(bisou, "astar")

		local t

		t = create_astar_tween(egbert, 150, 175, false)
		append_tween(t, create_astar_tween(egbert, 150, 142, false))
		append_tween(t, create_astar_tween(egbert, 175, 142, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)

		t = create_astar_tween(frogbert, 150, 175, false)
		append_tween(t, create_astar_tween(frogbert, 150, 142, false))
		append_tween(t, create_astar_tween(frogbert, 155, 142, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)

		t = create_astar_tween(bisou, 150, 175, false)
		append_tween(t, create_astar_tween(bisou, 150, 142, false))
		append_tween(t, create_astar_tween(bisou, 133, 142, false))
		append_tween(t, { run = in_canoe })
		new_tween(t)
	elseif (exit_right:entity_is_colliding(0)) then
		set_milestone_complete("CANOEING", false)
		start_map("OLD_FOREST_ENTRANCE")
	end

	if (getting_off and not got_off) then
		dont_draw_plank = false

		got_off = true

		set_entity_direction(pyou, DIR_E)
		set_entity_direction(egbert, DIR_W)
		set_entity_direction(frogbert, DIR_W)
		set_entity_direction(bisou, DIR_W)

		if (not milestone_is_complete(MILESTONE_NAME)) then
			simple_speak{
				true,
				"OLDFORESTDOCK_PYOU_1", "", pyou,
				"OLDFORESTDOCK_EGBERT_1", "", egbert,
				"OLDFORESTDOCK_PYOU_2", "greet", pyou,
				"OLDFORESTDOCK_FROGBERT_1", "thinking", frogbert
			}

			set_milestone_complete(MILESTONE_NAME, true)
		end

		local t

		t = create_astar_tween(egbert, 150, 142, false)
		append_tween(t, create_astar_tween(egbert, 150, 175, false))
		append_tween(t, create_astar_tween(egbert, 256, 175, false))
		append_tween(t, { run = off_boat })
		new_tween(t)

		t = create_astar_tween(frogbert, 150, 142, false)
		append_tween(t, create_astar_tween(frogbert, 150, 175, false))
		append_tween(t, create_astar_tween(frogbert, 256, 175, false))
		append_tween(t, { run = off_boat })
		new_tween(t)

		t = create_astar_tween(bisou, 150, 142, false)
		append_tween(t, create_astar_tween(bisou, 150, 175, false))
		append_tween(t, create_astar_tween(bisou, 256, 175, false))
		append_tween(t, { run = off_boat })
		new_tween(t)
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
		set_entity_animation(canoe, "west")

		local t = create_row_tween(canoe, x, y, x-300, y, 4.0, 0, -3)
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

local seconds = 0

function start()
	play_music("music/castle.mid");

	process_outline()

	-- Table and chairs
	add_wall_group(2, 100, 39, 2, 3, 0)
	add_wall_group(2, 105, 39, 2, 3, 0)
	add_wall_group(2, 102, 39, 3, 4, 0)

	-- Railings
	add_tile_group(
		2,
		21*TILE_SIZE,
		6*TILE_SIZE,
		9*TILE_SIZE,
		10,
		0,
		29, 1,
		29, 2,
		28, 2,
		27, 2,
		28, 3,
		27, 3,
		26, 3,
		25, 3,
		26, 4,
		25, 4,
		24, 4,
		23, 4,
		24, 5,
		23, 5,
		22, 5,
		21, 5,
		22, 6
	)

	add_tile_group(
		2,
		96*TILE_SIZE,
		6*TILE_SIZE,
		9*TILE_SIZE,
		10,
		0,
		96, 1,
		96, 2,
		97, 2,
		98, 2,
		97, 3,
		98, 3,
		99, 3,
		100, 3,
		99, 4,
		100, 4,
		101, 4,
		102, 4,
		101, 5,
		102, 5,
		103, 5,
		104, 5,
		103, 6

	)

	pig1 = add_npc("pig_guard", 2, 1744, 94)
	set_entity_direction(pig1, DIR_S)

	pig2 = add_npc("pig_guard", 2, 448, 383)
	local t = create_character_role_change_tween(pig2, "null")
	append_tween(t, create_change_direction_tween(pig2, DIR_S))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_W))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_N))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_E))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_S))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_character_role_change_tween(pig2, "astar"))
	append_tween(t, create_astar_tween(pig2, 608, 752))
	append_tween(t, create_astar_tween(pig2, 967, 773))
	append_tween(t, create_astar_tween(pig2, 1408, 752))
	append_tween(t, create_astar_tween(pig2, 1568, 383))
	append_tween(t, create_character_role_change_tween(pig2, "null"))
	append_tween(t, create_change_direction_tween(pig2, DIR_S))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_W))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_N))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_E))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_change_direction_tween(pig2, DIR_S))
	append_tween(t, create_idle_tween(1))
	append_tween(t, create_character_role_change_tween(pig2, "astar"))
	append_tween(t, create_astar_tween(pig2, 1408, 752))
	append_tween(t, create_astar_tween(pig2, 967, 773))
	append_tween(t, create_astar_tween(pig2, 608, 752))
	append_tween(t, create_astar_tween(pig2, 448, 383))
	append_tween(t, t)
	new_tween(t)

	pig3 = add_entity("pig_guard", 2, 239, 590)
	set_entity_direction(pig3, DIR_S)

	left_throne = Active_Block:new{x=42, y=24, width=1, height=6}
	right_throne = Active_Block:new{x=84, y=24, width=1, height=6}

	-- Exits to exterior, left to right
	ext_exit1 = Active_Block:new{x=5, y=10, width=3, height=2}
	ext_exit2 = Active_Block:new{x=18, y=44, width=3, height=2}
	ext_exit3 = Active_Block:new{x=59, y=51, width=3, height=2}
	ext_exit4 = Active_Block:new{x=105, y=44, width=3, height=2}
	ext_exit5 = Active_Block:new{x=118, y=10, width=3, height=2}

	knight1 = Active_Block:new{x=28, y=2, width=1, height=2}
	knight2 = Active_Block:new{x=97, y=2, width=1, height=2}
	
	chest = Chest:new{x=1928, y=96, layer=2, contains="DIRTYSOCK", quantity=1, milestone="castle_hall_sock"}
end

function logic()
	if (left_throne:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_throne", DIR_E, 51, 680)
	elseif (right_throne:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_throne", DIR_W, 617, 680)
	elseif (ext_exit1:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 247, 694)
	elseif (ext_exit2:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 304, 799)
	elseif (ext_exit3:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 576, 696)
	elseif (ext_exit4:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 848, 799)
	elseif (ext_exit5:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 904, 694)
	elseif (knight1:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle_knight_left1", DIR_E, 79, 207)
	elseif (knight2:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle_knight_right1", DIR_W, 192, 207)
	end
end

function activate(activator, activated)
	if (activated == pig1) then
		speak(false, false, true, t("PIG_TOWER_5"), "", pig1, pig1)
	elseif (activated == pig2) then
		speak(false, false, true, t("PIG_HALL_1"), "", pig2, pig2)
	elseif (activated == pig3) then
		speak(false, false, true, t("PIG_HALL_2"), "", pig3, pig3)
	elseif (activated == chest.id) then
		chest:open()
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

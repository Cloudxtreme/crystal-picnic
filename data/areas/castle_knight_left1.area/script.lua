function start()
	play_music("music/castle.mid");

	process_outline()

	add_wall_group(3, 3, 8, 2, 1, 0)
	add_wall_group(3, 6, 8, 2, 1, 0)
	add_wall_group(3, 10, 8, 2, 1, 0)
	add_wall_group(3, 13, 8, 1, 1, 0)

	-- down rail
	add_tile_group(
		3,
		1*TILE_SIZE,
		12*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		4, 10,
		4, 11,
		3, 10,
		3, 11,
		2, 11,
		1, 11
	)

	-- up rail
	add_tile_group(
		3,
		10*TILE_SIZE,
		12*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		10, 10,
		10, 11,
		11, 10,
		11, 11,
		12, 10,
		13, 9,
		13, 10
	)

	pig = add_npc("pig_guard", 3, 144, 127)
	set_character_role(pig, "wander", 128, 0.2, 0.6)

	going_down = Active_Block:new{x=2, y=12, width=1, height=2}
	going_up = Active_Block:new{x=14, y=2, width=3, height=1}

	layer_change_down = Active_Block:new{x=10, y=11, width=1, height=3}
	layer_change_up = Active_Block:new{x=12, y=11, width=1, height=2}
end

function logic()
	if (layer_change_down:entity_is_colliding(0)) then
		set_entity_layer(0, 3)
		set_entity_layer(1, 3)
	elseif (layer_change_up:entity_is_colliding(0)) then
		set_entity_layer(0, 4)
		set_entity_layer(1, 4)
	elseif (going_up:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_knight_left2", DIR_N, 168, 134)
	elseif (going_down:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_hall", DIR_W, 424, 59)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_TOWER_4_2"), "", pig, pig)
		else
			speak(false, false, true, t("PIG_TOWER_4"), "", pig, pig)
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
end

function stop()
end

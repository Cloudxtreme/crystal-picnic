function start()
	play_music("music/castle.mid");

	process_outline()

	add_wall_group(3, 4, 8, 1, 1, 0)
	add_wall_group(3, 6, 8, 2, 1, 0)
	add_wall_group(3, 10, 8, 2, 1, 0)
	add_wall_group(3, 13, 8, 2, 1, 0)

	-- down rail
	add_tile_group(
		3,
		13*TILE_SIZE,
		12*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		13, 10,
		14, 10,
		13, 11,
		14, 11,
		15, 11,
		16, 11
	)

	-- up rail
	add_tile_group(
		3,
		4*TILE_SIZE,
		12*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		4, 9,
		4, 10,
		5, 10,
		6, 10,
		7, 10,
		6, 11,
		7, 11
	)

	going_down = Active_Block:new{x=15, y=12, width=1, height=2}
	going_up = Active_Block:new{x=1, y=2, width=3, height=1}

	layer_change_down = Active_Block:new{x=7, y=11, width=1, height=3}
	layer_change_up = Active_Block:new{x=5, y=11, width=1, height=2}
end

function logic()
	if (layer_change_down:entity_is_colliding(0)) then
		set_entity_layer(0, 3)
		set_entity_layer(1, 3)
	elseif (layer_change_up:entity_is_colliding(0)) then
		set_entity_layer(0, 4)
		set_entity_layer(1, 4)
	elseif (going_up:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle_knight_right2", DIR_N, 39, 134)
	elseif (going_down:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_hall", DIR_E, 1590, 56)
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

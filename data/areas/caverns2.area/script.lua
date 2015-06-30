is_dungeon = true

function start(game_just_loaded)
	play_music("music/caverns.mid");

	process_outline()

	-- stalactites
	add_wall_group(2, 32, 11, 2, 6, 0)
	add_wall_group(2, 44, 9, 2, 6, 0)
	add_wall_group(2, 47, 14, 2, 6, 0)
	add_wall_group(2, 67, 30, 2, 6, 0)
	add_wall_group(2, 61, 33, 2, 6, 0)
	add_wall_group(2, 66, 37, 2, 6, 0)
	add_wall_group(2, 8, 53, 2, 6, 0)
	add_wall_group(2, 14, 52, 2, 6, 0)
	add_wall_group(2, 41, 52, 2, 6, 0)
	add_wall_group(2, 46, 55, 2, 6, 0)
	add_wall_group(2, 83, 56, 2, 6, 0)
	
	if (not game_just_loaded) then
		add_enemies(2, { "grenade_ant" }, 12, 4, 5, "caverns", { "machete_ant", "scorpion", "grenade_ant", "bat" })
	end

	chest1 = Chest:new{x=81.5*TILE_SIZE, y=43.5*TILE_SIZE, layer=2, contains="HEALTHVIAL", quantity=5, milestone="caverns2_chest1"}
	chest2 = Chest:new{x=47.5*TILE_SIZE, y=57.5*TILE_SIZE, layer=2, contains="ANTIDOTE", quantity=3, milestone="caverns2_chest2"}
	chest3 = Chest:new{x=67.5*TILE_SIZE, y=11.5*TILE_SIZE, layer=2, contains_crystals=true, contains="CRYSTAL", quantity=1, milestone="caverns2_chest3"}

	to_caverns1 = Active_Block:new{x=60, y=76, width=5, height=1}
	to_caverns3 = Active_Block:new{x=82, y=0, width=4, height=1}

	start_bat_swoops()
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest3.id) then
		chest3:open()
	end
end

function logic()
	if (to_caverns1:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns1", DIR_S, 29.5*TILE_SIZE, 4.5*TILE_SIZE)
	elseif (to_caverns3:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns3", DIR_N, 29.5*TILE_SIZE, 57*TILE_SIZE)
	end

	check_bat_swoops()
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
	stop_bat_swoops()
end

function post_draw_layer(layer)
	if (layer == 2) then
		draw_bat_swoops()
	end
end

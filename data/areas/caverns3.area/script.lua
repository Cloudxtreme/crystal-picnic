is_dungeon = true

function start(game_just_loaded)
	play_music("music/caverns.mid")

	process_outline()

	-- stalactites
	add_wall_group(2, 23, 48, 2, 6, 0)
	add_wall_group(2, 32, 45, 2, 6, 0)

	-- bridge posts
	-- vert
	add_wall_group(2, 11, 11, 1, 1, 0)
	add_wall_group(2, 13, 11, 1, 1, 0)
	add_wall_group(2, 11, 23, 1, 1, 0)
	add_wall_group(2, 13, 23, 1, 1, 0)
	add_wall_group(2, 11, 36, 1, 1, 0)
	add_wall_group(2, 13, 36, 1, 1, 0)
	add_wall_group(2, 27, 23, 1, 1, 0)
	add_wall_group(2, 29, 23, 1, 1, 0)
	add_wall_group(2, 27, 35, 1, 1, 0)
	add_wall_group(2, 29, 35, 1, 1, 0)
	add_wall_group(2, 43, 11, 1, 1, 0)
	add_wall_group(2, 45, 11, 1, 1, 0)
	add_wall_group(2, 43, 23, 1, 1, 0)
	add_wall_group(2, 45, 23, 1, 1, 0)
	add_wall_group(2, 43, 35, 1, 1, 0)
	add_wall_group(2, 45, 35, 1, 1, 0)
	add_wall_group(2, 59, 11, 1, 1, 0)
	add_wall_group(2, 61, 11, 1, 1, 0)
	add_wall_group(2, 59, 35, 1, 1, 0)
	add_wall_group(2, 61, 35, 1, 1, 0)
	add_wall_group(2, 91, 11, 1, 1, 0)
	add_wall_group(2, 93, 11, 1, 1, 0)
	add_wall_group(2, 91, 23, 1, 1, 0)
	add_wall_group(2, 93, 23, 1, 1, 0)
	add_wall_group(2, 91, 35, 1, 1, 0)
	add_wall_group(2, 93, 35, 1, 1, 0)
	add_wall_group(2, 75, 47, 1, 1, 0)
	add_wall_group(2, 77, 47, 1, 1, 0)
	-- horiz
	add_bridge_post_group(2, 15, 21)
	add_bridge_post_group(2, 25, 21)
	add_bridge_post_group(2, 31, 9)
	add_bridge_post_group(2, 41, 9)
	add_bridge_post_group(2, 31, 33)
	add_bridge_post_group(2, 41, 33)
	add_bridge_post_group(2, 47, 21)
	add_bridge_post_group(2, 57, 21)
	add_bridge_post_group(2, 47, 45)
	add_bridge_post_group(2, 57, 45)
	add_bridge_post_group(2, 63, 9)
	add_bridge_post_group(2, 73, 9)
	add_bridge_post_group(2, 63, 33)
	add_bridge_post_group(2, 73, 33)
	add_bridge_post_group(2, 79, 9)
	add_bridge_post_group(2, 89, 9)
	add_bridge_post_group(2, 79, 21)
	add_bridge_post_group(2, 89, 21)
	add_bridge_post_group(2, 79, 45)
	add_bridge_post_group(2, 89, 45)
	
	if (not game_just_loaded) then
		add_enemies(2, { "grenade_ant" }, 8, 4, 5, "caverns", { "machete_ant", "scorpion", "grenade_ant", "bat" })
	end

	chest2 = Chest:new{x=76.5*TILE_SIZE, y=22.5*TILE_SIZE, layer=2, contains="HEALTHVIAL", quantity=5, milestone="caverns3_chest2"}
	chest3 = Chest:new{x=76.5*TILE_SIZE, y=34.5*TILE_SIZE, layer=2, contains="DIRTYSOCK", quantity=2, milestone="caverns3_chest3"}

	to_caverns2 = Active_Block:new{x=28, y=61, width=3, height=1}
	to_caverns4 = Active_Block:new{x=74, y=61, width=6, height=1}

	start_bat_swoops()
end

function activate(activator, activated)
	if (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest3.id) then
		chest3:open()
	end
end

function logic()
	if (to_caverns2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns2", DIR_S, 83.5*TILE_SIZE, 5.5*TILE_SIZE)
	elseif (to_caverns4:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns4", DIR_S, 13.5*TILE_SIZE, 4.5*TILE_SIZE)
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

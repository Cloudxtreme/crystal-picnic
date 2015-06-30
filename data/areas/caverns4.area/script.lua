is_dungeon = true

function start(game_just_loaded)
	play_music("music/caverns.mid")

	process_outline()

	-- bridge posts
	-- vert
	add_wall_group(2, 7, 7, 1, 1, 0)
	add_wall_group(2, 9, 7, 1, 1, 0)
	add_wall_group(2, 44, 15, 1, 1, 0)
	add_wall_group(2, 46, 15, 1, 1, 0)
	add_wall_group(2, 69, 26, 1, 1, 0)
	add_wall_group(2, 71, 26, 1, 1, 0)
	-- horiz
	add_bridge_post_group(2, 76, 40)
	add_bridge_post_group(2, 86, 40)
	add_bridge_post_group(2, 115, 49)
	add_bridge_post_group(2, 125, 49)
	
	if (not game_just_loaded) then
		add_enemies(2, { "grenade_ant" }, 10, 4, 5, "caverns", { "machete_ant", "scorpion", "grenade_ant", "bat" })
	end

	chest1 = Chest:new{x=11.5*TILE_SIZE, y=20.5*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=WEAPON, contains="IRONARROW", quantity=35, milestone="caverns4_chest1"}
	chest2 = Chest:new{x=60.5*TILE_SIZE, y=11.5*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ACCESSORY, contains="GREENRING", quantity=1, milestone="caverns4_chest2"}
	chest4 = Chest:new{x=141*TILE_SIZE, y=5.5*TILE_SIZE, layer=2, contains_crystals=true, contains="CRYSTAL", quantity=1, milestone="caverns4_chest4"}

	to_caverns3 = Active_Block:new{x=11, y=0, width=5, height=1}
	to_caverns5 = Active_Block:new{x=119, y=0, width=5, height=1}

	start_bat_swoops()
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest4.id) then
		chest4:open()
	end
end

function logic()
	if (to_caverns3:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns3", DIR_N, 77*TILE_SIZE, 58*TILE_SIZE)
	elseif (to_caverns5:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns5", DIR_N, 17.5*TILE_SIZE, 16.5*TILE_SIZE)
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

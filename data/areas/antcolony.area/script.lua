function start()
	play_music("music/other.mid");

	load_sample("sfx/water_cooler.ogg")

	process_outline()

	-- doors
	add_wall_group(2, 8, 34, 2, 2, 0)
	add_wall_group(2, 27, 29, 2, 2, 0)
	add_wall_group(2, 39, 16, 2, 2, 0)
	add_wall_group(2, 60, 16, 2, 2, 0)
	add_wall_group(2, 59, 3, 2, 2, 0)

	-- bar
	add_wall_group(2, 48, 25, 6, 1, 0) -- bar
	add_wall_group(2, 50, 29, 1, 1, 0) -- stools
	add_wall_group(2, 55, 30, 1, 1, 0)
	add_wall_group(2, 52, 32, 1, 1, 0)
	add_wall_group(2, 45, 34, 1, 1, 0)
	add_wall_group(2, 47, 36, 1, 1, 0)
	add_wall_group(2, 52, 37, 1, 1, 0)
	add_wall_group(2, 55, 28, 1, 1, 0) -- tipped stools
	add_wall_group(2, 47, 31, 1, 2, 0)
	add_wall_group(2, 50, 33, 1, 1, 0)
	add_wall_group(2, 53, 33, 1, 2, 0)
	add_wall_group(2, 56, 37, 1, 1, 0)
	add_wall_group(2, 51, 29, 4, 1, 0) -- tables
	add_wall_group(2, 46, 33, 4, 1, 0)

	-- bucket
	add_wall_group(2, 45, 57, 1, 1, 0)
	
	chest1 = Chest:new{x=40*TILE_SIZE, y=6*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ARMOR, contains="SHIRT", quantity=1, milestone="antcolony_chest1"}
	chest2 = Chest:new{x=44*TILE_SIZE+TILE_SIZE/2, y=27*TILE_SIZE, layer=2, contains="HEALTHVIAL", quantity=2, milestone="antcolony_chest2"}

	cooler = add_polygon_entity(2, 7*TILE_SIZE, 18*TILE_SIZE, 9*TILE_SIZE, 18*TILE_SIZE, 9*TILE_SIZE, 20*TILE_SIZE, 7*TILE_SIZE, 20*TILE_SIZE)

	main_exit = Active_Block:new{x=27, y=29, width=2, height=1}
	second_exit = Active_Block:new{x=59, y=3, width=2, height=1}
end

function logic()
	if (main_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("ants", DIR_S, 2630, 2743)
	elseif (second_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("ants", DIR_S, 2598, 1780)
	end
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == cooler) then
		play_water_cooler_motif()
		revive_everyone()
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
	destroy_sample("sfx/water_cooler.ogg")
end

is_dungeon = true

function start(game_just_loaded)
	play_music("music/stonecrater.mid");

	process_outline()

	-- shabby bushes
	add_wall_group(2, 19, 13, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 11, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 28, 14, 3, 2, TILE_GROUP_BUSHES)
	-- cactuses
	add_wall_group(2, 23, 15, 1, 2, 0)
	
	add_ladder(2, 11*TILE_SIZE, 8*TILE_SIZE+10, 2*TILE_SIZE, 5*TILE_SIZE-20)
	
	if (not game_just_loaded) then
		add_enemies(2, { "armadillo", "machete_ant" }, 4, 3, 4, "stonecrater", { "scorpion", "armadillo", "machete_ant", "bazooka_ant" })
	end

	to_map = Active_Block:new{x=10, y=35, width=13, height=1}
	to_stonecrater2 = Active_Block:new{x=34, y=5, width=1, height=2}
	
	chest1 = Chest:new{x=11*TILE_SIZE+TILE_SIZE/2, y=6*TILE_SIZE, layer=2, contains="ANTIDOTE", quantity=5, milestone="stonecrater1_chest1"}
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	end
end

function logic()
	if (to_map:entity_is_colliding(0)) then
		start_map("STONE_CRATER")
	elseif (to_stonecrater2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater2", DIR_E, 64, 877)
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


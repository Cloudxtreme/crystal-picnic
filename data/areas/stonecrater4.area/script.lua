is_dungeon = true

function start(game_just_loaded)
	play_music("music/stonecrater.mid");

	process_outline()

	-- shabby bushes
	add_wall_group(2, 27, 6, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 30, 23, 3, 2, TILE_GROUP_BUSHES)
	-- cactuses
	add_wall_group(2, 19, 13, 1, 2, 0)
	-- bridges
	add_wall_group(2, 26, 12, 1, 1, 0)
	add_wall_group(2, 28, 12, 1, 1, 0)
	
	if (not game_just_loaded) then
		add_enemies(2, { "armadillo", "machete_ant" }, 3, 3, 4, "stonecrater", { "scorpion", "armadillo", "machete_ant", "bazooka_ant" })
	end

	to_stonecrater3 = Active_Block:new{x=5, y=33, width=5, height=1}
	
	chest1 = Chest:new{x=20*TILE_SIZE+TILE_SIZE/2, y=29*TILE_SIZE, layer=2, contains="DIRTYSOCK", quantity=1, milestone="stonecrater4_chest1"}

	to_stonecrater5 = Active_Block:new{x=34, y=8, width=1, height=6}
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	end
end

function logic()
	if (to_stonecrater3:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater3", DIR_S, 32*TILE_SIZE, 5*TILE_SIZE)
	elseif (to_stonecrater5:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater5", DIR_E, 68, 223)
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

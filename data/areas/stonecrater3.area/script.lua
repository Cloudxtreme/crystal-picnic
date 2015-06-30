is_dungeon = true

function start(game_just_loaded)
	play_music("music/stonecrater.mid");

	process_outline()

	-- shabby bushes
	add_wall_group(2, 45, 6, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 8, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 51, 7, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 18, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 40, 20, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 46, 19, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 22, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 50, 41, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 39, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 58, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 52, 40, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 40, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 41, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 57, 40, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 25, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 28, 47, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 5, 81, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 83, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 77, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 15, 69, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 74, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 75, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 11, 78, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 4, 81, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 82, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 14, 84, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 80, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 47, 67, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 82, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 84, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 33, 88, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 47, 66, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 49, 71, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 105, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 34, 107, 3, 2, TILE_GROUP_BUSHES)
	-- cactuses
	add_wall_group(2, 14, 18, 1, 2, 0)
	add_wall_group(2, 53, 14, 1, 2, 0)
	add_wall_group(2, 50, 43, 1, 2, 0)
	add_wall_group(2, 16, 51, 1, 2, 0)
	add_wall_group(2, 9, 46, 1, 2, 0)
	add_wall_group(2, 4, 63, 1, 2, 0)
	add_wall_group(2, 17, 69, 1, 2, 0)
	
	add_ladder(2, 15*TILE_SIZE, 73*TILE_SIZE+10, 2*TILE_SIZE, 5*TILE_SIZE-20)
	add_ladder(2, 16*TILE_SIZE, 13*TILE_SIZE+10, 2*TILE_SIZE, 5*TILE_SIZE-20)
	
	if (not game_just_loaded) then
		add_enemies(2, { "armadillo", "machete_ant" }, 15, 3, 4, "stonecrater", { "scorpion", "armadillo", "machete_ant", "bazooka_ant" })
	end

	to_stonecrater2 = Active_Block:new{x=24, y=119, width=6, height=1}
	to_stonecrater4 = Active_Block:new{x=30, y=0, width=4, height=1}

	-- leave milestones as 2.5!!!
	chest1 = Chest:new{x=9.5*TILE_SIZE, y=110.5*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ACCESSORY, contains="REDRING", quantity=1, milestone="stonecrater2.5_chest1"}
	chest2 = Chest:new{x=51.5*TILE_SIZE, y=112.5*TILE_SIZE, layer=2, contains_crystals=true, contains="CRYSTAL", quantity=1, milestone="stonecrater2.5_chest2"}
	chest3 = Chest:new{x=53.5*TILE_SIZE, y=64.5*TILE_SIZE, layer=2, contains="ANTIDOTE", quantity=2, milestone="stonecrater2.5_chest3"}
	chest4 = Chest:new{x=9.5*TILE_SIZE, y=70.5*TILE_SIZE, layer=2, contains="HEALTHJAR", quantity=1, milestone="stonecrater2.5_chest4"}
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest3.id) then
		chest3:open()
	elseif (activated == chest4.id) then
		chest4:open()
	end
end

function logic()
	if (to_stonecrater2:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("stonecrater2", DIR_S, 846, 61)
	elseif (to_stonecrater4:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater4", DIR_N, 128, 475)
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


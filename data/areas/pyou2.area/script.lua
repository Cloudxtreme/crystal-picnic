is_dungeon = true

function start(game_just_loaded)
	play_music("music/abw2.mid");

	process_outline()

	-- apple trees
	add_wall_group(2, 26, 13, 5, 7, 0)
	add_wall_group(2, 31, 11, 5, 7, 0)
	add_wall_group(2, 33, 19, 5, 7, 0)
	add_wall_group(2, 36, 9, 5, 7, 0)
	add_wall_group(2, 38, 18, 5, 7, 0)
	add_wall_group(2, 41, 8, 5, 7, 0)
	add_wall_group(2, 46, 5, 5, 7, 0)
	add_wall_group(2, 49, 12, 5, 7, 0)
	add_wall_group(2, 51, 2, 5, 7, 0)
	add_wall_group(2, 60, 8, 5, 7, 0)
	add_wall_group(2, 56, 0, 5, 5, 0)
	-- smaller trees
	add_wall_group(2, 14, 3, 4, 6, 0)
	add_wall_group(2, 18, 7, 4, 6, 0)
	add_wall_group(2, 98, 3, 4, 6, 0)
	add_wall_group(2, 101, 10, 4, 6, 0)
	add_wall_group(2, 112, 10, 4, 6, 0)
	add_wall_group(2, 96, 13, 4, 6, 0)
	add_wall_group(2, 90, 21, 4, 6, 0)
	add_wall_group(2, 86, 39, 4, 6, 0)
	add_wall_group(2, 60, 45, 4, 6, 0)
	add_wall_group(2, 70, 72, 4, 6, 0)
	add_wall_group(2, 74, 75, 4, 6, 0)
	-- som bushes
	add_wall_group(2, 19, 23, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 12, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 20, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 85, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 72, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 98, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 51, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 111, 52, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 115, 55, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 106, 59, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 69, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 58, 47, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 12, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 13, 54, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 5, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 7, 60, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 6, 62, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 66, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 67, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 68, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 69, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 85, 72, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 79, 78, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 79, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 78, 81, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 89, 73, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 103, 73, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 96, 74, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 102, 76, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 79, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 104, 78, 2, 2, TILE_GROUP_BUSHES)
	-- mario bushes
	add_wall_group(2, 23, 31, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 3, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 16, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 23, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 23, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 93, 29, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 28, 44, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 49, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 15, 57, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 32, 60, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 63, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 54, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 55, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 76, 55, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 57, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 68, 58, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 62, 62, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 62, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 54, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 97, 57, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 103, 57, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 116, 58, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 94, 81, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 72, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 65, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 67, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 70, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 55, 76, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 77, 2, 2, TILE_GROUP_BUSHES)
	
	if (not game_just_loaded) then
		add_enemies(2, { "ant", "wolf" }, 16, 3, 5, "abw", { "ant", "bazooka_ant", "tough_ant", "bird", "wolf" })
		choppable_array = {}
		choppable_array[1] = { name="bush", layer=2, x=64*TILE_SIZE+8, y=18*TILE_SIZE+8, dead=false }
		choppable_array[2] = { name="bush", layer=2, x=65*TILE_SIZE+8, y=20*TILE_SIZE+8, dead=false }
		choppable_array[3] = { name="bush", layer=2, x=72*TILE_SIZE+8, y=29*TILE_SIZE+8, dead=false }
		choppable_array[4] = { name="bush", layer=2, x=63*TILE_SIZE+8, y=44*TILE_SIZE+8, dead=false }
		choppable_array[5] = { name="bush", layer=2, x=78*TILE_SIZE+8, y=61*TILE_SIZE+8, dead=false }
		choppable_array[6] = { name="bush", layer=2, x=99*TILE_SIZE+8, y=67*TILE_SIZE+8, dead=false }
		choppable_array[7] = { name="bush", layer=2, x=100*TILE_SIZE+8, y=67*TILE_SIZE+8, dead=false }
		choppable_array[8] = { name="bush", layer=2, x=101*TILE_SIZE+8, y=67*TILE_SIZE+8, dead=false }
		choppable_array[9] = { name="bush", layer=2, x=98*TILE_SIZE+8, y=68*TILE_SIZE+8, dead=false }
		choppable_array[10] = { name="bush", layer=2, x=98*TILE_SIZE+8, y=69*TILE_SIZE+8, dead=false }
		choppable_array[11] = { name="bush", layer=2, x=98*TILE_SIZE+8, y=70*TILE_SIZE+8, dead=false }
		choppable_array[12] = { name="bush", layer=2, x=102*TILE_SIZE+8, y=68*TILE_SIZE+8, dead=false }
		choppable_array[13] = { name="bush", layer=2, x=102*TILE_SIZE+8, y=69*TILE_SIZE+8, dead=false }
		choppable_array[14] = { name="bush", layer=2, x=102*TILE_SIZE+8, y=70*TILE_SIZE+8, dead=false }
		choppable_array[15] = { name="bush", layer=2, x=99*TILE_SIZE+8, y=71*TILE_SIZE+8, dead=false }
		choppable_array[16] = { name="bush", layer=2, x=100*TILE_SIZE+8, y=71*TILE_SIZE+8, dead=false }
		choppable_array[17] = { name="bush", layer=2, x=101*TILE_SIZE+8, y=71*TILE_SIZE+8, dead=false }
		choppable_array[18] = { name="bush", layer=2, x=65*TILE_SIZE+8, y=65*TILE_SIZE+8, dead=false }
		choppable_array[19] = { name="bush", layer=2, x=66*TILE_SIZE+8, y=70*TILE_SIZE+8, dead=false }
		choppable_array[20] = { name="bush", layer=2, x=40*TILE_SIZE+8, y=75*TILE_SIZE+8, dead=false }
		choppable_array[21] = { name="bush", layer=2, x=38*TILE_SIZE+8, y=76*TILE_SIZE+8, dead=false }
	end
	spawn_choppable(choppable_array)

	to_dock = Active_Block:new{x=119, y=70, width=1, height=6}
	to_pyou = Active_Block:new{x=0, y=5, width=1, height=6}
	
	chest1 = Chest:new{x=34*TILE_SIZE+TILE_SIZE, y=38*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ARMOR, contains="COVERALLS", quantity=1, milestone="pyou2_chest1"}
	chest2 = Chest:new{x=100*TILE_SIZE+TILE_SIZE/2, y=70*TILE_SIZE, layer=2, contains="HEALTHJAR", quantity=1, milestone="pyou2_chest2"}
	chest3 = Chest:new{x=111*TILE_SIZE+TILE_SIZE/2, y=25*TILE_SIZE, layer=2, contains_gold=true, contains="CASH", quantity=50, milestone="pyou2_chest3"}
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
	update_choppable()

	if (to_dock:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("pyoudock", DIR_E, 39, 137)
	elseif (to_pyou:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("pyou", DIR_W, 1789, 492)
	end
end

function collide(id1, id2)
	collide_with_coins(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
end


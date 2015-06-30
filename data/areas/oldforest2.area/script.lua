is_dungeon = true

function start(game_just_loaded)
	play_music("music/old_forest.mid");

	process_outline()

	-- som bushes
	add_wall_group(2, 49, 9, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 63, 2, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 2, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 12, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 15, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 72, 20, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 23, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 75, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 81, 25, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 26, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 75, 26, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 30, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 41, 38, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 39, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 39, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 40, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 17, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 42, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 10, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 1, 46, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 45, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 13, 47, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 11, 48, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 32, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 81, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 72, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 38, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 58, 42, 2, 2, TILE_GROUP_BUSHES)
	-- mario bushes
	add_wall_group(2, 44, 7, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 23, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 23, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 22, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 81, 28, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 35, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 5, 43, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 15, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 57, 33, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 37, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 38, 3, 2, TILE_GROUP_BUSHES)
	
	add_no_enemy_zone(62, 0, 86, 18)
	
	if (not game_just_loaded) then
		add_enemies(2, { "tough_ant", "ghost" }, 8, 3, 4, "of", { "bazooka_ant", "tough_ant", "machete_ant", "wolf", "faff", "ghost" })
		choppable_array = {}
		choppable_array[1] = { name="of_bush", layer=2, x=18*TILE_SIZE+8, y=27*TILE_SIZE+8, dead=false }
		choppable_array[2] = { name="of_bush", layer=2, x=18*TILE_SIZE+8, y=28*TILE_SIZE+8, dead=false }
		choppable_array[3] = { name="of_bush", layer=2, x=40*TILE_SIZE+8, y=8*TILE_SIZE+8, dead=false }
		choppable_array[4] = { name="of_bush", layer=2, x=42*TILE_SIZE+8, y=8*TILE_SIZE+8, dead=false }
		choppable_array[5] = { name="of_bush", layer=2, x=40*TILE_SIZE+8, y=9*TILE_SIZE+8, dead=false }
		choppable_array[6] = { name="of_bush", layer=2, x=41*TILE_SIZE+8, y=9*TILE_SIZE+8, dead=false }
		choppable_array[7] = { name="of_bush", layer=2, x=57*TILE_SIZE+8, y=12*TILE_SIZE+8, dead=false }
		choppable_array[8] = { name="of_bush", layer=2, x=57*TILE_SIZE+8, y=13*TILE_SIZE+8, dead=false }
		choppable_array[9] = { name="of_bush", layer=2, x=41*TILE_SIZE+8, y=20*TILE_SIZE+8, dead=false }
		choppable_array[10] = { name="of_bush", layer=2, x=70*TILE_SIZE+8, y=7*TILE_SIZE+8, dead=false }
		choppable_array[11] = { name="of_bush", layer=2, x=72*TILE_SIZE+8, y=11*TILE_SIZE+8, dead=false }
		choppable_array[12] = { name="of_bush", layer=2, x=78*TILE_SIZE+8, y=17*TILE_SIZE+8, dead=false }
		choppable_array[13] = { name="of_bush", layer=2, x=63*TILE_SIZE+8, y=25*TILE_SIZE+8, dead=false }
		choppable_array[14] = { name="of_bush", layer=2, x=27*TILE_SIZE+8, y=34*TILE_SIZE+8, dead=false }
		choppable_array[15] = { name="of_bush", layer=2, x=27*TILE_SIZE+8, y=36*TILE_SIZE+8, dead=false }
		choppable_array[16] = { name="of_bush", layer=2, x=64*TILE_SIZE+8, y=33*TILE_SIZE+8, dead=false }
		choppable_array[17] = { name="of_bush", layer=2, x=79*TILE_SIZE+8, y=35*TILE_SIZE+8, dead=false }
		choppable_array[18] = { name="of_bush", layer=2, x=57*TILE_SIZE+8, y=36*TILE_SIZE+8, dead=false }
		choppable_array[19] = { name="of_bush", layer=2, x=76*TILE_SIZE+8, y=36*TILE_SIZE+8, dead=false }
		choppable_array[20] = { name="of_bush", layer=2, x=63*TILE_SIZE+8, y=37*TILE_SIZE+8, dead=false }
		choppable_array[21] = { name="of_bush", layer=2, x=53*TILE_SIZE+8, y=39*TILE_SIZE+8, dead=false }
		choppable_array[22] = { name="of_bush", layer=2, x=54*TILE_SIZE+8, y=39*TILE_SIZE+8, dead=false }
		choppable_array[23] = { name="of_bush", layer=2, x=55*TILE_SIZE+8, y=39*TILE_SIZE+8, dead=false }
		choppable_array[24] = { name="of_bush", layer=2, x=54*TILE_SIZE+8, y=42*TILE_SIZE+8, dead=false }
		choppable_array[25] = { name="of_bush", layer=2, x=55*TILE_SIZE+8, y=42*TILE_SIZE+8, dead=false }
		choppable_array[26] = { name="of_bush", layer=2, x=56*TILE_SIZE+8, y=42*TILE_SIZE+8, dead=false }
	end
	spawn_choppable(choppable_array)

	to_of1 = Active_Block:new{x=0, y=42, width=1, height=6}
	to_oldoak = Active_Block:new{x=62, y=0, width=11, height=1}

	chest = Chest:new{x=40*TILE_SIZE, y=41*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=WEAPON, contains="CLEAVER", quantity=1, milestone="of2_chest1"}
	
	flea_items = add_entity("flea_items", 2, 1184, 229)
	set_entity_direction(flea_items, DIR_S)
	load_sample("sfx/flea_items.ogg", false)
end

function enable_input(tween, id, elapsed)
	set_entity_input_disabled(0, false)
	return true
end

function activate(activator, activated)
	if (activated == chest.id) then
		chest:open()
	elseif (activated == flea_items) then
		play_sample("sfx/flea_items.ogg", 1, 0, 1)
		set_entity_input_disabled(0, true)
		speak(false, false, true, t("FLEA_OF2"), "", flea_items, flea_items)
		local tbl = create_idle_tween(0.5)
		append_tween(tbl, { run = enable_input })
		new_tween(tbl)
		do_item_shop(
			"HEALTHVIAL", 5,
			"MAGICVIAL", 15,
			"DIRTYSOCK", 75
		)
	end
end

function logic()
	update_choppable()

	if (to_of1:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("oldforest1", DIR_W, 1498, 1649)
	elseif (to_oldoak:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("oldoak", DIR_N, 295, 749)
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
	destroy_sample("sfx/flea_items.ogg")
end


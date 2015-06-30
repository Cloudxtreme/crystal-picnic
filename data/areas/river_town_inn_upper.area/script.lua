function start()
	play_music("music/river_town.mid")

	load_sample("sfx/water_cooler.ogg")

	process_outline()

	add_tile_group(
		2,
		1*TILE_SIZE,
		6*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		1, 5,
		2, 5,
		3, 5,
		2, 4,
		3, 4,
		4, 4
	)

	pig = add_npc("pig", 2, 151, 136)
	set_character_role(pig, "wander", 128, 4, 8)

	cooler = add_polygon_entity(2, 15*TILE_SIZE, 3*TILE_SIZE, 17*TILE_SIZE, 3*TILE_SIZE, 17*TILE_SIZE, 4.5*TILE_SIZE, 15*TILE_SIZE, 4.5*TILE_SIZE)

	down = Active_Block:new{x=2, y=4, width=2, height=2}
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=11.5*TILE_SIZE, y=5*TILE_SIZE, layer=2, contains_gold=true, contains="CASH", quantity=1, milestone="inn_bed_chest"}
end

function logic()
	if (down:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("river_town_inn_lower", DIR_S, 87, 77)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		if (done_pyou1()) then
			speak(false, false, true, t("INN_PIG_3"), "", pig, pig)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("INN_PIG_2"), "", pig, pig)
		else
			speak(false, false, true, t("INN_PIG_1"), "", pig, pig)
		end
	elseif (activated == cooler) then
		play_water_cooler_motif()
		revive_everyone()
	elseif (activated == chest.id) then
		chest:open()
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

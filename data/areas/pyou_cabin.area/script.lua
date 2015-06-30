LEFT_AREA = "left_pyou_area"

function start()
	play_music("music/other.mid");
	
	load_sample("sfx/water_cooler.ogg")

	process_outline()

	-- chairs
	add_wall_group(2, 3, 7, 1, 2, 0)
	add_wall_group(2, 7, 7, 1, 2, 0)
	-- table
	add_wall_group(2, 4, 7, 3, 3, 0)
	-- crate
	add_wall_group(2, 1, 8, 1, 2, 0)
	-- bucket
	add_wall_group(2, 13, 9, 1, 1, 0)
	-- bottom table
	add_wall_group(2, 9, 10, 5, 2, 0)

	if (milestone_is_complete(LEFT_AREA)) then
		pyou = add_npc("pyou", 2, 120, 120)
	end

	exit = Active_Block:new{x=3, y=12, width=2, height=1}
	cooler = add_polygon_entity(2, 3*TILE_SIZE, 4*TILE_SIZE, 5*TILE_SIZE, 4*TILE_SIZE, 5*TILE_SIZE, 6*TILE_SIZE, 3*TILE_SIZE, 6*TILE_SIZE)
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=11.5*TILE_SIZE, y=5.5*TILE_SIZE, layer=2, contains_gold=true, contains="CASH", quantity=10, milestone="pyou_chest"}
end

function activate(activator, activated)
	if (not (pyou == nil) and activated == pyou) then
		if (done_oldoak()) then
			simple_speak{
				true,
				"PYOU_CABIN_2", "", pyou
			}
		else
			simple_speak{
				true,
				"PYOU_CABIN_1", "", pyou
			}
		end
	elseif (activated == cooler) then
		play_water_cooler_motif()
		revive_everyone()
	elseif (activated == chest.id) then
		chest:open()
	end
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("pyou", DIR_S, 1478, 225)
	end
end

function collide(id1, id2)
end

function uncollide(id1, id2)
end

function attacked(attacker, attackee)
end

function stop()
	destroy_sample("sfx/water_cooler.ogg")
end

function start()
	play_music("music/river_town.mid")

	process_outline()

	add_wall_group(2, 16, 6, 3, 2, 0)

	vert = add_entity("mill_vertical", 3, 248+TILE_SIZE*2, 112)
	horz = add_entity("mill_horizontal", 4, 269+TILE_SIZE*2, 77)

	turtle = add_entity("turtle_female", 2, 231, 127)
	set_entity_direction(turtle, DIR_E)

	exit = Active_Block:new{x=4, y=12, width=2, height=1}
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=3.5*TILE_SIZE, y=5*TILE_SIZE, layer=2, contains="HEALTHVIAL", quantity=1, milestone="mill_chest"}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 1064, 703)
	end
end

function activate(activator, activated)
	if (activated == turtle) then
		if (done_oldoak()) then
			speak(false, false, true, t("MILL_TURTLE1_3"), "", turtle, turtle)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("MILL_TURTLE1_2"), "", turtle, turtle)
		else
			speak(false, false, true, t("MILL_TURTLE1_1"), "", turtle, turtle)
		end
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
end

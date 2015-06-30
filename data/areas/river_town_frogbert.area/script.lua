function start()
	play_music("music/river_town.mid")

	process_outline()

	squirrel = add_npc("squirrel", 2, 74, 123)
	set_character_role(squirrel, "wander", 80, 3, 3)

	exit = Active_Block:new{x=9, y=9, width=2, height=2}
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=3.5*TILE_SIZE, y=5*TILE_SIZE, layer=2, contains_gold=true, contains="CASH", quantity=5, milestone="frogbert_chest"}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 280, 305)
	end
end

function activate(activator, activated)
	if (activated == squirrel) then
		if (done_oldoak()) then
			speak(false, false, true, t("FROGBERT_HOUSE_SQUIRREL_4"), "", squirrel, squirrel)
		elseif (done_pyou1()) then
			speak(false, false, true, t("FROGBERT_HOUSE_SQUIRREL_3"), "", squirrel, squirrel)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("FROGBERT_HOUSE_SQUIRREL_2"), "", squirrel, squirrel)
		else
			speak(false, false, true, t("FROGBERT_HOUSE_SQUIRREL_1"), "", squirrel, squirrel)
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

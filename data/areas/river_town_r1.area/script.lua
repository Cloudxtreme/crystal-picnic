function start()
	play_music("music/river_town.mid")

	process_outline()

	add_wall_group(2, 12, 3, 2, 3, 0)

	cat = add_npc("cat_female", 2, 134, 118)
	set_character_role(cat, "wander", 96, 1, 2)

	exit = Active_Block:new{x=10, y=10, width=2, height=1}
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=15.5*TILE_SIZE, y=4.5*TILE_SIZE, layer=2, contains="HEALTHJAR", quantity=1, milestone="r1_chest"}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 1147, 236)
	end
end

function activate(activator, activated)
	if (activated == cat) then
		if (done_pyou1()) then
			speak(false, false, true, t("RIVER_TOWN_R1_CAT_3"), "", cat, cat)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("RIVER_TOWN_R1_CAT_2"), "", cat, cat)
		else
			speak(false, false, true, t("RIVER_TOWN_R1_CAT_1"), "", cat, cat)
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

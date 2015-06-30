function start()
	play_music("music/river_town.mid")

	process_outline()

	female = add_npc("cat_female", 2, 144, 128)
	set_character_role(female, "wander", 64, 2, 4)

	exit = Active_Block:new{x=8, y=11, width=2, height=1}
	up = Active_Block:new{x=2, y=3, width=1, height=2}
	
	chest = Chest:new{sound="sfx/item_found.ogg", entity_name="invisible_chest", x=2.5*TILE_SIZE, y=10*TILE_SIZE, layer=2, contains="MAGICJAR", quantity=1, milestone="r3_chest"}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 810, 1024)
	elseif (up:entity_is_colliding(0)) then
		next_player_layer = 4
		change_areas("river_town", DIR_S, 799, 920)
	end
end

function activate(activator, activated)
	if (activated == female) then
		speak(false, false, true, t("RIVER_TOWN_R3_FEMALE_1"), "", female, female)
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

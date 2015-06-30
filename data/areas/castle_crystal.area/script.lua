function start()
	play_music("music/castle.mid");

	process_outline()

	-- rails going up
	add_wall_group(2, 1, 30, 4, 2, 0)
	add_tile_group(
		2,
		5*TILE_SIZE,
		35*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		5,
		0,
		5, 32,
		5, 33,
		6, 33,
		7, 33,
		8, 33,
		7, 34,
		8, 34
	)

	-- Pillars
	add_wall_group(2, 9, 4, 2, 6, 0)
	add_wall_group(2, 9, 10, 2, 6, 0)
	add_wall_group(2, 25, 4, 2, 6, 0)
	add_wall_group(2, 25, 10, 2, 6, 0)

	going_up = Active_Block:new{x=4, y=34, width=2, height=2}

	pig1 = add_npc("pig_guard", 2, 151, 545)
	set_entity_direction(pig1, DIR_S)

	pig2 = add_npc("pig_guard", 2, 319, 102)
	set_entity_direction(pig2, DIR_S)

	pig3 = add_npc("pig_guard", 2, 464, 190)
	set_character_role(pig3, "wander", 96, 1.0, 3.0)

	pig4 = add_npc("pig_guard", 2, 111, 190)
	set_character_role(pig4, "wander", 96, 1.0, 3.0)
	
	chest = Chest:new{x=200, y=96, layer=2, contains_gold=true, contains="CASH", quantity=100, milestone="castle_crystal_cash"}
end

function logic()
	if (going_up:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_banquet", DIR_W, 896, 663)
	end
end

function activate(activator, activated)
	if (activated == pig1) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_CRYSTAL_1_4"), "", pig1, pig1)
		elseif (done_pyou1()) then
			speak(false, false, true, t("PIG_CRYSTAL_1_3"), "", pig1, pig1)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("PIG_CRYSTAL_1_2"), "", pig1, pig1)
		else
			speak(false, false, true, t("PIG_CRYSTAL_1"), "", pig1, pig1)
		end
	elseif (activated == pig2) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_CRYSTAL_2_4"), "", pig2, pig2)
		elseif (done_pyou1()) then
			speak(false, false, true, t("PIG_CRYSTAL_2_3"), "", pig2, pig2)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("PIG_CRYSTAL_2_2"), "", pig2, pig2)
		else
			speak(false, false, true, t("PIG_CRYSTAL_2"), "", pig2, pig2)
		end
	elseif (activated == pig3) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_CRYSTAL_3_4"), "", pig3, pig3)
		elseif (done_pyou1()) then
			speak(false, false, true, t("PIG_CRYSTAL_3_3"), "", pig3, pig3)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("PIG_CRYSTAL_3_2"), "", pig3, pig3)
		else
			speak(false, false, true, t("PIG_CRYSTAL_3"), "", pig3, pig3)
		end
	elseif (activated == pig4) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_CRYSTAL_4_4"), "", pig4, pig4)
		elseif (done_pyou1()) then
			speak(false, false, true, t("PIG_CRYSTAL_4_3"), "", pig4, pig4)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("PIG_CRYSTAL_4_2"), "", pig4, pig4)
		else
			speak(false, false, true, t("PIG_CRYSTAL_4"), "", pig4, pig4)
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

local seconds = 0

function start()
	play_music("music/castle.mid")

	process_outline()

	load_sample("sfx/roar.ogg")

	queen = add_entity("queen_female", 2, 297, 94)
	king = add_entity("king", 2, 377, 100)
	advisor = add_npc("turtle_advisor", 2, 448, 239)
	set_entity_direction(advisor, DIR_S)
	knight = add_entity("knight", 2, 423, 789)
	pig1 = add_npc("pig_guard", 2, 223, 239)
	set_entity_direction(pig1, DIR_S)
	pig2 = add_npc("pig_guard", 2, 199, 583)
	pig3 = add_npc("pig_guard", 2, 471, 583)
	set_entity_direction(pig2, DIR_E)
	set_entity_direction(pig3, DIR_W)
	set_entity_direction(knight, DIR_S)

	cat = add_npc("cat", 2, 335, 718)
	set_character_role(cat, "wander", 200, 2, 4)
	
	load_sample("sfx/boing.ogg", false)

	to_main_entrance = Active_Block:new{x=19, y=58, width=4, height=2}
	left_exit = Active_Block:new{x=0, y=40, width=1, height=5}
	right_exit = Active_Block:new{x=41, y=40, width=1, height=5}
end

function logic()
	if (to_main_entrance:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_main_entrance", DIR_S, 191, 103)
	elseif (left_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_hall", DIR_W, 652, 429)
	elseif (right_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_hall", DIR_E, 1382, 429)
	end
end

function activate(activator, activated)
	if (activated == queen) then
		if (done_oldoak()) then
			speak(false, false, true, t("QUEEN_4"), "talk", queen)
		elseif (done_pyou1()) then
			speak(false, false, true, t("QUEEN_3"), "talk", queen)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("QUEEN_2"), "talk", queen)
		else
			speak(false, false, true, t("QUEEN_1"), "talk", queen)
		end
	elseif (activated == king) then
		if (done_oldoak()) then
			speak(false, false, true, t("KING_4"), "talk", king)
		elseif (done_pyou1()) then
			speak(false, false, true, t("KING_3"), "talk", king)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("KING_2"), "talk", king)
		else
			play_sample("sfx/boing.ogg", 1, 0, 1)
			for i=0,(get_num_players()-1) do
				local id = get_player_id(i)
				stop_entity(id)
				set_entity_role_paused(id, true)
				set_entity_animation(id, "surprised")
			end
			speak(false, false, true, t("KING_1"), "talk", king)
			for i=0,(get_num_players()-1) do
				local id = get_player_id(i)
				set_entity_animation(id, "idle-up")
				set_entity_role_paused(id, false)
			end
		end
	elseif (activated == advisor) then
		if (done_amaysa1()) then
			speak(false, false, true, t("ADVISOR_THRONE_2"), "", advisor, advisor)
		else
			speak(false, false, true, t("ADVISOR_THRONE_1"), "", advisor, advisor)
		end
	elseif (activated == knight) then
		speak(false, false, true, t("KNIGHT_THRONE_1"), "", knight, knight)
	elseif (activated == pig1) then
		if (done_amaysa1()) then
			speak(false, false, true, t("PIG1_THRONE_2"), "", pig1, pig1)
		else
			speak(false, false, true, t("PIG1_THRONE_1"), "", pig1, pig1)
		end
	elseif (activated == pig2) then
		speak(false, false, true, t("PIG2_THRONE_1"), "", pig2, pig2)
	elseif (activated == pig3) then
		speak(false, false, true, t("PIG3_THRONE_1"), "", pig3, pig3)
	elseif (activated == cat) then
		speak(false, false, true, t("CAT_THRONE_1"), "", cat, cat)
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
	destroy_sample("sfx/roar.ogg")
	destroy_sample("sfx/boing.ogg")
end

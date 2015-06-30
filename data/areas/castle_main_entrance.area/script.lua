function start()
	play_music("music/castle.mid");

	process_outline()

	add_wall_group(2, 4, 7, 1, 6, 0)
	add_wall_group(2, 19, 7, 1, 6, 0)

	pig = add_entity("pig_guard", 2, 280, 132)
	set_entity_direction(pig, DIR_S)

	to_banquet = Active_Block:new{x=11, y=10, width=2, height=2}
	to_courtyard = Active_Block:new{x=10, y=21, width=4, height=2}
	to_throne = Active_Block:new{x=11, y=3, width=2, height=2}
end

function logic()
	if (to_banquet:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_banquet", DIR_N, 496, 736)
	elseif (to_courtyard:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle", DIR_S, 576, 912)
	elseif (to_throne:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_throne", DIR_N, 336, 912)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		if (done_oldoak()) then
			speak(false, false, true, t("PIG_ENTRANCE_3"), "", pig, pig)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("PIG_ENTRANCE_2"), "", pig, pig)
		else
			speak(false, false, true, t("PIG_ENTRANCE_1"), "", pig, pig)
		end
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

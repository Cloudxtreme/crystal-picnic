function start()
	play_music("music/castle.mid");

	process_outline()

	add_wall_group(2, 1, 6, 2, 3, 0)
	add_wall_group(2, 6, 6, 2, 3, 0)
	add_wall_group(2, 3, 6, 3, 4, 0)

	pig = add_npc("pig_guard", 2, 71, 182)
	set_entity_direction(pig, DIR_S)

	going_down = Active_Block:new{x=9, y=10, width=3, height=2}
	going_out = Active_Block:new{x=5, y=13, width=3, height=2}
end

function logic()
	if (going_down:entity_is_colliding(0)) then
		next_player_layer = 4
		change_areas("castle_knight_left1", DIR_S, 246, 86)
	elseif (going_out:entity_is_colliding(0)) then
		next_player_layer = 6
		change_areas("castle", DIR_S, 320, 391)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		speak(false, false, true, t("PIG_TOWER_7"), "", pig, pig)
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

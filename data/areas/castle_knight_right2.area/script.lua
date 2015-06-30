function start()
	play_music("music/castle.mid");

	process_outline()

	add_wall_group(3, 10, 6, 2, 3, 0)

	pig = add_npc("pig_guard", 3, 119, 134)
	set_character_role(pig, "wander", 48, 0.25, 1.0)

	going_down = Active_Block:new{x=1, y=10, width=3, height=2}
	going_out = Active_Block:new{x=5, y=13, width=3, height=2}
end

function logic()
	if (going_down:entity_is_colliding(0)) then
		next_player_layer = 4
		change_areas("castle_knight_right1", DIR_S, 38, 86)
	elseif (going_out:entity_is_colliding(0)) then
		next_player_layer = 6
		change_areas("castle", DIR_S, 832, 391)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		speak(false, false, true, t("PIG_TOWER_6"), "", pig, pig)
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

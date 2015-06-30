function start()
	play_music("music/castle.mid");

	process_outline()

	pig = add_npc("pig_guard", 2, 111, 157)
	set_character_role(pig, "wander", 96, 0.25, 1.0)

	chest1 = Chest:new{x=80, y=96, layer=2, contains="ANTIDOTE", quantity=1, milestone="castle_tower3_chest1"}
	chest2 = Chest:new{x=128, y=96, layer=2, contains="HEALTHVIAL", quantity=2, milestone="castle_tower3_chest2"}

	going_down = Active_Block:new{x=12, y=5, width=2, height=2}
end

function logic()
	if (going_down:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_tower2", DIR_S, 208, 135)
	end
end

function activate(activator, activated)
	if (activated == pig) then
		if (done_pyou1()) then
			speak(false, false, true, t("PIG_TOWER_3_2"), "", pig, pig)
		else
			speak(false, false, true, t("PIG_TOWER_3"), "", pig, pig)
		end
	elseif (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
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

function start()
	play_music("music/river_town.mid")

	process_outline()

	add_wall_group(2, 12, 7, 3, 2, 0)

	turtle = add_entity("turtle_female", 2, 199, 122)
	set_entity_direction(turtle, DIR_S)

	exit = Active_Block:new{x=3, y=9, width=2, height=1}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 968, 511)
	end
end

function activate(activator, activated)
	if (activated == turtle) then
		if (done_oldoak()) then
			speak(false, false, true, t("RIVER_TOWN_R2_TURTLE_3"), "", turtle, turtle)
		elseif (done_pyou1()) then
			speak(false, false, true, t("RIVER_TOWN_R2_TURTLE_2"), "", turtle, turtle)
		else
			speak(false, false, true, t("RIVER_TOWN_R2_TURTLE_1"), "", turtle, turtle)
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

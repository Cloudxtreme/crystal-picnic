function start()
	play_music("music/river_town.mid")

	process_outline()

	squirrel = add_entity("squirrel", 2, 32, 88)
	set_entity_direction(squirrel, DIR_N)

	exit = Active_Block:new{x=2, y=13, width=2, height=1}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 456, 864)
	end
end

function activate(activator, activated)
	if (activated == squirrel) then
		if (done_oldoak()) then
			speak(false, false, true, t("EGBERT_HOUSE_SQUIRREL_4"), "", squirrel, squirrel)
		elseif (done_pyou1()) then
			speak(false, false, true, t("EGBERT_HOUSE_SQUIRREL_3"), "", squirrel, squirrel)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("EGBERT_HOUSE_SQUIRREL_2"), "", squirrel, squirrel)
		else
			speak(false, false, true, t("EGBERT_HOUSE_SQUIRREL_1"), "", squirrel, squirrel)
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

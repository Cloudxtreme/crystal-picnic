function start()
	play_music("music/river_town.mid")

	process_outline()

	add_wall_group(3, 10, 5, 1, 2, 0)
	add_wall_group(3, 10, 7, 1, 2, 0)
	add_wall_group(3, 1, 5, 1, 2, 0)
	add_wall_group(3, 1, 8, 1, 2, 0)
	add_wall_group(3, 5, 5, 1, 2, 0)
	add_wall_group(3, 5, 8, 1, 2, 0)
	add_wall_group(3, 2, 5, 3, 2, 0)
	add_wall_group(3, 2, 8, 3, 2, 0)

	add_tile_group(
		3,
		1*TILE_SIZE,
		5*TILE_SIZE-TILE_SIZE,
		4*TILE_SIZE,
		10,
		0,
		1, 2,
		2, 2,
		1, 3,
		2, 3,
		3, 3,
		4, 3,
		3, 4,
		4, 4
	)

	owner = add_npc("turtle", 3, 201, 119)
	set_character_role(owner, "wander", 40, 1, 2)
	fox = add_entity("fox", 3, 167, 92)
	set_entity_direction(fox, DIR_E)

	exit = Active_Block:new{x=3, y=12, width=2, height=1}
	up = Active_Block:new{x=2, y=2, width=1, height=2}
end

function logic()
	if (exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town", DIR_S, 720, 204)
	elseif (up:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("river_town_inn_upper", DIR_E, 80, 80)
	end
end

function activate(activator, activated)
	if (activated == owner) then
		speak(false, false, true, t("INN_OWNER_1"), "", owner, owner)
	elseif (activated == fox) then
		if (done_oldoak()) then
			speak(false, false, true, t("INN_FOX_4"), "", fox, fox)
		elseif (done_pyou1()) then
			speak(false, false, true, t("INN_FOX_3"), "", fox, fox)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("INN_FOX_2"), "", fox, fox)
		else
			speak(false, false, true, t("INN_FOX_1"), "", fox, fox)
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

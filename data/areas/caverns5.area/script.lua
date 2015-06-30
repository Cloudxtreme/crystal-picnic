function start(game_just_loaded)
	play_music("music/caverns.mid")

	process_outline()

	if  (not milestone_is_complete("chest_puzzle")) then
		add_polygon_entity(2, 31*TILE_SIZE, 8*TILE_SIZE, 32*TILE_SIZE, 8*TILE_SIZE, 32*TILE_SIZE, 13*TILE_SIZE, 31*TILE_SIZE, 13*TILE_SIZE)
	else
		local w, h = get_area_tile_size()
		local x, y
		for y=1,h-1 do
			for x=1,w-1 do
				set_tile(4, x, y, -1, -1, false)
				set_tile(5, x, y, -1, -1, false)
			end
		end
	end

	to_caverns4 = Active_Block:new{x=15, y=19, width=5, height=1}

	to_caverns6a = Active_Block:new{x=7, y=5, width=3, height=1}
	to_caverns6b = Active_Block:new{x=16, y=5, width=3, height=1}
	to_caverns6c = Active_Block:new{x=25, y=5, width=3, height=1}
	to_caverns7 = Active_Block:new{x=36, y=19, width=6, height=1}
	
	flea_items = add_entity("flea_items", 2, 29.5*TILE_SIZE, 9.5*TILE_SIZE)
	set_entity_direction(flea_items, DIR_S)
	load_sample("sfx/flea_items.ogg", false)
end

function enable_input(tween, id, elapsed)
	set_entity_input_disabled(0, false)
	return true
end

function activate(activator, activated)
	if (activated == flea_items) then
		play_sample("sfx/flea_items.ogg", 1, 0, 1)
		set_entity_input_disabled(0, true)
		speak(false, false, true, t("FLEA_CAVERNS"), "", flea_items, flea_items)
		local tbl = create_idle_tween(0.5)
		append_tween(tbl, { run = enable_input })
		new_tween(tbl)
		do_item_shop(
			"HEALTHJAR", 25,
			"MAGICJAR", 60,
			"DIRTYSOCK", 75
		)
	end
end

function logic()
	if (to_caverns4:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns4", DIR_S, 121.5*TILE_SIZE, 4.5*TILE_SIZE)
	elseif (to_caverns6a:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns6a", DIR_N, 13.5*TILE_SIZE, 13.5*TILE_SIZE)
	elseif (to_caverns6b:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns6b", DIR_N, 13.5*TILE_SIZE, 13.5*TILE_SIZE)
	elseif (to_caverns6c:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns6c", DIR_N, 13.5*TILE_SIZE, 13.5*TILE_SIZE)
	elseif (to_caverns7:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("caverns7", DIR_S, 9*TILE_SIZE, 5*TILE_SIZE)
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


local seconds = 0
local top_x, top_y

function start()
	play_music("music/castle.mid");

	process_outline()

	-- Queen's
	add_wall_group(2, 2, 27, 3, 3, 0)
	add_wall_group(2, 5, 27, 2, 3, 0)

	-- Banquet
	-- Table
	add_wall_group(2, 28, 10, 6, 17, 0)

	-- Chairs
	add_wall_group(2, 26, 11, 2, 3, 0)
	add_wall_group(2, 26, 14, 2, 3, 0)
	add_wall_group(2, 26, 17, 2, 3, 0)
	add_wall_group(2, 26, 20, 2, 3, 0)
	add_wall_group(2, 26, 23, 2, 3, 0)
	add_wall_group(2, 34, 11, 2, 3, 0)
	add_wall_group(2, 34, 14, 2, 3, 0)
	add_wall_group(2, 34, 17, 2, 3, 0)
	add_wall_group(2, 34, 20, 2, 3, 0)
	add_wall_group(2, 34, 23, 2, 3, 0)

	-- Kitchen
	add_wall_group(2, 43, 25, 2, 5, 0)
	add_wall_group(2, 54, 25, 2, 3, 0)
	add_wall_group(2, 59, 26, 2, 2, 0)

	-- cutting table
	add_tile_group(
		2,
		50*TILE_SIZE,
		25*TILE_SIZE,
		4*TILE_SIZE,
		1,
		0,
		50, 24,
		51, 24,
		52, 24,
		53, 24
	)

	stove = load_bitmap("misc_graphics/stove.png")
	stoveglow = load_bitmap("misc_graphics/stoveglow.png")
	stovefloor = load_bitmap("misc_graphics/stovefloor.png")

	add_entity("fire", 1, 54*TILE_SIZE, 21*TILE_SIZE-10)

	chopper = add_entity("chopping_squirrel_female", 2, 823, 408)
	stirrer = add_entity("stirring_squirrel", 2, 876, 465)
	chef = add_entity("chef_squirrel_female", 2, 810, 347)
	set_entity_animation(chef, "idle-down")
	runner = add_npc("kitchen_squirrel_female", 2, 759, 478)
	set_character_role(runner, "astar")
	local t = create_astar_tween(runner, 922, 478, false)
	t.next_tween = create_idle_tween(2)
	t.next_tween.next_tween = create_astar_tween(runner, 831, 368, false)
	t.next_tween.next_tween.next_tween = create_idle_tween(2)
	t.next_tween.next_tween.next_tween.next_tween = create_astar_tween(runner, 759, 478, false)
	t.next_tween.next_tween.next_tween.next_tween.next_tween = create_idle_tween(2)
	t.next_tween.next_tween.next_tween.next_tween.next_tween.next_tween = t
	new_tween(t)

	fox = add_npc("fox", 2, 207, 657)
	set_character_role(fox, "astar")
	local t = create_idle_tween(5)
	append_tween(t, create_astar_tween(fox, 448, 657, false))
	append_tween(t, create_idle_tween(5))
	append_tween(t, create_astar_tween(fox, 207, 657, false))
	append_tween(t, t)
	new_tween(t)

	knight = add_entity("knight", 2, 407, 230)
	set_entity_direction(knight, DIR_S)

	to_main_entrance = Active_Block:new{x=29, y=47, width=4, height=2}
	to_crystal = Active_Block:new{x=59, y=41, width=2, height=2}

	chest = Chest:new{x=280, y=384, layer=2, contains_equipment=true, equipment_type=ACCESSORY, contains="HOCKEYMASK", quantity=1, milestone="castle_banquet_hockeymask"}

end

function logic()
	seconds = seconds + LOGIC_MILLIS / 1000.0

	if (to_main_entrance:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_main_entrance", DIR_S, 192, 207)
	elseif (to_crystal:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_crystal", DIR_E, 151, 565)
	end
end

function pre_draw_layer(layer)
	local tx, ty = get_area_top()
	-- These can be nil in speech loop
	if (not (tx == nil)) then
		top_x = tx
		top_y = ty
	end
	if (layer == 1) then
		draw_bitmap(stovefloor, 54*TILE_SIZE-85/2-top_x, 21*TILE_SIZE-14-top_y, 0)
		draw_bitmap(stove, 54*TILE_SIZE-74/2-top_x, 21*TILE_SIZE-95-top_y, 0)
	elseif (layer == 2) then
		draw_bitmap(stoveglow, 54*TILE_SIZE-74/2-top_x, 21*TILE_SIZE-95-top_y, 0)
		local alpha = seconds % 2
		if (alpha > 1) then
			alpha = 1 - (alpha - 1)
		end
		draw_bitmap_additive(stoveglow, 54*TILE_SIZE-74/2-top_x, 21*TILE_SIZE-95-top_y, alpha, 0)
	end
end

function activate(activator, activated)
	if (activated == chopper) then
		if (done_pyou1()) then
			speak(false, false, true, t("KITCHEN_SQUIRREL1_2"), "", chopper, chopper)
		else
			speak(false, false, true, t("KITCHEN_SQUIRREL1_1"), "", chopper, chopper)
		end
	elseif (activated == stirrer) then
		speak(false, false, true, t("KITCHEN_SQUIRREL2_1"), "", stirrer, stirrer)
	elseif (activated == chef) then
		if (done_oldoak()) then
			speak(false, false, true, t("KITCHEN_SQUIRREL3_3"), "", chef, chef)
		elseif (done_amaysa1()) then
			speak(false, false, true, t("KITCHEN_SQUIRREL3_2"), "", chef, chef)
		else
			speak(false, false, true, t("KITCHEN_SQUIRREL3_1"), "", chef, chef)
		end
	elseif (activated == runner) then
		if (done_amaysa1()) then
			speak(false, false, true, t("KITCHEN_SQUIRREL4_2"), "", runner, runner)
		else
			speak(false, false, true, t("KITCHEN_SQUIRREL4_1"), "", runner, runner)
		end
	elseif (activated == fox) then
		speak(false, false, true, t("FOX_BANQUET_1"), "", fox, fox)
	elseif (activated == knight) then
		speak(false, false, true, t("PIG_BANQUET_1"), "", knight, knight)
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
	destroy_bitmap("misc_graphics/stove.png")
	destroy_bitmap("misc_graphics/stoveglow.png")
	destroy_bitmap("misc_graphics/stovefloor.png")
end

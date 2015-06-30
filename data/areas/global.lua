TILE_GROUP_BUSHES = 1

Door = {}
Chest = {}
Chest_Puzzle = {}
Active_Block = {}

function Door:collide()
	if (not self.open) then
		if (self.sound == true) then
			play_sample("sfx/open_door.ogg", 1, 0, 1)
		end
		self.open = true
		set_entity_animation(self.id, "open")
		set_entity_solid_with_entities(self.id, false)
	end
end

-- entity_name, layer, x, y, open, sound
function Door:new(d)
	d = d or {}
	setmetatable(d, self)
	self.__index = self

	d.id = add_entity(d.entity_name, d.layer, d.x, d.y)
	
	if (d.open == nil) then
		d.open = false
	end

	if (d.open) then
		set_entity_animation(d.id, "open")
	else
		set_entity_animation(d.id, "closed")
	end

	return d
end

function Chest:open()
	if (not self.is_open) then
		if (not (self.sound == nil)) then
			play_sample(self.sound, 1, 0, 1)
		else
			play_sample("sfx/chest_open.ogg", 1, 0, 1)
			local t = create_idle_tween(0.5)
			append_tween(t, create_play_sample_tween("sfx/item_found.ogg"))
			new_tween(t)
		end
		self.is_open = true
		set_milestone_complete(self.milestone, true)
		set_entity_animation(self.id, "open")
		if (self.contains_equipment) then
			give_equipment(self.equipment_type, self.contains, self.quantity)
		elseif (self.contains_gold) then
			add_cash(self.quantity)
		elseif (self.contains_crystals) then
			add_crystals(self.quantity)
		else
			give_items(self.contains, self.quantity)
		end
		if (self.quantity == 1) then
			speak(
				true,
				false,
				true,
				string.format(t("FOUND_ONE"), t(self.contains)),
				"",
				0
			)
		else
			speak(
				true,
				false,
				true,
				string.format(t("FOUND_MULTIPLE"), self.quantity, t(self.contains)),
				"",
				0
			)
		end
	end
end

function Chest:new(c)
	c = c or {}
	setmetatable(c, self)
	self.__index = self

	c.id = add_entity(c.entity_name or "chest", c.layer, c.x, c.y)

	if (milestone_is_complete(c.milestone)) then
		c.is_open = true
	end

	if (c.is_open) then
		set_entity_animation(c.id, "open")
	else
		set_entity_animation(c.id, "closed")
	end

	return c
end

function Chest_Puzzle:open()
	if (not self.is_open) then
		local found

		if (milestone_is_complete(self.other_ms_1) == self.other1 and milestone_is_complete(self.other_ms_2) == self.other2) then
			found = true
		else
			found = false
		end

		if (not (self.sound == nil)) then
			play_sample(self.sound, 1, 0, 1)
		else
			play_sample("sfx/chest_open.ogg", 1, 0, 1)
			if (found) then
				local t = create_idle_tween(0.5)
				append_tween(t, create_play_sample_tween("sfx/item_found.ogg"))
				new_tween(t)
			end
		end
		self.is_open = true
		set_milestone_complete(self.milestone, true)
		set_entity_animation(self.id, "open")

		if (found) then
			if (milestone_is_complete(self.other_ms_1) and milestone_is_complete(self.other_ms_2) and milestone_is_complete(self.milestone)) then
				play_sample("sfx/chest_puzzle.ogg", 1, 0, 1)
				set_milestone_complete("chest_puzzle", true)
				set_milestone_complete(self.other_ms_1, false)
				set_milestone_complete(self.other_ms_2, false)
				set_milestone_complete(self.milestone, false)
			else
				add_cash(1)
				speak(
					true,
					false,
					true,
					string.format(t("FOUND_ONE"), t("CASH")),
					"",
					0
				)
			end
		else
			play_sample("sfx/error.ogg", 1, 0, 1)
			simple_speak{
				true,
				"CHEST_PUZZLE_ERROR", "", get_player_id(0)
			}
			return true
		end
	end
	return false
end

function Chest_Puzzle:new(c)
	c = c or {}
	setmetatable(c, self)
	self.__index = self

	c.id = add_entity(c.entity_name or "chest", c.layer, c.x, c.y)

	if (milestone_is_complete(c.milestone) or milestone_is_complete("chest_puzzle")) then
		c.is_open = true
	end

	if (c.is_open) then
		set_entity_animation(c.id, "open")
	else
		set_entity_animation(c.id, "closed")
	end

	return c
end

function Chest_Puzzle:close(c)
	set_milestone_complete(self.other_ms_1, false)
	set_milestone_complete(self.other_ms_2, false)
	set_milestone_complete(self.milestone, false)
	set_entity_animation(self.id, "closed")
	self.is_open = false
end

-- x, y, width, height
function Active_Block:new(params)
	params = params or {}
	setmetatable(params, self)
	self.__index = self
	return params;
end

function Active_Block:entity_is_colliding(id)
	if ((not (self.layer == nil)) and (not (self.layer == get_entity_layer(id)))) then
		return false
	end
	local ex, ey;
	if (area_is_isometric()) then
		ex, ey = get_entity_iso_position(id)
		if (ex == nil) then
			return false
		end
	else
		ex, ey = get_entity_position(id)
		if (ex == nil) then
			return false
		end
		ex = ex / TILE_SIZE
		ey = ey / TILE_SIZE
	end
	if (ex >= self.x and ex < self.x+self.width and ey >= self.y and ey < self.y+self.height) then
		return true
	else
		return false
	end
end

function add_hot_edge(layer, direction, split1, split2)
	local w, h = get_area_pixel_size()
	local e

	if (not (direction == DIR_N or direction == DIR_S or direction == DIR_W or direction == DIR_E)) then
		print("*** Invalid edge specified")
		return nil
	end

	if (split1 == nil) then
		if (direction == DIR_N) then
			e = add_entity("box", layer, w/2, 10)
		elseif (direction == DIR_E) then
			e = add_entity("box", layer, w-6, h-1)
		elseif (direction == DIR_S) then
			e = add_entity("box", layer, w/2, h-1)
		elseif (direction == DIR_W) then
			e = add_entity("box", layer, 5, h-1)
		end
	else
		local pos
		if (direction == DIR_N or direction == DIR_S) then
			w = split2 - split1
			pos = split1 + w/2
		else
			h = split2 - split1
			pos = split2
		end
		if (direction == DIR_N) then
			e = add_entity("box", layer, pos, 10)
		elseif (direction == DIR_S) then
			e = add_entity("box", layer, pos, h-1)
		elseif (direction == DIR_E) then
			e = add_entity("box", layer, w-6, pos)
		elseif (direction == DIR_W) then
			e = add_entity("box", layer, 5, pos)
		end
	end
		
	if (direction == DIR_N or direction == DIR_S) then
		set_entity_bb_size(e, -w/2, -10, w, 10)
	else
		set_entity_bb_size(e, -5, -h, 10, h)
	end
	
	return e
end

-- convenience for grouping walls
function add_wall_group(layer, tl_x, tl_y, tile_w, tile_h, flags)
	local tiles = {}

	local i, j
	for j=tl_y,tl_y+tile_h-1 do
		for i=tl_x,tl_x+tile_w-1 do
			tiles[#tiles+1] = i
			tiles[#tiles+1] = j
		end
	end

	add_tile_group(
		layer,
		tl_x*TILE_SIZE,
		tl_y*TILE_SIZE+tile_h*TILE_SIZE-TILE_SIZE,
		tile_w*TILE_SIZE,
		10,
		flags,
		table.unpack(tiles)
	)
end

-- convenience for grouping horizontal bridge posts (bottom row, 1x2 just the post)
function add_bridge_post_group(layer, tl_x, tl_y)
	local tiles = {}

	local i, j
	for j=tl_y,tl_y+1 do
		for i=tl_x,tl_x do
			tiles[#tiles+1] = i
			tiles[#tiles+1] = j
		end
	end

	add_tile_group(
		layer,
		tl_x*TILE_SIZE,
		tl_y*TILE_SIZE+15,
		TILE_SIZE,
		5,
		0,
		table.unpack(tiles)
	)
end

function new_choppable(name, layer, x, y)
	c = {}
	c.id = add_entity(name, layer, x, y)
	c.hit = false
	c.dead = false
	return c
end

function update_choppable()
	for i=1,#choppable do
		if (choppable[i].hit and not choppable[i].dead) then
			if (entity_animation_is_finished(choppable[i].id)) then
				choppable[i].dead = true
				remove_entity(choppable[i].id)
			end
		end
	end
end

function chop_choppable(attacker, attackee)
	for i=1,#choppable do
		if (choppable[i].id == attackee and not choppable[i].hit) then
			choppable[i].hit = true
			set_entity_animation(choppable[i].id, "chopped")
			reset_entity_animation(choppable[i].id)
			choppable_array[i].dead = true
			-- sometimes spawn coins
			if (rand(10) == 0) then
				local x, y = get_entity_position(choppable[i].id)
				local layer = get_entity_layer(choppable[i].id)
				choppable[i].coin_id = add_entity("coin0", layer, x, y)
				choppable[i].coin_value = 1
			end
		end
	end
end

function collide_with_coins(id1, id2)
	for i=1,#choppable do
		if (colliding(id1, id2, 0, choppable[i].coin_id)) then
			play_sample("sfx/coin.ogg", 1, 0, 1)
			add_cash(choppable[i].coin_value)
			remove_entity(choppable[i].coin_id)
			choppable[i].coin_id = nil
		end
	end
end

function double_outline(outline)
	print("outline = {")
	print("\t" .. outline[1] .. ",")
	for i=2,#outline,2 do
		if (not (outline[i] == -1 and outline[i+1] == -1)) then
			outline[i] = outline[i] * 2
			outline[i+1] = outline[i+1] * 2
		end
		print("\t" .. outline[i] .. ", " .. outline[i+1] .. ",")
	end
	print("}")
end

function set_camera_to(entity)
	local x, y = get_entity_position(0)
	local ex, ey = get_entity_position(entity)
	local dx = ex - x
	local dy = ey - y
	local aw, ah = get_area_pixel_size()
	local sw, sh = get_screen_size()
	local add_x = 0
	local add_y = 0
	if (x > aw-sw/2) then
		add_x = add_x - ((aw-sw/2) - x)
	elseif (x < sw/2) then
		add_x = add_x - ((sw/2) - x)
	end
	if (y > ah-sh/2) then
		add_y = add_y - ((ah-sh/2) - y)
	elseif (y < sh/2) then
		add_y = add_y - ((sh/2) - y)
	end
	if (ex > aw-sw/2) then
		add_x = add_x + ((aw-sw/2) - ex)
	elseif (ex < sw/2) then
		add_x = add_x + ((sw/2) - ex)
	end
	if (ey > ah-sh/2) then
		add_y = add_y + ((ah-sh/2) - ey)
	elseif (ey < sh/2) then
		add_y = add_y + ((sh/2) - ey)
	end
	set_camera_offset(dx+add_x, dy+add_y)
end

function handle_level_save_item(num)
	local type = get_level_save_item_type(num)
	if (type == SAVE_PLAYER) then
		local layer, x, y = get_level_save_item_data(num)
		for i=1,get_num_players() do
			set_entity_layer(i-1, layer)
			set_entity_position(i-1, x, y)
		end
	elseif (type == SAVE_CHOPPABLE) then
		local name, layer, x, y = get_level_save_item_data(num)
		if (choppable_array == nil) then
			choppable_array = {}
		end
		choppable_array[#choppable_array+1] = { name=name, layer=layer, x=x, y=y, dead=false }
	elseif (type == SAVE_ENEMY) then
		local array = {get_level_save_item_data(num)}
		if (enemy_array == nil) then
			enemy_array = {}
		end
		local avatars = { array[6] }
		local num = array[7]
		local enemies = {}
		for i=1,num do
			enemies[i] = array[i+7]
		end
		enemy_array[#enemy_array+1] = {
			level = array[1],
			script = array[2],
			layer = array[3],
			x = array[4],
			y = array[5],
			avatars = avatars,
			num = num,
			enemies = enemies,
			dead = false
		}
	end
end

function spawn_enemies(array)
	local t = {}
	for i=1,#array do
		t[i] = add_enemy_avatar(
			array[i].level,
			array[i].script,
			array[i].layer,
			true, -- set_position
			array[i].x,
			array[i].y,
			array[i].enemies[1],
			array[i].num,
			table.unpack(array[i].enemies)
		)
	end
	return t
end

function get_random_enemies(enemies, always_enemy, min, max)
	local baddies = {}
	local r = rand(max-min+1) + min
	baddies[1] = always_enemy
	local count = 0
	for j=2,r do
		local e = enemies[rand(#enemies)+1]
		if (e == always_enemy and #enemies > 1) then
			if (count == 0) then
				while (e == always_enemy) do
					e = enemies[rand(#enemies)+1]
				end
			end
			count = count + 1
		end
		baddies[j] = enemies[rand(#enemies)+1]
	end
	return baddies
end

function add_enemies(layer, avatars, num, min, max, prefix, enemies)
	if (enemy_array == nil) then
		enemy_array = {}
	end
	for i=1,num do
		local x, y = get_random_enemy_spawn_point(avatars[1], layer)
		if (not (x == nil)) then
			local avi = avatars[rand(#avatars)+1]
			local baddies = get_random_enemies(enemies, avi, min, max)
			enemy_array[#enemy_array+1] = {
				level = prefix .. (rand(3) + 1),
				script = prefix,
				layer = layer,
				x = x,
				y = y,
				avatars = avatars,
				num = #baddies,
				enemies = baddies,
				dead = false
			}
			local tmp_array = {}
			tmp_array[1] = enemy_array[#enemy_array]
			local t = spawn_enemies(tmp_array)
			enemy_array[#enemy_array].id = t[1]
		end
	end
end

function spawn_choppable(array)
	if (choppable == nil) then
		choppable = {}
	end
	for i=1,#array do
		choppable[#choppable+1] = new_choppable(
			array[i].name,
			array[i].layer,
			array[i].x,
			array[i].y
		)
	end
end

function remove_enemy(id)
	for i=1,#enemy_array do
		if (enemy_array[i].id == id) then
			enemy_array[i].dead = true
		end
	end
end

function update_level_state()
	if (not (enemy_array == nil)) then
		for i=1,#enemy_array do
			enemy_array[i].x, enemy_array[i].y = get_entity_position(enemy_array[i].id)
		end
	end
end

function save_level_state()
	if (not is_dungeon) then
		return
	end

	if (not (choppable_array == nil)) then
		for i=1,#choppable_array do
			if (not choppable_array[i].dead) then
				save_item(
					SAVE_CHOPPABLE,
					choppable_array[i].name,
					choppable_array[i].layer,
					choppable_array[i].x,
					choppable_array[i].y
				)
			end
		end
	end
	if (not (enemy_array == nil)) then
		for i=1,#enemy_array do
			if (not enemy_array[i].dead) then
				save_item(
					SAVE_ENEMY,
					enemy_array[i].level,
					enemy_array[i].script,
					enemy_array[i].layer,
					enemy_array[i].x,
					enemy_array[i].y,
					enemy_array[i].enemies[1],
					enemy_array[i].num,
					table.unpack(enemy_array[i].enemies)
				)
			end
		end
	end
end

function load_level_state()
	for i=1,get_num_level_save_items() do
		handle_level_save_item(i-1)
	end
	if (not (enemy_array == nil)) then
		local t = spawn_enemies(enemy_array)
		for i=1,#enemy_array do
			enemy_array[i].id = t[i]
		end
	end
end

function water_cooler_ramp_up(tween, id, elapsed)
	ramp_music_up(0.5)
	_water_cooler_motif_is_playing = false
	return true
end

function play_water_cooler_motif()
	if (_water_cooler_motif_is_playing) then
		return
	end
	_water_cooler_motif_is_playing = true
	ramp_music_down(0.5)
	play_sample("sfx/water_cooler.ogg", 1, 0, 1)
	local t = create_idle_tween(6.5)
	append_tween(t, { run = water_cooler_ramp_up })
	new_tween(t)
end

function done_amaysa1()
	return milestone_is_complete("first_amaysa_encounter")
end

function done_pyou1()
	return milestone_is_complete("pyou_intro")
end

function done_oldoak()
	return milestone_is_complete("old_oak_battle")
end

local bat_swoop_count = 0
local next_bat_swoop_time
local doing_swoop_battle = false
local set_next_swoop_time = true
local should_draw_bat = false
local start_swoop_chance = 8
local swoop_chance = start_swoop_chance

function bat_swoop_tween(tween, id, elapsed)
	bat_swoop_y = bat_swoop_y + 1
	if (bat_swoop_y >= bat_swoop_start_y + bat_swoop_distance) then
		return true
	end
	return false
end

function start_bat_swoop_battle_tween(tween, id, elapsed)
	doing_swoop_battle = true
	bat_swoop_enemies = get_random_enemies({ "machete_ant", "scorpion", "grenade_ant", "bat" }, "bat", 4, 5)
	return true
end

function set_next_bat_swoop()
	bat_swoop_count = 0
	next_bat_swoop_time = 7*60
end

function check_bat_swoops()
	bat_swoop_count = bat_swoop_count + 1

	if (set_next_swoop_time) then
		should_draw_bat = false
		set_next_bat_swoop()
		set_next_swoop_time = false
	elseif (doing_swoop_battle) then
		start_battle("caverns" .. (rand(3)+1), "caverns", false, table.unpack(bat_swoop_enemies))
		doing_swoop_battle = false
		set_next_swoop_time = true
	elseif (bat_swoop_count > next_bat_swoop_time and not get_entity_input_disabled(get_player_id(0))) then
		if (rand(swoop_chance) == 0) then
			swoop_chance = start_swoop_chance
			next_bat_swoop_time = 1000000 -- stop from entering here again for now

			play_sample("sfx/enemy_alerted.ogg", 1, 0, 1)

			local px, py = get_entity_position(get_player_id(0))

			set_battle_was_event(BATTLE_EVENT_SIGHTED)

			stop_entity(get_player_id(0))
			set_entity_input_disabled(get_player_id(0), true)

			bat_swoop_x = px
			bat_swoop_y = py - 75
			bat_swoop_start_y = bat_swoop_y
			bat_swoop_distance = 50

			should_draw_bat = true

			local t = { run = bat_swoop_tween }
			t.next_tween = { run = start_bat_swoop_battle_tween }
			new_tween(t)
		else
			bat_swoop_count = 0
			swoop_chance = swoop_chance - 1
			if (swoop_chance < 0) then
				swoop_chance = 0
			end
		end
	end
end

function start_bat_swoops()
	bat_swoop_img = load_bitmap("misc_graphics/bat.png")
end

function stop_bat_swoops()
	destroy_bitmap("misc_graphics/bat.png")
end

function draw_bat_swoops()
	if (should_draw_bat) then
		local top_x, top_y = get_area_top()
		local w, h = get_bitmap_size(bat_swoop_img)
		local p = (bat_swoop_y - bat_swoop_start_y) / bat_swoop_distance
		draw_tinted_rotated_bitmap(bat_swoop_img, p, p, p, 1, w/2, h, bat_swoop_x-top_x, bat_swoop_y-top_y, 0, 0)
	end
end

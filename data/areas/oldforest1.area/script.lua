is_dungeon = true

local waterfall_count = 0
local waterfall_frame = 1
local WATERFALL_TICKS = 10

local waterfall1_x = 66*TILE_SIZE+TILE_SIZE/2
local waterfall1_y = 3*TILE_SIZE+TILE_SIZE/2
local waterfall2_x = 52*TILE_SIZE+TILE_SIZE/2
local waterfall2_y = 48*TILE_SIZE+TILE_SIZE/2
local waterfall3_x = 39*TILE_SIZE+TILE_SIZE/2
local waterfall3_y = 97*TILE_SIZE+TILE_SIZE/2

local top_x, top_y

function get_vol()
	local px, py = get_entity_position(0)
	local dx1 = px - waterfall1_x
	local dy1 = py - waterfall1_y
	local dist1 = math.sqrt(dx1*dx1 + dy1*dy1)
	local dx2 = px - waterfall2_x
	local dy2 = py - waterfall2_y
	local dist2 = math.sqrt(dx2*dx2 + dy2*dy2)
	local dx3 = px - waterfall3_x
	local dy3 = py - waterfall3_y
	local dist3 = math.sqrt(dx3*dx3 + dy3*dy3)
	local dist
	if (dist1 < dist2) then
		if (dist1 < dist3) then
			dist = dist1
		else
			dist = dist3
		end
	else
		if (dist2 < dist3) then
			dist = dist2
		else
			dist = dist3
		end
	end
	if (dist >= 400) then
		return 0
	else
		return (400 - dist) / 400
	end
end

function adjust_waterfall_sound()
	adjust_sample("sfx/waterfall.ogg", get_vol(), 0.0, 1.0)
end

function start(game_just_loaded)
	play_music("music/old_forest.mid");

	process_outline()

	waterfall_bmps = {}
	for i=1,4 do
		waterfall_bmps[i] = load_bitmap("misc_graphics/of_waterfall" .. i .. ".png")
	end

	-- som bushes
	add_wall_group(2, 15, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 4, 20, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 4, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 18, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 33, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 11, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 15, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 21, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 16, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 64, 17, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 15, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 9, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 49, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 53, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 47, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 48, 52, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 32, 53, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 56, 54, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 56, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 58, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 96, 32, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 35, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 86, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 66, 76, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 69, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 73, 89, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 79, 92, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 93, 62, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 95, 68, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 97, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 100, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 58, 100, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 91, 95, 2, 2, TILE_GROUP_BUSHES)
	-- mario bushes
	add_wall_group(2, 24, 3, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 17, 9, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 41, 8, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 42, 12, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 13, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 59, 16, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 46, 17, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 20, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 1, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 3, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 4, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 44, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 61, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 31, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 33, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 88, 54, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 12, 88, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 65, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 69, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 76, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 76, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 80, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 53, 80, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 84, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 40, 87, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 48, 88, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 89, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 46, 91, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 85, 66, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 67, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 25, 97, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 97, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 49, 98, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 63, 94, 4, 2, TILE_GROUP_BUSHES)
	-- bridges
	add_wall_group(2, 65, 6, 1, 1, 0)
	add_wall_group(2, 72, 6, 1, 1, 0)
	add_wall_group(2, 65, 7, 8, 2, 0)
	add_wall_group(2, 48, 43, 1, 1, 0)
	add_wall_group(2, 55, 43, 1, 1, 0)
	add_wall_group(2, 48, 44, 8, 2, 0)
	add_wall_group(2, 25, 55, 1, 1, 0)
	add_wall_group(2, 32, 55, 1, 1, 0)
	add_wall_group(2, 25, 56, 8, 2, 0)
	add_wall_group(2, 35, 53, 1, 1, 0)
	add_wall_group(2, 45, 53, 1, 1, 0)
	add_wall_group(2, 35, 54, 11, 2, 0)
	add_wall_group(2, 48, 54, 1, 1, 0)
	add_wall_group(2, 50, 54, 1, 1, 0)
	add_wall_group(2, 35, 100, 1, 1, 0)
	add_wall_group(2, 42, 100, 1, 1, 0)
	add_wall_group(2, 35, 101, 8, 2, 0)
	
	if (not game_just_loaded) then
		add_enemies(2, { "tough_ant", "ghost" }, 24, 3, 4, "of", { "bazooka_ant", "tough_ant", "machete_ant", "wolf", "faff", "ghost" })
		choppable_array = {}
		choppable_array[1] = { name="of_bush", layer=2, x=15*TILE_SIZE+8, y=21*TILE_SIZE+8, dead=false }
		choppable_array[2] = { name="of_bush", layer=2, x=16*TILE_SIZE+8, y=21*TILE_SIZE+8, dead=false }
		choppable_array[3] = { name="of_bush", layer=2, x=14*TILE_SIZE+8, y=22*TILE_SIZE+8, dead=false }
		choppable_array[4] = { name="of_bush", layer=2, x=15*TILE_SIZE+8, y=22*TILE_SIZE+8, dead=false }
		choppable_array[5] = { name="of_bush", layer=2, x=13*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[6] = { name="of_bush", layer=2, x=14*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[7] = { name="of_bush", layer=2, x=22*TILE_SIZE+8, y=30*TILE_SIZE+8, dead=false }
		choppable_array[8] = { name="of_bush", layer=2, x=23*TILE_SIZE+8, y=30*TILE_SIZE+8, dead=false }
		choppable_array[9] = { name="of_bush", layer=2, x=62*TILE_SIZE+8, y=6*TILE_SIZE+8, dead=false }
		choppable_array[10] = { name="of_bush", layer=2, x=49*TILE_SIZE+8, y=10*TILE_SIZE+8, dead=false }
		choppable_array[11] = { name="of_bush", layer=2, x=42*TILE_SIZE+8, y=14*TILE_SIZE+8, dead=false }
		choppable_array[12] = { name="of_bush", layer=2, x=56*TILE_SIZE+8, y=14*TILE_SIZE+8, dead=false }
		choppable_array[13] = { name="of_bush", layer=2, x=42*TILE_SIZE+8, y=23*TILE_SIZE+8, dead=false }
		choppable_array[14] = { name="of_bush", layer=2, x=41*TILE_SIZE+8, y=24*TILE_SIZE+8, dead=false }
		choppable_array[15] = { name="of_bush", layer=2, x=37*TILE_SIZE+8, y=29*TILE_SIZE+8, dead=false }
		choppable_array[16] = { name="of_bush", layer=2, x=64*TILE_SIZE+8, y=11*TILE_SIZE+8, dead=false }
		choppable_array[17] = { name="of_bush", layer=2, x=65*TILE_SIZE+8, y=11*TILE_SIZE+8, dead=false }
		choppable_array[18] = { name="of_bush", layer=2, x=80*TILE_SIZE+8, y=30*TILE_SIZE+8, dead=false }
		choppable_array[19] = { name="of_bush", layer=2, x=5*TILE_SIZE+8, y=36*TILE_SIZE+8, dead=false }
		choppable_array[20] = { name="of_bush", layer=2, x=22*TILE_SIZE+8, y=48*TILE_SIZE+8, dead=false }
		choppable_array[21] = { name="of_bush", layer=2, x=87*TILE_SIZE+8, y=37*TILE_SIZE+8, dead=false }
		choppable_array[22] = { name="of_bush", layer=2, x=87*TILE_SIZE+8, y=39*TILE_SIZE+8, dead=false }
		choppable_array[23] = { name="of_bush", layer=2, x=94*TILE_SIZE+8, y=43*TILE_SIZE+8, dead=false }
		choppable_array[24] = { name="of_bush", layer=2, x=9*TILE_SIZE+8, y=69*TILE_SIZE+8, dead=false }
		choppable_array[25] = { name="of_bush", layer=2, x=12*TILE_SIZE+8, y=70*TILE_SIZE+8, dead=false }
		choppable_array[26] = { name="of_bush", layer=2, x=0*TILE_SIZE+8, y=84*TILE_SIZE+8, dead=false }
		choppable_array[27] = { name="of_bush", layer=2, x=11*TILE_SIZE+8, y=85*TILE_SIZE+8, dead=false }
		choppable_array[28] = { name="of_bush", layer=2, x=1*TILE_SIZE+8, y=87*TILE_SIZE+8, dead=false }
		choppable_array[29] = { name="of_bush", layer=2, x=77*TILE_SIZE+8, y=79*TILE_SIZE+8, dead=false }
		choppable_array[30] = { name="of_bush", layer=2, x=89*TILE_SIZE+8, y=82*TILE_SIZE+8, dead=false }
		choppable_array[31] = { name="of_bush", layer=2, x=93*TILE_SIZE+8, y=88*TILE_SIZE+8, dead=false }
		choppable_array[32] = { name="of_bush", layer=2, x=94*TILE_SIZE+8, y=90*TILE_SIZE+8, dead=false }
		choppable_array[33] = { name="of_bush", layer=2, x=9*TILE_SIZE+8, y=96*TILE_SIZE+8, dead=false }
		choppable_array[34] = { name="of_bush", layer=2, x=1*TILE_SIZE+8, y=97*TILE_SIZE+8, dead=false }
		choppable_array[35] = { name="of_bush", layer=2, x=17*TILE_SIZE+8, y=97*TILE_SIZE+8, dead=false }
		choppable_array[36] = { name="of_bush", layer=2, x=4*TILE_SIZE+8, y=98*TILE_SIZE+8, dead=false }
		choppable_array[37] = { name="of_bush", layer=2, x=5*TILE_SIZE+8, y=99*TILE_SIZE+8, dead=false }
		choppable_array[38] = { name="of_bush", layer=2, x=7*TILE_SIZE+8, y=101*TILE_SIZE+8, dead=false }
		choppable_array[39] = { name="of_bush", layer=2, x=35*TILE_SIZE+8, y=104*TILE_SIZE+8, dead=false }
		choppable_array[40] = { name="of_bush", layer=2, x=83*TILE_SIZE+8, y=101*TILE_SIZE+8, dead=false }
		choppable_array[41] = { name="of_bush", layer=2, x=93*TILE_SIZE+8, y=104*TILE_SIZE+8, dead=false }
		choppable_array[42] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=98*TILE_SIZE+8, dead=false }
		choppable_array[43] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=98*TILE_SIZE+8, dead=false }
		choppable_array[44] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=99*TILE_SIZE+8, dead=false }
		choppable_array[45] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=99*TILE_SIZE+8, dead=false }
		choppable_array[46] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=100*TILE_SIZE+8, dead=false }
		choppable_array[47] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=100*TILE_SIZE+8, dead=false }
		choppable_array[48] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=101*TILE_SIZE+8, dead=false }
		choppable_array[49] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=101*TILE_SIZE+8, dead=false }
		choppable_array[50] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=102*TILE_SIZE+8, dead=false }
		choppable_array[51] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=102*TILE_SIZE+8, dead=false }
		choppable_array[52] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=103*TILE_SIZE+8, dead=false }
		choppable_array[53] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=103*TILE_SIZE+8, dead=false }
		choppable_array[54] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=104*TILE_SIZE+8, dead=false }
		choppable_array[55] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=104*TILE_SIZE+8, dead=false }
		choppable_array[56] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=105*TILE_SIZE+8, dead=false }
		choppable_array[57] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=105*TILE_SIZE+8, dead=false }
		choppable_array[58] = { name="of_bush", layer=2, x=99*TILE_SIZE+8, y=106*TILE_SIZE+8, dead=false }
		choppable_array[59] = { name="of_bush", layer=2, x=100*TILE_SIZE+8, y=106*TILE_SIZE+8, dead=false }
	end
	spawn_choppable(choppable_array)

	to_map = Active_Block:new{x=0, y=9, width=1, height=5}
	to_of2 = Active_Block:new{x=100, y=98, width=1, height=10}
	scene_block = Active_Block:new{x=92, y=101, width=1, height=7}
	
	load_sample("sfx/waterfall.ogg", true)
	play_sample("sfx/waterfall.ogg", get_vol(), 0.0, 1.0)
	
	chest1 = Chest:new{x=50*TILE_SIZE, y=51*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=WEAPON, contains="RAKE", quantity=1, milestone="of_chest1"}
	chest2 = Chest:new{x=948, y=1273, layer=2, contains="HEALTHVIAL", quantity=3, milestone="of_chest2"}
	chest3 = Chest:new{x=20*TILE_SIZE+TILE_SIZE/2, y=74*TILE_SIZE, layer=2, contains_gold=true, contains="CASH", quantity=75, milestone="of_chest3"}
	chest4 = Chest:new{x=6*TILE_SIZE+TILE_SIZE, y=37*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ACCESSORY, contains="GLOVES", quantity=1, milestone="of_chest4"}
	chest5 = Chest:new{x=33*TILE_SIZE, y=97*TILE_SIZE, layer=2, contains="DIRTYSOCK", quantity=1, milestone="of_chest5"}
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest3.id) then
		chest3:open()
	elseif (activated == chest4.id) then
		chest4:open()
	elseif (activated == chest5.id) then
		chest5:open()
	end
end

local waterfall_sound_count = 0

function logic()
	update_choppable()
	
	waterfall_sound_count = waterfall_sound_count + 1
	if (waterfall_sound_count == 15) then
		waterfall_sound_count = 0
		adjust_waterfall_sound()
	end

	if (to_map:entity_is_colliding(0)) then
		start_map("OLD_FOREST")
	elseif (to_of2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("oldforest2", DIR_E, 56, 728)
	elseif (scene_block:entity_is_colliding(0) and not milestone_is_complete("learned_chop")) then
		set_milestone_complete("learned_chop", true)

		local egbert
		local num = get_num_players()
		for i=1,num do
			local id = get_player_id(i-1)
			if (get_entity_name(id) == "egbert") then
				egbert = id
				break
			end
		end
		simple_speak{
			true,
			"OF_EGBERT1", "", egbert,
			"OF_EGBERT2", "snooty", egbert,
			"OF_CHOP", "", -1
		}
	end

	waterfall_count = waterfall_count + 1
	if (waterfall_count >= WATERFALL_TICKS) then
		waterfall_frame = waterfall_frame + 1
		if (waterfall_frame > 4) then
			waterfall_frame = 1
		end
		waterfall_count = 0
	end
end

function collide(id1, id2)
	collide_with_coins(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
	for i=1,4 do
		destroy_bitmap("misc_graphics/of_waterfall" .. i .. ".png")
	end
	destroy_sample("sfx/waterfall.ogg")
end

function draw_waterfall(x, y)
	draw_bitmap(waterfall_bmps[waterfall_frame], x, y, 0)
end

function mid_draw_layer(layer)
	if (layer == 1) then
		local x, y = get_area_top()
		if (not (x == nil)) then
			top_x = x
			top_y = y
		end

		draw_waterfall(65*TILE_SIZE-top_x, 1*TILE_SIZE-top_y)
		draw_waterfall(51*TILE_SIZE-top_x, 46*TILE_SIZE-top_y)
		draw_waterfall(38*TILE_SIZE-top_x, 95*TILE_SIZE-top_y)
	end
end

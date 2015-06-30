is_dungeon = true
local jumping = false

function start(game_just_loaded)
	play_music("music/stonecrater.mid");

	process_outline()

	-- outline0 groups
	add_wall_group(2, 10, 56, 1, 2, 0)

	-- outline1 groups
	-- shabby bushes
	add_wall_group(3, 86, 57, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 87, 59, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 53, 36, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 80, 14, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 83, 19, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 84, 21, 1, 1, TILE_GROUP_BUSHES)
	add_wall_group(3, 81, 17, 3, 2, TILE_GROUP_BUSHES)
	-- cactuses
	add_wall_group(3, 75, 59, 1, 2, 0)
	add_wall_group(3, 83, 45, 1, 2, 0)
	add_wall_group(3, 90, 42, 1, 2, 0)
	add_wall_group(3, 91, 40, 1, 2, 0)
	add_wall_group(3, 89, 37, 1, 2, 0)
	-- bridges
	add_wall_group(3, 35, 42, 1, 1, 0)
	add_wall_group(3, 37, 42, 1, 1, 0)
	add_wall_group(3, 40, 54, 11, 2, 0)
	add_wall_group(3, 33, 9, 11, 2, 0)
	
	if (not game_just_loaded) then
		add_enemies(3, { "armadillo", "machete_ant" }, 10, 3, 4, "stonecrater", { "scorpion", "armadillo", "machete_ant", "bazooka_ant" })
	end

	to_stonecrater1 = Active_Block:new{x=0, y=53, width=1, height=4}
	to_stonecrater3 = Active_Block:new{x=50, y=0, width=5, height=1}
	
	jump_block1 = Active_Block:new{x=13, y=55, width=2, height=2}
	local jump_entity1 = add_entity("jump_bubble", 2, 14*TILE_SIZE, 56*TILE_SIZE)
	set_show_entity_shadow(jump_entity1, true);
	jump_block2 = Active_Block:new{x=18, y=55, width=2, height=2}
	local jump_entity2 = add_entity("jump_bubble", 3, 19*TILE_SIZE, 56*TILE_SIZE)
	set_show_entity_shadow(jump_entity2, true);
	
	chest1 = Chest:new{x=36*TILE_SIZE+TILE_SIZE/2, y=40*TILE_SIZE, layer=3, contains_equipment=true, equipment_type=ACCESSORY, contains="BOOTS", quantity=1, milestone="stonecrater2_chest1"}
	chest2 = Chest:new{x=16*TILE_SIZE, y=5*TILE_SIZE, layer=3, contains_equipment=true, equipment_type=WEAPON, contains="SLEDGEHAMMER", quantity=1, milestone="stonecrater2_chest2"}
	chest3 = Chest:new{x=53*TILE_SIZE, y=24*TILE_SIZE, layer=3, contains_equipment=true, equipment_type=WEAPON, contains="IRONARROW", quantity=50, milestone="stonecrater2_chest3"}
end

function activate(activator, activated)
	if (activated == chest1.id) then
		chest1:open()
	elseif (activated == chest2.id) then
		chest2:open()
	elseif (activated == chest3.id) then
		chest3:open()
	end
end

function logic()
	if (to_stonecrater1:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater1", DIR_W, 486, 102)
	elseif (to_stonecrater3:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("stonecrater3", DIR_N, 27.5*TILE_SIZE, 116.5*TILE_SIZE)
	end
	
	-- NOTE: always needed for jump levels
	if (jumping) then
		local all_done = true
		for i=1,#jump_table do
			if (jump_table[i].done == false) then
				all_done = false
				local x, y = get_entity_position(jump_table[i].tween.entity)
				if (math.abs(x-jump_table[i].x) <= 5 and math.abs(y-jump_table[i].y) <= 5) then
					local t = create_character_role_change_tween(jump_table[i].tween.entity, "")
					append_tween(t, jump_table[i].tween)
					local t2 = create_character_role_change_tween(jump_table[i].tween.entity, "follow")
					t2.role_parameters[1] = jump_table[i].follow
					append_tween(t, t2)
					new_tween(t)
					jump_table[i].done = true
				end
			end
		end
		if (all_done) then
			jumping = false
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

-- NOTE: always needed for jump levels
function jump()
	local tween
	if (jump_block1:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 19*TILE_SIZE,
			end_y = 56*TILE_SIZE,
			end_layer = 3,
			right = true
		}
		new_tween(tween)
		jumping = true
	elseif (jump_block2:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 14*TILE_SIZE,
			end_y = 56*TILE_SIZE,
			end_layer = 2,
			right = false
		}
		new_tween(tween)
		jumping = true
	end
	if (jumping) then
		jump_table = {}
		for i=1,get_num_players()-1 do
			jump_table[i] = {}
			jump_table[i].entity = get_player_id(i)
			jump_table[i].follow = get_player_id(i-1)
			jump_table[i].x, jump_table[i].y = get_entity_position(0)
			jump_table[i].done = false
			jump_table[i].tween = create_jump_tween{
				entity = get_player_id(i),
				time = tween.time,
				height = tween.height,
				end_x = tween.end_x,
				end_y = tween.end_y,
				end_layer = tween.end_layer,
				right = tween.right
			}
		end
	end
end


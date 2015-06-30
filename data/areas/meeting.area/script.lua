local jumping = false

function start()
	play_music("music/abw2.mid");

	process_outline()
	
	last_player_x, last_player_y = get_entity_position(0)

	-- layer 2 groups
	-- trees
	add_wall_group(2, 13, 10, 4, 6, 0)
	add_wall_group(2, 14, 23, 4, 6, 0)
	add_wall_group(2, 6, 26, 4, 6, 0)
	add_wall_group(2, 43, 16, 4, 6, 0)
	add_wall_group(2, 38, 12, 5, 7, 0)
	-- mario bushes
	add_wall_group(2, 3, 12, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 10, 13, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 7, 17, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 10, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 12, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 8, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 34, 14, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 16, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 40, 20, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 41, 22, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 48, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 50, 25, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 39, 42, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 42, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 41, 45, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 47, 41, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 52, 41, 3, 2, TILE_GROUP_BUSHES)
	-- SOM bushes
	add_wall_group(2, 1, 16, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 6, 13, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 8, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 23, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 7, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 14, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 21, 26, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 37, 4, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 6, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 34, 10, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 18, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 20, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 46, 24, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 39, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 37, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 38, 45, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 41, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 51, 39, 2, 2, TILE_GROUP_BUSHES)
	-- signs
	add_wall_group(2, 32, 9, 2, 2, 0)
	add_wall_group(2, 37, 26, 2, 2, 0)
	add_wall_group(2, 15, 19, 2, 2, 0)

	-- layer 3 groups
	-- trees
	add_wall_group(3, 14, 37, 4, 6, 0)
	add_wall_group(3, 19, 34, 4, 6, 0)
	add_wall_group(3, 30, 36, 4, 6, 0)
	-- mario bushes
	add_wall_group(3, 2, 44, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 7, 43, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 24, 34, 3, 2, TILE_GROUP_BUSHES)
	-- SOM bushes
	add_wall_group(3, 0, 44, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(3, 10, 40, 2, 2, TILE_GROUP_BUSHES)

	choppable = {}
	--choppable[1] = new_choppable("bush", 3, 60*TILE_SIZE+8, 20*TILE_SIZE+8) etc

	to_rivertown = Active_Block:new{x=21, y=0, width=16, height=2}
	to_ants = Active_Block:new{x=0, y=12, width=1, height=22}
	to_ants2 = Active_Block:new{x=0, y=43, width=1, height=9}
	to_pyou = Active_Block:new{x=55, y=22, width=1, height=26}

	sign1 = add_polygon_entity(
		2,
		32*TILE_SIZE, 9*TILE_SIZE,
		32*TILE_SIZE+TILE_SIZE*2, 9*TILE_SIZE,
		32*TILE_SIZE+TILE_SIZE*2, 9*TILE_SIZE+TILE_SIZE*2,
		32*TILE_SIZE, 9*TILE_SIZE+TILE_SIZE*2
	)

	sign2 = add_polygon_entity(
		2,
		15*TILE_SIZE, 19*TILE_SIZE,
		15*TILE_SIZE+TILE_SIZE*2, 19*TILE_SIZE,
		15*TILE_SIZE+TILE_SIZE*2, 19*TILE_SIZE+TILE_SIZE*2,
		15*TILE_SIZE, 19*TILE_SIZE+TILE_SIZE*2
	)

	sign3 = add_polygon_entity(
		2,
		37*TILE_SIZE, 26*TILE_SIZE,
		37*TILE_SIZE+TILE_SIZE*2, 26*TILE_SIZE,
		37*TILE_SIZE+TILE_SIZE*2, 26*TILE_SIZE+TILE_SIZE*2,
		37*TILE_SIZE, 26*TILE_SIZE+TILE_SIZE*2
	)

	jump_block = Active_Block:new{x=26, y=29, width=2, height=2}
	local jump_entity = add_entity("jump_bubble", 3, 27*TILE_SIZE, 30*TILE_SIZE)
	set_show_entity_shadow(jump_entity, true);

	jump_switch_info_block = Active_Block:new{x=20, y=30, width=7, height=7}
end

function activate(activator, activated)
	if (activated == sign1) then
		speak(false, false, true, t("MEETING_SIGN1"), "", sign1, sign1)
	elseif (activated == sign2) then
		speak(false, false, true, t("MEETING_SIGN2"), "", sign2, sign2)
	elseif (activated == sign3) then
		speak(false, false, true, t("MEETING_SIGN3"), "", sign3, sign3)
	end
end

local MILESTONE_NAME = "first_amaysa_encounter"
local JUMP_SWITCH_INFO_MILESTONE = "jump_switch_info"

function logic()
	if (to_rivertown:entity_is_colliding(0)) then
		start_map("INTERSECTION")
	elseif (to_ants:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("ants", DIR_W, 3286, 362)
	elseif (to_ants2:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("ants", DIR_W, 3311, 1751)
	elseif (to_pyou:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("pyou", DIR_E, 50, 480)
	elseif (not milestone_is_complete(JUMP_SWITCH_INFO_MILESTONE) and jump_switch_info_block:entity_is_colliding(0)) then
		set_milestone_complete(JUMP_SWITCH_INFO_MILESTONE, true)
		simple_speak{
			true,
			"FROGBERT_JUMP_INFO", "", 1,
			"JUMP_INFO", "", -1
		}
	end
	update_choppable()

	if (not milestone_is_complete(MILESTONE_NAME)) then
		local x, y = get_entity_position(0)
		if (x >= 51*TILE_SIZE) then
			set_entity_position(0, last_player_x, last_player_y)
			speak(false, false, true, t("MEETING_EAST"), "", 0, 0)
		else
			last_player_x, last_player_y = get_entity_position(0)
		end
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

-- NOTE: always needed for jump levels
function jump()
	local tween
	if (jump_block:entity_is_colliding(0)) then
		tween = create_jump_tween{
			entity = get_player_id(0),
			time = 1,
			height = 64,
			end_x = 29*TILE_SIZE+TILE_SIZE/2,
			end_y = 27*TILE_SIZE+TILE_SIZE/2,
			end_layer = 2,
			right = true
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

function collide(id1, id2)
end

function uncollide(id1, id2)
end

function action_button_pressed(n)
end

function attacked(attacker, attackee)
	chop_choppable(attacker, attackee)
end

function stop()
end

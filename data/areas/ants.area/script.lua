is_dungeon = true

local MILESTONE_NAME = "first_amaysa_encounter"
local CRYSTAL1_MILESTONE_NAME = "ants_crystal1"
local CRYSTAL2_MILESTONE_NAME = "ants_crystal2"
local JUMP_SWITCH_INFO_MILESTONE = "jump_switch_info"
local EGBERT_SCENE_X = 2631
local EGBERT_SCENE_Y = 2840

function create_crystal_block()
	crystal_block = Active_Block:new{
		x=math.floor((EGBERT_SCENE_X-285/2)/16),
		y=math.floor((EGBERT_SCENE_Y-160/2)/16),
		width=math.floor(285/16),
		height=math.floor(160/16)
	}
end

-- often here 0 is used for egbert and 1 for frogbert
-- at this point those are the only possibilities

function start(game_just_loaded)
	play_music("music/abw2.mid");
	
	last_player_x, last_player_y = get_entity_position(0)
	create_crystal_block()
	
	load_sample("sfx/boing.ogg", false)
	load_sample("sfx/thud.ogg", false)
	load_sample("sfx/chomp_crystal.ogg", false)
	load_sample("sfx/amaysa_fly_l_r.ogg", false)
	load_sample("sfx/amaysa_fly_l_r_short.ogg", false)
	load_sample("sfx/amaysa_fly_r_l.ogg", false)
	load_sample("sfx/amaysa.ogg", false)

	process_outline()

	-- group stuff
	-- small trees
	add_wall_group(2, 12, 28, 4, 6, 0)
	add_wall_group(2, 21, 94, 4, 6, 0)
	add_wall_group(2, 48, 184, 4, 6, 0)
	add_wall_group(2, 61, 12, 4, 6, 0)
	add_wall_group(2, 69, 193, 4, 6, 0)
	add_wall_group(2, 85, 176, 4, 6, 0)
	add_wall_group(2, 101, 172, 4, 6, 0)
	add_wall_group(2, 104, 51, 4, 6, 0)
	add_wall_group(2, 107, 76, 4, 6, 0)
	add_wall_group(2, 140, 194, 4, 6, 0)
	add_wall_group(2, 156, 71, 4, 6, 0)
	add_wall_group(2, 159, 113, 4, 6, 0)
	add_wall_group(2, 169, 115, 4, 6, 0)
	add_wall_group(2, 148, 124, 4, 6, 0)
	add_wall_group(2, 196, 156, 4, 6, 0)
	add_wall_group(2, 178, 103, 4, 6, 0)
	add_wall_group(2, 186, 110, 4, 6, 0)
	add_wall_group(2, 198, 102, 4, 6, 0)
	add_wall_group(2, 197, 112, 4, 6, 0)
	add_wall_group(2, 203, 110, 4, 6, 0)
	add_wall_group(2, 175, 110, 4, 6, 0)
	add_wall_group(2, 178, 89, 4, 6, 0)
	add_wall_group(2, 184, 99, 4, 6, 0)
	-- apple blossom trees
	add_wall_group(2, 45, 177, 5, 7, 0)
	add_wall_group(2, 54, 180, 5, 7, 0)
	add_wall_group(2, 47, 169, 5, 7, 0)
	add_wall_group(2, 58, 165, 5, 7, 0)
	add_wall_group(2, 75, 165, 5, 7, 0)
	add_wall_group(2, 65, 172, 5, 7, 0)
	add_wall_group(2, 61, 182, 5, 7, 0)
	add_wall_group(2, 72, 182, 5, 7, 0)
	add_wall_group(2, 95, 155, 5, 7, 0)
	add_wall_group(2, 101, 145, 5, 7, 0)
	add_wall_group(2, 102, 128, 5, 7, 0)
	add_wall_group(2, 105, 89, 5, 7, 0)
	add_wall_group(2, 109, 61, 5, 7, 0)
	add_wall_group(2, 121, 149, 5, 7, 0)
	add_wall_group(2, 112, 173, 5, 7, 0)
	add_wall_group(2, 167, 103, 5, 7, 0)
	add_wall_group(2, 164, 121, 5, 7, 0)
	add_wall_group(2, 88, 138, 5, 7, 0)
	-- mario bushes
	add_wall_group(2, 21, 29, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 29, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 14, 39, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 19, 53, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 100, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 30, 102, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 30, 90, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 33, 77, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 72, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 60, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 48, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 36, 41, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 26, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 31, 32, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 35, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 107, 177, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 104, 161, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 79, 144, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 79, 153, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 91, 150, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 89, 132, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 94, 130, 3, 2, TILE_GROUP_BUSHE)
	add_wall_group(2, 95, 134, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 98, 130, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 95, 137, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 88, 124, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 126, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 94, 125, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 95, 122, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 98, 126, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 100, 123, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 90, 39, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 92, 41, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 99, 41, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 43, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 99, 45, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 23, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 95, 23, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 105, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 116, 80, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 120, 81, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 130, 74, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 108, 87, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 117, 177, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 128, 164, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 136, 161, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 146, 17, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 147, 20, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 152, 131, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 155, 129, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 160, 131, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 170, 130, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 151, 145, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 157, 144, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 156, 146, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 152, 147, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 147, 148, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 161, 149, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 164, 150, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 154, 164, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 157, 166, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 156, 169, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 159, 164, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 162, 164, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 168, 163, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 167, 165, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 169, 168, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 172, 169, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 168, 171, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 174, 194, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 189, 188, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 188, 191, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 193, 192, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 190, 193, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 187, 195, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 191, 154, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 181, 9, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 185, 10, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 200, 14, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 139, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 148, 192, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 182, 106, 3, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 189, 103, 4, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 194, 105, 3, 2, TILE_GROUP_BUSHES)
	-- SOM bushes
	add_wall_group(2, 18, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 20, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 27, 53, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 64, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 24, 66, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 22, 67, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 29, 93, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 177, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 174, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 31, 117, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 31, 88, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 32, 83, 2, 2, TILE_GROUP_BUSHES)	
	add_wall_group(2, 30, 54, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 47, 51, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 49, 50, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 37, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 43, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 45, 37, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 51, 32, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 54, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 57, 15, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 56, 21, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 57, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 59, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 34, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 29, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 61, 38, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 65, 42, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 67, 48, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 71, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 69, 133, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 70, 135, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 68, 136, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 69, 140, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 143, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 83, 138, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 80, 142, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 71, 147, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 152, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 82, 148, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 79, 155, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 64, 167, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 60, 178, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 74, 177, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 77, 175, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 88, 174, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 100, 180, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 185, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 104, 159, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 102, 164, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 86, 127, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 129, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 91, 130, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 94, 128, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 92, 122, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 100, 120, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 98, 124, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 88, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 93, 77, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 97, 78, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 82, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 75, 2, 2, TIE_GROUP_BUSHES)
	add_wall_group(2, 103, 76, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 104, 78, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 87, 67, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 112, 78, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 120, 122, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 117, 129, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 118, 131, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 108, 142, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 117, 141, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 116, 143, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 110, 148, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 113, 150, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 127, 195, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 125, 197, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 129, 200, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 134, 201, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 139, 167, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 137, 169, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 137, 172, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 136, 174, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 137, 178, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 143, 6, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 147, 5, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 160, 46, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 162, 48, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 153, 103, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 149, 109, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 147, 113, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 151, 114, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 143, 116, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 145, 116, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 153, 116, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 145, 120, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 157, 132, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 162, 135, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 165, 135, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 163, 140, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 161, 143, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 167, 143, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 152, 202, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 157, 200, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 159, 206, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 160, 201, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 163, 205, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 166, 202, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 167, 205, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 169, 200, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 171, 204, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 200, 162, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 190, 165, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 195, 165, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 192, 167, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 190, 8, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 187, 12, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 193, 12, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 196, 13, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 99, 27, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 101, 28, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 103, 29, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 100, 33, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 44, 185, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 171, 143, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 172, 146, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 176, 145, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 179, 146, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 177, 125, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 178, 127, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 173, 128, 2, 2, TILE_GROUP_BUSHES)
	add_wall_group(2, 177, 130, 2, 2, TILE_GROUP_BUSHES)
	
	add_no_enemy_zone(144, 152, 184, 191)

	if (not game_just_loaded) then
		add_enemies(2, { "ant", "gopher" }, 32, 3, 5, "grass", { "ant", "bazooka_ant", "bird", "sunflower", "gopher" })
		choppable_array = {}
		choppable_array[1]
			 = { name="bush", layer=2, x=8*TILE_SIZE+8, y=38*TILE_SIZE+8, dead=false }
		choppable_array[2]
			 = { name="bush", layer=2, x=10*TILE_SIZE+8, y=44*TILE_SIZE+8, dead=false }
		choppable_array[3]
			 = { name="bush", layer=2, x=8*TILE_SIZE+8, y=45*TILE_SIZE+8, dead=false }
		choppable_array[4]
			 = { name="bush", layer=2, x=9*TILE_SIZE+8, y=45*TILE_SIZE+8, dead=false }
		choppable_array[5]
			 = { name="bush", layer=2, x=10*TILE_SIZE+8, y=45*TILE_SIZE+8, dead=false }
		choppable_array[6]
			 = { name="bush", layer=2, x=49*TILE_SIZE+8, y=43*TILE_SIZE+8, dead=false }
		choppable_array[7]
			 = { name="bush", layer=2, x=39*TILE_SIZE+8, y=51*TILE_SIZE+8, dead=false }
		choppable_array[8]
			 = { name="bush", layer=2, x=40*TILE_SIZE+8, y=51*TILE_SIZE+8, dead=false }
		choppable_array[9]
			 = { name="bush", layer=2, x=65*TILE_SIZE+8, y=58*TILE_SIZE+8, dead=false }
		choppable_array[10]
			 = { name="bush", layer=2, x=66*TILE_SIZE+8, y=58*TILE_SIZE+8, dead=false }
		choppable_array[11]
			 = { name="bush", layer=2, x=65*TILE_SIZE+8, y=59*TILE_SIZE+8, dead=false }
		choppable_array[12]
			 = { name="bush", layer=2, x=78*TILE_SIZE+8, y=65*TILE_SIZE+8, dead=false }
		choppable_array[13]
			 = { name="bush", layer=2, x=73*TILE_SIZE+8, y=57*TILE_SIZE+8, dead=false }
		choppable_array[14]
			 = { name="bush", layer=2, x=73*TILE_SIZE+8, y=58*TILE_SIZE+8, dead=false }
		choppable_array[15]
			 = { name="bush", layer=2, x=74*TILE_SIZE+8, y=58*TILE_SIZE+8, dead=false }
		choppable_array[16]
			 = { name="bush", layer=2, x=74*TILE_SIZE+8, y=59*TILE_SIZE+8, dead=false }
		choppable_array[17]
			 = { name="bush", layer=2, x=58*TILE_SIZE+8, y=41*TILE_SIZE+8, dead=false }
		choppable_array[18]
			 = { name="bush", layer=2, x=61*TILE_SIZE+8, y=43*TILE_SIZE+8, dead=false }
		choppable_array[19]
			 = { name="bush", layer=2, x=62*TILE_SIZE+8, y=42*TILE_SIZE+8, dead=false }
		choppable_array[20]
			 = { name="bush", layer=2, x=91*TILE_SIZE+8, y=125*TILE_SIZE+8, dead=false }
		choppable_array[21]
			 = { name="bush", layer=2, x=100*TILE_SIZE+8, y=122*TILE_SIZE+8, dead=false }
		choppable_array[22]
			 = { name="bush", layer=2, x=102*TILE_SIZE+8, y=127*TILE_SIZE+8, dead=false }
		choppable_array[23]
			 = { name="bush", layer=2, x=103*TILE_SIZE+8, y=126*TILE_SIZE+8, dead=false }
		choppable_array[24]
			 = { name="bush", layer=2, x=83*TILE_SIZE+8, y=136*TILE_SIZE+8, dead=false }
		choppable_array[25]
			 = { name="bush", layer=2, x=84*TILE_SIZE+8, y=135*TILE_SIZE+8, dead=false }
		choppable_array[26]
			 = { name="bush", layer=2, x=82*TILE_SIZE+8, y=156*TILE_SIZE+8, dead=false }
		choppable_array[27]
			 = { name="bush", layer=2, x=84*TILE_SIZE+8, y=130*TILE_SIZE+8, dead=false }
		choppable_array[28]
			 = { name="bush", layer=2, x=100*TILE_SIZE+8, y=143*TILE_SIZE+8, dead=false }
		choppable_array[29]
			 = { name="bush", layer=2, x=102*TILE_SIZE+8, y=142*TILE_SIZE+8, dead=false }
		choppable_array[30]
			 = { name="bush", layer=2, x=135*TILE_SIZE+8, y=187*TILE_SIZE+8, dead=false }
		choppable_array[31]
			 = { name="bush", layer=2, x=135*TILE_SIZE+8, y=190*TILE_SIZE+8, dead=false }
		choppable_array[32]
			 = { name="bush", layer=2, x=135*TILE_SIZE+8, y=191*TILE_SIZE+8, dead=false }
		choppable_array[33]
			 = { name="bush", layer=2, x=136*TILE_SIZE+8, y=192*TILE_SIZE+8, dead=false }
		choppable_array[34]
			 = { name="bush", layer=2, x=128*TILE_SIZE+8, y=152*TILE_SIZE+8, dead=false }
		choppable_array[35]
			 = { name="bush", layer=2, x=129*TILE_SIZE+8, y=152*TILE_SIZE+8, dead=false }
		choppable_array[36]
			 = { name="bush", layer=2, x=128*TILE_SIZE+8, y=153*TILE_SIZE+8, dead=false }
		choppable_array[37]
			 = { name="bush", layer=2, x=129*TILE_SIZE+8, y=153*TILE_SIZE+8, dead=false }
		choppable_array[38]
			 = { name="bush", layer=2, x=127*TILE_SIZE+8, y=154*TILE_SIZE+8, dead=false }
		choppable_array[39]
			 = { name="bush", layer=2, x=128*TILE_SIZE+8, y=154*TILE_SIZE+8, dead=false }
		choppable_array[40]
			 = { name="bush", layer=2, x=127*TILE_SIZE+8, y=155*TILE_SIZE+8, dead=false }
		choppable_array[41]
			 = { name="bush", layer=2, x=128*TILE_SIZE+8, y=155*TILE_SIZE+8, dead=false }
		choppable_array[42]
			 = { name="bush", layer=2, x=127*TILE_SIZE+8, y=156*TILE_SIZE+8, dead=false }
		choppable_array[43]
			 = { name="bush", layer=2, x=128*TILE_SIZE+8, y=156*TILE_SIZE+8, dead=false }
		choppable_array[44]
			 = { name="bush", layer=2, x=136*TILE_SIZE+8, y=145*TILE_SIZE+8, dead=false }
		choppable_array[45]
			 = { name="bush", layer=2, x=139*TILE_SIZE+8, y=146*TILE_SIZE+8, dead=false }
		choppable_array[46]
			 = { name="bush", layer=2, x=122*TILE_SIZE+8, y=70*TILE_SIZE+8, dead=false }
		choppable_array[47]
			 = { name="bush", layer=2, x=125*TILE_SIZE+8, y=76*TILE_SIZE+8, dead=false }
		choppable_array[48]
			 = { name="bush", layer=2, x=129*TILE_SIZE+8, y=81*TILE_SIZE+8, dead=false }
		choppable_array[49]
			 = { name="bush", layer=2, x=155*TILE_SIZE+8, y=24*TILE_SIZE+8, dead=false }
		choppable_array[50]
			 = { name="bush", layer=2, x=156*TILE_SIZE+8, y=25*TILE_SIZE+8, dead=false }
		choppable_array[51]
			 = { name="bush", layer=2, x=159*TILE_SIZE+8, y=43*TILE_SIZE+8, dead=false }
		choppable_array[52]
			 = { name="bush", layer=2, x=154*TILE_SIZE+8, y=169*TILE_SIZE+8, dead=false }
		choppable_array[53]
			 = { name="bush", layer=2, x=157*TILE_SIZE+8, y=168*TILE_SIZE+8, dead=false }
		choppable_array[54]
			 = { name="bush", layer=2, x=166*TILE_SIZE+8, y=166*TILE_SIZE+8, dead=false }
		choppable_array[55]
			 = { name="bush", layer=2, x=155*TILE_SIZE+8, y=205*TILE_SIZE+8, dead=false }
		choppable_array[56]
			 = { name="bush", layer=2, x=157*TILE_SIZE+8, y=204*TILE_SIZE+8, dead=false }
		choppable_array[57]
			 = { name="bush", layer=2, x=169*TILE_SIZE+8, y=203*TILE_SIZE+8, dead=false }
		choppable_array[58]
			 = { name="bush", layer=2, x=171*TILE_SIZE+8, y=167*TILE_SIZE+8, dead=false }
		choppable_array[59]
			 = { name="bush", layer=2, x=185*TILE_SIZE+8, y=169*TILE_SIZE+8, dead=false }
		choppable_array[60]
			 = { name="bush", layer=2, x=186*TILE_SIZE+8, y=171*TILE_SIZE+8, dead=false }
		choppable_array[61]
			 = { name="bush", layer=2, x=177*TILE_SIZE+8, y=10*TILE_SIZE+8, dead=false }
		choppable_array[62]
			 = { name="bush", layer=2, x=178*TILE_SIZE+8, y=10*TILE_SIZE+8, dead=false }
		choppable_array[63]
			 = { name="bush", layer=2, x=159*TILE_SIZE+8, y=43*TILE_SIZE+8, dead=false }
	end
	spawn_choppable(choppable_array)

	if (milestone_is_complete(MILESTONE_NAME)) then
		if (not milestone_is_complete(CRYSTAL1_MILESTONE_NAME)) then
			crystal1 = add_entity("crystal", 2, EGBERT_SCENE_X-54, EGBERT_SCENE_Y+30)
		end
		if (not milestone_is_complete(CRYSTAL2_MILESTONE_NAME)) then
			crystal2 = add_entity("crystal", 2, EGBERT_SCENE_X-27, EGBERT_SCENE_Y+31)
		end
	end
	
	chest1 = Chest:new{x=113*TILE_SIZE+TILE_SIZE/2, y=16*TILE_SIZE, layer=2, contains_equipment=true, equipment_type=ARMOR, contains="BOX", quantity=1, milestone="ants_chest1"}
	chest2 = Chest:new{x=487, y=805, layer=2, contains="HEALTHVIAL", quantity=1, milestone="ants_chest2"}
	chest3 = Chest:new{x=123*TILE_SIZE+TILE_SIZE/2, y=123*TILE_SIZE, layer=2, contains="MAGICVIAL", quantity=1, milestone="ants_chest3"}
	chest4 = Chest:new{x=138*TILE_SIZE, y=147*TILE_SIZE, layer=2, contains="HEALTHVIAL", quantity=2, milestone="ants_chest4"}
	chest5 = Chest:new{x=57*TILE_SIZE+TILE_SIZE/2, y=26*TILE_SIZE, layer=2, contains_crystals=true, contains="CRYSTAL", quantity=1, milestone="ants_chest5"}

	first_exit = Active_Block:new{x=209, y=16, width=1, height=14}
	end_exit = Active_Block:new{x=209, y=104, width=1, height=16}
	anthill_entrance = Active_Block:new{x=163, y=169, width=3, height=1}
	anthill_exit = Active_Block:new{x=161, y=109, width=3, height=1}

	scene_start_block = Active_Block:new{x=151, y=186, width=13, height=1}
end

local CAMERA_Y_OFFSET = -60
local AMAYSA_FLY_SPEED = 400
local started_scene = false
local stopped_walking = false
local scene_start_egbert_x, scene_start_egbert_y
local started_talk = false
local started_impudence = false
local started_amaysa_talk = false

function set_amaysa_direction(tween, id, elapsed)
	if (right) then
		play_sample("sfx/amaysa_fly_l_r_short.ogg", 1, 0, 1)
	else
		play_sample("sfx/amaysa_fly_r_l.ogg", 1, 0, 1)
	end
	set_entity_right(amaysa, tween.right)
	return true
end

function start_amaysa_talk(tween, id, elapsed)
	started_amaysa_talk = true
	return true
end

function stop_walking(tween, id, elapsed)
	stopped_walking = true
	set_camera_offset(0, CAMERA_Y_OFFSET)
	set_entity_direction(0, DIR_N)
	set_entity_direction(1, DIR_N)
	return true
end

function set_tossed1(tween, id, elapsed)
	set_entity_layer(crystal1, 2)
	tossed1 = true
	return true
end

function play_thud_tween(tween, id, elapsed)
	play_sample("sfx/thud.ogg", 1, 0, 1)
	return true
end

function do_toss1(tween, id, elapsed)
	local x = EGBERT_SCENE_X-16
	local y = AMAYSA_Y-40
	local dx, dy = get_entity_position(1)
	dx = dx - 20
	local dist_x = dx - x
	local dist_y = dy - y
	local angle = math.atan2(dist_y, dist_x)
	local bounce1_x = dx + math.cos(angle) * 16
	local bounce1_y = dy + math.sin(angle) * 16
	local bounce2_x = bounce1_x + math.cos(angle) * 16
	local bounce2_y = bounce1_y + math.sin(angle) * 16
	crystal1 = add_entity("crystal", 4, x, y)
	play_sample("sfx/throw.ogg", 1, 0, 1)
	local t = create_toss_tween(crystal1, x, y, dx, dy, 1)
	append_tween(t, { run = play_thud_tween })
	append_tween(t, create_toss_tween(crystal1, dx, dy, bounce1_x, bounce1_y, 0.2))
	append_tween(t, { run = play_thud_tween })
	append_tween(t, create_toss_tween(crystal1, bounce1_x, bounce1_y, bounce2_x, bounce2_y, 0.2))
	append_tween(t, { run = play_thud_tween })
	append_tween(t, { run = set_tossed1 })
	new_tween(t)
	return true
end

function set_tossed2(tween, id, elapsed)
	set_entity_layer(crystal2, 2)
	tossed2 = true
	return true
end

function do_toss2(tween, id, elapsed)
	local x = EGBERT_SCENE_X-16
	local y = AMAYSA_Y-40
	local dx, dy = get_entity_position(1)
	local dist_x = dx - x
	local dist_y = dy - y
	local angle = math.atan2(dist_y, dist_x)
	local bounce1_x = dx + math.cos(angle) * 16
	local bounce1_y = dy + math.sin(angle) * 16
	local bounce2_x = bounce1_x + math.cos(angle) * 16
	local bounce2_y = bounce1_y + math.sin(angle) * 16
	crystal2 = add_entity("crystal", 4, x, y)
	play_sample("sfx/throw.ogg", 1, 0, 1)
	local t = create_toss_tween(crystal2, x, y, dx, dy, 1)
	append_tween(t, { run = play_thud_tween })
	append_tween(t, create_toss_tween(crystal2, dx, dy, bounce1_x, bounce1_y, 0.2))
	append_tween(t, { run = play_thud_tween })
	append_tween(t, create_toss_tween(crystal2, bounce1_x, bounce1_y, bounce2_x, bounce2_y, 0.2))
	append_tween(t, { run = play_thud_tween })
	append_tween(t, { run = set_tossed2 })
	new_tween(t)
	t = create_idle_tween(0.3)
	append_tween(t, create_astar_tween(1, dx-20, dy, true))
	append_tween(t, create_gesture_tween(1, "idle-up"))
	new_tween(t)
	return true
end

function holding_crystal(tween, id, elapsed)
	did_holding_crystal = true
	return true
end

function remove_crystal3(tween, id, elapsed)
	remove_entity(crystal3)
	return true
end

function ate_crystal(tween, id, elapsed)
	did_eat_crystal = true
	return true
end

function start_battle_talk(tween, id, elapsed)
	started_battle_talk = true
	set_entity_animation(ant1, "idle")
	set_entity_animation(ant2, "idle")
	return true
end

function finish_scene(tween, id, elapsed)
	set_character_role(1, "follow", 0)
	set_character_role(0, "")
	remove_entity(amaysa)
	set_milestone_complete(MILESTONE_NAME, true);
	set_entity_solid_with_area(0, true)
	set_entity_solid_with_entities(0, true)
	return true
end

function add_ants(tween, id, elapsed)
	local x, y = get_entity_position(amaysa)
	ant1 = add_entity("tough_ant", 4, x, y)
	set_entity_solid_with_entities(ant1, false)
	set_entity_solid_with_area(ant1, false)
	set_entity_animation(ant1, "jump-in-air")
	ant2 = add_entity("machete_ant", 4, x, y)
	set_entity_solid_with_entities(ant2, false)
	set_entity_solid_with_area(ant2, false)
	set_entity_animation(ant2, "jump-in-air")

	local ex, ey = get_entity_position(0)
	local fx, fy = get_entity_position(1)

	local x1 = (fx + x) / 2
	local y1 = (fy + y) / 2
	local x2 = (ex + x) / 2
	local y2 = (ey + y) / 2

	play_sample("sfx/throw.ogg", 1, 0, 1)
	local t = create_toss_tween(ant1, x, y, x1, y1, 1)
	append_tween(t, { run = play_thud_tween })
	new_tween(t)
	t = create_toss_tween(ant2, x, y, x2, y2, 1.1)
	append_tween(t, { run = play_thud_tween })
	append_tween(t, { run = start_battle_talk })
	new_tween(t)

	return true
end

function logic()
	update_choppable()

	if (first_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("meeting", DIR_E, 53, 384)
	elseif (end_exit:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("meeting", DIR_E, 43, 754)
	elseif (anthill_entrance:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("antcolony", DIR_S, 448, 506)
	elseif (anthill_exit:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("antcolony", DIR_S, 960, 86)
	end

	if (milestone_is_complete(MILESTONE_NAME)) then
		local x, y = get_entity_position(0)
		if (not (milestone_is_complete(CRYSTAL1_MILESTONE_NAME) and milestone_is_complete(CRYSTAL2_MILESTONE_NAME)) and not crystal_block:entity_is_colliding(0)) then
			set_entity_position(0, last_player_x, last_player_y)
			simple_speak{
				true,
				"MUST_GET_CRYSTALS", "", 1
			}
		elseif (not milestone_is_complete(JUMP_SWITCH_INFO_MILESTONE) and (y > 185*TILE_SIZE)) then
			set_entity_position(0, last_player_x, last_player_y)
			simple_speak{
				true,
				"SHORTCUT", "", 1
			}
		else
			last_player_x, last_player_y = get_entity_position(0)
		end
	end

	if (battle_started and not battle_started_logic) then
		battle_started_logic = true
		
		play_music("music/abw2.mid");

		set_entity_animation(1, "idle")
		set_entity_right(1, true)

		local t = create_center_view_tween()
		new_tween(t)

		simple_speak{
			true,
			"SCN5_FROGBERT_8", "", 1,
			"SCN5_EGBERT_12", "", 0,
			"SCN5_FROGBERT_9", "thinking", 1,
			"SCN5_EGBERT_13", "", 0,
			"SCN5_FROGBERT_10", "", 1,
			"SCN5_EGBERT_14", "", 0,
			"SCN5_FROGBERT_11", "", 1,
			"SCN5_EGBERT_15", "snooty", 0,
			"SCN5_FROGBERT_12", "", 1,
			"SCN5_EGBERT_16", "", 0
		}

		local ex, ey = get_entity_position(0)
		t = create_astar_tween(1, ex, ey, false)
		append_tween(t, { run = finish_scene })
		new_tween(t)
	elseif (started_battle_talk and not started_battle_talk_logic) then
		started_battle_talk_logic = true

		set_entity_animation(0, "idle")
		set_entity_right(0, false)

		simple_speak{
			true,
			"SCN5_EGBERT_11", "", 0
		}

		play_music("music/boss.mid");

		start_battle("grass1", "grass", true, "tough_ant", "machete_ant")

		remove_entity(ant1)
		remove_entity(ant2)

		battle_started = true
	elseif (did_eat_crystal and not ate_crystal_logic) then
		ate_crystal_logic = true

		set_entity_right(1, true)

		simple_speak{
			true,
			"SCN5_FROGBERT_6", "thinking", 1,
			"SCN5_EGBERT_9", "", 0,
			"SCN5_FROGBERT_7", "", 1,
			"SCN5_EGBERT_10", "", 0
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_8"), "", amaysa)

		set_entity_animation(amaysa, "left-right")
		set_entity_right(amaysa, false)
		play_sample("sfx/amaysa_fly_l_r_short.ogg", 1, 0, 1)
		local t = create_direct_move_tween(amaysa, EGBERT_SCENE_X-400, AMAYSA_Y, AMAYSA_FLY_SPEED)
		append_tween(t, { run = set_amaysa_direction, right = true })
		append_tween(t, create_direct_move_tween(amaysa, EGBERT_SCENE_X-120, AMAYSA_Y, AMAYSA_FLY_SPEED))
		append_tween(t, { run = add_ants })
		append_tween(t, create_direct_move_tween(amaysa, EGBERT_SCENE_X+50, EGBERT_SCENE_Y-220, AMAYSA_FLY_SPEED))
		new_tween(t)
	elseif (did_holding_crystal and not holding_crystal_logic) then
		holding_crystal_logic = true

		simple_speak{
			true,
			"SCN5_FROGBERT_5", "", 1
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_7"), "idle-crystal", amaysa)

		local x = EGBERT_SCENE_X-16
		local y = AMAYSA_Y-40
		crystal3 = add_entity("crystal", 4, x, y)
		play_sample("sfx/chomp_crystal.ogg", 1, 0, 1)
		local t = create_toss_tween(crystal3, x, y, EGBERT_SCENE_X, y-20, 4*64/1000)
		append_tween(t, { run = remove_crystal3 })
		new_tween(t)

		t = create_play_animation_tween(amaysa, "eat")
		append_tween(t, create_gesture_tween(amaysa, "idle"))
		append_tween(t, { run = ate_crystal })
		new_tween(t)
	elseif (tossed2 and not tossed2_logic) then
		tossed2_logic = true

		simple_speak{
			true,
			"SCN5_EGBERT_8", "", 0,
			"SCN5_FROGBERT_4", "", 1
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_6"), "", amaysa)

		local t = create_play_animation_tween(amaysa, "grab")
		append_tween(t, create_gesture_tween(amaysa, "idle-crystal"))
		append_tween(t, { run = holding_crystal })
		new_tween(t)
	elseif (tossed1 and not tossed1_logic) then
		tossed1_logic = true

		simple_speak{
			true,
			"SCN5_FROGBERT_3", "", 1
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_4"), "", amaysa)

		simple_speak{
			true,
			"SCN5_EGBERT_7", "", 0
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_5"), "impudence", amaysa)

		local t = create_play_animation_tween(amaysa, "grab")
		append_tween(t, create_gesture_tween(amaysa, "throw"))
		append_tween(t, create_idle_tween(4*64/1000))
		append_tween(t, { run = do_toss2 })
		append_tween(t, create_idle_tween(3*64/1000))
		append_tween(t, create_gesture_tween(amaysa, "idle"))
		new_tween(t)
	elseif (started_amaysa_talk and not (started_impudence)) then
		started_impudence = true

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_1"), "", amaysa)

		simple_speak{
			true,
			"SCN5_EGBERT_5", "", 0
		}

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_2"), "", amaysa)

		simple_speak{
			true,
			"SCN5_EGBERT_6", "", 0
		}

		play_sample("sfx/boing.ogg", 1, 0, 1)
		
		for i=0,(get_num_players()-1) do
			local id = get_player_id(i)
			stop_entity(id)
			set_entity_role_paused(id, true)
			set_entity_animation(id, "surprised")
		end

		speak_force_bottom(false, false, true, t("SCN5_AMAYSA_3"), "impudence", amaysa)

		for i=0,(get_num_players()-1) do
			local id = get_player_id(i)
			set_entity_animation(id, "idle-up")
			set_entity_role_paused(id, false)
		end

		local t = create_play_animation_tween(amaysa, "grab")
		append_tween(t, create_gesture_tween(amaysa, "throw"))
		append_tween(t, create_idle_tween(4*64/1000))
		append_tween(t, { run = do_toss1 })
		append_tween(t, create_idle_tween(3*64/1000))
		append_tween(t, create_gesture_tween(amaysa, "idle"))
		new_tween(t)
	elseif (stopped_walking and (not started_talk)) then
		started_talk = true

		simple_speak{
			true,
			"SCN5_EGBERT_1", "", 0,
			"SCN5_FROGBERT_1", "", 1,
			"SCN5_EGBERT_2", "", 0,
			"SCN5_EGBERT_3", "", 0,
			"SCN5_FROGBERT_2", "", 1,
			"SCN5_EGBERT_4", "", 0
		}
		
		play_music("music/boss_encounter.mid");
	
		local x, y = get_entity_position(0)

		AMAYSA_Y = y - 40

		amaysa = add_entity("amaysa", 3, x-400, AMAYSA_Y)
		set_entity_z(amaysa, 40)
		set_show_entity_shadow(amaysa, true)
		set_entity_animation_set_prefix(amaysa, "fly-")
		set_entity_animation(amaysa, "left-right")
		set_entity_right(amaysa, true)
		set_entity_solid_with_area(amaysa, false)
		set_entity_solid_with_entities(amaysa, false)

		play_sample("sfx/amaysa_fly_l_r.ogg", 1, 0, 1)
		local t = create_direct_move_tween(amaysa, x+400, AMAYSA_Y, AMAYSA_FLY_SPEED)
		append_tween(t, { run = set_amaysa_direction, right = false, started = false })
		append_tween(t, create_direct_move_tween(amaysa, x-400, AMAYSA_Y, AMAYSA_FLY_SPEED))
		append_tween(t, { run = set_amaysa_direction, right = true, started = false })
		append_tween(t, create_direct_move_tween(amaysa, x, AMAYSA_Y, AMAYSA_FLY_SPEED))
		append_tween(t, { run = start_amaysa_talk, started = false })
		new_tween(t)
	elseif (started_scene and (not stopped_walking)) then
		local dx = scene_start_egbert_x - EGBERT_SCENE_X
		local dy = scene_start_egbert_y - EGBERT_SCENE_Y
		local total_distance = math.sqrt(dx*dx + dy*dy)
		local x, y = get_entity_position(0)
		dx = x - scene_start_egbert_x
		dy = y - scene_start_egbert_y
		local current_distance = math.sqrt(dx*dx + dy*dy)
		local p = current_distance / total_distance
		set_camera_offset(0, p*CAMERA_Y_OFFSET)
	-- scene start
	elseif ((not milestone_is_complete(MILESTONE_NAME)) and scene_start_block:entity_is_colliding(0) and (not started_scene)) then
		save_boss_save()
		started_scene = true
		scene_start_egbert_x, scene_start_egbert_y = get_entity_position(0)
		set_character_role(0, "astar")
		local t = create_astar_tween(0, EGBERT_SCENE_X, EGBERT_SCENE_Y)
		append_tween(t, create_character_role_change_tween(1, "astar"))
		append_tween(t, create_astar_tween(1, EGBERT_SCENE_X-24, EGBERT_SCENE_Y, false))
		append_tween(t, { run = stop_walking, started = false })
		new_tween(t)
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
	destroy_sample("sfx/boing.ogg")
	destroy_sample("sfx/thud.ogg")
	destroy_sample("sfx/chomp_crystal.ogg")
	destroy_sample("sfx/amaysa_fly_l_r.ogg")
	destroy_sample("sfx/amaysa_fly_l_r_short.ogg")
	destroy_sample("sfx/amaysa_fly_r_l.ogg")
	destroy_sample("sfx/amaysa.ogg")
end

function activate(activator, activated)
	if (activated == crystal1) then
		remove_entity(crystal1)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL1_MILESTONE_NAME, true)
	elseif (activated == crystal2) then
		remove_entity(crystal2)
		add_crystals(1)
		play_sample("sfx/item_found.ogg", 1, 0, 1);
		set_milestone_complete(CRYSTAL2_MILESTONE_NAME, true)
	elseif (activated == chest1.id) then
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


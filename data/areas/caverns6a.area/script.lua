function start(game_just_loaded)
	play_music("music/caverns.mid")

	process_outline()

	-- stalactites
	add_wall_group(2, 10, 3, 2, 6, 0)
	add_wall_group(2, 15, 3, 2, 6, 0)

	chest = Chest_Puzzle:new{layer=2, x=13.5*TILE_SIZE, y=7.5*TILE_SIZE, milestone="chest_puzzle1", other_ms_1="chest_puzzle2", other_ms_2="chest_puzzle3", other1=true, other2=true}

	to_caverns5 = Active_Block:new{x=12, y=20, width=3, height=1}

	load_sample("sfx/chest_puzzle.ogg", false)
end

local do_battle = false

function activate(activator, activated)
	if (activated == chest.id) then
		if (chest:open()) then
			do_battle = true
		end
	end
end

function logic()
	if (to_caverns5:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("caverns5", DIR_S, 8.5*TILE_SIZE, 9.5*TILE_SIZE)
	end

	if (do_battle) then
		do_battle = false
		start_battle("caverns1", "caverns", false, "chest")
		chest:close()
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
	destroy_sample("sfx/chest_puzzle.ogg")
end


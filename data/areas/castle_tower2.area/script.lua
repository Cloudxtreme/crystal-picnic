function start()
	play_music("music/castle.mid");

	process_outline()

	going_up = Active_Block:new{x=12, y=5, width=2, height=2}
	going_down = Active_Block:new{x=2, y=5, width=2, height=2}
end

function logic()
	if (going_up:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_tower3", DIR_S, 208, 135)
	elseif (going_down:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_tower1", DIR_S, 48, 135)
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

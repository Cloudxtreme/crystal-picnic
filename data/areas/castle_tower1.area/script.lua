function start()
	play_music("music/castle.mid");

	process_outline()

	going_up = Active_Block:new{x=2, y=5, width=2, height=2}
	going_out = Active_Block:new{x=6, y=13, width=4, height=2}
end

function logic()
	if (going_up:entity_is_colliding(0)) then
		next_player_layer = 2
		change_areas("castle_tower2", DIR_S, 48, 135)
	elseif (going_out:entity_is_colliding(0)) then
		next_player_layer = 3
		change_areas("castle", DIR_S, 575, 520)
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

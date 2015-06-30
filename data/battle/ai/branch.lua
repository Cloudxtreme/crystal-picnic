function start(this_id)
	id = this_id
	set_battle_entity_flying(id, true)
	set_battle_entity_unhittable(id, true)
	set_battle_entity_layer(id, 3)
	set_entity_stops_battle_end(id, false)

	push_entity_to_front(id)
end

function logic()
	local elapsed = get_skeleton_animation_elapsed_time(id)
	if (elapsed >= 1050 and elapsed < 1550) then
		set_battle_entity_attacking(id, true)
	else
		set_battle_entity_attacking(id, false)
	end
end

function get_attack_sound()
	return ""
end

function decide()
	return "nil"
end

function collide(with, me)
end

function get_should_auto_attack()
	return false
end


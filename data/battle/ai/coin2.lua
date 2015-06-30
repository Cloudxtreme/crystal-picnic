function start(this_id)
	id = this_id
	set_battle_entity_unhittable(id, true)
	set_battle_entity_jumping(id)
end

function get_attack_sound()
	return ""
end

function decide()
	return "nil"
end

function collide(with, me)
	local name = get_entity_name(with)
	if (not (name == "egbert" or name == "frogbert" or name == "bisou")) then
		return
	end
	play_sample("sfx/coin.ogg", 1, 0, 1)
	add_cash(25)
	remove_entity(id)
end

function get_should_auto_attack()
	return false
end

function logic()
	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

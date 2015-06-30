function start(this_id)
	id = this_id
	set_hp(id, 15)
	set_battle_entity_attack(id, 3)
end

function get_attack_sound()
	return ""
end

function decide()
	if (get_hp(id) <= 0) then
		return "nil nostop"
	else
		return "seek within 0 20 A_PLAYER nostop"
	end
end

function get_should_auto_attack()
	return true
end

function stop()
end

function die()
	if (rand(5) == 0) then
		local coin = add_battle_enemy("coin1")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	elseif (rand(5) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function logic()
	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

function start(this_id)
	id = this_id
	set_hp(id, 20)
	set_battle_entity_attack(id, 3)
	set_enemy_aggressiveness(id, 10)
end

function get_attack_sound()
	return ""
end

function decide()
	-- bounce
	local r = (rand(1000)/1000) * 2
	return "seek within 0 5 A_PLAYER nostop"
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin2")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function get_should_auto_attack()
	return true
end

function logic()
	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

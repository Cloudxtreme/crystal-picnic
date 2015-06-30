function start(this_id)
	id = this_id
end

function get_attack_sound()
	return "sfx/swing_weapon.ogg"
end

function decide()
	local r = (rand(1000)/1000) * 2
	return "seek within 0 20 A_PLAYER nostop rest " .. (r+0.5) .. " nostop"
end

function die()
	if (rand(3) == 0) then
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

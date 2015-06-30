function start(this_id)
	id = this_id
	set_battle_entity_flying(id, true)
	local x = get_entity_position(id)
	local y = get_highest_point() - 55
	set_entity_position(id, x, y)
	load_sample("sfx/fart.ogg", false)
	load_sample("sfx/splat.ogg", false)
	load_sample("sfx/splat_hit.ogg", false)
end

function get_attack_sound()
	return ""
end

function move()
	local x, y = get_entity_position(id)
	local r = (rand(1000)/1000) * 100 + 250
	if (rand(2) == 0) then
		x = x + r
	else
		x = x - r
	end
	-- NOTE: sz must be > half birds bones size
	local sz = 30
	if (x < sz) then
		x = sz
	elseif (x >= get_battle_width()-sz) then
		x = get_battle_width()-sz-1
	end
	return "direct_move " .. x .. " -1 nostop"
end

function rest()
	return "rest 0.25 nostop"
end

function poop()
	local x, y = get_entity_position(id)
	local poop_id = add_battle_enemy("bird_poop")
	set_entity_position(poop_id, x, y)
	set_entity_right(poop_id, get_entity_right(id))
end

local move_next = true

function decide()
	if (move_next) then
		move_next = false
		return move()
	else
		move_next = true
		poop()
		return rest()
	end
end

function get_should_auto_attack()
	return false
end

function die()
	if (rand(5) == 0) then
		local coin = add_battle_enemy("coin1")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	elseif (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end

function stop()
	destroy_sample("sfx/fart.ogg")
	destroy_sample("sfx/splat.ogg")
	destroy_sample("sfx/splat_hit.ogg")
end

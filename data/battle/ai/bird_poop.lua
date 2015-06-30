function start(this_id)
	id = this_id
	set_battle_entity_flying(id, true)
	set_battle_entity_attacking(id, true)
	play_sample("sfx/fart.ogg", 1, 0, 1)
end

function get_attack_sound()
	return ""
end

function get_hit_sound()
	return "sfx/splat_hit.ogg";
end

local x_speed = 0.5
local y_speed = 2.0
local step = 1

function dir()
	if (get_entity_right(id)) then
		return 1
	else
		return -1
	end
end

function splat(x, y)
	local pgid = add_particle_group("bird_poop", 0, PARTICLE_HURT_NONE, "poop_splat1", "poop_splat2", "poop_splat3")
	local pid = add_particle(
		pgid,
		0,
		0,
		1,
		1,
		1,
		1,
		0,
		0,
		get_entity_right(id),
		false
	)
	set_particle_position(pid, x, y)
	set_particle_blackboard(pid, 0, 0)
	remove_entity(id)
end

function logic()
	local x, y = get_entity_position(id)
	if (y >= get_battle_height()) then
		remove_entity(id)
	elseif (get_battle_entity_hit_something_this_attack(id)) then
		splat(x, y)
	elseif (battle_entity_is_colliding_with_area(id)) then
		play_sample("sfx/splat.ogg", 1, 0, 1)
		splat(x, y)
	else
		x = x + x_speed * step * dir()
		y = y + y_speed * step
		set_entity_position(id, x, y)
	end
end

function decide()
	return "rest 1 nostop"
end


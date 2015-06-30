local NUM_ACORN_SPOTS = 5
local acorn_spot = 1

function start(level_name)
	add_parallax_bitmap("of_back.png", false)
	add_parallax_bitmap("of_mid.png", false)

	set_battle_entity_default_layer(2);

	acorn_spots = {
		{ x=245, y=21 },
		{ x=114, y=98 },
		{ x=334, y=59 },
		{ x=166, y=197 },
		{ x=409, y=265 }
	}
	
	load_sample("sfx/rumble.ogg", true)
end

function stop()
	destroy_sample("sfx/rumble.ogg")
end

function oldoak_end_shake(tween, id, elapsed)
	stop_sample("sfx/rumble.ogg")
	local num = get_num_platforms()
	for i=1,num-1 do
		set_platform_solid(i-1, true)
	end
	set_battle_entity_unhittable(tween.oldoak, false)
	return true
end

function oldoak_do_shake(tween, id, elapsed)
	play_sample("sfx/rumble.ogg", 1, 0, 1)
	local num_entities = get_num_entities_in_battle()
	for i=1,num_entities do
		local ent_id = get_battle_entity_id_by_number(i-1)
		set_battle_entity_jumping(ent_id, true)
		local name = get_entity_name(ent_id)
		if (name == "egbert" or name == "frogbert" or name == "bisou") then
			if (get_hp(ent_id > 0)) then
				set_entity_animation(ent_id, "jump-in-air")
			end
		end
	end
	local num = get_num_platforms()
	for i=1,num-1 do
		set_platform_solid(i-1, false)
	end
	shake(0.0, 2.0, 2)
	local t = create_idle_tween(2.0)
	append_tween(t, { run = oldoak_end_shake, oldoak = tween.oldoak })
	new_tween(t)
	return true
end

function oldoak_end_angry(tween, id, elapsed)
	set_entity_animation(tween.oldoak, "battle-idle")
	return true
end

function oldoak_get_angry(tween, id, elapsed)
	set_entity_animation(tween.oldoak, "angry")
	local t = create_idle_tween(0.2)
	append_tween(t, { run = oldoak_do_shake, oldoak = tween.oldoak })
	new_tween(t)
	local t2 = create_idle_tween(1.0)
	append_tween(t2, { run = oldoak_end_angry, oldoak = tween.oldoak })
	new_tween(t2)
	return true
end

function oldoak_hit(id)
	set_battle_entity_unhittable(id, true)
	local t = create_idle_tween(0.6)
	append_tween(t, { run = oldoak_get_angry, oldoak = id })
	new_tween(t)
end

function drop_acorn()
	play_sample("sfx/acorn_drop.ogg", 1, 0, 1)

	local x = acorn_spots[acorn_spot].x
	local y = acorn_spots[acorn_spot].y

	acorn_spot = acorn_spot + 1
	if (acorn_spot > NUM_ACORN_SPOTS) then
		acorn_spot = 1
	end

	local pgid = add_particle_group("acorn", 0, PARTICLE_HURT_PLAYER,
		"acorn/1",
		"acorn/2",
		"acorn/3",
		"acorn/4",
		"acorn/5",
		"acorn/6",
		"acorn/7",
		"acorn/8",
		"acorn/9",
		"acorn/10",
		"acorn/11",
		"acorn/12",
		"acorn/13",
		"acorn/14",
		"acorn/15",
		"acorn/16"
	)
	local acorn = add_particle(
		pgid,
		4, 4,
		1, 1, 1, 1,
		0,
		HIT_DOWN,
		true,
		false
	)
	set_particle_position(acorn, x, y)
	set_particle_blackboard(acorn, 0, 16) -- frames
	set_particle_blackboard(acorn, 1, 0) -- frame
	set_particle_blackboard(acorn, 2, 0) -- tick
	set_particle_blackboard(acorn, 3, 1) -- rotate right
	set_particle_blackboard(acorn, 4, 0) -- colliding with level
end

function drop_acorn_tween(tween, id, elapsed)
	drop_acorn()
	return true
end

function drop_lots_of_acorns()
	local t = create_idle_tween(0.8)
	append_tween(t, { run = drop_acorn_tween })
	append_tween(t, create_idle_tween(0.1))
	append_tween(t, { run = drop_acorn_tween })
	append_tween(t, create_idle_tween(0.1))
	append_tween(t, { run = drop_acorn_tween })
	append_tween(t, create_idle_tween(0.1))
	append_tween(t, { run = drop_acorn_tween })
	append_tween(t, create_idle_tween(0.1))
	append_tween(t, { run = drop_acorn_tween })
	new_tween(t)
end

function slow_wolf_tween(tween, id, elapsed)
	set_can_accelerate_quickly(tween.wolf, false);
	return true
end

function slow_wolf(id)
	local t = create_idle_tween(0.15)
	append_tween(t, { run = slow_wolf_tween, wolf = id })
	new_tween(t)
end

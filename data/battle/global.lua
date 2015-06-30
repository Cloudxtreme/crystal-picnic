ABILITY_ATTACK = 3
ABILITY_ICE = 5
ABILITY_SLASH = 6
ABILITY_THROW = 7
ABILITY_KICK = 8
ABILITY_PLANT = 9
ABILITY_FIRE = 10
ABILITY_ROLL = 11
ABILITY_BURROW = 12
ABILITY_HEAL = 13

function play_sample_later(name, delay)
	local t
	if (delay > 0) then
		t = create_idle_tween(delay)
	else
		t = {}
	end

	append_tween(t, create_play_sample_tween(name))

	new_tween(t)
end

function add_heal_drop(tween, id, elapsed)
	local drop = add_particle(
		tween.pgid,
		1, 1,
		1, 1, 1, 1,
		0,
		0,
		true,
		false
	)

	set_particle_position(drop, tween.x, tween.y)
	
	set_particle_blackboard(drop, 0, tween.entity_start_x)
	set_particle_blackboard(drop, 1, tween.entity_start_y)
	set_particle_blackboard(drop, 2, tween.entity_id)
	set_particle_blackboard(drop, 3, 0)
	set_particle_blackboard(drop, 4, 18)
	set_particle_blackboard(drop, 5, 2)
	set_particle_blackboard(drop, 6, 0)

	return true
end

function heal_player(tween, id, elapsed)
	play_sample("sfx/heal_drop.ogg")

	increase_hp(tween.entity_id, tween.amount)

	local x, y = get_entity_position(tween.entity_id)

	local pgid = add_particle_group(
		"heal_part",
		0,
		PARTICLE_HURT_NONE,
		"heal_star/1",
		"heal_star/2",
		"heal_star/3",
		"heal_star/4",
		"heal_star/5",
		"heal_star/6",
		"heal_star/7",
		"heal_star/8"
	)

	local star = add_particle(
		pgid,
		1, 1,
		1, 1, 1, 1,
		0,
		0,
		true,
		false
	)

	local w, h = get_entity_animation_size(tween.entity_id)

	set_particle_position(star, x, y-32)

	set_particle_blackboard(star, 0, x)
	set_particle_blackboard(star, 1, y)
	set_particle_blackboard(star, 2, tween.entity_id)
	set_particle_blackboard(star, 3, 0)
	set_particle_blackboard(star, 4, 8)
	set_particle_blackboard(star, 5, 4)
	set_particle_blackboard(star, 6, 0)

	pgid = add_particle_group(
		"heal_part",
		0,
		PARTICLE_HURT_NONE,
		"heal_drop/1",
		"heal_drop/2",
		"heal_drop/3",
		"heal_drop/4",
		"heal_drop/5",
		"heal_drop/6",
		"heal_drop/7",
		"heal_drop/8",
		"heal_drop/9",
		"heal_drop/10",
		"heal_drop/11",
		"heal_drop/12",
		"heal_drop/13",
		"heal_drop/14",
		"heal_drop/15",
		"heal_drop/16",
		"heal_drop/17",
		"heal_drop/18"
	)

	for i=0,31 do
		local t = create_idle_tween(LOGIC_MILLIS/1000*i)
		append_tween(t, { run=add_heal_drop, x=x-32+i*2, y=y-24, entity_start_x=x, entity_start_y=y, entity_id=tween.entity_id, pgid=pgid })
		new_tween(t)
	end

	return true
end

function heal(id, amount)
	local t = create_idle_tween(0.2)
	append_tween(t, { run = heal_player, entity_id = id, amount = amount })
	new_tween(t)
end

function player_ability_button(player_id, ability)
	for i=0,3 do
		if (get_ability_name(player_id, true, i) == ability) then
			return i
		end
	end
	return nil
end

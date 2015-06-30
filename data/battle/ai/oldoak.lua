local ACORN_TIME = LOGIC_RATE*6
local next_acorn_count = ACORN_TIME
local wolf_left = true

function start(this_id)
	id = this_id
	set_battle_entity_flying(id, true)
	set_entity_position(id, 246, 126)
	set_hp(id, 30)

	-- create branch enemies
	branches = {}
	branches[1] = add_battle_enemy("branch")
	set_entity_position(branches[1], 90, 260)
	set_entity_right(branches[1], true)
	branches[2] = add_battle_enemy("branch")
	set_entity_position(branches[2], 175, 130)
	set_entity_right(branches[2], false)
	branches[3] = add_battle_enemy("branch")
	set_entity_position(branches[3], 280, 220)
	set_entity_right(branches[3], false)

	load_sample("sfx/acorn_drop.ogg", false)
	load_sample("sfx/knock.ogg", false)

	push_entity_to_front(id)
end

function add_wolf()
	local x
	local x_force = 3
	local right = wolf_left

	if (wolf_left) then
		x = 0
	else
		x = get_battle_width()-1
	end

	local wolf = add_battle_enemy("wolf")
	set_entity_position(wolf, x, get_battle_height()-80)
	set_entity_right(wolf, right)
	set_can_accelerate_quickly(wolf, true)
	apply_force(wolf, HIT_UP, right, x_force, 0.5)
	call_on_battle_script("slow_wolf", wolf)
	play_sample("sfx/enemy_jump.ogg", 1, 0, 1)

	wolf_left = not wolf_left
end

function logic()
	next_acorn_count = next_acorn_count - 1
	if (next_acorn_count <= 0) then
		next_acorn_count = ACORN_TIME
		call_on_battle_script("drop_acorn")
	end

	local have_oldoak = false
	local num_wolves = 0
	local num_entities = get_num_entities_in_battle()
	for i=1,num_entities do
		local ent_id = get_battle_entity_id_by_number(i-1)
		local name = get_entity_name(ent_id)
		if (not (name == nil)) then
			if (name == "wolf") then
				num_wolves = num_wolves + 1
			elseif (name == "oldoak") then
				have_oldoak = true
			end
		end
	end

	if (have_oldoak) then
		local needed = 2 - num_wolves
		for i=1,needed do
			add_wolf()
		end
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

function got_hit(hitter)
	if (get_hp(id) <= 0) then
		return
	end
	call_on_battle_script("oldoak_hit", id)

	call_on_battle_script("drop_lots_of_acorns")
end

function stop()
	destroy_sample("sfx/acorn_drop.ogg")
	destroy_sample("sfx/knock.ogg")
end


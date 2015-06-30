local hp = 10

function start(this_id)
	id = this_id
	set_entity_right(id, true)
	set_entity_immovable(id, true)
	set_battle_entity_attack(id, 8)
	
	push_entity_to_front(id)

	load_sample("sfx/antboss_laugh.ogg", false)
	load_sample("sfx/antboss_yell.ogg", false)
	load_sample("sfx/thud.ogg", false)

	set_hp(id, 1000000)
end

local attack_times = {
	{ 3, 3, 3 },
	{ 2, 2, 2 },
	{ 2, 1, 1 },
	{ 1, 1, 1 },
	{ 1, 1, 0.5 }
}

local last_frame = 1
local current_attack = 1
local done_intro = 0.0
local dying = false

function logic()
	if (done_intro == 0.0 or dying or get_hp(get_player_id(0)) <= 0) then
		return
	end

	local i
	local count = 0
	for i=1,#attack_times[current_attack] do
		count = count + attack_times[current_attack][i]
	end

	local t = (get_time()-done_intro) % count

	local current_frame = 1
	count = 0

	for i=1,#attack_times[current_attack] do
		count = count + attack_times[current_attack][i]
		if (t < count) then
			break
		end
		current_frame = current_frame + 1
	end

	if (not (current_frame == last_frame)) then
		last_frame = current_frame
		set_should_attack(id, true)
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

function stop()
	destroy_sample("sfx/antboss_laugh.ogg")
	destroy_sample("sfx/antboss_yell.ogg")
	destroy_sample("sfx/thud.ogg")
end

function got_hit(hitter)
	hp = hp - 1
	if (hp == 8) then
		current_attack = 2
	elseif (hp == 6) then
		current_attack = 3
	elseif (hp == 4) then
		current_attack = 4
	elseif (hp == 3) then
		call_on_battle_script("set_seventh_hit_time")
	elseif (hp == 2) then
		current_attack = 5
	elseif (hp == 0) then
		dying = true
		play_sample("sfx/antboss_yell.ogg", 1, 0, 1)
		set_battle_entity_attacking(id, false)
		set_entity_animation(id, "falling")
		call_on_battle_script("make_bigant_fall")
	end
end

function start_attacking()
	done_intro = get_time()
end


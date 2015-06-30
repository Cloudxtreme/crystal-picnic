seventh_hit_time = 0.0

function start(level_name)
	play_music("music/final_boss.mid")
end

local spoke1 = false

function laugh(tween, id, elapsed)
	play_sample("sfx/thud.ogg", 1, 0, 1)
	play_sample("sfx/antboss_laugh.ogg", 1, 0, 1)
	set_entity_animation(antboss, "laugh")
	do_speech = true
	return true
end

function enable_input(tween, id, elapsed)
	set_entity_input_disabled(tween.player, false)
	return true
end

function logic()
	local player = get_player_id(0)
	local px, py = get_entity_position(player)
	local x1 = get_battle_width()/2 - 80
	local x2 = get_battle_width()/2 + 80
	local y = 200
	if (get_hp(player) > 0) then
		if (px < x1) then
			stop_entity(player)
			set_entity_position(player, x1, py)
			set_entity_input_disabled(player, true)
			set_battle_entity_jumping(player)
			set_entity_right(player, false)
			set_entity_animation(player, "hit")
			local t = create_direct_move_tween(player, x1+10, py, 200)
			append_tween(t, { run = enable_input, player = player })
			new_tween(t)
		elseif (px > x2) then
			stop_entity(player)
			set_entity_position(player, x2, py)
			set_entity_input_disabled(player, true)
			set_battle_entity_jumping(player)
			set_entity_right(player, true)
			set_entity_animation(player, "hit")
			local t = create_direct_move_tween(player, x2-10, py, 200)
			append_tween(t, { run = enable_input, player = player })
			new_tween(t)
		end
	end

	if (player_hit) then
		player_hit = false
		stop_entity(player)
		set_entity_input_disabled(player, true)
		set_battle_entity_jumping(player)
		set_entity_right(player, false)
		set_entity_animation(player, "hit")
		local t = create_toss_tween(player, px, py-5, x2-10, y, 0.5)
		append_tween(t, { run = enable_input, player = player })
		new_tween(t)

		play_sample("sfx/antboss_laugh.ogg", 1, 0, 1)
	end


	if (spoke1 and do_speech) then
		do_speech = false

		speak_top(false, false, true, "<b><color 255 255 255>" .. t("FROGBERT") .. ":</color></b> " .. t("CART_BATTLE_FROGBERT_2"), "", -1)

		set_entity_input_disabled(player, false)

		set_entity_animation(antboss, "battle-idle")

		call_on_battle_entity_script(antboss, "start_attacking")
	elseif (not spoke1) then
		spoke1 = true

		set_entity_input_disabled(player, true)

		speak_top(false, false, true, "<b><color 255 255 255>" .. t("FROGBERT") .. ":</color></b> " .. t("CART_BATTLE_FROGBERT_1"), "", -1)

		antboss = add_battle_enemy("antboss")
		redraw()

		set_show_entity_shadow(antboss, false);

		set_entity_position(antboss, 80, y)

		set_entity_animation(antboss, "charge")	

		local t = create_direct_move_tween(antboss, 180, y, 100/0.4)
		append_tween(t, { run = laugh })
		new_tween(t)
	end
end

function set_seventh_hit_time()
	seventh_hit_time = get_time()
end

function launch_player()
	local player = get_player_id(0)
	if (get_hp(player) <= 0) then
		local px, py = get_entity_position(player)
		stop_entity(player)
		set_battle_entity_jumping(player)
		set_entity_right(player, false)
		set_entity_animation(player, "hit")
		local t = create_toss_tween(player, px, py-5, get_battle_width()/2 + 192/2 + 64, 300, 2)
		new_tween(t)

		egbert_hit = add_battle_entity("egbert_hit")
		px = get_battle_width()/2 + 192/2 - 72 + 10
		py = 200
		set_entity_position(egbert_hit, px, py)
		set_entity_right(egbert_hit, false)
		t = create_toss_tween(egbert_hit, px, py, get_battle_width()/2 + 192/2 + 54, 300, 2)
		new_tween(t)

		bisou_hit = add_battle_entity("bisou_hit")
		px = get_battle_width()/2 + 192/2 - 72 + 54
		py = 200
		set_entity_position(bisou_hit, px, py)
		set_entity_right(bisou_hit, false)
		t = create_toss_tween(bisou_hit, px, py, get_battle_width()/2 + 192/2 + 74, 300, 2)
		new_tween(t)

		play_sample("sfx/antboss_laugh.ogg", 1, 0, 1)
	else
		player_hit = true
	end
end

function kill(tween, id, elapsed)
	kill_enemy(antboss)
	return true
end

function make_bigant_fall()
	local x, y = get_entity_position(antboss)
	local t = create_toss_tween(antboss, x, y, x-300, y+100, 1)
	append_tween(t, { run = kill })
	new_tween(t)
end

function play_bomb(tween, id, elapsed)
	play_sample("sfx/bomb.ogg", 1, math.random()*2-1, 1)
	return true
end

function crash()
	shake(0.0, 3.0, 10)
	for i=1,20 do
		local t = create_idle_tween(math.random()*2)
		append_tween(t, { run = play_bomb })
		new_tween(t)
	end
end

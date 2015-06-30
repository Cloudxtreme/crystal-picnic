function start(this_id)
	id = this_id
	hole_bmps = {}
	hole_bmps[1] = load_bitmap("battle/misc_graphics/gopher_hole/1.png")
	hole_bmps[2] = load_bitmap("battle/misc_graphics/gopher_hole/2.png")
	hole_bmps[3] = load_bitmap("battle/misc_graphics/gopher_hole/3.png")
	hole_bmps[4] = load_bitmap("battle/misc_graphics/gopher_hole/4.png")
end

function get_attack_sound()
	return ""
end

local DIRT_SPEED = 4
local dirt_y

function new_particle()
	play_sample("sfx/plant_shovel.ogg", 1, 0, 1)
	local dirt = add_particle(
		pgid,
		1, 1,
		1, 1, 1, 1,
		0,
		HIT_SIDE,
		true,
		false
	)
	local x = get_entity_position(id)
	local y = dirt_y
	set_particle_position(dirt, x, y)
	set_particle_blackboard(dirt, 0, x)
	set_particle_blackboard(dirt, 1, y)
	local angle = math.random() * (math.pi-math.pi/4) + math.pi + math.pi/8
	local vx = math.cos(angle) * DIRT_SPEED
	local vy = math.sin(angle) * DIRT_SPEED
	set_particle_blackboard(dirt, 2, vx)
	set_particle_blackboard(dirt, 3, vy)
	set_particle_blackboard(dirt, 4, 6)
	set_particle_blackboard(dirt, 5, 0) -- life
end

function start_particles()
	pgid = add_particle_group("fading_outward_shooting", 0, PARTICLE_HURT_NONE, "dirt_chunk")
end

local SEARCH_ANGLE = -math.pi/6
local SEARCH_DIST = 60
local JUMP_FORCE_X = 5
local JUMP_FORCE_Y = 0.6
local WIDTH = 64
local HEIGHT = 64

local next_attack = 0
local should_attack = false
local attack_timeout = 0

local DIG_START = 1
local DIG_WAIT = 2
local DIGGING_OUT = 3

local DIG_TIME = 150
local WAIT_TIME = 5*60

local digging = false
local dig_stage = DIG_START
local dig_count = 0
local dig_start_y
local particle_count = 0

function logic()
	if (get_hp(id) <= 0) then
		return
	end

	if (digging) then
		dig_count = dig_count + 1
		if (dig_stage == DIG_START) then
			local x = get_entity_position(id)
			local y = dig_start_y + (dig_count/DIG_TIME) * HEIGHT
			set_entity_position(id, x, y)
			if (dig_count == DIG_TIME) then
				dig_stage = DIG_WAIT
				set_entity_visible(id, false)
				dig_count = 0
			end
			particle_count = particle_count + 1
			if (particle_count == 5) then
				new_particle()
				particle_count = 0
			end
		elseif (dig_stage == DIG_WAIT) then
			if (dig_count == WAIT_TIME) then
				dig_stage = DIGGING_OUT
				set_entity_visible(id, true)
				local x, y = get_random_start_platform(0, 20)
				dig_start_y = y-2
				gopher_y = dig_start_y+HEIGHT
				set_entity_position(id, x, gopher_y)
				dig_count = 0
				start_particles()
				dirt_y = y
			end
		elseif (dig_stage == DIGGING_OUT) then
			local x = get_entity_position(id)
			local y = gopher_y - (dig_count/DIG_TIME) * HEIGHT
			set_entity_position(id, x, y)
			if (dig_count == DIG_TIME) then
				digging = false
				set_battle_entity_unhittable(id, false)
				set_entity_immovable(id, false)
				set_battle_entity_jumping(id)
				next_attack = 0
				dig_stage = DIG_START
				dig_count = 0
			end
			particle_count = particle_count + 1
			if (particle_count == 5) then
				new_particle()
				particle_count = 0
			end
		end
		return
	end

	if (should_attack) then
		if (get_height_from_ground(id) > 2) then
			set_should_attack(id, true)
			should_attack = false
		else
			attack_timeout = attack_timeout - 1
			if (attack_timeout <= 0) then
				should_attack = false
			end
		end
		return
	end

	if (next_attack > 0) then
		next_attack = next_attack - 1
		return
	end

	local x, y = get_entity_position(id)
	y = y - 5

	local a, jump_force_x
	if (get_entity_right(id)) then
		a = SEARCH_ANGLE
		jump_force_x = JUMP_FORCE_X
	else
		a = SEARCH_ANGLE + math.pi
		jump_force_x = -JUMP_FORCE_X
	end

	local x2 = x + math.cos(a) * SEARCH_DIST
	local y2 = y + math.sin(a) * SEARCH_DIST

	if (checkcoll_line_player(x, y, x2, y2)) then
		play_sample("sfx/enemy_jump.ogg", 1, 0, 1)
		apply_force(id, HIT_UP, get_entity_right(id), jump_force_x, JUMP_FORCE_Y)
		set_battle_entity_jumping(id)
		set_entity_animation(id, "jump-in-air")
		should_attack = true
		next_attack = 15 -- 15 = quarter second
		attack_timeout = 5
	end

	local x, y = get_entity_position(id)
	local h = get_battle_height()
	if (y >= h) then
		remove_entity(id)
	end
end

function decide()
	if (next_attack > 0 or digging or get_hp(id) <= 0) then
		return "nil nostop"
	end
	r = rand(4)
	if (r == 0 and entity_is_on_ground(id) and get_height_from_ground(id) <= 1 and get_distance_from_nearest_edge(id) > 16) then
		digging = true
		set_battle_entity_unhittable(id, true)
		set_entity_immovable(id, true)
		local unused
		unused, dig_start_y = get_entity_position(id)
		start_particles()
		dirt_y = dig_start_y
		return "nil nostop"
	else
		return "seek within 0 20 A_PLAYER nostop rest 0.25 nostop"
	end
end

function get_should_auto_attack()
	return false
end

function stop()
	destroy_bitmap("battle/misc_graphics/gopher_hole/1.png")
	destroy_bitmap("battle/misc_graphics/gopher_hole/2.png")
	destroy_bitmap("battle/misc_graphics/gopher_hole/3.png")
	destroy_bitmap("battle/misc_graphics/gopher_hole/4.png")
end

function get_clip_position()
	local x = get_entity_position(id)
	local top_x, top_y = get_battle_top()
	x = x - top_x
	x = x - WIDTH/2
	y = dig_start_y - top_y
	y = y - HEIGHT
	return x, y
end

function pre_draw()
	if (digging) then
		if (dig_stage == DIG_START or dig_stage == DIGGING_OUT) then
			if (dig_stage == DIG_START) then
				set_entity_animation_no_reset(id, "burrow-in")
			else
				set_entity_animation_no_reset(id, "burrow-out")
			end
			update_entity_animation(id)
			clip_x, clip_y, clip_w, clip_h = get_clipping_rectangle()
			local x, y = get_clip_position()
			set_clipping_rectangle(x, y, WIDTH, HEIGHT)
		end
	end
end

function post_draw()
	if (digging) then
		if (dig_stage == DIG_START or dig_stage == DIGGING_OUT) then
			set_clipping_rectangle(clip_x, clip_y, clip_w, clip_h)
			local mul
			if (dig_stage == DIG_START) then
				mul = 2
			else
				mul = 1
			end
			local bmp = math.ceil(dig_count / DIG_TIME * mul * 4)
			if (bmp < 1) then
				bmp = 1
			elseif (bmp > 4) then
				bmp = 4
			end
			local x, y = get_clip_position()
			x = x + WIDTH/2
			y = y + HEIGHT
			local w, h = get_bitmap_size(hole_bmps[bmp])
			draw_bitmap(hole_bmps[bmp], x-w/2, y-h/2, 0)
		elseif (dig_stage == DIG_WAIT) then
			local x, y = get_clip_position()
			x = x + WIDTH/2
			y = y + HEIGHT
			local w, h = get_bitmap_size(hole_bmps[4])
			draw_bitmap(hole_bmps[4], x-w/2, y-h/2, 0)
		end
	end
end

function die()
	if (rand(3) == 0) then
		local coin = add_battle_enemy("coin0")
		local x, y = get_entity_position(id)
		set_entity_position(coin, x, y-5)
	end
end


TILE_SIZE = 16
CHUNK_SIZE = 128

ALLEGRO_FLIP_HORIZONTAL = 0x01
ALLEGRO_FLIP_VERTICAL   = 0x02

LOGIC_RATE = 60
LOGIC_MILLIS = math.floor(1000/60)
		
HIT_SIDE = 0
HIT_UP = 1
HIT_DOWN = 2

PARTICLE_HURT_ENEMY = 1
PARTICLE_HURT_PLAYER = 2
PARTICLE_HURT_BOTH = 3
PARTICLE_HURT_NONE = 4
PARTICLE_HURT_EGBERT = 5

ELEMENT_NONE = 0
ELEMENT_FIRE = 1
ELEMENT_ICE = 2
ELEMENT_LIGHTNING = 3

WEAPON = 1
ARMOR = 2
ACCESSORY = 3

SAVE_PLAYER = 1
SAVE_CHOPPABLE = 2
SAVE_ENEMY = 3

BOTTOM_SPRITE_PADDING = 16

BATTLE_EVENT_SIGHTED = 0
BATTLE_EVENT_TRIPPED = 1
BATTLE_EVENT_SLIPPED = 2

SPEECH_LOC_TOP = 0
SPEECH_LOC_BOTTOM = 1
SPEECH_LOC_ANY = 2
SPEECH_LOC_FORCE_TOP = 3
SPEECH_LOC_FORCE_BOTTOM = 4

direction_suffixs = {
	"n",
	"ne",
	"e",
	"se",
	"s",
	"sw",
	"w",
	"nw"
}

function add_jump_emoticon(t)
	if (get_milestone_complete(t.milestone)) then
		return -1
	end

	local x, y = get_entity_position(t.entity)

	local id = add_solo_emoticon(
		t.layer,
		x-32,
		y-74,
		"jump",
		-1
	)

	return id
end

function search(val, array)
	local i
	for i=1,#array do
		if (array[i] == val) then
			return i
		end
	end

	return -1
end

function colliding(id1, id2, obj1, obj2)
	if ((id1 == obj1 and id2 == obj2) or (id1 == obj2 and id2 == obj1)) then
		return true
	end
	return false
end

local _tween_id = 0
local _tweens = {}

function new_tween(tween)
	local id = _tween_id
	_tween_id = _tween_id + 1
	tween.id = id
	_tweens[id] = tween

	add_tween(id)
end

function run_tween(id, adjustment)
	local tween = _tweens[id]

	if (tween._started) then
		tween._start_time = tween._start_time + adjustment
	else
		tween._started = true
		tween._start_time = get_time()
	end

	if (tween == nil) then
		return true
	end
	local ret
	if (tween.done) then
		ret = true
	else
		ret = tween:run(id, get_time() - tween._start_time)
	end
	if (ret == true) then
		tween._started = false
		if (not (tween.next_tween == nil)) then
			_tweens[id] = tween.next_tween
			return false
		end
	end
	return ret
end

function delete_tween(id)
	_tweens[id] = nil
end

function append_tween(t, tween)
	local _t = t
	while (not (_t.next_tween == nil)) do
		_t = _t.next_tween
	end
	_t.next_tween = tween
end

-- if it returns false, erase it from c
local function jump_tween(tween, id, elapsed)
	local pos = elapsed / tween.time

	if (not tween.started) then
		play_sample("sfx/single_jump.ogg", 1, 0, 1)
		if (tween.start_x == nil) then
			tween.start_x = get_entity_position(tween.entity)
		end
		if (tween.start_y == nil) then
			local tmp
			tmp, tween.start_y = get_entity_position(tween.entity)
		end
		tween.x_radius = math.abs((tween.end_x - tween.start_x) / 2)
		tween.y_radius = math.abs(tween.height)
		if (tween.end_y > tween.start_y) then
			tween.drop = tween.height - (tween.end_y - tween.start_y)
		else
			tween.drop = tween.height - (tween.start_y - tween.end_y)
		end
		tween.ratio = 1.0 - (tween.drop / (tween.height+tween.drop))
		tween.center_x = tween.start_x + (tween.end_x - tween.start_x) / 2
		tween.center_y = math.max(tween.start_y, tween.end_y)

		if (tween.entity == get_player_id(0)) then
			set_entity_input_disabled(tween.entity, true)
		end
		set_entity_right(tween.entity, tween.right)
		set_entity_animation(tween.entity, "jump")
		tween.num_frames = get_entity_animation_num_frames(tween.entity)
		tween.shadow_was_shown = get_entity_shadow_is_shown(tween.entity)
		set_show_entity_shadow(tween.entity, false)
		tween.started = true
	end

	if (pos >= 1) then
		if (tween.entity == get_player_id(0)) then
			set_entity_input_disabled(tween.entity, false)
		end
		set_entity_layer(tween.entity, tween.end_layer)
		set_show_entity_shadow(tween.entity, tween.shadow_was_shown)
		reset_entity_animation(tween.entity)
		set_entity_animation(tween.entity, "idle")
		tween.started = false
		tween.finished = true
		set_entity_position(tween.entity, tween.end_x, tween.end_y)
		center_entity_cameras(tween.entity)
		dec_num_jumping()
		return true
	else
		set_entity_animation_frame(tween.entity, pos * tween.num_frames)
	end

	if (tween.end_y > tween.start_y) then
		pos = 1.0 - pos
	end

	local percent
	local y_scale
	local x_inc
	local y_inc
	local angle_mul
	local x_mul = 1

	if (pos < tween.ratio) then
		y_scale = 1
		y_inc = 0
		percent = pos / tween.ratio
		x_inc = percent * (((tween.x_radius*2) * tween.ratio) - tween.x_radius)
		angle_mul = 1
	else
		y_scale = (1.0 - tween.ratio) / tween.ratio
		y_inc = -(tween.height-tween.drop)
		full_xinc = ((tween.x_radius*2) * tween.ratio) - tween.x_radius
		percent = (pos - tween.ratio) / (1.0 - tween.ratio)
		x_inc = full_xinc + percent * (((tween.x_radius * 2) * (1.0 - tween.ratio)) - tween.x_radius)
		angle_mul = 1.5
	end

	if (tween.start_x < tween.end_x) then
		if (tween.end_y > tween.start_y) then
			x_mul = -1
		end
	else
		if (tween.end_y <= tween.start_y) then
			x_mul = -1
		end
	end

	if (tween.updown) then
		if (pos < 0.5) then
			y_inc = y_inc + -(pos / 0.5) * tween.height
		else
			y_inc = y_inc + -(1.0 - (pos - 0.5) / 0.5) * tween.height
		end
	end

	local x = tween.center_x + x_mul * (x_inc + tween.x_radius * math.cos(math.pi*angle_mul + percent*(math.pi/2)))
	local y = tween.center_y + y_inc + (tween.y_radius * y_scale) * math.sin(math.pi*angle_mul + percent*(math.pi/2))

	set_entity_position(tween.entity, x, y)

	return false
end

--[[
table_data needs:
entity -- entity id
time -- in seconds
height
start_x --opt
start_y --opt
end_x
end_y
end_layer
right
]]--
function create_jump_tween(table_data)
	inc_num_jumping()
	table_data.run = jump_tween
	table_data.started = false
	return table_data
end

local function character_role_change_tween(tween, id, elapsed)
	set_character_role(tween.entity, tween.role, table.unpack(tween.role_parameters))
	return true
end

--[[
entity, role -- "wander", "astar", "none", ...
]]--
function create_character_role_change_tween(eid, role)
	table_data = {}
	table_data.entity = eid
	table_data.role = role
	table_data.run = character_role_change_tween
	table_data.role_parameters = {}
	return table_data
end

local function gesture_tween(tween, id, elapsed)
	set_entity_animation(tween.entity_id, tween.gesture)
	return true
end

--[[
eid - entity id
g - gesture name (string)
]]--
function create_gesture_tween(eid, g)
	return { run = gesture_tween, entity_id = eid, gesture = g }
end

local function change_direction_tween(tween, id, elapsed)
	set_entity_direction(tween.entity_id, tween.direction)
	return true
end

--[[
eid - entity id
dir - direction
]]--
function create_change_direction_tween(eid, dir)
	return { run = change_direction_tween, entity_id = eid, direction = dir }
end

local function idle_tween(tween, id, elapsed)
	if (elapsed >= tween.time) then
		return true
	end
	return false
end

function create_idle_tween(duration)
	return { run = idle_tween, time = duration }
end

local function reset_camera_tween(tween, id, elapsed)
	if (elapsed < tween.start_delay) then
		return false
	end

	if (not tween.started) then
		tween.started = true
		tween.last_update = elapsed
	end

	local time = elapsed - tween.last_update
	tween.last_update = elapsed

	local ox, oy = get_camera_offset()

	if (math.sqrt(ox*ox + oy*oy) < 3) then
		set_camera_offset(
			0, 0
		)
		return true
	end

	local angle = math.atan2(-oy, -ox)
	local movex = math.cos(angle) * (tween.move_speed * time)
	local movey = math.sin(angle) * (tween.move_speed * time)
	set_camera_offset(ox+movex, oy+movey)
	
	return false
end

function create_reset_camera_tween(start, speed)
	return { run = reset_camera_tween, start_delay = start, move_speed = speed, started = false }
end

local function center_camera_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
		tween.start_x, tween.start_y = get_camera_offset()
		local top_x, top_y = get_area_top()
		local scr_w, scr_h = get_screen_size()
		tween.center_x = (tween.cx - scr_w/2) - top_x
		tween.center_y = (tween.cy - scr_h/2) - top_y
		tween.player_start_x, tween.player_start_y = get_entity_position(0)
	end

	if (elapsed < tween.start_delay) then
		return false
	end

	local x, y = get_entity_position(0)
	local x2 = x - tween.player_start_x
	local y2 = y - tween.player_start_y
	tween.player_start_x = x
	tween.player_start_y = y

	local time = elapsed - tween.start_delay

	local dx = tween.center_x - tween.start_x
	local dy = tween.center_y - tween.start_y

	local angle = math.atan2(dy, dx)
	local ox = tween.start_x + math.cos(angle) * (tween.move_speed * time) - x2
	local oy = tween.start_y + math.sin(angle) * (tween.move_speed * time) - y2

	set_camera_offset(ox, oy)

	dx = tween.center_x - ox
	dy = tween.center_y - oy

	if (math.sqrt(dx*dx + dy*dy) <= (tween.move_speed*LOGIC_MILLIS/1000+0.1)) then
		return true
	end
	
	return false
end

function create_center_camera_tween(start, cx, cy, speed)
	return { run = center_camera_tween, start_delay = start, move_speed = speed, cx = cx, cy = cy }
end

local function astar_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
		set_character_destination(tween.entity_id, tween.x, tween.y, tween.running)
	end
	if (character_is_following_path(tween.entity_id)) then
		return false
	end
	tween.started = false
	return true
end

function create_astar_tween(eid, x, y, running)
	return { run = astar_tween, entity_id = eid, x = x, y = y, running = running, started = false }
end

local function direct_move_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
		tween.start_x, tween.start_y = get_entity_position(tween.entity_id)
		local dx = tween.x - tween.start_x
		local dy = tween.y - tween.start_y
		tween.distance = math.sqrt(dx*dx + dy*dy)
	end

	local pixels_to_move = tween.speed * elapsed
	if (pixels_to_move >= tween.distance) then
		set_entity_position(tween.entity_id, tween.x, tween.y)
		return true
	else
		local dx = tween.x - tween.start_x
		local dy = tween.y - tween.start_y
		local angle = math.atan2(dy, dx)
		local x = tween.start_x
		local y = tween.start_y
		x = x + math.cos(angle) * pixels_to_move
		y = y + math.sin(angle) * pixels_to_move
		set_entity_position(tween.entity_id, x, y)
		return false
	end
end

function create_direct_move_tween(eid, x, y, pixels_per_second)
	return {
		run = direct_move_tween,
		entity_id = eid,
		x = x,
		y = y,
		speed = pixels_per_second,
		started = false
	}
end

local function toss_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
	end

	local percent = elapsed / tween.time
	if (percent >= 1.0) then
		set_entity_position(tween.entity_id, tween.dx, tween.dy)
		return true
	else
		local x, y = calculate_toss(
			tween.sx,
			tween.sy,
			tween.dx,
			tween.dy,
			percent
		)
		set_entity_position(tween.entity_id, x, y)
		return false
	end
end

function create_toss_tween(eid, sx, sy, dx, dy, time)
	local t = {}
	t.run = toss_tween
	t.entity_id = eid
	t.sx = sx
	t.sy = sy
	t.dx = dx
	t.dy = dy
	t.time = time
	t.started = false
	return t
end

function toss_banana(layer, sx, sy, angle)
	local banana = add_entity("banana", layer, sx, sy)
	set_entity_solid_with_area(banana, true)
	set_entity_solid_with_entities(banana, false)
	-- use 64 pixel distance
	local dx = math.cos(angle) * 64 + sx
	local dy = math.sin(angle) * 64 + sy
	local t = create_toss_tween(banana, sx, sy, dx, dy, 1.0)
	new_tween(t)
end

local function play_animation_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
		set_entity_animation(tween.entity_id, tween.anim_name)
	end

	if (entity_animation_is_finished(tween.entity_id)) then
		return true
	else
		return false
	end
end

function create_play_animation_tween(eid, name)
	return { run = play_animation_tween, entity_id = eid, anim_name = name, started = false }
end

local function play_sample_tween(tween, id, elapsed)
	play_sample(tween.sample_name, 1, 0, 1)
	return true
end

function create_play_sample_tween(sample_name)
	return {
		run = play_sample_tween,
		sample_name = sample_name
	}
end

local function center_view_tween(tween, id, elapsed)
	local x, y = get_camera_offset()

	if (x > 0) then
		x = x - 1
		if (x < 0) then
			x = 0
		end
	elseif (x < 0) then
		x = x + 1
		if (x > 0) then
			x = 0
		end
	end

	if (y > 0) then
		y = y - 1
		if (y < 0) then
			y = 0
		end
	elseif (y < 0) then
		y = y + 1
		if (y > 0) then
			y = 0
		end
	end

	set_camera_offset(x, y)

	if (x == 0 and y == 0) then
		return true
	else
		return false
	end
end

function create_center_view_tween()
	return { run = center_view_tween }
end

function row_tween(tween, id, elapsed)
	local dx = tween.dx - tween.sx
	local dy = tween.dy - tween.sy
	local angle = math.atan2(dy, dx)
	local dist = math.sqrt(dx * dx + dy * dy)

	if (dist <= tween.velocity) then
		return true
	end

	tween.sx = tween.sx + math.cos(angle) * tween.velocity
	tween.sy = tween.sy + math.sin(angle) * tween.velocity

	set_entity_position(tween.entity, tween.sx, tween.sy)

	tween.velocity = tween.velocity - 0.0002
	if (tween.velocity < 0.0) then
		tween.velocity = 0.0
	end

	local frame = math.floor((elapsed / 0.1) % 7)
	if (not (tween.current_frame == frame)) then
		if (frame == 6) then
			play_sample("sfx/rowing.ogg", 1, 0, 1)
			tween.velocity = tween.velocity + 0.1
			if (tween.velocity > 1.0) then
				tween.velocity = 1.0
			end
		end
		tween.current_frame = frame
	end

	if (not tween.trail_added) then
		if (elapsed >= tween.trail_delay) then
			tween.trail = add_entity("water_trail", get_entity_layer(tween.entity), tween.sx + tween.trail_ox, tween.sy + tween.trail_oy)
			set_entity_right(tween.trail, tween.right)
			tween.trail_added = true
		end
	else
		set_entity_position(tween.trail, tween.sx + tween.trail_ox, tween.sy + tween.trail_oy)
	end

	return false
end

function create_row_tween(entity, sx, sy, dx, dy, trail_delay, trail_ox, trail_oy)
	local t = {
		run = row_tween,
		entity = entity,
		sx = sx,
		sy = sy,
		dx = dx,
		dy = dy,
		trail_delay = trail_delay,
		trail_added = false,
		trail_ox = trail_ox,
		trail_oy = trail_oy,
		velocity = 0.0,
		current_frame = 0,
		right = dx > sx
	}
	return t
end

function slowing_row_tween(tween, id, elapsed)
	local dx = tween.dx - tween.sx
	local dy = tween.dy - tween.sy
	local angle = math.atan2(dy, dx)
	local dist = math.sqrt(dx * dx + dy * dy)

	if (dist <= tween.velocity) then
		remove_entity(tween.trail)
		return true
	end

	tween.sx = tween.sx + math.cos(angle) * tween.velocity
	tween.sy = tween.sy + math.sin(angle) * tween.velocity

	set_entity_position(tween.entity, tween.sx, tween.sy)

	local sub = 0.0002
	tween.velocity = tween.velocity - sub
	if (tween.velocity < 0.0) then
		tween.velocity = 0.0
	end

	local frame = math.floor((elapsed / 0.1) % 7)
	if (not (tween.current_frame == frame)) then
		if (frame == 6) then
			play_sample("sfx/rowing.ogg", 1, 0, 1)
			tween.velocity = tween.velocity + 0.1
			if (tween.velocity > 1.0) then
				tween.velocity = 1.0
			end
		end
		tween.current_frame = frame
	end
	
	if (dist < 100) then
		local mul = 1.0 - ((100 - dist) / 100 * 0.025)
		tween.velocity = tween.velocity * mul
		if (not tween.trail_removed) then
			remove_entity(tween.trail)
			tween.trail_removed = true
		end
	end

	if (not tween.trail_removed) then
		set_entity_position(tween.trail, tween.sx + tween.trail_ox, tween.sy + tween.trail_oy)
	end

	return false
end

function create_slowing_row_tween(entity, sx, sy, dx, dy, trail_ox, trail_oy)
	local fulldist = math.sqrt((dx-sx)*(dx-sx) + (dy-sy)*(dy-sy))
	local t = {
		run = slowing_row_tween,
		entity = entity,
		sx = sx,
		sy = sy,
		dx = dx,
		dy = dy,
		trail_ox = trail_ox,
		trail_oy = trail_oy,
		velocity = 1.0,
		current_frame = 0,
		fulldist = fulldist,
	}
	t.trail = add_entity("water_trail", get_entity_layer(entity), sx + trail_ox, sy + trail_oy)
	set_entity_right(t.trail, dx > sx)
	return t
end

local function direct_move_xyz_tween(tween, id, elapsed)
	if (not tween.started) then
		tween.started = true
		local sx, sy = get_entity_position(tween.entity_id)
		local sz = get_entity_z(tween.entity_id)
		tween.sx = sx
		tween.sy = sy
		tween.sz = sz
	end

	local p = elapsed / tween.time
	local ret
	if (p > 1) then
		p = 1
		ret = true
		tween.started = false
	else
		ret = false
	end
	
	local x = (tween.x - tween.sx) * p + tween.sx
	local y = (tween.y - tween.sy) * p + tween.sy
	local z = (tween.z - tween.sz) * p + tween.sz

	set_entity_position(tween.entity_id, x, y)
	set_entity_z(tween.entity_id, z)

	return ret
end

function create_direct_move_xyz_tween(eid, time, x, y, z)
	return { run = direct_move_xyz_tween, entity_id = eid, x = x, y = y, z = z, time = time, started = false }
end

function update_animation(anim)
	anim.count = anim.count + LOGIC_MILLIS
	if (anim.count >= anim.delays[anim.curr_frame]) then
		anim.count = anim.count - anim.delays[anim.curr_frame]
		if (anim.curr_frame >= anim.num_frames) then
			anim.curr_frame = 1
		else
			anim.curr_frame = anim.curr_frame + 1
		end
	end
end

function draw_animation(anim)
	if (anim.frames[anim.curr_frame] == nil) then
		return
	end
	local topx, topy = get_area_top()
	draw_script_bitmap(
		anim.frames[anim.curr_frame],
		anim.x-topx,
		anim.y-topy,
		0
	)
end

function make_animation(num_frames, frames, delays, x, y)
	local anim = {}
	anim.frames = frames
	anim.delays = delays
	anim.num_frames = num_frames
	anim.curr_frame = 1
	anim.count = 0
	anim.x = x
	anim.y = y
	return anim
end

function simple_speak(speech)
	local data = {}
	local count = 1
	for i=2,#speech,3 do
		if (speech[i+2] < 0) then
			data[count] = t(speech[i])
		else
			data[count] = "<b><color 255 255 255>" .. t(string.upper(get_entity_name(speech[i+2]))) .. ":" .. "</color></b>" .. " " .. t(speech[i])
		end
		count = count + 1
		data[count] = speech[i+1]
		count = count + 1
		data[count] = speech[i+2]
		count = count + 1
	end
	speak(false, false, speech[1], table.unpack(data))
end

function speak_force_top(ignore1, ignore2, ignore3, text, gesture, id)
	if (id >= 0) then
		text = "<b><color 255 255 255>" .. t(string.upper(get_entity_name(id))) .. ":" .. "</color></b>" .. " " .. text
	end
	speak_force_t(false, false, true, text, gesture, id)
end

function speak_force_bottom(ignore1, ignore2, ignore3, text, gesture, id)
	if (id >= 0) then
		text = "<b><color 255 255 255>" .. t(string.upper(get_entity_name(id))) .. ":" .. "</color></b>" .. " " .. text
	end
	speak_force_b(false, false, true, text, gesture, id)
end

function add_bomb_puff(l, x, y, radius) -- l is draw layer
	local in_layer = 1
	local layer = 0

	local count = in_layer
	local npuffs = 0
	local r = 0
	while (r < radius) do
		npuffs = npuffs + count
		r = r + 10
		local circumference = math.pi * r * 2
		count = circumference / 20 
	end
	npuffs = npuffs + count
	count = 0

	local pgid = add_particle_group("bomb_puff", l, PARTICLE_HURT_NONE, "bomb_puff")
	local i
	local a2 = rand(1000)/1000 * math.pi * 2
	for i=1,npuffs do
		local pid = add_particle(
			pgid,
			24, 24,
			1, 1, 1, 1,
			0,
			0,
			false,
			false
		)
		set_particle_position(pid, x, y)

		set_particle_blackboard(pid, 0, x) -- start pos
		set_particle_blackboard(pid, 1, y) -- start pos

		local r = layer * 10
		count = count + 1
		if (count >= in_layer) then
			count = 0
			local circumference = math.pi * r * 2
			in_layer = circumference / 20 
			layer = layer + 1
		end

		set_particle_blackboard(pid, 2, r)

		local angle = a2 + (count-1) / in_layer * math.pi * 2
		local dx = math.cos(angle) * (r / 0.25 / LOGIC_RATE) -- 0.25 seconds to reach dest
		local dy = math.sin(angle) * (r / 0.25 / LOGIC_RATE)
		set_particle_blackboard(pid, 3, dx) -- velocity
		set_particle_blackboard(pid, 4, dy) -- velocity

		local lifetime = 2.5 -- 0.25 seconds in, wait 1.5 seconds, 0.75 seconds out
		set_particle_blackboard(pid, 5, lifetime) -- time before death
		set_particle_blackboard(pid, 6, 0) -- time count

		set_particle_scale(pid, 0, 0)
	end
end

function add_video_sfx(video_player, callback, prev_frame)
	local frame = get_video_frame_num(video_player)
	for i=prev_frame,frame,1 do
		callback(i)
	end
	return frame
end

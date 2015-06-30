local frame = 1
local count = 0

local bmps = {}
local bmp_w
local bmp_h

function start(this_id)
	id = this_id
	set_battle_entity_unhittable(id, true)

	for i=1,10 do
		bmps[i] = load_bitmap("battle/misc_graphics/plant/" .. i .. ".png")
	end
	bmp_w, bmp_h = get_bitmap_size(bmps[1])
end

function get_attack_sound()
	return ""
end

function decide()
	return "nil"
end

function logic()
	count = count + 1
	if (frame == 1) then
		if (count >= 60) then
			play_sample("sfx/plant_pop_up.ogg", 1, 0, 1)
			frame = frame + 1
			count = 0
		end
	elseif (frame == 10) then
		dead = true
		for i=1,10 do
			destroy_bitmap("battle/misc_graphics/plant/" .. i .. ".png")
		end
		remove_entity(id)
	else
		if (count >= 8) then
			if (frame == 9) then
				play_sample("sfx/plant_fire.ogg", 1, 0, 1)
				play_sample("sfx/plant_pop_down.ogg", 1, 0, 1)

				local x, y = get_entity_position(id)
				y = y - 24 
				local pgid = add_particle_group("petal", 0, PARTICLE_HURT_ENEMY, "petal1", "petal2", "petal3")
				local petal1 = add_particle(
					pgid,
					4, 4,
					1, 1, 1, 1,
					0,
					HIT_DOWN,
					true,
					false
				)
				set_particle_blackboard(petal1, 0, 0)
				set_particle_blackboard(petal1, 1, x)
				set_particle_blackboard(petal1, 2, y)
				set_particle_blackboard(petal1, 3, false)
				set_particle_blackboard(petal1, 4, x)
				set_particle_blackboard(petal1, 5, y)
				set_particle_blackboard(petal1, 6, math.pi+math.pi/4)
				set_particle_blackboard(petal1, 7, math.pi/2)
				set_particle_blackboard(petal1, 8, math.pi/60)

				local petal2 = add_particle(
					pgid,
					4, 4,
					1, 1, 1, 1,
					0,
					HIT_DOWN,
					true,
					false
				)
				set_particle_blackboard(petal2, 0, 0)
				set_particle_blackboard(petal2, 1, x)
				set_particle_blackboard(petal2, 2, y)
				set_particle_blackboard(petal2, 3, false)
				set_particle_blackboard(petal2, 4, x)
				set_particle_blackboard(petal2, 5, y)
				set_particle_blackboard(petal2, 6, math.pi+math.pi/8)
				set_particle_blackboard(petal2, 7, math.pi/2)
				set_particle_blackboard(petal2, 8, math.pi/60)

				local petal3 = add_particle(
					pgid,
					4, 4,
					1, 1, 1, 1,
					0,
					HIT_DOWN,
					false,
					false
				)
				set_particle_blackboard(petal3, 0, 0)
				set_particle_blackboard(petal3, 1, x)
				set_particle_blackboard(petal3, 2, y)
				set_particle_blackboard(petal3, 3, false)
				set_particle_blackboard(petal3, 4, x)
				set_particle_blackboard(petal3, 5, y)
				set_particle_blackboard(petal3, 6, -math.pi/4)
				set_particle_blackboard(petal3, 7, math.pi/2)
				set_particle_blackboard(petal3, 8, math.pi/60)

				local petal4 = add_particle(
					pgid,
					4, 4,
					1, 1, 1, 1,
					0,
					HIT_DOWN,
					false,
					false
				)
				set_particle_blackboard(petal4, 0, 0)
				set_particle_blackboard(petal4, 1, x)
				set_particle_blackboard(petal4, 2, y)
				set_particle_blackboard(petal4, 3, false)
				set_particle_blackboard(petal4, 4, x)
				set_particle_blackboard(petal4, 5, y)
				set_particle_blackboard(petal4, 6, -math.pi/8)
				set_particle_blackboard(petal4, 7, math.pi/2)
				set_particle_blackboard(petal4, 8, math.pi/60)
			end

			frame = frame + 1
			count = 0
		end
	end
end

function collide(with, me)
end

function get_should_auto_attack()
	return false
end

function post_draw()
	if (dead) then
		return
	end
	local top_x, top_y = get_battle_top()
	local x, y = get_entity_position(id)
	draw_bitmap(bmps[frame], (x-bmp_w/2)-top_x, (y-bmp_h)-top_y+BOTTOM_SPRITE_PADDING, 0)
end

function stop()
end

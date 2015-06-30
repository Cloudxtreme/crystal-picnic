function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	remove_particle(pid)
end

local GOING_UP = 0
local SWAYING = 1
local SITTING = 2

function logic(pid)
	local stage = get_particle_blackboard(pid, 0)
	local center_x = get_particle_blackboard(pid, 1)
	local center_y = get_particle_blackboard(pid, 2)
	local right = get_particle_blackboard(pid, 3)
	local start_x = get_particle_blackboard(pid, 4)
	local start_y = get_particle_blackboard(pid, 5)
	local up_angle = get_particle_blackboard(pid, 6)
	local sway_angle = get_particle_blackboard(pid, 7)
	local sway_delta = get_particle_blackboard(pid, 8)

	local x, y = get_particle_position(pid)

	if (stage == GOING_UP) then
		center_x = center_x + math.cos(up_angle) * 2.0
		center_y = center_y + math.sin(up_angle) * 2.0
		local dx = center_x - start_x
		local dy = center_y - start_y
		local dist = math.sqrt(dx*dx + dy*dy)
		if (dist >= 100) then
			stage = SWAYING
		end
	elseif (stage == SWAYING) then
		center_y = center_y + 0.5
		sway_angle = sway_angle + sway_delta
		if (sway_delta < 0) then
			if (sway_angle < (math.pi/180.0*80.0)) then
				sway_delta = -sway_delta
			end
		else
			if (sway_angle > math.pi) then
				sway_delta = -sway_delta
			end
		end
		local diff = (math.pi - (math.pi/180.0*80.0)) / 3
		local index
		if (sway_angle > math.pi-diff) then
			index = 0
		elseif (sway_angle > math.pi-diff*2) then
			index = 1
		else
			index = 2
		end
		set_particle_bitmap_index(pid, index)
	elseif (stage == SITTING) then
		count = count + 1
		if (count >= 5*60) then
			remove_particle(pid)
			return
		end
	end

	local radius = 16

	local angle
	if (right) then
		angle = math.pi - sway_angle
	else
		angle = sway_angle
	end

	x = center_x + math.cos(angle) * radius
	y = center_y + math.sin(angle) * radius

	if (x < 0 or x >= get_battle_width()) then
		remove_particle(pid)
		return
	end

	if (stage == SWAYING) then
		if (particle_is_colliding_with_area(pid)) then
			stage = SITTING
			count = 0
		end
	end

	set_particle_position(pid, x, y)
	
	set_particle_blackboard(pid, 0, stage)
	set_particle_blackboard(pid, 1, center_x)
	set_particle_blackboard(pid, 2, center_y)
	set_particle_blackboard(pid, 7, sway_angle)
	set_particle_blackboard(pid, 8, sway_delta)
end

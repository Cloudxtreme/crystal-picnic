function start(this_pgid)
	pgid = this_pgid
end

function stop()
end

function collide(pid, entity_id)
	for i=0,2 do
		local id = get_player_id(i)
		local name = get_entity_name(id)
		if (name == "egbert") then
			set_player_lost_weapon(id, false)
		end
	end
	remove_particle(pid)
end

function logic(pid)
	for i=0,2 do
		local id = get_player_id(i)
		local name = get_entity_name(id)
		if (name == "egbert") then
			if (get_hp(id) <= 0) then
				remove_particle(pid)
			end
		end
	end
end

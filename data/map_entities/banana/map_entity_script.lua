function start(this_id)
	id = this_id
end

function logic()
	local layer = get_entity_layer(id)
	local x, y = get_entity_position(id)
	if (not point_in_area_bounds(layer, x, y)) then
		return "remove"
	end
	return ""
end

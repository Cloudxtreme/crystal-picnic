function start(userdata)
	add_map_location(userdata, "CRYSTAL_CASTLE_ENTRANCE", 192, 322)
	add_map_location(userdata, "RIVER_TOWN", 162, 337)
	if (milestone_is_complete("exited_river_town_south")) then
		add_map_location(userdata, "INTERSECTION", 153, 383)
	end
	if (milestone_is_complete("entered_old_forest")) then
		add_map_location(userdata, "OLD_FOREST_ENTRANCE", 362, 440)
		add_map_location(userdata, "OLD_FOREST", 377, 491)
	end
	if (milestone_is_complete("old_oak_battle")) then
		add_map_location(userdata, "STONE_CRATER", 502, 392)
	end
	if (milestone_is_complete("STONECRATER_BIGHOLE")) then
		add_map_location(userdata, "CAVERNS", 537, 358)
	end
	if (milestone_is_complete("beat_antboss")) then
		add_map_location(userdata, "JUNGLE_VALLEY", 571, 357)
	end

	set_map_neighbors(userdata, "CRYSTAL_CASTLE_ENTRANCE", "RIVER_TOWN", "", "", "")
	set_map_neighbors(userdata, "RIVER_TOWN", "", "CRYSTAL_CASTLE_ENTRANCE", "", "INTERSECTION")
	if (milestone_is_complete("exited_river_town_south")) then
		set_map_neighbors(userdata, "INTERSECTION", "", "", "RIVER_TOWN", "")
	end
	if (milestone_is_complete("entered_old_forest")) then
		set_map_neighbors(userdata, "INTERSECTION", "", "OLD_FOREST_ENTRANCE", "RIVER_TOWN", "")
		set_map_neighbors(userdata, "OLD_FOREST_ENTRANCE", "INTERSECTION", "", "", "OLD_FOREST")
		set_map_neighbors(userdata, "OLD_FOREST", "", "", "OLD_FOREST_ENTRANCE", "")
	end
	if (milestone_is_complete("old_oak_battle")) then
		set_map_neighbors(userdata, "OLD_FOREST", "", "STONE_CRATER", "OLD_FOREST_ENTRANCE", "")
		set_map_neighbors(userdata, "STONE_CRATER", "OLD_FOREST", "", "", "")
	end
	if (milestone_is_complete("STONECRATER_BIGHOLE")) then
		set_map_neighbors(userdata, "STONE_CRATER", "OLD_FOREST", "", "CAVERNS", "")
		set_map_neighbors(userdata, "CAVERNS", "", "", "", "STONE_CRATER")
	end
	if (milestone_is_complete("beat_antboss")) then
		set_map_neighbors(userdata, "CAVERNS", "", "JUNGLE_VALLEY", "", "STONE_CRATER")
		set_map_neighbors(userdata, "JUNGLE_VALLEY", "CAVERNS", "", "", "")
	end
end

-- FIXME: use last_area
-- returns area name, x, y, layer
function get_dest(name, last_area)
	if (name == "CRYSTAL_CASTLE_ENTRANCE") then
		return "heading_west", 3, 66, 97
	elseif (name == "RIVER_TOWN") then
		return "river_town", 2, 1226, 279
	elseif (name == "INTERSECTION") then
		return "meeting", 2, 472, 185
	elseif (name == "OLD_FOREST_ENTRANCE") then
		set_milestone_complete("CANOEING", false)
		return "oldforestdock", 2, 312, 167
	elseif (name == "OLD_FOREST") then
		return "oldforest1", 2, 55, 186
	elseif (name == "STONE_CRATER") then
		return "stonecrater1", 2, 269, 513
	elseif (name == "CAVERNS") then
		return "caverns1", 2, 8.5*TILE_SIZE, 12*TILE_SIZE
	-- FIXME: jungle valley
	else
		return "", -1, -1,  -1
	end
end

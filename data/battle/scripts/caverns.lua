function start(level_name)
	add_parallax_bitmap("caverns.png", false)
	
	if (level_name == "caverns1") then
		add_battle_data(
			"stalactite",
			24,
			45, 93, 28, 36, 60, 36, -- tip, left, right
			113, 111, 94, 54, 126, 56,
			173, 94, 159, 40, 190, 40,
			364, 55, 352, 2, 381, 3
		)
	elseif (level_name == "caverns2") then
		add_battle_data(
			"stalactite",
			24,
			40, 52, 24, 1, 52, 2,
			144, 37, 133, 2, 152, 2,
			250, 28, 246, 2, 259, 1,
			418, 39, 410, 1, 430, 2
		)
	elseif (level_name == "caverns3") then
		add_battle_data(
			"stalactite",
			30,
			78, 52, 66, 1, 95, 1,
			170, 80, 151, 25, 183, 24,
			253, 54, 236, 2, 266, 2,
			308, 87, 296, 32, 328, 30,
			379, 36, 372, 2, 391, 1
		)
	end
end


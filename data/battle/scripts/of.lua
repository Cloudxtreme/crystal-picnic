function start(level_name)
	add_parallax_bitmap("of_back.png", false)
	add_parallax_bitmap("of_mid.png", false)
	add_parallax_bitmap("of_fore.png", true)

	if (level_name == "of1") then
		add_battle_data(
			"faff",
			36,
			128, 440, 128, 397, 0, 1,
			128, 397, 128, 440, 1, -1,
			--

			256, 440, 256, 397, 0, -1,
			256, 397, 256, 440, 0, -1,
			--
			360, 440, 360, 397, 0, 0,
			360, 397, 360, 440, 1, -1
		)
	elseif (level_name == "of2") then
		add_battle_data("faff", 0)
	elseif (level_name == "of3") then
		add_battle_data(
			"faff",
			48,
			58, 428, 58, 397, 0, 1,
			58, 397, 58, 428, 1, -1,
			--
			100, 428, 100, 397, 0, 0,
			100, 397, 100, 428, 1, -1,
			--
			295, 428, 295, 397, 0, 1,
			295, 397, 295, 428, 1, -1,
			--
			340, 428, 340, 397, 0, 0,
			340, 397, 340, 428, 1, -1
		)
	end
end


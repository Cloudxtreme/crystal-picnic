function start(level_name)
	add_parallax_bitmap("stonecrater-back.png", false)
	add_parallax_bitmap("stonecrater-middle.png", false)
	add_parallax_bitmap("stonecrater-fore.png", true)
end

function slow_armadillo_tween(tween, id, elapsed)
	set_can_accelerate_quickly(tween.armadillo, false);
	return true
end

function slow_armadillo(id)
	local t = create_idle_tween(0.15)
	append_tween(t, { run = slow_armadillo_tween, armadillo = id })
	new_tween(t)
end

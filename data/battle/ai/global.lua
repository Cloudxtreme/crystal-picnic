PARTICLE_GOOD = 1
PARTICLE_BAD = 2

function mod_inc(n, elements)
	n = n + 1
	if (n > elements) then
		n = 0
	end
	return n
end

def example001():
	def rotcy(rot, r, h):
		return RotateAxis(90, rot,
			Cylinder(r,h,True))
	def r_from_dia(d):
		return d/2.0

	size = 50
	hole = 25
	cy_r = r_from_dia(hole)
	cy_h = r_from_dia(size * 2.5)

	return Difference([
		Sphere(r_from_dia(size)),
			rotcy([0,0,0], cy_r, cy_h),
			rotcy([1,0,0], cy_r, cy_h),
			rotcy([0,1,0], cy_r, cy_h)
		])

result = example001()

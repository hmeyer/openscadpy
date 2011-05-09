def example001():
	def rotcy(rot, r, h):
		return openscad.RotateAxis(90, rot,
			openscad.Cylinder(r,h,True))
	def r_from_dia(d):
		return d/2.0

	size = 50
	hole = 25
	cy_r = r_from_dia(hole)
	cy_h = r_from_dia(size * 2.5)

	return openscad.Difference([
		openscad.Sphere(r_from_dia(size)),
			rotcy([0,0,0], cy_r, cy_h),
			rotcy([1,0,0], cy_r, cy_h),
			rotcy([0,1,0], cy_r, cy_h)
		])

openscad.result = example001()

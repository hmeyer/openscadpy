def example001():
	def rotcy(rot, rad, height):
		return openscad.rotate(90, rot,
			openscad.cylinder(height,rad,center=True))
	def r_from_dia(d):
		return d/2.0

	size = 50
	hole = 25
	cy_r = r_from_dia(hole)
	cy_h = r_from_dia(size * 2.5)

	return (openscad.sphere(r_from_dia(size)) 
		- rotcy([0,0,0], cy_r, cy_h) 
		- rotcy([1,0,0], cy_r, cy_h) 
		- rotcy([0,1,0], cy_r, cy_h))

openscad.result = example001()

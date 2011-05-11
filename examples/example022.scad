from openscad import *

# size is a vector [w, h, d]
def roundedBox(size, radius, sidesonly):
	rot = [ [0,0,0], [90,0,90], [90,90,0] ]
	res = []
	def r(x):
		res.append(x)
	if sidesonly: 
		r(Cube([size[0]-2*radius, size[1], size[2]], True))
		r(Cube([size[0], size[1]-2*radius, size[2]], True))

		for x in [radius-size[0]/2, -radius+size[0]/2]:
			for y in [radius-size[1]/2, -radius+size[1]/2]:
				r(Translate([x,y,0], Cylinder(radius, size[2], True)))
	else:
		r(Cube([size[0], size[1]-radius*2, size[2]-radius*2], True))
		r(Cube([size[0]-radius*2, size[1], size[2]-radius*2], True))
		r(Cube([size[0]-radius*2, size[1]-radius*2, size[2]], True))

		for axis in range(0,3):
			for x in [radius-size[axis]/2, -radius+size[axis]/2]:
				for y in [radius-size[(axis+1)%3]/2, -radius+size[(axis+1)%3]/2]:
					r(Rotate(rot[axis],
						Translate([x,y,0],
							Cylinder(radius, size[(axis+2)%3]-2*radius, True))))

		for x in [radius-size[0]/2, -radius+size[0]/2]:
			for y in [radius-size[1]/2, -radius+size[1]/2]:
				for z in [radius-size[2]/2, -radius+size[2]/2]:
					r(Translate([x,y,z], Sphere(radius)))

	return res

openscad.result = Union([
	Translate([-15,0,0], roundedBox([20,30,40], 5, True)),
	Translate([15,0,0], roundedBox([20,30,40], 5, False))
	])


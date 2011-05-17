from openscad import *

# size is a vector [w, h, d]
def roundedBox(size, radius, sidesonly):
	rot = [ [0,0,0], [90,0,90], [90,90,0] ]
	res = []
	def r(x):
		res.append(x)
	if sidesonly: 
		r(cube([size[0]-2*radius, size[1], size[2]], True))
		r(cube([size[0], size[1]-2*radius, size[2]], True))

		for x in [radius-size[0]/2, -radius+size[0]/2]:
			for y in [radius-size[1]/2, -radius+size[1]/2]:
				r(translate([x,y,0], cylinder(size[2], r=radius, center=True)))
	else:
		r(cube([size[0], size[1]-radius*2, size[2]-radius*2], True))
		r(cube([size[0]-radius*2, size[1], size[2]-radius*2], True))
		r(cube([size[0]-radius*2, size[1]-radius*2, size[2]], True))

		for axis in range(0,3):
			for x in [radius-size[axis]/2, -radius+size[axis]/2]:
				for y in [radius-size[(axis+1)%3]/2, -radius+size[(axis+1)%3]/2]:
					r(rotate(rot[axis],
						translate([x,y,0],
							cylinder(r=radius, h=size[(axis+2)%3]-2*radius, center=True))))

		for x in [radius-size[0]/2, -radius+size[0]/2]:
			for y in [radius-size[1]/2, -radius+size[1]/2]:
				for z in [radius-size[2]/2, -radius+size[2]/2]:
					r(translate([x,y,z], sphere(radius)))

	return res

openscad.result = union([
	translate([-15,0,0], roundedBox([20,30,40], 5, True)),
	translate([15,0,0], roundedBox([20,30,40], 5, False))
	])


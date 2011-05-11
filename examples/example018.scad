from openscad import *

def thing(type):
	return [Sphere(30), 
		Cube(60, True), 
		Cylinder(30, 50, True),
		Union([Cube(45, True)]+
			[Rotate([0,0,45][i:i+3], Cube(50, True)) for i in [0,1,2]]
		)
		][(type+1)%4]


openscad.result = Union(
	[Translate([x*100-150,y*100-150], thing(x+y)) 
		for x in range(0,4) for y in range(0,4)]
	)

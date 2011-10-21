from openscad import *
from math import pi

def thing(type):
	return [sphere(30),
		cube(60, True),
		cylinder(50, 30, center=True),
		union([cube(45, True)]+
			[rotate([0,0,pi/4][i:i+3], cube(50, True)) for i in [0,1,2]]
		)
		][(type+1)%4]


openscad.result = union(
	[translate([x*100-150,y*100-150], thing(x+y))
		for x in range(0,4) for y in range(0,4)]
	)

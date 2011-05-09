from openscad import *
from math import sin,cos,pi

def example005():
	return Translate([0, 0, -120],
		[Difference([
			Cylinder(100,50),
			Translate([0, 0, 10], Cylinder(80, 50)),
			Translate([100, 0, 35], Cube(50, True))
		])]
		+ map(lambda i:
			Translate([sin(pi*i/3)*80, 
				cos(pi*i/3)*80, 0 ],
				Cylinder(10,200)), range(0,6))
		+ [Translate([0, 0, 200],
			Cylinder(120,0,80.0)
		)]
	)


openscad.result = example005()

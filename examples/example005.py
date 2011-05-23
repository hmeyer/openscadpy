from openscad import *
from math import sin,cos,pi

def example005():
	return translate([0, 0, -120],
		[difference([
			cylinder(h=50,r=100),
			translate([0, 0, 10], cylinder(h=50, r=80)),
			translate([100, 0, 35], cube(50, True))
		])]
		+ map(lambda i:
			translate([sin(pi*i/3)*80, 
				cos(pi*i/3)*80, 0 ],
				cylinder(r=10,h=200)), range(0,6))
		+ [translate([0, 0, 200],
			cylinder(h=80,r1=120,r2=0)
		)]
	)


openscad.result = example005()

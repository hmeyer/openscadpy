from openscad import *


def example002():
	return intersection([
		difference([
			union([
				cube(30, True),
				translate([0, 0, -25],
					cube([15, 15, 50], True))
			]),
			union([
				cube([50, 10, 10], True),
				cube([10, 50, 10], True),
				cube([10, 10, 50], True)
			])
		]),
		translate([0, 0, 5],
			cylinder(h=50, r1=20, r2=5, center=True))
	])

openscad.result = example002();


from openscad import *

def example003():
	return Difference([
		Union([
			Cube(30, True),
			Cube([40, 15, 15], True),
			Cube([15, 40, 15], True),
			Cube([15, 15, 40], True)
		]),
		Union([
			Cube([50, 10, 10], True),
			Cube([10, 50, 10], True),
			Cube([10, 10, 50], True)
		])
	])

openscad.result = example003()


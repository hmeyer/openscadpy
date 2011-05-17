from openscad import *

def example003():
	return difference([
		union([
			cube(30, True),
			cube([40, 15, 15], True),
			cube([15, 40, 15], True),
			cube([15, 15, 40], True)
		]),
		union([
			cube([50, 10, 10], True),
			cube([10, 50, 10], True),
			cube([10, 10, 50], True)
		])
	])

openscad.result = example003()


from openscad import *

def example004():
	return Difference([
		Cube(30, True),
		Sphere(20)
	])

openscad.result = example004()


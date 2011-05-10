from openscad import *

openscad.result = Intersection(
	map(lambda i:
		Rotate(i, Cube([100,20,20], True)),
		[
			[0, 0, 0],
			[10, 20, 300],
			[200, 40, 57],
			[20, 88, 57]
		])
	)
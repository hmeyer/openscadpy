from openscad import *
from math import pi

openscad.result = intersection(
	map(lambda i:
		rotate([i[0]*pi/180, i[1]*pi/180, i[2]*pi/180], cube([100,20,20], True)),
		[
			[0, 0, 0],
			[10, 20, 300],
			[200, 40, 57],
			[20, 88, 57]
		])
	)
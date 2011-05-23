from openscad import *
import numpy


openscad.result = union([
	translate([x,0,-30],
		cylinder(r1=6,r2=2,h=3*numpy.interp(x,
			[-200,-50,-20,80,150],[5,20,18,25,2])))
				for x in  range(-100,105,5)
	])
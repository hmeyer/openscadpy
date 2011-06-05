from openscad import *
from math import pi

def dilate(r,obj):
	return minkowski([
		obj,
		sphere(r)
	])

def erode(r,obj):
	maxl = 100
	invobj = difference([cube(maxl,center=True),obj])
	dinvobj = dilate(r, invobj)
	invdinvobj = difference([cube(maxl-3*r, center=True), dinvobj])
	return invdinvobj

def morphOpen(r,obj):
	return dilate(r, erode(r, obj) )

def morphClose(r,obj):
	return erode(r, dilate(r, obj) )

thing = union([cube(50, True),
		rotate([pi/9,pi/6,pi/4], cube([10,10,100], True))
		])

#openscad.result = thing
openscad.result = morphClose(5,thing)


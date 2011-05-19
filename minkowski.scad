from openscad import *


def dilate(obj,se):
	return minkowski([
		obj,
		se
	])

def erode(obj,se):
	maxl = 1000
	r = 10
	invobj = difference([cube(maxl,center=True),obj])
	dinvobj = dilate(invobj,se)
	invdinvobj = difference([cube(maxl-3*r, center=True), dinvobj])
	return invdinvobj

def morphOpen(obj,se):
	return dilate(erode(obj,se),se )

def morphClose(obj,se):
	return erode(dilate(obj, se), se)

def bevel(obj):
	return intersection([
		obj,
		rotate(45,[0,0,1],obj),
		rotate(45,[0,1,0],obj),
		rotate(45,[1,0,0],obj),
		rotate(90,[1,1,0],obj),
		rotate(90,[1,0,1],obj),
		rotate(90,[0,1,1],obj),
		rotate(90,[1,-1,0],obj),
		rotate(90,[1,0,-1],obj),
		rotate(90,[0,1,-1],obj),
		rotate(90,[-1,1,0],obj),
		rotate(90,[-1,0,1],obj),
		rotate(90,[0,-1,1],obj),
		])

se = render( bevel( bevel( cube(10, True) ) ))


thing = union([cube(50, True),
		rotate([20,30,40], cube([10,10,100], True))
		])

#openscad.result = se
openscad.result = morphClose(thing,se)
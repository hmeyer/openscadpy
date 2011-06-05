from openscad import *
from math import sin,cos,acos,pi

def screw(type = 2, r1 = 15, r2 = 20, n = 7, height = 100, t = 8):
	p = [ [ [ 2*r2, 0 ],
		[ r2, 0 ],
		[ r1*cos(pi/n), r1*sin(pi/n) ],
		[ r2*cos(pi*2/n), r2*sin(pi*2/n) ],
		[ 2*r2*cos(pi*2/n), 2*r2*sin(pi*2/n) ]],
		[[ 2*r2, 0 ],
		[ r2, 0 ],
		[ r1*cos(pi*0.5/n), r1*sin(pi*0.5/n) ],
		[ r1*cos(pi/n), r1*sin(pi/n) ],
		[ r2*cos(pi*1.5/n), r2*sin(pi*1.5/n) ],
		[ 2*r2*cos(pi*1.5/n), 2*r2*sin(pi*1.5/n) ]]
		]

	return linear_extrude(child=difference( [circle(r2)] +
		[ rotate(i*2*pi/n, child=polygon(p[type-1])) for i in range(0,n)]),
		h=height, twist=pi*2*t/n,convexity=t)


def nut(type = 2, r1 = 16, r2 = 21, r3 = 30, s = 6, n = 7, height = 100/5, t = 8/5):
	return difference([
		cylinder(height,r3).fn(s),
		translate([ 0, 0, -height/2 ], screw(type, r1, r2, n, height*2, t*2))
	])

def spring(r1 = 100, r2 = 10, height = 100, hr = 12, steps = 16):
	def pointFromWirealpha(wirealpha):
		wirer = sin(wirealpha)*r2
		wireh = (cos(wirealpha)+1)/2
		springalpha = 2*pi*wireh*r2/hr
		return [cos(springalpha)*(r1+wirer), sin(springalpha)*(r1+wirer)]
	points1 = []
	points2 = []
	for i in range(0,steps):
		wirealpha = acos((2.0*i-steps)/steps)
		points1.append(pointFromWirealpha(wirealpha))
		points2.append(pointFromWirealpha(wirealpha-pi))
	return linear_extrude(child=polygon(points1+points2),h=height, twist=pi*height/hr, convexity=5).fn( steps*hr/r2 )
	
openscad.result = union([

	translate([ -30, 0, 0 ],
		screw()),

	translate([ 30, 0, 0 ],
		nut()),
	
	spring()
	])

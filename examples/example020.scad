from openscad import *
from math import sin,cos,acos,pi

def screw(type = 2, r1 = 15, r2 = 20, n = 7, h = 100, t = 8):
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

	return LinearExtrude(Difference( [Circle(r2)] +
		[ RotateAxis(i*360/n, Polygon(p[type-1])) for i in range(0,n)]),
		h, 360*t/n,t,-1)


def nut(type = 2, r1 = 16, r2 = 21, r3 = 30, s = 6, n = 7, h = 100/5, t = 8/5):
	cyl = Cylinder(r3,h)
	cyl.fn = s
	return Difference([
		cyl,
		Translate([ 0, 0, -h/2 ], screw(type, r1, r2, n, h*2, t*2))
	])

def spring(r1 = 100, r2 = 10, h = 100, hr = 12, steps = 16):
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
	spring = LinearExtrude(Polygon(points1+points2),h, 180.0*h/hr, 5)
	spring.fn = steps*hr/r2
	return spring
	
openscad.result = Union([

	Translate([ -30, 0, 0 ],
		screw()),

	Translate([ 30, 0, 0 ],
		nut()),
	
	spring()
	])
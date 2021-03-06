from openscad import *
from math import pi

def thing():
	fa( 30 )
	cyl = cylinder(h=62.5,r1=12.5,r2=6.25, center=True)
	return difference([
		sphere(25),
		cyl,
		rotate(pi/2, [ 1, 0, 0 ], cyl),
		rotate(pi/2, [ 0, 1, 0 ], cyl)
		])

def demo_proj():
	t = thing()
	t.background = True
	return union([
		linear_extrude(
			child=projection(child=thing()), h=0.5, center=True),
		t
	])

def demo_cut():
	t = thing()
	t.background = True
	return union(
		[rotate(-pi/6, [ 1, 1, 0 ],
			translate([ 0, 0, -i ],
				linear_extrude(child=projection(cut_mode=True, child=
					translate([ 0, 0, i ],
						rotate(+pi/6, [ 1, 1, 0 ], thing())
					)
				), h=0.5, center=True)
			)
		) for i in range(-20,21,5)]
		+[t])

openscad.result = union([
	translate([ -30, 0, 0 ], demo_proj()),
	translate([ +30, 0, 0 ], demo_cut())
	])

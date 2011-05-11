from openscad import *

def thing():
	openscad.fa = 30
	cyl = Cylinder(12.5,6.25,62.5, True)
	return Difference([
		Sphere(25),
		cyl,
		RotateAxis(90, [ 1, 0, 0 ], cyl),
		RotateAxis(90, [ 0, 1, 0 ], cyl)
		])

def demo_proj():
	t = thing()
	t.background = True
	return Union([
		LinearExtrude(
			Projection(thing()), 0.5, 0, 5, -1, True),
		t
	])

def demo_cut():
	t = thing()
	t.background = True
	return Union(
		[RotateAxis(-30, [ 1, 1, 0 ],
			Translate([ 0, 0, -i ],
				LinearExtrude(Projection(
					Translate([ 0, 0, i ],
						RotateAxis(+30, [ 1, 1, 0 ], thing())
					)
				,True), 0.5, 0, 5, -1, True)
			)
		) for i in range(-20,21,5)]
		+[t])

openscad.result = Union([
	Translate([ -30, 0, 0 ], demo_proj()),
	Translate([ +30, 0, 0 ], demo_cut())
	])

from openscad import *

def ext(lay):
	return dxf_linear_extrude( file="example008.dxf",
			layer=lay, h=50, convexity=3)


openscad.result = difference([
	intersection([
		translate([ -25, -25, -25], ext("G")),

		rotate(90,[1, 0, 0], child=
			translate([ -25, -125, -25], ext("E"))),
		
		rotate(90, [0, 1, 0], child=
			translate([ -125, -125, -25], ext("B")))
	]),

	intersection([
		translate([ -125, -25, -25], ext("X")),

		rotate(90, [0, 1, 0], child=
			translate([ -125, -25, -25], ext("X")))
	])
])
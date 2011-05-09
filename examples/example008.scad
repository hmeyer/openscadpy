from openscad import *

def ext(layer):
	return DxfLinearExtrude( "example008.dxf",
			layer, 50,0,0,0,1,3)


openscad.result = Difference([
	Intersection([
		Translate([ -25, -25, -25], ext("G")),

		RotateAxis(90,[1, 0, 0],
			Translate([ -25, -125, -25], ext("E"))),
		
		RotateAxis(90, [0, 1, 0],
			Translate([ -125, -125, -25], ext("B")))
	]),

	Intersection([
		Translate([ -125, -25, -25], ext("X")),

		RotateAxis(90, [0, 1, 0],
			Translate([ -125, -25, -25], ext("X")))
	])
])
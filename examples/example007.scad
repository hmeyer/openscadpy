from openscad import *

def clip():
	return Difference(
		[DxfRotateExtrude(
				"example007.dxf",
				"dorn",
				0,0,1,3)]
		+ map(lambda r: RotateAxis(r, [0,0,1], cutout()), [0, 90])
		)

def cutout():
	ext = map(lambda layer: Translate([0, 0, -50], 
					DxfLinearExtrude(
						"example007.dxf",
						layer,
						100,0,0,0,1,
						2)), ["cutout1", "cutout2"])
	return Intersection([
		RotateAxis(90, [1, 0, 0], ext[0]),
		RotateAxis(90, [0, 0, 1], 
			RotateAxis(90, [1, 0, 0], ext[1])
		)
	])



def cutview():
	combo = [
			Translate([0, 0, -10],
				clip()),
			RotateAxis(20, [0, 0, 1],
				RotateAxis(-20, [0, 1, 0],
					Translate([18, 0, 0],
						Cube(30, True)
					)
				)
			)
		]
	r = Render(Intersection(combo))
	r.highlight = True
	return Difference([
		Difference(combo),r
	])

openscad.result = Translate([0, 0, -10],
	clip())

#openscad.result = cutview()


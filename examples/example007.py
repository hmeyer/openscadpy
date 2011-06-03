from openscad import *

def clip():
	return difference(
		[dxf_rotate_extrude(
				file="example007.dxf",
				layer="dorn",
				convexity=3)]
		+ map(lambda r: rotate(r, [0,0,1], child=cutout()), [0, 90])
		)

def cutout():
	ext = map(lambda lay: translate([0, 0, -50], 
					dxf_linear_extrude(
						file="example007.dxf",
						layer=lay,
						h=100,
						convexity=2)), ["cutout1", "cutout2"])
	return intersection([
		rotate(90, [1, 0, 0], child=ext[0]),
		rotate(90, [0, 0, 1], 
			rotate(90, [1, 0, 0], child=ext[1])
		)
	])



def cutview():
	combo = [
			translate([0, 0, -10],
				clip()),
			rotate(20, [0, 0, 1], child=
				rotate(-20, [0, 1, 0], child=
					translate([18, 0, 0],
						cube(30, True)
					)
				)
			)
		]
	r = render(intersection(combo))
	r.highlight = True
	return difference([
		difference(combo),r
	])

assemble( translate([0, 0, -10],
	clip()) )

#openscad.result = cutview()


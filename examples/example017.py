from openscad import *

# To render the DXF file from the command line:
# openscad -x example017.dxf -D'mode="parts"' example017.scad

#mode = "parts"
#mode = "exploded"
mode = "assembled"

thickness = 6
locklen1 = 15
locklen2 = 10
boltlen = 15
midhole = 10
inner1_to_inner2 = 50
total_height = 80


def shape_tripod():
	x1 = 0
	x2 = x1 + thickness
	x3 = x2 + locklen1
	x4 = x3 + thickness
	x5 = x4 + inner1_to_inner2
	x6 = x5 - thickness
	x7 = x6 - locklen2
	x8 = x7 - thickness
	x9 = x8 - thickness
	x10 = x9 - thickness
	
	y1 = 0
	y2 = y1 + thickness
	y3 = y2 + thickness
	y4 = y3 + thickness
	y5 = y3 + total_height - 3*thickness
	y6 = y5 + thickness
	roundedCorner = intersection([
		circle(thickness),
		translate([ -thickness*2, 0 ],
			square(thickness*2))
	])
	return union([
		difference([
			polygon([
				[ x1, y2 ], [ x2, y2 ],
				[ x2, y1 ], [ x3, y1 ], [ x3, y2 ],
				[ x4, y2 ], [ x4, y1 ], [ x5, y1 ],
				[ x5 + thickness, y3 ], [ x5, y4 ],
				[ x5, y5 ],
				[ x6, y5 ], [ x6, y6 ], [ x7, y6 ], [ x7, y5 ], [ x8, y5 ],
				[ x8, y6 ], [ x9, y5 ],
				[ x9, y4 ], [ x10, y3 ],
				[ x2, y3 ]
			]),
			translate([ x10, y4 ], circle(thickness)),
			translate([ x5 + thickness, y4 ], circle(thickness))
		]),
		translate([ x5, y1 ],
			square([ boltlen - thickness, thickness*2 ])),
	
		translate([ x5 + boltlen - thickness, y2 ],
			circle(thickness)),

		translate([ x2, y2 ],roundedCorner),
	
		translate([ x8, y5 ],roundedCorner)
	])


def shape_X_disc(r1, holetrans, holelen, r2):
	return difference(
		[circle(r1)]
		+map(lambda alpha: rotate(alpha, child=
			translate([ 0, holetrans ],
				square([ thickness, holelen ], True)
			)
		), [ 0, 120, 240 ])
		+ [circle(r2)])

def shape_inner_disc():
	return shape_X_disc(
		midhole + boltlen + 2*thickness + locklen2,
		midhole + boltlen + thickness + locklen2/2,
		locklen2,
		midhole + boltlen)

def shape_outer_disc():
	return shape_X_disc(
		midhole + boltlen + inner1_to_inner2 + 2*thickness + locklen1,
		midhole + boltlen + inner1_to_inner2 + thickness + locklen1/2,
		locklen1,
		midhole + boltlen + inner1_to_inner2)
		

def parts():
	tripod_x_off = locklen1 - locklen2 + inner1_to_inner2;
	tripod_y_off = max([midhole + boltlen + inner1_to_inner2 + 4*thickness + locklen1, total_height]);

	return union([
		shape_inner_disc(),
		shape_outer_disc()]
		+ map(lambda s: scale(s, translate([ tripod_x_off, -tripod_y_off ], shape_tripod())), [ [1,1], [-1,1], [1,-1] ])
	)



def exploded():
	return construct(1.5 * thickness)

def bottle():
	r = boltlen + midhole
	h = total_height - thickness*2
	return rotate_extrude(children=[
		square([ r, h ]),

		translate([ 0, h ],
			intersection([
				square([ r, r ]),
				scale([ 1, 0.7 ], circle(r))
			])
		),

		translate([ 0, h+r ],
			intersection([
				translate([ 0, -r/2 ], square([ r/2, r ])),
				circle(r/2)
			])
		)
	], convexity=2)

def construct(extraspace=0): #1.5*thickness
	assembly = [
		translate([ 0, 0, total_height - thickness + 2*extraspace],
			linear_extrude(child=shape_inner_disc(), h=thickness)),
		linear_extrude(child=shape_outer_disc(), h=thickness),
		color([ 0.7, 0.7, 1 ],
			map(lambda alpha: rotate(alpha, child=
				translate([ 0, thickness*2 + locklen1 + inner1_to_inner2 + boltlen + midhole, extraspace ],
					rotate([ 90, 0, -90 ],
						linear_extrude(child=shape_tripod(), h=thickness, convexity=10, center=True)
					)
				)
			),[ 0, 120, 240 ])
		)
	]
	if  extraspace == 0:
		b = translate([ 0, 0, thickness*2], bottle())
		b.background = True
		assembly.append(b)
	return union(assembly)

def assembled():
	return construct()

if mode == "parts":
	openscad.result = parts()

if mode == "exploded":
	openscad.result = exploded()

if mode == "assembled":
	openscad.result = assembled()

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
	roundedCorner = Intersection([
		Circle(thickness),
		Translate([ -thickness*2, 0 ],
			Square(thickness*2))
	])
	return Union([
		Difference([
			Polygon([
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
			Translate([ x10, y4 ], Circle(thickness)),
			Translate([ x5 + thickness, y4 ], Circle(thickness))
		]),
		Translate([ x5, y1 ],
			Square([ boltlen - thickness, thickness*2 ])),
	
		Translate([ x5 + boltlen - thickness, y2 ],
			Circle(thickness)),

		Translate([ x2, y2 ],roundedCorner),
	
		Translate([ x8, y5 ],roundedCorner)
	])


def shape_X_disc(r1, holetrans, holelen, r2):
	return Difference(
		[Circle(r1)]
		+map(lambda alpha: RotateAxis(alpha, 
			Translate([ 0, holetrans ],
				Square([ thickness, holelen ], True)
			)
		), [ 0, 120, 240 ])
		+ [Circle(r2)])

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

	return Union([
		shape_inner_disc(),
		shape_outer_disc()]
		+ map(lambda s: Scale(s, Translate([ tripod_x_off, -tripod_y_off ], shape_tripod())), [ [1,1], [-1,1], [1,-1] ])
	)



def exploded():
	return construct(1.5 * thickness)

def bottle():
	r = boltlen + midhole
	h = total_height - thickness*2
	return RotateExtrude([
		Square([ r, h ]),

		Translate([ 0, h ],
			Intersection([
				Square([ r, r ]),
				Scale([ 1, 0.7 ], Circle(r))
			])
		),

		Translate([ 0, h+r ],
			Intersection([
				Translate([ 0, -r/2 ], Square([ r/2, r ])),
				Circle(r/2)
			])
		)
	],2)

def construct(extraspace=0): #1.5*thickness
	assembly = [
		Translate([ 0, 0, total_height - thickness + 2*extraspace],
			LinearExtrude(shape_inner_disc(), thickness)),
		LinearExtrude(shape_outer_disc(), thickness),
		Color([ 0.7, 0.7, 1 ],
			map(lambda alpha: RotateAxis(alpha, 
				Translate([ 0, thickness*2 + locklen1 + inner1_to_inner2 + boltlen + midhole, extraspace ],
					Rotate([ 90, 0, -90 ],
						LinearExtrude(shape_tripod(), thickness, 0, 10, -1, True)
					)
				)
			),[ 0, 120, 240 ])
		)
	]
	if  extraspace == 0:
		b = Translate([ 0, 0, thickness*2], bottle())
		b.background = True
		assembly.append(b)
	return Union(assembly)

def assembled():
	return construct()

if mode == "parts":
	openscad.result = parts()

if mode == "exploded":
	openscad.result = exploded()

if mode == "assembled":
	openscad.result = assembled()


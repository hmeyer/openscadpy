from openscad import *
from math import sqrt, pow, sin, cos, pi

# numbers of iterations to recurse, you don't need many
iterations = 4

# size is the distance from the center of the base
# to the corner.
size = 35

# set to True to render inverse pyramid
inverse = False

# set to True to render a square-base pyramid, False for triangular
square = False

# some geometry to compute the height of the pyramids
def get_height(size, square=True):
	if square:
		c = sqrt(pow(size, 2)+pow(size, 2))
		return sqrt(pow(c, 2)-pow(c/2, 2))
	else:
		return 2*size*sin(pi/3)*sqrt(6)/3

def pyramid(size, height, square=True):
	if square:
		return polyhedron(
			[#points
				[size, 0, 0],
				[0, size, 0],
				[-size, 0, 0],
				[0, -size, 0],
				[0, 0, height]
			],
			[#triangles
				[0, 1, 2, 3],
				[4, 1, 0],
				[4, 2, 1],
				[4, 3, 2],
				[4, 0, 3]
			]
		)
	else:
		sval = size*sin(pi/3)
		cval = size*cos(pi/3)
		return polyhedron(
			[#points
				[sval, -cval, 0],
				[0, size, 0],
				[-sval, -cval, 0],
				[0, 0, height]
			],
			[#triangles
				[0, 1, 2],
				[3, 1, 0],
				[3, 2, 1],
				[0, 2, 3]
			]
		)

# recursize sierpinski pyramid method
def sierpinski_rec(iters, size, inverse, square):
	if iters == 0:
		if inverse:
			height = get_height(size/2, square)
			return translate(
				[0, 0, height],
				rotate([180, 0, 180], pyramid(size/2, height, square))
			)
		else:
			height = get_height(size, square)
			return pyramid(size, height, square)
	else:
		size = size/2
		tiny = 0.01*size
		height = get_height(size, square)
		if square:
			trans = \
				[#individual pyramid locations
					[0, -size+tiny, 0],
					[size-tiny, 0, 0],
					[-size+tiny, 0, 0],
					[0, size-tiny, 0], 
					[0, 0, height-tiny]
				]
		else:
			sval = size*sin(pi/3)
			cval = size*cos(pi/3)
			trans = \
				[#individual pyramid locations
					[sval-tiny, -cval+tiny, 0],
					[0, size-tiny, 0],
					[-sval+tiny, -cval+tiny, 0],
					[0, 0, height-tiny]
				]
	
		pyramids = []
		for t in trans:
			pyramids.append(
				translate(t, sierpinski_rec(iters-1, size, inverse, square))
			)
		if inverse:
			pyramids.append(
				translate([0, 0, 0], sierpinski(iters-1, size*2, True, square))
			)
		
		return union(pyramids)

def sierpinski(iters=4, size=35, inverse=False, square=True):
	# use power of two size to avoid using floats, then scale down/up
	# use iters+1 to handle inverse case
	full_size = pow(2, (iters+1))
	real_scale = size/full_size
	return scale(
		[real_scale, real_scale, real_scale],
		[sierpinski_rec(iters, full_size, inverse, square)]
	)

openscad.result = sierpinski(iterations, size, inverse, square)
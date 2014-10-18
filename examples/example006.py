from openscad import *
from math import pi


def example006():
	def edgeprofile():
		return render(difference([
			cube([20, 20, 150], center=True),
			translate([-10, -10, 0],
				cylinder(h=80, r=10, center=True)),
			translate([-10, -10, +40],
				sphere(10)),
			translate([-10, -10, -40],
				sphere(10))
		]))
	return difference([
		cube(100, True)]
		+map(lambda rot: rotate(ang=pi/2, vec=rot, children=
			map(lambda p: translate([ p[0]*50, p[1]*50, 0 ],
				rotate(ang=p[2], vec=[0,0,1], child=edgeprofile())
				), [	[+1, +1, 0],
					[-1, +1, pi/2],
					[-1, -1, pi],
					[+1, -1, 3*pi/2]])
			)
			,[ [0, 0, 0], [1, 0, 0], [0, 1, 0] ])
		+ map(lambda i: rotate(i[0],[0,0,1], child=
				rotate(ang=i[1],vec=[1,0,0], child=
					translate([0,-50,0],
						map(lambda j: translate([j[0],0,j[1]],
							sphere(10)), i[2]
						)
					)
				)
			),
			[
				[ 0, 0, [ [0, 0] ] ],
				[ pi/2, 0, [ [-20, -20], [+20, +20] ] ],
				[ pi, 0, [ [-20, -25], [-20, 0], [-20, +25], [+20, -25], [+20, 0], [+20, +25] ] ],
				[ 3*pi/2, 0, [ [0, 0], [-25, -25], [+25, -25], [-25, +25], [+25, +25] ] ],
				[ 0, pi/2, [ [-25, -25], [0, 0], [+25, +25] ] ],
				[ 0, -pi/2, [ [-25, -25], [+25, -25], [-25, +25], [+25, +25] ] ]
			])
		)

assemble( example006() )

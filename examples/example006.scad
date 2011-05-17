from openscad import *

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
		+map(lambda rot: rotate(ang=90, vec=rot, children=
			map(lambda p: translate([ p[0]*50, p[1]*50, 0 ],
				rotate(ang=p[2], vec=[0,0,1], child=edgeprofile())
				), [	[+1, +1, 0], 
					[-1, +1, 90], 
					[-1, -1, 180], 
					[+1, -1, 270]])
			)
			,[ [0, 0, 0], [1, 0, 0], [0, 1, 0] ])
		+ map(lambda i: rotate(i[0],[0,0,1],
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
				[ 90, 0, [ [-20, -20], [+20, +20] ] ],
				[ 180, 0, [ [-20, -25], [-20, 0], [-20, +25], [+20, -25], [+20, 0], [+20, +25] ] ],
				[ 270, 0, [ [0, 0], [-25, -25], [+25, -25], [-25, +25], [+25, +25] ] ],
				[ 0, 90, [ [-25, -25], [0, 0], [+25, +25] ] ],
				[ 0, -90, [ [-25, -25], [+25, -25], [-25, +25], [+25, +25] ] ]
			])
		)

openscad.result = example006()
print openscad.result

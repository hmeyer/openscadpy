def example006():
	def edgeprofile():
		return Render(Difference([
			Cube([20, 20, 150], True),
			Translate([-10, -10, 0],
				Cylinder(10, 80, True)),
			Translate([-10, -10, +40],
				Sphere(10)),
			Translate([-10, -10, -40],
				Sphere(10))
		]))

	return Difference([
		Cube(100, True)]
		+map(lambda rot: RotateAxis(90, rot,
			map(lambda p: Translate([ p[0]*50, p[1]*50, 0 ],
				RotateAxis(p[2], [0,0,1], edgeprofile())
				), [	[+1, +1, 0], 
					[-1, +1, 90], 
					[-1, -1, 180], 
					[+1, -1, 270]])
			)
			,[ [0, 0, 0], [1, 0, 0], [0, 1, 0] ])
		+ map(lambda i: RotateAxis(i[0],[0,0,1],
				RotateAxis(i[1],[1,0,0], 
					Translate([0,-50,0],
						map(lambda j: Translate([j[0],0,j[1]],
							Sphere(10)), i[2]
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

result = example006()

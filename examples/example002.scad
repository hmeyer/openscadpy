def example002():
	return Intersection([
		Difference([
			Union([
				Cube(30, True),
				Translate([0, 0, -25],
					Cube([15, 15, 50], True))
			]),
			Union([
				Cube([50, 10, 10], True),
				Cube([10, 50, 10], True),
				Cube([10, 10, 50], True)
			])
		]),
		Translate([0, 0, 5],
			Cylinder(20, 5, 50, True))
	])

result = example002();


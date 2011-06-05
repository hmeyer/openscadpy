from openscad import *
from math import pi

def shape():
	return union([
		difference([
			translate([ -35, -35 ], intersection([
				union([
					difference([
						square(100, True),
						square(50, True)
					]),
					translate([ 50, 50 ],
						square(15, True) )
				]),
				rotate(pi/4, child=
					translate([ 0, -15 ],
						square([ 100, 30 ])
					)
				)
			])),
			rotate(-pi/4, child=scale([ 0.7, 1.3 ], circle(5)))
		]),
 		import_dxf( "example009.dxf",
			"body",scale=2.0, convexity=10)	
	])


#openscad.result = LinearExtrude(shape(),10,0,5,-1,True)
openscad.result = shape()
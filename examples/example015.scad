from openscad import *

def shape():
	return Union([
		Difference([
			Translate([ -35, -35 ], Intersection([
				Union([
					Difference([
						Square(100, True),
						Square(50, True)
					]),
					Translate([ 50, 50 ],
						Square(15, True) )
				]),
				Rotate(45,
					Translate([ 0, -15 ],
						Square([ 100, 30 ])
					)
				)
			]),
			Rotate(-45, Scale([ 0.7, 1.3 ], Circle(5))
		])#,
 		#ImportDXF( "example009.dxf",
		#	"body",0,0,2.0,10)	
	])


#// linear_extrude(convexity = 10, center = true)
openscad.result = LinearExtrude(shape(),10)
#openscad.result = shape()

'''
'''

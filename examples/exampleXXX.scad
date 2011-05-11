from openscad import *

spongemin = 3
cubesize = 100

def sponge(offx,offy,size):
	s3 = size/3.0
	res = [
		Translate([s3+offx,s3+offy,-1], 
			Cube([s3, s3, cubesize+2]))]
	if size > spongemin:
		res += [
			sponge(offx,offy,s3),
			sponge(offx+s3,offy,s3),
			sponge(offx+s3+s3,offy,s3),
			sponge(offx,offy+s3,s3),
			#sponge(offx+s3+s3+s3,offy,s3),
			sponge(offx+s3+s3,offy+s3,s3),
			sponge(offx,offy+s3+s3,s3),
			sponge(offx+s3,offy+s3+s3,s3),
			sponge(offx+s3+s3,offy+s3+s3,s3)
		]
	return Union(res)

s = Translate([-cubesize/2.0]*3, sponge(0,0,cubesize))

openscad.result = Difference(
  [Cube(cubesize, True),
   s,
   Rotate([90,0,0],s),
   Rotate([0,90,0],s)
   ])

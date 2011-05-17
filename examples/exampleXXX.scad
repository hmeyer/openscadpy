from openscad import *

spongemin = 30
cubesize = 100

def sponge(offx,offy,size):
	s3 = size/3.0
	res = [
		translate([s3+offx,s3+offy,-1], 
			cube([s3, s3, cubesize+2]))]
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
	return union(res)

s = translate([-cubesize/2.0]*3, sponge(0,0,cubesize))

openscad.result = difference(
  [cube(cubesize, True),
   s,
   rotate([90,0,0],s),
   rotate([0,90,0],s)
   ])

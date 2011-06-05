from openscad import *
from math import pi

iterations = 3
cubesize = 100

def bars(maxIter = 1, offx=0,offy=0,iter=1):
	s3 = cubesize / pow(3, iter)
	res = [
		translate([s3+offx,s3+offy,-1], 
			cube([s3, s3, cubesize+2]))]
	if iter < maxIter:
		iter+=1
		res += [
			bars(maxIter,offx+x*s3,offy+y*s3,iter) 
				for x in range(0,3) for y in range (0,3)]
	return union(res)

def sponge(maxIter=3):
	s = translate([-cubesize/2.0]*3, bars(maxIter))
	return difference(
		[cube(cubesize, True),
 			s,
 		 	rotate([pi/2,0,0],s),
 			rotate([0,pi/2,0],s)
 		])


openscad.result = sponge(iterations)

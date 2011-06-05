from openscad import *

# example010.dat generated using octave:
#   d = (sin(1:0.2:10)' * cos(1:0.2:10)) * 10;
#   save("example010.dat", "d");

s = surface("example010.dat",5,True)

openscad.result = intersection([
	s, rotate(pi/4, [0, 0, 1], child=s)	])

from openscad import *

# example010.dat generated using octave:
#   d = (sin(1:0.2:10)' * cos(1:0.2:10)) * 10;
#   save("example010.dat", "d");

s = Surface("example010.dat",5,True)

openscad.result = Intersection([
	s, RotateAxis(45, [0, 0, 1],s)	])

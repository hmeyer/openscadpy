from openscad import *
from math import pi

t = dxf_linear_extrude(file="example013.dxf",
			h=100,convexity=3,center=True)

assemble(
	t & t.rotate([0, pi/2, 0]) & t.rotate([pi/2, 0, 0])
)
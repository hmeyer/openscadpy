from openscad import *

t = dxf_linear_extrude(file="example013.dxf",
			h=100,convexity=3,center=True)
openscad.result =intersection([
	t, rotate([0, 90, 0], child=t),	rotate([90, 0, 0], child=t )])

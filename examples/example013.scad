from openscad import *

t = DxfLinearExtrude( "example013.dxf",
			"", 100,0,0,0,1,3,-1,True)

openscad.result =Intersection([
	t, Rotate([0, 90, 0], t),	Rotate([90, 0, 0], t )])
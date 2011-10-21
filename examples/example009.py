from openscad import *

dxf = "example009.dxf"
bodywidth = DxfDim(dxf,"","bodywidth")
fanwidth = DxfDim(dxf,"","fanwidth")
platewidth = DxfDim(dxf,"","platewidth")
fan_side_center = DxfCross(dxf,"fan_side_center")
fanrot = DxfDim(dxf,"","fanrot")

frame = union( [
	dxf_linear_extrude(dxf, "body",
		bodywidth, convexity=10, center=True)]
	+map(lambda z:
		translate([0, 0, z],
			dxf_linear_extrude(dxf, "plate",
				platewidth, convexity = 10, center=True)),
		[+(bodywidth/2 + platewidth/2),
			-(bodywidth/2 + platewidth/2)]
	))

frame.background = True

fan = intersection([
		dxf_linear_extrude(dxf, "fan_top",
			fanwidth,-fanrot,convexity = 10, center=True),
		dxf_rotate_extrude(dxf, "fan_side",
			origin=fan_side_center, convexity=10)
		])

openscad.result = union( [frame,fan] )
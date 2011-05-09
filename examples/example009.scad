from openscad import *

dxf = "example009.dxf"
bodywidth = DxfDim(dxf,"","bodywidth")
fanwidth = DxfDim(dxf,"","fanwidth")
platewidth = DxfDim(dxf,"","platewidth")
fan_side_center = DxfCross(dxf,"fan_side_center")
fanrot = DxfDim(dxf,"","fanrot")

frame = Union( [
	DxfLinearExtrude(dxf, "body",
		bodywidth,0,0,0,1,10,-1,True)]
	+map(lambda z: 
		Translate([0, 0, z],
			DxfLinearExtrude(dxf, "plate",
				platewidth,0,0,0,1,10,-1,True)),
		[+(bodywidth/2 + platewidth/2),
			-(bodywidth/2 + platewidth/2)]
	))

frame.background()

fan = Intersection([
		DxfLinearExtrude(dxf, "fan_top",
			fanwidth,-fanrot,0,0,1,10,-1,True),
		DxfRotateExtrude(dxf, "fan_side",
			fan_side_center[0], fan_side_center[1],1,10)
		])

openscad.result = Union( [frame,fan] )
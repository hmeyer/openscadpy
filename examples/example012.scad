from openscad import *

# example012.stl is Mblock.stl, (c) 2009 Will Langford
# licensed under the Creative Commons - GNU GPL license.
# http://www.thingiverse.com/thing:753

openscad.result = Difference([
	Sphere(20),
	
	Translate([ -2.92, 0.5, +20 ], Rotate([180, 0, 180],
			ImportSTL("example012.stl", 5)
	))])


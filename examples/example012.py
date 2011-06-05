from openscad import *
from math import pi

# example012.stl is Mblock.stl, (c) 2009 Will Langford
# licensed under the Creative Commons - GNU GPL license.
# http://www.thingiverse.com/thing:753

openscad.result = difference([
	sphere(20),
	
	translate([ -2.92, 0.5, +20 ], rotate([pi, 0, pi],
			import_stl("example012.stl", 5)
	))])


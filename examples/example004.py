from openscad import *

def example004():
	return cube(30, True) - sphere(20)

assemble( example004() )


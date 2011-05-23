from openscad import *


# example016.stl is derived from Mblock.stl
# (c) 2009 Will Langford licensed under
# the Creative Commons - GNU GPL license.
# http://www.thingiverse.com/thing:753
#
# Jonas Pfeil converted the file to binary
# STL and duplicated its content.


def blk1():
	return cube([ 65, 28, 28 ], True)


def blk2():
	return difference([
		translate([ 0, 0, 7.5 ],
			cube([ 60, 28, 14 ], True)),
		cube([ 8, 32, 32 ], True)
	])


def chop():
	return translate([ -14, 0, 0 ],
			import_stl("example016.stl", 12))

openscad.result = difference(
	[blk1()]
	+map(lambda alpha: rotate(alpha, [1], render(
		difference([
			blk2(),
			chop()
		]), 12)),  [0, 90, 180, 270])
)


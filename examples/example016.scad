from openscad import *


# example016.stl is derived from Mblock.stl
# (c) 2009 Will Langford licensed under
# the Creative Commons - GNU GPL license.
# http://www.thingiverse.com/thing:753
#
# Jonas Pfeil converted the file to binary
# STL and duplicated its content.


def blk1():
	return Cube([ 65, 28, 28 ], True)


def blk2():
	return Difference([
		Translate([ 0, 0, 7.5 ],
			Cube([ 60, 28, 14 ], True)),
		Cube([ 8, 32, 32 ], True)
	])


def chop():
	return Translate([ -14, 0, 0 ],
			ImportSTL("example016.stl", 12))

'''
dfference() {
	blk1();
	for (alpha = [0, 90, 180, 270]) {
		rotate(alpha, [ 1, 0, 0]) render(convexity = 12)
			difference() {
				blk2();
				chop();
			}
	}
}
'''

openscad.result = Difference(
	[blk1()]
	+map(lambda alpha: RotateAxis(alpha, [1], Render(
		Difference([
			blk2(),
			chop()
		]), 12)),  [0, 90, 180, 270])
)


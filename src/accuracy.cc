#include "accuracy.h"
#include "grid.h"
#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;

/*!
	Returns the number of subdivision of a whole circle, given radius and
	the three special variables $fn, $fs and $fa
*/

int get_fragments_from_r(double r, const Accuracy &acc) {
	if (r < GRID_FINE) return 0;
	if (acc.fn > 0.0)
		return (int)acc.fn;
	return (int)ceil(max(min(360.0 / acc.fa, r*M_PI / acc.fs), 5.0));
}
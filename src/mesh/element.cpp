/*
Copyright 2015, Matthijs van Dorp.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <tiny/mesh/staticmesh.h>

#include "vecmath.hpp"
#include "element.hpp"

using namespace strata::mesh;

/* Note the right-handedness and axis convention of OpenGL space:
 *    y
 *   /|\
 *    |
 *    o------> x
 *   /
 * |/_
 * z    
 *
 * where we are using the zx plane, (positive z horizontal, positive x vertical if mapped to screen) looking pretty much like standard 2d xy. */

/* Approach:
 * We start with a single edge _ and then try to add polygons to the right and above of this starting point.
 * We repeatedly do the following:
 * - for every edge in the list of to-be-created polygons plist, try making a polygon;
 * - if successful (it didn't already exist), add the following to plist:
 *   - if the edge from plist looks like _, we create a polygon /\ and we add polygons to the left (/) and right (\);
 *   - if the edge from plist looks like \, we create a polygon \/ and we add polygons to the top (_) and right (/);
 *   - if the edge from plist looks like /, it depends:
 *     - either we create to the right /\ and we only add a polygon to the right (\);
 *     - or     we create to the left  \/ and we only add a polygon to the above (_); 
 *
 * The last branching is required because otherwise we either have to make non-equilateral triangles, or we can only fill to the right
 * and above (and the top left area would never be covered). */

/** An instruction to create a polygon a-b-c (clockwise). Other polygons to be created are added to the list.
  */
void MeshBundle::createFlatLayerPolygon(std::deque<VertPair> & plist, xVert _a, xVert _b, float limit)
{
	Vertex & a = vertices[ve[_a]];
	Vertex & b = vertices[ve[_b]];
	tiny::vec3 ab(b.pos-a.pos);
	tiny::vec3 bc = roty(ab,-3.1415926535f*0.6666666667f); // a rotation of 120 degrees or 2pi/3
	if( std::max(std::fabs((b.pos+bc).x),std::fabs((b.pos+bc).z)) > limit ) return; // Don't make polygons whose vertices are outside of the limit
	xVert _c = findNeighbor(b.pos+bc, b); // find neighbor of 'b' at c pos
	if(_c == 0) _c = findNeighbor(b.pos+bc, a); // try 'a' too
	if(_c == 0) _c = addVertex(b.pos+bc); // add new vertex if neither a nor b have a neighbor at c's pos
	Vertex & c = vertices[ve[_c]];
	addPolygon(a,b,c); // add the polygon. It may already exist but then this call is just ignored.

//	std::cout << " going to list new pairs for polygon "<<a.index<<","<<b.index<<","<<c.index<<std::endl;
	// check whether polygon added has ab as a horizontal line (note that in this case b.z < a.z in this case because of clockwise-ness) or is a \ side (note the xor):
	if( a.pos.z > b.pos.z + 0.9*length(ab) || ( (b.pos.x > a.pos.x) != (b.pos.z > a.pos.z) ))
	{ plist.push_back( VertPair(a.index, c.index) ); plist.push_back( VertPair(c.index, b.index) ); }
	else
	{
		if(a.pos.z > b.pos.z) plist.push_back( VertPair(a.index, c.index) ); // the to-the-left-of (/) case: add to the top
		else plist.push_back( VertPair(c.index, b.index) ); // the to-the-right-of (/) case: add to the right
	}
//	std::cout << " Printing plist: "; for(unsigned int i = 0; i < plist.size(); i++) std::cout << "("<<plist[i].a<<","<<plist[i].b<<"), "; std::cout << std::endl;
}

void MeshBundle::createFlatLayer(float size, unsigned int ndivs, float height)
{
	float step = size/ndivs;
	Vertex v1(-size/2, height, -size/2);
	Vertex v2(-size/2, height, -size/2 + step);
	xVert b = addVertex(v1);
	xVert a = addVertex(v2);
	printLists();

	std::deque<VertPair> plist;
	plist.push_back( VertPair(a,b) );
	while(plist.size() > 0) { createFlatLayerPolygon(plist, plist.front().a, plist.front().b, size/2); plist.pop_front(); }
	printLists();
}

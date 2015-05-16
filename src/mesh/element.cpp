/*
This file is part of Chathran Strata: https://github.com/takenu/strata
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
void MeshBundle::createFlatLayerPolygon(std::deque<VertPair> & plist, xVert _a, xVert _b, float limit, float step)
{
	++polyAttempts;
	Vertex & a = vertices[ve[_a]];
	Vertex & b = vertices[ve[_b]];
	tiny::vec3 ab(b.pos-a.pos);
/*	tiny::vec3 bc = roty(ab,-3.1415926535*0.6666666667); // a rotation of 120 degrees or 2pi/3
	if( std::max(std::fabs((b.pos+bc).x),std::fabs((b.pos+bc).z)) > limit ) return; // Don't make polygons whose vertices are outside of the limit
	xVert _c = findNeighbor(b.pos+bc, b); // find neighbor of 'b' at c pos
	if(_c == 0) _c = findNeighbor(b.pos+bc, a); // try 'a' too
	if(_c == 0) _c = addVertex(b.pos+bc); // add new vertex if neither a nor b have a neighbor at c's pos*/
	ab = normalize(ab)*step;
	tiny::vec3 cpos = a.pos + ab*0.5 + tiny::vec3(-ab.z, 0.0f, ab.x)*sqrt(3.0)*0.5;
//	std::cout << " dist diff to step: "<<step-length(cpos-b.pos)<<","<<step-length(cpos-a.pos)<<std::endl;
	if( std::max(std::fabs(cpos.x),std::fabs(cpos.z)) > limit ) return; // Don't make polygons whose vertices are outside of the limit
/*	xVert _c = findNeighbor(cpos, b); // find neighbor of 'b' at c pos
	if(_c == 0) _c = findNeighbor(cpos, a); // try 'a' too
	if(_c == 0) _c = addVertex(cpos); // add new vertex if neither a nor b have a neighbor at c's pos */
	xVert _c = findNeighborVertex(b, a, true); // find neighbor of 'a'
	if(_c == 0) _c = findNeighborVertex(a, b, false); // find neighbor of 'b' (now we must look counterclockwise)
	if(_c == 0) _c = addVertex(cpos); // If no suitable neighbors exist, make a vertex right in the middle and at the usual grid distance (such that the polygon will be equilateral)

	Vertex & c = vertices[ve[_c]];
	if(addPolygon(a,b,c)) // add the polygon. It may already exist but then this call is just ignored.
	{
//		std::cout << " going to list new pairs for polygon "<<a.index<<","<<b.index<<","<<c.index<<std::endl;
		// check whether polygon added has ab as a horizontal line (note that in this case b.z < a.z in this case because of clockwise-ness) or is a \ side (note the xor):
		if( a.pos.z > b.pos.z + 0.9*length(ab) || ( (b.pos.x > a.pos.x) != (b.pos.z > a.pos.z) ))
		{ plist.push_back( VertPair(a.index, c.index) ); plist.push_back( VertPair(c.index, b.index) ); }
		else
		{
			if(a.pos.z > b.pos.z) plist.push_back( VertPair(a.index, c.index) ); // the to-the-left-of (/) case: add to the top
			else plist.push_back( VertPair(c.index, b.index) ); // the to-the-right-of (/) case: add to the right
		}
//		std::cout << " Printing plist: "; for(unsigned int i = 0; i < plist.size(); i++) std::cout << "("<<plist[i].a<<","<<plist[i].b<<"), "; std::cout << std::endl;
		if( (polygons.size()%100)==0) std::cout << " Added "<<polygons.size()<<" polygons so far. "<<std::endl;
	}
}

void MeshBundle::createFlatLayer(float size, unsigned int ndivs, float height)
{
	float step = size/ndivs;
	float xstart = floor(size/(2*step*sqrt(0.75)))*(step*sqrt(0.75));
	Vertex v1(-xstart, height, -size/2);
	Vertex v2(-xstart, height, -size/2 + step);
	xVert b = addVertex(v1);
	xVert a = addVertex(v2);
	printLists();

	std::deque<VertPair> plist;
	plist.push_back( VertPair(a,b) );
	while(plist.size() > 0)
	{
		createFlatLayerPolygon(plist, plist.front().a, plist.front().b, 1.00001*size/2, step);
		plist.pop_front();
		if(polygons.size() > 10 * ndivs * ndivs) { std::cerr << " Warning : createFlatLayer() : Too many polygons are getting created, stopping prematurely. "<<std::endl; break; }
	}
//	printLists();
	printPolygons(step, 100);
	std::cout << " Finished creating a flat layer with "<<vertices.size()<<" vertices and "<<polygons.size()<<" polygons, using "<<polyAttempts<<" attempts. "<<std::endl;
}

tiny::mesh::StaticMesh MeshBundle::convertToMesh(float size)
{
	tiny::mesh::StaticMesh mesh;
	for(unsigned int i = 1; i < vertices.size(); i++) mesh.vertices.push_back( tiny::mesh::StaticMeshVertex(
				tiny::vec2(vertices[i].pos.z/size + 0.5, vertices[i].pos.x/size + 0.5), // texture coordinate
				tiny::vec3(1.0f,0.0f,0.0f), // tangent (appears to do nothing)
				tiny::vec3(0.0f,1.0f,0.0f), // normal
				vertices[i].pos ) ); // position
	for(unsigned int i = 1; i < polygons.size(); i++)
	{
		mesh.indices.push_back( ve[polygons[i].c] - 1 ); // -1 because vertices[0] is the error value and the mesh doesn't have that so it's shifted by 1
		mesh.indices.push_back( ve[polygons[i].b] - 1 ); // Note that we add polygons in reverse order because OpenGL likes them counterclockwise
		mesh.indices.push_back( ve[polygons[i].a] - 1 );
	}

	return mesh;
}

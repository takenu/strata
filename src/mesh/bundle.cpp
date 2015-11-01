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

#include <exception>

#include "vecmath.hpp"
#include "element.hpp"
#include "bundle.hpp"
#include "strip.hpp"

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
void Bundle::createFlatLayerPolygon(std::deque<VertPair> & plist, xVert _a, xVert _b, float limit, float step)
{
	++polyAttempts;
	Vertex & a = vertices[ve[_a]];
	Vertex & b = vertices[ve[_b]];
	tiny::vec3 ab(b.pos-a.pos); // The line between vertices a and b
	ab = normalize(ab)*step; // rescaled to the step size
	tiny::vec3 cpos = a.pos + ab*0.5 + tiny::vec3(-ab.z, 0.0f, ab.x)*sqrt(3.0)*0.5;
	if( std::max(std::fabs(cpos.x),std::fabs(cpos.z)) > limit ) return; // Don't make polygons whose vertices are outside of the limit
	xVert _c = findNeighborVertex(b, a, true); // find neighbor of 'a'
	if(_c == 0) _c = findNeighborVertex(a, b, false); // find neighbor of 'b' (now we must look counterclockwise)
	if(_c == 0) _c = addVertex(cpos); // If no suitable neighbors exist, make a vertex right in the middle and at the usual grid distance (such that the polygon will be equilateral)

	Vertex & a2 = vertices[ve[_a]]; // refresh reference ('a' can be broken after adding vertex because of vector resize, "a = vertices[ve[_a]]" fails because you can't reset refs)
	Vertex & b2 = vertices[ve[_b]]; // refresh also
	Vertex & c = vertices[ve[_c]];
	if(addPolygon(a2,b2,c)) // add the polygon. It may already exist but then this call is just ignored.
	{
//		if(polygons.size() > 64690) std::cout << " createFlatLayerPolgyon() : a.pos = "<<a.pos<<", b.pos="<<b.pos<<", ab="<<ab<<std::endl; 
		// check whether polygon added has ab as a horizontal line (note that in this case b.z < a.z in this case because of clockwise-ness) or is a \ side (note the xor):
		if( a2.pos.z > b2.pos.z + 0.9*length(ab) || ( (b2.pos.x > a2.pos.x) != (b2.pos.z > a2.pos.z) ))
		{ plist.push_back( VertPair(a2.index, c.index) ); plist.push_back( VertPair(c.index, b2.index) ); }
		else
		{
			if(a2.pos.z > b2.pos.z) plist.push_back( VertPair(a2.index, c.index) ); // the to-the-left-of (/) case: add to the top
			else plist.push_back( VertPair(c.index, b2.index) ); // the to-the-right-of (/) case: add to the right
		}
//		if( polygons.size()>64690)
		if( (polygons.size()%1000)==0)
			std::cout << " Added "<<polygons.size()<<" polygons so far. "<<std::endl;
	}
}

void Bundle::createFlatLayer(float _size, unsigned int ndivs, float height)
{
	scaleTexture = _size;
	float step = scaleTexture/ndivs;
	float xstart = floor(scaleTexture/(2*step*sqrt(0.75)))*(step*sqrt(0.75));
	Vertex v1(-xstart, height, -scaleTexture/2);
	Vertex v2(-xstart, height, -scaleTexture/2 + step);
	xVert b = addVertex(v1);
	xVert a = addVertex(v2);
//	printLists();

	std::deque<VertPair> plist;
	plist.push_back( VertPair(a,b) );
	while(plist.size() > 0)
	{
		createFlatLayerPolygon(plist, plist.front().a, plist.front().b, 1.00001*scaleTexture/2, step);
		plist.pop_front();
		if(polygons.size() > 10 * ndivs * ndivs)
		{
			std::cerr << " Warning : createFlatLayer() : Too many polygons are getting created, stopping prematurely. "<<std::endl;
			break;
		}
	}
	assert(checkVertexIndices());
//	printLists();
	std::cout << " Finished creating a flat layer with "<<vertices.size()<<" vertices and "<<polygons.size()<<" polygons, using "<<polyAttempts<<" attempts. "<<std::endl;
}

void Bundle::splitUpdateAdjacentStrips(std::map<xVert, xVert> & vmap, Bundle * newBundle)
{
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
	{
		if(adjacentStrips[i]->updateAdjacentBundle(vmap, this, newBundle))
			newBundle->addAdjacentStrip(adjacentStrips[i]);
	}
}

/** Split a Bundle into two parts. The splitting is done such that each vertex is assigned to the member
  * of farthestPair that it can reach in the smallest number of steps. */
void Bundle::split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip)
{
//	std::cout << " Bundle::split() : Preparing to split the following mesh: "<<std::endl; printLists();
	Bundle * f = 0;
	Bundle * g = 0;
	std::map<xVert, xVert> fvert, gvert; // Mapping with key = old xVert and value = new xVert
	splitMesh(makeNewBundle, f, g, fvert, gvert);
	if(f==0 || g==0) { std::cout << " Bundle::split() : Bundles do not exist, splitting aborted. "<<std::endl; return; }

	// TODO: Write code to swap vertices between bundles in order to fix leftover vertices at the end of a stitch (i.e. situations
	// where a vertex is part of only 1 polygon from the original bundle, and the other 2 vertices of the polygon are not in the
	// same bundle).
	splitMergeOrphanVertices(f,g,fvert,gvert);

	if(vertices.size()+1 > f->vertices.size() + g->vertices.size())
	{
		std::cout << " Bundle::split() : ERROR: Not all vertices were assigned a Bundle! "<<std::endl;
		std::cout << " Mappings fvert = "; for(std::map<xVert, xVert>::iterator it = fvert.begin(); it != fvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
		std::cout << " Mappings gvert = "; for(std::map<xVert, xVert>::iterator it = gvert.begin(); it != gvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
	}

	Strip * s = makeNewStrip(); s->setParentLayer(parentLayer);
	splitAssignPolygonsToConstituentMeshes(f,g,s,fvert,gvert);

	std::cout << " Split mesh into bundles with "<<f->polygons.size()<<" and "<<g->polygons.size()<<" polys and a strip with "<<s->nPolys()<<" polys. "<<std::endl;

	f->setScaleFactor(scaleTexture);
	g->setScaleFactor(scaleTexture);
	s->setScaleFactor(scaleTexture);

	f->initMesh();
	g->initMesh();
	s->initMesh();
	s->resetTexture(scaleTexture, 250, 200, 0);

	splitUpdateAdjacentStrips(fvert, f);
	splitUpdateAdjacentStrips(gvert, g);

//	f->addAdjacentMesh(s);
//	g->addAdjacentMesh(s);
//	s->addAdjacentMesh(f);
//	s->addAdjacentMesh(g);

//	duplicateAdjacentMeshes(f);

//	updateVerticesInAdjacentMeshes(fvert, getKey(), f->getKey(), f);
//	updateVerticesInAdjacentMeshes(gvert, getKey(), g->getKey(), g);
}

Bundle::~Bundle(void)
{
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
		assert(adjacentStrips[i]->releaseAdjacentBundle(this));
}

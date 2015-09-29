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
	scale = _size;
	float step = scale/ndivs;
	float xstart = floor(scale/(2*step*sqrt(0.75)))*(step*sqrt(0.75));
	Vertex v1(-xstart, height, -scale/2);
	Vertex v2(-xstart, height, -scale/2 + step);
	xVert b = addVertex(v1);
	xVert a = addVertex(v2);
//	printLists();

	std::deque<VertPair> plist;
	plist.push_back( VertPair(a,b) );
	while(plist.size() > 0)
	{
		createFlatLayerPolygon(plist, plist.front().a, plist.front().b, 1.00001*scale/2, step);
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

/** Add a new vertex provided that it has not yet been assigned a new Bundle.
  *
  * Parameters:
  * - w             : the Vertex whose neighbours are to be considered as new members of Bundle 'b'
  * - b             : the Bundle to which this function should add new vertices
  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
  */
void Bundle::splitAddIfNewVertex(const xVert & w, Bundle * b, std::vector<xVert> & newVertices,
		std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices)
{
//	std::cout << " Bundle::splitAddIfNewVertex() : attempt to add "<<w<<"..."<<std::endl;
	if( addedVertices.count(w) == 0 && otherVertices.count(w) == 0 )
	{
//		std::cout << " Bundle::splitAddIfNewVertex() : Added vertex "<<w<<" to Bundle "<<b->getKey()<<std::endl;
		newVertices.push_back(w);
		addedVertices.insert( std::make_pair(w, b->addVertex(vertices[ve[w]]) ) ); // add vertex to the mapping of b's vertices
	}
}

/** Find new vertices to be added to a Bundle under construction, during the process of splitting an existing Bundle.
  *
  * Parameters:
  * - oldVertices   : list of vertices of the original Bundle that were most recently added to Bundle 'b'
  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
  * - b             : the Bundle to which this function should add new vertices
  */
void Bundle::splitAddNewVertices(const std::vector<xVert> & oldVertices, std::vector<xVert> & newVertices,
		std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices, Bundle * b)
{
	for(unsigned int i = 0; i < oldVertices.size(); i++)
	{
		Vertex & v = vertices[ve[oldVertices[i]]];
		xVert w = 0;
		for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
		{
			if(v.poly[j] == 0) break;
			assert(v.poly[j] < po.size());
			assert(po[v.poly[j]] < polygons.size());
			w = findPolyNeighbor(polygons[po[v.poly[j]]], oldVertices[i], true);
			splitAddIfNewVertex(w, b, newVertices, addedVertices, otherVertices);
			assert(v.poly[j] < po.size());
			assert(po[v.poly[j]] < polygons.size());
			w = findPolyNeighbor(polygons[po[v.poly[j]]], oldVertices[i], false);
			splitAddIfNewVertex(w, b, newVertices, addedVertices, otherVertices);
		}
	}
}

/** Split a Bundle into two parts. The splitting is done such that each vertex is assigned to the member
  * of farthestPair that it can reach in the smallest number of steps. */
void Bundle::split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip)
{
//	std::cout << " Bundle::split() : Preparing to split the following mesh: "<<std::endl; printLists();
	VertPair farthestPair(0,0);
	findFarthestPair(farthestPair);
	Bundle * f = makeNewBundle();
	Bundle * g = makeNewBundle();
	Strip * s = makeNewStrip();

	std::map<xVert, xVert> fvert, gvert; // Mapping with key = old xVert and value = new xVert
	xVert v;

	v = f->addVertex(getVertexPosition(farthestPair.a)); fvert.emplace(farthestPair.a, v);
	v = g->addVertex(getVertexPosition(farthestPair.b)); gvert.emplace(farthestPair.b, v);

	std::vector<xVert> fOldVertices, fNewVertices, gOldVertices, gNewVertices;
	fOldVertices.push_back(farthestPair.a);
	gOldVertices.push_back(farthestPair.b);
	while(fOldVertices.size() > 0 || gOldVertices.size() > 0)
	{
		splitAddNewVertices(fOldVertices, fNewVertices, fvert, gvert, f);
		splitAddNewVertices(gOldVertices, gNewVertices, gvert, fvert, g);
		fOldVertices.swap(fNewVertices);
		gOldVertices.swap(gNewVertices);
		fNewVertices.clear();
		gNewVertices.clear();
	}
//	std::cout << " Mappings fvert = "; for(std::map<xVert, xVert>::iterator it = fvert.begin(); it != fvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
//	std::cout << " Mappings gvert = "; for(std::map<xVert, xVert>::iterator it = gvert.begin(); it != gvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
	for(unsigned int i = 1; i < polygons.size(); i++)
	{
		xVert a = polygons[i].a;
		xVert b = polygons[i].b;
		xVert c = polygons[i].c;
		// Add polygon to the correct object.
		if(gvert.find(a) == gvert.end() && gvert.find(b) == gvert.end() && gvert.find(c) == gvert.end()) // None of the vertices are in Bundle g? Then this polygon is in f.
		{
			try
			{
				assert(fvert.at(a) < f->ve.size());
				assert(fvert.at(b) < f->ve.size());
				assert(fvert.at(c) < f->ve.size());
				assert(f->ve[fvert.at(a)] < f->vertices.size());
				assert(f->ve[fvert.at(b)] < f->vertices.size());
				assert(f->ve[fvert.at(c)] < f->vertices.size());
			}
			catch(std::exception &e)
			{
				std::cout << " Bundle::split() : Exception: "<<e.what()<<" with a->"<<a<<" b->"<<b<<" c->"<<c<<" on ve of size "<<ve.size()<<std::endl;
			}
//			std::cout << " Bundle::split() : f has "<<f->polygons.size()<<" polys and an index array of size "<<f->po.size()<<std::endl;
			f->addPolygon(f->vertices[f->ve[fvert.at(a)]], f->vertices[f->ve[fvert.at(b)]], f->vertices[f->ve[fvert.at(c)]]);
		}
		else if(fvert.find(a) == fvert.end() && fvert.find(b) == fvert.end() && fvert.find(c) == fvert.end()) // None of the vertices are in Bundle f? Then this polygon is in g.
		{
			try
			{
				assert(gvert.at(a) < g->ve.size());
				assert(gvert.at(b) < g->ve.size());
				assert(gvert.at(c) < g->ve.size());
				assert(g->ve[gvert.at(a)] < g->vertices.size());
				assert(g->ve[gvert.at(b)] < g->vertices.size());
				assert(g->ve[gvert.at(c)] < g->vertices.size());
			}
			catch(std::exception &e)
			{
				std::cout << " Bundle::split() : Exception: "<<e.what()<<" with a->"<<a<<" b->"<<b<<" c->"<<c<<" on ve of size "<<ve.size()<<std::endl;
			}
//			std::cout << " Bundle::split() : g has "<<g->polygons.size()<<" polys and an index array of size "<<g->po.size()<<std::endl;
			g->addPolygon(g->vertices[g->ve[gvert.at(a)]], g->vertices[g->ve[gvert.at(b)]], g->vertices[g->ve[gvert.at(c)]]);
		}
		else
		{
			// TODO: Make stitch from the remaining polygons.
		}
	}
}

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

#include "strip.hpp"

using namespace strata::mesh;

/** Add a new vertex provided that it has not yet been assigned a new Strip.
  *
  * Parameters:
  * - w             : the Vertex whose neighbours are to be considered as new members of Bundle 'b'
  * - s             : the Strip to which this function should add new vertices
  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
  */
/*void Bundle::splitAddIfNewVertex(const xVert & w, Strip * s, std::vector<xVert> & newVertices,
		std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices)
{
//	std::cout << " Bundle::splitAddIfNewVertex() : attempt to add "<<w<<"..."<<std::endl;
	if( addedVertices.count(w) == 0 && otherVertices.count(w) == 0)
	{
//		std::cout << " Bundle::splitAddIfNewVertex() : Added vertex "<<w<<" to Bundle "<<b->getKey()<<std::endl;
		newVertices.push_back(w);
		addedVertices.insert( std::make_pair(w, s->addVertex(vertices[ve[w]]) ) ); // add vertex to the mapping of b's vertices
	}
}*/

/** Find new vertices to be added to a Bundle under construction, during the process of splitting an existing Bundle.
  *
  * Parameters:
  * - oldVertices   : list of vertices of the original Bundle that were most recently added to Bundle 'b'
  * - newVertices   : list of vertices newly added to Bundle 'b', to be filled by this function
  * - addedVertices : map of vertices from the original Bundle that already became members of the new Bundle 'b' (every vertex only should have 1 new Bundle)
  * - otherVertices : list of vertices from the original Bundle that became members of a new Bundle other than 'b'
  * - b             : the Bundle to which this function should add new vertices
  */
/*void Strip::splitAddNewVertices(const std::vector<xVert> & oldVertices, std::vector<xVert> & newVertices,
		std::map<xVert, xVert> & addedVertices, const std::map<xVert, xVert> & otherVertices, Strip * s)
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
			if(splitVertexHasConnectedPolygon(w, addedVertices))
				splitAddIfNewVertex(w, s, newVertices, addedVertices, otherVertices);
			assert(v.poly[j] < po.size());
			assert(po[v.poly[j]] < polygons.size());
			w = findPolyNeighbor(polygons[po[v.poly[j]]], oldVertices[i], false);
			if(splitVertexHasConnectedPolygon(w, addedVertices))
				splitAddIfNewVertex(w, s, newVertices, addedVertices, otherVertices);
		}
	}
}*/

/** Split the Strip object. This function is roughly similar to the Bundle implementation for splitting Bundles,
  * but it is simpler since the Strip is only split into two Strips that are each roughly half as long as the
  * original Strip. */
void Strip::split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip)
{
/*	std::cout << " Strip::split() : Preparing to split the following mesh: "<<std::endl; printLists();
	VertPair farthestPair(0,0);
	findFarthestPair(farthestPair);

	// Check that the farthest pair vertices are not part of the same polygon (by checking that b is not part of any of a's polygons),
	// and that they also are not connected to the same vertex.
	if( verticesHaveCommonNeighbor(farthestPair.a, farthestPair.b) )
	{
		std::cout << " Bundle::split() : Farthest pair vertices seem to be members of the same (very large) polygon. Cannot split! "<<std::endl;
		return;
	}

	Strip * k = makeNewStrip(); k->setParentLayer(parentLayer);
	Strip * m = makeNewStrip(); m->setParentLayer(parentLayer);

	std::map<xVert, xVert> kvert, mvert; // Mapping with key = old xVert and value = new xVert
	xVert v;

	v = k->addVertex(getVertexPosition(farthestPair.a)); kvert.emplace(farthestPair.a, v);
	v = m->addVertex(getVertexPosition(farthestPair.b)); mvert.emplace(farthestPair.b, v);

	std::vector<xVert> kOldVertices, kNewVertices, mOldVertices, mNewVertices;
	kOldVertices.push_back(farthestPair.a);
	mOldVertices.push_back(farthestPair.b);

	// First add all the neighbours of the initial vertex, while avoiding the usual check that it is well-connected to the Bundle.
	// That check only works well if there is at least 1 edge already present in the Bundle.
	for(unsigned int i = 0; i < STRATA_VERTEX_MAX_LINKS; i++)
	{
		if(vertices[ve[farthestPair.a]].poly[i] != 0)
		{
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].a, k, kNewVertices, kvert, mvert);
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].b, k, kNewVertices, kvert, mvert);
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.a]].poly[i]]].c, k, kNewVertices, kvert, mvert);
		}
		if(vertices[ve[farthestPair.b]].poly[i] != 0)
		{
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].a, m, mNewVertices, mvert, kvert);
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].b, m, mNewVertices, mvert, kvert);
			splitAddIfNewVertex(polygons[po[vertices[ve[farthestPair.b]].poly[i]]].c, m, mNewVertices, mvert, kvert);
		}
	}*/
}

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
#include "layer.hpp"

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

/** Update the adjacent strips of the Bundle so that they replace their remote indices as specified
  * by the 'vmap' mapping. This function is called when the Bundle is split into two Bundles, where
  * newBundle is one of the resulting Bundles.
  * Since the current Bundle will cease to exist after the split, it needs the Strips to transfer their
  * references to the new Bundle. The 'vmap' mapping therefore consists of pairs (oldIndex, newIndex)
  * and the Strip simply replaces every reference to an oldIndex by a corresponding newIndex. */
void Bundle::splitUpdateAdjacentStrips(std::map<xVert, xVert> & vmap, Bundle * newBundle)
{
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
	{
		if(adjacentStrips[i]->updateAdjacentBundle(vmap, this, newBundle))
		{
//			std::cout << " Bundle::splitUpdateAdjacentStrips() : Adjacent strip "<<adjacentStrips[i]<<" ("<<i<<"/"<<adjacentStrips.size()-1<<") uses Bundle "<<newBundle<<std::endl;
			newBundle->addAdjacentStrip(adjacentStrips[i]);
		}
	}
}

/** While splitting, assign vertices to a bundle in the problematic case that the vertex is isolated
  * on a 'spike' where it is connected via only 1 edge to a vertex in Bundle f and via 1 other edge to a
  * vertex in Bundle g.
  *
  * If this is the case, we split the edge between the f and g bundle such that 1 vertex is added and
  * the two polygons adjacent to this edge are replaced by four new ones.
  *
  * The procedure for splitting an edge is standard and delegated to the Mesh class. This function itself therefore
  * has to identify the local topology and determine where the operation should take place. */
bool Bundle::splitAssignSpikeVertices(Bundle * f, Bundle * g, std::map<xVert, xVert> &fvert, std::map<xVert, xVert> &gvert)
{
	bool allVerticesAreAssigned = true;
	for(unsigned int i = 1; i < vertices.size(); i++)
	{
		if(fvert.find(vertices[i].index) == fvert.end() && gvert.find(vertices[i].index) == gvert.end())
		{
			std::cout << " Bundle::splitAssignSpikeVertices() : Attempting to assign previously unassigned vertex "<<vertices[i].index<<"..."<<std::endl;
			for(unsigned int j = 0; j < STRATA_VERTEX_MAX_LINKS; j++)
			{
				if(vertices[i].poly[j] == 0)
				{
					// Stop if all vertices are exhausted (this will leave the vertex unassigned, requiring yet another
					// call to this function).
					allVerticesAreAssigned = false;
					break;
				}
				if((fvert.find(findPolyNeighbor(j, vertices[i].index, true)) != fvert.end() ||
					fvert.find(findPolyNeighbor(j, vertices[i].index, false)) != fvert.end()) &&
				   (gvert.find(findPolyNeighbor(j, vertices[i].index, true)) != gvert.end() ||
					gvert.find(findPolyNeighbor(j, vertices[i].index, false)) != gvert.end()) )
				{
					// Now we found the polygon for which the unassigned vertex is together with 1 vertex
					// from Bundle 'f' and 1 vertex from Bundle 'g'. We are going to cut the f-g edge in
					// order to solve the unassignability problem. The other polygon that the f-g edge is part of,
					// must have already been assigned to either 'f' or 'g'. (This should be guaranteed for well-connected meshes.)
					// The new vertex and the unassigned vertex will both be assigned to the same group as that vertex.

					// Split the edge opposite to the spike vertex.
					std::cout << " Bundle::splitAssignSpikeVertices() : Splitting edge for unassigned vertex "<<vertices[i].index<<"..."<<std::endl;
					splitEdge(findPolyNeighbor(j, vertices[i].index, true), findPolyNeighbor(j, vertices[i].index, false));

					// Immediately retry adding vertices to meshes. Both the new vertex and the i-th vertex should now be added.
					// If we would postpone adding these, we risk trying to manipulate the topology even more while it's not necessary.
					splitAssignOrphanVertices(f,g,fvert,gvert); // Set to false to signal that we're not done.
					if(fvert.find(vertices[i].index) == fvert.end() && gvert.find(vertices[i].index) == gvert.end())
					{
						std::cout << " Bundle::splitAssignSpikeVertices() : WARNING: Failed to assign "<<vertices[i].index<<"!"<<std::endl;
						allVerticesAreAssigned = false;
					}
					break;
				}
			}
		}
	}
	return allVerticesAreAssigned;
}

/** Split a Bundle into two parts. The splitting is done such that each vertex is assigned to the member
  * of farthestPair that it can reach in the smallest number of steps. */
bool Bundle::split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip)
{
//	std::cout << " Bundle::split() : Preparing to split the following mesh: "<<std::endl; printLists();
	Bundle * f = 0;
	Bundle * g = 0;
	std::map<xVert, xVert> fvert, gvert; // Mapping with key = old xVert and value = new xVert

	// Rebalance the mesh, such that vertices have space in their 'poly' array of polygons they are part of.
	// Splitting may run into issues that are then resolved by modifying the mesh, but this modification is
	// considerably easier if polygons can be split and new vertices can be introduced.
	// If splitting does encounter a non-recoverable problem (usually, a full 'poly' array) it will abort,
	// but with this rebalancing such aborted splits should be extremely rare (since a single vertex would
	// need to receive more than 3 additional connections, which would require a very peculiar mesh (one can
	// manually construct such examples).)
	rebalanceVertexConnections();

	// Split the mesh's vertices, assigning all vertices to either 'f' or 'g'. The splitting may fail if
	// the mesh has too few polygons, in this case f and g are not created and we abort the splitting.
	splitMesh(makeNewBundle, f, g, fvert, gvert);
	if(f==0 || g==0) { std::cout << " Bundle::split() : Bundles do not exist, splitting aborted. "<<std::endl; return false; }

	// Assign any vertices not yet in f or g to either f or g. Since the assignment failed during splitMesh,
	// it will be necessary to modify the mesh such that the resulting meshes f and g will be well-connected.
	// This modification can add or merge vertices, but it is not allowed to delete vertices or change the polygon
	// topology (at least of polyogns that are entirely contained in either f or g), since this could cause
	// the vertex sets 'f' and 'g' to result in improperly connected meshes.
	splitAssignSpikeVertices(f,g,fvert,gvert);

	if(vertices.size()+1 > f->vertices.size() + g->vertices.size())
	{
		std::cout << " Bundle::split() : ERROR: Not all vertices were assigned a Bundle! "<<std::endl;
		std::cout << " Mappings fvert = "; for(std::map<xVert, xVert>::iterator it = fvert.begin(); it != fvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
		std::cout << " Mappings gvert = "; for(std::map<xVert, xVert>::iterator it = gvert.begin(); it != gvert.end(); it++) std::cout <<" "<<it->first<<"->"<<it->second; std::cout << std::endl;
	}

	Strip * s = makeNewStrip();
	splitAssignPolygonsToConstituentMeshes(f,g,s,fvert,gvert);

	// Copy texture scaling.
	f->setScaleFactor(scaleTexture);
	g->setScaleFactor(scaleTexture);
	s->setScaleFactor(scaleTexture);

	parentLayer->addBundle(f);
	parentLayer->addBundle(g);

	f->setParentLayer(parentLayer);
	g->setParentLayer(parentLayer);
	s->setParentLayer(parentLayer);

	// Initialize rendered objects for the new meshes.
	f->resetTexture(parentLayer->getBundleTexture());
	g->resetTexture(parentLayer->getBundleTexture());
	s->resetTexture(parentLayer->getStripTexture());

	// Make strips adjacent to the old Bundle update their adjacency to include the new Bundle objects.
	addAdjacentStrip(s); // Add the newly created strip as an adjacent strip (so that it will become linked to f and g in the following lines)
	s->addAdjacentBundle(this); // Also add reverse link to avoid a crash when the Bundle is deleted
	splitUpdateAdjacentStrips(fvert, f);
	splitUpdateAdjacentStrips(gvert, g);

	return true;
}

void Bundle::duplicateBundle(Bundle * b) const
{
	if(b->vertices.size() > 1 || b->polygons.size() > 1)
	{
		std::cout << " Bundle::duplicateBundle() : ERROR: Cannot duplicate, target Bundle already contains vertices and/or polygons! "<<std::endl;
		return;
	}
	duplicateMesh(b);
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
		b->addAdjacentStrip(adjacentStrips[i]);
}

/** Check the correctness of adjacency tracking of the Bundle.
  * This checks the following:
  * - All Strip objects in adjacentStrips have a reverse reference to this Bundle.
  * It cannot check whether all edge vertices are in a strip, because meshes can well be at the very edge of the entire Terrain.
  * It also doesn't check whether Strips exist outside of its adjacentStrips array that still refer to the Bundle's vertices.
  * The latter is up to the Strip to check.
  */
bool Bundle::checkAdjacentMeshes(void) const
{
	bool adjacentMeshesAreComplete = true;
	if(getParentLayer() == 0)
	{
		std::cout << " Bundle::checkAdjacentMeshes() : Parent layer not set! "<<std::endl;
		adjacentMeshesAreComplete = false;
	}
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
	{
		if(!adjacentStrips[i]->isAdjacentToBundle(this))
		{
			std::cout << " Bundle::checkAdjacentMeshes() : Strip does not contain a reverse reference to Bundle! "<<std::endl;
			adjacentMeshesAreComplete = false;
		}
	}
	return adjacentMeshesAreComplete;
}

Bundle::~Bundle(void)
{
	for(unsigned int i = 0; i < adjacentStrips.size(); i++)
		assert(adjacentStrips[i]->releaseAdjacentBundle(this));
	if(parentLayer)
		parentLayer->releaseBundle(this);
	else std::cout << " Bundle::~Bundle() : WARNING: No parentLayer found, cannot release Bundle from Layer! "<<std::endl;
}

bool Bundle::findVertexAtLayerEdge(xVert &index) const
{
//	return false;
	for(unsigned int i = 1; i < vertices.size(); i++)
	{
		if(isAtLayerEdge(vertices[i].index))
		{
//			std::cout << " Bundle::findVertexAtLayerEdge() : Found edge at "<<vertices[i].pos<<"!"<<std::endl;
			index = vertices[i].index;
			return true;
		}
//		else std::cout << " Bundle::findVertexAtLayerEdge() : Vertex "<<i<<" at "<<vertices[i].pos<<" is not at edge! "<<std::endl;
	}
	return false;
}

/** Check whether the vertex with index 'v' is at the Layer's edge.
  * The check has two steps: first we check if the vertex is an edge vertex under the
  * usual definition of the TopologicalMesh. If not, it cannot be on the Layer's edge
  * either. If it is, we do a second check, namely that we can complete a circle around
  * the current Vertex by finding neighbours until we reach the start again.
  * If this procedure breaks, it means such a circle doesn't exist and we are indeed at
  * the Layer's edge.
  * The strategy for this second step is analogous to the non-edge-vertex procedure in
  * TopologicalMesh::checkTopology().
  */
bool Bundle::isAtLayerEdge(xVert v) const
{
	if(!isEdgeVertex(v)) return false;
	else
	{
		xVert startIndex = findAdjacentEdgeVertex(v, false);
		xVert endIndex = findAdjacentEdgeVertex(v, true);
//		std::cout << " Neighbor vertices are at "<<vertices[ve[startIndex]].pos<<" and "<<vertices[ve[endIndex]].pos<<std::endl;
		// Now try to find polygons via which we can move clockwise around 'v'
		// from startIndex to endIndex.
		xVert neighborIndex = startIndex;
		const Bundle * neighborBundle = this;
		xVert nextIndex = 0;
		const Bundle * nextBundle = 0;
		while(neighborIndex != endIndex || neighborBundle != this)
		{
			// Try to find neighborIndex from all nearby Strips.
			for(unsigned int i = 0; i < adjacentStrips.size(); i++)
			{
				// Try finding (nextIndex, nextBundle) using the pairs (v, this)
				// and (neighborIndex, neighborBundle) to uniquely fix the target.
				nextIndex = adjacentStrips[i]->findRemoteVertexPolyNeighbor(
						nextBundle, v, neighborIndex, this, neighborBundle, true);
				if(nextIndex > 0)
				{
//					std::cout << " Bundle::isAtLayerEdge() : Found next vertex in Strip! "<<std::endl;
					neighborIndex = nextIndex;
					neighborBundle = nextBundle;
					break; // Break loop over strips, continue 'while'
				}
//				else if(i+1==adjacentStrips.size()) std::cout << " Bundle::isAtLayerEdge() : No next vertex! "<<std::endl;
			}
			if(nextIndex == 0) break; // Failed to find next vertex - this is the edge
		}
		return (neighborIndex != endIndex);
	}
}

/** Check whether the position 'p' is 'near' the Bundle's mesh. This function does most of
  * the work for isAboveMeshAtIndex() and isBelowMeshAtIndex(). See the former for a detailed
  * functional description. */
bool Bundle::isNearMeshAtIndex(xVert v, tiny::vec3 p, bool isAlongNormal)
{
	// TODO: Write function body, using inner products on normals of neighboring vertices
	// to determine whether the position 'p' is near the mesh in the required way.
	return false;
}

/** Check whether the position 'p' is strictly above the mesh defined by the neighborhood
  * of the vertex with index 'v'.
  * More precisely, this function looks whether 'p' is contained within the cone formed by
  * the normals of all of v's neighbors.
  * In the special case that 'v' happens to be an edge vertex, every 'p' with a positive
  * inner product with v's normal and being 'in between' the two mesh edges that v is
  * connected to, will also be considered as 'above'.
  * In all remaining cases, 'p' is either under or next to the mesh, and is thus considered
  * 'not above'. */
bool Bundle::isAboveMeshAtIndex(xVert v, tiny::vec3 p)
{
	return isNearMeshAtIndex(v, p, true);
}

/** As isAboveMeshAtIndex() except that the region opposite to the normal at 'v' is
  * considered. */
bool Bundle::isBelowMeshAtIndex(xVert v, tiny::vec3 p)
{
	return isNearMeshAtIndex(v, p, false);
}



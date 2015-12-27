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
#pragma once

#include <vector>
#include <deque>

#include <tiny/math/vec.h>
#include <tiny/algo/typecluster.h>

#include "vecmath.hpp"
#include "mesh.hpp"

namespace strata
{
	namespace mesh
	{
		class Strip;

		/** A Bundle is a mesh, consisting of vertices and polygons that link the vertices together. Some helper functions
		  * are never required as 'outside' functions, therefore they are hidden (as opposed to some direct mesh alteration
		  * functions which are made publicly available to aid smoothly and effectively carrying out mesh manipulations).
		  *
		  * Bundles are identified by their key of the TypeCluster, which is globally unique ('globally' in the scope of
		  * the Terrain object). This key is used to find the owning Bundle of a Vertex.
		  */
		class Bundle : public tiny::algo::TypeClusterObject<long unsigned int, Bundle>, public Mesh<Vertex>
		{
			private:
				friend class Mesh<Vertex>; /**< Our base class can add vertices to us. */

				std::vector<Strip*> adjacentStrips; /**< A list of all Strips that use vertices belonging to this Bundle. */

				long unsigned int polyAttempts;

				bool splitVertexHasConnectedPolygon(const xVert &w, const std::map<xVert, xVert> & addedVertices) const;

				/** Update the adjacent strips to refer to the new Bundle instead of 'this'. */
				void splitUpdateAdjacentStrips(std::map<xVert, xVert> & vmap, Bundle * newBundle);

				/** Assign a thus-far-unassigned spike vertex with index 'v' to the mapping 'vmap' by splitting the edge
				  * opposite to it on its i-th polygon. */
				void splitAssignSpikeVertexToNewBundle(Bundle * f, unsigned int i, const xVert &v, std::map<xVert, xVert> &vmap);

				/** Assign so-far-not-connected vertices by adding a new vertex along one of its connected edges and then
				  * redoing the assignment. */
				bool splitAssignSpikeVertices(Bundle * f, Bundle * g, std::map<xVert, xVert> &fvert, std::map<xVert, xVert> &gvert);

				virtual xVert addVertex(const Vertex &v) { return Mesh<Vertex>::addVertex(v); }
				xVert addVertex(tiny::vec3 &p) { return addVertex( Vertex(p) ); }
				xVert addVertex(float x, float y, float z) { return addVertex( Vertex(tiny::vec3(x,y,z)) ); }
			public:
				unsigned int usedMemory(void) const
				{
					return vertices.size()*sizeof(Vertex) + polygons.size()*sizeof(Polygon)
						+ ve.size()*sizeof(xVert) + po.size()*sizeof(xPoly);
				}

				unsigned int numberOfVertices(void) const
				{
					return vertices.size();
				}

				using tiny::algo::TypeClusterObject<long unsigned int, Bundle>::getKey;

				/** Get the owning bundle of a Vertex. Since Bundles are always owner of vertices
				  * belonging to them, there is no other possibility than 'this' Checks that the vertex
				  * is really in this Bundle are not performed. */
				virtual Bundle * getVertexOwner(const xVert &) { return this; }

				/** Get the vertex index of a vertex in its owning Bundle. Since 'this' owns all of the verticese
				  * indexed by it, the vertex index itself is returned. */
				virtual xVert getRemoteVertexIndex(const xVert & v) { return v; }

				Bundle(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Bundle> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Bundle>(meshId, this, tc),
					Mesh<Vertex>(_renderer),
					polyAttempts(0)
				{
				}

				/** Duplicate the Bundle, making 'b' a copy of itself. The duplication will be rejected if the
				  * Bundle 'b' already has some vertices in it.
				  * The duplication copies all vertices and polygons, resulting in a mesh with identical shape and structure.
				  * It also copies the adjacent strips list, but since the references will be the same as these for the
				  * current Bundle they may need to be adjusted later on (when the adjacent Strips themselves have also been
				  * duplicated).
				  */
				void duplicateBundle(Bundle * b) const;

				void duplicateAdjustAdjacentStrips(std::map<Strip*, Strip*> &smap)
				{
					for(unsigned int i = 0; i < adjacentStrips.size(); i++)
					{
						if(smap.find(adjacentStrips[i]) == smap.end())
							std::cout << " Strip::duplicateAdjustAdjacentBundles() : WARNING: Failed to find adjacent bundle in map! "<<std::endl;
						else adjacentStrips[i] = smap.at(adjacentStrips[i]);
					}
				}

				/** Add a Strip as being adjacent to this Bundle. */
				void addAdjacentStrip(Strip * strip)
				{
					for(unsigned int i = 0; i < adjacentStrips.size(); i++)
						if(adjacentStrips[i] == strip) return;
					adjacentStrips.push_back(strip);
				}

				bool isAdjacentToStrip(const Strip * strip) const
				{
					for(unsigned int i = 0; i< adjacentStrips.size(); i++)
						if(adjacentStrips[i] == strip)
							return true;
					return false;
				}
				/** Remove a Strip currently listed as adjacent, so that it can be removed safely. */
				bool releaseAdjacentStrip(Strip * strip)
				{
					for(unsigned int i = 0; i< adjacentStrips.size(); i++)
						if(adjacentStrips[i] == strip)
						{
							adjacentStrips[i] = adjacentStrips.back();
							adjacentStrips.pop_back();
							return true;
						}
					std::cout << " Bundle::releaseAdjacentStrip() : ERROR: Failed to find adjacent strip!"<<std::endl;
					return false;
				}

				virtual ~Bundle(void);

				/** Create a complete flat layer in this Bundle object. */
				void createFlatLayer(float _size, unsigned int ndivs, float height = 0.0f);

				/** Add a polygon (in the shape of an equilateral triangle) to this Bundle, using the edge _a-_b. Use an edge size
				  * 'step' (this is equal to the length of the _a-_b edge but is used instead of this length to avoid numerical
				  * noise). Do not add if the position of the third vertex is farther from the origin than 'limit'. */
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);

				/** Split a layer into pieces. This creates two new layers from the old one, and finishes by deleting the original layer. */
				virtual bool split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip);

				/** Check the adjacent strips to this Bundle. */
				virtual bool checkAdjacentMeshes(void) const;

				/** Provide a means to detect invalid vertex indices. */
				inline bool isValidVertexIndex(const xVert & _index) const { return (_index > 0 && _index < ve.size()); }
		};
	}
}

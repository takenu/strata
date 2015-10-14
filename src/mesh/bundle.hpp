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

		/** A mesh, consisting of vertices and polygons that link the vertices together. It is used intensely as a container object so data is public.
		  * However, since some helper functions are never required as outside functions, they are hidden (as opposed to the data).
		  *
		  * Bundles are identified by their key of the TypeCluster, which is globally unique ('globally' in the scope of
		  * the Terrain object). This key is used to find the owning Bundle of a Vertex.
		  */
		class Bundle : public tiny::algo::TypeClusterObject<long unsigned int, Bundle>, public Mesh<Vertex>
		{
			private:
				friend class Mesh<Vertex>; /**< Our base class can add vertices to us. */

				long unsigned int polyAttempts;

				virtual void purgeVertex(long unsigned int, const xVert &, const xVert &) {}

				bool splitVertexHasConnectedPolygon(const xVert &w, const std::map<xVert, xVert> & addedVertices) const;
//				void splitAddIfNewVertex(const xVert & w, Bundle * b, std::vector<xVert> & newVertices, std::map<xVert, xVert> & addedVertices,
//						const std::map<xVert, xVert> & otherVertices);
//				void splitAddNewVertices(const std::vector<xVert> & oldVertices, std::vector<xVert> & newVertices, std::map<xVert, xVert> & addedVertices,
//						const std::map<xVert, xVert> & otherVertices, Bundle * b);

				virtual bool isAdjacentToVertices(const Bundle *) const { return false; }

				virtual xVert addVertex(const Vertex &v) { return Mesh<Vertex>::addVertex(v); }
				xVert addVertex(tiny::vec3 &p) { return addVertex( Vertex(p) ); }
				xVert addVertex(float x, float y, float z) { return addVertex( Vertex(tiny::vec3(x,y,z)) ); }
			public:
				using tiny::algo::TypeClusterObject<long unsigned int, Bundle>::getKey;

				Bundle(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Bundle> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Bundle>(meshId, this, tc),
					Mesh<Vertex>(_renderer),
					polyAttempts(0)
				{
				}

				virtual long unsigned int getMeshFragmentId(void) const { return getKey(); }

				virtual bool updateRemoteVertexIndices(const std::map<xVert,xVert> &, long unsigned int, long unsigned int) { return false; }

				virtual ~Bundle(void) {}

				/** Create a complete flat layer in this Bundle object. */
				void createFlatLayer(float _size, unsigned int ndivs, float height = 0.0f);

				/** Add a polygon (in the shape of an equilateral triangle) to this Bundle, using the edge _a-_b. Use an edge size
				  * 'step' (this is equal to the length of the _a-_b edge but is used instead of this length to avoid numerical
				  * noise). Do not add if the position of the third vertex is farther from the origin than 'limit'. */
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);

				/** Split a layer into pieces. This creates two new layers from the old one, and finishes by deleting the original layer. */
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip);
		};
	}
}

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
		  */
		class Bundle : public tiny::algo::TypeClusterObject<long unsigned int, Bundle>, public Mesh<Vertex>
		{
			private:
				long unsigned int polyAttempts;

				virtual void purgeVertex(long unsigned int , xVert , xVert ) {}
			public:
				using tiny::algo::TypeClusterObject<long unsigned int, Bundle>::getKey;

				Bundle(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Bundle> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Bundle>(meshId, this, tc),
					Mesh<Vertex>(_renderer),
					polyAttempts(0)
				{
				}

				virtual ~Bundle(void) {}

				void createFlatLayer(float _size, unsigned int ndivs, float height = 0.0f);
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);

				/** Split a layer into pieces. This creates two new layers from the old one, and finishes by deleting the original layer. */
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip);
		};
	}
}

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
#include <tiny/draw/staticmesh.h>

#include "vecmath.hpp"
#include "element.hpp"

namespace strata
{
	namespace mesh
	{
		/** A mesh, consisting of vertices and polygons that link the vertices together. It is used intensely as a container object so data is public.
		  * However, since some helper functions are never required as outside functions, they are hidden (as opposed to the data).
		  */
		class Bundle : public Mesh<Vertex>
		{
			private:
				long unsigned int polyAttempts;
			public:
				/** At construction, add error values for polygons and vertices. */
				Bundle(void) :
					Mesh<Vertex>(),
					polyAttempts(0)
				{
					vertices.push_back( Vertex(0.0f, 0.0f, 0.0f) );
				}

				virtual ~Bundle(void) {}

				void createFlatLayer(float _size, unsigned int ndivs, float height = 0.0f);
				void createFlatLayerPolygon(std::deque<VertPair> &plist, xVert _a, xVert _b, float limit, float step);
		};
	}
}

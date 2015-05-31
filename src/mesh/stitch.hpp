
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

#include "element.hpp"

namespace strata
{
	namespace mesh
	{
		class ForeignVertex : public Vertex
		{
			public:
				long unsigned int mfid; /**< The id of the MeshFragment that owns this vertex. */

				ForeignVertex(tiny::vec3 _pos, xVert _vertex, long unsigned int _mfid) : Vertex(_pos), mfid(_mfid)
				{
					index = _vertex;
				}
		};

		/** A borrowed vertex from some MeshBundle. */
		class StitchVertex : public ForeignVertex
		{
			private:
			public:
				StitchVertex(tiny::vec3 _pos, xVert _vertex, long unsigned int _mfid) : ForeignVertex(_pos, _vertex, _mfid) {}
		};

		/** A polygon on StitchedVertices. */
		class StitchPolygon : public Polygon
		{
			private:
			public:
				StitchPolygon(xVert _a, xVert _b, xVert _c) : Polygon(_a,_b,_c)
				{
				}
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class MeshStitch : public Mesh<StitchVertex, StitchPolygon>
		{
			private:
			public:
				MeshStitch(void) { vertices.push_back( StitchVertex(tiny::vec3(0.0f,0.0f,0.0f),0,0) ); polygons.push_back( StitchPolygon(0,0,0) ); po.push_back(0); }

				/** Create a MeshStitch in order to connect meshbundle 'a' to 'b' on all vertices 'aVerts'. */
				void connectMeshes(MeshBundle &a, MeshBundle &b, std::vector<xVert> aVerts);
		};
	}
}

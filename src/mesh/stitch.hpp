
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
		/** A borrowed vertex from some MeshBundle. */
		class StitchVertex
		{
			private:
				xVert vertex; /**< The index of the vertex in its own MeshBundle. */
				xVert index; /**< The index of the vertex in the MeshStitch. */
				long unsigned int mfid; /**< The id of the MeshBundle that contains this vertex. */
			public:
				StitchVertex(xVert _vertex, long unsigned int _mfid) : vertex(_vertex), mfid(_mfid) {}
		};

		/** A polygon on StitchedVertices. */
		class StitchPolygon
		{
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class MeshStitch
		{
			private:
				std::vector<StitchVertex> vertices;
				std::vector<StitchPolygon> polygons;

				std::vector<xVert> ve;
				std::vector<xPoly> po;
			public:
		};
	}
}

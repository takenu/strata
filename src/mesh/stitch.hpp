
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
		class ForeignVertex
		{
			public:
				xVert vertex; /**< The index of the vertex in its own MeshBundle. */
				long unsigned int mfid; /**< The id of the MeshBundle that owns this  vertex. */

				ForeignVertex(xVert _vertex, long unsigned int _mfid) : vertex(_vertex), mfid(_mfid) {}
		};

		/** A borrowed vertex from some MeshBundle. */
		class StitchVertex : public ForeignVertex
		{
			private:
				xVert index; /**< The index of the vertex in the MeshStitch. */
			public:
				StitchVertex(xVert _vertex, long unsigned int _mfid) : ForeignVertex(_vertex, _mfid), index(0) {}
		};

		/** A polygon on StitchedVertices. */
		class StitchPolygon
		{
			private:
				xVert a; /**< The index of vertex a. Indices use numbering local to the MeshStitch object. */
				xVert b;
				xVert c;

				xPoly index; /**< The index of this polygon in the MeshStitch object. */
			public:
				StitchPolygon(xVert _a, xVert _b, xVert _c) : a(_a), b(_b), c(_c)
				{
				}
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class MeshStitch : public Mesh<StitchVertex, StitchPolygon>
		{
			private:
			public:
				MeshStitch(void) { vertices.push_back( StitchVertex(0,0) ); polygons.push_back( StitchPolygon(0,0,0) ); po.push_back(0); }

				/** Create a MeshStitch in order to connect meshbundle 'a' to 'b' on all vertices 'aVerts'. */
				void connectMeshes(MeshBundle &a, MeshBundle &b, std::vector<xVert> aVerts);
		};

		/** A class for stitching together stitch-meshes, using a single polygon. */
/*		class MeshStitchJunction
		{
			private:
				ForeignVertex a;
				ForeignVertex b;
				ForeignVertex c;
			public:
				MeshStitchJunction(const ForeignVertex & _a, const ForeignVertex & _b, const ForeignVertex & _c) : a(_a), b(_b), c(_c) {}
		};*/
	}
}

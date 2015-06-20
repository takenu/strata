
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

#include "element.hpp"

namespace strata
{
	namespace mesh
	{
		class Bundle;

		/** A borrowed vertex from some Bundle. */
		class StripVertex : public Vertex
		{
			private:
				xVert remoteIndex;
			public:
				long unsigned int mfid; /**< The id of the MeshFragment that owns this vertex. */
				StripVertex(tiny::vec3 _pos, xVert _vertex, long unsigned int _mfid) : Vertex(_pos), mfid(_mfid)
				{
					index = _vertex;
				}
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class Strip : public tiny::algo::TypeClusterObject<long unsigned int, Strip>, public Mesh<StripVertex>
		{
			private:
				virtual void purgeVertex(long unsigned int /*mfid*/, xVert /*oldVert*/, xVert /*newVert*/)
				{
				}
			public:
				Strip(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Strip> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Strip>(meshId, this, tc),
					Mesh<StripVertex>(_renderer)
				{
				}

				/** Create a Strip in order to connect meshbundle 'a' to 'b' on all vertices 'aVerts'. */
				void connectMeshes(Bundle &a, Bundle &b, std::vector<xVert> aVerts);

				/** For stitch meshes, use direct analysis to calculate shape (i.e. skip first finding the edge vertices) since all
				  * stitch vertices are already edge vertices. */
				virtual float findFarthestPair(VertPair &farthestPair)
				{
					return analyseShapeDirect(farthestPair);
				}

				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip)
				{
				}

				virtual float meshSize(void)
				{
					return size();
				}

		};
	}
}

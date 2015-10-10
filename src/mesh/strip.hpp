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

#include <tiny/math/vec.h>
#include <tiny/algo/typecluster.h>

#include "mesh.hpp"

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
				long unsigned int mfid; /**< The id of the MeshFragment that owns this vertex. */
			public:
				/** Create a StripVertex. Note that this does not set the 'index' field of the Vertex. */
				StripVertex(tiny::vec3 _pos, long unsigned int _mfid, xVert _remoteIndex) : Vertex(_pos), remoteIndex(_remoteIndex), mfid(_mfid)
				{
				}

				/** Allow construction from existing vertex plus mfid. Used when copying a Vertex from a Bundle into a Strip.
				  * After construction, will not yet have a valid index which must be set by the Strip creating it. */
				StripVertex(const Vertex &v, long unsigned int _mfid) : StripVertex(v.pos,  _mfid, v.index) {}

				/** Allow construction from existing strip vertex plus mfid.
				  * This duplicates the remoteIndex, and is used when making a copy of a StripVertex from a Strip for another Strip object. */
				StripVertex(const StripVertex &v, long unsigned int _mfid) : StripVertex(v.pos,  _mfid, v.remoteIndex) {}

				StripVertex(const StripVertex &v) : Vertex(v), remoteIndex(v.remoteIndex), mfid(v.mfid)
				{
				}

				const xVert & getRemoteIndex(void) const { return remoteIndex; }
				void setRemoteIndex(const xVert &v) { remoteIndex = v; }
		};

		/** A class for special stitch-meshes, which do not contain vertices but which are used to link together
		  * meshes that do have vertices. They thus contain polygons whose vertices belong to distinct meshes. */
		class Strip : public tiny::algo::TypeClusterObject<long unsigned int, Strip>, public Mesh<StripVertex>
		{
			private:
//				friend class Mesh<StripVertex>;
				friend class Mesh<Vertex>; // to give the Bundle access to our protected Mesh base functions

				virtual void purgeVertex(long unsigned int /*mfid*/, xVert /*oldVert*/, xVert /*newVert*/)
				{
				}
			protected:
			public:
				Strip(long unsigned int meshId, tiny::algo::TypeCluster<long unsigned int, Strip> &tc, core::intf::RenderInterface * _renderer) :
					tiny::algo::TypeClusterObject<long unsigned int, Strip>(meshId, this, tc),
					Mesh<StripVertex>(_renderer)
				{
				}

				unsigned int nPolys(void) const { return polygons.size(); }
				unsigned int nPolyIndices(void) const { return po.size(); }

//				virtual xVert addVertex(const StitchVertex &v)
//				{
//					return Mesh<StripVertex>::addVertex(v);
////					xVert _v = Mesh<StripVertex>::addVertex(v);
////					vertices[ve[_v]].setRemoteIndex(v.getRemoteIndex()); // Copy remote index of vertex in owner's mesh. (Shouldn't be necessary if copy constructor does what it is expected to do.)
//				}

				/** Create a Strip in order to connect meshbundle 'a' to 'b' on all vertices 'aVerts'. */
				void connectMeshes(Bundle &a, Bundle &b, std::vector<xVert> aVerts);

				/** For stitch meshes, use direct analysis to calculate shape (i.e. skip first finding the edge vertices) since all
				  * stitch vertices are already edge vertices. */
				virtual float findFarthestPair(VertPair &farthestPair)
				{
					return analyseShapeDirect(farthestPair);
				}

				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip);
		};
	}
}

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

#include "element.hpp"

namespace strata
{
	namespace mesh
	{
		class Bundle;

		/** A borrowed vertex from some Bundle. It is used for Strip meshes, and it
		  * contains a reference to the Bundle that owns the original Vertex as well
		  * as the index of this Vertex in the owning Bundle.
		  * For Strip meshes that are also Stitches, a secondary remote Vertex is
		  * defined and the actual location of the RemoteVertex is understood to be
		  * somewhere along the line connecting the primary and the secondary Vertex.
		  *
		  * Besides their primary use as components of Strip objects, RemoteVertex objects
		  * can also be used as standalone objects for representing vertex-bundle
		  * pairs (i.e. an object carrying information that a certain vertex exists
		  * in the Mesh as a component of a Bundle with a certain index).
		  */
		class RemoteVertex : public Vertex
		{
			private:
				Bundle * owner; /**< The Bundle that owns this Vertex. */
				xVert remoteIndex; /**< The index of this Vertex in its owning Bundle. */
				Bundle * secondaryOwner; /**< The Bundle that owns the secondary Vertex. */
				xVert secondaryIndex; /**< The index of the secondary Vertex in its Bundle. */
				tiny::vec3 secondaryPos; /**< The position of the secondary Vertex. */
				/** The offset (between 0 and 1) of the vertex from the primary towards the secondary Vertex,
				  * with the convention that at 0 the vertex is at the primary remote vertex. */
				float offset;
			public:
				/** Allow construction from existing vertex plus its owner. Used when copying a Vertex
				  * from a Bundle into a Strip. After construction, the RemoteVertex will not yet have
				  * a valid local index for the Strip, as this must be set by the Strip creating it. */
				RemoteVertex(const Vertex &v, Bundle * _owner) : RemoteVertex(_owner, v.index) {}

				/** Create a RemoteVertex. Note that this does not set the 'index' field of the Vertex. 
				  * This form allows construction from remote index + remote bundle, which is the default
				  * form. While the RemoteVertex should be a valid class in itself, its parameters must
				  * be further specified in order for it to be used as an element of a Strip object, in
				  * particular the 'index' field that is inherited from the Vertex class. */
				RemoteVertex(Bundle * _owner, xVert _remoteIndex) :
					Vertex(0.0f,0.0f,0.0f), owner(_owner), remoteIndex(_remoteIndex),
					secondaryOwner(0), secondaryIndex(0), secondaryPos(0.0f,0.0f,0.0f), offset(0.0f)
				{
					resetPosition();
				}

				/** A constructor for creating uninitialized strip vertices. Used by TopologicalMesh as
				  * the generic VertexType constructor. */
				RemoteVertex(tiny::vec3) : RemoteVertex(0, 0) {}

				/** Allow construction from existing strip vertex.
				  * This duplicates the remoteIndex and (now unused) mfid, and is used when making a copy of a
				  * RemoteVertex from a Strip for another Strip object. */
				RemoteVertex(const RemoteVertex &v, long unsigned int) : RemoteVertex(v.owner, v.remoteIndex) {}

				RemoteVertex(const RemoteVertex &v) : Vertex(v), owner(v.owner), remoteIndex(v.remoteIndex),
					secondaryOwner(0), secondaryIndex(0), secondaryPos(0.0f,0.0f,0.0f), offset(0.0f)
				{
					resetPosition();
				}

				const xVert & getRemoteIndex(void) const { return remoteIndex; }
				void setRemoteIndex(const xVert &v) { remoteIndex = v; }

				Bundle * getOwningBundle(void) { return owner; }
				const Bundle * getOwningBundle(void) const { return owner; }
				void setOwningBundle(Bundle * _owner) { owner = _owner; }

				const xVert & getSecondaryIndex(void) const { return secondaryIndex; }
				void setSecondaryIndex(const xVert &v) { secondaryIndex = v; }

				void setSecondaryBundle(Bundle * _owner) { secondaryOwner = _owner; }
				Bundle * getSecondaryBundle(void) { return secondaryOwner; }
				const Bundle * getSecondaryBundle(void) const { return secondaryOwner; }

				void setOffset(float f) { offset = f; }
				float getOffset(void) const { return offset; }

				void setSecondaryPos(tiny::vec3 p) { secondaryPos = p; }
				tiny::vec3 getSecondaryPos(void) const { return secondaryPos; }

				bool isStitchVertex(void) const { return (secondaryOwner != 0); }

				tiny::vec3 getPosition(void) const
				{
					if(isStitchVertex()) return (pos*(1.0f-offset)+secondaryPos*offset);
					else return pos;
				}

				void resetPosition(void);

				bool isValid(void) const { return (owner != 0 && remoteIndex != 0); }

				inline bool operator == (const RemoteVertex &sv) const
				{
					return (owner == sv.owner && remoteIndex == sv.remoteIndex);
				}

				inline bool operator != (const RemoteVertex &sv) const { return !(*this == sv); }
		};
	}
}

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

namespace strata
{
	namespace mesh
	{
		class Bundle;

		class VertexId
		{
			public:
				VertexId(Bundle * _b, xVert _i) :
					owningBundle(_b), index(_i)
				{
				}

				Bundle * owningBundle; /** The Bundle that owns the Vertex. */
				xVert index; /** The index of the vertex in that Bundle. */

				inline bool operator == (const VertexId &v) const
				{
					return (v.owningBundle == owningBundle && v.index == index);
				}

				inline bool operator < (const VertexId &v) const
				{
					return (v.owningBundle < owningBundle ? true :
							( (v.owningBundle == owningBundle && v.index < index) ? true : false) );
				}
		};

		class VertexNeighbor : public VertexId
		{
			public:
				VertexNeighbor(Bundle * _b, xVert _i) :
					VertexId(_b, _i),
					isAlongFracture(false),
					distanceToVertex(0.0f)
				{
				}

				/** Whether the line between this vertex and the origin vertex tracks a fracture. */
				bool isAlongFracture;

				/** The distance to the neighbor vertex. */
				float distanceToVertex;
		};

		class VertexModifier
		{
			public:
				VertexModifier(void) :
					netForce(0.0f,0.0f,0.0f),
					dForce(0.0f,0.0f,0.0f),
					compression(0.0f,0.0f,0.0f),
					extension(0.0f,0.0f,0.0f),
					neighbors()
				{
				}

				/** A vector to contain the net force on this vertex. */
				tiny::vec3 netForce;

				/** A vector to contain the derivatives on the force vector, using
				  * net forces on the neighbors. */
				tiny::vec3 dForce;

				/** A vector to denote the direction and magnitude of maximal compression. */
				tiny::vec3 compression;

				/** A vector to denote the direction and magnitude of maximal extension (=of minimal compressive force). */
				tiny::vec3 extension;

				/** A list of neighbor vertices that this Vertex interacts with. */
				std::vector<VertexNeighbor> neighbors;

				void addNeighbor(VertexNeighbor &v)
				{
					for(unsigned int i = 0; i < neighbors.size(); i++)
						if(neighbors[i] == v) return;
					neighbors.push_back(v);
				}
		};
	} // end namespace mesh
} // end namespace strata


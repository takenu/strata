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
		class VertexModifier;

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
				VertexNeighbor(Bundle * _b, xVert _i, VertexModifier * vm) :
					VertexId(_b, _i),
					neighbor(vm),
					isAcrossFracture(false),
					initialDistanceToVertex(0.0f),
					restorativeForce(0.0f,0.0f,0.0f),
					dForce(0.0f,0.0f,0.0f)
				{
				}

				/** A pointer to a VertexModifier.
				  * VertexModifiers are stored in an std::map, but references to them should remain
				  * valid under insertion/erasure on the map (so long as the object itself is not
				  * erased, obviously). Therefore, storing this pointer should in principle be safe. */
				VertexModifier * neighbor;

				/** Whether the line connecting these neighbor vertices crosses a fracture plane.
				  * Such neighbors can only exert compressive force on vertices on the other
				  * side of the fracture plane in a direction perpendicular to that plane.
				  * They cannot exert any extensional force at all across the fracture plane.
				  */
				bool isAcrossFracture;

				/** The distance to the neighbor vertex. */
				float initialDistanceToVertex;

				/** The restorative force due to extension or compression along the line connecting
				  * the vertices. */
				tiny::vec3 restorativeForce;

				/** The net force difference between the two verices. */
				tiny::vec3 dForce;
		};

		class VertexModifier
		{
			public:
				VertexModifier(void) :
					isBaseVertex(false),
					initialArea(0.0f),
//					isAlongFracture(false),
					forceMultiplier(0.0f),
					netForce(0.0f,0.0f,0.0f),
					compression(0.0f,0.0f,0.0f),
					extension(0.0f,0.0f,0.0f),
					neighbors()
				{
				}

				/** A flag to denote vertex modifiers that are at the base of the terrain (i.e.
				  * part of the lowest layer). Such vertices have different dynamics, since the
				  * force equilibrium is not merely due to neighbors but also due to buoyancy as
				  * a result of the underlying mass, which is not explicitly part of the Terrain. */
				bool isBaseVertex;

				/** The initial area of a base vertex. This determines the amount of area for which
				  * that base vertex can feel a force. */
				float initialArea;

				/** Whether the vertex is along a fracture.
				  * This is the case if and only if at least one of its neighbors has 'isAcrossFracture'.
				  * Fractures are across a layer and connect vertices of different layers. Vertices
				  * that are along a fracture can move along the fracture lines (creep) in response
				  * to applied force. Their movement rules thus differ from those of ordinary vertices.
				  */
//				bool isAlongFracture;

				/** A multiplier for neighbor forces. This reduces the effect of neighbor forces.
				  * Normalization ensures that no more than half of the net force is lost as a
				  * consequence of force transfer. */
				float forceMultiplier;

				/** A vector to contain the net force on this vertex. */
				tiny::vec3 netForce;

				/** A vector to denote the direction and magnitude of maximal compression. */
				tiny::vec3 compression;

				/** A vector to denote the direction and magnitude of maximal extension (=of minimal compressive force). */
				tiny::vec3 extension;

				/** A list of neighbor vertices that this Vertex interacts with. */
				std::vector<VertexNeighbor> neighbors;

				/** Add a neighbor to the neighbor list. The neighbor may already exist, in that case
				  * we simply don't add it. */
				void addNeighbor(VertexId &v, VertexModifier * vm)
				{
					for(unsigned int i = 0; i < neighbors.size(); i++)
						if(neighbors[i] == v) return;
					neighbors.push_back( VertexNeighbor(v.owningBundle, v.index, vm) );
				}

				/** Calculate the neighbor force differences. */
				void updateNeighborForces(void)
				{
					tiny::vec3 netNeighborForce(0.0f,0.0f,0.0f);
					for(unsigned int i = 0; i < neighbors.size(); i++)
					{
						neighbors[i].dForce = neighbors[i].neighbor->netForce - netForce;
						netNeighborForce += neighbors[i].dForce;
					}
					// Cap between 0.1 and 0.5. Above 0.5 more than half the force would be used
					// (just in opposite directions among neighbors), which still risks unreasonable
					// transfer. Below 0.2 we don't transfer much at all. Additionally, vertices that start
					// with near-zero net force must still be able to receive force, which they can't if
					// the little bit of force they presently have is preserved at all costs.
					forceMultiplier = 0.5f*std::max(0.05f, std::min(2.0f,
								length(netForce)/length(netNeighborForce)));
				}

				/** Apply neighbor forces on net force. */
				void applyNeighborForces(void)
				{
					for(unsigned int i = 0; i < neighbors.size(); i++)
					{
						netForce += neighbors[i].dForce * std::min( forceMultiplier,
									   neighbors[i].neighbor->forceMultiplier);
						netForce += neighbors[i].restorativeForce;
					}
				}
		};
	} // end namespace mesh
} // end namespace strata


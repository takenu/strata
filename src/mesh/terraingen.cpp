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

#include "terrain.hpp"

using namespace strata::mesh;

// This file contains terrain generator definitions. For static functions, see terrain.cpp.

/** Check whether the neighbor at position 'newCandidate' is strictly better than currCandidate.
  * A position is strictly better than another if the other falls inside the former's cone of
  * coverage determined by the dot product of the normalized vectors from newCandidate towards
  * currCandidate and the reference position.
  * Note that this function is not necessarily symmetric: 'a not closer than b' AND 'b not closer than a'
  * is simultaneously possible. In that case, a and b cannot 'cover' each other and must both be
  * included as neighbors.
  */
inline bool isStrictlyCloserNeighbor(tiny::vec3 newCandidate, tiny::vec3 currCandidate, tiny::vec3 refPos)
{
	// If angle between ref and curr candidate, seen from new candidate, is less than 90 degrees,
	// then dot product > 0 and the new candidate is NOT strictly closer than current.
	return !(dot(refPos - newCandidate, currCandidate - newCandidate) > 0.0f);
}

/** Build a vertex map, which creates a VertexModifier for vertices of the Terrain.
  * The building is a two step process: first we create all the VertexModifiers, and then we list
  * neighbors. We can't list neighbors while adding, because neighborship must be a mutual property. */
void Terrain::buildVertexMap(void)
{
	std::cout << " Terrain::buildVertexMap() : Building vertex map for terrain modification..."<<std::endl;
	// Clean up existing map, if any.
	vmap.clear();
	// List vertices.
	for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
	{
		for(unsigned int i = 0; i < it->second->numVertices(); i++)
		{
			vmap.emplace(
					VertexId(it->second, it->second->getVertexIndex(i)),
					VertexModifier() );
		}
	}
	// On listed vertices, list neighbors. Neighbors are always listed in pairs.
	float maxVertSeparation = 10.0f; // Maximal distance for which neighbors are never 'missed'.
	for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
	{
		std::vector<Bundle*> nearbyBundles;
		listNearbyMeshes(bundles, nearbyBundles,
				it->second->getCentralPoint(), it->second->getMaxVertexDistance() + maxVertSeparation);
		for(unsigned int i = 0; i < it->second->numVertices(); i++)
		{
			VertexModifier & v = vmap.at(VertexId(it->second, it->second->getVertexIndex(i)));
			std::vector<VertexNeighbor> neighbors;
			for(unsigned int j = 0; j < nearbyBundles.size(); j++)
			{
				for(unsigned int k = 0; k < nearbyBundles[j]->numVertices(); k++)
				{
					bool addAsNewNeighbor = false;
					// Add as neighbor if nearby vertex is not farther than existing neighbors.
					for(unsigned int l = 0; l < neighbors.size(); l++)
						if( isStrictlyCloserNeighbor(
								nearbyBundles[j]->getVertexPosition(k),
								neighbors[l].owningBundle->getVertexPositionFromIndex(neighbors[l].index),
								it->second->getVertexPosition(i) ) )
						{
							// Neighbor found as k-th vertex of nearby Bundle j. Then we need to clean
							// up the neighbors vector of neighbor candidates by adding the new vertex
							// and removing the vertex that was strictly farther than the new vertex.
							// NOTE: This may remove neighbors that caused other vertices to be non-
							// neighbors. Since strictly-neighborness isn't transitive, we may thus
							// be skipping vertices as neighbors even though they are not strictly
							// covered by neighbor vertices. I hope this will not cause issues.
							addAsNewNeighbor = true;
							neighbors[l] = neighbors.back();
							neighbors.pop_back();
							--l;
						}
					if(addAsNewNeighbor)
						neighbors.push_back( VertexNeighbor( nearbyBundles[j],
															 nearbyBundles[j]->getVertexIndex(k)));
				}
			}
		}
	}
	std::cout << " Terrain::buildVertexMap() : Done."<<std::endl;
}

void Terrain::compress(void)
{
}


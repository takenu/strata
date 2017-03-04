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
	float maxNeighborDistance = 10.0f; // Maximal allowed distance for neighbors.
	long unsigned int nVerticesDone = 0;
	long unsigned int nNeighborsAdded = 0;
	long unsigned int nNeighborsSkipped = 0;
	long unsigned int nNeighborsReplaced = 0;
	long unsigned int nListedMeshes = 0;
	for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
	{
		std::vector<Bundle*> nearbyBundles;
		listNearbyMeshes(bundles, nearbyBundles,
				it->second->getCentralPoint(), it->second->getMaxVertexDistance() + maxVertSeparation);
		nListedMeshes += nearbyBundles.size();
		for(unsigned int i = 0; i < it->second->numVertices(); i++)
		{
			std::vector<VertexNeighbor> neighbors;
			for(unsigned int j = 0; j < nearbyBundles.size(); j++)
			{
				for(unsigned int k = 0; k < nearbyBundles[j]->numVertices(); k++)
				{
					// Skip faraway vertices.
					if( dist(	it->second->getVertexPosition(i),
								nearbyBundles[j]->getVertexPosition(k) ) > maxNeighborDistance) continue;
					// Do not add self as neighbor.
					if( it->second == nearbyBundles[j] && i == k ) continue;
					bool addAsNewNeighbor = true;
					// If the vertex has already caused deletion of existing neighbors, it must be
					// added. If it hasn't caused such deletion, we check if it is already covered by
					// another neighbor, and we only add it if it is not covered.
					for(unsigned int l = 0; l < neighbors.size(); l++)
					{
						if( isStrictlyCloserNeighbor(
								neighbors[l].owningBundle->getVertexPositionFromIndex(neighbors[l].index),
								nearbyBundles[j]->getVertexPosition(k),
								it->second->getVertexPosition(i) ) )
						{
							// If existing neighbor is already covering new candidate, we do not add it.
							addAsNewNeighbor = false;
							++nNeighborsSkipped;
							break;
						}
					}
					if(addAsNewNeighbor)
					{
						// Clean up neighbors that are no longer needed because they are covered by
						// the neighbor that is going to be added.
						for(unsigned int l = 0; l < neighbors.size(); l++)
						{
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
								neighbors[l] = neighbors.back();
								neighbors.pop_back();
								--l;
								++nNeighborsReplaced;
							}
						}
						neighbors.push_back( VertexNeighbor( nearbyBundles[j],
															 nearbyBundles[j]->getVertexIndex(k)));
					}
				}
			}
			// Add list of neighbors to vertex, and add vertex to its neighbors. The addNeighbor
			// function is responsible for avoiding duplicates.
			VertexNeighbor vn(it->second, it->second->getVertexIndex(i));
			for(unsigned int l = 0; l < neighbors.size(); l++)
			{
				vmap.at( vn           ).addNeighbor( neighbors[l] );
				vmap.at( neighbors[l] ).addNeighbor( vn           );
				++nNeighborsAdded;
			}
			++nVerticesDone;
		}
	}
	std::cout << " Terrain::buildVertexMap() : Vertices: "<<nVerticesDone<<" Neighbors: "<<nNeighborsAdded
		<<" Skipped: "<<nNeighborsSkipped<<" Replaced: "<<nNeighborsReplaced<<". Average "
		<<nNeighborsAdded/(1.0*nVerticesDone)<<" neighbors per vertex, for "<<bundles.size()<<" meshes used "
		<<nListedMeshes/(1.0*bundles.size())<<" nearby meshes on average."<<std::endl;
	// Now we mark all vertices of the base layer as such.
	unsigned int nBaseVertices = 0;
	for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
	{
		if(it->second->getParentLayer() == masterLayer)
		{
			for(unsigned int i = 0; i < it->second->numVertices(); i++)
			{
				vmap.at( VertexId(it->second, it->second->getVertexIndex(i)) ).isBaseVertex = true;
				++nBaseVertices;
			}
		}
	}
	std::cout << " Terrain::buildVertexMap() : Marked "<<nBaseVertices<<" base vertices ("
		<<nBaseVertices/(0.01*vmap.size())<<"% of total)."<<std::endl;
	std::cout << " Terrain::buildVertexMap() : Done."<<std::endl;
}

/** Calculate forces acting on the Terrain base (the base layer).
  * These forces are mainly buoyancy (counteracting gravity on the Terrain) and drift (the driving
  * force of compression, mimicking tectonic drift). */
void Terrain::calculateBaseForces(void)
{
	std::cout << " Terrain::calculateBaseForces() : Calculating on "<<vmap.size()<<" vertices. "<<std::endl;
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		if(it->second.isBaseVertex)
		{
			tiny::vec3 pos = getPosition(it->first);
			tiny::vec3 force = tiny::vec3(0.0f,0.0f,0.0f);
			float area = it->first.owningBundle->calculateVertexSurface(it->first.index);
			// Buoyancy.
			force.y += area * (parameters.buoyancyCutoff - pos.y) * parameters.buoyancyGradient;
			// Drift.
			force += area * parameters.compressionAxis * (
					dot(parameters.compressionCenter - pos,
						parameters.compressionCenter - parameters.compressionAxis) > 0.0f ? -1.0f : 1.0f);
		}
	}
	std::cout << " Terrain::calculateBaseForces() : Done. "<<std::endl;
}

void Terrain::calculateNeighborForces(void)
{
	std::cout << " Terrain::calculateNeighborForces() : Calculating on "<<vmap.size()<<" vertices. "<<std::endl;
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		// Neighbor forces.
	}
	std::cout << " Terrain::calculateNeighborForces() : Done. "<<std::endl;
}
void Terrain::compress(void)
{
	if(vmap.size() == 0) buildVertexMap();
	calculateBaseForces();
	calculateNeighborForces();
}


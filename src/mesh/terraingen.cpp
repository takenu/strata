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
	return !(dot(normalize(refPos - newCandidate), normalize(currCandidate - newCandidate)) > 0.0f);
//	return !(dot(normalize(refPos - newCandidate), normalize(currCandidate - newCandidate)) > -0.1f);
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
			std::vector<VertexId> neighbors;
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
						neighbors.push_back( VertexId( nearbyBundles[j],
													   nearbyBundles[j]->getVertexIndex(k)));
					}
				}
			}
			// Add list of neighbors to vertex, and add vertex to its neighbors. The addNeighbor
			// function is responsible for avoiding duplicates.
			VertexId vn(it->second, it->second->getVertexIndex(i));
			for(unsigned int l = 0; l < neighbors.size(); l++)
			{
				vmap.at( vn           ).addNeighbor( neighbors[l] , &(vmap.at(neighbors[l])) );
				vmap.at( neighbors[l] ).addNeighbor( vn           , &(vmap.at(vn          )) );
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
				vmap.at( VertexId(it->second, it->second->getVertexIndex(i)) ).initialArea =
					it->second->calculateVertexSurface(it->second->getVertexIndex(i));
				++nBaseVertices;
			}
		}
	}
	// Set all initial distances between neighbors.
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		for(unsigned int i = 0; i < it->second.neighbors.size(); i++)
		{
			it->second.neighbors[i].initialDistanceToVertex =
				length(getPosition(it->first) - getPosition(it->second.neighbors[i]));
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
	float totBaseForce = 0.0f;
	float totGravity = 0.0f;
	tiny::vec3 alongAxis = normalize(tiny::vec3(parameters.compressionAxis.x, 0, -parameters.compressionAxis.z));
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		if(it->second.isBaseVertex)
		{
			tiny::vec3 pos = getPosition(it->first);
			tiny::vec3 force = tiny::vec3(0.0f,0.0f,0.0f);
//			float area = it->first.owningBundle->calculateVertexSurface(it->first.index);
			float area = it->second.initialArea;
			// Project normal to vertical, since area for buoyancy needs to be projected on horizontal plane.
			float proj = dot( tiny::vec3(0.0f,1.0f,0.0f),
							  it->first.owningBundle->calculateVertexNormal(it->first.index) );
			// Buoyancy.
			force.y += area * proj * (parameters.buoyancyCutoff - pos.y) * parameters.buoyancyGradient;
			// Drift. The drift decreases linearly with the distance to the zero-compression line.
			tiny::vec3 centerToPos = pos - parameters.compressionCenter;
			float distToAxis = length(pos - (parameters.compressionCenter + dot(centerToPos, alongAxis) * alongAxis ));
			force += area * parameters.compressionAxis * (2.0f * distToAxis / maxMeshSize) * (
					dot(parameters.compressionCenter - pos,
						parameters.compressionCenter - parameters.compressionAxis) > 0.0f ? -1.0f : 1.0f);
			// Note: We may instead RESET the net force here to the basal force (instead of adding to it).
			// This prevents us from adding the same force multiple times.
			// However, so long as we decay the force (elsewhere), adding should be fine too.
			it->second.netForce += force;
			totBaseForce += length(force);
		}
		else
		{
			float grav = parameters.gravityFactor * it->first.owningBundle->getVertexWeightByIndex(it->first.index);
			it->second.netForce.y -= grav;
			totGravity += grav;
		}
	}
	std::cout << " Terrain::calculateBaseForces() : Done, avg force = "
		<<totBaseForce/vmap.size()<<", tot gravity = "<<totGravity/vmap.size()<<". "<<std::endl;
}

void Terrain::calculateNeighborForces(void)
{
	std::cout << " Terrain::calculateNeighborForces() : Calculating on "<<vmap.size()<<" vertices. "<<std::endl;
	// Calculate neighbor forces.
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		it->second.updateNeighborForces();
		for(unsigned int i = 0; i < it->second.neighbors.size(); i++)
		{
			tiny::vec3 difVector = getPosition(it->second.neighbors[i]) - getPosition(it->first);
			float deformation = length(difVector) /	it->second.neighbors[i].initialDistanceToVertex - 1.0f;
			it->second.neighbors[i].restorativeForce = normalize(difVector) * (deformation > 0.0f ?
					std::min(parameters.maxExtensionResistance,
						1.0f * parameters.extensionResistance * deformation * deformation) :
					-1.0f * parameters.compressionResistance * deformation * deformation );
			float adjustment = length(it->second.neighbors[i].restorativeForce) * parameters.iterationStep /
					0.1f * length(difVector);
			if(adjustment > 1.0f)
			{
				// In this case the restorative force exceeds the deformation. In that case, cap it.
				// (We multiply by 0.1 because many neighbors feel this force, and all contribute.)
				it->second.neighbors[i].restorativeForce /= adjustment;
			}
		}
	}
	// Apply neighbor forces to net force.
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		it->second.applyNeighborForces();
	}
	std::cout << " Terrain::calculateNeighborForces() : Done. "<<std::endl;
}

void Terrain::applyForces(void)
{
	std::cout << " Terrain::applyForces() : Calculating on "<<vmap.size()<<" vertices. "<<std::endl;
	// Calculate neighbor forces.
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		it->first.owningBundle->moveVertexByIndex(it->first.index,
				parameters.iterationStep * it->second.netForce);
		it->second.netForce *= (1.0f - parameters.forceDecay);
	}
	std::cout << " Terrain::applyForces() : Done. "<<std::endl;
}

void Terrain::resetForces(void)
{
	for(VmapIterator it = vmap.begin(); it != vmap.end(); it++)
	{
		it->second.netForce = tiny::vec3(0.0f,0.0f,0.0f);
	}
}

void Terrain::resetMeshes(void)
{
	std::cout << " Terrain::resetMeshes() : Resetting meshes for all "<<bundles.size()<<" bundles and "
		<<strips.size()<<" strips. "<<std::endl;
	for(BundleIterator it = bundles.begin(); it != bundles.end(); it++)
		it->second->resetMesh();
	for(StripIterator it = strips.begin(); it != strips.end(); it++)
	{
		it->second->recalculateVertexPositions();
		it->second->resetMesh();
	}
}

void Terrain::compress(void)
{
	if(vmap.size() == 0) buildVertexMap();
	calculateBaseForces();
	for(unsigned int i = 0; i < parameters.numForceIterations; i++)
		calculateNeighborForces();
	applyForces();
//	resetForces();
	resetMeshes();
}


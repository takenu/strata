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
#include <functional>

#include "element.hpp"
#include "drawable.hpp"

namespace strata
{
	namespace mesh
	{
		class Bundle;
		class Strip;

		/** An interface for the Mesh class. */
		class MeshInterface : public DrawableMesh
		{
			private:
				/** A list of meshes adjacent to the current one. If a vertex is deleted, all adjacent meshes are notified such that
				  * they can update their mapping.
				  * Since the only dependency is that of Strips borrowing vertices from Bundles, a Bundle has only Strips as adjacentMeshes
				  * and a Strip has only Bundles as adjacent meshes.
				  */
				std::vector<MeshInterface*> adjacentMeshes;
			protected:
				unsigned int numAdjacentMeshes(void) const { return adjacentMeshes.size(); }
				MeshInterface * getAdjacentMesh(unsigned int i) { return (i < adjacentMeshes.size() ? adjacentMeshes[i] : 0); }

				/** Purge a vertex, cleaning it from all references. After this operation, the derived class should consider the vertex with
				  * index "oldVert" belonging to the mesh fragment with id "mfid" as no longer existing. The vertex with index "newVert" is the
				  * suggested replacement of the old vertex. */
				virtual void purgeVertex(long unsigned int mfid, const xVert & oldVert, const xVert & newVert) = 0;

				/** Get the Mesh fragment identifier used to identify the Mesh, which is used in e.g. purgeVertex. */
				virtual long unsigned int getMeshFragmentId(void) const = 0;

				/** Update all vertices in the map, which now have new indices and belong to a new MeshFragment (i.e. a Bundle).
				  * Returns 'true' if anything was updated (in which case an addition to the AdjacentMeshes is in order).
				  * This function can only be used for Strips when a neighboring Bundle is restructured or replaced, with the
				  * mutation given by the 'vmap' and the mapping by the _oldmfid, _newmfid. (Strips that are replaced can simply
				  * copy their references to neighboring Bundles, and only need to create a new list of adjacent meshes. Bundles,
				  * in turn, do not need to update anything besides their own adjacent meshes.)
				  */
				virtual bool updateRemoteVertexIndices(const std::map<xVert,xVert> & vmap, long unsigned int _oldmfid, long unsigned int _newmfid) = 0;

				/** Check whether we are adjacent to any of the vertices in the list. */
//				virtual bool isAdjacentToVertices(const std::vector<xVert> & vlist, long unsigned int _mfid) const = 0;

				/** Check whether the MeshInterface 'm' is an adjacent mesh. */
//				virtual bool isAdjacentToMesh(MeshInterface * m) const = 0;

				/** Purge a vertex from all adjacent meshes. */
				void purgeVertexFromAdjacentMeshes(const xVert & oldVert, const xVert & newVert)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						adjacentMeshes[i]->purgeVertex(getMeshFragmentId(), oldVert, newVert);
				}

				/** Update a set of vertices in all adjacent meshes. Used when a Bundle is being replaced by a new Bundle.
				  * Conversely, if a Strip is being replaced by another Strip, it needs to recalculate its list of adjacent
				  * meshes by itself and add itself directly to those, as the MeshInterface is too basic for this.
				  */
				void updateVerticesInAdjacentMeshes(const std::map<xVert,xVert> & vmap, long unsigned int _oldmfid, long unsigned int _newmfid,
						MeshInterface * newm)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i]->updateRemoteVertexIndices(vmap, _oldmfid, _newmfid))
						{
							addAdjacentMesh(newm);
							newm->addAdjacentMesh(this);
						}
				}

				/** Purge an adjacent mesh, removing it from the adjacentMeshes list. After this action is performed, the adjacent mesh
				  * removed from the list will no longer be informed of changes and communication (e.g. regarding vertices being deleted
				  * or added) becomes impossible. In general, this function should be used only immediately before deleting a mesh. */
				void purgeAdjacentMesh(MeshInterface * _mesh)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i] == _mesh)
						{
							adjacentMeshes[i] = adjacentMeshes.back();
							adjacentMeshes.pop_back();
						}
				}

				/** Copy all members of the adjacentMeshes array from the MeshInterface into the adjacent meshes of the TopologicalMesh
				  * 'm', if required. The adjacency is established by comparing 'vlist' (typically all vertices of 'm') to our vertices.
				  * Checking whether adjacency is already present is not done in this function but only in addAdjacentMesh().
				  */
/*				void duplicateAdjacentMeshes(MeshInterface * m, const std::vector<Vertex> & vlist)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(isAdjacentToVertices(vlist))
						{
							std::cout << " Adding mesh adjacency for "<<adjacentMeshes[i]->getMeshFragmentId()<<" and "<<m->getMeshFragmentId()<<"... "<<std::endl;
							addAdjacentMesh(m);
							m->addAdjacentMesh(this);
						}
				}*/

				/** Check whether a given mesh is already listed as adjacent to this mesh. */
				bool hasAdjacentMesh(MeshInterface * _mesh)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i] == _mesh) return true;
					return false;
				}

				/** Add an adjacent mesh, if it doesn't exist yet. */
				void addAdjacentMesh(MeshInterface * _mesh, bool addReverseAdjacency = false)
				{
					if(!hasAdjacentMesh(_mesh))
						adjacentMeshes.push_back(_mesh);
					if(addReverseAdjacency)
						_mesh->addAdjacentMesh(this, false);
				}

				MeshInterface(core::intf::RenderInterface * _renderer) : DrawableMesh(_renderer) {}

				virtual ~MeshInterface(void)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						adjacentMeshes[i]->purgeAdjacentMesh(this);
				}
			public:
				/** Split the Mesh into two parts. This operation should always reduce the size() of the Mesh. */
				virtual void split(std::function<Bundle * (void)> makeNewBundle, std::function<Strip * (void)> makeNewStrip) = 0;

				/** Determine the size of the fragment, defined as the maximal end-to-end distance between two edge vertices. */
				virtual float meshSize(void) = 0;

				/** Re-declare DrawableMesh's function for converting a mesh to a drawable object. */
				virtual tiny::mesh::StaticMesh convertToMesh(void) const = 0;
		};
	} // end namespace mesh
} // end namespace strata

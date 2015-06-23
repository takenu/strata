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
			protected:
				/** A list of meshes adjacent to the current one. If a vertex is deleted, all adjacent meshes are notified such that
				  * they can update their mapping. */
				std::vector<MeshInterface*> adjacentMeshes;

				/** Purge a vertex, cleaning it from all references. After this operation, the derived class should consider the vertex with
				  * index "oldVert" belonging to the mesh fragment with id "mfid" as no longer existing. The vertex with index "newVert" is the
				  * suggested replacement of the old vertex. */
				virtual void purgeVertex(long unsigned int mfid, xVert oldVert, xVert newVert) = 0;

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

				/** Check whether a given mesh is already listed as adjacent to this mesh. */
				bool hasAdjacentMesh(MeshInterface * _mesh)
				{
					for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
						if(adjacentMeshes[i] == _mesh) return true;
					return false;
				}

				/** Add an adjacent mesh, if it doesn't exist yet. */
				void addAdjacentMesh(MeshInterface * _mesh)
				{
					if(!hasAdjacentMesh(_mesh))
						adjacentMeshes.push_back(_mesh);
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

				virtual tiny::mesh::StaticMesh convertToMesh(void) = 0;
		};
	} // end namespace mesh
} // end namespace strata

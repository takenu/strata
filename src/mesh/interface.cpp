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

#include "interface.hpp"
#include "bundle.hpp"

using namespace strata::mesh;

void MeshInterface::duplicateAdjacentMeshes(Bundle * b)//, const std::vector<Vertex> & vlist)
{
	for(unsigned int i = 0; i < adjacentMeshes.size(); i++)
		//if(isAdjacentToVertices(vlist))
		if(adjacentMeshes[i]->isAdjacentToVertices(b))
		{
			std::cout << " Adding mesh adjacency for "<<adjacentMeshes[i]->getMeshFragmentId()<<" and "<<b->getMeshFragmentId()<<"... "<<std::endl;
			addAdjacentMesh(b);
			b->addAdjacentMesh(this);
		}
}

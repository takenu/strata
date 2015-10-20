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
			protected:
				MeshInterface(core::intf::RenderInterface * _renderer) : DrawableMesh(_renderer) {}

				virtual ~MeshInterface(void)
				{
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

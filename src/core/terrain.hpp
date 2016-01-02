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

#include <tiny/math/vec.h>

#include <tiny/mesh/staticmesh.h>

#include "../interface/render.hpp"
#include "../interface/terrain.hpp"

#include "../mesh/terrain.hpp"

namespace strata
{
	namespace core
	{
		/** Manage all terrain. */
		class TerrainManager : public intf::TerrainInterface
		{
			private:
				intf::RenderInterface * renderInterface;
				intf::UIInterface * uiInterface;

				mesh::Terrain * terrain;
			public:
				TerrainManager(intf::RenderInterface * _renderer, intf::UIInterface * _uiInterface) :
					intf::TerrainInterface(),
					renderInterface(_renderer),
					uiInterface(_uiInterface),
					terrain(0)
				{
				}

				virtual float getVerticalHeight(tiny::vec3 pos)
				{
					if(!terrain) return 0.0f;
					else return terrain->getVerticalHeight(pos);
				}

				/** Register Lua functions used for creating the Terrain. */
				void registerLuaFunctions(sel::State & luaState);

				void makeFlatLayer(float terrainSize, float maxMeshSize, unsigned int meshSubdivisions, float height);
				void addLayer(float thickness);

				void update(double)
				{
					terrain->update();
				}
		};
	}
}

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

#include <config.h>

#include <selene.h>

#include "terrain.hpp"

using namespace strata::core;

void TerrainManager::registerLuaFunctions(sel::State & luaState)
{
	luaState["terrain"].SetObj(*this,
			"makeFlatLayer", &TerrainManager::makeFlatLayer,
			"addLayer", &TerrainManager::addLayer
			);
}

void TerrainManager::makeFlatLayer(float terrainSize, float maxMeshSize, unsigned int meshSubdivisions, float height)
{
	if(terrain) delete terrain;
	terrain = new mesh::Terrain(renderInterface, uiInterface);
//	terrain->makeFlatLayer(1000.0f, 400.0f, 300, 0.0f);
	terrain->makeFlatLayer(terrainSize, maxMeshSize, meshSubdivisions, height);
}

void TerrainManager::addLayer(float thickness)
{
	terrain->addLayer(thickness);
}

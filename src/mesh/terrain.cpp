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

#include <functional>

#include "terrain.hpp"

using namespace strata::mesh;
using namespace std::placeholders; // for using std::bind

Layer * Terrain::makeNewLayer(void)
{
	return new Layer(++meshcounter, meshes, renderer);
}

Stitch * Terrain::makeNewStitch(void)
{
	return new Stitch(++meshcounter, meshes, renderer);
}

void Terrain::splitLargeFragments(float _maxSize)
{
	for(std::map<long unsigned int, MeshFragment*>::iterator it = meshes.begin(); it != meshes.end(); it++)
	{
		if(it->second->meshSize() > _maxSize)
			it->second->split(std::bind(&Terrain::makeNewLayer, this), std::bind(&Terrain::makeNewStitch, this));
	}
}
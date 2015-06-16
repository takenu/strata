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

#include <set>
#include <map>
#include <functional>

#include "layer.hpp"

using namespace strata::mesh;

/** Split a layer into two parts. The splitting is done such that each vertex is assigned to the member
  * of farthestPair that it can reach in the smallest number of steps. */
void Layer::split(std::function<Layer * (void)> makeNewLayer, std::function<Stitch * (void)> makeNewStitch)
{
	VertPair farthestPair(0,0);
	mesh.findFarthestPair(farthestPair);
	Layer * f = makeNewLayer();
	Layer * g = makeNewLayer();
	Stitch * s = makeNewStitch();

	std::map<xVert, xVert> fvert, gvert;
	xVert v;

	v = f->addVertex(getVertexPosition(farthestPair.a)); fvert.emplace(farthestPair.a, v);
	v = g->addVertex(getVertexPosition(farthestPair.b)); gvert.emplace(farthestPair.b, v);
}

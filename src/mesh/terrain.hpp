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

#include <tiny/algo/typecluster.h>
#include <tiny/draw/staticmesh.h>

#include "../core/interface/render.hpp"

#include "layer.hpp"

namespace strata
{
	namespace mesh
	{
		class Terrain
		{
			private:
				core::intf::RenderInterface * renderer;

				long unsigned int layercounter;
				tiny::algo::TypeCluster<long unsigned int, Layer> layers;
			public:
				Terrain(core::intf::RenderInterface * _renderer) : renderer(_renderer), layercounter(0), layers((long unsigned int)(-1), "LayerTC")
				{
					new Layer(++layercounter, layers, renderer, 100.0f, 30);
				}

				~Terrain(void)
				{
				}
		};
	}
}

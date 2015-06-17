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
#include <tiny/draw/staticmesh.h>

#include "../core/interface/render.hpp"

#include "texture.hpp"

namespace strata
{
	namespace mesh
	{
		class MeshInterface;

		/** A DrawableMesh is the base class for all objects that are to be represented by a mesh (i.e.
		  * an object consisting of a set of polygons). In other words, the terrain is defined through the
		  * set of all DrawableMeshes. */
		class DrawableMesh
		{
			private:
			protected:
				core::intf::RenderInterface * renderer;
				tiny::draw::StaticMesh * renderMesh;
				tiny::draw::RGBTexture2D * texture;
			public:
				DrawableMesh(core::intf::RenderInterface * _renderer) :
					renderer(_renderer),
					renderMesh(0),
					texture(createTestTexture(512,255,0,0))
				{
				}

				void initMesh(MeshInterface * mesh);

				void resetTexture(MeshInterface * mesh, unsigned int _size, unsigned char _r, unsigned char _g, unsigned char _b);

				virtual ~DrawableMesh(void) {}
		};
	} // end namespace mesh
} // end namespace strata

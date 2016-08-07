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

#include "../interface/render.hpp"
#include "../tools/texture.hpp"

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
				DrawableMesh(const DrawableMesh &); /**< Nowhere defined - forbid duplication of DrawableMeshes. */
			protected:
				intf::RenderInterface * renderer;
				tiny::draw::StaticMesh * renderMesh;
				tiny::draw::RGBTexture2D * texture;
			public:
				DrawableMesh(intf::RenderInterface * _renderer) :
					renderer(_renderer),
					renderMesh(0),
					texture(0)
				{
				}

				/** Initialize the mesh. This will give the mesh a valid renderMesh, using the function
				  * convertToMesh(). It also sets the mesh as renderable by the WorldRenderer. */
				void initMesh(void);

				/** Create a StaticMesh object (defined in the tiny-game-engine library) to visualise the
				  * DrawableMesh object. The deriving class must specify how it needs to be rendered. */
				virtual tiny::mesh::StaticMesh convertToMesh(void) const = 0;

				/** Get a const reference to the Drawable's texture, in order to allow making a copy of it. */
				tiny::draw::RGBTexture2D * getTexture(void) { return texture; }

				/** Initialize the texture from another texture. If there is no Mesh yet, this
				  * function will also initialize it through initMesh() and convertToMesh().
				  */
				void resetTexture(tiny::draw::RGBTexture2D * _texture);

				/** Reset the Mesh, e.g. when vertex positions change. */
				void resetMesh(void);

				virtual ~DrawableMesh(void);
		};
	} // end namespace mesh
} // end namespace strata

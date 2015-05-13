/*
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

#include <functional>

#include <tiny/math/vec.h>
#include <tiny/draw/texture2d.h>

namespace strata
{
	namespace core
	{
		//A simple bilinear texture sampler, which converts world coordinates to the corresponding texture coordinates on the zoomed-in terrain.
		template<typename TextureType>
		tiny::vec4 sampleTextureBilinear(const TextureType &texture, const tiny::vec2 &scale, const tiny::vec2 &a_pos)
		{
			//Sample texture at the four points surrounding pos.
			const tiny::vec2 pos = tiny::vec2(a_pos.x/scale.x + 0.5f*static_cast<float>(texture.getWidth()), a_pos.y/scale.y + 0.5f*static_cast<float>(texture.getHeight()));
			const tiny::ivec2 intPos = tiny::ivec2(floor(pos.x), floor(pos.y));
			const tiny::vec4 h00 = texture(intPos.x + 0, intPos.y + 0);
			const tiny::vec4 h01 = texture(intPos.x + 0, intPos.y + 1);
			const tiny::vec4 h10 = texture(intPos.x + 1, intPos.y + 0);
			const tiny::vec4 h11 = texture(intPos.x + 1, intPos.y + 1);
			const tiny::vec2 delta = tiny::vec2(pos.x - floor(pos.x), pos.y - floor(pos.y));
			
			//Interpolate between these four points.
			return delta.y*(delta.x*h11 + (1.0f - delta.x)*h01) + (1.0f - delta.y)*(delta.x*h10 + (1.0f - delta.x)*h00);
		}

		namespace intf
		{
			/** Manage interfacing between the TerrainManager and classes that need to use the Terrain (e.g. when they need to know its height).
			  * Instead of passing the whole TerrainManager to such classes, we just pass a pointer to the interface which allows only these
			  * functions that are strictly necessary.
			  */
			class TerrainInterface
			{
				private:
					float getHeightIndirect(tiny::vec2 pos);
					float getHeightFromTexture(tiny::vec2 pos)
					{
						return sampleTextureBilinear(*heightTexture, scale, pos).x;
					}
				protected:
					tiny::draw::FloatTexture2D * heightTexture; /**< The height texture can be in the interface (rather than the TerrainManager) such that we don't need to virtualize getHeight(). */
					const tiny::vec2 scale;
				public:
					TerrainInterface(void) : heightTexture(0), scale(7.0f,7.0f) {}
					~TerrainInterface(void) {}

					float getHeight(tiny::vec3 pos) { return getHeightFromTexture( tiny::vec2(pos.x,pos.z) ); }
					float getHeight(tiny::vec2 pos) { return getHeightFromTexture( pos ); }

					std::function<float(tiny::vec2)> getHeightFunc(void);
			};
		} // end namespace intf
	} // end namespace core
}


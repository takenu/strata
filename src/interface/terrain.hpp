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

#include <functional>

#include <tiny/math/vec.h>
#include <tiny/draw/texture2d.h>

namespace strata
{
	namespace intf
	{
		/** Manage interfacing between the TerrainManager and classes that need to use the Terrain (e.g. when they need to know its height).
		  * Instead of passing the whole TerrainManager to such classes, we just pass a pointer to the interface which allows only these
		  * functions that are strictly necessary.
		  */
		class TerrainInterface
		{
			private:
				/** Indirect function to get the vertical height at a given position. */
				float getVerticalHeightIndirect(tiny::vec3 pos);

				/** Get the vertical height at 'pos', where the vertical height is defined as the first intersection with a terrain
				  * surface by moving straight down. The returned value is then the vertical coordinate of the point of intersection. */
				virtual float getVerticalHeight(tiny::vec3 pos) = 0;
			protected:
				const tiny::vec2 scale;
			public:
				TerrainInterface(void) : scale(7.0f,7.0f) {}
				~TerrainInterface(void) {}

				float getHeight(tiny::vec3 pos) { return getVerticalHeight( pos ); }

				std::function<float(tiny::vec3)> getHeightFunc(void);
		};
	} // end namespace intf
}

/*
This file is part of Chathran Strata: https://github.com/takenu/strata
Copyright 2017, Matthijs van Dorp.

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

namespace strata
{
	namespace mesh
	{
		/** A class to hold parameters used in altering the Terrain.
		  * Such parameters determine the terrain that will be generated by applying modifying actions
		  * such as erosion, compression and sedimentation. */
		class TerrainParameters
		{
			public:
				TerrainParameters(void) :
					compressionAxis(0.8f,0.0f,0.6f),
					compressionCenter(0.0f,0.0f,0.0f)
				{
				}

				/** A vector along which compression takes place on a global scale. */
				tiny::vec3 compressionAxis;

				/** A point along the line of no net force that is perpendicular to the compressionAxis.
				  * Points along this line may move during compression due to pressure imbalances, but
				  * they are subjected to a zero 'artifiial' compressional force (the external force used
				  * to generate terrain deformation).
				  */
				tiny::vec3 compressionCenter;
		};
	}
}

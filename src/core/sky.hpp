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

#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/math/vec.h>

#include "interface/render.hpp"

namespace sel { class State; }

namespace ch
{
	namespace core
	{
		class SkyManager
		{
			private:
				intf::RenderInterface * renderer;

				tiny::draw::StaticMesh *skyBox;
				tiny::draw::RGBTexture2D *skyBoxTexture;
				tiny::draw::RGBATexture2D *skyGradientTexture;
				float sunAngle;
				tiny::draw::effects::SunSky *sunSky;

				void cleanup(void);

				void loadSkyBox(float cubeMeshSize, int boxTexSize, std::string gradTex);
			public:
				SkyManager(intf::RenderInterface * _renderer) :
					renderer(_renderer),
					skyBox(0), skyBoxTexture(0), skyGradientTexture(0), sunAngle(-0.4f), sunSky(0)
				{
				}

				~SkyManager(void) { cleanup(); }

				void update(double)
				{
				}

				void setSun(float angle)
				{
					sunAngle = angle;
					sunSky->setSun(tiny::vec3(sin(sunAngle), cos(sunAngle), 0.5f));
				}

				void registerLuaFunctions(sel::State & luaState);
		};
	}
}

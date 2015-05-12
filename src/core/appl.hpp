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

#include <tiny/os/sdlapplication.h>
#include <tiny/math/vec.h>

#include <config.h>

#include "interface/appl.hpp"

namespace ch
{
	namespace core
	{
		class ApplManager : public intf::ApplInterface
		{
			private:
				tiny::os::Application * application; /**< The engine's application class, for user input, OpenGL, OpenAL, SDL, etcetera. */
			public:
				ApplManager(void) :
					intf::ApplInterface(),
					application(new tiny::os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT))
				{
				}

				~ApplManager(void)
				{
					delete application;
				}

				virtual bool isRunning(void) const { return application->isRunning(); }
				virtual int getScreenWidth(void) const { return application->getScreenWidth(); }
				virtual int getScreenHeight(void) const { return application->getScreenHeight(); }
				virtual tiny::os::MouseState getMouseState(const bool &b) const { return application->getMouseState(b); }

				double pollEvents(void) { return application->pollEvents(); }
				void updateSimpleCamera(double dt, tiny::vec3 & cameraPosition, tiny::vec4 & cameraOrientation) { application->updateSimpleCamera(dt, cameraPosition,cameraOrientation); }
				void paint(void) { application->paint(); }
		};
	}
}

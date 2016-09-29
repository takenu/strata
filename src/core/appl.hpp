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

#include <tiny/os/sdlapplication.h>
#include <tiny/math/vec.h>

#include <config.h>

#include "../interface/appl.hpp"
#include "../interface/ui.hpp"

namespace strata
{
	namespace core
	{
		/** The ApplManager manages low-level application processes such as input/output
		  * and (de)initialization of SDL. */
		class ApplManager : public intf::ApplInterface, public tiny::os::SDLApplication
		{
			private:
				intf::UIInterface * uiInterface;
				tiny::os::MouseState mouseState;

				double dt; /**< Time elapsed between last two updates. */
			public:
				ApplManager(void) :
					intf::ApplInterface(),
					tiny::os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT)
				{
				}

				~ApplManager(void)
				{
				}

				/** Register the UI, for sending events. Note that this cannot be done in the
				  * constructor, because the UI cannot exist yet when the Application is initialized. */
				void registerUIInterface(intf::UIInterface * _ui) { uiInterface = _ui; }

				virtual int getScreenWidth(void) const { return tiny::os::SDLApplication::getScreenWidth(); }
				virtual int getScreenHeight(void) const { return tiny::os::SDLApplication::getScreenHeight(); }

				virtual void stop(void) { stopRunning(); }

				/** Inherit function from tiny::os::SDLApplication to send key up/down events to UI. */
				virtual void keyDownCallback(const int &k)
				{
					uiInterface->keyEvent(static_cast<SDLKey>(k), true);
				}

				/** Inherit function from tiny::os::SDLApplication to send key up/down events to UI. */
				virtual void keyUpCallback(const int &k)
				{
					uiInterface->keyEvent(static_cast<SDLKey>(k), false);
				}

				/** Calculate current fps count. */
				virtual double getFPS(void) const { return dt; }

				double update(void)
				{
					dt = pollEvents();
					tiny::os::MouseState mouseNew = getMouseState(false); // Do not reposition mouse
//					std::cout << " mouse state = "<<mouseNew.x<<", "<<mouseNew.y<<", "<<mouseNew.buttons<<std::endl;
					if(mouseState.buttons != mouseNew.buttons || mouseState.x != mouseNew.x || mouseState.y != mouseNew.y)
					{
//						std::cout << " ApplManager::update() : Mouse event at ("<<mouseNew.x<<","<<mouseNew.y<<")! "<<std::endl;
						uiInterface->mouseEvent( mouseNew.x, -mouseNew.y, mouseNew.buttons ); // Note inversion across y-direction to make +1 the top of the screen and -1 the bottom
					}
					mouseState = getMouseState(false); // Snapshot mouse state for comparison in next cycle
					return dt;
				}
		};
	}
}

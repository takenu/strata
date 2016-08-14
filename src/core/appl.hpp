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
		class ApplManager : public intf::ApplInterface, public tiny::os::SDLApplication
		{
			private:
				intf::UIInterface * uiInterface;
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
					uiInterface->keyEvent(k, true);
				}

				/** Inherit function from tiny::os::SDLApplication to send key up/down events to UI. */
				virtual void keyUpCallback(const int &k)
				{
					uiInterface->keyEvent(k, false);
				}
		};
	}
}

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

#include <sstream>

#include "../interface/ui.hpp"
#include "../interface/appl.hpp"

#include "window.hpp"

namespace strata
{
	namespace ui
	{
		/** The MainMenu is a Window used by the UIManager in order to perform primary tasks that
		  * have widespread impact on the entire state of the program. Examples include exiting, saving
		  * and loading maps, and changing global preferences. */
		class MainMenu : public Window
		{
			private:
				intf::ApplInterface * applInterface;
			public:
				MainMenu(intf::UIInterface * _ui, intf::ApplInterface * _appl,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_ui, _fontTexture), applInterface(_appl)
				{
				}

				void quit(void)
				{
					std::cout << " MainMenu::receiveKeyInput() : Exiting! "<<std::endl;
					applInterface->stop(); // Will set flag, exit takes place after returning
				}

				/** Update the text displayed by the monitor window.
				  * Since we cannot use tiny::draw::TextBox::reserve() from
				  * here, we do not use setText(), which instead must be
				  * done by whoever created the Monitor and has the ability
				  * to add and remove renderable objects. */
				virtual void updateWindow(void)
				{
					if(!isVisible()) return;
					clear();
					drawTitle();
				}

				/** Allow setting of attributes. */
				virtual void setWindowAttribute(std::string, std::string)
				{
				}

				/** Receive UI input (e.g. through button clicks). This function implements behavior
				 * that can be bound to certain inputs using Lua. */
				virtual void receiveUIFunctionCall(std::string args)
				{
					if(args == "quit") quit();
					else if(args == "resume") setInvisible();
					else std::cout << " MainMenu::receiveUIFunctionCall() : Unknown parameter '"<<args<<"'!"<<std::endl;
				}
		};
	} // end namespace ui
} // end namespace strata

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

#include "ui/window.hpp"

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
				std::string title;
				virtual void receiveWindowInput(const SDLKey & k, const SDLMod &, bool isDown)
				{
					if(isDown)
					{
						if(k == SDLK_r)
						{
							setInvisible();
						}
						else if(k == SDLK_q)
						{
							std::cout << " MainMenu::receiveKeyInput() : Exiting! "<<std::endl;
							applInterface->stop(); // Will set flag, exit takes place after returning
						}
					}
				}
			public:
				MainMenu(intf::UIInterface * _ui, intf::ApplInterface * _appl,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_ui, _fontTexture), applInterface(_appl)
				{
					registerTriggerKey(SDLK_ESCAPE);
					registerActiveKey(SDLK_q);
					registerActiveKey(SDLK_r);
				}

				/** Update the text displayed by the monitor window.
				  * Since we cannot use tiny::draw::TextBox::reserve() from
				  * here, we do not use setText(), which instead must be
				  * done by whoever created the Monitor and has the ability
				  * to add and remove renderable objects. */
				virtual void update(void)
				{
					if(!isVisible()) return;
					clear();
					drawTitle();
					addTextFragment("Q", getSecondaryColour());
					addTextFragment("uit", getColour());
					addNewline();
					addTextFragment("R", getSecondaryColour());
					addTextFragment("esume", getColour());
					addNewline();
				}

				/** Allow setting of attributes. */
				virtual void setWindowAttribute(std::string, std::string)
				{
				}
		};
	} // end namespace ui
} // end namespace strata

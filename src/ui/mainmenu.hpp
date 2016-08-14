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
				std::string title;
				tiny::draw::Colour secondaryColour;
			public:
				MainMenu(intf::UIInterface * _ui, tiny::draw::IconTexture2D * _fontTexture,
						float _fontSize, float _aspectRatio, tiny::draw::Colour _colour,
						std::string _title = "") :
					Window(_ui, _fontTexture, _fontSize, _aspectRatio, _colour),
					title(_title), secondaryColour(55,55,0)
				{
					inputKeys->addKey(SDLK_ESCAPE);
				}

				virtual void receiveKeyInput(const SDLKey & k, const SDLMod & m, bool isDown)
				{
					if(isDown)
					{
						if(k == SDLK_ESCAPE || k == SDLK_r)
						{
							if(!isVisible()) setVisible(true);
							else setVisible(false);
						}
						else if(k == SDLK_q)
						{
						}
					}
				}

				/** Update the text displayed by the monitor window.
				  * Since we cannot use tiny::draw::TextBox::reserve() from
				  * here, we do not use setText(), which instead must be
				  * done by whoever created the Monitor and has the ability
				  * to add and remove renderable objects. */
				void update(void)
				{
					clear();
					if(!isVisible()) return;
					if(title.length() > 0)
					{
						addTextFragment(title, getColour());
						addNewline();
					}
					addTextFragment("Q", secondaryColour);
					addTextFragment("uit", getColour());
					addNewline();
					addTextFragment("R", secondaryColour);
					addTextFragment("esume", getColour());
					addNewline();
				}

				void setSecondaryColour(tiny::draw::Colour _colour)
				{
					secondaryColour = _colour;
				}
		};
	} // end namespace ui
} // end namespace strata

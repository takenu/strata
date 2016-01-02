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
		class Monitor : public Window
		{
			private:
				std::string title;
				bool showFramesPerSecond;
				bool showMemoryUsage;
			public:
				Monitor(intf::UIInterface * _ui, tiny::draw::IconTexture2D * _fontTexture,
						float _fontSize, float _aspectRatio, tiny::draw::Colour _colour,
						std::string _title = "") :
					Window(_ui, _fontTexture, _fontSize, _aspectRatio, _colour),
					title(_title), showFramesPerSecond(false),
					showMemoryUsage(false)
				{
				}

				/** Update the text displayed by the monitor window.
				  * Since we cannot use tiny::draw::TextBox::reserve() from
				  * here, we do not use setText(), which instead must be
				  * done by whoever created the Monitor and has the ability
				  * to add and remove renderable objects. */
				void update(double dt)
				{
					clear();
					if(title.length() > 0)
					{
						addTextFragment(title, getColour());
						addNewline();
					}
					if(showFramesPerSecond)
					{
						std::stringstream ss;
						ss << "Running at "<<1.0/dt<<" fps.";
						addTextFragment(ss.str(), getColour());
						addNewline();
					}
					if(showMemoryUsage)
					{
						intf::UIInformation meminfo = uiInterface->getUIInfo("Terrain");
						for(unsigned int i = 0; i < meminfo.pairs.size(); i++)
						{
							addTextFragment("Terrain: "+meminfo.pairs[i].first+" is "+meminfo.pairs[i].second, getColour());
							addNewline();
						}
					}
				}

				/** Show/hide frames per second. */
				void displayFPS(bool b) { showFramesPerSecond = b; }

				/** Show/hide memory usage. */
				void displayMemoryUsage(bool b) { showMemoryUsage = b; }
		};
	} // end namespace ui
} // end namespace strata

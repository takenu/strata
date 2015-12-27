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
			public:
				Monitor(tiny::draw::IconTexture2D * _fontTexture,
						float _fontSize, float _aspectRatio, tiny::draw::Colour _colour,
						std::string _title = "", bool _showfps = false) :
					Window(_fontTexture, _fontSize, _aspectRatio, _colour),
					title(_title), showFramesPerSecond(_showfps)
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
				}
		};
	} // end namespace ui
} // end namespace strata

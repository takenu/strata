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

#include <tiny/draw/textbox.h>
#include <tiny/img/io/image.h>

#include "../ui/monitor.hpp"

#include "../interface/appl.hpp"
#include "../interface/render.hpp"

namespace strata
{
	namespace core
	{
		/** Take care of user input and user interface. */
		class UIManager : public intf::UIInterface
		{
			private:
				intf::ApplInterface * applInterface;
				intf::RenderInterface * renderInterface;

				tiny::draw::IconTexture2D * fontTexture;
				std::vector<ui::Window*> windows;
				ui::Monitor * monitor;
				float defaultFontSize;
				float defaultAspectRatio;

				/** Reserve enough space for the Window to draw all of its text. */
				void reserve(ui::Window * window);
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) :
					intf::UIInterface(),
					applInterface(_appl), renderInterface(_renderer),
					fontTexture(0), monitor(0), defaultFontSize(0.01f), defaultAspectRatio(1.0f)
				{
				}

				void update(double);

				/** Register Lua functions used for composing the UI. */
				void registerLuaFunctions(sel::State & luaState);

				void loadFont(std::string fontTex, float fontSize, float fontAspectRatio, unsigned int fontPixels, unsigned int fontResolution);

				void loadMonitorWindow(float left, float top, float right, float bottom,
						unsigned int red, unsigned int green, unsigned int blue, std::string text);

				void loadMonitorWindowAttribute(std::string attribute, std::string value);
		};
	}
}

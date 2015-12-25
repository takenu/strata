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

#include "interface/appl.hpp"
#include "interface/render.hpp"

namespace strata
{
	namespace core
	{
		/** Take care of user input and user interface. */
		class UIManager
		{
			private:
				intf::ApplInterface * applInterface;
				intf::RenderInterface * renderer;

				tiny::draw::IconTexture2D * fontTexture;
				tiny::draw::TextBox * textBox;
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) : applInterface(_appl), renderer(_renderer), fontTexture(0), textBox(0)
				{
				}

				void update(double)
				{
				}

				/** Register Lua functions used for composing the UI. */
				void registerLuaFunctions(sel::State & luaState);

				void loadUI(std::string fontTex, float fontSize, float fontAspectRatio, unsigned int fontPixels, unsigned int fontResolution);
		};
	}
}

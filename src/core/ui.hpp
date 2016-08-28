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

#include <map>

#include <SDL.h>

#include <tiny/draw/textbox.h>
#include <tiny/img/io/image.h>

#include "../ui/monitor.hpp"
#include "../ui/mainmenu.hpp"
#include "../ui/console.hpp"
#include "../ui/input.hpp"

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

				ui::InputInterpreter inputInterpreter;

				tiny::draw::IconTexture2D * fontTexture;
				std::map<std::string, ui::Window*> windows;
				float defaultFontSize;
				float defaultAspectRatio;

				/** Reserve enough space for the Window to draw all of its text, using the
				  * Window's reserve() function. */
				void reserve(ui::Window * window);
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) :
					intf::UIInterface(),
					applInterface(_appl), renderInterface(_renderer),
					inputInterpreter(),
					fontTexture(0), defaultFontSize(0.01f), defaultAspectRatio(1.0f)
				{
				}

				virtual void keyEvent(const SDLKey & k, bool isDown);

				void update(double);

				/** Register Lua functions used for composing the UI. */
				virtual void registerLuaFunctions(sel::State & luaState);

				void loadFont(std::string fontTex, float fontSize, float fontAspectRatio, unsigned int fontPixels, unsigned int fontResolution);

				void loadFlatTexture(std::string target, unsigned int size, unsigned int red,
						unsigned int green, unsigned int blue, unsigned int alpha);

				void loadMonitorWindow(std::string id);
				void loadMainMenuWindow(std::string id);

				/** Load an attribute-value pair for the Window whose key in the 'windows' map is 'target'. */
				void loadWindowAttribute(std::string target, std::string attribute, std::string value);

				/** Load a font colour for a target Window. */
				void loadWindowFontColour(std::string target, std::string attribute,
						unsigned int red, unsigned int green, unsigned int blue);

				/** Load the dimensions of (part of) a Window. */
				void loadWindowDimensions(std::string target, std::string attribute,
						float left, float top, float right, float bottom);

				/** Redirect InputInterpreter subscription requests. */
				virtual intf::InputSet * subscribe(intf::UIListener * l)
				{
					return inputInterpreter.subscribe(l);
				}

				/** Redirect InputInterpreter unsubscription requests. */
				virtual void unsubscribe(intf::UIListener * l)
				{
					inputInterpreter.unsubscribe(l);
				}

				/** Redirect InputInterpreter bump requests. */
				virtual void bump(intf::UIListener * l)
				{
					inputInterpreter.bump(l);
				}
		};
	}
}

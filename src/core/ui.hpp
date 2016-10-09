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

#include "../ui/input.hpp"
#include "../ui/window.hpp"
#include "../ui/console.hpp"

#include "../interface/appl.hpp"
#include "../interface/lua.hpp"
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
				intf::LuaInterface * luaInterface;

				ui::InputInterpreter inputInterpreter;
				ui::Console * console; /**< A console window to be loaded from Lua. */

				tiny::draw::IconTexture2D * fontTexture;
				std::map<std::string, ui::Window*> windows;
				std::map<std::string, intf::UIReceiver*> receivers;
				float defaultFontSize;
				float defaultAspectRatio;

				/** Reserve enough space for the Window to draw all of its text, using the
				  * Window's reserve() function. */
				void reserve(ui::Window * window);
			public:
				UIManager(intf::ApplInterface * _appl, intf::RenderInterface * _renderer) :
					intf::UIInterface(),
					applInterface(_appl), renderInterface(_renderer), luaInterface(0),
					inputInterpreter(), console(0),
					fontTexture(0), defaultFontSize(0.01f), defaultAspectRatio(1.0f)
				{
				}

				virtual void keyEvent(const SDLKey & k, bool isDown);
				virtual void mouseEvent(float x, float y, unsigned int buttons);
				virtual void callExternalFunction(std::string receiver, std::string args);

				void update(double);

				/** Register the Lua manager, such that the UI can be used to execute Lua code. */
				void registerLuaInterface(intf::LuaInterface* _luaInterface)
				{
					luaInterface = _luaInterface;
				}

				/** Register Lua functions used for composing the UI. */
				virtual void registerLuaFunctions(sel::State & luaState);

				void loadFont(std::string fontTex, float fontSize, float fontAspectRatio, unsigned int fontPixels, unsigned int fontResolution);

				void loadFlatTexture(std::string target, std::string type, unsigned int size,
						unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha);

				void loadConsoleWindow(std::string id);
				void loadMonitorWindow(std::string id);
				void loadMainMenuWindow(std::string id);

				/** Load an attribute-value pair for the Window whose key in the 'windows' map is 'target'. */
				void loadWindowAttribute(std::string target, std::string attribute, std::string value);

				/** Load an attribute-value pair for a Button. */
				void loadButtonAttribute(std::string target, std::string button,
						std::string attribute, std::string value);

				/** Load a font colour for a target Window. */
				void loadWindowFontColour(std::string target, std::string attribute,
						unsigned int red, unsigned int green, unsigned int blue);

				/** Load the dimensions of (part of) a Window. */
				void loadWindowDimensions(std::string target, std::string attribute,
						float left, float top, float right, float bottom);

				/** Load a button for a target object. */
				void loadButton(std::string target, std::string buttonId);

				virtual void logConsoleMessage(const intf::UIMessage & message)
				{
					if(console) console->logMessage(message);
				}

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

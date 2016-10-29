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

#include "../tools/convertstring.hpp"

#include "../interface/appl.hpp"
#include "../interface/keys.hpp"
#include "../interface/lua.hpp"
#include "../interface/ui.hpp"

#include "window.hpp"

namespace strata
{
	namespace ui
	{
		class Console : public Window
		{
			private:
				intf::LuaInterface * luaInterface;
				std::deque<std::string> log;
				tiny::draw::Colour logFontColour;
				unsigned int maxLogLines;
				bool textIsAlwaysVisible;

				/** This string can contain an arbitrary chunk of executable Lua code. */
				std::string command;

				/** Execute currently entered Lua code, and hide the Console. */
				void executeAndHide(void)
				{
					std::cout << " lua interface = "<<luaInterface<<std::endl;
					if(luaInterface && command.size() > 0)
					{
						std::cout << " Console: Execute Lua: "<<command<<std::endl;
						luaInterface->executeLua(command);
					}
					if(command.size() > 0)
					{
						log.push_front(command);
						if(log.size() > maxLogLines) log.pop_back();
						command.clear();
					}
					setInvisible();
				}
			public:
				Console(std::string _id, intf::UIInterface * _ui, intf::LuaInterface * _lua,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_id, _ui, _fontTexture),
					luaInterface(_lua),
					log(),
					logFontColour(0.0f,0.0f,0.0f),
					maxLogLines(3),
					textIsAlwaysVisible(true)
				{
				}

				virtual ~Console(void) {}

				virtual void updateWindow(void)
				{
					if(!isVisible() && !textIsAlwaysVisible) return;
					clear();
					if(command.size()>0)
					{
						addTextFragment(command, getColour());
					}
					addNewline();
					for(unsigned int i = 0; i < log.size(); i++)
					{
						addTextFragment(log[i], logFontColour);
						addNewline();
					}
				}

				virtual void setWindowAttribute(std::string attribute, std::string value)
				{
					if(attribute == "maxLogLines") maxLogLines = tool::toUnsignedInteger(value);
					if(attribute == "textAlwaysVisible") textIsAlwaysVisible = tool::toBoolean(value);
				}

				// Not necessary (yet), log lines are part of Console textbox
/*				virtual void setWindowDimensions(std::string attribute,
						float left, float top, float right, float bottom)
				{
				}*/

				virtual void setWindowFontColour(std::string attribute, const tiny::draw::Colour & _c)
				{
					if(attribute == "logFontColour") logFontColour = _c;
				}

				/** Receive functions. This is used to receive key input, and basically does the following:
				  * - if the executeKey is pressed, without modifiers, execute the text;
				  * - if the input is valid text (or valid text navigation), perform the associated action;
				  * - if the input is not valid text, do nothing.
				  */
				virtual void receiveUIFunctionCall(std::string args)
				{
					// TODO: Implement backspace, implement multi-line, and implement cursor movement
					if(args == "Execute") executeAndHide();
					else if(args == "BACKSPACE" && command.size() > 0 ) command.pop_back();
					else if(toSDLKey(args) != SDLK_UNKNOWN)
						command.push_back( convertSDLinput(toSDLKey(args), getUIInterface()->getKeyMods()) );
					else std::cout << " Console : Skipping non-text key press '"<<args<<"'!"<<std::endl;
				}

				void logMessage(const intf::UIMessage & message)
				{
					// TODO: Add old-message window for external messages and executed Lua commands
					// TODO: Add list structure using a single TextBox
					// TODO: Add clickable button objects
				}
		};
	} // end namespace ui
} // end namespace strata

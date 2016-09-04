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

#include "ui/window.hpp"

namespace strata
{
	namespace ui
	{
		class Console : public Window
		{
			private:
				intf::LuaInterface * luaInterface;

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
					command.clear();
					setInvisible();
				}
			public:
				Console(intf::UIInterface * _ui, intf::LuaInterface * _lua,
						tiny::draw::IconTexture2D * _fontTexture) :
					Window(_ui, _fontTexture),
					luaInterface(_lua)
				{
					registerTriggerKey(SDLK_RETURN);
					registerActiveKeySet( keySetTextComplete() );
					registerActiveKey(SDLK_RETURN);
				}

				virtual void receiveWindowInput(const SDLKey & k, const SDLMod & m, bool isDown)
				{
					std::cout << " Console : receive "<<k<<std::endl;
					if(isDown)
					{
						if(k == SDLK_RETURN) executeAndHide();
						else command.push_back( convertSDLinput(k,m) );
						std::cout << " Console : Command = '"<<command<<"'"<<std::endl;
					}
				}

				virtual void update(void)
				{
					if(!isVisible()) return;
					clear();
					addTextFragment(command, getColour());
					addNewline();
				}

				virtual void setWindowAttribute(std::string, std::string)
				{
				}

				void logMessage(const intf::UIMessage & message)
				{
					// TODO: Add old-message window for external messages and executed Lua commands
				}
		};
	} // end namespace ui
} // end namespace strata

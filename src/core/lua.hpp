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

#include <tiny/os/sdlapplication.h>
#include <tiny/math/vec.h>

#include <config.h>

#include "../interface/ui.hpp"
#include "../interface/render.hpp"
#include "../interface/terrain.hpp"
#include "../interface/sky.hpp"
#include "../interface/lua.hpp"

namespace strata
{
	namespace core
	{
		/** The LuaManager manages the Lua state and execution of Lua scripts. */
		class LuaManager : public intf::LuaInterface
		{
			private:
				intf::RenderInterface * renderInterface;
				intf::UIInterface * uiInterface;
				intf::TerrainInterface * terrainInterface;
				intf::SkyInterface * skyInterface;

				sel::State luaState; /**< A Lua state. */

				double dt; /**< Time elapsed between last two updates. */
			public:
				LuaManager(intf::RenderInterface * _renderInterface, intf::UIInterface * _uiInterface,
						intf::TerrainInterface * _terrainInterface, intf::SkyInterface * _skyInterface) :
					intf::LuaInterface(),
					renderInterface(_renderInterface),
					uiInterface(_uiInterface),
					terrainInterface(_terrainInterface),
					skyInterface(_skyInterface),
					luaState(true)
				{
				}

				~LuaManager(void)
				{
				}

				/** Initialize the world using Lua scripts. */
				void composeWorld(void)
				{
					luaState.Load(DATA_DIRECTORY + "lua/start.lua");
					luaState["start"]();
				}

				/** Register functions that Lua can call. Note that due to Chathran's O-O
				  * nature the functions to be registered are not the usual static functions
				  * but instead member functions of instantiated classes. This means that
				  * during the lifetime of Lua these instantiations must not be deleted.
				  * Therefore it's probably best to only register member functions of the
				  * Manager instances of the Game class. */
				void registerLuaFunctions(void)
				{
					skyInterface->registerLuaFunctions(luaState);
					uiInterface->registerLuaFunctions(luaState);
					terrainInterface->registerLuaFunctions(luaState);
				}

				/** Execute an arbitrary chunk of Lua code. This code can reference anything that has
				  * been previously registered to Lua. */
				virtual void executeLua(std::string command)
				{
					luaState(command.c_str());
				}
		};
	}
}

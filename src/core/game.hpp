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

#include <selene.h> // for Selene, C++11 Lua interface library. Include before Lua (it defines compatibility with Lua 5.2)

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "appl.hpp"
#include "lua.hpp"
#include "render.hpp"
#include "sky.hpp"
#include "terrain.hpp"
#include "ui.hpp"

namespace strata
{
	namespace core
	{
		class Game
		{
			private:
				ApplManager applManager; /**< Manage application specifics (key presses, low level rendering, sound, etcetera). */
				RenderManager renderManager; /**< Manage graphics rendering. */
				UIManager uiManager; /**< Manage user input and user interface. */
				TerrainManager terrainManager; /**< Manage terrain. */
				SkyManager skyManager; /**< Manage sky and weather. */
				LuaManager luaManager; /**< Manage sky and weather. */
			public:
				/** Construct the Game's primary components.
				  * The order of construction is of paramount importance. The first
				  * is the applManager, which creates the tiny-game-engine's
				  * application object for OpenGL, SDL etcetera. The applManager
				  * is then used to create the Renderer, which draws things. Both the
				  * applManager and the renderManager are then used to create the
				  * uiManager. The uiManager is used to give a UI form to all objects,
				  * and everything created after it can use the uiManager to generate
				  * a representation of itself in UI form.
				  *
				  * Concerning the order of destruction: note that C++ is guaranteed
				  * to call the destructors in reverse order (see e.g. here:
				  * http://stackoverflow.com/questions/2254263 ). Therefore, it is
				  * perfectly safe for every member variable to use all members declared
				  * before it, and it is dangerous to use later-declared members.
				  */
				Game(void) :
					applManager(),
					renderManager(static_cast<intf::ApplInterface*>(&applManager)),
					uiManager(static_cast<intf::ApplInterface*>(&applManager),static_cast<intf::RenderInterface*>(&renderManager)),
					terrainManager(static_cast<intf::RenderInterface*>(&renderManager),static_cast<intf::UIInterface*>(&uiManager)),
					skyManager(static_cast<intf::RenderInterface*>(&renderManager)),
					luaManager(static_cast<intf::RenderInterface*>(&renderManager),
							static_cast<intf::UIInterface*>(&uiManager),
							static_cast<intf::TerrainInterface*>(&terrainManager),
							static_cast<intf::SkyInterface*>(&skyManager))
				{
					applManager.registerUIInterface(static_cast<intf::UIInterface*>(&uiManager));
					uiManager.registerLuaInterface(static_cast<intf::LuaInterface*>(&luaManager));
					luaManager.registerLuaFunctions();
					luaManager.composeWorld();
					mainLoop();
				}

				~Game(void)
				{
				}

				void mainLoop(void)
				{
					while(applManager.isRunning())
					{
						double dt = applManager.update();
						renderManager.update(dt);
						uiManager.update(dt);
						terrainManager.update(dt);
						skyManager.update(dt);
						applManager.paint();
					}
					std::cout << " Game : exiting main loop. "<<std::endl;
				}
		};
	}
}

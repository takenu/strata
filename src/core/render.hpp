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

/** Start of counting for default (=unordered) world renderables. */
#define WORLD_RENDERABLE_DEFAULT_COUNTER_START 100000
/** Start of counting for default (=unordered) screen renderables.
  * Such 'unordered' renderables will always be rendered in order of addition to the Renderer.
  */
#define SCREEN_RENDERABLE_DEFAULT_COUNTER_START 200000

#include <tiny/draw/worldrenderer.h>
#include <tiny/draw/renderer.h>

#include "../interface/appl.hpp"
#include "../interface/render.hpp"

namespace strata
{
	namespace core
	{
		/** The RenderManager is Strata's final interface towards the rendering capabilities
		  * of the tiny-game-engine. Its core is the WorldRenderer, which renders to screen
		  * all the Renderable's added to it. Strata makes explicit use of the WorldRenderer's
		  * optional resource management, by keeping unique keys to all Renderables.
		  *
		  * The Renderer distinguishes two types of renderable objects. Firstly, there are
		  * WorldRenderables, who have a three-dimensional, spatial representation in the world.
		  * Secondly, there are ScreenRenderables, who do not have such a representation and are
		  * merely painted on top of the scene. Examples are UI elements such as menus and text,
		  * which are printed at screen-constant positions, and lighting effects such as light
		  * sources. */
		class RenderManager : public intf::RenderInterface
		{
			private:
				intf::ApplInterface * applInterface;
				tiny::vec3 cameraPosition;
				tiny::vec4 cameraOrientation;
				bool lodFollowsCamera;
				unsigned int worldRenderableKeyCounter;
				unsigned int screenRenderableKeyCounter;
				std::map<tiny::draw::Renderable *, unsigned int> worldRenderableKeyMap;
				std::map<tiny::draw::Renderable *, unsigned int> screenRenderableKeyMap;

				tiny::draw::WorldRenderer * worldRenderer;

				void setup(void)
				{
				}

				void cleanup(void)
				{
					delete worldRenderer;
				}

				void render(void)
				{
					worldRenderer->clearTargets();
					worldRenderer->render();
				}

				virtual void addWorldRenderableWithIndex(tiny::draw::Renderable * renderable,
						unsigned int & renderableIndex,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
					if(worldRenderableKeyMap.find(renderable) != worldRenderableKeyMap.end())
					{
						std::cout << " RenderManager::addWorldRenderableWithIndex() : ERROR: Renderable "
							<<renderable<<" with key "<<worldRenderableKeyMap.at(renderable)
							<<" already exists! Cannot add! "<<std::endl;
						return;
					}
					if(renderableIndex == 0) renderableIndex = ++worldRenderableKeyCounter;
					unsigned int num_attempts = 0;
					while(++num_attempts < 1000 && worldRenderer->worldRenderableIndexExists(renderableIndex))
					{
						if(renderableIndex == worldRenderableKeyCounter) ++worldRenderableKeyCounter;
						++renderableIndex;
					}
					if(worldRenderer->worldRenderableIndexExists(renderableIndex))
					{
						std::cout << " RenderManager::addWorldRenderableWithIndex() : No index!"<<std::endl;
						return;
					}
					try
					{
						worldRenderer->addWorldRenderable(renderableIndex, renderable,
								readDepthTex, writeDepthTex, blendMode);
						worldRenderableKeyMap.insert( std::make_pair(renderable, renderableIndex) );
					}
					catch(std::exception &)
					{
						// Exceptions are thrown (without filling 'what()') on duplicate renderable indices.
						std::cout << " RenderManager::addWorldRenderableWithIndex() : Skipped! "<<std::endl;
					}
				}

				virtual void addScreenRenderableWithIndex(tiny::draw::Renderable * renderable,
						unsigned int & renderableIndex,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
					if(screenRenderableKeyMap.find(renderable) != screenRenderableKeyMap.end())
					{
						std::cout << " RenderManager::addScreenRenderable() : ERROR: Renderable "
							<<renderable<<" with key "<<screenRenderableKeyMap.at(renderable)
							<<" already exists! Cannot add! "<<std::endl;
						return;
					}
					if(renderableIndex == 0) renderableIndex = ++screenRenderableKeyCounter;
					unsigned int num_attempts = 0;
					while(++num_attempts < 1000 && worldRenderer->screenRenderableIndexExists(renderableIndex))
					{
						if(renderableIndex == screenRenderableKeyCounter) ++screenRenderableKeyCounter;
						++renderableIndex;
					}
					if(worldRenderer->screenRenderableIndexExists(renderableIndex))
					{
						std::cout << " RenderManager::addScreenenderableWithIndex() : No index!"<<std::endl;
						return;
					}
					try
					{
						worldRenderer->addScreenRenderable(renderableIndex, renderable,
								readDepthTex, writeDepthTex, blendMode);
						screenRenderableKeyMap.insert( std::make_pair(renderable, renderableIndex) );
					}
					catch(std::exception &)
					{
						std::cout << " RenderManager::addScreenRenderableWithIndex() : Skipped! "<<std::endl;
					}
				}

				virtual void freeWorldRenderable(tiny::draw::Renderable * renderable)
				{
					worldRenderer->freeWorldRenderable(worldRenderableKeyMap.find(renderable)->second);
					worldRenderableKeyMap.erase(renderable);
				}

				virtual void freeScreenRenderable(tiny::draw::Renderable * renderable)
				{
					worldRenderer->freeScreenRenderable(screenRenderableKeyMap.find(renderable)->second);
					screenRenderableKeyMap.erase(renderable);
				}

				virtual unsigned int getWorldRenderableIndex(tiny::draw::Renderable * renderable) const
				{
					return (worldRenderableKeyMap.count(renderable) > 0 ?
							worldRenderableKeyMap.find(renderable)->second : 0);
				}

				virtual unsigned int getScreenRenderableIndex(tiny::draw::Renderable * renderable) const
				{
					return (screenRenderableKeyMap.count(renderable) > 0 ?
							screenRenderableKeyMap.find(renderable)->second : 0);
				}

			public:
				RenderManager(intf::ApplInterface * _interface) :
					intf::RenderInterface(),
					applInterface(_interface),
					cameraPosition(tiny::vec3(0.001f, 20.0f, 3.001f)),
					cameraOrientation(tiny::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
					lodFollowsCamera(true),
					worldRenderableKeyCounter(WORLD_RENDERABLE_DEFAULT_COUNTER_START),
					screenRenderableKeyCounter(SCREEN_RENDERABLE_DEFAULT_COUNTER_START),
					worldRenderer(new tiny::draw::WorldRenderer(
								applInterface->getScreenWidth(), applInterface->getScreenHeight()))
				{
				}
				
				void update(double )
				{
					render();
				}

				virtual bool lodUpdates(void) const { return lodFollowsCamera; }
				virtual tiny::vec3 getCameraPosition(void) const { return cameraPosition; }
				virtual tiny::vec4 getCameraOrientation(void) const { return cameraOrientation; }
				virtual void setCameraPosition(tiny::vec3 pos)
				{
					cameraPosition = pos;
					worldRenderer->setCamera(cameraPosition, cameraOrientation);
				}

				virtual void setCameraOrientation(tiny::vec4 orient)
				{
					cameraOrientation = orient;
					worldRenderer->setCamera(cameraPosition, cameraOrientation);
				}

				void setLodFollowsCamera(bool b) { lodFollowsCamera = b; }
		};
	}
}

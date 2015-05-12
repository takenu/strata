/*
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

#include <tiny/draw/worldrenderer.h>
#include <tiny/draw/renderer.h>

#include "interface/appl.hpp"
#include "interface/render.hpp"

namespace ch
{
	namespace core
	{
		class RenderManager : public intf::RenderInterface
		{
			private:
				intf::ApplInterface * applInterface;
				tiny::vec3 cameraPosition;
				tiny::vec4 cameraOrientation;

				bool lodFollowsCamera;

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

/*				bool getWorldRenderable(RenderableObjectSpecs & ro)
				{
					if(worldRenderables.size() > 0)
					{
						ro = worldRenderables.front();
						worldRenderables.pop_front();
						return true;
					}
					else return false;
				}

				bool getScreenRenderable(RenderableObjectSpecs & ro)
				{
					if(screenRenderables.size() > 0)
					{
						ro = screenRenderables.front();
						screenRenderables.pop_front();
						return true;
					}
					else return false;
				}*/

				virtual void addWorldRenderable(tiny::draw::Renderable * renderable, const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
//					worldRenderables.push_back( RenderableObjectSpecs(renderable, readDepthTex, writeDepthTex, blendMode) );
					worldRenderer->addWorldRenderable(renderable, readDepthTex, writeDepthTex, blendMode);
				}

				virtual void addScreenRenderable(tiny::draw::Renderable * renderable, const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
//					screenRenderables.push_back( RenderableObjectSpecs(renderable, readDepthTex, writeDepthTex, blendMode) );
					worldRenderer->addScreenRenderable(renderable, readDepthTex, writeDepthTex, blendMode);
				}

				// The list-first, add-to-world-renderer-later doesn't seem very necessary - we could skip the RenderableObjectSpecs altogether and
				// redirect addWorldRenderable() to worldRenderer->addWorldRenderable(). Commented out for now, likely to be deleted later.
/*				void getRenderables(void)
				{
					RenderableObjectSpecs ro(0,true,true,tiny::draw::BlendReplace);
					while(getWorldRenderable(ro)) worldRenderer->addWorldRenderable(
				}*/
			public:
				RenderManager(intf::ApplInterface * _interface) :
					intf::RenderInterface(),
					applInterface(_interface),
					cameraPosition(tiny::vec3(0.001f, 256.0f, 0.001f)), cameraOrientation(tiny::vec4(0.0f, 0.0f, 0.0f, 1.0f)), lodFollowsCamera(true),
					worldRenderer(new tiny::draw::WorldRenderer(applInterface->getScreenWidth(), applInterface->getScreenHeight()))
				{
				}
				
				void update(double )
				{
					render();
				}

				virtual bool lodUpdates(void) const { return lodFollowsCamera; }
				virtual tiny::vec3 getCameraPosition(void) const { return cameraPosition; }
				virtual tiny::vec4 getCameraOrientation(void) const { return cameraOrientation; }
				void setCameraPosition(tiny::vec3 pos) { cameraPosition = pos; worldRenderer->setCamera(cameraPosition, cameraOrientation); }
				void setCameraOrientation(tiny::vec4 orient) { cameraOrientation = orient; worldRenderer->setCamera(cameraPosition, cameraOrientation); }
				void setLodFollowsCamera(bool b) { lodFollowsCamera = b; }
		};
	}
}

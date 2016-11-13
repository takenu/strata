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

#include <tiny/draw/renderable.h>
#include <tiny/draw/renderer.h>
//#include <tiny/algo/typecluster.h>

namespace strata
{
	namespace intf
	{
		/** The RenderInterface is a base class via which distinct parts of Chathran can communicate
		  * with the RenderManager. In this way they can add and remove objects to be rendered. */
		class RenderInterface 
		{
			private:
			protected:
				RenderInterface(void) {}
				~RenderInterface(void) {}
			public:
				virtual bool lodUpdates(void) const = 0;
				virtual tiny::vec3 getCameraPosition(void) const = 0;
				virtual tiny::vec4 getCameraOrientation(void) const = 0;
				virtual void setCameraPosition(tiny::vec3 v) = 0;
				virtual void setCameraOrientation(tiny::vec4 v) = 0;

				/** Find the index of a WorldRenderable. */
				virtual unsigned int getWorldRenderableIndex(tiny::draw::Renderable * renderable) const = 0;

				/** Find the index of a ScreenRenderable. */
				virtual unsigned int getScreenRenderableIndex(tiny::draw::Renderable * renderable) const = 0;

				/** Free a previously added WorldRenderable. */
				virtual void freeWorldRenderable(tiny::draw::Renderable * renderable) = 0;

				/** Free a previously added ScreenRenderable. */
				virtual void freeScreenRenderable(tiny::draw::Renderable * renderable) = 0;

				/** Add a new WorldRenderable (a Renderable drawn as a World object, with a particular
				  * position in 3D space). */
				inline void addWorldRenderable(tiny::draw::Renderable * renderable,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
					unsigned int dummy = 0;
					addWorldRenderableWithIndex(renderable, dummy, readDepthTex, writeDepthTex, blendMode);
				}

				/** Add a new ScreenRenderable (a Renderable that is merely a UI element, visible to
				  * the user but without a real meaning in the 3D world, and whose location on screen
				  * is in the form of 2D screen coordinates). */
				inline void addScreenRenderable(tiny::draw::Renderable * renderable,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace)
				{
					unsigned int dummy = 0;
					addScreenRenderableWithIndex(renderable, dummy, readDepthTex, writeDepthTex, blendMode);
				}

				/** Add a WorldRenderer WorldRenderable object with its index. Renderable indices, used
				  * by the tiny-game-engine's Renderer, determine the order of rendering, with lower
				  * indices being rendered first and higher indices being rendered on top of lower ones. */
				virtual void addWorldRenderableWithIndex(tiny::draw::Renderable * renderable,
						unsigned int & renderableIndex,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace) = 0;

				/** Add a WorldRenderer ScreenRenderable object with its index. Renderable indices, used
				  * by the tiny-game-engine's Renderer, determine the order of rendering, with lower
				  * indices being rendered first and higher indices being rendered on top of lower ones. */
				virtual void addScreenRenderableWithIndex(tiny::draw::Renderable * renderable,
						unsigned int & renderableIndex,
						const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace) = 0;
		};
	} // end namespace intf
}

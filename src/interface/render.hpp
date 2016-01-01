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
		/** The RenderInterface is a base class via which distinct parts of Chathran can communicate with the RenderManager. In this way they can add and remove objects to be rendered. */
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

				virtual void addWorldRenderable(tiny::draw::Renderable * renderable, const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace) = 0;

				virtual void addScreenRenderable(tiny::draw::Renderable * renderable, const bool & readDepthTex = true, const bool & writeDepthTex = true,
						const tiny::draw::BlendMode & blendMode = tiny::draw::BlendReplace) = 0;

				virtual void freeWorldRenderable(tiny::draw::Renderable * renderable) = 0;
				virtual void freeScreenRenderable(tiny::draw::Renderable * renderable) = 0;
		};
	} // end namespace intf
}

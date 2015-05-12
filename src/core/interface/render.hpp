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

#include <tiny/draw/renderable.h>
#include <tiny/draw/renderer.h>
//#include <tiny/algo/typecluster.h>

namespace ch
{
	namespace core
	{
		namespace intf
		{
			/** A base class to be used for all objects that need to be rendered to screen. */
	/*		class RenderableObject : private tiny::algo::TypeClusterObject<long unsigned int, RenderableObject>
			{
				private:
				public:
					tiny::draw::Renderable * renderable;

					RenderableObject(long unsigned int _key, tiny::draw::Renderable * _renderable, tiny::algo::TypeCluster<long unsigned int, RenderableObject> * _typeCluster) :
						tiny::algo::TypeClusterObject<long unsigned int, RenderableObject>(_key, this, _typeCluster), renderable(_renderable)
					{
					}
			};*/

			/** The RenderInterface is a base class via which distinct parts of Chathran can communicate with the RenderManager. In this way they can add and remove objects to be rendered. */
			class RenderInterface 
			{
				private:
	//				long unsigned int renderableCounter;
	/*				std::deque<RenderableObjectSpecs> newWorldRenderables;
					std::deque<RenderableObjectSpecs> newScreenRenderables;

					class RenderableObjectSpecs
					{
						private:
						public:
							tiny::draw::Renderable * renderable;
							const bool readDepthTex;
							const bool writeDepthTex;
							const tiny::draw::BlendMode blendMode;

							RenderableObject(tiny::draw::Renderable * _renderable, const bool & _readDepthTex, const bool & _writeDepthTex, const tiny::draw::BlendMode & _blendMode) :
								renderable(_renderable), readDepthTex(_readDepthTex), writeDepthTex(_writeDepthTex), blendMode(_blendMode)
							{
							}
					};*/
				protected:
					// To be used later, when we want to manage Renderables that are added and deleted a lot (these are outside of the Managers).
	//				tiny::algo::TypeCluster<long unsigned int, RenderableObject> worldRenderables;
	//				tiny::algo::TypeCluster<long unsigned int, RenderableObject> screenRenderables;
	//				long unsigned int getRenderableObjectKey(void) { return ++renderableCounter; }

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
			};
		} // end namespace intf
	} // end namespace core
}

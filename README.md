# strata
**[stratum](http://en.wiktionary.org/wiki/stratum#English)** (*plural* **strata**):
* One of several parallel horizontal layers of material arranged one on top of another.
* (*geology*) A layer of sedimentary rock having approximately the same composition throughout.

## Introduction
Chathran Strata, or Strata for short, is a project to provide the
role-playing game Chathran (itself still under development) with a
sophisticated engine dedicated towards generating large, diverse,
detailed and most importantly realistic terrains. It should provide both
a standalone environment for aiding the generation of terrains according
to specific requirements, as well as a library that can be used for
actually loading and using a previously generated terrain.

## Project goal
Strata is being developed in order to create highly differentiated
terrains, where features like landforms and rock textures are determined
through simulating geological processes. In order to do this, it should
simultaneously generate new layers and deform and erode existing ones,
keeping track of all layers relative to each other.

The standalone part of Strata will use the [tiny-game-engine](https://github.com/BasFaggingerAuer/tiny-game-engine)
for rendering. Lua scripts will be used in order to facilitate
instructing Strata when creating terrains, including console
functionality allowing the user to decide upon new instructions after
inspecting the results of the older ones.

## Current status
Strata is still in its earliest stages of development, having been
started at 2015. While the Chathran project (as started in 2007) used to
have its own terrain generator, the code of that terrain generator was
inadequate for the aims of Strata and could not be reused.

## Future directions
Strata development will first work towards handling layers correctly and
being able to save and load terrains. After that, console functionality
should be added and some simple tools for terrain modification should be
implemented.

## Licensing
Analogous to the Chathran project itself, Chathran Strata is available
as free software and licensed under the terms of the GNU GPLv3 license
(or any later version, at your option). This is a copyleft license that
allows end users to use, distribute, modify and build upon the licensed
software. However, please note that it does not allow end users to use
it in proprietary or closed-source software that is not licensed under a
compatible license. For the precise definitions of the GNU GPL, please
refer to the LICENSE file present in this repository.

#### Selene
Strata incorporates a somewhat modified source of Selene for binding Lua
to C++. Selene is licensed under the zlib license as follows:

The Zlib License

Copyright (c) 2013 Jeremy Ong

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.


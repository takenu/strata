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

#include <iostream>
#include <exception>

#include "core/game.hpp"


int main(int, char **)
{
	ch::core::Game * game = 0;
	try
	{
		game = new ch::core::Game();
	}
	catch (std::exception &e)
	{
		std::cerr << "Application threw uncaught exception!" << std::endl;
		return -1;
	}

	std::cout << " Freeing resources... " << std::endl;

	delete game;
	
	std::cout << "Goodbye." << std::endl;
	
	return 0;
}


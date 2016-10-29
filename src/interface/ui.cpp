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

#include "ui.hpp"

using namespace strata::intf;

UISource::UISource(std::string _id, UIInterface * _ui) :
	tiny::algo::TypeClusterObject<std::string, UISource>(_id, this, _ui->getSourceTypeCluster())
{
}

UISource::~UISource(void)
{
}

UIReceiver::UIReceiver(std::string _id, UIInterface * _ui) :
	tiny::algo::TypeClusterObject<std::string, UIReceiver>(_id, this, _ui->getReceiverTypeCluster())
{
}

UIReceiver::~UIReceiver(void)
{
}

UIInterface::UIInterface(void) : sources("", "UISourceTC"), receivers("", "UIReceiverTC")
{
}

UIInformation UIInterface::getUIInfo(std::string _id)
{
	UISource * source = sources.find(_id);
	if(!source) return UIInformation();
	else return source->getUIInfo();
}

UIReceiver * UIInterface::getUIReceiver(std::string _id)
{
	UIReceiver * receiver = receivers.find(_id);
	if(!receiver) return 0;
	else return receiver;
}

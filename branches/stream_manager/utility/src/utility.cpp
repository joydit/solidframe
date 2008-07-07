/* Implementation file utility.cpp
	
	Copyright 2007, 2008 Valentin Palade 
	vipalade@gmail.com

	This file is part of SolidGround framework.

	SolidGround is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Foobar is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SolidGround.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "utility/workpool.h"
#include "utility/polycontainer.h"

//------	WorkPoolPlugin --------------------------------------------

WorkPoolPlugin::~WorkPoolPlugin(){
}
void WorkPoolPlugin::prepare(){
}
void WorkPoolPlugin::unprepare(){
}
bool WorkPoolPlugin::release()const{
	return false;//do not delete
}

static WorkPoolPlugin pwp;

WorkPoolPlugin* basicWorkPoolPlugin(){
	return &pwp;
}

//------	PolyContainter --------------------------------------------

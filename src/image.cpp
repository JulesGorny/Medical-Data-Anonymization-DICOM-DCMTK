/*
This file is part of Anonymizer.

Anonymizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Foobar is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Anonymizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "image.h"

Image::Image() {}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Image::Image(QString type)
{
	_type = type;
}		

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Image::~Image()
{
	qDeleteAll(_studies.begin(), _studies.end());
	_studies.clear();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Study* Image::addStudy(Study* s)
{
	for(int st=0; st<_studies.size(); st++)
	{
		if( _studies[st]->getDescription() == s->getDescription() )
		{
			delete s;
			return _studies[st];
		}
	}
	_studies.append(s);
	return s;
}
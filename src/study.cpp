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

#include "study.h"

Study::Study() {}		

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Study::Study(QString descr)
{
	_description = descr;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Study::~Study()
{
	qDeleteAll(_series.begin(), _series.end());
	_series.clear();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Series* Study::addSeries(Series* s) 
{
	for(int se=0; se<_series.size(); se++)
	{
		if( _series[se]->getDescription() == s->getDescription() )
		{
			delete s;
			return _series[se];
		}
	}
	_series.append(s);
	return s;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Study::addProperty(Property prop) 
{
	if( !_properties.contains(prop) )
		_properties.append(prop);
}

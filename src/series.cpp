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

#include "series.h"

Series::Series() {}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Series::Series(QString descr)
{
	_description = descr;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Series::~Series()
{
	qDeleteAll(_extracts.begin(), _extracts.end());
	_extracts.clear();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Extract* Series::addExtract(Extract* ex) 
{
	for(int e=0; e<_extracts.size(); e++)
	{
		if( _extracts[e]->getNumber() == ex->getNumber() )
		{
			delete ex;
			return _extracts[e];
		}
	}
	_extracts.append(ex);
	return ex;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Series::addProperty(Property prop) 
{
	if( !_properties.contains(prop) )
		_properties.append(prop);
}
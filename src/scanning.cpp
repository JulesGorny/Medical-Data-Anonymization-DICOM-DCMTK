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

#include "scanning.h"

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Scanning::Scanning(QList<Anonymization> *todo, bool *wdBool, bool *icswBool)
{
	_toDo = todo;
	_workDoneBool = wdBool;
	_iCanStartWorking = icswBool;
}

Scanning::~Scanning()
{ }

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Scanning::process()
{
	*_workDoneBool = true;
	while(1)
	{
		int remainAn = _toDo->size();
		if( remainAn > 0 && *_workDoneBool && *_iCanStartWorking)
		{
			*_workDoneBool = false;
			
			Anonymization a;
			a = _toDo->at(0);

			if( a._anonType == "xmlFile" )
				emit anonymize(a);
			else if( a._anonType == "videos" )
				emit anonymizeFiles(a);
			else if( a._anonType == "dicomDir" )
				emit anonymize(a);
			else if( a._anonType == "dicomFiles" )
				emit anonymizeFiles(a);

			_toDo->pop_front();
		}
	}
    emit finished();
}
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

#ifndef EXTRACT_H
#define EXTRACT_H

//Includes Qt
#include <QString>
//My includes
#include "file.h"

/**
* \class Extract
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Extract
{
	private:
		int _number;
		QVector<File*> _files;
		QVector<Property> _properties;

	public:	
		Extract();
		Extract(int nb);
		~Extract();
		
		bool operator == (const Extract& ex2)
		{
			return (_number == ex2._number);
		}
		/*friend bool operator== (Extract &e1, Extract &e2);
		friend bool operator!= (Extract &e1, Extract &e2); */

		//getters
		int getNumber() { return _number; }
		const QVector<File*> getFiles() { return _files; }
		int getFilesCount() { return _files.size(); }
		void fillFilesCommonProperty();
		QVector<Property> getProperties() { return _properties; }
		//setters
		File* addFile(File* f);
		void addProperty(Property prop) { _properties.append(prop); }
};
/*
///
/// \brief operator == for two Extracts
///
bool operator== (Extract &e1, Extract &e2)
{
	return (e1.getNumber() == e2.getNumber());
}

///
/// \brief operator != for two Extracts
///
bool operator!= (Extract &e1, Extract &e2)
{
    return !(e1 == e2);
}*/

#endif
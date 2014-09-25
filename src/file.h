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

#ifndef FILE_H
#define FILE_H

//Includes Qt
#include <QString>
//My includes 
#include "property.h"

/**
* \class File
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class File
{
	private:
		QString _path;
		QVector<Property> _properties;

	public:	
		File();
		File(QString path);
		~File();

		bool operator == (const File& f2)
		{
			return (_path == f2._path);
		}
		/*friend bool operator== (File &f1, File &f2);
		friend bool operator!= (File &f1, File &f2); */

		//getters
		QString getPath() { return _path; }
		const QVector<Property> getProperties() { return _properties; }
		//setters
		void addProperty(Property prop);
		void removeProp(int ind) { _properties.remove(ind); }
};
/*
///
/// \brief operator == for two Files
///
bool operator== (File &f1, File &f2)
{
	return (f1.getPath() == f2.getPath());
}

///
/// \brief operator != for two Files
///
bool operator!= (File &f1, File &f2)
{
    return !(f1 == f2);
}*/

#endif
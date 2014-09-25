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

#ifndef SERIES_H
#define SERIES_H

//Includes Qt
#include <QString>
#include <QMap>
//My includes
#include "extract.h"

/**
* \class Series
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Series
{
	private:
		QString _description;
		QVector<Extract*> _extracts;
		QVector<Property> _properties;
		
	public:	
		Series();
		Series(QString descr);
		~Series();
		
		bool operator == (const Series& s2)
		{
			return (_description == s2._description);
		}
		/*friend bool operator== (Series &s1, Series &s2);
		friend bool operator!= (Series &s1, Series &s2); */

		//getters
		QString getDescription() { return _description; }
		const QVector<Extract*> getExtracts() { return _extracts; }
		const QVector<Property> getProperties() { return _properties; }
		//setters
		void setDescription(QString descr) { _description = descr; }
		Extract* addExtract(Extract* ex);
		void addProperty(Property prop);
};
/*
///
/// \brief operator == for two Series
///
bool operator== (Series &s1, Series &s2)
{
	return (s1.getDescription() == s2.getDescription());
}

///
/// \brief operator != for two Series
///
bool operator!= (Series &s1, Series &s2)
{
    return !(s1 == s2);
}*/

#endif
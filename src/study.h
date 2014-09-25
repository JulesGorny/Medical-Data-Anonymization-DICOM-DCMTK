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

#ifndef STUDY_H
#define STUDY_H

//Includes Qt
#include <QString>
#include <QMap>
//My includes
#include "series.h"

/**
* \class Study
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Study
{
	private:
		QString _description;
		QVector<Series*> _series;
		QVector<Property> _properties;
		
	public:	
		Study();
		Study(QString descr);
		~Study();
		
		bool operator == (const Study& s2)
		{
			return (_description == s2._description);
		}
		/*friend bool operator== (Study &s1, Study &s2);
		friend bool operator!= (Study &s1, Study &s2); */

		//getters
		QString getDescription() { return _description; }
		const QVector<Series*> getSeries() { return _series; }
		const QVector<Property> getProperties() { return _properties; }
		//setters
		void setDescription(QString descr) { _description = descr; }
		Series* addSeries(Series* s);
		void addProperty(Property prop);
};
/*
///
/// \brief operator == for two Studies
///
bool operator== (Study &s1, Study &s2)
{
	return (s1.getDescription() == s2.getDescription());
}

///
/// \brief operator != for two Studies
///
bool operator!= (Study &s1, Study &s2)
{
    return !(s1 == s2);
}*/

#endif
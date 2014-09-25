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

#ifndef PROPERTY_H
#define PROPERTY_H

//Includes Qt
#include <QString>
#include <QVector>

/**
* \class Property
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Property
{
	private:
		QString _tag;
		QString _tagName;
		QString _value;

	public:	
		Property();
		Property(QString tag, QString tagName, QString val);
		~Property();

		bool operator == (const Property& p2)
		{
			return (_tag == p2._tag && _tagName == p2._tagName && _value == p2._value);
		}
		/*friend bool operator== (Property &p1, Property &p2);
		friend bool operator!= (Property &p1, Property &p2);*/

		//getters
		QString getTag() { return _tag; }
		QString getTagName() { return _tagName; }
		QString getValue() { return _value; }
		//setters
		void setTag(QString tag) { _tag = tag; }
		void setTagName(QString tagName) { _tagName = tagName; }
		void setValue(QString val) { _value = val; }
};
/*
///
/// \brief operator == for two Properties
///
bool operator== (Property &p1, Property &p2)
{
	return (p1.getTag() == p2.getTag() && p1.getTagName() == p2.getTagName() && p1.getValue() == p2.getValue());
}

///
/// \brief operator != for two Properties
///
bool operator!= (Property &p1, Property &p2)
{
    return !(p1 == p2);
}
*/
#endif
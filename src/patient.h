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

#ifndef PATIENT_H
#define PATIENT_H

#include <iostream>
//Includes Qt
#include <QString>
#include <QDate>
//My includes
#include "image.h"

/**
* \class Patient
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Patient
{
	private :
		QString _id; /// Patient's id
		QString _name; /// Patient's name
		QString _firstName; /// Patient's first name
		QString _birth; /// Patient's birth
		QVector<Image*> _imgs;
		QVector<Property> _properties;

	public : 
		Patient();
		Patient(QString i);
		Patient(QString i, QString n, QString fn, QString b);
		~Patient();
		
		friend bool operator== (Patient &p1, Patient &p2);
		friend bool operator!= (Patient &p1, Patient &p2);
		
		//getters
		QString toString();
		QString getId() { return _id; }
		QString getName() { return _name; }
		QString getFirstName() { return _firstName; }
		QString getBirth() { return _birth; }
		QVector<Image*> getImgs() { return _imgs; }
		const QVector<Property> getProperties() { return _properties; }
		//Setters
		void setBirth(QString birth) { _birth = birth; }
		Image* addImage(Image* img);
		void addProperty(Property prop);
};

#endif // PATIENT_H
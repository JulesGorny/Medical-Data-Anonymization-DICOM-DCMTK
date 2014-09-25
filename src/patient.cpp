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

#include "patient.h"

Patient::Patient()
{ }

/**
* \brief Patient constructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Patient::Patient(QString i)
{ 
	_id = i;
}

/**
* \brief Patient constructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Patient::Patient(QString i, QString n, QString fn, QString b)
{ 
	_id = i;
	_name = n;
	_firstName = fn;
	_birth = b;
}

/**
* \brief Patient destructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Patient::~Patient()
{
	qDeleteAll(_imgs.begin(), _imgs.end());
	/*for(int i=0; i<_imgs.size(); i++)
		delete _imgs[i];*/
	_imgs.clear();
}

/**
* \brief Description of a patient.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
QString Patient::toString()
{
	return _id + " : " + _name + " " + _firstName + ", " + _birth;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Image* Patient::addImage(Image *img)
{
	for(int i=0; i<_imgs.size(); i++)
	{
		if( _imgs[i]->getType() == img->getType() )
		{
			delete img;
			return _imgs[i];
		}
	}
	_imgs.append(img);
	return img;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Patient::addProperty(Property prop) 
{
	if( !_properties.contains(prop) )
		_properties.append(prop);
}

/**
* \brief operator == for two patients
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
bool operator== (Patient &p1,  Patient &p2)
{
	return (p1.getName() == p2.getName() && p1.getFirstName() == p2.getFirstName() && p1.getBirth() == p2.getBirth());
}

/**
* \brief operator != for two patients
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
bool operator!= (Patient &p1, Patient &p2)
{
    return !(p1 == p2);
}
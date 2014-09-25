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

#ifndef IMAGE_H
#define IMAGE_H

//Includes Qt
#include <QString>
//My includes
#include "study.h"

/**
* \class Image
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Image
{
	private:
		QString _type;
		QVector<Study*> _studies;

	public:	
		Image();
		Image(QString type);
		~Image();
		
		bool operator == (const Image& i2)
		{
			return (_type == i2._type);
		}
		/*friend bool operator== (Image &i1, Image &i2);
		friend bool operator!= (Image &i1, Image &i2);*/ 

		//getters
		QString getType() { return _type; }
		const QVector<Study*> getStudies() { return _studies; }
		//Setters
		void setType(QString t) { _type = t; }
		Study* addStudy(Study* s);
};
/*
///
/// \brief operator == for two images
///
bool operator== (Image &i1, Image &i2)
{
	return (i1.getType() == i2.getType());
}

///
/// \brief operator != for two images
///
bool operator!= (Image &i1, Image &i2)
{
    return !(i1 == i2);
}*/

#endif
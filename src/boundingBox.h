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

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

/**
* \class BoundingBox
* \brief This class is a boundingbox container
* It is used in the area selection by the user to anonymize annotations in DICOM files and videos
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
class BoundingBox 
{
public:
	BoundingBox() { _x1=0; _x2=0; _y1=0; _y2=0; }

	void setX1Y1(int x, int y) { _x1 = x; _y1 = y; }
	void setX2Y2(int x, int y) { _x2 = x; _y2 = y; }
	int x1() { return _x1; }
	int x2() { return _x2; }
	int y1() { return _y1; }
	int y2() { return _y2; }
	void setX1(int x) { _x1 = x; }
	void setX2(int x) { _x2 = x; }
	void setY1(int y) { _y1 = y; }
	void setY2(int y) { _y2 = y; }

	bool isForAll() { return _isForAll; }
	void setIsForAll(bool b) { _isForAll = b; }

	int imgNumber() { return _imgNumber; }
	void setImgNumber(int i) { _imgNumber = i; }

	QVector<int> notIn() { return _notIn; }
	void addToNotIn(int imgNb) { _notIn.append(imgNb); }

private:
	//int id;
	int _x1, _y1;
	int _x2, _y2;
	//A boundingbox can be for all the images
	bool _isForAll;
	//if isForAll is true, we can have images not concerned by this image 
	QVector<int> _notIn;
	//if isForAll is false, this boundingbox concern one image
	int _imgNumber;
};

#endif
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

#ifndef QIMAGELABEL_H
#define QIMAGELABEL_H

#include <iostream>
//Includes Qt
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QTextStream>
#include "boundingBox.h"

/**
* \class QImageLabel
* The QImageLabel destructor supports the destruction of QImages' pointers given in the ImageEditor's constructor's parameters
* (this pointers are then passed to QImageLabel). 
* This is different for the QVector of BoudingBox*. It has to be initialize before the call of ImageEditor constructor as well but has to be deleted after
* the use of the BoundingBox by the external function calling the ImageEditor. 
* In fact the modification isn't supported by the ImageEditor class (and even less by the QImageLabel class). 
* The function calling has to do the modification and the delete of BoundingBox*.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class QImageLabel : public QLabel
{
    Q_OBJECT
    
public:
    explicit QImageLabel(QVector<QImage*> imagesForLab, int &width, int &height, QVector<BoundingBox*>* boundingBoxes, QString &modifM, QString &displayM, int &displayedI, QWidget *parent = 0);
    ~QImageLabel();

signals:
	void signal_bbNumberChanged();

public slots:
	void drawBoundingBoxes();

private slots:
	void slot_enableMouseMoveUpdate();
	void slot_forceMoveUpdate();
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent* event);
 
private:
	void mousePress(int x, int y);
	void mouseRelease(int x, int y);
	void mouseMoving(int x2, int y2);
	void deleteRect(int x, int y);

	//Initial images
	QVector<QImage> initImgs;
	//Image with boundingbox
	QVector<QImage*> imagesForLabel;
	//List of boundingbox
	QVector<BoundingBox*>* bbs;
	//Display ratio
	float ratio;
	//Timer and boolean to restrict the number of mouseMoveEvent called
	QTimer *timer;
	bool refreshMoveEvent;
	//Timer, boolean and coords to draw the correct last rectangle if the user stop moving
	//In fact, if the user goes too fast, he has time to move far from a previous rectangle before the timer allow a new mouseMoveEvent
	QTimer *timerAfk;
	bool rectNotEnded;
	int mouseX, mouseY;
	QString* modifMode;
	//View mode (rectangles or result)
	QString* displayMode;
	int* displayedImg;
};

#endif // IMAGEEDITOR_H
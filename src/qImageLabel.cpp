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

#include "qImageLabel.h"


/**
* \brief QImageLabel constructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param *img : pointer on the image to display
* \param &width : reference to the width of the displayed image (after ratio change)
* \param &height : reference to the height of the displayed image (after ratio change)
* \param &boundingBoxes : reference for the list of boundingBoxes already created (we keep them from an image to an other)
* \param *parent : parent of the QImageLabel
**/
QImageLabel::QImageLabel(QVector<QImage*> imagesForLab, int &width, int &height, QVector<BoundingBox*>* boundingBoxes, QString &modifM, QString &displayM, int &displayedI, QWidget * parent ) : QLabel(parent) 
{
	bbs = boundingBoxes;
	modifMode = &modifM;
	displayMode = &displayM;
	displayedImg = &displayedI;
	imagesForLabel = imagesForLab;
	mouseX = 0, mouseY = 0;

	width = imagesForLabel[*displayedImg-1]->width();
	height = imagesForLabel[*displayedImg-1]->height();

	QDesktopWidget desktop;
	QRect mainScreenSize = desktop.availableGeometry(desktop.primaryScreen());
	float desktopHeight = mainScreenSize.height();
	float desktopWidth = mainScreenSize.width();

	//Calculate a perfect ratio for display
	if( height > (desktopHeight-100) || width > desktopWidth )
	{
		float ratioheight = height/(desktopHeight-100);
		float ratiowidth = width/desktopWidth;
		ratio = std::max(ratioheight, ratiowidth);
		width /= ratio;
		height = height/ratio;
	}
	else
		ratio = 1.0;
		
	//For the mouseMoveEvent (draw rectangles in real time)
	setMouseTracking(false);
	refreshMoveEvent = true;
	if( ratio != 1 )
	{
		timer = new QTimer();
		timer->connect(timer, SIGNAL(timeout()), this, SLOT(slot_enableMouseMoveUpdate()));
		timer->start(20);
		rectNotEnded = false;
		timerAfk = new QTimer();
		timer->start(20);
		timer->connect(timer, SIGNAL(timeout()), this, SLOT(slot_forceMoveUpdate()));
	}
	
	for(int nbI=0; nbI<imagesForLabel.size(); nbI++)
		*imagesForLabel[nbI] = imagesForLabel[nbI]->convertToFormat(QImage::Format_RGB32);
	for(int nbI=0; nbI<imagesForLabel.size(); nbI++)
		initImgs.append(*imagesForLabel[nbI]);

	drawBoundingBoxes();
}

/**
* QImageLabel destructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
QImageLabel::~QImageLabel()
{
	for(int nbI=imagesForLabel.size()-1; nbI>=0; nbI--)
		delete imagesForLabel[nbI];
}

/**
* \brief When we press a button on the mouse
* left click : begin the draw of a boundingbox
* right cick : remove the rectangles we clicked
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mousePressEvent(QMouseEvent * event) 
{
	if(event->button() == Qt::LeftButton)
		mousePress(event->pos().x(), event->pos().y());
	else if(event->button() == Qt::RightButton)
		deleteRect(event->pos().x(), event->pos().y());
}

/**
* \brief When we release a button on the mouse
* left click : end the draw of a boundingbox
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mouseReleaseEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton)
		mouseRelease(event->pos().x(), event->pos().y());
}

/**
* \brief When we move the mouse and if the left button is pressed we send a signal to draw the rectangle evolving
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mouseMoveEvent(QMouseEvent* event)
{
	mouseX = event->pos().x();
	mouseY = event->pos().y();

	if( refreshMoveEvent )
	{
		if( ratio != 1 )
			refreshMoveEvent = false;
		if(event->buttons() == Qt::LeftButton)
			mouseMoving(mouseX, mouseY);
	}
}

/**
* \brief Add the first point of a bounding box when we press the left button of the mouse
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mousePress(int x, int y)
{
	rectNotEnded = true;
	x = int(x*ratio);
	y = int(y*ratio);
	
	BoundingBox *bb = new BoundingBox();
	bb->setX1Y1(x, y);
	if( *modifMode == "All" )
		bb->setIsForAll(true);
	else if( *modifMode == "ThisOne" )
	{
		bb->setIsForAll(false);
		bb->setImgNumber(*displayedImg);
	}
	bbs->append(bb);
}

/**
* \brief Add the second point of a bounding box when we release the clic
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mouseRelease(int x, int y)
{
	int w = imagesForLabel[*displayedImg-1]->width(); 
	int h = imagesForLabel[*displayedImg-1]->height();
	//If we release the mouse out of the image
	if(x<0) x=0;
	if(x>w) x=w-1;
	if(y<0) y=0;
	if(y>h) y=h-1;
	
	x = int(x*ratio);
	y = int(y*ratio);
	
	BoundingBox *tmp = bbs->at(bbs->size()-1);

	//The BoundingBox box will be define by the upper left corner and the bottom right corner
	if( tmp->x1() < x )
		tmp->setX2(x);
	else
	{
		tmp->setX2(tmp->x1());
		tmp->setX1(x);
	}

	if( tmp->y1() < y )
		tmp->setY2(y);
	else
	{
		tmp->setY2(tmp->y1());
		tmp->setY1(y);
	}
	
	drawBoundingBoxes();
		
	emit signal_bbNumberChanged();
	rectNotEnded = false;
}


/**
* \brief Find the pointer of the mouse during a move and update the view to see the rectangle evolution in real time
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::mouseMoving(int x2, int y2)
{
	int w = imagesForLabel[*displayedImg-1]->width();
	int h = imagesForLabel[*displayedImg-1]->height();
	//If we release the mouse out of the image
	if(x2<0) x2=0;
	if(x2>w) x2=w-1;
	if(y2<0) y2=0;
	if(y2>h) y2=h-1;

	x2 = int(x2*ratio);
	y2 = int(y2*ratio);
	
	drawBoundingBoxes();
	BoundingBox *tmp = bbs->at(bbs->size()-1);
	int x1 = tmp->x1(), y1 = tmp->y1(), temp;
	if( x1 > x2 )
	{
		temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if( y1 > y2 )
	{
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	//Draw red rectangles
	if( *displayMode == "rectangles" )
	{
		QRgb edgeCol = qRgb(255, 165, 0); //Orange
		for(int i=x1; i<=x2; i++)
		{
			imagesForLabel[*displayedImg-1]->setPixel(i, y1, edgeCol);
			imagesForLabel[*displayedImg-1]->setPixel(i, y2, edgeCol);
		}

		for(int j=y1; j<=y2; j++)
		{
			imagesForLabel[*displayedImg-1]->setPixel(x1, j, edgeCol);
			imagesForLabel[*displayedImg-1]->setPixel(x2, j, edgeCol);
		}
	}
	else if( *displayMode == "result" )
	{
		QRgb resultCol = qRgb(0, 0, 0); //Black
		for(int i=x1; i<=x2; i++)
			for(int j=y1; j<=y2; j++)
				imagesForLabel[*displayedImg-1]->setPixel(i, j, resultCol);
	}

	//Update de view
	QPixmap img = QPixmap::fromImage(*imagesForLabel[*displayedImg-1]);
	if( ratio != 1 )
		this->setPixmap(img.scaled(imagesForLabel[*displayedImg-1]->width()/ratio, imagesForLabel[*displayedImg-1]->height()/ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else
		this->setPixmap(img);
}

/**
* \brief Allow the signal_mouseMoving to be emitted again
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::slot_enableMouseMoveUpdate()
{
	refreshMoveEvent = true;
}

/**
* \brief Force an update if the mouse stops move
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::slot_forceMoveUpdate()
{
	if( rectNotEnded && refreshMoveEvent )
		mouseMoving(mouseX, mouseY);
}

/**
* \brief Remove a BoudingBox
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::deleteRect(int x, int y)
{
	x = int(x*ratio);
	y = int(y*ratio);

	for(int nbRect=0; nbRect<bbs->size(); nbRect++)
	{
		if( bbs->at(nbRect)->x1() < x && bbs->at(nbRect)->x2() > x 
			&& bbs->at(nbRect)->y1() < y && bbs->at(nbRect)->y2() > y )
		{
			BoundingBox* tmp = bbs->at(nbRect);
			if( *modifMode == "All" )
			{
				if( (tmp->isForAll() && !tmp->notIn().contains(*displayedImg)) || (!tmp->isForAll() && tmp->imgNumber()==*displayedImg) )
				{
					delete tmp;
					bbs->remove(nbRect);
					nbRect--; //We continue the loop cause the clic can affect more than one boundingbox
				}
			}
			else if( *modifMode == "ThisOne" )
			{
				if( tmp->isForAll() && !tmp->notIn().contains(*displayedImg) )
					tmp->addToNotIn(*displayedImg);
				else if( !tmp->isForAll() && tmp->imgNumber()==*displayedImg )
				{
					delete tmp;
					bbs->remove(nbRect);
					nbRect--;
				}
			}
		}
	}
	drawBoundingBoxes();
	emit signal_bbNumberChanged();
}

/**
* \brief Draw all the bounding box in the image
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void QImageLabel::drawBoundingBoxes()
{
	*imagesForLabel[*displayedImg-1] = initImgs[*displayedImg-1];

	for(int bb = 0; bb < bbs->size(); bb++)
	{
		BoundingBox* tmpBB = bbs->at(bb);
		if( (tmpBB->isForAll() && !tmpBB->notIn().contains(*displayedImg)) || ( !tmpBB->isForAll() && tmpBB->imgNumber() == *displayedImg) )
		{
			if( *displayMode == "rectangles" )
			{
				QRgb edgeCol;
				if( !tmpBB->isForAll() && tmpBB->imgNumber() == *displayedImg )
					edgeCol = qRgb(210, 105, 13); //Orange
				else if( tmpBB->isForAll() && !tmpBB->notIn().contains(*displayedImg) )
					edgeCol = qRgb(255, 0, 0); //Red

				for(int i=tmpBB->x1(); i<=tmpBB->x2(); i++)
				{
					imagesForLabel[*displayedImg-1]->setPixel(i, tmpBB->y1(), edgeCol);
					imagesForLabel[*displayedImg-1]->setPixel(i, tmpBB->y2(), edgeCol);
				}

				for(int j=tmpBB->y1(); j<=tmpBB->y2(); j++)
				{
					imagesForLabel[*displayedImg-1]->setPixel(tmpBB->x1(), j, edgeCol);
					imagesForLabel[*displayedImg-1]->setPixel(tmpBB->x2(), j, edgeCol);
				}
			}
			else if( *displayMode == "result" )
			{
				QRgb resultCol = qRgb(0, 0, 0); //Black
				for(int i=tmpBB->x1(); i<=tmpBB->x2(); i++)
					for(int j=tmpBB->y1(); j<=tmpBB->y2(); j++)
						imagesForLabel[*displayedImg-1]->setPixel(i, j, resultCol);
			}
		}
	}

	//Write image in QLabel
	QPixmap img = QPixmap::fromImage(*imagesForLabel[*displayedImg-1]);
	if( ratio != 1 )
		this->setPixmap(img.scaled(imagesForLabel[*displayedImg-1]->width()/ratio, imagesForLabel[*displayedImg-1]->height()/ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else
		this->setPixmap(img);
}
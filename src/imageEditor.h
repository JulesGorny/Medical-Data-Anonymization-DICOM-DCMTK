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

#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <iostream>
//Includes Qt
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
//My includes
#include "qImageLabel.h"

namespace Ui {
	class Dialog;
}

/**
* \class ImageEditor
* This class creates an instance of QImageLabel to manage the display of images. 
* The QImageLabel destructor supports the destruction of QImages' pointers given in the ImageEditor's constructor's parameters
* (this pointers are then passed to QImageLabel).
* Using ImageEditor therefore requires the creation of a QVector of QImage* before creating the instance of ImageEditor without asking their destruction.
*
* This is different for the QVector of BoudingBox*. It has to be initialize before the call of ImageEditor constructor as well but has to be deleted after
* the use of the BoundingBox by the external function calling the ImageEditor. 
* In fact the modification isn't supported by the ImageEditor class. The function calling has to do the modification and the delete of BoundingBox*.
*
* An other important point is that the QVector of QImage* should contain only images of the same width/height.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class ImageEditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit ImageEditor(QVector<QImage*> imagesForLabel, QVector<BoundingBox*>* boundingBoxes, bool isVideos, QWidget *parent = 0);
    ~ImageEditor();
	bool isModifAccepted() { return _closeWithModif; }
	
signals:
	void signal_CtrlZ();
	void signal_CtrlTab();
	void signal_drawBoundingBoxes();

private slots:
	void closeEvent(QCloseEvent * ev);
	void keyPressEvent( QKeyEvent * ev );
	void wheelEvent( QWheelEvent * ev );
	void on_informations_clicked();
	void on_deleteAllRect_clicked();
	void slot_CtrlZ();
	void slot_CtrlTab();
	void updateDeleteAll();
	void slot_changeToRectangles();
	void slot_changeToResult();
	void imgModeChangedWithForAll(int status);
	void imgModeChangedWithThisOne(int status);
	void showNext();
	void showPrevious();

private:
	//Interface elements
	QCheckBox* forAll;
	QCheckBox* forThisOne;
	QToolButton* deleteAllRect;
	QRadioButton* rectanglesMode;
	QRadioButton* resultMode;
	QPushButton* validateButton;
	QLabel* imgsEvolution;
	QImageLabel* imgDisplay;
	QToolButton* _prevImg;
	QToolButton* _nextImg;
	QMessageBox* _helpMessage;
	//List of boundingbox
	QVector<BoundingBox*>* bbs;
	QString displayMode;
	QString modifMode;
	bool _closeWithModif;
	int displayedImg;
	int nbImgs;
	int maxImgReached;
	bool _isVideos;
};

#endif // IMAGEEDITOR_H
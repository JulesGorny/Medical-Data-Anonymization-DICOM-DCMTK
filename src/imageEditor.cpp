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

#include "imageEditor.h"
#include "config.h"

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
ImageEditor::ImageEditor(QVector<QImage*> imagesForLabel, QVector<BoundingBox*>* boundingBoxes, bool isVideos, QWidget *parent): QDialog(parent)
{
	setWindowTitle("Select the text to delete");
	_helpMessage = new QMessageBox(this);
	_isVideos = isVideos;
	_closeWithModif = false;

	displayedImg = 1;
	nbImgs = imagesForLabel.size();
	displayMode = "rectangles";

	bbs = boundingBoxes;
	int imgWidth, width, height;
	imgDisplay = new QImageLabel(imagesForLabel, imgWidth, height, bbs, modifMode, displayMode, displayedImg, this);
	
	width = imgWidth;
	if( width < 500 ) width = 500;

	int groupBoxWidth = 120, groupBoxHeight = 60;
	QGroupBox* imgModeGrpBox = new QGroupBox(this);
	if( isVideos )
	{
		forAll = new QCheckBox("Modify all videos", this);
		forThisOne = new QCheckBox("Modify this video", this);
	}
	else
	{
		forAll = new QCheckBox("Modify all images", this);
		forThisOne = new QCheckBox("Modify this image", this);
	}
	connect(forAll, SIGNAL(stateChanged(int)), this, SLOT(imgModeChangedWithForAll(int)));
	connect(forThisOne, SIGNAL(stateChanged(int)), this, SLOT(imgModeChangedWithThisOne(int)));
	forAll->setChecked(true);
	QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(forAll);
    vbox->addWidget(forThisOne);
    imgModeGrpBox->setLayout(vbox);
	imgModeGrpBox->setGeometry(0,0, groupBoxWidth, groupBoxHeight);

	int buttInfoWidth = 25, buttonDelWidth = 120, buttonHeight = 25, buttonNextHeight = 33, buttonNextWidth = 72;
	QToolButton* informations = new QToolButton(this);
	QPixmap pixmap(Config::IMAGES_FOLDER + "infos.png");
	QIcon ButtonIcon(pixmap);
	informations->setIcon(ButtonIcon);
	informations->setIconSize(QSize(buttInfoWidth, buttonHeight));
	informations->setGeometry(QRect((width-groupBoxWidth-buttonDelWidth-buttInfoWidth-5)/2-10 + groupBoxWidth - 25, 5, buttInfoWidth, buttonHeight));
	informations->setStyleSheet("border: 0px"); 

	rectanglesMode = new QRadioButton("Rectangles", this);
    rectanglesMode->setGeometry(QRect((width-groupBoxWidth-80-70-5)/2 + groupBoxWidth - 25, buttonHeight + 10, 100, 25));
    rectanglesMode->setChecked(true);
 
    resultMode = new QRadioButton("Result", this);
    resultMode->setGeometry(QRect((width-groupBoxWidth-80-70-5)/2 + groupBoxWidth + 65, buttonHeight + 10, 100, 25));

	validateButton = new QPushButton("Validate all\nmodifications", this);
	validateButton->setAutoDefault(false);
	if( nbImgs > 1 )
		validateButton->setEnabled(false);
	validateButton->setGeometry(QRect(width - buttonNextWidth - 5, 6, buttonNextWidth, buttonNextHeight));

	if( nbImgs == 1 )
	{
		imgsEvolution = new QLabel("File 1/" + QString::number(nbImgs), this);
		imgsEvolution->setStyleSheet("QLabel { color: green; qproperty-alignment: AlignCenter }");
	}
	else
	{
		imgsEvolution = new QLabel("File 1/" + QString::number(nbImgs) + "\n(" + QString::number(nbImgs-1) + " to verify)", this);
		imgsEvolution->setStyleSheet("QLabel { color: red; qproperty-alignment: AlignCenter }");
	}
	imgsEvolution->setGeometry(QRect(validateButton->x()-10, 30, validateButton->width()+20, 40));
	imgsEvolution->setWordWrap(true);
	QFont f( "Arial", 7);
	imgsEvolution->setFont(f);
	maxImgReached = 1;

	deleteAllRect = new QToolButton(this);
	deleteAllRect->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	QPixmap deleteAllpixmap(Config::IMAGES_FOLDER + "clear.png");
	QIcon deleteAllIcon(deleteAllpixmap);
	deleteAllRect->setIcon(deleteAllIcon);
	deleteAllRect->setText("Undo all modif. (" + QString::number(bbs->size()) + ")");
	deleteAllRect->setGeometry(QRect((width-groupBoxWidth-buttonDelWidth-buttInfoWidth-5)/2+buttInfoWidth + groupBoxWidth - 30, 5, buttonDelWidth, buttonHeight));
	
	if( nbImgs > 1 )
	{
		_prevImg = new QToolButton(this);
		QPixmap prevpixmap(Config::IMAGES_FOLDER + "previous.png");
		QIcon prevIcon(prevpixmap);
		_prevImg->setIcon(prevIcon);
		_prevImg->setIconSize(QSize(50, 50));
		_prevImg->setGeometry(QRect(informations->x()-60, 5, 50, 50));
		_prevImg->setStyleSheet("border: 0px");
		_prevImg->setEnabled(false);

		_nextImg = new QToolButton(this);
		QPixmap nextpixmap(Config::IMAGES_FOLDER + "next.png");
		QIcon nextIcon(nextpixmap);
		_nextImg->setIcon(nextIcon);
		_nextImg->setIconSize(QSize(50, 50));
		_nextImg->setGeometry(QRect(deleteAllRect->x() + deleteAllRect->width() + 20, 5, 50, 50));
		_nextImg->setStyleSheet("border: 0px");
		
		connect(_prevImg, SIGNAL(clicked()), this, SLOT(showPrevious()));
		connect(_nextImg, SIGNAL(clicked()), this, SLOT(showNext()));
	}

	imgDisplay->setFixedHeight(height);
	imgDisplay->setFixedWidth(width);
	imgDisplay->setGeometry(QRect(width/2 - imgWidth/2, buttonHeight + 10 + 30, height, width));
	this->setMinimumSize(width,height + buttonHeight + 10 + 30);
	this->setMaximumSize(width,height + buttonHeight + 10 + 30);

	connect(this, SIGNAL(signal_CtrlZ()), this, SLOT(slot_CtrlZ()));
	connect(informations, SIGNAL(clicked()), this, SLOT(on_informations_clicked()));
	connect(deleteAllRect, SIGNAL(clicked()), this, SLOT(on_deleteAllRect_clicked()));
	connect(this, SIGNAL(signal_drawBoundingBoxes()), imgDisplay, SLOT(drawBoundingBoxes()));
	connect(imgDisplay, SIGNAL(signal_bbNumberChanged()), this, SLOT(updateDeleteAll()));
	connect(rectanglesMode, SIGNAL(toggled(bool)), this, SLOT(slot_changeToRectangles()));
	connect(resultMode, SIGNAL(toggled(bool)), this, SLOT(slot_changeToResult()));
	connect(this, SIGNAL(signal_CtrlTab()), this, SLOT(slot_CtrlTab()));
	connect(validateButton, SIGNAL(clicked()), this, SLOT(accept()));
	
	if( !_isVideos )
		setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint );
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
ImageEditor::~ImageEditor()
{
	delete deleteAllRect;

	delete rectanglesMode;
	delete resultMode;

	delete validateButton;
	delete imgsEvolution;

	delete forAll;
	delete forThisOne;

	delete imgDisplay;
	delete _helpMessage;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::closeEvent(QCloseEvent * ev)
{
    if( !_isVideos )
		ev->ignore();
}

/**
* \brief capture the Ctrl Z and Ctrl Tab events
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::keyPressEvent( QKeyEvent * ev )
{
	if( ev->modifiers() == Qt::ControlModifier )
	{
		if( ev->key() == Qt::Key_Z )
			emit signal_CtrlZ();
		else if( ev->key() == Qt::Key_Tab )
			emit signal_CtrlTab();
	}
	else if( ev->key() == Qt::Key_F4 )
		return;
}


/**
* \brief capture the wheelEvent
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::wheelEvent( QWheelEvent * ev )
{
	if(ev->delta() > 0)  
		showPrevious();
	else  
		showNext();  
}

/**
* \brief if we want to display the next image with a wheelEvent
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::showNext()
{
	if( displayedImg < nbImgs )
	{
		displayedImg++;
		if( maxImgReached < displayedImg )
			maxImgReached = displayedImg;

		if( maxImgReached == nbImgs )
		{
			validateButton->setEnabled(true);
			imgsEvolution->setText("File " + QString::number(displayedImg) + "/" + QString::number(nbImgs));
			imgsEvolution->setStyleSheet("QLabel { color: green; qproperty-alignment: AlignCenter }");
			_nextImg->setEnabled(false);
		}
		else
			imgsEvolution->setText("File " + QString::number(displayedImg) + "/" + QString::number(nbImgs) + "\n(" + QString::number(nbImgs-maxImgReached) + " to verify)");
		emit signal_drawBoundingBoxes();
		updateDeleteAll();
		_prevImg->setEnabled(true);
	}
}

/**
* \brief if we want to display the previous image with a wheelEvent
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::showPrevious()
{
	if( displayedImg > 1 )
	{
		_nextImg->setEnabled(true);
		displayedImg--;
		if( maxImgReached == nbImgs )
			imgsEvolution->setText("File " + QString::number(displayedImg) + "/" + QString::number(nbImgs));
		else
			imgsEvolution->setText("File " + QString::number(displayedImg) + "/" + QString::number(nbImgs) + "\n(" + QString::number(nbImgs-maxImgReached) + " to verify)");
		emit signal_drawBoundingBoxes();
		updateDeleteAll();
		if( displayedImg == 1 )
			_prevImg->setEnabled(false);
	}
}

/**
* \brief if we change the image modification mode
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::imgModeChangedWithForAll(int status)
{
	if( forAll->isChecked() )
	{
		forThisOne->setChecked(false);
		modifMode = "All";
	}
	else
	{
		forThisOne->setChecked(true);
		modifMode = "ThisOne";
	}
}

/**
* \brief if we change the image modification mode
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::imgModeChangedWithThisOne(int status)
{
	if( forThisOne->isChecked() )
	{
		forAll->setChecked(false);
		modifMode = "ThisOne";
	}
	else
	{
		forAll->setChecked(true);
		modifMode = "All";
	}
}

/**
* \brief if we clicked on delete all, we update the number displayed on the button 'delete all'
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void ImageEditor::updateDeleteAll()
{
	int nbRectOnThisOne = 0;
	for(int bb=0; bb<bbs->size(); bb++)
	{
		BoundingBox* tmp = (*bbs)[bb];
		if( (tmp->isForAll() && !tmp->notIn().contains(displayedImg)) || tmp->imgNumber() == displayedImg )
			nbRectOnThisOne++;
	}
	deleteAllRect->setText("Undo all modif. (" + QString::number(nbRectOnThisOne) + ")");
}

/**
* \brief Remove the last boundingbox created
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::slot_CtrlZ()
{
	if( bbs->size() > 0 )
	{
		if( modifMode == "All" )
		{
			for(int b=bbs->size()-1; b>=0; b--)
			{
				BoundingBox* tmp = bbs->at(b);
				if( (tmp->isForAll() && !tmp->notIn().contains(displayedImg)) || (!tmp->isForAll() && tmp->imgNumber()==displayedImg) )
				{
					delete tmp;
					bbs->remove(b);
					break;
				}
			}
		}
		else if( modifMode == "ThisOne" )
		{
			for(int b=bbs->size()-1; b>=0; b--)
			{
				BoundingBox* tmp = bbs->at(b);
				if( tmp->isForAll() && !tmp->notIn().contains(displayedImg) )
				{
					tmp->addToNotIn(displayedImg);
					break;
				}
				else if( !tmp->isForAll() && tmp->imgNumber() == displayedImg )
				{
					delete tmp;
					bbs->remove(b);
					break;
				}
			}
		}

		emit signal_drawBoundingBoxes();
		updateDeleteAll();
	}
}

/**
* \brief Remove the last boundingbox created
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::slot_CtrlTab()
{
	if( displayMode == "rectangles" )
	{
		displayMode = "result";
		resultMode->setChecked(true);
		resultMode->setChecked(true);
	}
	else if( displayMode == "result" )
	{
		displayMode = "rectangles";
		rectanglesMode->setChecked(true);
	}
	emit signal_drawBoundingBoxes();
}

/**
* \brief Delete all the boundingbox
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::on_deleteAllRect_clicked()
{
	//We clear the BoundingBox list after the end of a serie
	for(int nbRect=bbs->size()-1; nbRect>=0; nbRect--)
	{
		BoundingBox* tmp = (*bbs)[nbRect];
		if( modifMode == "All" )
		{
			delete tmp;
			bbs->remove(nbRect);
		}
		else if( modifMode == "ThisOne" )
		{
			if( tmp->isForAll() && !tmp->notIn().contains(displayedImg) )
				tmp->addToNotIn(displayedImg);
			else if( !tmp->isForAll() && tmp->imgNumber() == displayedImg )
			{
				delete tmp;
				bbs->remove(nbRect);
			}
		}
	}

	emit signal_drawBoundingBoxes();
	updateDeleteAll();
}

/**
* \brief Change the display mode to rectangles
* The user can see the rectangles he draws
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::slot_changeToRectangles()
{
	displayMode = "rectangles";
	emit signal_drawBoundingBoxes();
}

/**
* \brief Change the display mode to result
* The user can see the result after the suppression of selected boundingbox
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::slot_changeToResult()
{
	displayMode = "result";
	emit signal_drawBoundingBoxes();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void ImageEditor::on_informations_clicked()
{
	QString msg = "You have to draw rectangles around the text to delete.<br>"
									"Draw a rectangle = Click, Move while keeping clicked, Release<br><br>"
									"The rectangles will be filled with black pixels.<br>"
									"You need to verify every image before the validation.<br>"
									"You need to browse the images with the wheel of the mouse.<br><br>"
									"You can change the view mode from rectangles (rectangles are visible)<br>"
									"to result (to display the result after modification).<br><br>"
									"At the top left corner, you have to choose if you want the add/suppression of<br>"
									"rectangles has to be made on the displayed image or on all of them.<br><br>"
									"<b>Shortcuts</b><br>"
									"- Right click on a rectangle to delete it.<br>"
									"- Ctrl+Z delete the last rectangle.<br>"
									"- Ctrl+Tab change the mode.<br><br>"
									"<b>What to delete ?</b><br>"
									"Any informations about the patient, the doctor or the hospital<br>"
									"(names, dates, ages...)<br>"
									"Generally, all that does not help the researcher should be deleted.<br><br>";

	if( _isVideos )
		msg += "<b>Output videos</b><br>"
				"WMV, AVI, MPG and MOV videos keeps their extension. <br>"
				"All the other formats become MPG.<br>"
				"You can read all the videos with VLC. Windows Media may be problematic<br>"
				"since it doesn't support all formats (for example for MOV files).<br>";

	_helpMessage->setWindowTitle("How does it work");
	_helpMessage->setText(msg);
	_helpMessage->setWindowModality(Qt::NonModal);
	_helpMessage->show();
}
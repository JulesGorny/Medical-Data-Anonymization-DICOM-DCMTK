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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "tools.h"
#include "xmlTools.h"
#include "config.h"
#include "tools.h"


/**
* \brief Prepare the main window 
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	//Then Qt knows what is Anonymization for the signals/slots
	qRegisterMetaType<Anonymization>("Anonymization");

	_toDo = new QList<Anonymization>();
	_patientList = new QVector<Patient>();
	_remainingAnon = new int();
	(*_remainingAnon) = 0;
	_scan = new Scanning(_toDo, &_workDoneBool, &_scanCanStartWorking);	
	_anonymizer = new Anonymizer(_patientList, &_workDoneBool, _remainingAnon, ui);
	_helpMessage = new QMessageBox(this);

	QDir().mkpath(Config::NON_ANONYMOUS_DOC_PATH);
	QDir().mkpath(Config::PATIENT_PV_PATH);

	_browseFolder = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
	
    ui->setupUi(this);

	ui->anonymizedFilesPathText->setText(Config::DEFAULT_DESTINATION_PATH);
	ui->remainingAnonLabel->setText("0 remaining");
	ui->remainingAnonLabel->setAlignment(Qt::AlignHCenter);
	ui->remainingAnonLabel->setVisible(false);
			
	ui->tabWidget->setTabText(0, "Name to Id");
	ui->tabWidget->setTabText(1, "Id to Name");
	ui->tabWidgetProgress->setTabText(0, "     Detailled     ");
	ui->tabWidgetProgress->setTabText(1, "     Undetailled     ");
	
	ui->nonAnonymousDocsLabelNtoID->setWordWrap(true);
    ui->nonAnonymousDocsLabelNtoID->setAlignment(Qt::AlignCenter);
	ui->nonAnonymousDocsLabelIDtoN->setWordWrap(true);
    ui->nonAnonymousDocsLabelIDtoN->setAlignment(Qt::AlignCenter);
	ui->progressCurrentAnonLabel->setWordWrap(true);
	ui->progressCurrentFileLabel->setWordWrap(true);
	
	QPixmap pixmap(Config::IMAGES_FOLDER + "resetDestination.png");
	QIcon ButtonIcon(pixmap);
	ui->resetDefaultPathButton->setIcon(ButtonIcon);
	ui->resetDefaultPathButton->setIconSize(QSize(28,28));
	ui->resetDefaultPathButton->setFlat(true);
	ui->resetDefaultPathButton->setToolTip("This change the default path for the common folder\nbetween the Anonymizer and the Viewer.\nUse it only if you know that the viewer will have the\nsame folder to build the database!");
		
	QPixmap pixmap2(Config::IMAGES_FOLDER + "browse.png");
	QIcon ButtonIcon2(pixmap2);	
	ui->anonymizedFilesPathButton->setIcon(ButtonIcon2);
	ui->anonymizedFilesPathButton->setIconSize(QSize(28,28));
	ui->anonymizedFilesPathButton->setFlat(true);
	ui->anonymizedFilesPathButton->setToolTip("This change the destination path\nUse it only if you want to export out of the database.");
	
	QPixmap pixmap3(Config::IMAGES_FOLDER + "translate.png");
	QIcon ButtonIcon3(pixmap3);
	ui->IDToNameButton->setIcon(ButtonIcon3);
	ui->IDToNameButton->setIconSize(QSize(28,28));
	ui->IDToNameButton->setFlat(true);
	ui->NameToIDButton->setIcon(ButtonIcon3);
	ui->NameToIDButton->setIconSize(QSize(28,28));
	ui->NameToIDButton->setFlat(true);

	ui->ulButtonIDtoN->setEnabled(false);
	ui->ulButtonNtoID->setEnabled(false);
	ui->dlButtonIDtoN->setEnabled(false);
	ui->dlButtonNtoID->setEnabled(false);
	
	ui->destinationNoMatch->setStyleSheet("QLabel { color : red; }");

	QPixmap pixmap4(Config::IMAGES_FOLDER + "infos.png");
	QIcon ButtonIcon4(pixmap4);
	ui->helpButton->setIcon(ButtonIcon4);
	ui->helpButton->setIconSize(QSize(28,28));
	ui->helpButton->setFlat(true);
	
	//When the destination folder text edit is modified
	connect(ui->anonymizedFilesPathText, SIGNAL(textChanged(const QString&)), this, SLOT(slot_destinationChange()));

	//When we modify something in the ID to Name or Name to ID, we disable the upload/download until the next validation
	connect(ui->IdEditIDtoN, SIGNAL(textChanged(const QString&)), this, SLOT(on_editIDtoN()));  
	connect(ui->FirstnEditNtoID, SIGNAL(textChanged(const QString&)), this, SLOT(on_editNtoID()));  
	connect(ui->NameEditNtoID, SIGNAL(textChanged(const QString&)), this, SLOT(on_editNtoID()));  
	
	//Enable the enter use in the in the ID to Name or Name to ID QLineEdit
	connect(ui->IdEditIDtoN, SIGNAL(returnPressed()), this, SLOT(on_IDToNameButton_clicked()));  
	connect(ui->FirstnEditNtoID, SIGNAL(returnPressed()), this, SLOT(on_NameToIDButton_clicked()));  
	connect(ui->NameEditNtoID, SIGNAL(returnPressed()), this, SLOT(on_NameToIDButton_clicked()));

	//Connects between MainWindow, Anonymizer and Scan
	connect(_scan, SIGNAL(anonymize(Anonymization)), _anonymizer, SLOT(slot_AskForAnonymize(Anonymization)));
	connect(_scan, SIGNAL(anonymizeFiles(Anonymization)), _anonymizer, SLOT(slot_AskForAnonymizeFiles(Anonymization)));
	connect(ui->cancelCurrentAnonymisationButton, SIGNAL(clicked()), _anonymizer, SLOT(slot_cancelAnonymization()));
	
	setVisibleMatchDestination(false);
	createScanningThread();
}


/**
* \brief Destructor of MainWindow
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
MainWindow::~MainWindow()
{
	delete _remainingAnon;
    delete ui;
}

/**
* \brief Slot for the clic on \b xmlDataButton. Open an \b XML file and get data
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_xmlDataButton_clicked()
{
	QString outPutFolder = ui->anonymizedFilesPathText->text();
	if( outPutFolder == "" )
		addProgressMessage(ui, "You have to choose a destination folder.\n", Qt::red, "", true);
	else
	{
		///- BROWSE AND FIND THE FILE OF DATA
		QString fileName = QFileDialog::getOpenFileName(0, tr("Open XML File"), _browseFolder, tr("Fichier XML (*.xml)"));

		//If a file is selected
		if(fileName != NULL)
		{
			_scanCanStartWorking = false; //The scanning thread can't start until scanCanStartWorking is true
			Anonymization a;
			a._fileNames.append(fileName);
			a._anonType = "xmlFile";
			a._destinationMatch = ( outPutFolder == Config::DEFAULT_DESTINATION_PATH );
			a._bddFolder = outPutFolder;
			_toDo->append(a);

			_browseFolder = fileName;
			_browseFolder = _browseFolder.replace("\\", "/").left(_browseFolder.lastIndexOf("/"));

			(*_remainingAnon)++;
			ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
			ui->remainingAnonLabel->setVisible(true);
			addProgressMessage(ui, "\nAnonymisation added to the queue! (type : xmlFile, fileSelected : " + fileName + ")\n", Qt::darkGreen, "", true);
			_scanCanStartWorking = true;
		}
		//End If a file is selected
	}
}

/**
* \brief This function allow a user to select different laparoscopy files to integrate them in the data base
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_videoButton_clicked()
{
	QString outPutFolder = ui->anonymizedFilesPathText->text();
	if( outPutFolder == "" )
		addProgressMessage(ui,"You have to choose a destination folder.\n", Qt::red, "", true);
	else
	{
		QStringList files = QFileDialog::getOpenFileNames(0, tr("Select one or more videos"), _browseFolder);

		if( files.size() > 0 )
		{
			_scanCanStartWorking = false; //The scanning thread can't start until scanCanStartWorking is true
			Anonymization a;
			a._fileNames.append(files);
			a._anonType = "videos";
			a._destinationMatch = ( outPutFolder == Config::DEFAULT_DESTINATION_PATH );
			a._bddFolder = outPutFolder;
			_toDo->append(a);

			_browseFolder = files[0];
			_browseFolder = _browseFolder.replace("\\", "/").left(_browseFolder.lastIndexOf("/"));

			(*_remainingAnon)++;
			ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
			ui->remainingAnonLabel->setVisible(true);
			QString pathElems = files[0].replace(QString("\\"), QString("/")).left(files[0].lastIndexOf("/"));
			addProgressMessage(ui, "\nAnonymisation added to the queue! (type : videos, folder : " + pathElems + ")\n", Qt::darkGreen, "", true);
			_scanCanStartWorking = true;
		}
	}
}

/**
* \brief Slot for the clic on \b dicomDirButton. Open a \b dicom file and get data
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_dicomDirButton_clicked()
{
	QString outPutFolder = ui->anonymizedFilesPathText->text();
	if( outPutFolder == "" )
		addProgressMessage(ui,"You have to choose a destination folder.\n", Qt::red, "", true);
	else
	{
		///- OPEN A DICOMDIR FILE
		QString fileName = QFileDialog::getOpenFileName(0, tr("Open DICOMDIR File"), _browseFolder);

		if( fileName != "" )
		{
			_scanCanStartWorking = false; //The scanning thread can't start until scanCanStartWorking is true
			Anonymization a;
			a._fileNames.append(fileName);
			a._anonType = "dicomDir";
			a._destinationMatch = ( outPutFolder == Config::DEFAULT_DESTINATION_PATH );
			a._bddFolder = outPutFolder;
			_toDo->append(a);

			_browseFolder = fileName;
			_browseFolder = _browseFolder.replace("\\", "/").left(_browseFolder.lastIndexOf("/"));

			(*_remainingAnon)++;
			ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
			ui->remainingAnonLabel->setVisible(true);
			addProgressMessage(ui, "\nAnonymisation added to the queue! (type : dicomDir, fileSelected : " + fileName + ")\n", Qt::darkGreen, "", true);
			_scanCanStartWorking = true;
		}
	}
}

/**
* \brief Slot for the clic on \b dicomFilesButton. Open a \b as much as dicom files you want and get data
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_dicomFilesButton_clicked()
{
	QString outPutFolder = ui->anonymizedFilesPathText->text();
	if( outPutFolder == "" )
		addProgressMessage(ui,"You have to choose a destination folder.\n", Qt::red, "", true);
	else
	{
		QStringList files = QFileDialog::getOpenFileNames(0, tr("Select one or more DICOM files"), _browseFolder);

		if( files.size() > 0 )
		{
			_scanCanStartWorking = false; //The scanning thread can't start until scanCanStartWorking is true

			Anonymization a;
			a._fileNames.append(files);
			a._anonType = "dicomFiles";
			a._destinationMatch = ( outPutFolder == Config::DEFAULT_DESTINATION_PATH );
			a._bddFolder = outPutFolder;
			_toDo->append(a);

			_browseFolder = files[0];
			_browseFolder = _browseFolder.replace("\\", "/").left(_browseFolder.lastIndexOf("/"));

			(*_remainingAnon)++;
			ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
			ui->remainingAnonLabel->setVisible(true);
			QString pathElems = files[0].replace(QString("\\"), QString("/")).left(files[0].lastIndexOf("/"));
			addProgressMessage(ui, "\nAnonymisation added to the queue! (type : dicomFiles, folder : " + pathElems + ")\n", Qt::darkGreen, "", true);
			_scanCanStartWorking = true;
		}
	}
}


/**
* \brief Browse for the destination file (it last until the Anonymizer is closed).
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_anonymizedFilesPathButton_clicked()
{
	QString fodlerName = QFileDialog::getExistingDirectory();

	if( fodlerName != "" )
	{
		_anonymousFilesPathText = fodlerName;
		ui->anonymizedFilesPathText->setText(_anonymousFilesPathText);
	}
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::slot_destinationChange()
{
	if( ui->anonymizedFilesPathText->text() != Config::DEFAULT_DESTINATION_PATH )
		setVisibleMatchDestination(true);
	else
		setVisibleMatchDestination(false);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_useViewerDBButton_clicked()
{
	ui->anonymizedFilesPathText->setText(Config::DEFAULT_DESTINATION_PATH);
}

/**
* \brief Browse for the destination file. It will rewrite the default path (common folder between Anonymizer and Viewer).
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_resetDefaultPathButton_clicked()
{
	QString fodlerName = QFileDialog::getExistingDirectory();

	if( fodlerName != "" )
	{
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Careful", "This will change the default path for the common folder between the Anonymizer and the Viewer. Are you sure you want to do this ?", QMessageBox::Yes|QMessageBox::No);
		if (reply == QMessageBox::Yes) 
		{
			Config::DEFAULT_DESTINATION_PATH = fodlerName;
			ui->anonymizedFilesPathText->setText(Config::DEFAULT_DESTINATION_PATH);

			QFile tagsFile(Config::COMMON_DB_WITH_VIEWER);	
			if(tagsFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
			{
				QTextStream in(&tagsFile);
				in << Config::DEFAULT_DESTINATION_PATH;
			}

			Config::ANONYMIZER_FOLDER = Config::DEFAULT_DESTINATION_PATH + "\\AnonymizerFiles";
			Config::PATIENT_PV_PATH = Config::ANONYMIZER_FOLDER + "\\Private\\PrivateResultData";
			Config::NON_ANONYMOUS_DOC_PATH = Config::ANONYMIZER_FOLDER + "\\Private\\NonAnonymousDocs";
			QDir().mkpath(Config::PATIENT_PV_PATH);
			QDir().mkpath(Config::NON_ANONYMOUS_DOC_PATH);
		}
	}
}

/**
* \brief This function get the ID of a patient for a given Name.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_NameToIDButton_clicked()
{
	ui->IdEditNtoID->setText("");
	if( ui->NameEditNtoID->text() == "" && ui->FirstnEditNtoID->text() == "" )
		return;

	QString firstName, Name; 
	Name = ui->NameEditNtoID->text().toLower();
	firstName = ui->FirstnEditNtoID->text().toLower();

	//We prepare a list of patient corresponding with the given informations.
	QVectorIterator<Patient> i(*_patientList);
	QStringList patientIds;
	while (i.hasNext())
	{
		Patient p = i.next();
		//Si on a un nom
		if( Name != "" )
		{
			if( Name.contains(p.getName().toLower()) || p.getName().toLower().contains(Name) )
			{
				//Et un prénom
				if (firstName != "" )
				{
					if( firstName.contains(p.getFirstName().toLower()) || p.getFirstName().toLower().contains(firstName) )
						patientIds << p.getId();
				}
				//Mais pas de prénom
				else
					patientIds << p.getId();
			}
		}
		//Si on a pas de nom mais un prénom
		else if (firstName != "" && ( firstName.contains(p.getFirstName().toLower()) || p.getFirstName().toLower().contains(firstName) ) )
			patientIds << p.getId();
	}

	//If the list contains more than one result, we can say the user the different results
	if( patientIds.size() > 1 )
	{
		QString patientsStr = patientIds.at(0);
		for(int pa=1; pa<patientIds.size(); pa++)
			patientsStr += ", " + patientIds.at(pa);
		QMessageBox::warning(this, "Multiple patients", QString::number(patientIds.size()) + " patient's Id fit to your research : " + patientsStr + ".");
	}
	//If the list contains only one result, we fill the result labels
	else if( patientIds.size() == 1 )
	{
		ui->IdEditNtoID->setText(patientIds.at(0));
		setEnableDLandULButtonsNtoID(true);
		return;
	}
	setEnableDLandULButtonsNtoID(false);
}

/**
* \brief This function get the Name of a patient for a given ID.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_IDToNameButton_clicked()
{
	ui->NameEditIDtoN->setText("");
	ui->FirstnEditIDtoN->setText("");
	if( ui->IdEditIDtoN->text() == "" )
		return;
		
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(5) << ui->IdEditIDtoN->text().toStdString().c_str();
	QString toCompare = QString::fromStdString(ss.str());

	ui->IdEditIDtoN->setText(toCompare);
	QVectorIterator<Patient> i(*_patientList);
	while (i.hasNext())
	{
		Patient p = i.next();
		if( p.getId() == toCompare )
		{
			ui->NameEditIDtoN->setText(p.getName());
			ui->FirstnEditIDtoN->setText(p.getFirstName());
			setEnableDLandULButtonsIDtoN(true);
			return;
		}
	}
	setEnableDLandULButtonsIDtoN(false);
}

/**
* \brief Slot for the clic on dlButton from Name to Id interface.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_dlButtonNtoID_clicked()
{
	dlNonAnonymousDocs(ui->IdEditNtoID->text());
}

/**
* \brief Slot for the clic on dlButton from Id to Name interface.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_dlButtonIDtoN_clicked()
{
	dlNonAnonymousDocs(ui->IdEditIDtoN->text());
}

/**
* \brief this function asks the user where he wants the non anonymized additionnal files to be copied and copy them.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param QString patientID : Id of the patient we want the files. If the Id of the visible interface isn't filled, the research won't begin.
*/
void MainWindow::dlNonAnonymousDocs(QString patientID)
{
	QString fodlerName = QFileDialog::getExistingDirectory(this, "Choose Destination Directory");

	//If the user has choosen a destination folder for the files
	if( fodlerName != "" )
	{
		QStringList filesList;
		int nbBadFile = 0;
		
		//Browse the folder in the non anonymous documents folder
		QDirIterator dirIterator(Config::NON_ANONYMOUS_DOC_PATH, QDir::Dirs | QDir::NoDotAndDotDot);
		while(dirIterator.hasNext())
		{
			QString actualFolderPath = dirIterator.next();
			QString actualFolderName = actualFolderPath.right(actualFolderPath.size() - actualFolderPath.lastIndexOf("/") - 1);
			
			//If we find a folder named with the patient's ID
			if( actualFolderName == patientID )
			{
				QDir directory(Config::NON_ANONYMOUS_DOC_PATH + "\\" + patientID);
				filesList = directory.entryList(QDir::Files);
				//We browse the files and do the copy/paste
				for(int file=0; file<filesList.size(); file++)
				{
					if( !copyFile(Config::NON_ANONYMOUS_DOC_PATH + "\\" + patientID + "\\" + filesList.at(file), fodlerName + "\\" + filesList.at(file), false, ui) )
						nbBadFile++;
				}
			}
		}

		if( filesList.size() > 0 )
		{
			if( nbBadFile == 0 )
				QMessageBox::information(this, "Good", "The download was successful. (" + QString::number(filesList.size()) + " files).");
			else
				QMessageBox::information(this, "Bad", "The download has fail for " + QString::number(nbBadFile) + "/" + QString::number(filesList.size()) + " files.");
		}
		else
			QMessageBox::information(this, "Sorry", "There is no file for this patient.");
	}
}

/**
* \brief Slot for the clic on ulButton from Name to Id interface.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_ulButtonNtoID_clicked()
{
	ulNonAnonymousDocs(ui->IdEditNtoID->text());
}

/**
* \brief Slot for the clic on ulButton from Id to Name interface.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_ulButtonIDtoN_clicked()
{
	ulNonAnonymousDocs(ui->IdEditIDtoN->text());
}

/**
* \brief this function asks the user where he wants the non anonymized additionnal files to be copied and copy them.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param QString patientID : Id of the patient we want the files. If the Id of the visible interface isn't filled, the research won't begin.
*/
void MainWindow::ulNonAnonymousDocs(QString patientID)
{
	int nbBadFile = 0;
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Select one or more videos"), "/home");

	if( files.size() > 0 )
	{
		QDir().mkpath(Config::NON_ANONYMOUS_DOC_PATH + "\\" + patientID);
		for(int f=0; f<files.size(); f++)
		{
			QString fileName = files.at(f).right(files.at(f).size() - files.at(f).lastIndexOf("\\") - 1);
			if( !copyFile(files.at(f), Config::NON_ANONYMOUS_DOC_PATH + "\\" + patientID + "\\" + fileName, false, ui) )
				nbBadFile++;
		}

		if( nbBadFile == 0 )
			QMessageBox::information(this, "Good", "The upload was successful. (" + QString::number(files.size()) + " files).");
		else
			QMessageBox::information(this, "Bad", "The upload has fail for " + QString::number(nbBadFile) + "/" + QString::number(files.size()) + " files.");
	}
}


/**
* \brief Slot for the edition of the ID (we need to disable the upload and download buttons)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_editIDtoN()
{
	setEnableDLandULButtonsIDtoN(false);
	ui->NameEditIDtoN->setText("");
	ui->FirstnEditIDtoN->setText("");
}


/**
* \brief Slot for the edition of the Name or the Firstname (we need to disable the upload and download buttons)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void MainWindow::on_editNtoID()
{
	setEnableDLandULButtonsNtoID(false);
	ui->IdEditNtoID->setText("");
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::setEnableDLandULButtonsIDtoN(bool b)
{
	ui->dlButtonIDtoN->setEnabled(b);
	ui->ulButtonIDtoN->setEnabled(b);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::setEnableDLandULButtonsNtoID(bool b)
{
	ui->dlButtonNtoID->setEnabled(b);
	ui->ulButtonNtoID->setEnabled(b);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::setEnableDLandULButtons(bool b)
{
	setEnableDLandULButtonsNtoID(b);
	setEnableDLandULButtonsIDtoN(b);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::setVisibleMatchDestination(bool b)
{
	ui->destinationNoMatch->setVisible(b);
	ui->useViewerDBButton->setVisible(b);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::createScanningThread()
{
	QThread* thread = new QThread;
	_scan->moveToThread(thread);
	connect(thread, SIGNAL(started()), _scan, SLOT(process()));
	connect(_scan, SIGNAL(finished()), thread, SLOT(quit()));
	connect(_scan, SIGNAL(finished()), _scan, SLOT(deleteLater()));
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	thread->start();
}

/**
* \brief If the window is resized, we just make the progress text area follow the movement an all the other items keep their places
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::resizeEvent (QResizeEvent * event)
{
	ui->tabWidgetProgress->setGeometry(ui->tabWidgetProgress->x(), ui->tabWidgetProgress->y(), ui->tabWidgetProgress->width(), 291 + (this->height()-570));
	ui->progressTextDetailled->setGeometry(ui->progressTextDetailled->x(), ui->progressTextDetailled->y(), ui->progressTextDetailled->width(), 267 + (this->height()-570));
	ui->progressTextSummary->setGeometry(ui->progressTextSummary->x(), ui->progressTextSummary->y(), ui->progressTextSummary->width(), 267 + (this->height()-570));
	ui->groupBoxBot->setGeometry(ui->groupBoxBot->x(), 400 + (this->height()-570), ui->groupBoxBot->width(), ui->groupBoxBot->height());
}

/**
* \brief Display a message to explain how to anonymize a file correctly
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::on_helpButton_clicked()
{
	_helpMessage->setWindowTitle("How does it work");
	_helpMessage->setText("<u>You have 4 buttons for the different kind of anonymization :</u><br><br>"
									"<b>Laparoscopy videos linked with a XML file</b><br>"
									"You have to browse and select the XML file. The software will be able to find and anonymize the patient informations. It will find the videos as well.<br><br>"
									"<b>Laparoscopy videos only</b><br>"
									"You need to browse and select all the videos you want to copy in the database<br>"
									"The software will ask you if you know the patient's name. If you don't the data will be stored under the ID '99999'. This ID contains all the data from unknown patients.<br><br>"
									"<b>DICOM files linked with a DICOMDIR file</b><br>"
									"You have to browse and select the DICOMDIR, the software will be able to find all the attached DICOM files.<br><br>"
									"<b>DICOM files only</b><br>"
									"You need to browse and select all the DICOM files you want to anonymize and copy in the database<br>"
									"<br><br><br>"
									"The bottom portion allow you to find the name of a patient from an ID and vice versa. Once an ID is found, you can download or upload documents that can not be anonymized such as informed consent.");
	_helpMessage->setWindowModality(Qt::NonModal);
	_helpMessage->show();
}

/**
* \brief Rewrite the close event. It can't happen if an anonymization is running
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void MainWindow::closeEvent(QCloseEvent * event)
{
	if( *_remainingAnon > 0 )
		event->ignore();
}
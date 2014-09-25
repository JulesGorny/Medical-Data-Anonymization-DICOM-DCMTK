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

#include "anonymizer.h"

/**
* \brief Anonymizer constructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Anonymizer::Anonymizer(QVector<Patient>* pl, bool *wdBool, int* remainingAno, Ui::MainWindow *ui)
{
	_workDoneBool = wdBool;
	_isCanceled = false;
	_canCopy = true;
	_copyHasToStop = false;
	_remainingAnon = remainingAno;

	//Open the file of anonymous tags
	QFile tagsFile(Config::INTERESTING_FILES_PATH + "\\anonymousTags.txt");
	if( !tagsFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		return;
	QTextStream in(&tagsFile);
	//Browse the file
	while( !in.atEnd() ) 
	{
		QString line = in.readLine();
		//If the line isn't a comment
		if( !line.startsWith("#") )
		{
			QStringList splittedLine;
			QStringList splittedTag;
			if(line.trimmed() != "")
			{
				splittedLine = line.split(" __ ");
				splittedTag = splittedLine.at(0).trimmed().split("|");
				//Create the tag to remove
				DcmTagKey tagK = DcmTagKey();
				bool isOk;
				tagK.setGroup(("0x" + splittedTag.at(0)).toUInt(&isOk, 16));
				tagK.setElement(("0x" + splittedTag.at(1)).toUInt(&isOk, 16));
				
				DcmTag tag = DcmTag(tagK);
				_anonymousTags.append(tag.toString().c_str());
			}
		}
	}

	this->ui = ui;

	_patientList = pl;
	refreshPatientList();
}

void Anonymizer::slot_AskForAnonymize(Anonymization a)
{
	if( a._anonType == "xmlFile" )
		xmlFile(a);
	else if( a._anonType == "dicomDir" )
		dicomDir(a);
}

void Anonymizer::slot_AskForAnonymizeFiles(Anonymization a)
{
	if( a._anonType == "videos" )
		videos(a);
	else if( a._anonType == "dicomFiles" )
		dicomFiles(a);
}

/**
* \brief Anonymization of videos with XML
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::xmlFile(Anonymization a)
{
	refreshPatientList();
	QString fileName = a._fileNames[0];
	bool destinationMatch = a._destinationMatch;
	QString outputFolder = a._bddFolder;
	
	//Able the click on Cancel button
	ui->cancelCurrentAnonymisationButton->setEnabled(true);
	//Prepare the progress bar
	ui->anonymisationProgressBar->setValue(0);
	ui->fileProgressBar->setValue(0);

	addProgressMessage(ui, "\nStarting anonymization (type : xmlFile, fileSelected : " + fileName + ")\n", Qt::blue, "", true);
	bool writeTheMissingInterestingXMLDataFile = false;

	addProgressMessage(ui,"Extracting data from XML file ...", Qt::black, "", false);

	//Read the videos XML fle
	QDomDocument docRead;
	fillDomDocFromXmlFile(docRead, fileName, ui);
		
	Patient* actualPatient;
	Image* actualImage;
	Study* actualStudy;
	Series* actualSeries;
	Extract* actualExtract;
	File* actualFile;

	///- FIND INTERESTING DATA AND WRITE IT IN A NEW FILE OR THE EXISTING PATIENT'S FILE
	//list of informations we need to get
	QString nameTmp, firstNameTmp, birthTmp, sexTmp;
	QString doctorTmp, fullDate, dateTmp, timeTmp, treatmentTmp;
	
	QString finalID;
	bool patientFound;
	//If we use the default database
	if( destinationMatch )
	{
		//Save de data in temporary variables
		nameTmp = getElemValue(docRead, "PatName");
		nameTmp = nameTmp.toUpper().trimmed();
		firstNameTmp = getElemValue(docRead, "PatFirstName");
		firstNameTmp = firstNameTmp.toLower().replace(0,1,firstNameTmp[0].toUpper()).trimmed();
		birthTmp = getElemValue(docRead, "PatBirth");
		sexTmp = getElemValue(docRead, "PATSex");
		sexTmp = sexTmp.trimmed();

		///- MAKE SURE THE PATIENT DOESN'T ALREADY EXISTS
		patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);
	
		bool userGaveAName = false;
		if( finalID == "99999" )
		{
			AskPatientInfo askPI(0);
			int result = askPI.exec();
			if( result == QDialog::Accepted )
			{
				askPI.GetData(nameTmp, firstNameTmp, sexTmp, birthTmp);
				nameTmp = nameTmp.toUpper().trimmed();
				firstNameTmp = firstNameTmp.toLower().replace(0,1,firstNameTmp[0].toUpper()).trimmed();
				sexTmp = sexTmp.trimmed();
				patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);
				userGaveAName = true;
			}
		}

		if( !patientFound && (finalID != "99999" || userGaveAName) )
		{
			///- CREATE ID IF NEW PATIENT
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(5) << getPatientNumber();
			finalID = QString::fromStdString(ss.str());
		}
	}
	//If the user has chosen a different output folder
	else
		finalID = "Output";
	
	_actualFinalId = finalID;
	int extractNumber = 0;
	
	if( destinationMatch )
	{
		//Add Element Patient
		actualPatient = new Patient(finalID);

		///- CREATE NEW ORDERED DOCUMENT FOR DATABASE
		if(patientFound)
			fillPatientFromXMLFile(*actualPatient, Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", ui);
		
		extractNumber = getExtractNumber(actualPatient);

		///- FILL THE DATABASE DOCUMENT
		actualImage = new Image("Laparoscopy");
		actualImage = actualPatient->addImage(actualImage);

		actualStudy = new Study("defaultStudy");
		actualStudy = actualImage->addStudy(actualStudy);

		actualSeries = new Series("defaultSeries");
		actualSeries = actualStudy->addSeries(actualSeries);
							
		actualExtract = new Extract(extractNumber);
		actualExtract = actualSeries->addExtract(actualExtract);

		//Read the file of interesting elements
		QFile tagsFile(Config::INTERESTING_DATA_XML);
		if (!tagsFile.open(QIODevice::ReadOnly | QIODevice::Text))
			writeTheMissingInterestingXMLDataFile = true;
		else
		{
			QTextStream in(&tagsFile);
			while (!in.atEnd()) 
			{
				QString line = in.readLine();
				QStringList splittedLine;
				if(line != "")
				{
					splittedLine = line.split(" __ ");
					treatmentTmp = getElemValue(docRead, splittedLine.at(0));
					if( treatmentTmp.trimmed() != "" )
					{
						Property prop(splittedLine.at(0), splittedLine.at(1), treatmentTmp);
						actualExtract->addProperty(prop);
					}
				}
			}
		}
	}	
		
	///- CREATE THE DIRECTORY FOR THE PATIENT IF IT DOESN'T EXIST
	QString extractFolder = "Extract_" + QString::number(extractNumber);
	QDir().mkpath(outputFolder + "/" + finalID + "/" + extractFolder);

	/// Find all the images and videos and copy them in the extract folder.
	QDomNodeList elements = docRead.elementsByTagName("File");
	QDir dirFile(fileName);

	//Ask the user if he wants to remove pixels area in videos
	bool useVideoModificationTool;
	if (QMessageBox::Yes == QMessageBox::question(0, "Video Modification Tool", "Is there any annotations in the videos that you need to anonymize ?\nIf you say Yes, the program will allow you to select areas to delete.\nIf you say No, the videos will simply be copied.", QMessageBox::Yes | QMessageBox::No) )
		useVideoModificationTool = true;
	else
		useVideoModificationTool = false;
	
	QVector<QStringList> filesOrdered;
	QVector<QVector<BoundingBox*>*> allBoundingBoxes;

	//If we want to use the pixel modification tool, we browse the videos and keep the boundingboxes selected for each video
	if( useVideoModificationTool )
	{
		QStringList files;
		if( elements.size() > 0 )
		{
			//For each path found
			for(int i=0; i<elements.size(); i++)
			{
				QString xmlPath = elements.at(i).toElement().text();
				QString srcF = QString(fileName.left(fileName.lastIndexOf("/")) + xmlPath.right(xmlPath.size()-1)).replace("\\","/");
				files.append(srcF);
			}
		}
		getBBandImgGroups(files, allBoundingBoxes, filesOrdered);
	}

	//If we found at least one file's path
	if( elements.size() > 0 )
	{
		ui->anonymisationProgressBar->setMaximum(elements.size());
		//For each path found
		for(int i=0; i<elements.size(); i++)
		{
			QString xmlPath = elements.at(i).toElement().text();
			QString xmlPathFoldsAndFile = xmlPath.right(xmlPath.size()-1);
			QString xmlPathFolders = xmlPathFoldsAndFile.left(xmlPath.lastIndexOf("/") - 1);

			addProgressMessage(ui,"Copying element : " + xmlPath, Qt::black, "", false);
			
			QDir().mkpath(outputFolder + "/" + finalID + "/" + extractFolder + xmlPathFolders);
			
			QString srcFile = fileName.left(fileName.lastIndexOf("/")) + xmlPathFoldsAndFile;
			QString destFile = QString(outputFolder + "/" + finalID + "/" + extractFolder + xmlPathFoldsAndFile).replace("\\","/");

			//If the user selected areas to delete in pixels
			if( !useVideoModificationTool )
			{			
				deleteFileDetailledProperties(srcFile, destFile);
			}
			//If the user didn't
			else
			{
				for(int listN=0; listN<filesOrdered.size(); listN++)
				{
					if( filesOrdered[listN].contains(srcFile) )
					{
						//Check the number of checkbox for this image
						int imgNumber = filesOrdered[listN].indexOf(srcFile);
						int nbBBforThisFile = 0;
						for(int bb = 0; bb < allBoundingBoxes[listN]->size(); bb++)
						{
							BoundingBox* bbox = (*allBoundingBoxes[listN])[bb];
							//The images numbers starts from 1 in the bounding boxes. we add 1 do im
							if( (bbox->isForAll() && !bbox->notIn().contains(imgNumber+1)) || (!bbox->isForAll() && bbox->imgNumber() == (imgNumber+1)) )
								for(int x = bbox->x1(); x <= bbox->x2(); x++)
									nbBBforThisFile++;
						}
						//If this image doesn't have to be modified
						if( nbBBforThisFile > 0 )
							modifyVideoFrames(srcFile, destFile, extractNumber, extractFolder, allBoundingBoxes[listN], filesOrdered[listN].indexOf(srcFile));
						else
						{
							addProgressMessage(ui, "You didn't select any area to delete on this video, we simply copy it.", Qt::black, "", false);
							deleteFileDetailledProperties(srcFile, destFile);
						}
						break;
					}
				}
			}
			
			//If the file exists in the destination file, the anonymization is good and we can add the file to the extract for the future xml file.
			if( QFile(destFile).exists() )
			{			
				if( destinationMatch )
				{
					actualFile = new File("./" + extractFolder + "/" + xmlPathFoldsAndFile);
					actualFile = actualExtract->addFile(actualFile);
				}
			}
			else
				addProgressMessage(ui, "Something went wrong with this file.", Qt::red, "", false);

			ui->anonymisationProgressBar->setValue(i+1);

			//If we interrupt the creation of one of the file we stop the program
			if( _isCanceled ) 
			{
				clearBBs(allBoundingBoxes);
				cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
				return;
			}
		}  
	}
	clearBBs(allBoundingBoxes);
	
	ui->cancelCurrentAnonymisationButton->setEnabled(false);
	
	if( actualExtract->getFilesCount() > 0 )
	{
		if( destinationMatch )
		{
			addProgressMessage(ui,"Patient ID : " + finalID, Qt::black, "", true);
			//Write patient's private data file
			writePrivateDataFile(patientFound, finalID, nameTmp, firstNameTmp, birthTmp, sexTmp);
			//Write general data files (for Anonymizer)
			addProgressMessage(ui,"Saving the database (Anonymizer)...", Qt::black, "", false);
			writeXMLfromPatient(Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", actualPatient);
			//Write general data file (for Destination)
			addProgressMessage(ui,"Saving the database (Destination)...", Qt::black, "", false);
			writeXMLfromPatient(outputFolder + "/" + finalID + "/" + finalID + ".xml", actualPatient);
			//Add the patient to the list
			addPatientToList(Patient(finalID, nameTmp, firstNameTmp, birthTmp));
			//Cleaning memory
			addProgressMessage(ui,"Cleaning memory...", Qt::black, "", false);
			delete actualPatient;
		}
	}
	else
	{
		addProgressMessage(ui,"\nNot a single file worked (no result in database), clearing destination folder...", Qt::red, "", true);
		cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
		return;
	}
			
	if( finalID == "99999" )
		addProgressMessage(ui, "The patient identity could not be found. The anonymized data have been saved under the ID 99999. This ID regroups all the data from unknown patients.\n", Qt::darkRed, "", true);

	addProgressMessage(ui,"Work done.", Qt::black, "", true);

	if( writeTheMissingInterestingXMLDataFile == true )
		addProgressMessage(ui, "", Qt::red, "missingInterestingXMLDataFile", true);

	(*_remainingAnon)--;
	ui->remainingAnonLabel->setText(QString::number(*_remainingAnon) + " remaining");
	if( *_remainingAnon == 0 )
		ui->remainingAnonLabel->setVisible(false);

	*_workDoneBool = true;
}

/**
* \brief Anonymization of videos without XML
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::videos(Anonymization a)
{
	refreshPatientList();
	QStringList files = a._fileNames;
	bool destinationMatch = a._destinationMatch;
	QString outputFolder = a._bddFolder;

	//Able the click on Cancel button
	ui->cancelCurrentAnonymisationButton->setEnabled(true);
	//Prepare the progress bar
	ui->anonymisationProgressBar->setMaximum(files.size());
	ui->anonymisationProgressBar->setValue(0);
	ui->fileProgressBar->setValue(0);

	QString fileFolder = files[0].replace("\\","/").left(files[0].lastIndexOf("/"));
	addProgressMessage(ui, "\nStarting anonymization (type : videos, folder : " + fileFolder + ")\n", Qt::blue, "", true);

	QString finalID;
	bool patientFound = false;
	
	bool writeTheMissingInterestingXMLDataFile = false;
	
	Patient* actualPatient;
	Image* actualImage;
	Study* actualStudy;
	Series* actualSeries;
	Extract* actualExtract;
	File* actualFile;

	QString nameTmp, firstNameTmp, birthTmp, sexTmp;
	//If we use the default database
	if( destinationMatch )
	{
		AskPatientInfo askPI(0);
		int result = askPI.exec();
		if( result == QDialog::Accepted )
		{
			askPI.GetData(nameTmp, firstNameTmp, sexTmp, birthTmp);
			nameTmp = nameTmp.toUpper().trimmed();
			firstNameTmp = firstNameTmp.toLower().replace(0,1,firstNameTmp[0].toUpper()).trimmed();
			sexTmp = sexTmp.trimmed();
		}

		if(!patientFound)					
			patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);

		if(!patientFound && finalID != "99999")
		{
			///- CREATE ID IF NEW PATIENT
			std::stringstream ss;
			ss << std::setfill('0') << std::setw(5) << getPatientNumber();
			finalID = QString::fromStdString(ss.str());
		}	
	}
	//If the user has chosen a different output folder
	else
		finalID = "Output";
	
	_actualFinalId = finalID;	
	
	int extractNumber = 0;
	
	if( destinationMatch )
	{
		//Add Element Patient
		actualPatient = new Patient(finalID);

		///- CREATE NEW ORDERED DOCUMENT FOR DATA
		if(patientFound)
			fillPatientFromXMLFile(*actualPatient, Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", ui);
	
		extractNumber = getExtractNumber(actualPatient);

		///- FILL THE DOCUMENT FOR DATA
		//Add Element Laparoscopy
		actualImage = new Image("Laparoscopy");
		actualImage = actualPatient->addImage(actualImage);

		actualStudy = new Study("defaultStudy");
		actualStudy = actualImage->addStudy(actualStudy);

		actualSeries = new Series("defaultSeries");
		actualSeries = actualStudy->addSeries(actualSeries);
							
		actualExtract = new Extract(extractNumber);
		actualExtract = actualSeries->addExtract(actualExtract);
	}
	
	///- CREATE THE DIRECTORY FOR THE PATIENT IF IT DOESN'T EXIST
	//if the patient folder doesn't exist, we create it
	QString extractFolder = "Extract_" + QString::number(extractNumber);
	QDir().mkpath(outputFolder + "/" + finalID + "/" + extractFolder);
	QString destinationFolder = QString(outputFolder + "/" + finalID + "/" + extractFolder + "/videos").replace("\\","/");

	QString imageFolderPath;
	QDir().mkpath(destinationFolder);
	
	//Ask the user if he wants to use the video modification tool or not
	bool useVideoModificationTool;
	if (QMessageBox::Yes == QMessageBox::question(0, "Video Modification Tool", "Is there any annotations in the videos that you need to anonymize ?\n"
									"If you say Yes, the program will allow you to select areas to delete.\nIf you say No, the videos will simply be copied.", QMessageBox::Yes | QMessageBox::No) )
		useVideoModificationTool = true;
	else
		useVideoModificationTool = false;

	QVector<QStringList> filesOrdered;
	QVector<QVector<BoundingBox*>*> allBoundingBoxes;

	if( useVideoModificationTool )
		getBBandImgGroups(files, allBoundingBoxes, filesOrdered);			

	//If we interrupt the creation of one of the file we stop the program
	if( _isCanceled ) 
	{
		clearBBs(allBoundingBoxes);
		cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
		return;
	}

	//For each path found
	for(int i=0; i<files.size(); i++)
	{		
		files[i] = files[i].replace("\\","/");

		QString theFileName = files[i].right(files[i].size() - files[i].lastIndexOf("/") - 1);
		addProgressMessage(ui,"Copying element : " + theFileName, Qt::black, "", false); //
		//We cut the path to have the differents folders of the path and the file name 
		
		QString destinationVideo = destinationFolder + "/" + theFileName;
		//We copy the file
		
		if( !useVideoModificationTool )
		{			
			deleteFileDetailledProperties(files[i], destinationVideo);
		}
		else
		{
			for(int listN=0; listN<filesOrdered.size(); listN++)
			{
				if( filesOrdered[listN].contains(files[i]) )
				{
					//Check the number of checkbox for this image
					int imgNumber = filesOrdered[listN].indexOf(files[i]);
					int nbBBforThisFile = 0;
					for(int bb = 0; bb < allBoundingBoxes[listN]->size(); bb++)
					{
						BoundingBox* bbox = (*allBoundingBoxes[listN])[bb];
						//The images numbers starts from 1 in the bounding boxes. we add 1 do im
						if( (bbox->isForAll() && !bbox->notIn().contains(imgNumber+1)) || (!bbox->isForAll() && bbox->imgNumber() == (imgNumber+1)) )
							for(int x = bbox->x1(); x <= bbox->x2(); x++)
								nbBBforThisFile++;
					}
					//If this image doesn't have to be modified
					if( nbBBforThisFile > 0 )
						modifyVideoFrames(files[i], destinationVideo, extractNumber, extractFolder, allBoundingBoxes[listN], imgNumber);
					else
					{
						addProgressMessage(ui, "You didn't select any area to delete on this video, we simply copy it.", Qt::black, "", false);
						deleteFileDetailledProperties(files[i], destinationVideo);
					}
					break;
				}
			}
		}
		
		//If the file exists in the destination file, the anonymization is good and we can add the file to the extract for the future xml file.
		if( QFile(destinationVideo).exists() )
		{
			if( destinationMatch )
			{
				//We add the xml element
				actualFile = new File("./" + extractFolder + "/videos/" + theFileName);
				actualFile = actualExtract->addFile(actualFile);
			}
		}
		else
			addProgressMessage(ui, "Something went wrong with this file.", Qt::red, "", false);

		ui->anonymisationProgressBar->setValue(i+1);		

		//If we interrupt the creation of one of the file we stop the program
		if( _isCanceled ) 
		{
			clearBBs(allBoundingBoxes);
			cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
			return;
		}
	}

	clearBBs(allBoundingBoxes);

	ui->cancelCurrentAnonymisationButton->setEnabled(false);

	//We check if we have a video at least in the destination folder. If it's not the case, all the videos failed and we do like tere was no anonymisation done.
	if( actualExtract->getFilesCount() > 0 )
	{
		if( destinationMatch )
		{
			addProgressMessage(ui,"Patient ID : " + finalID, Qt::black, "", true);	
			//Write patient's private data file
			writePrivateDataFile(patientFound, finalID, nameTmp, firstNameTmp, birthTmp, sexTmp);
			//Write general data file (for Anonymizer)
			addProgressMessage(ui,"Saving the database (Anonymizer)...", Qt::black, "", false);
			writeXMLfromPatient(Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", actualPatient);
			//Write general data file (for Destination)
			addProgressMessage(ui,"Saving the database (Destination)...", Qt::black, "", false);
			writeXMLfromPatient(outputFolder + "/" + finalID + "/" + finalID + ".xml", actualPatient);
			//Add the patient to the list
			addPatientToList(Patient(finalID, nameTmp, firstNameTmp, birthTmp));
			//Clean memory
			addProgressMessage(ui,"Cleaning memory...", Qt::black, "", false);
			delete actualPatient;
		}
	}
	else
	{
		addProgressMessage(ui,"\nNot a single file worked (no result in database), clearing destination folder...", Qt::red, "", true);
		cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
		return;
	}
				
	if( finalID == "99999" )
		addProgressMessage(ui, "\nThe patient identity could not be found. The anonymized data have been saved under the ID 99999. This ID regroups all the data from unknown patients.\n", Qt::darkRed, "", true);

	addProgressMessage(ui,"Work done.", Qt::black, "", true);

	(*_remainingAnon)--;
	ui->remainingAnonLabel->setText(QString::number(*_remainingAnon) + " remaining");
	if( *_remainingAnon == 0 )
		ui->remainingAnonLabel->setVisible(false);

	*_workDoneBool = true;
}

/**
* \brief This function allow you to copy a file from a folder to an other while deleting all the properties.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::deleteFileDetailledProperties(QString srcFile, QString destinationFile)
{
	QString exepath = QDir::currentPath();
	QStringList panthels = exepath.split("/");
	//Move in the ffmpeg.exe folder if the path contains spaces
	if( exepath.contains(" ") )
		for(int pe=0; pe<panthels.size(); pe++)
		{
			QString cd = "cd  " + panthels[pe];
			system(cd.toLocal8Bit().data());
		}			
	
	// Documentation about how to work on metadata with ffmpeg
	// http://jonhall.info/how_to/create_id3_tags_using_ffmpeg
	// -map_metadata -1 : removes every property
	// -codec copy : keep the previous video codecs (or something like that)
	QString cmd;
	if( exepath.contains(" ") )
		cmd = "ffmpeg -i \"" + srcFile + "\" -map_metadata -1 -codec copy \"" + destinationFile + "\"";
	else
		cmd = exepath + "/ffmpeg -i \"" + srcFile + "\" -map_metadata -1 -codec copy \"" + destinationFile + "\"";
	system(cmd.toLocal8Bit().data());
}

/**
* \brief This function modify the video frames using OpenCV
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::modifyVideoFrames(QString srcFile, QString &destinationVideo, int extractNumber, QString extractFolder, QVector<BoundingBox*>* boundingBoxes, int imgNumber)
{
	cv::Mat frame;
	cv::VideoCapture capture(srcFile.toStdString().c_str());

	if (!capture.isOpened())
	{
		addProgressMessage(ui,"We can\'t open, copying it instead.", Qt::red, "", false);
		createCopyThread(srcFile, destinationVideo, true);
	}
	else
	{
		double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
		double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

		cv::Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
		
		//Open the videoWriter with the working codecs. If you need to add a new video format you have to test different codecs. 
		QString format = destinationVideo.right(3);
		cv::VideoWriter oVideoWriter;
		if( format == "wmv" || format == "avi" || format == "mov" )
			oVideoWriter.open(destinationVideo.toStdString().c_str(), CV_FOURCC('D', 'I', 'V', 'X'), capture.get(CV_CAP_PROP_FPS), frameSize);
		else
		{
			destinationVideo = destinationVideo.left(destinationVideo.lastIndexOf(".")+1) + "mpg";
			oVideoWriter.open(destinationVideo.toStdString().c_str(), CV_FOURCC('M', 'P', 'E', 'G'), capture.get(CV_CAP_PROP_FPS), frameSize); 
		}

		if ( !oVideoWriter.isOpened() ) //if not initialize the VideoWriter successfully
		{
			addProgressMessage(ui, "Can\'t modify the file, copying it instead.", Qt::red, "", false);
			createCopyThread(srcFile, destinationVideo, true);
			return;
		}
				
		long long int numberOfFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);
		QString nbFrames;
		if( numberOfFrames > 0 )
			nbFrames = QString::number(numberOfFrames);
		else
			nbFrames = "Unknown";
		ui->fileProgressBar->setMaximum(numberOfFrames);
		ui->fileProgressBar->setValue(1);
		oVideoWriter.write(frame);
		
		bool hasNext = true;
		long long int cpt = 0;

		while( hasNext )
		{
			hasNext = capture.read(frame);
			if( !hasNext )
				break;
			else
			{
				cpt++;
				addProgressMessage(ui, "Rewriting the video ... Frames modified : " + QString::number(cpt) + "/~" + nbFrames, Qt::black, "", false);
				ui->fileProgressBar->setValue(cpt);

				//Delete all the pixel contains in the rectangles selected by the user.				
				for(int bb = 0; bb < boundingBoxes->size(); bb++)
				{
					BoundingBox* bbox = (*boundingBoxes)[bb];
					//The images numbers starts from 1 in the bounding boxes. we add 1 do im
					if( (bbox->isForAll() && !bbox->notIn().contains(imgNumber+1)) || (!bbox->isForAll() && bbox->imgNumber() == (imgNumber+1)) )
					{
						for(int x = bbox->x1(); x <= bbox->x2(); x++)
						{
							for(int y = bbox->y1(); y <= bbox->y2(); y++)
							{
								//Modification of the pixel
								frame.at<cv::Vec3b>(y,x)[0] = 0;
								frame.at<cv::Vec3b>(y,x)[1] = 0;
								frame.at<cv::Vec3b>(y,x)[2] = 0;
							}
						}
					}
				}
			}

			oVideoWriter.write(frame);
					
			prepareProgressTextForRewrite();

			//If we interrupt the creation of one of the file we stop the program
			if( _isCanceled ) 
			{
				oVideoWriter.release();
				return;
			}
		}
		ui->fileProgressBar->setValue(numberOfFrames);
	}
}

/**
* \brief Anonymization of dicoms with DICOMDIR
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::dicomDir(Anonymization a)
{
	refreshPatientList();
	QString fileName = a._fileNames[0];
	bool destinationMatch = a._destinationMatch;
	QString outputFolder = a._bddFolder;
	
	//Able the click on Cancel button
	ui->cancelCurrentAnonymisationButton->setEnabled(true);
	//Prepare the progress bar
	ui->anonymisationProgressBar->setValue(0);
	ui->fileProgressBar->setValue(0);

	addProgressMessage(ui, "\nStarting anonymization (type : dicomDir, fileSelected : " + fileName + ")\n", Qt::blue, "", true);

	//This variable is used to see if we at least anonymized a file
	//In fact if nothing worked, we have to clear destination folder
	int fileThatWorkedCount;

	QString extractFolder;
	int extractNumber = 0;
	QString failedFiles;

	QDomDocument docWritePV;
	QDomDocument docWrite;
	QDomElement parentNode;
	QString finalID;

	//Browse the DICOMDIR file
	QString folderName = fileName;
	folderName.truncate(folderName.lastIndexOf("/"));
	
	//Open the DICOMDIR File
	DcmDicomDir dicomdir(fileName.toStdString().c_str());
	//Retrieve root node
	DcmDirectoryRecord *root = &dicomdir.getRootRecord();
	//Prepare child elements
	DcmDirectoryRecord *rootTest = new DcmDirectoryRecord(*root);
	DcmDirectoryRecord *PatientRecord = NULL;
	DcmDirectoryRecord *StudyRecord = NULL;
	DcmDirectoryRecord *SeriesRecord = NULL;
	DcmDirectoryRecord* image = NULL;

	//Selected file not valid
	if(rootTest == NULL || rootTest->nextSub(PatientRecord) == NULL) 
		addProgressMessage(ui,"It looks like the selected file does not have the expected format.\n", Qt::red, "", true);
	///- GO THROUGH THE DICOMDIR STUCTURE AND FIND PATIENTS AND PATIENTS IMAGES IN EACH DICOM FILE
	//Structure of a DICOMFILE can be found at the 
	//page 734 and the following of this pdf : http://medical.nema.org/dicom/2003/03_03pu.pdf
	else
	{
		//Initialize number of files for QProgressBar
		int numFiles = countNbFilesFromDICOMDIR(fileName);
		int nbFilesCopied = 0;
		ui->anonymisationProgressBar->setMaximum(numFiles);

		while ((PatientRecord = root->nextSub(PatientRecord)) != NULL)
		{
			//A DICOMDIR file can contain more than one patient. 
			QString nameTmp, firstNameTmp, birthTmp, sexTmp;
			bool patientFound = false;
			bool weAskedPatientInfoAlready = false;
			//Boolean used to know if we already load an existing file (we don't want to load it before each dicom file).
			bool fileAlreadyOpen = false;
			bool isFirstLoop = true;

			Patient* actualPatient;
			Image* actualImage;
			Study* actualStudy;
			Series* actualSeries;
			Extract* actualExtract;
			File* actualFile;

			fileThatWorkedCount = 0;
			
			while ((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL)
			{
				//Récupération de la description de la Study actuelle
				const char *sStudyDescr;
				StudyRecord->findAndGetString(DCM_StudyDescription, sStudyDescr);
				if( sStudyDescr == "" )
					sStudyDescr = "defaultStudy";

				while ((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL)
				{
					//This variables contains the combinaison of rows/cols to group the image with the same size and the path for the image that need to be manually anonymized
					QVector<int> rowsForManualAnonym;
					QVector<int> colsForManualAnonym;
					QVector<QStringList> imagesForManualAnonym;
					Uint16 prevImgW = 0, prevImgH = 0;

					while ((image = SeriesRecord->nextSub(image)) != NULL)
					{
						const char* sName;
						image->findAndGetString(DCM_ReferencedFileID, sName);

						//If a file is selected
						if(sName != "")
						{
							QString actualDicomName = sName;
							actualDicomName.replace(QString("\\"), QString("/"));
							//actualDicomName format : S01/000001
							addProgressMessage(ui,"Working on file : " + actualDicomName, Qt::black, "", false);
							QString oldDicomFile = folderName + "/" + actualDicomName; //format : C:/.../.../S01/000001
							
									
							//Verify if the file exists
							QFile Fout(oldDicomFile);
							if(!Fout.exists())
							{
								addProgressMessage(ui, "This file does not exist.\n", Qt::red, "", false);
								failedFiles += actualDicomName + " (File not found) ";
								continue;
							}

							QString tagValue;
							QString modalityTmp;
									
							OFString tagValueOF;
							
							DcmFileFormat fileformat;
							OFCondition status = fileformat.loadFile(oldDicomFile.toStdString().c_str());
							DcmDataset *dictionary;

							//Check if the dictionary is correct
							if( status.good() )				
								dictionary = fileformat.getDataset();
							else
							{
								addProgressMessage(ui, "Data dictionary can't be found, file corrupted (We skip it!).\n", Qt::red, "", false);
								failedFiles += actualDicomName + " (Data corrupted) ";
								continue;
							}

							//Check if it's an interesting image. We don't need :
							//VXTL STATE : identifies a Voxtool state SC (private data of the screen save holds information to restore the state of the application)
							QString imageType;
							if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x0008), tagValueOF).good())
								imageType = tagValueOF.c_str();

							if( imageType.toLower().contains("vxtl state") )
							{
								QMessageBox::StandardButton reply = QMessageBox::question(0, "File probably useless", "This file is not interesting. Image Type : VXTL STATE (private data of the screen save holds information to restore the state of the application). Do you still want to anonymize this file ?", QMessageBox::Yes|QMessageBox::No);
								if (reply == QMessageBox::No) 
								{
									addProgressMessage(ui,"File ignored.\n", Qt::black, "", false);
									failedFiles += actualDicomName + " (Useless file) ";

									//Update the progressBar
									nbFilesCopied++;
									ui->anonymisationProgressBar->setValue(nbFilesCopied);
									continue;
								}
							}								

							//If the file isn't in the interesting modalities, we skip it.
							if (dictionary->findAndGetOFString(DcmTagKey(0x0008, 0x0060), tagValueOF).good())
								modalityTmp = tagValueOF.c_str();

							if( !Config::INTERESTING_MODALITIES.contains(modalityTmp) )
							{
								addProgressMessage(ui,"File ignored because this modality (" + modalityTmp + ") is not in the interesting modalities (see config.cpp).\n", Qt::red, "", false);
								failedFiles += actualDicomName + " (Bad modality " + modalityTmp + ") ";
								continue;
							}

							bool userGaveAName = false;
							
							if( destinationMatch )
							{
								if( !patientFound && finalID != "99999" )	
								{
									///- GET THE PATIENT PRIVATE DATAS
									dicomGetPatientPV(dictionary, nameTmp, firstNameTmp, birthTmp, sexTmp);
									///- MAKE SURE THE PATIENT DOESN'T ALREADY EXISTS
									patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);
	
									if( finalID == "99999" && !weAskedPatientInfoAlready )
									{
										AskPatientInfo askPI(0);
										int result = askPI.exec();
										if( result == QDialog::Accepted )
										{
											askPI.GetData(nameTmp, firstNameTmp, sexTmp, birthTmp);
											nameTmp = nameTmp.toUpper().trimmed();
											firstNameTmp = firstNameTmp.toLower().replace(0,1,firstNameTmp[0].toUpper()).trimmed();
											sexTmp = sexTmp.trimmed();
											patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);
											userGaveAName = true;
										}
										weAskedPatientInfoAlready = true;
									}
								}

								if( !patientFound && (finalID != "99999" || userGaveAName) )
								{
									///- CREATE ID IF NEW PATIENT
									std::stringstream ss;
									ss << std::setfill('0') << std::setw(5) << getPatientNumber();
									finalID = QString::fromStdString(ss.str());
								}
							}
							//If the user has chosen a different output folder
							else
								finalID = "Output";

							_actualFinalId = finalID;
							
							if( destinationMatch )
							{
								//Add Element Patient
								if( isFirstLoop )
									actualPatient = new Patient(finalID);

								///- CREATE NEW ORDERED DOCUMENT FOR DATA		
								if(patientFound && !fileAlreadyOpen)
								{
									fileAlreadyOpen = true;
									fillPatientFromXMLFile(*actualPatient, Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", ui);
								}

								if( isFirstLoop )
								{
									extractNumber = getExtractNumber(actualPatient);				
									extractFolder = "Extract_" + QString::number(extractNumber);
								}

								///- FILL THE DOCUMENT OF DATA WITH THE ACTUAL DICOM
								// Browse the useful modality
								for(int mod=0; mod<Config::INTERESTING_MODALITIES.size(); mod++)
								{
									QString tmpMod = Config::INTERESTING_MODALITIES.at(mod);
									if( modalityTmp == tmpMod )
									{
										//Add Element modality
										actualImage = new Image(tmpMod);
										actualImage = actualPatient->addImage(actualImage);
									}
								}

								actualStudy = new Study(sStudyDescr);
								actualStudy = actualImage->addStudy(actualStudy);

								QString SeriesDescr;
								if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x103E), tagValueOF).good())
									SeriesDescr = tagValueOF.c_str();
								if( SeriesDescr == "" )
									sStudyDescr = "defaultSeries";

								actualSeries = new Series(SeriesDescr);
								actualSeries = actualStudy->addSeries(actualSeries);
							
								actualExtract = new Extract(extractNumber);
								actualExtract = actualSeries->addExtract(actualExtract);

								actualFile = new File("./" + extractFolder + "/" + actualDicomName);
								actualFile = actualExtract->addFile(actualFile);

								for(unsigned long i = 0; i<dictionary->card(); i++) 
								{ 
									DcmElement* element = dictionary->getElement(i);
									DcmTag dcTag = element->getTag();

									if( !_anonymousTags.contains(dcTag.toString().c_str()) && QString(dcTag.getTagName()) != "Unknown Tag & Data" && QString(dcTag.getTagName()) != "PixelData" )
									{
										OFString val;
										element->getOFString(val, 0);
										QString tagvalue = QString(val.c_str());
										tagvalue = tagvalue.replace("\"", "_"); //We replace an evantual " in the tag value by another character, it could corrupt the XML file
										actualFile->addProperty(Property(QString(dcTag.toString().c_str()), QString(dcTag.getTagName()), tagvalue));
									}
								}
							}

							///- CREATE THE DIRECTORY FOR THE PATIENT IF IT DOESN'T EXIST
							//Copy the file
							QStringList splittedDicomFile = actualDicomName.split("/");
							//if the patient folder doesn't exist, we create it			
							QString newDicomFile = outputFolder + "/" + finalID + "/" + extractFolder;
							//Browse all the directories for the dicomdir tree and create the same directiories tree for this DICOM file in our destination folder.
							for(int pathAt=0; pathAt<splittedDicomFile.size()-1; pathAt++)
								newDicomFile += "/" + splittedDicomFile.at(pathAt);
							QDir().mkpath(newDicomFile);
							//Add the file name to the newDicomFile path
							newDicomFile += "/" + splittedDicomFile.at(splittedDicomFile.size()-1);
							newDicomFile.replace(QString("\\"), QString("/"));
								
							///- COPY AND ANONYMIZE THE ACTUAL DICOM FROM THE OLD FOLDER TO THE NEW ONE							
							createCopyThread(oldDicomFile, newDicomFile, true);

							//Update the progressBar
							nbFilesCopied++;
							ui->anonymisationProgressBar->setValue(nbFilesCopied);
							//Anonymize file
							anonymizeDicomFile(newDicomFile, rowsForManualAnonym, colsForManualAnonym, imagesForManualAnonym);		

							//If we interrupt the creation of one of the file we stop the program
							if( _isCanceled ) 
							{
								cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
								QFile(Config::DEFAULT_DESTINATION_PATH + "\\someoneAlreadyAnonymizingData.txt").remove();
								return;
							}
							
							isFirstLoop = false;
							fileThatWorkedCount++;
						}
					}

					if( imagesForManualAnonym.size() > 0 )
						dicomManualAnonymization(imagesForManualAnonym);
				} //End of series

			} //End of study

			//Update the progressBar
			ui->anonymisationProgressBar->setValue(numFiles);
			
			if( destinationMatch && fileThatWorkedCount > 0 )
			{
				addProgressMessage(ui,"Patient ID : " + finalID, Qt::black, "", true);
				//Write patient's private data file
				writePrivateDataFile(patientFound, finalID, nameTmp, firstNameTmp, birthTmp, sexTmp);
				//Write general data file (for Anonymizer)
				addProgressMessage(ui,"Saving the database (Anonymizer)...", Qt::black, "", false);
				writeXMLfromPatient(Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", actualPatient);		
				//Write general data file (for Destination)
				addProgressMessage(ui,"Saving the database (Destination)...", Qt::black, "", false);
				writeXMLfromPatient(outputFolder + "/" + finalID + "/" + finalID + ".xml", actualPatient);
				//Add the patient to the list
				addPatientToList(Patient(finalID, nameTmp, firstNameTmp, birthTmp));
				//Cleaning memory
				addProgressMessage(ui,"Cleaning memory...", Qt::black, "", false);
				delete actualPatient;
			}
		} //End of patient

		if( fileThatWorkedCount > 0 )
		{
			///- COPY AND ANONYMIZE THE DICOMDIR FILE
			writeDICOMDIR(fileName, finalID, outputFolder, extractFolder);

			if( failedFiles == "" )
				addProgressMessage(ui,"Work done.\n", Qt::black, "", true);
			else
				addProgressMessage(ui,"Work done. Failed files : " + failedFiles + "\n", Qt::red, "", true);
		}
		else
		{
			addProgressMessage(ui,"\nNot a single file worked, clearing destination folder...", Qt::red, "", true);
			cancelActualAnonymisation(extractNumber, outputFolder, extractFolder);
			QFile(Config::DEFAULT_DESTINATION_PATH + "\\someoneAlreadyAnonymizingData.txt").remove();
			return;
		}
	}

	ui->cancelCurrentAnonymisationButton->setEnabled(false);
	
	(*_remainingAnon)--;
	ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
	if( (*_remainingAnon) == 0 )
		ui->remainingAnonLabel->setVisible(false);

	*_workDoneBool = true;
	QFile(Config::DEFAULT_DESTINATION_PATH + "\\someoneAlreadyAnonymizingData.txt").remove();
}

/**
* \brief Anonymization of dicoms without DICOMDIR
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::dicomFiles(Anonymization a)
{
	refreshPatientList();
	QStringList files = a._fileNames;
	bool destinationMatch = a._destinationMatch;
	QString outputFolder = a._bddFolder;

	QString pathElems = files[0].replace(QString("\\"), QString("/")).left(files[0].lastIndexOf("/"));
	addProgressMessage(ui, "\nStarting anonymization (type : dicomFiles, folder : " + pathElems + ")\n", Qt::blue, "", true);
	//Prepare the progress bar
	ui->anonymisationProgressBar->setValue(0);
	ui->anonymisationProgressBar->setMaximum(files.size());
	ui->fileProgressBar->setValue(0);

	QString extractFolder;
	int extractNumber = 0;
	QString failedFiles;

	//Initialize progress bar
	int nbFilesCopied = 0;

	//This list contains the patient we already found during this anonymization
	//We want all the files from the same patient in the same extract folder.
	QStringList patientIdAlreadyPassed;
	//This variables contains the combinaison of rows/cols to group the image with the same size and the path for the image that need to be manually anonymized
	QVector<int> rowsForManualAnonym;
	QVector<int> colsForManualAnonym;
	QVector<QStringList> imagesForManualAnonym;
	QMap<QString, Patient*> myPatientsTrees;

	//List of nodes where we can add the children elements (To avoid searching in XML file each time)
	Image* actualImage;
	Study* actualStudy;
	Series* actualSeries;
	Extract* actualExtract;
	File* actualFile;
	
	bool weAskedPatientInfoAlready = false;

	for(int f=0; f<files.size(); f++)
	{
		bool patientFound = false;
		QString finalID;

		QString oldDicomFile = files[f];
		oldDicomFile.replace(QString("\\"), QString("/"));
		//oldDicomFile format : C:/.../.../S01/000001
		QString actualDicomName = oldDicomFile.right(oldDicomFile.size() - oldDicomFile.lastIndexOf("/") - 1);;
		//actualDicomName format : S01/000001
		addProgressMessage(ui,"Working on file : " + actualDicomName, Qt::black, "", false);
									
		QString tagValue;
		QString nameTmp, firstNameTmp, birthTmp, sexTmp;
		QString doctorTmp, dateTmp, timeTmp, stationNameTmp;
		QString modalityTmp;
									
		OFString tagValueOF;

		DcmFileFormat fileformat;
		OFCondition status = fileformat.loadFile(oldDicomFile.toStdString().c_str());
		DcmDataset *dictionary;

		//Check if the dictionary is correct
		if (status.good())				
			dictionary = fileformat.getDataset();
		else
		{
			addProgressMessage(ui,"Data dictionary can't be found, file corrupted (We skip it!).\n", Qt::red, "", false);
			failedFiles += actualDicomName + " (Data corrupted) ";
			continue;
		}

		//Check if it's an interesting image. We don't need :
		//VXTL STATE : identifies a Voxtool state SC (private data of the screen save holds information to restore the state of the application)
		QString imageType;
		if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x0008), tagValueOF).good())
			imageType = tagValueOF.c_str();

		if( imageType.toLower().contains("vxtl state") )
		{
			QMessageBox::StandardButton reply = QMessageBox::question(0, "File probably useless", "This file is not interesting. Image Type : VXTL STATE (private data of the screen save holds information to restore the state of the application). Do you still want to anonymize this file ?", QMessageBox::Yes|QMessageBox::No);
			if (reply == QMessageBox::No) {
				addProgressMessage(ui,"File ignored.\n", Qt::black, "", false);
				failedFiles += actualDicomName + " (Useless file) ";
				continue;
			}
		}		
					
		//If the file isn't in the interesting modalities, we skip it.
		if (dictionary->findAndGetOFString(DcmTagKey(0x0008, 0x0060), tagValueOF).good())
			modalityTmp = tagValueOF.c_str();

		if( !Config::INTERESTING_MODALITIES.contains(modalityTmp) )
		{
				addProgressMessage(ui,"File ignored because this modality (" + modalityTmp + ") is not in the interesting modalities (see config.cpp).\n", Qt::red, "", false);
				failedFiles += actualDicomName + " (Bad modality " + modalityTmp + ") ";
				continue;
		}

		if( destinationMatch )
		{
			///- GET THE PRIVATE DATAS
			dicomGetPatientPV(dictionary, nameTmp, firstNameTmp, birthTmp, sexTmp);

			///- MAKE SURE THE PATIENT DOESN'T ALREADY EXISTS
			if( !patientFound )
				patientFound = patientAlreadyExists(nameTmp, firstNameTmp, birthTmp, finalID);

			if(!patientFound && finalID != "99999")
			{
				///- CREATE ID IF NEW PATIENT
				std::stringstream ss;
				ss << std::setfill('0') << std::setw(5) << getPatientNumber();
				finalID = QString::fromStdString(ss.str());
			}
		}
		else
			finalID = "Output";

		_actualFinalId = finalID;
		
		if( destinationMatch )
		{
			///- CREATE NEW ORDERED DOCUMENT FOR DATA
			if( !myPatientsTrees.contains(finalID) )
			{
				myPatientsTrees.insert(finalID, new Patient(finalID));
				if( patientFound )
					fillPatientFromXMLFile(*myPatientsTrees[finalID], Config::ANONYMIZER_FOLDER + "\\" + finalID + ".xml", ui);
			}
				
			//If we didn't found files for this patient already, we create a new extract folder.
			if( !patientIdAlreadyPassed.contains(finalID) )
			{
				extractNumber = getExtractNumber(myPatientsTrees[finalID]);
				extractFolder = "Extract_" + QString::number(extractNumber);
			}

			///- FILL THE DOCUMENT OF DATA WITH THE ACTUAL DICOM
			// Browse the useful modality
			for(int mod=0; mod<Config::INTERESTING_MODALITIES.size(); mod++)
			{
				QString tmpMod = Config::INTERESTING_MODALITIES.at(mod);
				if( modalityTmp == tmpMod )
				{
					actualImage = new Image(tmpMod);
					actualImage = myPatientsTrees[finalID]->addImage(actualImage);
				}
			}

			QString StudyDescr;
			if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x1030), tagValueOF).good())
				StudyDescr = tagValueOF.c_str();
			if( StudyDescr == "" )
				StudyDescr = "defaultStudy";
		
			actualStudy = new Study(StudyDescr);
			actualStudy = actualImage->addStudy(actualStudy);
		
			QString SeriesDescr;
			if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x103E), tagValueOF).good())
				SeriesDescr = tagValueOF.c_str();
			if( SeriesDescr == "" )
				SeriesDescr = "defaultSeries";
		
			actualSeries = new Series(SeriesDescr);
			actualSeries = actualStudy->addSeries(actualSeries);
		
			actualExtract = new Extract(extractNumber);
			actualExtract = actualSeries->addExtract(actualExtract);

			actualFile = new File("./" + extractFolder + "/images/" + actualDicomName);
			actualFile = actualExtract->addFile(actualFile);
		
			for(unsigned long i = 0; i<dictionary->card();i++) 
			{ 
				DcmElement* element = dictionary->getElement(i);
				DcmTag dcTag = element->getTag();
								
				if( !_anonymousTags.contains(dcTag.toString().c_str()) && QString(dcTag.getTagName()) != "Unknown Tag & Data" && QString(dcTag.getTagName()) != "PixelData" )
				{
					OFString val;
					element->getOFString(val, 0);
					actualFile->addProperty(Property(QString(dcTag.toString().c_str()), QString(dcTag.getTagName()), QString(val.c_str())));
				}
			}

			//Write patient's private data file
			writePrivateDataFile(patientFound, finalID, nameTmp, firstNameTmp, birthTmp, sexTmp);
			addPatientToList(Patient(finalID, nameTmp, firstNameTmp, birthTmp));
		}

		QString anonymizedFilesPath = outputFolder + "/" + finalID + "/" + extractFolder;
		QDir().mkpath(anonymizedFilesPath + "/images");
									
		QString newDicomFile = anonymizedFilesPath + "/images/" + actualDicomName;
		newDicomFile.replace(QString("\\"), QString("/"));
			
		///- COPY AND ANONYMIZE THE ACTUAL DICOM FROM THE OLD FOLDER TO THE NEW ONE
		createCopyThread(oldDicomFile, newDicomFile, true);

		//Update the progressBar
		nbFilesCopied++;
		ui->anonymisationProgressBar->setValue(nbFilesCopied);				
		
		//Anonymize file
		anonymizeDicomFile(newDicomFile, rowsForManualAnonym, colsForManualAnonym, imagesForManualAnonym);

		if( finalID == "99999" )
			addProgressMessage(ui, "The patient identity could not be found. The anonymized data have been saved under the ID 99999. This ID regroups all the data from unknown patients.\n", Qt::darkRed, "", true);
		
		patientIdAlreadyPassed.append(finalID);
	}

	if( imagesForManualAnonym.size() > 0 )
		dicomManualAnonymization(imagesForManualAnonym);
	
	//Write general data file (for Anonymizer)
	if( destinationMatch )
	{
		QMapIterator<QString, Patient*> xml(myPatientsTrees);
		while (xml.hasNext()) 
		{
			addProgressMessage(ui,"Saving the database...", Qt::black, "", false);
			xml.next();
			Patient* pat = xml.value();
			writeXMLfromPatient(Config::ANONYMIZER_FOLDER + "\\" + xml.key() + ".xml", pat);
			//Write general data file (for Destination)
			writeXMLfromPatient(outputFolder + "/" + xml.key() + "/" + xml.key() + ".xml", pat);
			addProgressMessage(ui,"Cleaning memory...", Qt::black, "", false);
			delete pat;
		}
	}

	//Update the progressBar
	ui->anonymisationProgressBar->setValue(files.size());
		
	if( failedFiles == "" )
		addProgressMessage(ui,"Work done.\n", Qt::black, "", true);
	else
		addProgressMessage(ui,"Work done. Failed files : " + failedFiles + "\n", Qt::black, "", true);
	
	(*_remainingAnon)--;
	ui->remainingAnonLabel->setText(QString::number(*_remainingAnon) + " remaining");
	if( *_remainingAnon == 0 )
		ui->remainingAnonLabel->setVisible(false);

	*_workDoneBool = true;
}

/**
* \brief Count the number of files in a DICOMDIR tree
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
int Anonymizer::countNbFilesFromDICOMDIR(QString fileName)
{
	int numFiles = 0;	
	DcmDicomDir dicomdir(fileName.toStdString().c_str());
	DcmDirectoryRecord* root = &dicomdir.getRootRecord();
	DcmDirectoryRecord* PatientRecord = NULL;
	DcmDirectoryRecord* StudyRecord = NULL;
	DcmDirectoryRecord* SeriesRecord = NULL;
	
	while (((PatientRecord = root->nextSub(PatientRecord)) != NULL)) 
	{
		while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL)) 
		{ 
			while ((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL)
			{
				numFiles += SeriesRecord->cardSub();
			}
		}
	}

	return numFiles;
}

/**
* \brief Get all the private data from the patient (name, firstname, birth, sex)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param dictionary : data dictionary from the DICOM file
*/
void Anonymizer::dicomGetPatientPV(DcmDataset *dictionary, QString &nameTmp, QString &firstNameTmp, QString &birthTmp, QString &sexTmp)
{
	OFString tagValueOF;
	QString tagValue;
	//Patient's Name
	if (dictionary->findAndGetOFString(DcmTagKey(0x0010, 0x0010), tagValueOF).good())
		tagValue = tagValueOF.c_str();
	QStringList list = tagValue.split("^");
	if(list.size() == 2)
	{
		nameTmp = list.at(0);
		nameTmp = nameTmp.toUpper().trimmed();
		firstNameTmp = list.at(1);
		firstNameTmp = firstNameTmp.toLower().replace(0,1,firstNameTmp[0].toUpper()).trimmed();

		//Patient's Birth
		if (dictionary->findAndGetOFString(DcmTagKey(0x0010, 0x0030), tagValueOF).good())
			birthTmp = tagValueOF.c_str();
		QDate birth = QDate::fromString(birthTmp, "yyyyMMdd");
		birthTmp = birth.toString("dd/MM/yyyy");

		//Patient's Sex
		if (dictionary->findAndGetOFString(DcmTagKey(0x0010, 0x0040), tagValueOF).good())
			sexTmp = tagValueOF.c_str();
		sexTmp = sexTmp.trimmed();
	}
}

/**
* \brief Anonymize a dicom file using a list of tags from a file
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param beforePath : path of the file to anonymize
*
* Open de dictionary of tags.
* Open de anonymousTagsFiles (which contains tags to be anonymized and how to anonymize it).
* Anonymize all the tags.
* Set the tag PatientIdentityRemoved (0012|0062) to YES.
* Check the value of the tag BurnedInAnnotation (0028|0301). It says if there is annotation that may identify the patient.
* If the BurnedInAnnotation tag doesn't exist, we check if the tag ImageType (0008|0008) contains the word 'SCREEN SAVE'.
* If BurnedInAnnotation = YES or ImageType contains SCREEN SAVE, we display the images and the user select the area to anonymize.
*
* OCR : not used !!!
* We first need to know how much pixels are allocated for a pixel in the PixelData.
* There is 2 possible values : 8 or 16 that can be written.
* We use similar functions to manipulate it, but specific for 8 or 16 pixels.
* Create a bmp image and run OCR with tesseract on it.
* We calculate an average height for a word (with a coefficient of 5 for the words found in the corners). 
* Words with more than twice the average height are outliers and won't be deleted from the image. The boundingbox of the others are filled with black pixels.
*/
void Anonymizer::anonymizeDicomFile(QString filePath, QVector<int> &rowsForManualAnonym, QVector<int> &colsForManualAnonym, QVector<QStringList> &imagesForManualAnonym)
{
	//Open the file and get the dictionary
	DcmFileFormat fileformat;
	OFCondition status = fileformat.loadFile(filePath.toStdString().c_str());
	DcmDataset *dictionary;
	if( status.good() )				
		dictionary = fileformat.getDataset();

	//Open the file of anonymous tags
	QFile tagsFile(Config::INTERESTING_FILES_PATH + "\\anonymousTags.txt");
	if( !tagsFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		return;
	QTextStream in(&tagsFile);
	//Browse the file
	while( !in.atEnd() ) 
	{
		QString line = in.readLine();
		//If the line isn't a comment
		if( !line.startsWith("#") )
		{
			QStringList splittedLine;
			QStringList splittedTag;
			if( line.trimmed() != "" )
			{
				splittedLine = line.split(" __ ");
				splittedTag = splittedLine.at(0).trimmed().split("|");
				//Create the tag to remove
				DcmTagKey tag = DcmTagKey();
				bool isOk;
				tag.setGroup(("0x" + splittedTag.at(0)).toUInt(&isOk, 16));
				tag.setElement(("0x" + splittedTag.at(1)).toUInt(&isOk, 16));				
				
				//Remove the tag
				if( splittedLine.at(1) == "X" )
					dictionary->remove(tag);
				//Replace with a dummy value
				else if( splittedLine.at(1) == "Z" || splittedLine.at(1) == "D" )
				{
					OFString tagValueOF;
					//If the tag exists
					if( dictionary->findAndGetOFString(tag, tagValueOF).good() )
						dictionary->putAndInsertString(tag, splittedLine.at(2).toStdString().c_str());
				}
				//We can keep it, just check if there is patient data and change it
				/*else if(splittedLine.at(1) == "C")
				{
					//TODO
				}*/
				//We should create a new UID
				else if( splittedLine.at(1) == "U" )
				{
					OFString tagValueOF;
					//Si le tag existait
					if (dictionary->findAndGetOFString(tag, tagValueOF).good())
						dictionary->putAndInsertString(tag, splittedLine.at(2).toStdString().c_str()); //TODO We need to replace it with a new UID
				}
				
			}
		}
	}
	
	//Replace patient ID with our patient ID
	dictionary->putAndInsertString(DcmTagKey(0x0010, 0x0020), _actualFinalId.toStdString().c_str());

	//Remove all the unknown tags (anything can be written)
	for(unsigned long i=0; i<dictionary->card(); i++) 
	{ 
		DcmElement* element = dictionary->getElement(i);
		DcmTag dcTag = element->getTag();
		DcmTagKey tag = DcmTagKey(element->getGTag(), element->getETag());

		QString tagName = dcTag.getTagName();
		if( tagName == "Unknown Tag & Data" )
		{
			dictionary->remove(tag);
			i--;
		}
	}
	//Specify we already anonymized this file
	//Patient Identity Removed (0012,0062)
	dictionary->putAndInsertString(DcmTagKey(0x0012,0x0062),"YES");

	//Define if the user will have to check for annotation in images
	bool annotationCheckToBeDone = false;
	QString BurnedInAnnotation;
	OFString BurnedInAnnotationOF;
	//Burned in annotation tag :
	//Indicates whether or not image contains sufficient burned in annotation to identify the patient and date the image was acquired . 
	//Enumerated Values: YES NO If this Attribute is absent, then the image may or may not contain burned in annotation.
	if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0028, 0x0301), BurnedInAnnotationOF).good())
	{
		BurnedInAnnotation = BurnedInAnnotationOF.c_str();
		if( BurnedInAnnotation == "YES" )
			annotationCheckToBeDone = true;
	}
	else
	{
		//OCR in images with "SCREEN SAVE" found in the tag 0008 0008 : imagetype
		QString imageType;
		OFString imageTypeOF;
		if (dictionary->findAndGetOFStringArray(DcmTagKey(0x0008, 0x0008), imageTypeOF).good())
			imageType = imageTypeOF.c_str();

		if( imageType.toLower().contains("screen save") )
			annotationCheckToBeDone = true;
	}
	
	Uint16 nbBitsAllocated;
	dictionary->findAndGetUint16(DcmTagKey(0x0028, 0x0100), nbBitsAllocated);
	//This tag allow you to know if a pixel value is an unsigned int (0) or a signed int (1)
	Uint16 pixelRepresentation = 0;
	dictionary->findAndGetUint16(DcmTagKey(0x0028, 0x0103), pixelRepresentation);
	E_TransferSyntax xfer = dictionary->getOriginalXfer();

	//Add the image to the list of images to be checked if there is a risk of annotations in pixels
	//We can't change the image if the number of bits allocated per pixel isn't 8 or 16
	if( annotationCheckToBeDone && (nbBitsAllocated == 8 || nbBitsAllocated == 16) )
	{		
		Uint16 rows, cols;

		//Get rows
		if(EC_Normal==dictionary->findAndGetUint16(DCM_Rows, rows))
		{
			//Get columns
			if(EC_Normal==dictionary->findAndGetUint16(DCM_Columns, cols))
			{
				if( !rowsForManualAnonym.contains(rows) || !colsForManualAnonym.contains(cols) )
				{
					rowsForManualAnonym.append(rows);
					colsForManualAnonym.append(cols);
					imagesForManualAnonym.append(QStringList(filePath));
				}
				else
				{
					for(int h=0; h<rowsForManualAnonym.size(); h++)
					{
						if( rowsForManualAnonym[h] == rows && colsForManualAnonym[h] == cols )
						{
							imagesForManualAnonym[h].append(filePath);
							break;
						}
					}
				}
			}
		}
	}
	
	saveAnonymizedDicom(filePath, &fileformat, dictionary, xfer);
}

/**
* \brief this function browse the waiting files for the manual anonymisation and rewrite the pixelData for each file after the user selected the area to remove
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::dicomManualAnonymization(QVector<QStringList> imagesForManualAnonym)
{
	for(int iGrp=0; iGrp<imagesForManualAnonym.size(); iGrp++)
	{
		QVector<BoundingBox*>* boundingBoxes = new QVector<BoundingBox*>();

		QVector<QImage*> imagesForLabel;
		QStringList actualFiles = imagesForManualAnonym[iGrp];

		for(int im=0; im<actualFiles.size(); im++)
		{
			QString toto = actualFiles[im];
			DcmFileFormat fileformat;
			OFCondition status = fileformat.loadFile(actualFiles[im].toStdString().c_str());
			DcmDataset *dictionary;
			if (status.good())				
				dictionary = fileformat.getDataset();
			
			//Get the transfer syntax to check if we need jpeg codecs
			E_TransferSyntax xfer = dictionary->getOriginalXfer();
			if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
					DJDecoderRegistration::registerCodecs(); // register JPEG codecs

			Uint16 samplePerPixel = 1; //Initialisation in case we can't get a proper value
			dictionary->findAndGetUint16(DCM_SamplesPerPixel, samplePerPixel);

			DicomImage *di = new DicomImage(&fileformat, xfer, 0UL, 0, 1);
			
			//Usually the value of the text can be the biggest value even if the image itself use only 1/10 of the possible values.
			//This function automatically render the image correctly without the problem of outer values
			di->setMinMaxWindow(1);
			if( samplePerPixel == 1 )
				di->writeBMP("output.bmp", 8, 0);
			else if( samplePerPixel == 3 )
				di->writeBMP("output.bmp", 24, 0);

			delete di;

			imagesForLabel.append(new QImage("output.bmp"));
		
			if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
				DJDecoderRegistration::cleanup();
		}	
		
		//Call the Dicom Display interface. it will delete de pointers in imagesForLabel
		ImageEditor disp(imagesForLabel, boundingBoxes, false);		
		disp.exec();

		for(int im=0; im<actualFiles.size(); im++)
		{
			DcmFileFormat fileformat;
			OFCondition status = fileformat.loadFile(actualFiles[im].toStdString().c_str());
			DcmDataset *dictionary;
			if (status.good())				
				dictionary = fileformat.getDataset();

			//Get the transfer syntax to check if we need jpeg codecs
			E_TransferSyntax xfer = dictionary->getOriginalXfer();
			if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
				DJDecoderRegistration::registerCodecs(); // register JPEG codecs

			Uint16 samplePerPixel = 1; //Initialisation in case we can't get a proper value
			dictionary->findAndGetUint16(DCM_SamplesPerPixel,samplePerPixel);

			//Get the number of rows and columns in the image
			Uint16 rows, cols;
			dictionary->findAndGetUint16(DCM_Rows, rows);
			dictionary->findAndGetUint16(DCM_Columns, cols);
			
			//Get the pixels of the image
			const Uint8* pixelValues8 = new Uint8();
			const Uint16* pixelValues16 = new Uint16();
			Uint16 nbBitsAllocated;
			dictionary->findAndGetUint16(DcmTagKey(0x0028, 0x0100), nbBitsAllocated);

			const OFBool searchIntoSub=OFFalse;
			OFCondition getDataok;
			//This has to be before the writeBMP.
			if( nbBitsAllocated == 8 )
				getDataok = dictionary->findAndGetUint8Array(DCM_PixelData, pixelValues8, 0, searchIntoSub);
			else if( nbBitsAllocated == 16 )
				getDataok = dictionary->findAndGetUint16Array(DCM_PixelData, pixelValues16, 0, searchIntoSub);
			if( getDataok != EC_Normal )
			{
				addProgressMessage(ui, "Can't get the pixels from PixelData, fail to anonymize " + actualFiles[im], Qt::red, "", false);
				continue;
			}

			//Get the pixelData lenght
			Uint32 pixelDataLenght = rows*cols*samplePerPixel*nbBitsAllocated/2;
						
			Uint8* newPixelValues8 = new Uint8[pixelDataLenght];
			Uint16* newPixelValues16 = new Uint16[pixelDataLenght];
				
			//We can't modify the original image.
			//Preparing a copy of the actual image.
			for(unsigned long y = 0; y < rows; y++)
				for(unsigned long x = 0; x < cols; x++)
				{
					int index=(x + y +  y*(cols-1))*samplePerPixel;
					if( nbBitsAllocated == 8 )
						copyPixel(pixelValues8, newPixelValues8, index, samplePerPixel);
					else if( nbBitsAllocated == 16 )
						copyPixel(pixelValues16, newPixelValues16, index, samplePerPixel);
				}
			
			//Delete all the pixels contained in the rectangles selected by the user.				
			for(int bb = 0; bb < boundingBoxes->size(); bb++)
			{
				BoundingBox* bbox = (*boundingBoxes)[bb];
				//The images numbers starts from 1 in the boundingboxes. we add 1 to im
				if( (bbox->isForAll() && !bbox->notIn().contains(im+1)) || (!bbox->isForAll() && bbox->imgNumber() == (im+1)) )
				{
					for(int x = bbox->x1(); x <= bbox->x2(); x++)
					{
						for(int y = bbox->y1(); y <= bbox->y2(); y++)
						{
							int index = (x + y*cols)*samplePerPixel;
							if( nbBitsAllocated == 8 )
								setDataGrayScale(newPixelValues8, index, samplePerPixel,0);
							else if( nbBitsAllocated == 16 )
								setDataGrayScale(newPixelValues16, index, samplePerPixel,0);
						}
					}
				}
			}

			OFCondition cond;
			//Write the anonymized image (with recognized words deleted)
			if( nbBitsAllocated == 8 )
				cond = dictionary->putAndInsertUint8Array(DCM_PixelData, (const Uint8*)newPixelValues8, pixelDataLenght, true);
			else if( nbBitsAllocated == 16 )
				cond = dictionary->putAndInsertUint16Array(DCM_PixelData, (const Uint16*)newPixelValues16, pixelDataLenght, true);

			if( cond.bad() )
				addProgressMessage(ui, "Cannot modify the pixels" + QString(cond.text()), Qt::red, "", false);
		
			if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
				DJDecoderRegistration::cleanup();
	
			saveAnonymizedDicom(actualFiles[im], &fileformat, dictionary, xfer);
			
			// Destroy used object and release memory
			delete [] newPixelValues8;
			delete [] newPixelValues16;
		}

		for(int bb = 0; bb < boundingBoxes->size(); bb++)
			delete (*boundingBoxes)[bb];
		delete boundingBoxes;
	}	
}

/**
* \brief This function saves the DICOM file after modification of the dictionary
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::saveAnonymizedDicom(QString filePath, DcmFileFormat *fileformat, DcmDataset *dictionary, E_TransferSyntax xfer)
{
	//If the Transfer syntax is JPEG something
	if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
	{
		DJEncoderRegistration::registerCodecs(); // register JPEG codecs

		DJ_RPLossless params; // codec parameters, we use the defaults
		// this causes the lossless JPEG version of the dataset to be created
		dictionary->chooseRepresentation(xfer, &params);
	}

	DcmItem *metaInfo = fileformat->getMetaInfo();
	// force the meta-header UIDs to be re-generated when storing the file
	// since the UIDs in the data set may have changed
	delete metaInfo->remove(DCM_MediaStorageSOPClassUID);
	delete metaInfo->remove(DCM_MediaStorageSOPInstanceUID);

	fileformat->loadAllDataIntoMemory();
	if( EC_Normal != fileformat->saveFile(filePath.toStdString().c_str(), xfer) )
		addProgressMessage(ui, "Cannot update the image.", Qt::red, "", false);
		
	if(xfer >= EXS_JPEGProcess1TransferSyntax && xfer <= EXS_JPEGProcess14SV1TransferSyntax )
		DJEncoderRegistration::cleanup();
}

/**
* \brief Set the value of a 8 bits pixel with a grayscale value
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param img : image to modify
* \param SamplesPerPixel : number of samples per pixel (3:RGB, 1:GrayScale)
* \param value : new 8 bits grayscale value
**/
void Anonymizer::setDataGrayScale(Uint8* img, int at, int SamplesPerPixel, Uint8 value)
{
	for(int spp=0; spp<SamplesPerPixel; spp++)
		img[at+spp] = value;
}

/**
* \brief Set the value of a 16 bits pixel with a grayscale value
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param img : image to modify
* \param SamplesPerPixel : number of samples per pixel (3:RGB, 1:GrayScale)
* \param value : new 16 bits grayscale value
**/
void Anonymizer::setDataGrayScale(Uint16* img, int at, int SamplesPerPixel, Uint16 value)
{
	for(int spp=0; spp<SamplesPerPixel; spp++)
		img[at+spp] = value;
}

/**
* \brief Copy a pixel from an image to another (8 bits)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param src : image source
* \param dest : image destination
* \param SamplesPerPixel : number of samples per pixel (3:RGB, 1:GrayScale)
**/
void Anonymizer::copyPixel(const Uint8* src, Uint8* dest, int at, int SamplesPerPixel)
{
	for(int spp=0; spp<SamplesPerPixel; spp++)
		dest[at+spp] = src[at+spp];
}

/**
* \brief Copy a pixel from an image to another (16 bits)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param src : image source
* \param dest : image destination
* \param SamplesPerPixel : number of samples per pixel (3:RGB, 1:GrayScale)
**/
void Anonymizer::copyPixel(const Uint16* src, Uint16* dest, int at, int SamplesPerPixel)
{
	for(int spp=0; spp<SamplesPerPixel; spp++)
		dest[at+spp] = src[at+spp];
}

/**
* \brief Write and anonymize the DICOMDIR file a the end of the anonymization by copying the previous one
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param toCopy : previous DICOMDIR we need to copy in our database and anonymize
* \param finalID : ID for the patient
* \param extractFolder : the number for the extract
**/
void Anonymizer::writeDICOMDIR(QString toCopy, QString finalID, QString outPutFolder, QString extractFolder)
{

	QString fileNameChar = outPutFolder + "\\" + finalID + "\\" + extractFolder + "\\DICOMDIR";
	addProgressMessage(ui,"Copying DICOMDIR file ...", Qt::black, "", false);
	createCopyThread(toCopy, fileNameChar, false);
	
	DcmDicomDir dicomdirAnonym(fileNameChar.toStdString().c_str());
	DcmDirectoryRecord *rootAnonym = &dicomdirAnonym.getRootRecord();
	DcmDirectoryRecord *PatientRecordAnonym = NULL;
	DcmDirectoryRecord *SeriesRecordAnonym = NULL;
	DcmDirectoryRecord* imageAnonym = NULL;
	int StudyNumber = 0;
		
	while (((PatientRecordAnonym = rootAnonym->nextSub(PatientRecordAnonym)) != NULL)) 
	{
		//Anonymisation du fichier DICOMDIR (suppression nom du patient)
		PatientRecordAnonym->findAndDeleteElement(DCM_PatientName);
		DcmDirectoryRecord *StudyRecordAnonym = NULL;
		while (((StudyRecordAnonym = PatientRecordAnonym->nextSub(StudyRecordAnonym)) != NULL)) 
		{
			StudyRecordAnonym->findAndDeleteElement(DCM_StudyDescription);
			StudyRecordAnonym->findAndDeleteElement(DCM_StudyDate);
			StudyRecordAnonym->findAndDeleteElement(DCM_StudyTime);
			while ((SeriesRecordAnonym = StudyRecordAnonym->nextSub(SeriesRecordAnonym)) != NULL)
			{
				StudyRecordAnonym->findAndDeleteElement(DCM_SeriesDescription);
				while ((imageAnonym = SeriesRecordAnonym->nextSub(imageAnonym)) != NULL)
				{
					const char* sNameAnonym;
					imageAnonym->findAndGetString(DCM_ReferencedFileID, sNameAnonym);

					//Check if the file is in the destination directory
					QFile fromFile(outPutFolder + "/" + finalID + "/" + extractFolder + "/" + sNameAnonym);
					if ( fromFile.open(QIODevice::ReadOnly) )
						imageAnonym->putAndInsertString(DCM_ReferencedFileID, sNameAnonym, true);
					//If it does not exist, the program skipped it cause it was unreadable, we don't keep it in the DICOMDIR
					else
						imageAnonym->findAndDeleteElement(DCM_ReferencedFileID);
				}
			}
			StudyNumber++;
		}
	}

	dicomdirAnonym.write();
}

//////////////////
//     BOTH     //
//////////////////

/**
* \brief Get the extract number based on the maximum extract number found in the patient
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
int Anonymizer::getExtractNumber(Patient* pat)
{
	int extractNumber = 0;
	QVector<Image*> imgs = pat->getImgs();
	for(int i=0; i<imgs.size(); i++)
	{
		Image* img = imgs[i];

		QVector<Study*> studies = img->getStudies();
		for(int st=0; st<studies.size(); st++)
		{
			Study* study = studies[st];

			QVector<Series*> series = study->getSeries();
			for(int se=0; se<series.size(); se++)
			{
				Series* serie = series[se];

				QVector<Extract*> extracts = serie->getExtracts();
				for(int e=0; e<extracts.size(); e++)
				{
					Extract* extract = extracts[e];
					if( extractNumber < extract->getNumber() )
						extractNumber = extract->getNumber();
				}
			}
		}
	}

	extractNumber++;
	return extractNumber;
}

/**
* \Get the patient number based on the number of patient already anonymized (-1 if there is the allTheOthers patient 99999)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
int Anonymizer::getPatientNumber()
{
	//The patient number is the number of private files created (one per patient)
	//If one of them contain the number 99999, we don't count it.
	QDir dir(Config::PATIENT_PV_PATH);
	dir.setFilter(QDir::Files);
	int nbFiles = dir.count();
	
	QStringList dirList = dir.entryList(QDir::Files);

	//The patient 99999 is the allTheOthers. He contains all the data for which we don't know the patient
	for(int i=0; i<dirList.size(); i++)
		if(dirList[i].contains("99999"))
			nbFiles--;

	return nbFiles;
}

/**
* \brief Create a Thread to copy a file and wait it ends
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void Anonymizer::createCopyThread(QString befPath, QString aftPath, bool useProgBar)
{
	_thread = new QThread();
	Copy* copyThread = new Copy(befPath, aftPath, useProgBar, &_canCopy, &_copyHasToStop);
	copyThread->moveToThread(_thread);
	connect(copyThread, SIGNAL(setProgress(int)), ui->fileProgressBar, SLOT(setValue(int)));
	connect(copyThread, SIGNAL(setProgressMax(int)), ui->fileProgressBar, SLOT(setMaximum(int)));
	connect(copyThread, SIGNAL(setProgressLabel(const QString &)), ui->progressCurrentFileLabel, SLOT(setText(const QString &)));
	connect(_thread, SIGNAL(started()), copyThread, SLOT(slot_copy()));
	connect(copyThread, SIGNAL(finished()), _thread, SLOT(quit()));
	connect(copyThread, SIGNAL(finished()), copyThread, SLOT(deleteLater()));
	connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
	_canCopy = false;
	_thread->start();

	while( !_canCopy )
	{
		qApp->processEvents();
		//The copy thread will set canCopy to true after this
		if( _isCanceled )
			_copyHasToStop = true;
	}
}

/**
* \brief Sort the videos by size of frame and get the boundingboxes from user
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::getBBandImgGroups(QStringList &files, QVector<QVector<BoundingBox*>*> &allBoundingBoxes, QVector<QStringList> &filesOrdered)
{
	QVector<double> heights, widths;
	QVector<QVector<QImage*>> firstFrames;

	for(int i=0; i<files.size(); i++)
	{
		files[i] = files[i].replace("\\","/");
		cv::VideoCapture capture(files[i].toStdString().c_str());
			
		double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
		double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
		
		cv::Mat frame;
		capture.read(frame);
		
		QImage* imgFrame = Mat2QImage(frame);

		//We are creating groups for each sizes of images
		if( !heights.contains(dHeight) || !widths.contains(dWidth) )
		{
			heights.append(dHeight);
			widths.append(dWidth);
			firstFrames.append(QVector<QImage*>());
			firstFrames[firstFrames.size()-1].append(imgFrame);
			filesOrdered.append(QStringList());
			filesOrdered[filesOrdered.size()-1].append(files[i]);
		}
		else
		{
			for(int h=0; h<heights.size(); h++)
			{
				if( heights[h] == dHeight && widths[h] == dWidth )
				{
					firstFrames[h].append(imgFrame);
					filesOrdered[h].append(files[i]);
					break;
				}
			}
		}
	}
	
	//For each different sizes of images
	for(int fra=0; fra<firstFrames.size(); fra++)
	{
		QVector<BoundingBox*>* boundingBoxes = new QVector<BoundingBox*>();
		//Call the Dicom Display interface. it will delete de pointers in imagesForLabel
		ImageEditor disp(firstFrames[fra], boundingBoxes, true);
		if( !disp.exec() )
			_isCanceled = true;
		allBoundingBoxes.append(boundingBoxes);
	}
}

/**
* \brief Clear the boundingboxes list selected by the user after using it
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::clearBBs(QVector<QVector<BoundingBox*>*> &allBoundingBoxes)
{	
	//Clear bounding boxes and images
	for(int bba=0; bba<allBoundingBoxes.size(); bba++)
	{
		for(int bbs=0; bbs<allBoundingBoxes[bba]->size(); bbs++)
			delete allBoundingBoxes[bba]->at(bbs);
		delete allBoundingBoxes[bba];
	}	
}

/**
* \brief Refresh the patient list managed by the user
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::refreshPatientList()
{
	_patientList->clear();
	//Filter for xml files
	QStringList listFilter;
	listFilter << "*.xml";
	//Find the list of patients directory
	QDirIterator dirIterator(Config::PATIENT_PV_PATH, listFilter, QDir::Files);
	//For each patient's file
	while(dirIterator.hasNext())
	{
		QString actualFile = dirIterator.next();
		std::cout << actualFile.toStdString() << std::endl;
		QString id, name, firstname;
		QDate birth;
		QDomDocument doc;
		//Fill the QDomDocument with the xml file
		fillDomDocFromXmlFile(doc, actualFile, ui);

		//Add a patient to our list of patients
		_patientList->append(Patient(getElemAttributeValue(doc, "Patient", "Id"),getElemValue(doc, "Name"),getElemValue(doc, "FirstName"),getElemValue(doc, "Birth")));
	}
}

/**
* \brief add a patient to the list managed by the software
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::addPatientToList(Patient p)
{
	bool patInBDD = false;
	for(int pa=0; pa<_patientList->size(); pa++)
		if( (*_patientList)[pa].getId() == p.getId() )
		{
			patInBDD = true;
			break;
		}
	if( !patInBDD )
		_patientList->append(p);
}

/**
* \brief This function browse the database trying to find if a patient is already created or not.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param _finalID is returned with the value of the patient found if there is one
* \return true if the patient already exists
**/
bool Anonymizer::patientAlreadyExists(QString &_name, QString &_firstname, QString &_birth, QString &_finalID)
{
	QString nameTmp, firstNameTmp;
	nameTmp = _name;
	firstNameTmp = _firstname;
	nameTmp = nameTmp.toLower();
	firstNameTmp = firstNameTmp.toLower();

	//If the patient has no name (already anonymized or not filled)
	if( nameTmp == "unknown" || nameTmp == "unknow" || nameTmp == "" || firstNameTmp == "unknown" || firstNameTmp == "unknow" || firstNameTmp == "" )
	{
		_finalID = "99999";
		_name = "allTheOthers";
		_firstname = "allTheOthers";
		_birth = "01/01/9999";

		QVectorIterator<Patient> i(*_patientList);
		while (i.hasNext())
		{
			Patient p = i.next();
			if( p.getId() == "99999" )
				return true;
		}
		_patientList->append(Patient(_finalID, _name, _firstname, _birth));
		return false;
	}
	
	//We create a fake patient with the values from the arguments to compare it with the database.
	Patient tmp("whoCares", _name, _firstname, _birth);

	QVector<int> similars;
	for(int k=0; k<_patientList->size(); k++)
	{
		Patient p = (*_patientList)[k];
		//If we have the name and firstname but not the birth date, we add it to a list of similar patient that will be analyzed after.
		if( _name == p.getName() && _firstname == p.getFirstName() && ( _birth == "" || p.getBirth() == "" ) )
		{
			similars.append(k);
		}
		//If the patient has normal informations (name + firstname + birthDate)
		else if( p == tmp )
		{
			_finalID = p.getId();
			_actualFinalId = _finalID;
			return true;
		}
	}
	
	if( similars.size() > 0 )
	{
		//If we found patients with same names but not same birth
		int toGo = 0;
		//If there is one similar patient only, we ask the user if he wants to group them.
		if( similars.size() == 1 )
		{
			Patient p = (*_patientList)[similars[0]];
			QMessageBox::StandardButton reply = QMessageBox::question(0, "Patient maybe found", 
				"The patient already exists but the birth dates are not the same (new<" + _birth + "> and old<" + p.getBirth() + ">). Do we still act like they are the same ? (If not, we will just create a new ID for the files you actually anonymize)", QMessageBox::Yes|QMessageBox::No);
			if (reply == QMessageBox::Yes) 
			{
				if( _birth != "" )
					(*_patientList)[similars[0]].setBirth(_birth);
				else
					_birth = p.getBirth();
				_finalID = p.getId();
				_actualFinalId = _finalID;
				return true;
			}
		}
		//If there is more than one similar patient, we ask the user for each patient if he wants to group with the actual patient.
		else
		{
			for(int k=0; k<similars.size(); k++)
			{
				toGo = similars.size()-k-1;
				Patient p = (*_patientList)[similars[k]];
				QMessageBox::StandardButton reply = QMessageBox::question(0, "Patient maybe found", 
					"The patient already exists (" + QString::number(similars.size()) + " possibilities) but the birth dates are not the same (new<" + _birth + "> and old<" + p.getBirth() + ">). Do want to act like this two are the same ("  + QString::number(toGo) + " other choice(s) after) ? (If not, we will just create a new ID for the files you actually anonymize)", QMessageBox::Yes|QMessageBox::No);
				if (reply == QMessageBox::Yes) 
				{
					if( _birth != "" )
						(*_patientList)[similars[k]].setBirth(_birth);
					else
						_birth = p.getBirth();
					_finalID = p.getId();
					_actualFinalId = _finalID;
					return true;
				}
			}
		}
	}

	return false;
}

/**
* \brief create and fill file for the private data of a patient.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void Anonymizer::writePrivateDataFile(bool patientFound, QString finalID, QString nameTmp, QString firstNameTmp, QString birthTmp, QString sexTmp)
{
	QDomElement patientNode = QDomElement();

	QMap<QString, QString> attributes;
	QMap<QString, QMap<QString, QString>> allParentsAttr;

	///- CREATE NEW ORDERED DOCUMENT PRIVATE DATA (PATIENT PERSONNAL INFORMATIONS)
	QDomDocument docWritePV;
	if(patientFound)
		fillDomDocFromXmlFile(docWritePV, Config::PATIENT_PV_PATH + "\\" + finalID + "_PV.xml", ui);
		
	///- FILL THE DOCUMENT FOR PRIVATE DATA
	//Add Element Patient
	attributes.insert("Id",finalID);
	writeElement(docWritePV, patientNode, "Patient", "", attributes, "", allParentsAttr, ui);
	allParentsAttr.insert("Patient", attributes);
	attributes.clear();
	//Add Elements : Name, FirstName, Birth, Sex
	writeElement(docWritePV, QDomElement(), "Name", nameTmp, attributes, "Patient", allParentsAttr, ui);
	writeElement(docWritePV, QDomElement(), "FirstName", firstNameTmp, attributes, "Patient", allParentsAttr, ui);
	if( birthTmp != "" )
		writeElement(docWritePV, QDomElement(), "Birth", birthTmp, attributes, "Patient", allParentsAttr, ui);
	if(sexTmp == "F" || sexTmp == "M")
		writeElement(docWritePV, QDomElement(), "Sex", sexTmp, attributes, "Patient", allParentsAttr, ui);

	///- CREATE FILES IF NEEDED OR SET EXISTING FILES.
	writeXML(Config::PATIENT_PV_PATH + "\\" + finalID + "_PV.xml", docWritePV, 4);
}

/**
* \brief Cancel the running anonymization and delete the file created during it
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
void Anonymizer::cancelActualAnonymisation(int extractNumber, QString outPutFolder, QString extractFolder)
{
	ui->cancelCurrentAnonymisationButton->setEnabled(false);
	addProgressMessage(ui,"\nYou just stopped the treatment of data.", Qt::black, "", true);
	_isCanceled = false;
	addProgressMessage(ui,"Trying to delete the created data (the program will try until it's done)...", Qt::black, "", true);
	if( extractNumber == 1 )
		rmNonEmptyDir(outPutFolder + "\\" + _actualFinalId);
	else
		rmNonEmptyDir(outPutFolder + "\\" + _actualFinalId + "\\" + extractFolder);
	addProgressMessage(ui,"The data have been cleared correctly.", Qt::black, "", true);
	ui->anonymisationProgressBar->setValue(0);
	ui->fileProgressBar->setValue(0);	

	(*_remainingAnon)--;
	ui->remainingAnonLabel->setText(QString::number((*_remainingAnon)) + " remaining");
	if( (*_remainingAnon) == 0 )
		ui->remainingAnonLabel->setVisible(false);

	*_workDoneBool = true;
	_canCopy = true;
	_copyHasToStop = false;
}

void Anonymizer::prepareProgressTextForRewrite()
{
	ui->progressTextDetailled->setFocus();
	QTextCursor storeCursorPos = ui->progressTextDetailled->textCursor();
	ui->progressTextDetailled->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
	ui->progressTextDetailled->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
	ui->progressTextDetailled->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
	ui->progressTextDetailled->textCursor().removeSelectedText();
	ui->progressTextDetailled->textCursor().deletePreviousChar();
	ui->progressTextDetailled->setTextCursor(storeCursorPos);
	ui->progressTextDetailled->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
}

///////////////////
//     TESTS     //
///////////////////

/**
* \brief This function retrieve all the metadata of an element in the DICOMDIR hierarchy
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Anonymizer::getPropertiesFromDicomDirHierarchy(DcmDirectoryRecord* hierarchyRecord, QVector<QVector<QString>> &properties)
{
	OFCondition status = EC_TagNotFound;
	DcmStack stack;
	DcmObject *object = NULL;

	//Browse the properties of the element
	while (hierarchyRecord->nextObject(stack, OFTrue).good()) 
	{
		object = stack.top();
		DcmTag tag = object->getTag();
		DcmTagKey tagKey = DcmTagKey(tag.getGroup(), tag.getElement());
		OFString tagVal;
		hierarchyRecord->findAndGetOFString(tagKey, tagVal);
		//Add the property to the list		
		QVector<QString> oneProp;
		oneProp.append(QString(tag.toString().c_str()));
		oneProp.append(QString(tag.getTagName()));
		oneProp.append(QString(tagVal.c_str()));
		properties.append(oneProp);
	}
}
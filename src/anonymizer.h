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

#ifndef ANONYMIZER_H
#define ANONYMIZER_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include "vector"
//OpenCV
#include <ole2.h>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//Qt includes
#include <QString>
#include <QObject>
#include <QtXml>
#include <QMainWindow>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
//Includes DCMTK  
#include "dcdicdir.h" 
#include "dcdeftag.h" 
#include "dcddirif.h" 
#include "dcmtk/config/osconfig.h" 
#include "dctk.h"
#include "dcmimage.h" 
#include "diregist.h"    // Support for color images 
#include "dcrledrg.h"    // Support for RLE images 
#include "djdecode.h"    // Support for JPEG images 
#include "djencode.h"    // Support for JPEG images 
#include "dcmtk/dcmdata/dcpxitem.h" 
#include "djrplol.h"
//My includes 
#include "copy.h" 
#include "patient.h" 
#include "xmlTools.h" 
#include "askdialog.h" 
#include "askPatientInfo.h" 
#include "imageEditor.h" 
#include "config.h" 
#include "tools.h" 
#include "boundingBox.h"
#include "anonymization.h"

// Implicitly link ole32.dll
#pragma comment( lib, "ole32.lib" )


const FMTID PropSetfmtid ={
/* F29F85E0-4FF9-1068-AB91-08002B27B3D9 */
        0xf29f85e0,
        0x4ff9,
        0x1068,
        {0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 }
        };

/**
* \class Anonymizer
* \brief This function handle all the anonymization part
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Anonymizer : public QObject
{
	Q_OBJECT

public:
	Anonymizer(QVector<Patient>* pl, bool *wdBool, int* remainingAno, Ui::MainWindow *ui);

public slots:
	void slot_AskForAnonymize(Anonymization a); //DICOMDIR and XML file
	void slot_AskForAnonymizeFiles(Anonymization a); //Videos or DICOM files
	void slot_cancelAnonymization() { _isCanceled = true; }

private:
	//Videos
    void xmlFile(Anonymization a);
	void videos(Anonymization a);
	void deleteFileDetailledProperties(QString srcFile, QString destinationFile);
	void modifyVideoFrames(QString srcFile, QString &destinationVideo, int extractNumber, QString extractFolder, QVector<BoundingBox*>* boundingBoxes, int imgNumber);
	//DICOM
    void dicomDir(Anonymization a);
	void dicomFiles(Anonymization a);
	int countNbFilesFromDICOMDIR(QString fileName);
	void dicomGetPatientPV(DcmDataset *dictionary, QString &nameTmp, QString &firstNameTmp, QString &birthTmp, QString &sexTmp);
	void anonymizeDicomFile(QString filePath, QVector<int> &rowsForManualAnonym, QVector<int> &colsForManualAnonym, QVector<QStringList> &imagesForManualAnonym);
	void dicomManualAnonymization(QVector<QStringList> imagesForManualAnonym);
	void saveAnonymizedDicom(QString filePath, DcmFileFormat *fileformat, DcmDataset *dictionary, E_TransferSyntax xfer);
	void setDataGrayScale(Uint8* img, int at, int SamplesPerPixel, Uint8 value);
	void setDataGrayScale(Uint16* img, int at, int SamplesPerPixel, Uint16 value);
	void copyPixel(const Uint8* src, Uint8* dest, int at, int SamplesPerPixel);
	void copyPixel(const Uint16* src, Uint16* dest, int at, int SamplesPerPixel);
	void writeDICOMDIR(QString toCopy, QString finalID, QString outPutFolder, QString extractFolder);
	//Both
	int getExtractNumber(Patient* pat);
	int getPatientNumber();	
	void createCopyThread(QString befPath, QString aftPath, bool useProgBar);
	void getBBandImgGroups(QStringList &files, QVector<QVector<BoundingBox*>*> &allBoundingBoxes, QVector<QStringList> &filesOrdered);
	void clearBBs(QVector<QVector<BoundingBox*>*> &allBoundingBoxes);
	void refreshPatientList();
	void addPatientToList(Patient p);
	bool patientAlreadyExists(QString &_name, QString &_firstname, QString &_birth, QString &_finalID);
	void writePrivateDataFile(bool patientFound, QString finalID, QString nameTmp, QString firstNameTmp, QString birthTmp, QString sexTmp);
	void cancelActualAnonymisation(int extractNumber, QString outPutFolder, QString extractFolder);
	void prepareProgressTextForRewrite();
	
	//Attributes
    Ui::MainWindow *ui;
	int *_remainingAnon;
	QStringList _anonymousTags;
	QString _actualFinalId;
	bool _isCanceled; // This boolean allow the cancel button to say to the running treatment he have to stop
	QVector<Patient>* _patientList; // List of patients already created
	bool *_workDoneBool;
	QThread* _thread;
	bool _canCopy;
	bool _copyHasToStop;

	//Tests (not used at the moment)
	void getPropertiesFromDicomDirHierarchy(DcmDirectoryRecord* hierarchyRecord, QVector<QVector<QString>> &properties);
};

#endif // ANONYMIZER_H
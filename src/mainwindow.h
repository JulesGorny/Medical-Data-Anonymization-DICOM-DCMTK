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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
//Includes Qt
#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QString>
#include <QUrl>
#include <QVector>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QThread>
#include <QMetaType>
//My files
#include "patient.h"
#include "scanning.h"
#include "anonymizer.h"
#include "anonymization.h"

namespace Ui {
class MainWindow;
}

class Anonymizer;

/**
* \class MainWindow
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_xmlDataButton_clicked();
	void on_videoButton_clicked();
    void on_dicomDirButton_clicked();
	void on_dicomFilesButton_clicked();
    void on_anonymizedFilesPathButton_clicked();
	void slot_destinationChange();
	void on_useViewerDBButton_clicked();
	void on_resetDefaultPathButton_clicked();
	void on_helpButton_clicked();
	void on_NameToIDButton_clicked();
	void on_IDToNameButton_clicked();
	void on_dlButtonNtoID_clicked();
	void on_ulButtonNtoID_clicked();
	void on_dlButtonIDtoN_clicked();
	void on_ulButtonIDtoN_clicked();
	void on_editIDtoN();
	void on_editNtoID();
	void resizeEvent (QResizeEvent * event);
	void closeEvent(QCloseEvent * event);

private:
	void setVisibleMatchDestination(bool b);

	void setEnableDLandULButtonsIDtoN(bool b);
	void setEnableDLandULButtonsNtoID(bool b);
	void setEnableDLandULButtons(bool b);

	void dlNonAnonymousDocs(QString patientID);
	void ulNonAnonymousDocs(QString patientID);

	void createScanningThread();

	QString _browseFolder;
	Anonymizer* _anonymizer;
	QMessageBox* _helpMessage;

    Ui::MainWindow* ui;
	QVector<Patient>* _patientList; // List of patients already created
	Scanning* _scan;
	int *_remainingAnon;
	//This variable allow the Anonymizer to control the scanning thread
	QList<Anonymization>* _toDo;
	bool _workDoneBool;
	//The scanning thread can't start until scanCanStartWorking is true
	bool _scanCanStartWorking;
	QString _anonymousFilesPathText;
};

#endif // MAINWINDOW_H

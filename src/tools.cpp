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

/**
* \file tools.cpp 
* \brief regroup tools to manipulate dicom files.
*/

#include "tools.h"

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void addProgressMessage(Ui::MainWindow *ui, QString message, Qt::GlobalColor color, QString errorName, bool isUnDetailled)
{
	QTextCharFormat tf;
	tf = ui->progressTextDetailled->currentCharFormat();
	tf.setForeground(QBrush(color));
	ui->progressTextDetailled->setCurrentCharFormat(tf);
	if( isUnDetailled )
		ui->progressTextSummary->setCurrentCharFormat(tf);

	if( errorName == "missingInterestingXMLDataFile" )
	{
		ui->progressTextDetailled->appendPlainText("The file containing the interesting data in any XML file cannot be open. Check the variable called INTERESTING_DATA_XML in the"
							  "config.h file from the src folder (maybe the path is not the good one. It has to refer to the file interestingXmlElements.txt in the folder interestingData)).\n"
							  "If there is no such file, you can create one called interestingXmlElements.txt, place it in the interestingData folder, and fill it with the html tags you want to extract.\n"
							  "Write a line per tag like that : HTMLtag __ Name of the tag (not important). \n\n");

		if( isUnDetailled )
			ui->progressTextSummary->appendPlainText("The file containing the interesting data in any XML file cannot be open. Check the variable called INTERESTING_DATA_XML in the"
								  "config.h file from the src folder (maybe the path is not the good one. It has to refer to the file interestingXmlElements.txt in the folder interestingData)).\n"
								  "If there is no such file, you can create one called interestingXmlElements.txt, place it in the interestingData folder, and fill it with the html tags you want to extract.\n"
								  "Write a line per tag like that : HTMLtag __ Name of the tag (not important). \n\n");
	}
	else if( errorName == "")
	{
		ui->progressTextDetailled->appendPlainText(message);
		if( isUnDetailled )
			ui->progressTextSummary->appendPlainText(message);
	}		
	qApp->processEvents();
}

/**
* \brief Copy a file from a path to another
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param beforePath : path of the file to copy
* \param afterPath : path where the copy is created
* \return true if there is no problem, false else
*/
bool copyFile(QString beforePath, QString afterPath, bool useProgressBar, Ui::MainWindow *ui)
{	
	QFile fromFile(beforePath);
	QFile toFile(afterPath);
	if (!fromFile.open (QIODevice::ReadOnly)) 
	{
        QMessageBox::critical(0, "Error : ", "can't open file to copy."); 
		return false;
	}
  
    qint64 fichier_taille = fromFile.size();
	int fileSizeMo = int(fichier_taille)/1000/1024;
	//If we are treating a big file (>10Mo), we will show a QProgressDialog to show the user the progression of the copy
	if( fichier_taille > 10000000 )
	{
		int bufferSize = 32768;
		if( useProgressBar )
		{
			ui->fileProgressBar->setMaximum(fichier_taille/bufferSize);
			ui->fileProgressBar->setValue(0);
		}

		if (!toFile.open (QIODevice::Truncate | QIODevice::Append))  
			QMessageBox::critical(0, "Error : ", "can't open the destination file.");

		int bytesReadTotal = 0;
  
		while (!fromFile.atEnd())
		{
			QByteArray buffer(fromFile.read(bufferSize));
			if( useProgressBar )
			{
				bytesReadTotal ++;
				int copiedMo = bytesReadTotal*bufferSize/1000/1024;
				ui->fileProgressBar->setValue(bytesReadTotal);
				ui->progressCurrentFileLabel->setText("Current file\n" + QString::number(copiedMo) + "/" + QString::number(fileSizeMo) +"Mo");
			}
			toFile.write(buffer);  
			if (fromFile.error () || toFile.error ())  
			{
				QMessageBox::critical(0, "Error : ", "fail during the transfer.");  
				return false;
			}
		}
	}
	//Else, we do a quick copy
	else
		return QFile::copy(beforePath, afterPath);
	
	if( useProgressBar )
		ui->fileProgressBar->setValue(0);
	return true;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
QImage* Mat2QImage(cv::Mat src)
{
     cv::Mat temp;
     cvtColor(src, temp, CV_BGR2RGB);

     QImage* dest = new QImage((const uchar *) temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
     dest->bits();
     return dest;
}

/**
* \brief Delete a folder and all the elements contained in that folder. This function keeps trying until the folder is deleted.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param _name : path of the folder
*/
bool rmNonEmptyDir(QString _name)
{
	//If somehow, the folder doesn't exist, we just return.
	if( !QDir(_name).exists() )
		return true;

    QDir dir(_name);
    QFileInfoList list;
    QStringList filter;
    filter << "*.lnk";
    // liste des raccourcis vers répertoires
    list = dir.entryInfoList(filter, QDir::Dirs | QDir::Hidden | QDir::System);
    foreach (QFileInfo elem, list)
        if (elem.isSymLink()) // on vérifie qu'il s'agit bien d'un lien symbolique
            if (!QFile::remove(elem.absoluteFilePath()))
			return rmNonEmptyDir(elem.absoluteFilePath());

    list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::System);
    foreach (QFileInfo elem, list)
        if (!dir.remove(elem.absoluteFilePath()))
			return rmNonEmptyDir(elem.absoluteFilePath());

    list = dir.entryInfoList(QDir::AllDirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDir::DirsLast);
    foreach (QFileInfo elem, list)
        if (!rmNonEmptyDir(elem.absoluteFilePath()))
			return rmNonEmptyDir(elem.absoluteFilePath());

    if( !dir.rmdir(dir.absolutePath()) )
		return rmNonEmptyDir(dir.absolutePath());

	return true;
}
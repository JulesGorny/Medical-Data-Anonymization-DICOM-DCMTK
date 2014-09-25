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

#include "copy.h"

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Copy::Copy(QString befPath, QString aftPath, bool useProgBar, bool *canCop, bool *stop)
{
	_beforePath = befPath;
	_afterPath = aftPath;
	_useProgressBar = useProgBar;
	_canCopy = canCop;
	_copyHasToStop = stop;
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Copy::~Copy()
{ }

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Copy::slot_copy()
{
	QFile fromFile(_beforePath);
	QFile toFile(_afterPath);
	if (!fromFile.open (QIODevice::ReadOnly)) 
	{
		QMessageBox::critical(0, "Error : ", "can't open file to copy."); 
		*_canCopy = true;
		emit finished();
		return;
	}
  
	qint64 fichier_taille = fromFile.size();
	int fileSizeMo = int(fichier_taille)/1000/1024;
	//If we are treating a big file (>10Mo), we will show a QProgressDialog to show the user the progression of the copy
	if( fichier_taille > 10000000 )
	{
		int bufferSize = 32768;
		if( _useProgressBar )
		{
			emit setProgressMax(fichier_taille/bufferSize);
			emit setProgress(0);
		}

		if (!toFile.open (QIODevice::Truncate | QIODevice::Append))  
		{
			QMessageBox::critical(0, "Error : ", "can't open the destination file.");
			*_canCopy = true;
			emit finished();
			return;
		}

		int bytesReadTotal = 0;
		
		while (!fromFile.atEnd() && !*_copyHasToStop)
		{
			//Fill the buffer
			QByteArray buffer(fromFile.read(bufferSize));
			int bytesInBuffer = buffer.size();

			//Calculate the actual progress of the copy
			if( _useProgressBar )
			{
				bytesReadTotal ++;
				int copiedMo = bytesReadTotal*bufferSize/1000/1024;
				emit setProgress(bytesReadTotal);
				emit setProgressLabel("Current file\n" + QString::number(copiedMo) + "/" + QString::number(fileSizeMo) +"Mo");
			}

			//Write the buffer in the destination file
			toFile.write(buffer);

			if (fromFile.error () || toFile.error ())  
			{
				QMessageBox::critical(0, "Error : ", "fail during the transfer.");
				*_canCopy = true;  
				emit finished();
				return;
			}
		}
	}
	//Else, we do a quick copy
	else
		QFile::copy(_beforePath, _afterPath);
	
	if( _useProgressBar )
	{
		emit setProgress(0);
		emit setProgressLabel("Current file");
	}
	*_canCopy = true;
	*_copyHasToStop = false;
    emit finished();
}
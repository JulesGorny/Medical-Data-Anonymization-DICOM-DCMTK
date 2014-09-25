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

#ifndef COPY_H
#define COPY_H

//Qt includes
#include <QObject>
#include <QFile>
#include <QMessageBox>

/**
* \class Copy
* \brief this class has been created to copy a file in a thread (to avoid the freeze of the main window)
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
class Copy : public QObject
{
	Q_OBJECT

public:
	Copy(QString befPath, QString aftPath, bool useProgBar, bool* canCop, bool* stop);
    ~Copy();
	 
public slots:
    void slot_copy();
 
signals:
    void finished();
	//Allow this class to ask for modification of the file progress bar
	void setProgress(int value);
	void setProgressMax(int value);
	void setProgressLabel(const QString &msg);

private:
	QString _beforePath;
	QString _afterPath;
	bool _useProgressBar;
	bool* _canCopy;
	bool* _copyHasToStop;
};

#endif
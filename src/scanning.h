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

#ifndef SCANNING_H
#define SCANNING_H

//Qt includes
#include <QObject>
#include <QPlainTextEdit>
#include <QTimer>
//My includes
#include "anonymization.h"

/**
* \class Scanning
* \brief This class is managing the anonymization queue
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class Scanning : public QObject
{
	Q_OBJECT

public:
	Scanning(QList<Anonymization> *todo, bool *wdBool, bool *icswBool);
    ~Scanning();
	 
public slots:
    void process();
 
signals:
    void finished();
    void emitMsg(QString text);
	void anonymize(Anonymization a);
	void anonymizeFiles(Anonymization a);

private:
	QList<Anonymization>* _toDo;
	bool *_workDoneBool;
	bool *_iCanStartWorking;
};

#endif // SCANNING_H
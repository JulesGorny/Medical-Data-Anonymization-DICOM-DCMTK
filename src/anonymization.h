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

#ifndef ANONYMIZATION_H
#define ANONYMIZATION_H

#include <QMetaType>

/**
* \struct Anonymization
* \brief This structure describe an anonymization
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* An anonymization object is created when the user launch a new anonymization
* This object is placed in the queue.
**/
struct Anonymization
{
	QStringList _fileNames;
	QString _anonType;
	bool _destinationMatch;
	QString _bddFolder;
};

//Then Qt knows what is Anonymization for the signals/slots
//with qRegisterMetaType<Anonymization>("Anonymization"); at the start of MainWindow constructor
Q_DECLARE_METATYPE(Anonymization)

#endif
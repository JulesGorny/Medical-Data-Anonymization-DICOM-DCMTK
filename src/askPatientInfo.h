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

#ifndef ASKPATIENTINFO_H
#define ASKPATIENTINFO_H

//Qt includes
#include <QDialog>
#include <QLabel>
#include <QMessageBox>

namespace Ui {
    class AskPatientInfo;
}

/**
* \class AskDialog
* \brief This class allows us to ask the user if he knows the patient
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
class AskPatientInfo : public QDialog
{
    Q_OBJECT
public:
    AskPatientInfo(QWidget *parent = 0);
	virtual ~AskPatientInfo();
	void GetData(QString &name, QString &firstname, QString &sex, QString &birth);

private slots:
	void slot_editValidateState(QString text);
	void slot_dontKnowClicked();

private:
	Ui::AskPatientInfo *dlg;
};

#endif // ASKPATIENTINFO_H
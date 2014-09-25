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

#include "askPatientInfo.h"
#include "ui_askPatientInfo.h"

/**
* \brief AskPatientInfo class constructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
AskPatientInfo::AskPatientInfo(QWidget *parent) : QDialog(parent), dlg(new Ui::AskPatientInfo)
{
	dlg->setupUi(this);

	dlg->validateButton->setEnabled(false);
	connect(dlg->validateButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(dlg->dunnoButton, SIGNAL(clicked()), this, SLOT(slot_dontKnowClicked()));
	connect(dlg->firstname, SIGNAL(textChanged(QString)), this, SLOT(slot_editValidateState(QString)));
	connect(dlg->name, SIGNAL(textChanged(QString)), this, SLOT(slot_editValidateState(QString)));
	setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint );
}

/**
* \brief AskPatientInfo class destructor
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
AskPatientInfo::~AskPatientInfo()
{
	delete dlg;
}

/**
* \brief Return the values given by the user
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void AskPatientInfo::GetData(QString &name, QString &firstname, QString &sex, QString &birth)
{
	name = dlg->name->text();
	firstname = dlg->firstname->text();
	if( dlg->sexu->isChecked() )
		sex = "U";
	else if( dlg->sexf->isChecked() )
		sex = "F";
	else if( dlg->sexm->isChecked() )
		sex = "M";
	birth = dlg->birth->text();
}

/**
* \brief Allow to disable the button or enable it depending on the informations filled by the user
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void AskPatientInfo::slot_editValidateState(QString text)
{
	if( dlg->firstname->text() != "" && dlg->name->text() != "" ) 
		dlg->validateButton->setEnabled(true);
	else
		dlg->validateButton->setEnabled(false);
}

/**
* \brief If the user doesn't know the patient identity
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void AskPatientInfo::slot_dontKnowClicked()
{
	QMessageBox::StandardButton reply = QMessageBox::question(this, "Are you sure ?", "Are you sure you have no information about the patient (name/firstname at least) ?", QMessageBox::Yes|QMessageBox::No);
	if (reply == QMessageBox::Yes) 
		emit reject();
}
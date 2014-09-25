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

#include "askSomething.h"

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
AskSomething::AskSomething(QString windowsTitle, QString labelText, bool pwType, bool browseNeed, QIcon icon, QWidget *parent) : QDialog(parent)
{
	this->setWindowIcon(icon);
	this->setWindowTitle(windowsTitle);

    m_answer = new QLineEdit(this);
	if( pwType )
		m_answer->setEchoMode(QLineEdit::Password);
	m_answerLbl = new QLabel(this);
	m_answerLbl->setText(labelText);
	connect(m_answer, SIGNAL(textChanged(QString)), this, SLOT(slot_textChanged(QString)));
	
    validateButton = new QPushButton(tr("Validate"));
	validateButton->setDefault( true );
	validateButton->setEnabled( false );

	QSpacerItem *Spacer1 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QSpacerItem *Spacer2 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		
	QPushButton *browseButton;
	if( browseNeed )
	{
		browseButton = new QPushButton();
		browseButton->setGeometry(browseButton->x(), browseButton->y(), 40, 28);
		QPixmap pixmap2(Config::IMAGES_FOLDER + "browse.png");
		QIcon ButtonIcon2(pixmap2);	
		browseButton->setIcon(ButtonIcon2);
		browseButton->setIconSize(QSize(28,28));
		browseButton->setFlat(true);
		connect(browseButton, SIGNAL(clicked()), this, SLOT(slot_browse()));
	}

    connect(validateButton, SIGNAL(clicked()), this, SLOT(accept()));
	
    QVBoxLayout *Vlt = new QVBoxLayout;
    QHBoxLayout *Hlt = new QHBoxLayout;
    QHBoxLayout *HltLbl = new QHBoxLayout;
    Vlt->addWidget(m_answerLbl);
    HltLbl->addWidget(m_answer);
	if( browseNeed )
		HltLbl->addWidget(browseButton);
	Hlt->addItem(Spacer1);
    Hlt->addWidget(validateButton);
    Hlt->addItem(Spacer2);
    Vlt->addLayout(HltLbl);
    Vlt->addLayout(Hlt);

    setLayout(Vlt);
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void AskSomething::slot_textChanged( const QString & text )
{
	if( text == "" )
		validateButton->setEnabled( false );
	else
		validateButton->setEnabled( true );
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
QString AskSomething::getString() const
{
    return m_answer->text();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void AskSomething::slot_browse()
{
	QString fodlerName = QFileDialog::getExistingDirectory();

	if( fodlerName != "" )
		m_answer->setText(fodlerName);
}
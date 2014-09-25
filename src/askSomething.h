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

#ifndef ASKSOMETHING_H
#define ASKSOMETHING_H

//Qt includes
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFileDialog>
//My includes
#include "config.h"

/**
* \class AskSomething
* \brief This class allows us to ask the user to write an answer to a question
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
class AskSomething : public QDialog
{
    Q_OBJECT
public:
    explicit AskSomething(QString windowsTitle, QString labelText, bool pwType, bool browseNeed, QIcon icon, QWidget *parent = 0);

    QString getString() const;

private slots:
	void slot_browse();
	void slot_textChanged( const QString & text );

private:
    QLabel *m_answerLbl;
    QLineEdit *m_answer;
	QPushButton *validateButton;
};

#endif //ASKSOMETHING_H
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

#include "askdialog.h"
#include "config.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

/**
* \brief Constructor of the CommentDialog class. Interface used to post a comment by the user.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
AskDialog::AskDialog(QString dataKind, QString labelStr, QString editStr, QWidget *parent) : QDialog(parent)
{
	this->setWindowTitle(dataKind);
    m_lineLabel = new QLabel(this);
	m_lineLabel->setText(labelStr);
    m_lineEdit = new QPlainTextEdit(this);
	m_lineEdit->textCursor().insertText(editStr);

    QPushButton *searchButton = new QPushButton(tr("Validate"));
    searchButton->setDefault(true);

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(searchButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *lt = new QVBoxLayout;
    lt->addWidget(m_lineLabel);
    lt->addWidget(m_lineEdit);
    lt->addWidget(buttonBox);

    setLayout(lt);
}

/**
* \brief Used to retrieve the user's answer
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
QString AskDialog::askedString() const
{
    return m_lineEdit->toPlainText();
}
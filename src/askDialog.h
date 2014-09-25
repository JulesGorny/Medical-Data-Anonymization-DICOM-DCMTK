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

#ifndef ASKDIALOG_H
#define ASKDIALOG_H

//Qt includes
#include <QDialog>
#include <QCloseEvent>
#include <QPlainTextEdit>
#include <QLabel>

/**
* \class AskDialog
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
class AskDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AskDialog(QString dataKind, QString labelStr, QString editStr, QWidget *parent = 0);

    QString askedString() const;

private:
    QLabel *m_lineLabel;
    QPlainTextEdit *m_lineEdit;
};

#endif // ASKDIALOG_H
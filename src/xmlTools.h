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

/**
* \file xmlTools.h 
* \brief regroup tools to manipulate xml files.
*/

#include <iostream>
//Includes Qt
#include <QXmlSchema>
#include <QFile>
#include <QString>
#include <QXmlSchemaValidator>
#include <QMessageBox>
#include <QtXml>
//My includes
#include "tools.h"
#include "config.h"

#ifndef XMLELEMENT_H
#define XMLELEMENT_H

/**
* \struct xmlElement
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
struct xmlElement
{
	QString name;
	QString path;
	QString value;
	QMap<QString,QString> attributes;
	QList<xmlElement> childs;
};

#endif

void fillDomDocFromXmlFile(QDomDocument &doc, QString xmlPath, Ui::MainWindow *ui);
void fillPatientFromXMLFile(Patient &pat, QString xmlPath, Ui::MainWindow *ui);
void writeXMLfromPatient(QString xmlPath, Patient* pat);
QString getElemValue(QDomDocument& doc, QString elementName);
QString getElemAttributeValue(QDomDocument& doc, QString elementName, QString attrName);
void writeXML(QString filename, QDomDocument doc, int indent);
void writeElement(QDomDocument &doc, QDomElement &addedNode, QString elemName, QString elemValue, QMap<QString, QString> attributes, QString parentsStr, QMap<QString, QMap<QString, QString>> parentsAttr, Ui::MainWindow *ui);
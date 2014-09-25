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
* \file xmlTools.cpp 
* \brief regroup tools to manipulate xml files.
*/

#include "xmlTools.h"

/**
* \brief Fill the QDomDocument with the data of a XML file.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param doc : QDomDocument to fill
* \param xmlPath : path of the file of data
*/
void fillDomDocFromXmlFile(QDomDocument &doc, QString xmlPath, Ui::MainWindow *ui)
{
	doc = QDomDocument();
	QFile xml_doc(xmlPath);

	if( xml_doc.open(QIODevice::ReadOnly) )
	{		
		if ( !doc.setContent(&xml_doc) )
			addProgressMessage(ui,"QDomDocument can't manage to read the XML file.\n", Qt::red, "", true);
	}
	else
		addProgressMessage(ui,"XML file can't be open.\n", Qt::red, "", true);

	xml_doc.close();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void fillPatientFromXMLFile(Patient &pat, QString xmlPath, Ui::MainWindow *ui)
{
	addProgressMessage(ui,"Retrieving data from database...\n", Qt::black, "", false);
	QString tab = "    ";
	QFile f(xmlPath);
	if(!f.open(QIODevice::ReadOnly)) 
		addProgressMessage(ui,"Can't read the existing database.\n", Qt::red, "", true);

	QTextStream in(&f);

	Image* img = NULL;
	Study* study = NULL;
	Series* series = NULL;
	Extract* extr = NULL;
	File* file = NULL;

	while(!in.atEnd()) 
	{
		QString line = in.readLine();
		if( line.endsWith(">") && !line.contains("</") )
		{
			//Image
			if( line.startsWith(tab + "<") )
			{				
				if( line.contains("<Property") )
				{
					QString tag, tagName, value;
					int start = line.indexOf("tag=")+5, end = line.indexOf("\"", start);
					tag = line.mid(start, end-start);
					start = line.indexOf("tagName=")+9; end = line.indexOf("\"", start);
					tagName = line.mid(start, end-start);
					start = line.indexOf("value=")+7; end = line.indexOf("\"", start);
					value = line.mid(start, end-start);
					Property prop(tag, tagName, value);
					pat.addProperty(prop);
				}
				else
				{
					line = line.trimmed();
					int start = line.indexOf("<")+1, end = line.lastIndexOf(">");
					QString imgType = line.mid(start, end-start);
					img = new Image(imgType);
					pat.addImage(img);
				}
			}
			//Study
			else if( line.startsWith(tab + tab + "<") )
			{
				line = line.trimmed();
				study = new Study();
				int start = line.indexOf("\"")+1, end = line.lastIndexOf("\"");
				QString descr = line.mid(start, end-start);
				study->setDescription(descr);
				img->addStudy(study);
			}
			//Series
			else if( line.startsWith(tab + tab + tab + "<") )
			{
				if( line.contains("<Property") )
				{
					QString tag, tagName, value;
					int start = line.indexOf("tag=")+5, end = line.indexOf("\"", start);
					tag = line.mid(start, end-start);
					start = line.indexOf("tagName=")+9; end = line.indexOf("\"", start);
					tagName = line.mid(start, end-start);
					start = line.indexOf("value=")+7; end = line.indexOf("\"", start);
					value = line.mid(start, end-start);
					Property prop(tag, tagName, value);
					study->addProperty(prop);
				}
				else if( line.contains("<Series") )
				{
					line = line.trimmed();
					series = new Series();
					int start = line.indexOf("\"")+1, end = line.lastIndexOf("\"");
					QString descr = line.mid(start, end-start);
					series->setDescription(descr);
					study->addSeries(series);
				}
			}
			//Extract
			else if( line.startsWith(tab + tab + tab + tab + "<") )
			{
				
				if( line.contains("<Property") )
				{
					QString tag, tagName, value;
					int start = line.indexOf("tag=")+5, end = line.indexOf("\"", start);
					tag = line.mid(start, end-start);
					start = line.indexOf("tagName=")+9; end = line.indexOf("\"", start);
					tagName = line.mid(start, end-start);
					start = line.indexOf("value=")+7; end = line.indexOf("\"", start);
					value = line.mid(start, end-start);
					Property prop(tag, tagName, value);
					series->addProperty(prop);
				}
				else if( line.contains("<Extract") )
				{
					line = line.trimmed();
					int start = line.indexOf("\"")+1, end = line.lastIndexOf("\"");
					extr = new Extract(line.mid(start, end-start).toInt());
					series->addExtract(extr);
				}
			}
			//File or Property (from Extract)
			else if( line.startsWith(tab + tab + tab + tab + tab + "<") )
			{
				if( line.contains("<Property") )
				{
					QString tag, tagName, value;
					int start = line.indexOf("tag=")+5, end = line.indexOf("\"", start);
					tag = line.mid(start, end-start);
					start = line.indexOf("tagName=")+9; end = line.indexOf("\"", start);
					tagName = line.mid(start, end-start);
					start = line.indexOf("value=")+7; end = line.indexOf("\"", start);
					value = line.mid(start, end-start);
					Property prop(tag, tagName, value);
					extr->addProperty(prop);
				}
				else if( line.contains("<File") )
				{
					line = line.trimmed();
					int start = line.indexOf("\"")+1, end = line.lastIndexOf("\"");
					QString location = line.mid(start, end-start);
					file = new File(location);
					extr->addFile(file);
				}
			}
			//Property
			else if( line.startsWith(tab + tab + tab + tab + tab + tab + "<") )
			{
				line = line.trimmed();
				QString tag, tagName, value;
				int start = line.indexOf("tag=")+5, end = line.indexOf("\"", start);
				tag = line.mid(start, end-start);
				start = line.indexOf("tagName=")+9; end = line.indexOf("\"", start);
				tagName = line.mid(start, end-start);
				start = line.indexOf("value=")+7; end = line.indexOf("\"", start);
				value = line.mid(start, end-start);
				Property prop(tag, tagName, value);
				file->addProperty(prop);
			}
		}  
	}

	f.close();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void writeXMLfromPatient(QString xmlPath, Patient* pat)
{
	QString tab = "    ";
	QFile file(xmlPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    out << "<Patient Id=\"" << pat->getId() << "\">\n";

	QVector<Property> propertiesPat = pat->getProperties();
	for(int p=0; p<propertiesPat.size(); p++)
	{
		Property prop = propertiesPat[p];
		out << tab << "<Property tag=\"" << prop.getTag() << "\" tagName=\"" << prop.getTagName() << "\" value=\"" << prop.getValue() << "\"/>\n";
	}

	QVector<Image*> imgs = pat->getImgs();
	for(int i=0; i<imgs.size(); i++)
	{
		Image* img = imgs[i];
		out << tab << "<" << img->getType() << ">\n";

		QVector<Study*> studies = img->getStudies();
		for(int st=0; st<studies.size(); st++)
		{
			Study* study = studies[st];
			out << tab << tab << "<Study description=\"" << study->getDescription() << "\">\n";

			QVector<Property> propertiesStud = study->getProperties();
			for(int p=0; p<propertiesStud.size(); p++)
			{
				Property prop = propertiesStud[p];
				out << tab << tab << tab << "<Property tag=\"" << prop.getTag() << "\" tagName=\"" << prop.getTagName() << "\" value=\"" << prop.getValue() << "\"/>\n";
			}

			QVector<Series*> series = study->getSeries();
			for(int se=0; se<series.size(); se++)
			{
				Series* serie = series[se];
				out << tab << tab << tab << "<Series description=\"" << serie->getDescription() << "\">\n";
				
				QVector<Property> propertiesSeries = serie->getProperties();
				for(int p=0; p<propertiesSeries.size(); p++)
				{
					Property prop = propertiesSeries[p];
					out << tab << tab << tab << tab << "<Property tag=\"" << prop.getTag() << "\" tagName=\"" << prop.getTagName() << "\" value=\"" << prop.getValue() << "\"/>\n";
				}

				QVector<Extract*> extracts = serie->getExtracts();
				for(int e=0; e<extracts.size(); e++)
				{
					Extract* extract = extracts[e];
					extract->fillFilesCommonProperty();
					out << tab << tab << tab << tab << "<Extract number=\"" << QString::number(extract->getNumber()) << "\">\n";

					QVector<Property> propertiesExtr = extract->getProperties();
					for(int p=0; p<propertiesExtr.size(); p++)
					{
						Property prop = propertiesExtr[p];
						out << tab << tab << tab << tab << tab << "<Property tag=\"" << prop.getTag() << "\" tagName=\"" << prop.getTagName() << "\" value=\"" << prop.getValue() << "\"/>\n";
					}

					QVector<File*> files = extract->getFiles();
					for(int f=0; f<files.size(); f++)
					{
						File* file = files[f];
						out << tab << tab << tab << tab << tab << "<File location=\"" << file->getPath() << "\">\n";

						QVector<Property> properties = file->getProperties();
						for(int p=0; p<properties.size(); p++)
						{
							Property prop = properties[p];
							out << tab << tab << tab << tab << tab << tab << "<Property tag=\"" << prop.getTag() << "\" tagName=\"" << prop.getTagName() << "\" value=\"" << prop.getValue() << "\"/>\n";
						}
						out << tab << tab << tab << tab << tab << "</File>\n";
					}
					out << tab << tab << tab << tab << "</Extract>\n";
				}
				out << tab << tab << tab << "</Series>\n";
			}
			out << tab << tab << "</Study>\n";
		}
		out << tab << "</" << img->getType() << ">\n";
	}
    out << "</Patient>";

    // optional, as QFile destructor will already do it:
    file.close(); 
}

/**
* \brief This function return the element's value
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param doc : QDomDocument containing the xml tree
* \param elementName : element name
* \return the element value or an empty character
*/
QString getElemValue(QDomDocument& doc, QString elementName)
{	
	QDomNodeList elements = doc.elementsByTagName(elementName);

	//If we find an element with the same name
	if( elements.size() == 1 )
		return elements.at(0).toElement().text();
	return "";
}

/**
* \brief This function return the named attribute's value of a given element
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param doc : QDomDocument containing the xml tree
* \param elementName : element name
* \param attrName : attribute name
* \return the element attribute value or an empty character
*/
QString getElemAttributeValue(QDomDocument& doc, QString elementName, QString attrName)
{	
	QDomNodeList elements = doc.elementsByTagName(elementName);

	//If we find an element with the same name
	if( elements.size() == 1 )
	{
		QDomNamedNodeMap elemAttributes = elements.at(0).attributes();
		for(int j=0; j<elemAttributes.size(); j++)
		{
			QString atName = elemAttributes.item(j).toAttr().name();
			if( attrName == atName )
				return elemAttributes.item(j).toAttr().value();
		}
	}
	return "";
}


/**
* \brief This function save an \b XML \b file from a \b QDomDocument
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param filename : path of the file we will write
* \param XSDpath : path of the XSD file to check the xml validity
* \param doc : our xml tree in a QDomDocument
* \param indent : number of spaces for an indentation
*/
void writeXML(QString filename, QDomDocument doc, int indent)
{
	QFile file(filename);
	file.open(QIODevice::WriteOnly);
	QTextStream ts(&file);
	doc.save(ts, indent);
}

/**
* \brief This function verify if a similar element is not already written and write the element at the good place
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
* \param doc : QDomDocument containing the xml tree
* \param elemName : Name of the XML element
* \param elemValue : Value of the XML element
* \param attributes : QMap<QString, QString> with the attribute's name in key and the atttribute's value in value
* \param parentsStr : A string containing a hierarchy of parents written like .../.../grandFather/father (used to compare the parents of existing elements and the new one)
* \param parentsAttr : QMap<QString, QMap<QString, QString>> containing the parent name in the key and a map build like the attributes param in the value (used to compare the parents of existing elements and the new one)
* \param ui : in case we want to write a message to the user.
**/
void writeElement(QDomDocument &doc, QDomElement &addedNode, QString elemName, QString elemValue, QMap<QString, QString> attributes, QString parentsStr, QMap<QString, QMap<QString, QString>> parentsAttr, Ui::MainWindow *ui)
{
	QStringList parents;
	if(parentsStr != "")
		parents = parentsStr.split("/");

	/**
	* We try to find where we want to include our new node. For that we browse the XML tree keeping a track on the closer parent of our node we found.
	* This closer parent is named "last parent" cause it's the last we found while going deeper in the tree.
	**/
	QDomNodeList lastParentFoundList;
	QDomNode ourActualLastParent;
	//If we have at least one parent, we search the first one (it will be "Patient")
	if( parents.size() > 0 )
	{
		lastParentFoundList = doc.elementsByTagName(parents.at(0));
		ourActualLastParent = lastParentFoundList.at(0);
	}
	//If we don't have parents
	else
	{
		//And if there is no Patient node 
		if( doc.elementsByTagName("Patient").size() == 0 )
		{
			//We create the element at the root (if the program is well written, it's supposed to be a new Patient node)
			addedNode = doc.createElement(elemName);
			if( elemValue != "" )
			{
				QDomText nameText = doc.createTextNode(elemValue);
				addedNode.appendChild(nameText);
			}
			if( attributes.size() > 0 )
			{
				QMap<QString, QString>::Iterator it;
				for( it = attributes.begin() ; it != attributes.end() ; ++it)
					addedNode.setAttribute(it.key(), it.value());
			}
			doc.appendChild(addedNode);
		}
	}

	//The lastParent is now initialized with "Patient", we can browse the rest of the parents tree.
	for( int p=1; p<parents.size(); p++ )
	{
		//We search the list of nodes corresponding to second older parent name (after Patient)
		lastParentFoundList = ourActualLastParent.toElement().elementsByTagName(parents.at(p));
		//If we can't find any parent candidate, there is a problem in the program ...
		if( lastParentFoundList.size() == 0 )
		{
			addProgressMessage(ui, "Looks like the developper didn't do a great job. The parents list doesn't match the XML file we are browsing.", Qt::black, "", false);
		}
		//If there is at least one parent node found with that name, we will compare the attribute
		else
		{
			bool weFoundWhatWeWanted = false;
			//Browse the different parent candidates
			for( int f=0; f<lastParentFoundList.size(); f++ )
			{
				//We estimate that the attributes correspond to what we want
				bool areAllAttributesSimilar = true;
				QDomNamedNodeMap parentsAttributes = lastParentFoundList.at(f).attributes();
				//We browse the attribute to find if one attribute doesn't correspond
				for(int a=0; a<parentsAttributes.size(); a++)
				{
					QString parentAttrName = parentsAttributes.item(a).toAttr().name();
					QString parentAttrValue = parentsAttributes.item(a).toAttr().value();
					QMap<QString, QString> tempAttr = parentsAttr.find(parents.at(p)).value();
					//In that case, we continue to browse the different parent candidates
					if( !tempAttr.contains(parentAttrName) || tempAttr.find(parentAttrName).value() != parentAttrValue)
					{
						areAllAttributesSimilar = false;
						break;
					}
				}
				//If we found the perfect candidate, we keep it and stop brownsing the candidates. We can go deeper in the tree.
				if( areAllAttributesSimilar )
				{
					ourActualLastParent = lastParentFoundList.at(f);
					weFoundWhatWeWanted = true;
					break;
				}
			}
			//If we didn't find any corresponding parent, there is a problem in the program ...
			if( !weFoundWhatWeWanted )
			{
				addProgressMessage(ui, "Looks like the developper didn't do a great job. The parents list doesn't match the XML file we are browsing.", Qt::black, "", false);
			}
		}
	}
	
	//We finally found our closer parent !
	QDomNodeList sameElementFoundList;
	sameElementFoundList = ourActualLastParent.toElement().elementsByTagName(elemName);
	bool isAnElementSimilar = false;
	//We try to find a similar element (Name + attributes)
	for( int f=0; f<sameElementFoundList.size(); f++ )
	{
		bool areAllAttributesSimilar = true;
		QDomNamedNodeMap parentsAttributes = sameElementFoundList.at(f).attributes();
		for(int a=0; a<parentsAttributes.size(); a++)
		{
			QString parentAttrName = parentsAttributes.item(a).toAttr().name();
			QString parentAttrValue = parentsAttributes.item(a).toAttr().value();
			if( !attributes.contains(parentAttrName) || attributes.find(parentAttrName).value() != parentAttrValue)
			{
				areAllAttributesSimilar = false;
				break;
			}
		}
		if( areAllAttributesSimilar )
		{
			isAnElementSimilar = true;
			addedNode = sameElementFoundList.at(f).toElement();
			break;
		}
	}

	//If the element definitly doesn't exists, we create it
	if( !isAnElementSimilar )
	{
		addedNode = doc.createElement(elemName);
		if( elemValue != "" )
		{
			QDomText nameText = doc.createTextNode(elemValue);
			addedNode.appendChild(nameText);
		}
		if( attributes.size() > 0 )
		{
			QMap<QString, QString>::Iterator it;
			for( it = attributes.begin() ; it != attributes.end() ; ++it)
				addedNode.setAttribute(it.key(), it.value());
		}
		ourActualLastParent.toElement().appendChild(addedNode);
	}
}
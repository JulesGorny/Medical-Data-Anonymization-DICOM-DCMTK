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

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QStringList>

/**
* \class Config
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
*/
class Config
{
    public:
		// Slashs have to be like / in that attributes.
		// An attribute will never end with a /.
		
		//Folder of the project (where we can find src, build ...)
		static QString PROJECT_FOLDER;
		//Path of the folder where we will put the folder for anonymized data of the patient (the content of this folder is used by the anonymizer
		//to know the previous anonymizations run about the patients). It doesn't contain any image.
		static QString ANONYMIZER_FOLDER;
		//Path of the folder where we put the file doing the link between patient's Id and privates datas (name, age ...)
        static QString PATIENT_PV_PATH;
		//Path of the folder where we can find the non-anonymous additionnal data (example: informed conscent form)
		static QString NON_ANONYMOUS_DOC_PATH;
		//Path of the folder where we can find the paths of the interesting data for each file
		static QString INTERESTING_FILES_PATH;
		//Default value of the destination path
		static QString DEFAULT_DESTINATION_PATH; //Slash have to be like that \ for this one
		//Path of the file for interesting data we can find in XML files
		static QString INTERESTING_DATA_XML;
		//Interesting modalities
		static QStringList INTERESTING_MODALITIES;
		//Images types
		static QStringList IMAGE_TYPES;
		//Path for the images
		static QString IMAGES_FOLDER;
		//Path for the commonDBFile
		static QString COMMON_DB_FILENAME;
		static QString COMMON_DB_WITH_VIEWER;
};

#endif
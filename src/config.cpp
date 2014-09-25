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

#include "config.h"

//Folder of the project (where we can find src, build ...)
//Has to be ".." when you work in visual studio
//Has to be "." when you create an installer
QString Config::PROJECT_FOLDER = ".";
//Folder where we will put the folder for anonymized data of the patient (the content of this folder is used by the anonymizer
//to know the previous anonymizations run about the patients). It doesn't contain any image.
QString Config::ANONYMIZER_FOLDER = Config::PROJECT_FOLDER + "\\MyBDDFolder";
//Folder where we put the file doing the link between patient's Id and privates datas (name, age ...)
QString Config::PATIENT_PV_PATH = Config::ANONYMIZER_FOLDER + "\\Private\\PrivateResultData";
//Folder where we can find the non-anonymous additionnal data (example: informed conscent form)
QString Config::NON_ANONYMOUS_DOC_PATH = Config::ANONYMIZER_FOLDER + "\\Private\\NonAnonymousDocs";
//Folder where we can find the paths of the interesting data for each file
QString Config::INTERESTING_FILES_PATH = PROJECT_FOLDER + "\\interestingData";
//Default value of the destination path
QString Config::DEFAULT_DESTINATION_PATH;
//Path of the file for interesting data we can find in XML files
QString Config::INTERESTING_DATA_XML = Config::INTERESTING_FILES_PATH + "\\interestingXmlElements.txt";
//Interesting modalities
QStringList Config::INTERESTING_MODALITIES = (QStringList() << "MR" << "US" << "CT");
//Order of the images in the list contained in the patient class
QStringList Config::IMAGE_TYPES = (QStringList() << "MR" << "US" << "Laparoscopy" << "CT");
//Path for the images
QString Config::IMAGES_FOLDER = PROJECT_FOLDER + "\\src\\imgs\\";
//Path for the commonDBFile
QString Config::COMMON_DB_FILENAME = "commonFolderWithViewer.txt";
QString Config::COMMON_DB_WITH_VIEWER = PROJECT_FOLDER + "\\commonDB\\" + Config::COMMON_DB_FILENAME;
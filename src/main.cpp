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

//Qt includes
#include <QApplication>
#include <QMessageBox>
//My includes
#include "mainwindow.h"
#include "tools.h"
#include "askSomething.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	QFile dataBase(Config::COMMON_DB_WITH_VIEWER);
	//Check if the file containing the common folder for the Anonymizer and the Viewer is filled
	//If it's not, we ask the user if he wants to fill it
	if( !dataBase.exists() )
	{
		QString db = Config::COMMON_DB_WITH_VIEWER;
		QString folder = db.left(db.size() - Config::COMMON_DB_FILENAME.size() - 1);
		QDir().mkpath(folder);
	}

	QFile tagsFile(Config::COMMON_DB_WITH_VIEWER);
	if (tagsFile.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		//Open the file and look if we find a path for the folder
		QTextStream adm(&tagsFile);
		while (!adm.atEnd()) 
		{
			QString line = adm.readLine();
			if(line != "")
			{
				Config::DEFAULT_DESTINATION_PATH = line;
				break;
			}
		}

		//If the file was empty, we ask the user
		if( Config::DEFAULT_DESTINATION_PATH == "" )
		{
			AskSomething* commonBddFolderWithViewer = new AskSomething("Common folder", "Please choose a default database for the anonymized files.", false, true, QIcon(Config::IMAGES_FOLDER + "exchange.png"));
			if( commonBddFolderWithViewer->exec() )
			{
				Config::DEFAULT_DESTINATION_PATH = commonBddFolderWithViewer->getString();
				QTextStream in(&tagsFile);
				in << Config::DEFAULT_DESTINATION_PATH;
			}
		}
	}
	else
	{
		QMessageBox::warning(0, "Error", "It looks like you don't have the authorization where you installed the Anonymizer. Please uninstall the software an reinstall it somewhere else.\n\n");
		return 0;
	}
	QDir().mkpath(Config::PATIENT_PV_PATH);
	QDir().mkpath(Config::NON_ANONYMOUS_DOC_PATH);
		
	//Main window display
    MainWindow w;	
    w.show();

	return a.exec();
}

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
* \file tools.h 
* \brief regroup tools to manipulate xml files.
*/

#include <iostream>
//Includes Qt
#include <QFile>
#include <QString>
#include <QImage>
#include <QDir>
#include "mainwindow.h"
#include "ui_mainwindow.h"
//OpenCV
#include <ole2.h>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

void addProgressMessage(Ui::MainWindow *ui, QString message, Qt::GlobalColor color, QString errorName, bool isDetailled);
bool copyFile(QString beforePath, QString afterPath, bool useProgressBar, Ui::MainWindow *ui);
QImage* Mat2QImage(cv::Mat src);
bool rmNonEmptyDir(QString _name);
QString connection();
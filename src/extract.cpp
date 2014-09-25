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

#include "extract.h"

Extract::Extract() {}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Extract::Extract(int nb)
{
	_number = nb;
}
	
/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
Extract::~Extract()
{
	qDeleteAll(_files.begin(), _files.end());
	/*for(int f=0; f<_files.size(); f++)
		delete _files[f];*/
	_files.clear();
}

/**
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
File* Extract::addFile(File* f) 
{
	for(int fi=0; fi<_files.size(); fi++)
	{
		if( _files[fi]->getPath() == f->getPath() )
		{
			delete f;
			return _files[fi];
		}
	}
	_files.append(f);
	return f;
}

/**
* \brief This function browse the files in the extract and look for properties in common for all the files (same tag and value). 
* If we find such properties, we remove them from the files and insert them in the extract properties.
* \author Jules Gorny - ALCoV team, ISIT, UMR 6284 UdA – CNRS
**/
void Extract::fillFilesCommonProperty()
{
	if( _files.size() > 0 )
	{
		File* fi = _files[0];
		QVector<Property> props = fi->getProperties();
		for(int p=0; p<props.size(); p++)
		{
			bool thisPropIsTheSameInEveryChild = true;
			for(int f=1; f<_files.size(); f++)
			{
				if( !_files[f]->getProperties().contains(props[p]) )
				{
					thisPropIsTheSameInEveryChild = false;
					break;
				}
			}

			if( thisPropIsTheSameInEveryChild )
			{
				_properties.append(props[p]);

				for(int f=_files.size()-1; f>=0; f--)
				{
					QVector<Property> props2 = _files[f]->getProperties();
					for(int p2=0; p2<props2.size(); p2++)
					{
						if( props2[p2] == props[p] )
						{
							_files[f]->removeProp(p2);
							break;
						}
					}
				}
			}
		}
	}
}
/***************************************************************************
 *   Copyright (C) 2005 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo AT gmail DOT com                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "mimeicon.h"
#include <QVectorIterator>

MimeIcon::MimeIcon () {
	MimeAssoc mime;

	mime.pattern << "avi" << "ogm" << "mpg" << "mpeg" << "mov" << "mkv" << "asf" << "rm" << "wmv";
	mime.icon = "video";
	types << mime;

	mime.pattern << "mp3" << "wav" << "ogg" << "wma";
	mime.icon = "audio";
	types << mime;

	mime.pattern << "jpg" << "jpeg" << "bmp" << "png" << "xpm" << "gif" << "tif";
	mime.icon = "image";
	types << mime;

	mime.pattern << "tar" << "gz" << "bz2" << "zip" << "rar" << "ace";
	mime.icon = "archive";
	types << mime;
}

QString MimeIcon::getIcon(QString filename) {
	QString ext = filename.mid( filename.lastIndexOf('.')+1 );
	MimeAssoc mime;

	QVectorIterator<MimeAssoc> i(types);
	while (i.hasNext()) {
		mime = i.next();
		if ( mime.pattern.contains(ext,Qt::CaseInsensitive) )
			return mime.icon;
	}

	return "unknown";
}

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

#include <QtGui>

#ifdef Q_WS_WIN
//#define _UNICODE
#include <wchar.h>
#include <windows.h>
#endif

#include "newdisk.h"

NewDisk::NewDisk(QWidget *parent, QString mountPoint) : QDialog( parent ) {
	setupUi(this);
	txtMountPoint->setText( mountPoint );
	txtDiskName->setText( getVolName(mountPoint) );
#ifdef Q_WS_X11
	lblMountPoint->setText( tr("CD device") );
	cmdMountPoint->hide();
#else
	connect( cmdMountPoint, SIGNAL(clicked()), this, SLOT(browseMountPoint()) );
#endif
}

void NewDisk::browseMountPoint() {
	QFileDialog *fd = new QFileDialog(this);
	fd->setFileMode(QFileDialog::DirectoryOnly);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	fd->setDirectory( txtMountPoint->text() );
	if (fd->exec())
		txtMountPoint->setText( fd->selectedFiles()[0] );
}

QString NewDisk::getDiskName() {
	return txtDiskName->text();
}

QString NewDisk::getMountPoint() {
	return txtMountPoint->text();
}

QString NewDisk::getVolName(QString dev) {
#if defined Q_WS_X11
	QString volname;
	QFile *fd = new QFile(dev);

	if (fd->open(QIODevice::ReadOnly) && fd->seek(32808)) {
		volname = QString(fd->read(32));
	}

	delete fd;
	return volname.trimmed();
#elif defined Q_WS_WIN
	QString volumeName;
	DWORD fileSystemFlags;
	const DWORD MAX_BUFFER = MAX_PATH + 1;
	TCHAR *volumeNameBuffer = new TCHAR[MAX_BUFFER];

	if (GetVolumeInformation(reinterpret_cast<const TCHAR *>(dev.utf16()),
		volumeNameBuffer, MAX_BUFFER, NULL, NULL, &fileSystemFlags, NULL, 0))
		volumeName = QString::fromUtf16(reinterpret_cast<const ushort *>
			(volumeNameBuffer));

	delete volumeNameBuffer;
	return volumeName;
#else
	return "";
#endif
}

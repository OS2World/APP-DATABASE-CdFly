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

#ifndef NEWDISK_H
#define NEWDISK_H

#include "ui_newdiskwidget.h"
#include <QApplication>
#include <QtGui>

class NewDisk : public QDialog, private Ui::NewDiskWidget {
		Q_OBJECT
	private:
		QString getVolName(QString dev);

	public:
		NewDisk(QWidget *parent, QString mountPoint);
		QString getDiskName();
		QString getMountPoint();

	public slots:
		virtual void browseMountPoint();
};

#endif

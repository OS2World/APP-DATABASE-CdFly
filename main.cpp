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

#include <QApplication>
#include "cdfly.h"
#include "cdsql.h"

int main(int argc, char **argv) {
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("Ryo Software");
	QCoreApplication::setOrganizationDomain("cdfly.sourceforge.net");
	QCoreApplication::setApplicationName("CdFly");

	bool add = false, verbose = false;
	QString label = "";
	QString dev = "";
	QStringList args;
	for (int x=1; x<argc-1; x++) {
		args << argv[x];
		if ( QString("--add").compare(argv[x])==0 )
			add = true;
		else if ( QString("--label").compare(argv[x])==0 && x<argc-2 )
			label = argv[x+1];
		else if ( QString("--dev").compare(argv[x])==0 && x<argc-2 )
			dev = argv[x+1];
		else if ( QString("-v").compare(argv[x])==0 || QString("--verbose").compare(argv[x])==0 )
			verbose = true;
	}

	CDFly mainWin;

	if (add) {
		if ( mainWin.sql->db.isOpen() ) {
			if (dev.isEmpty()) dev = mainWin.conf.mountpoint;
			//if (label.isEmpty()) ;

	#ifdef Q_WS_X11
			bool mountedByCdfly = false;
			if ( !mainWin.isMounted(dev) ) {
				if ( mainWin.mountDev(dev)==0 )
					mountedByCdfly = true;
				else if (verbose)
					qDebug("Failed to mount device");
			}
			dev = mainWin.devToMountpoint(dev);
	#else
			dev = dev;
	#endif

			if ( !label.isEmpty() && !dev.isEmpty() ) {
				if (mainWin.sql->scanDisk( dev, label )) {
					qDebug("Disk added");
				} else {
					if (verbose) qDebug("Failed to add disk");
					return -1;
				}
			}

	#ifdef Q_WS_X11
			if (mountedByCdfly) mainWin.umountDev(dev);
	#endif
		} else if (verbose) qDebug("No database open");

		return 0;
	}

	mainWin.show();

	return app.exec();
}

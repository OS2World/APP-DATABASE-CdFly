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
#include <QtSql>
#include <QRegExp>

#include "find.h"
#include "cdfly.h"
#include "cdsql.h"
#include "qsqlitems.h"
#include "mimeicon.h"

Find::Find(CDFly *parent) : QDialog( parent ) {
	setupUi(this);
	cdfly = parent;
	sql = parent->sql;
	clearTable();
	connect(btnSearch, SIGNAL(clicked()), this, SLOT(startSearch()));
	connect(btnStop, SIGNAL(clicked()), this, SLOT(stopSearch()));
	connect(listResults, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
}

void Find::clearTable() {
	listResults->setRowCount(0);
	QStringList th;
	th << tr("Filename");
	listResults->setHorizontalHeaderLabels( th );
	listResults->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
}

void Find::startSearch() {
	clearTable();
	stop = false;
	btnStop->setEnabled( true );
	progress->setEnabled( true );
	btnSearch->setEnabled( false );

	QString icon;
	MimeIcon mime;
	int pos = 0;

	if ( chkRegexp->isChecked() ) {
		progress->setMaximum( sql->getCount() );
		cdfly->showProgress( true, progress->maximum() );

		QSqlQuery result( "SELECT * FROM nodes" );
		while (result.next() && !stop) {
			NodeRecord nr = sql->recordToNodeRecord( result.record() );
			pos++;

			progress->setValue(pos);
			cdfly->setProgress(pos);

			QRegExp reg( txtRegexp->text() );
			int ind = reg.indexIn(nr.name);
			if ( ind>=0 ) {
				int row = listResults->rowCount();
				listResults->setRowCount(row+1);
				QSqlTableItem *nameItem = new QSqlTableItem( nr.name );
				nameItem->setToolTip( QString(nr.name.left(ind) + "<b>" + nr.name.mid(ind,reg.matchedLength()) + "</b>" + nr.name.right(nr.name.length()-ind-reg.matchedLength())) );
				nameItem->id = nr.id;
				if (nr.type==1) {
					icon = "folder";
				} else {
					icon = mime.getIcon( nr.name );
				}
				nameItem->setIcon( QIcon(":/icons/"+icon+".png") );
				listResults->setItem(row, 0, nameItem);
				listResults->verticalHeader()->resizeSection(row, 22);
			}

			qApp->processEvents();
		}
		result.clear();
	} else {
		QString s = txtRegexp->text();
		s.replace("'","''");
		int count = sql->getCount("name LIKE '%"+s+"%'");

		QSqlQuery result;
		result.prepare("SELECT * FROM nodes WHERE name LIKE :search");
		result.bindValue(":search","%"+txtRegexp->text()+"%");
		result.exec();

		progress->setMaximum( count );
		cdfly->showProgress( true, progress->maximum() );

		for (int x=0; result.next() && !stop; x++) {
			NodeRecord nr = sql->recordToNodeRecord(result.record());

			progress->setValue(x);
			cdfly->setProgress(x);

			int row = listResults->rowCount();
			listResults->setRowCount(row+1);

			QSqlTableItem *nameItem = new QSqlTableItem(nr.name);
			nameItem->id = nr.id;
			if (nr.type==1) {
				icon = "folder";
			} else {
				icon = mime.getIcon( nr.name );
			}
			nameItem->setIcon( QIcon(":/icons/"+icon+".png") );
			listResults->setItem(row, 0, nameItem);
			listResults->verticalHeader()->resizeSection(row, 25);

			qApp->processEvents();
		}
		result.clear();
	}

	cdfly->showProgress(false);
	btnStop->setEnabled( false );
	progress->setEnabled( false );
	progress->setValue(0);
	btnSearch->setEnabled( true );

	if (isHidden())
		QMessageBox::information(this, QCoreApplication::applicationName(), tr("Search finished") + "\n" + QString::number(listResults->rowCount()) + " " + tr("results found."));
}

void Find::stopSearch( ) {
	stop = true;
}

void Find::itemClicked(QTableWidgetItem* i) {
	if (i!=NULL) {
		cdfly->setPath( ((QSqlTableItem*)i)->id );
	}
}

//
// C++ Interface: qsqltreeitem
//
// Description:
//
//
// Author: Massimiliano Torromeo <massimiliano.torromeo AT gmail DOT com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QSQLITEMS_H
#define QSQLITEMS_H

#include <QListWidgetItem>
/**
By overloading QListWidgetItem this class adds a field which represents the id contained in the sqlite db for that item

@author Ryo Saeba
*/
class QSqlListItem : public QListWidgetItem {
public:
	int id;
	QSqlListItem( QListWidget* l ) : QListWidgetItem(l) {}
};

#include <QTableWidgetItem>
/**
By overloading QTableWidgetItem this class adds a field which represents the id contained in the sqlite db for that item

@author Ryo Saeba
*/
class QSqlTableItem : public QTableWidgetItem {
public:
	QSqlTableItem( QString s ) : QTableWidgetItem(s) {}
	int id;
};

#endif

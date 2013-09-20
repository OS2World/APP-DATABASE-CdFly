//
// C++ Interface: cdsql
//
// Description:
//
//
// Author: Massimiliano Torromeo <massimiliano.torromeo AT gmail DOT com>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CDSQL_H
#define CDSQL_H

#include <QVector>
#include <QString>
#include <QSqlDatabase>

class ProgressDialog;
class QStringList;
class QSqlQuery;
class QSqlRecord;
class QVariant;
struct configuration;

struct NodeRecord {
	int id;
	QString name;
	int parent;
	int type;
	int size;
	bool hasThumb;
	QString date;
};

/**
This class is meant to manage the db and provides various functions to ease the interactions with the collection

@author Ryo Saeba
*/
class CdSql{
private:
	configuration *conf;
	QString aname;
	bool checkQuery(QSqlQuery q);
	void scanPath( ProgressDialog *prog, QString path, int parent );
	int filesCount( QString path );

public:
	CdSql( configuration *conf );
	~CdSql();

	QSqlDatabase db;

	QString name() { return aname; }
	bool setName( QString name );

	int scanDisk( const QString diskPath, const QString diskName );

	QVariant valueOf(QSqlRecord rec, QString field);
	NodeRecord recordToNodeRecord( QSqlRecord r );

	QVector<NodeRecord> getAll(QString where = "");
	NodeRecord getNode(const int node);
	QVector<NodeRecord> getNodes(const int parent, QString where = "");
	QVector<NodeRecord> getNodesWithMeta(const int parent, QString where = "");
	QString getNodePath(int node);
	QVector<NodeRecord> getPathNodes(int node);

	int getCount(QString where = "");
	int getCountRecursive(int);
	void rename(int,QString);
	void deleteNodes( const QVector<NodeRecord> list, ProgressDialog *prog );
};

#endif

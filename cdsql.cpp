//
// C++ Implementation: cdsql
//
// Description:
//
//
// Author: Massimiliano Torromeo <massimiliano.torromeo AT gmail DOT com>, (C) 2005
//
// Copyright: See LICENSE file that comes with this distribution
//
//
#include "cdsql.h"
#include <QApplication>
#include <QMessageBox>
#include <QtSql>

#include "cdfly.h"
#include "progress.h"

CdSql::CdSql( configuration *conf ) {
	this->conf = conf;
	db = QSqlDatabase::addDatabase("QSQLITE");
	if (!db.isValid())
		QMessageBox::critical( 0, QCoreApplication::applicationName(), QCoreApplication::tr("The database is not valid!") );
}


CdSql::~CdSql() {
	if (db.isOpen()) db.close();
}


bool CdSql::setName( QString name ) {
	if (db.isOpen()) db.close();

	db.setDatabaseName( name );
	db.open() ? this->aname = name : this->aname = "";

	if (this->aname.isEmpty()) return false;

	QString thumbTable("CREATE TABLE thumbnails (id INTEGER PRIMARY KEY, data BLOB NOT NULL)");

	if ( db.tables().count()==0 ) {
		db.exec( thumbTable );
		return db.exec("CREATE TABLE nodes (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, parent INTEGER DEFAULT -1, type INTEGER, size INTEGER, date TEXT DEFAULT '1900-01-01 00:00:00' )").lastError().type() == QSqlError::NoError;
	}

	if (!db.tables().contains("thumbnails"))
		db.exec( thumbTable );

	return true;
}


void CdSql::scanPath( ProgressDialog *prog, QString path, int parent ) {
	QDir* dir = new QDir( path );

	//aggiungo se mancante lo slash finale
	if ( !path.endsWith('/') )
		path = path.append('/');
	prog->setPath( path );

	//includo file e cartelle ma non symlinks
	dir->setFilter( QDir::Files | QDir::Dirs | QDir::NoSymLinks );

	//thumbnails operations
	QStringList thumbext;
	thumbext << "png" << "jpg" << "jpeg" << "bmp";
	int thumbnum = 0;

	QStringList lst = dir->entryList();
	QFileInfo* fi;

	for ( QStringList::Iterator it = lst.begin(); it != lst.end() && !prog->isCanceled(); ++it ) {
		fi = new QFileInfo( path + *it );
		//non li considero altrimenti loop-infinito
		if ((fi->fileName()!=".") && (fi->fileName()!="..")) {
			prog->incProgress();
			prog->setFile( fi->fileName() );

			QSqlQuery query;
			query.prepare("INSERT INTO nodes (name,parent,type,size,date) VALUES (:name,:parent,:type,:size,:date)");
			query.bindValue(":name", fi->fileName());
			query.bindValue(":parent", parent);
			query.bindValue(":type", fi->isDir() ? "1" : "2");
			query.bindValue(":size", fi->size());
			query.bindValue(":date", fi->created().toString("yyyy-MM-dd hh:mm:ss"));
			query.exec();
			int lastid = query.lastInsertId().toInt();
			query.clear();

			if ( conf->thenabled && (conf->thperdir>thumbnum || conf->thperdir==0) && fi->isFile() && thumbext.contains(fi->suffix(), Qt::CaseInsensitive) ) {
				QImage im( fi->filePath() );
				im = im.scaled(120,120,Qt::KeepAspectRatio);
				QByteArray ba;
				QBuffer buffer(&ba);
				buffer.open(QIODevice::WriteOnly);
				im.save(&buffer, "PNG");

				query.prepare("INSERT INTO thumbnails (id,data) VALUES (:id,:data)");
				query.bindValue(":id", lastid);
				query.bindValue(":data", ba);
				query.exec();
				query.clear();

				thumbnum++;
			}

			QCoreApplication::processEvents();

			if ( checkQuery(query) && fi->isDir() && !prog->isCanceled() ) {
				scanPath(prog, path + fi->fileName(), lastid);
			}
		}
	}
}


int CdSql::filesCount( QString path ) {
	QDir* dir = new QDir( path );

	//aggiungo se mancante lo slash finale
	if ( !path.endsWith('/') )
		path = path.append('/');

	//includo file e cartelle ma non symlinks
	dir->setFilter( QDir::Files | QDir::Dirs | QDir::NoSymLinks );

	QStringList lst = dir->entryList();
	QFileInfo* fi;
	int count = 0;

	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
		fi = new QFileInfo( path + *it );
		//non li considero altrimenti loop-infinito
		if ((fi->fileName()!=".") && (fi->fileName()!="..")) {
			count++;
			if ( fi->isDir() )
				count += filesCount(path + fi->fileName());
		}
	}

	return count;
}


int CdSql::scanDisk( const QString diskPath, const QString diskName ) {
	QSqlQuery query;
	query.prepare("INSERT INTO nodes (name,type,date) VALUES (:name,:type,:date)");
	query.bindValue(":name", diskName);
	query.bindValue(":type", 0);
	query.bindValue(":date", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
	query.exec();

	if (checkQuery(query)) {
		int cdnumber = query.lastInsertId().toInt();
		query.clear();

		int c = filesCount( diskPath );
		ProgressDialog *prog = new ProgressDialog(0, c);
		prog->show();
		scanPath( prog, diskPath, cdnumber );
		prog->close();
		return cdnumber;
	}

	query.clear();
	return -1;
}


QVector<NodeRecord> CdSql::getNodes( const int parent, QString where ) {
	if (!where.isEmpty())
		where.prepend(" AND ");
	where.prepend("parent=" + QString::number(parent));
	return getAll(where);
}


QVector<NodeRecord> CdSql::getNodesWithMeta( const int parent, QString where ) {
	if (!where.isEmpty())
		where.prepend(" AND ");
	where.prepend("parent=" + QString::number(parent));


	QVector<NodeRecord> list;
	QSqlQuery result( "SELECT n.*, COUNT(t.id) as thumb FROM nodes n LEFT JOIN thumbnails t USING(id) WHERE " + where + " GROUP BY n.id ORDER BY type DESC,name DESC" );

	if ( checkQuery(result) )
		while ( result.next() )
			list.prepend( recordToNodeRecord(result.record()) );

	result.clear();
	return list;
}


bool CdSql::checkQuery( QSqlQuery q ) {
	if ( q.lastError().type() != QSqlError::NoError ) {
		QMessageBox::critical( 0, QCoreApplication::applicationName(), "SQL: " + q.lastQuery() + "\nError: " + q.lastError().text() );
		return false;
	}

	return true;
}


NodeRecord CdSql::getNode( const int node ) {
	QSqlQuery result;
	result.prepare("SELECT * FROM nodes WHERE id=:id");
	result.bindValue(":id", node );
	result.exec();

	NodeRecord nr;
	if ( checkQuery(result) ) {
		result.next();
		nr = recordToNodeRecord( result.record() );
	}
	result.clear();

	return nr;
}


QVariant CdSql::valueOf( QSqlRecord rec, QString field ) {
	int i = rec.indexOf(field);
	return rec.value(i);
}


QString CdSql::getNodePath( int node ) {
	QString path;
	while (node>=0) {
		NodeRecord nr = getNode( node );
		path.prepend( "/" + nr.name );
		node = nr.parent;
	}
	return path;
}

QVector<NodeRecord> CdSql::getPathNodes( int node ) {
	QVector<NodeRecord> list;
	QVector<int> previous;

	while (node>=0) {
		if ( previous.contains(node) ) {
			QMessageBox::critical( 0, QCoreApplication::applicationName(), QCoreApplication::tr("The database is not consistent.") );
			list.clear();
			return list;
		}

		NodeRecord nr = getNode( node );
		list.prepend( nr );
		previous << node;
		node = nr.parent;
	}

	return list;
}


int CdSql::getCount( QString where ) {
	if (!where.isEmpty())
		where.prepend(" WHERE ");
	QSqlQuery result( "SELECT COUNT(*) FROM nodes" + where );
	if ( checkQuery(result) ) {
		result.next();
		int count = result.record().value(0).toInt();
		result.clear();
		return count;
	}
	result.clear();
	return -1;
}


int CdSql::getCountRecursive( int id ) {
	QVector<NodeRecord> records = getNodes(id);
	int count = records.count();

	QVectorIterator<NodeRecord> i(records);
	while (i.hasNext()) {
		NodeRecord nr = i.next();
		count += getCountRecursive( nr.id );
	}

	return count;
}

void CdSql::rename(int id, QString newname) {
	if (!newname.isEmpty()) {
		QSqlQuery q;
		q.prepare("UPDATE nodes SET name=:newname WHERE id=:itemid");
		q.bindValue(":newname",newname);
		q.bindValue(":itemid",id);
		q.exec();
		checkQuery(q);
		q.clear();
	}
}


void CdSql::deleteNodes( const QVector<NodeRecord> list, ProgressDialog *prog ) {
	QCoreApplication::processEvents();

	QSqlQuery q;

	//cycle through elements
	QVectorIterator<NodeRecord> i(list);
	while (i.hasNext()) {
		NodeRecord nr = i.next();

		//delete node element
		q.prepare("DELETE FROM nodes WHERE id=:id");
		q.bindValue(":id", nr.id);
		q.exec();
		if (q.lastError().type() != QSqlError::NoError)
			qDebug() << q.lastError().text();
		q.clear();

		//delete node thumbnail
		q.prepare("DELETE FROM thumbnails WHERE id=:id");
		q.bindValue(":id", nr.id);
		q.exec();
		if (q.lastError().type() != QSqlError::NoError)
			qDebug() << q.lastError().text();
		q.clear();

		prog->incProgress();

		if (nr.type<=1)
			deleteNodes( getNodes(nr.id), prog );
	}
}


QVector<NodeRecord> CdSql::getAll( QString where ) {
	QVector<NodeRecord> list;

	if (!where.isEmpty())
		where.prepend(" WHERE ");
	QSqlQuery result( "SELECT * FROM nodes" + where + " ORDER BY type DESC,name DESC" );

	if ( checkQuery(result) )
		while ( result.next() )
			list.prepend( recordToNodeRecord(result.record()) );

	result.clear();
	return list;
}

NodeRecord CdSql::recordToNodeRecord( QSqlRecord r ) {
	NodeRecord nr;
	nr.id = r.value(0).toInt();
	nr.name = r.value(1).toString();
	nr.parent = r.value(2).toInt();
	nr.type = r.value(3).toInt();
	nr.size = r.value(4).toInt();
	nr.date = r.value(5).toString();
	nr.hasThumb = r.value(6).toInt()>0;
	return nr;
}

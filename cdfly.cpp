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

#include "cdfly.h"
#include "settings.h"
#include "find.h"
#include "newdisk.h"
#include "mimeicon.h"
#include "cdsql.h"
#include "qsqlitems.h"
#include "progress.h"

#include "ui_aboutwidget.h"

/**
 * Creation of the form:
 * - load settings
 * - start translator
 * - setupUI for interface
 * - create actions, menus and toolbars
 * - dirList defaults
 * - create sql connection (not opened yet)
 * - connect() all signals
 * - switch what to do on startup and eventually open a collection
 * @param parent
 * @return
 */
CDFly::CDFly(QWidget *parent) : QMainWindow( parent ) {
	QSettings settings;

	settings.beginGroup("application");
	conf.mountpoint = settings.value("mountpoint","").toString();
	conf.startup = settings.value("startup",0).toInt();
	conf.startkat = settings.value("startkat","").toString();
	conf.lastkat = settings.value("lastkat","").toString();
	conf.locale = settings.value("locale",QLocale::system().name()).toString();
	settings.endGroup();

	settings.beginGroup("thumbnails");
	conf.thperdir = settings.value("perdir",3).toInt();
	conf.thenabled = settings.value("enabled",true).toBool();
	conf.thsize = settings.value("size",120).toInt();
	settings.endGroup();

	{ langinfo l = {"en_EN","English"};
		languages << l; }
	{ langinfo l = {"fr_FR",QString::fromUtf8("Français")};
		languages << l; }
	{ langinfo l = {"he_IL",QString::fromUtf8("עברית")};
		languages << l; }
	{ langinfo l = {"it_IT","Italiano"};
		languages << l; }
	{ langinfo l = {"pl_PL","Polski"};
		languages << l; }
	{ langinfo l = {"ru_RU",QString::fromUtf8("Pусский")};
		languages << l; }
	{ langinfo l = {"tr_TR",QString::fromUtf8("Türkçe")};
		languages << l; }

	//Localisations
	translator = new QTranslator();
	QString translatorFile = QApplication::applicationDirPath() + "/cdfly_"+conf.locale;
	if (QFile::exists(translatorFile+".qm"))
		translator->load( translatorFile );
#ifndef Q_WS_WIN
	else
		translator->load( "/usr/share/cdfly/cdfly_"+conf.locale );
#endif
	QApplication::installTranslator(translator);

	setupWidgets();

	resize( settings.value("window/size", QSize(600, 400)).toSize() );
	restoreState( settings.value("window/state").toByteArray() );
	dirList->horizontalHeader()->resizeSection(0, settings.value("tableview/name", 300).toInt() );

	settings.deleteLater();

	sql = new CdSql( &conf );

	if ( QApplication::argc()>0 && QFile::exists(QApplication::argv()[QApplication::argc()]) ) {

		setFilename( QApplication::argv()[1] );
		open();

	} else {

		switch( conf.startup ) {
			case 0:
				if (!conf.lastkat.isEmpty() && QFile::exists(conf.lastkat)) {
					setFilename( conf.lastkat );
					open();
				}
			break;
			case 1:
				if (!conf.startkat.isEmpty() && QFile::exists(conf.startkat)) {
					setFilename( conf.startkat );
					open();
				}
			break;
		}

	}
}

/**
 * On form destruction, destroy formFind if not NULL
 * @return
 */
CDFly::~CDFly() {
	delete sql;
	if (formFind!=NULL)
		delete formFind;
}

/**
 * Executes last operations before closing application:
 * - if a search is in progress, stop it
 * - close the db
 * - save settings
 * @param
 */
void CDFly::closeEvent(QCloseEvent *) {
	if (formFind!=NULL)
		formFind->stopSearch();
	sql->db.close();

	QSettings settings;
	settings.setValue("window/size",size());
	settings.setValue("tableview/name", dirList->horizontalHeader()->sectionSize(0));
	settings.setValue("window/state", saveState());
	settings.deleteLater();
}

void CDFly::setupWidgets() {
	//PROGRESS STATUS
	progress = new QProgressBar( statusBar() );
	progress->setMaximumSize( QSize(200,24) );
	progress->setVisible( false );
	statusBar()->addPermanentWidget( progress );

	btnStop = new QToolButton( statusBar() );
	btnStop->setVisible( false );
	btnStop->setIcon( QIcon(":/icons/stop.png") );
	statusBar()->addPermanentWidget( btnStop );
	connect( btnStop, SIGNAL( clicked() ), this, SLOT( stopClicked() ) );

	//CD LIST DOCK
	cdDock = new QDockWidget(tr("CD List"),this);
	cdDock->setObjectName("cdDock");
	cdlist = new QListWidget(cdDock);
	cdlist->setEditTriggers( QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked );
	connect( cdlist, SIGNAL( itemChanged(QListWidgetItem*) ), this, SLOT( cdItemChanged(QListWidgetItem*) ) );
	connect( cdlist, SIGNAL( itemClicked(QListWidgetItem*) ), this, SLOT( cdlistClicked(QListWidgetItem*) ) );
	cdDock->setWidget(cdlist);
	cdDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	addDockWidget( Qt::LeftDockWidgetArea, cdDock );

	//THUMBNAIL PIXMAP
	thumbDock = new QDockWidget(tr("Thumbnail"),this);
	thumbDock->setObjectName("thumbDock");
	pixThumbnail = new QLabel(thumbDock);
	pixThumbnail->setAlignment(Qt::AlignCenter);
	thumbDock->setWidget(pixThumbnail);
	thumbDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
	addDockWidget( Qt::LeftDockWidgetArea, thumbDock );

	//DIR LIST
	dirList = new QTableWidget(this);
	dirList->setAlternatingRowColors(true);
	dirList->setSelectionBehavior( QAbstractItemView::SelectRows );
	dirList->setColumnCount(3);
	dirList->setShowGrid(false);
	dirList->horizontalHeader()->setStretchLastSection(true);
	dirList->verticalHeader()->hide();
	dirList->setEditTriggers( QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked );
	clearDirList();
	connect( dirList, SIGNAL( itemChanged(QTableWidgetItem*) ), this, SLOT( dirItemChanged(QTableWidgetItem*) ) );
	connect( dirList, SIGNAL( itemSelectionChanged() ), this, SLOT( dirListSelectionChanged() ) );
	connect( dirList, SIGNAL( itemDoubleClicked(QTableWidgetItem*) ), this, SLOT( dirListDoubleClicked(QTableWidgetItem*) ) );
	setCentralWidget(dirList);

	formFind = NULL;
	createActions();
	createToolbar();
	createMenu();
}

/**
 * Empty dirList and sets its horizontalHeader ( clear() followed by setRowCount(0) is needed to prevent crashes on windows (!?) )
 */
void CDFly::clearDirList() {
	dirList->clear();
	dirList->setRowCount(0);
	QStringList th;
	th << tr("Name") << tr("Size") << tr("Date");
	dirList->setHorizontalHeaderLabels( th );
}

/**
 * Creates all actions used by both menubars and toolbars.
 */
void CDFly::createActions() {
	newAct = new QAction(QIcon(":/icons/filenew.png"), tr("&New"), this);
	newAct->setShortcut(tr("Ctrl+N"));
	newAct->setStatusTip(tr("New collection"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(fileNewClicked()));

	openAct = new QAction(QIcon(":/icons/fileopen.png"), tr("&Open"), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open collection"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(fileOpenClicked()));

	exitAct = new QAction(QIcon(":/icons/exit.png"), tr("&Quit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	exitAct->setStatusTip(tr("Close application"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(fileExitClicked()));

	settingsAct = new QAction(QIcon(":/icons/configure.png"), tr("&Settings"), this);
	settingsAct->setStatusTip(tr("Edit settings"));
	connect(settingsAct, SIGNAL(triggered()), this, SLOT(editSettingsClicked()));

	deleteAct = new QAction(QIcon(":/icons/editdelete.png"), tr("&Delete"), this);
	deleteAct->setShortcut(tr("Del"));
	deleteAct->setStatusTip(tr("Delete"));
	connect(deleteAct, SIGNAL(triggered()), this, SLOT(editDeleteClicked()));

	findAct = new QAction(QIcon(":/icons/find.png"), tr("&Find"), this);
	findAct->setShortcut(tr("Ctrl+F"));
	findAct->setStatusTip(tr("Search in the collection"));
	connect(findAct, SIGNAL(triggered()), this, SLOT(editFindClicked()));

	addCDAct = new QAction(QIcon(":/icons/cdrom_unmount.png"), tr("&Add CD"), this);
	addCDAct->setShortcut(tr("Ctrl+Ins"));
	addCDAct->setStatusTip(tr("Add a CD"));
	connect(addCDAct, SIGNAL(triggered()), this, SLOT(katalogAddClicked()));

	aboutAct = new QAction(QIcon(":/icons/cdfly.png"), tr("&About CdFly"), this);
	aboutAct->setStatusTip(tr("Show program info"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutClicked()));

	eraseLocationAct = new QAction(QIcon(":/icons/locationbar_erase.png"), tr("&Erase Location"), this);
	eraseLocationAct->setStatusTip(tr("Erase location"));
	connect(eraseLocationAct, SIGNAL(triggered()), this, SLOT(cmdEraseLocationClick()));

	setPathAct = new QAction(QIcon(":/icons/key_enter.png"), tr("&Go To"), this);
	setPathAct->setStatusTip(tr("Go to specified location"));
	connect(setPathAct, SIGNAL(triggered()), this, SLOT(cmdPathClick()));
}

/**
 * Creates the menuBar for the application.
 */
void CDFly::createMenu() {
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction( newAct );
	fileMenu->addAction( openAct );
	fileMenu->addSeparator();
	fileMenu->addAction( exitAct );

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction( findAct );
	editMenu->addAction( deleteAct );
	editMenu->addSeparator();
	editMenu->addAction( addCDAct );
	editMenu->addSeparator();
	editMenu->addAction( settingsAct );

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction( fileToolbar->toggleViewAction() );
	viewMenu->addAction( editToolbar->toggleViewAction() );
	viewMenu->addAction( locationToolbar->toggleViewAction() );
	viewMenu->addSeparator();
	viewMenu->addAction( cdDock->toggleViewAction() );
	viewMenu->addAction( thumbDock->toggleViewAction() );

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction( aboutAct );
}

/**
 * Creates the toolbars and inserts the default actions.
 */
void CDFly::createToolbar() {
	fileToolbar = addToolBar( tr("&File") );
	fileToolbar->setObjectName("fileToolbar");
	fileToolbar->setIconSize( QSize(16,16) );
	fileToolbar->addAction( newAct );
	fileToolbar->addAction( openAct );
	fileToolbar->addAction( exitAct );

	editToolbar = addToolBar( tr("&Edit") );
	editToolbar->setObjectName("editToolbar");
	editToolbar->setIconSize( QSize(16,16) );
	editToolbar->addAction( findAct );
	editToolbar->addAction( deleteAct );
	editToolbar->addAction( addCDAct );

	//LOCATION BAR
	locationToolbar = addToolBar( tr("&Location") );
	locationToolbar->setObjectName("locationToolbar");
	locationToolbar->setIconSize( QSize(16,16) );
	locationToolbar->addAction( eraseLocationAct );

	lblPath = new QLabel( tr("Location"), locationToolbar );
	locationToolbar->addWidget( lblPath );

	txtPath = new QLineEdit(locationToolbar);
	txtPath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	locationToolbar->addWidget( txtPath );
	connect(txtPath, SIGNAL(returnPressed()), this, SLOT(cmdPathClick()));

	locationToolbar->addAction( setPathAct );
}

/**
 * Attempts to open the collection specified by the filename property. If the collection is opened conf.lastkat is updated to remeber this as the last collection opened.
 */
void CDFly::open() {
	if (!filename().isEmpty()) {

		if ( !sql->setName(filename()) ) {
			cdlist->clear();
			clearDirList();
			txtPath->setText("");
			setFilename("");
			sql->db.close();
			QMessageBox::critical(this, QCoreApplication::applicationName(), tr("Error during file opening")+"\n"+sql->db.lastError().text());
		} else {
			//lastPathNode helps prevent useless dirList refresh
			lastPathNode = -1;
			createCdlist();
			setPath("/");
			conf.lastkat = filename();

			QSettings settings;
			settings.setValue("application/lastkat",conf.lastkat);
			settings.deleteLater();
		}
	}
}

/**
 * Generates the cd list
 */
void CDFly::createCdlist() {
	cdlist->clear();

	QVector<NodeRecord> nodes = sql->getNodes(-1);
	QVectorIterator<NodeRecord> i(nodes);
	while (i.hasNext()) {
		NodeRecord nr = i.next();
		QSqlListItem* newCD = new QSqlListItem(cdlist);
		newCD->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
		newCD->setIcon( QIcon(":/icons/cdrom_unmount.png") );
		newCD->id = nr.id;
		newCD->setText( nr.name );
	}
}

/**
 * Filename getter
 * @return current filename
 */
QString CDFly::filename() {
	return this->aFilename;
}

/**
 * Filename setter, this sets also the Window Title to include filename
 * @param name the name to set
 */
void CDFly::setFilename( const QString name ) {
	this->aFilename = name;
	this->setWindowTitle( QCoreApplication::applicationName() );
	if (!name.isEmpty())
		this->setWindowTitle( this->windowTitle() + " - \"" + name + "\"" );
}

/**
 * Show the clicked cd
 * @param node the item clicked
 */
void CDFly::cdlistClicked(QListWidgetItem *item) {
	if (item != NULL)
		setPath( ((QSqlListItem*)item)->id );
}

/**
 * Shows the statusTip of the selected item on the statusBar
 * @param node
 */
void CDFly::dirListSelectionChanged() {
	pixThumbnail->setText(" ");

	if (dirList->selectedItems().count()>0) {
		statusBar()->showMessage( dirList->selectedItems()[0]->statusTip() );

		QSqlQuery query;
		query.prepare("SELECT data FROM thumbnails WHERE id=:id");
		query.bindValue(":id",((QSqlTableItem*)dirList->selectedItems()[0])->id);
		query.exec();

		if ( query.next() ) {
			QPixmap pix;
			pix.loadFromData( query.record().value(0).toByteArray() );
			pixThumbnail->setPixmap( pix );
		}

		query.clear();
	}
}

/**
 * Changes the path to the double-clicked item
 * @param node selected item
 */
void CDFly::dirListDoubleClicked( QTableWidgetItem* item ) {
	if ( dirList->column(item)>0 )
		item = dirList->item( dirList->row(item), 0 );
	setPath( ((QSqlTableItem*)item)->id );
}

/**
 * Sets the path to the content of txtPath
 */
void CDFly::cmdPathClick() {
	setPath( txtPath->text() );
}

/**
 * Given the path string, this function finds the id of the last path element and passes the value to the setPath(int) function
 * @param path the path to show
 */
void CDFly::setPath(QString path) {
	QStringList nodes = path.split("/",QString::SkipEmptyParts);
	if (nodes.count()>0) {
		int node = -1;
		for (int x=0; x<nodes.count() && node>=-1; x++) {
			QString name( nodes[x] );
			name.replace("'","''");
			QVector<NodeRecord> list = sql->getNodes(node,"name='"+name+"'");

			if ( list.count()>0 )
				node = list[0].id;
			else
				node = -2;
		}

		if ( node>0 ) setPath(node);
	} else {
		clearDirList();
		txtPath->setText("/");
		lastPathNode = -1;
	}
}

/**
 * Given the node id, this function shows the content of the node in the dirList (id it's a directory), sets the correct text in txtPath and selects the items from cdList and dirList
 * @param node the node to show
 */
void CDFly::setPath(int node) {
	if (lastPathNode == node) return;

	QVector<NodeRecord> list = sql->getPathNodes(node);

	QVectorIterator<NodeRecord> i(list);
	QString path;
	while (i.hasNext()) {
		NodeRecord nr = i.next();
		path += "/" + nr.name;
	}

	NodeRecord last = list.last();
	NodeRecord first = list.first();

	//select the correct cd from the list (before that, deselect selected items)
	for (int x=0; x<cdlist->selectedItems().count(); x++)
		cdlist->setItemSelected( cdlist->selectedItems()[x], false);
	cdlist->setItemSelected( cdlist->findItems(first.name, Qt::MatchExactly)[0], true);

	if (last.type<2) {
		lastPathNode = node;

		txtPath->setText( path );

		clearDirList();
		MimeIcon mime;
		QString icon;

		//add .. elem only if the parent exists (IE: curElem is not a CD)
		if ( last.parent>0 ) {
			//new row
			int row = dirList->rowCount();
			dirList->setRowCount(row+1);
			//.. item with folder icon in the name column
			QSqlTableItem *preitem = new QSqlTableItem( ".." );
			preitem->setIcon( QIcon(":/icons/folder.png") );
			preitem->id = last.parent; //id to the parent
			preitem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ); //non-editable
			dirList->setItem(row, 0, preitem);
			dirList->verticalHeader()->resizeSection(row, 22);
		}

		//fill the tableview with all the children elements
		QVector<NodeRecord> nodes = sql->getNodesWithMeta( last.id );
		QVectorIterator<NodeRecord> in(nodes);
		while (in.hasNext()) {
			NodeRecord nr = in.next();
			//get icon type (check for folder element)
			if (nr.type==1) {
				icon = "folder";
			} else if (nr.hasThumb) {
				icon = "image-thumb";
			} else {
				icon = mime.getIcon( nr.name );
			}

			//add a new row to the table
			int row = dirList->rowCount();
			dirList->setRowCount(row+1);

			//name cell
			QString name( nr.name );
			QSqlTableItem *nitem = new QSqlTableItem( name );
			nitem->setIcon( QIcon(":/icons/"+icon+".png") );
			nitem->id = nr.id;
			nitem->setStatusTip(name);
			dirList->setItem(row, 0, nitem);

			//human readable size string
			double size = nr.size;
			QStringList units;
			units << "" << "K" << "M" << "G" << "T" << "P";
			int x = 0;
			while (x<units.count() && size>1024) {
				size /= 1024;
				x++;
			}

			//size cell
			QTableWidgetItem *sitem = new QTableWidgetItem( QString::number(size,'f',2) + " " + units[x] + "Byte" );
			sitem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ); //non-editable
			dirList->setItem(row, 1, sitem);

			//date cell
			QDateTime dateTime = QDateTime::fromString(nr.date, "yyyy-MM-dd hh:mm:ss");
			QTableWidgetItem *ditem = new QTableWidgetItem( dateTime.toString("hh:mm, d MMMM yyyy") ); //user friendly date string
			ditem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled ); //non-editable
			dirList->setItem(row, 2, ditem);

			dirList->verticalHeader()->resizeSection(row, 22);
			//dirList->resizeRowToContents(row); //a bit too short
		}
	} else {
		//if item is a file
		//browse to its folder
		if ( (lastPathNode != last.parent) && (last.parent != last.id) )
			setPath( last.parent );
		//and then select the correct entry in the file list
		for (int x=0; x<dirList->selectedItems().count(); x++)
			dirList->setItemSelected( dirList->selectedItems()[x], false);
		dirList->setItemSelected( dirList->findItems(last.name, Qt::MatchExactly)[0], true);
	}
}

/**
 * Renames the item in the database
 * @param item The id of the item to rename
 * @param text The new name for the item
 */
void CDFly::cdItemChanged( QListWidgetItem *item ) {
	QSqlListItem *curItem = (QSqlListItem *)item;
	NodeRecord nr = sql->getNode( curItem->id );
	if (nr.name != item->text()) {
		sql->rename(curItem->id, item->text());
	}
}

/**
 * Renames the item in the database
 * @param item The id of the item to rename
 * @param text The new name for the item
 */
void CDFly::dirItemChanged( QTableWidgetItem *item ) {
	//if (item->tableWidget()->column(item)==1) {
	if (dirList->column(item)==0) {
		QSqlTableItem *curItem = (QSqlTableItem *)item;
		NodeRecord nr = sql->getNode( curItem->id );
		if (nr.name != item->text() && item->text() != "..") { //strange bug
			qDebug() << nr.name << item->text();
			sql->rename(curItem->id, item->text());
		}
	}
}

/**
 * Creates a dialog and sets its content with aboutWidget::setupUi, the it modally shows it
 */
void CDFly::aboutClicked() {
	QDialog *diag = new QDialog(this);
	Ui::aboutWidget().setupUi(diag);
	diag->exec();
	delete diag;
}

#ifdef Q_WS_X11
bool CDFly::isMounted(QString dev) {
	//checking presence of dev in /etc/mtab
	QFile *mtab = new QFile("/etc/mtab");
	if ( mtab->open(QIODevice::ReadOnly) ) {
		QStringList mtabCont;
		mtabCont << QString( mtab->readAll() ).split("\n");

		QStringListIterator i(mtabCont);
		while (i.hasNext())
			if (i.next().startsWith(dev)) return true;
	}

	return false;
}

int CDFly::mountDev(QString dev) {
	//try running "mount $dev"
	QProcess mount;
	mount.start( "mount", QStringList() << dev );
	mount.waitForFinished();
	return mount.exitCode();
}

int CDFly::umountDev(QString dev) {
	//try running "umount $dev"
	QProcess umount;
	umount.start( "umount", QStringList() << dev );
	umount.waitForFinished();
	return umount.exitCode();
}

QString CDFly::devToMountpoint(QString dev) {
	//checking /etc/mtab for corresponding mountpoint
	//checking presence of dev in /etc/mtab
	QFile *mtab = new QFile("/etc/mtab");
	if ( mtab->open(QIODevice::ReadOnly) ) {
		QStringList mtabCont;
		mtabCont << QString( mtab->readAll() ).split("\n");

		QString cur;
		QStringListIterator i(mtabCont);
		while (i.hasNext()) {
			cur = i.next();
			if (cur.startsWith(dev)) {
				QRegExp rx(dev+"\\s([^\\s]+)");
				if (rx.indexIn(cur, 0) != -1) {
					return rx.cap(1);
				}
			}
		}
	}

	return "";
}
#endif

/**
 * If the db is Open, shows the newDisk dialog and starts crawling cd structure
 */
void CDFly::katalogAddClicked() {
	if ( sql->db.isOpen() ) {
		NewDisk *ndisk = new NewDisk( this, conf.mountpoint );

		if ( ndisk->exec() ) {
			QString diskName = ndisk->getDiskName();
			QString mountPoint;

			QString dev = ndisk->getMountPoint();
#ifdef Q_WS_X11
			bool mountedByCdfly = false;
			if ( !isMounted(dev) ) {
				if (QMessageBox::question(this, QCoreApplication::applicationName(), tr("The media is not mounted.\nWould you like CdFly to try to mount the device?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
					if ( mountDev(dev)!=0 ) {
						QMessageBox::warning(this, QCoreApplication::applicationName(), tr("Failed to mount device, verify that the specified device node is correct."));
						return;
					}
					mountedByCdfly = true;
			}
			mountPoint = devToMountpoint(dev);
#else
			mountPoint = dev;
#endif

			if ( !diskName.isEmpty() && !dev.isEmpty()  ) {
				conf.mountpoint = dev;
				int cdnumber = sql->scanDisk( mountPoint, diskName );
				if (cdnumber >= 0) {
					QSqlListItem *newCD = new QSqlListItem( cdlist );
					newCD->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
					newCD->id = cdnumber;
					newCD->setText( diskName );
					newCD->setIcon( QIcon(":/icons/cdrom_unmount.png") );

					//resort items
					cdlist->sortItems();
				}
			}

#ifdef Q_WS_X11
			if (mountedByCdfly) {
				if ( umountDev(dev)!=0 ) {
					QMessageBox::warning(this, QCoreApplication::applicationName(), tr("Failed to umount device."));
					return;
				}
			}
#endif
		}
	} else {
		QMessageBox::warning(this, QCoreApplication::applicationName(), tr("Operation not permitted.\nYou have to open or create a collection first."));
	}
}

void CDFly::editSettingsClicked() {
	Settings *set = new Settings(this);
	if (set->exec()) {
		QSettings settings;

		settings.beginGroup("application");
		settings.setValue("mountpoint",conf.mountpoint);
		settings.setValue("startup",conf.startup);
		settings.setValue("startkat",conf.startkat);
		settings.setValue("locale",conf.locale);
		settings.endGroup();

		settings.beginGroup("thumbnails");
		settings.setValue("perdir",conf.thperdir);
		settings.setValue("enabled",conf.thenabled);
		settings.setValue("size",conf.thsize);
		settings.endGroup();

		settings.deleteLater();
	}
	delete set;
}


void CDFly::editFindClicked() {
	if ( sql->db.isOpen() ) {
		if (formFind==NULL)
			formFind = new Find(this);
		formFind->show();
	} else {
		QMessageBox::warning(this, QCoreApplication::applicationName(), tr("Operation not permitted.\nYou have to open or create a collection first."));
	}
}

void CDFly::stopClicked() {
	if (formFind!=NULL) {
		formFind->stopSearch();
	}
}

void CDFly::showProgress( bool show, int max ) {
	progress->setVisible( show );
	btnStop->setVisible( show );
	progress->setMaximum( max );
}

void CDFly::setProgress( int pos ) {
	progress->setValue( pos );
}

void CDFly::editDeleteClicked() {
	int id = -1;
	if ( cdlist->hasFocus() && cdlist->selectedItems().count()>0 ) {
		id = ((QSqlListItem*)cdlist->selectedItems()[0])->id;
	} else if ( dirList->hasFocus() && dirList->selectedItems().count()>0 ) {
		id = ((QSqlTableItem*)dirList->selectedItems()[0])->id;
	}

	if (id != -1) {
		QVector<NodeRecord> list;
		list << sql->getNode( id );

		if (QMessageBox::question(this, tr("Confirm item deletion"), QString(tr("Do you really want to delete '%1'?")).arg(list[0].name), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
			int parent = list[0].parent;

			//spawn a progress dialog
			ProgressDialog *prog = new ProgressDialog(this, sql->getCountRecursive(id)+1);
			prog->hideDirInfo();
			prog->show();
			sql->deleteNodes( list, prog );
			sql->db.exec( QString("VACUUM") );
			prog->close();

			lastPathNode = -1;
			createCdlist();
			if (parent<0)
				setPath("/");
			else
				setPath(parent);
		}

	}
}

void CDFly::fileNewClicked() {
	QString cdf = QFileDialog::getSaveFileName( this, tr("Choose a name for this CdFly collection"), "", "CdFly collections (*.cdf)" );
	if (!cdf.isEmpty()) {
		setFilename( cdf );
		open();
	}
}

void CDFly::fileExitClicked() {
	close();
}

void CDFly::fileOpenClicked() {
	QString cdf = QFileDialog::getOpenFileName( this, tr("Choose a CdFly collection"), "", "CdFly collections (*.cdf)" );
	if (!cdf.isEmpty()) {
		setFilename( cdf );
		open();
	}
}

void CDFly::cmdEraseLocationClick() {
	setPath("/");
}

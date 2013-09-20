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

#ifndef _CDBASE_H_
#define _CDBASE_H_

#include <QMainWindow>

class Find;
class CdSql;
class QSqlListItem;
class QSqlTableItem;
class QLineEdit;
class QLabel;
class QApplication;
class QProgressBar;
class QToolButton;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QTableWidgetItem;
class QMenu;
class QAction;
class QToolBar;
class QTranslator;
class QDockWidget;

struct configuration {
	bool thenabled;
	int startup, thsize, thperdir;
	QString mountpoint, startkat, lastkat, locale;
};

struct langinfo {
	QString langcode, langname;
};

/**
 * @short Application Main Window
 * @author Ryo Saeba <ryosaeba@dotgeek.org>
 * @version 0.3
 */
class CDFly : public QMainWindow {
		Q_OBJECT
	protected:
		void closeEvent(QCloseEvent *);

	private:
		QString aFilename;
		int lastPathNode;
		Find *formFind;

		//LOCATION BAR
		QLineEdit *txtPath;
		QLabel *lblPath;

		//PROGRESS STATUS
		QProgressBar *progress;
		QToolButton *btnStop;

		//MENUS
		QMenu *fileMenu;
		QMenu *editMenu;
		QMenu *viewMenu;
		QMenu *helpMenu;

		//TOOLBARS
		QToolBar *fileToolbar;
		QToolBar *editToolbar;
		QToolBar *locationToolbar;

		//CDLIST
		QListWidget *cdlist;
		QDockWidget *cdDock;

		QTableWidget *dirList;

		//THUMB
		QLabel *pixThumbnail;
		QDockWidget *thumbDock;

		//ACTIONS
		QAction *newAct;
		QAction *openAct;
		QAction *exitAct;

		QAction *settingsAct;
		QAction *renameAct;
		QAction *findAct;
		QAction *deleteAct;

		QAction *addCDAct;

		QAction *aboutAct;

		QAction *eraseLocationAct;
		QAction *setPathAct;

		void createCdlist();
		void createActions();
		void createMenu();
		void createToolbar();

		void clearDirList();

		void setupWidgets();

	public slots:
		virtual void cdlistClicked(QListWidgetItem *item);
		virtual void cdItemChanged( QListWidgetItem * );
		virtual void dirItemChanged( QTableWidgetItem * );
		virtual void cmdPathClick();
		virtual void cmdEraseLocationClick();
		virtual void dirListSelectionChanged();
		virtual void dirListDoubleClicked(QTableWidgetItem *item);
		virtual void stopClicked();

		//menu events
		virtual void aboutClicked();
		virtual void katalogAddClicked();
		virtual void editSettingsClicked();
		virtual void editFindClicked();
		virtual void editDeleteClicked();
		virtual void fileExitClicked();
		virtual void fileOpenClicked();
		virtual void fileNewClicked();

	public:
#ifdef Q_WS_X11
		bool isMounted(QString dev);
		int mountDev(QString dev);
		int umountDev(QString dev);
		QString devToMountpoint(QString dev);
#endif

		configuration conf;
		CdSql *sql;
		QTranslator *translator;
		QVector<langinfo> languages;
		QString filename();
		void setFilename( const QString name );
		void open();
		void setPath(QString);
		void setPath(int);
		void showProgress( bool, int max = 100 );
		void setProgress( int );

		/**
		 * Default Constructor
		 */
		CDFly(QWidget *parent = 0);

		/**
		 * Default Destructor
		 */
		~CDFly();
};

#endif // _CDBASE_H_

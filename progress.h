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

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include "ui_progresswidget.h"
#include <QApplication>
#include <QtGui>

class ProgressDialog : public QDialog, private Ui::ProgressWidget {
		Q_OBJECT
	private:
		bool canceled;
		QDateTime start;
		int lastElapsed;

	public:
		ProgressDialog(QWidget *parent = 0, int max = 100);
		bool isCanceled() { return canceled; }
		void setProgress( int x );
		void incProgress();
		void setFile( QString s );
		void setPath( QString s );
		void hideDirInfo();

	public slots:
		virtual void cancel() { canceled = true; }
};

#endif

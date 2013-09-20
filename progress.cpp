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

#include "progress.h"

ProgressDialog::ProgressDialog(QWidget *parent, int max) : QDialog( parent ) {
	setupUi(this);
	progressBar->setMaximum( max );
	canceled = false;
	connect( cancelButton, SIGNAL(clicked()), this, SLOT(cancel()) );

	start = QDateTime::currentDateTime();
	lastElapsed = 0;
}

void ProgressDialog::hideDirInfo() {
	lblPath->hide();
	lblPathHead->hide();
	lblFile->hide();
	lblFileHead->hide();
	resize(size().width(),90);
}

void ProgressDialog::setProgress( int x ) {
	progressBar->setValue(x);
}

void ProgressDialog::setFile( QString s ) {
	lblFile->setText(s);
}

void ProgressDialog::setPath( QString s ) {
	lblPath->setText(s);
}

void ProgressDialog::incProgress() {
	progressBar->setValue( progressBar->value()+1 );

	int tsecs = start.secsTo( QDateTime::currentDateTime() );
	if (lastElapsed != tsecs) {
		lastElapsed = tsecs;

		int secs = tsecs % 60;
		int mins = tsecs / 60;
		lblElapsed->setText(QString::number(mins)+"m "+QString::number(secs)+"s");

		tsecs = tsecs*(progressBar->maximum()-progressBar->value())/progressBar->value();
		secs = tsecs % 60;
		mins = tsecs / 60;
		lblEta->setText( QString::number(mins)+"m "+QString::number(secs)+"s");
	}
}

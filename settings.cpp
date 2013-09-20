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

#include "settings.h"
#include "cdfly.h"

Settings::Settings(QWidget *parent)
		: QDialog(parent) {
	setupUi(this);

#ifdef Q_WS_X11
	lblMount->setText(tr("CD device node (Example: /dec/hdc)"));
	btnMount->hide();
#else
	connect( btnMount, SIGNAL(clicked()), this, SLOT(browseMount()) );
#endif

	connect( btnBrowseKat, SIGNAL(clicked()), this, SLOT(browseKat()) );
	connect( buttonOk, SIGNAL(clicked()), this, SLOT(apply()) );

	conf = &((CDFly*)parent)->conf;
	switch(conf->startup) {
			case 0:
			radStartLast->setChecked(true);
			break;
			case 1:
			radStartSpecified->setChecked(true);
			break;
			case 2:
			radStartBlank->setChecked(true);
	}

	txtMount->setText( conf->mountpoint );
	txtStart->setText( conf->startkat );

	//Thumbnails Tab
	chkThumbnails->setChecked( conf->thenabled );
	spinSize->setValue( conf->thsize );
	spinNumThumbs->setValue( conf->thperdir );

	//Locale
	languages = &((CDFly*)parent)->languages;
	for (int x=0; x<languages->size(); x++) {
		cmbLocale->insertItem(x,QIcon(":/flags/"+languages->at(x).langcode.left(2)+".png"),languages->at(x).langname);
		if (languages->at(x).langcode == conf->locale)
			cmbLocale->setCurrentIndex(x);
	}
}

void Settings::browseKat() {
	QString kat = QFileDialog::getOpenFileName( this, tr("Choose a CdFly collection"), txtStart->text(), "CdFly collections (*.cdf)" );
	if (!kat.isEmpty())
		txtStart->setText( kat );
}

void Settings::browseMount() {
	QFileDialog *fd = new QFileDialog(this);
	fd->setFileMode(QFileDialog::DirectoryOnly);
	fd->setAcceptMode(QFileDialog::AcceptOpen);
	fd->setDirectory( txtMount->text() );
	if (fd->exec())
		txtMount->setText( fd->selectedFiles()[0] );
}

void Settings::apply() {
	conf->mountpoint = txtMount->text();
	if (radStartLast->isChecked())
		conf->startup = 0;
	if (radStartSpecified->isChecked())
		conf->startup = 1;
	if (radStartBlank->isChecked())
		conf->startup = 2;
	conf->startkat = txtStart->text();

	//Locale
	conf->locale = languages->at(cmbLocale->currentIndex()).langcode;

	//Thumbnails Tab
	conf->thenabled = chkThumbnails->isChecked();
	conf->thperdir = spinNumThumbs->value();
	conf->thsize = spinSize->value();

	accept();
}

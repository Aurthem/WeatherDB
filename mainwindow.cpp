//#include <QtWidgets>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
//#include <QMenu>
#include <QMenuBar>
#include <QApplication>

#include "mainwindow.h"
#include "database.h"
#include "dialogs/customsearchdialog.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	searchDialog=new CustomSearchDialog(this);
	searchDialog->hide();

	createActions();
	createMenus();

	setWindowTitle(tr("WeatherDB"));
	setMinimumSize(200, 350);
	resize(800, 565);

	msg=new QMessageBox(QMessageBox::Information,"Information","Message");
}
MainWindow::~MainWindow() {
	delete msg;
}

void MainWindow::closeEvent(QCloseEvent * event) {
	Database::getInstance().finalize();
	QMainWindow::closeEvent(event);
}

void MainWindow::newFile() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Create new database"),
				QApplication::applicationDirPath(), tr("Database (*.db)"));
	Database::getInstance().setupDatabase(fileName);

//	QFile *newFile = new QFile(fileName);
//	if(!newFile->open(QIODevice::WriteOnly)) {
//		QMessageBox::warning(NULL, "Test", "Unable to open: " + fileName , "OK");
//	} else {
////		QTextStream testStream(newFile);
////		testStream << "Write this text to the file\n";
//	}
//	delete newFile;
//	msg->setText(fileName);
//	msg->show();
}

void MainWindow::open() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open database"),
				QApplication::applicationDirPath(), tr("Database (*.db)"),
				0, QFileDialog::DontConfirmOverwrite);
	Database::getInstance().setupDatabase(fileName);
}

void MainWindow::copy() {
	msg->setText(tr("Invoked <b>Edit|Copy</b>"));
	msg->show();
}

void MainWindow::about() {
	QMessageBox::about(this, tr("About"),
										 tr("<p align='center'>"
												"This program is written with<br>Qt 5.5.1 and SQLite 3.8.10.2<br>"
												"by Borodenko Aurthem &lt;aurthem@gmail.com&gt;</p>"));
}

void MainWindow::createActions() {
	newAct = new QAction(tr("Create..."), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create new database"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
//	newAct->setEnabled(false);

	openAct = new QAction(tr("Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open existing database"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
//	openAct->setEnabled(false);

	exitAct = new QAction(tr("Exit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Terminate this program"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

	searchAct = new QAction(tr("Search..."), this);
	searchAct->setStatusTip(tr("Perform compound search query"));
	connect(searchAct, &QAction::triggered, [this] {
		searchDialog->show();
		searchDialog->raise();
		searchDialog->activateWindow();	//modeless dialog
//		emit searchRequested();
	});
	connect(searchDialog,&CustomSearchDialog::accepted,[this]{
		emit searchRequested(searchDialog->getSearchQuery());
	});
//	searchAct->setEnabled(false);

	toggleVertical= new QAction(tr("Numbers: vertical"), this);
	toggleVertical->setStatusTip(tr("Use vertical representation of values in the table"));
	toggleVertical->setCheckable(true); toggleVertical->setChecked(true);
	connect(toggleVertical, &QAction::triggered, [this] {
		emit toggledNumberRepresentation(toggleVertical->isChecked());
	});
	toggleAdvanced= new QAction(tr("Additional options"), this);
	toggleAdvanced->setStatusTip(tr("Show additional options for database control"));
	toggleAdvanced->setCheckable(true); toggleAdvanced->setChecked(false);
	connect(toggleAdvanced, &QAction::triggered, [this] {
		emit toggledAdvancedOptions(toggleAdvanced->isChecked());
	});

	aboutAct = new QAction(tr("About"), this);
	aboutAct->setStatusTip(tr("Show information about this program"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("Database"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	viewMenu = menuBar()->addMenu(tr("Representation"));
	viewMenu->addAction(searchAct);
	viewMenu->addSeparator();
	viewMenu->addAction(toggleVertical);
	viewMenu->addAction(toggleAdvanced);

	helpMenu = menuBar()->addMenu(tr("Help"));
	helpMenu->addAction(aboutAct);
}

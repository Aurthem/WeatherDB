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
//	QWidget *topFiller = new QWidget;
//	topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//	QWidget *bottomFiller = new QWidget;
//	bottomFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//	QVBoxLayout *layout = new QVBoxLayout;
//	layout->setMargin(5);
//	layout->addWidget(topFiller);
//	layout->addWidget(centralWidget());
//	layout->addWidget(bottomFiller);
//	this->setLayout(layout);

	searchDialog=new CustomSearchDialog(this);
	searchDialog->hide();

	createActions();
	createMenus();

//	QString message = tr("A context menu is available by right-clicking");
//	statusBar()->showMessage(message,2000);

	setWindowTitle(tr("WeatherDB"));
	setMinimumSize(200, 350);
	resize(800, 500);

	msg=new QMessageBox(QMessageBox::Information,"Information","Message");

}
MainWindow::~MainWindow() {
	delete msg;
}


void MainWindow::closeEvent(QCloseEvent * event) {
	Database::getInstance().finalize();
	QMainWindow::closeEvent(event);
}

//void MainWindow::contextMenuEvent(QContextMenuEvent *event)
//{
//	QMenu menu(this);
//	menu.addAction(copyAct);
//	menu.exec(event->globalPos());
//}

void MainWindow::newFile() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Создать новую базу данных"),
				QApplication::applicationDirPath(), tr("База данных (*.db)"));
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
	QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть базу данных"),
				QApplication::applicationDirPath(), tr("База данных (*.db)"),
				0, QFileDialog::DontConfirmOverwrite);
	Database::getInstance().setupDatabase(fileName);
//	msg->setText(tr("Invoked <b>File|Open</b>"));
//	msg->show();
}

//void MainWindow::save() {
//	msg->setText(tr("Invoked <b>File|Save</b>"));
//	msg->show();
//}

void MainWindow::undo() {
	msg->setText(tr("Invoked <b>Edit|Undo</b>"));
	msg->show();
}

void MainWindow::redo() {
	msg->setText(tr("Invoked <b>Edit|Redo</b>"));
	msg->show();
}

void MainWindow::copy() {
	msg->setText(tr("Invoked <b>Edit|Copy</b>"));
	msg->show();
}

void MainWindow::about() {
//	msg->setText(tr("Invoked <b>Help|About</b>"));
//	msg->show();
	QMessageBox::about(this, tr("О программе"),
										 tr("<p align='center'>"
												"Программа написана на Qt 5.5.1 и SQLite 3.8.10.2<br>"
												"Бороденко Артёмом &lt;aurthem@gmail.com&gt;</p>"));
}

//void MainWindow::emitSearchRequested() {
//	emit searchRequested();
//}
//void MainWindow::emitToggledNumberRepresentation(void) {
//	emit toggledNumberRepresentation(toggleVertical->isChecked());
//}

void MainWindow::createActions() {
	newAct = new QAction(tr("Создать..."), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Создать новую базу данных"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
//	newAct->setEnabled(false);

	openAct = new QAction(tr("Открыть..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Открыть существующую базу данных"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
//	openAct->setEnabled(false);

//	saveAct = new QAction(tr("Save"), this);
//	saveAct->setShortcuts(QKeySequence::Save);
//	saveAct->setStatusTip(tr("Save the document to disk"));
//	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	exitAct = new QAction(tr("Выход"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Выйти из приложения"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

//	undoAct = new QAction(tr("&Undo"), this);
//	undoAct->setShortcuts(QKeySequence::Undo);
//	undoAct->setStatusTip(tr("Undo the last operation"));
//	connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

//	redoAct = new QAction(tr("&Redo"), this);
//	redoAct->setShortcuts(QKeySequence::Redo);
//	redoAct->setStatusTip(tr("Redo the last operation"));
//	connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

	searchAct = new QAction(tr("Выборка..."), this);
	searchAct->setStatusTip(tr("Произвести выборку с составными условиями"));
//	connect(searchAct, SIGNAL(triggered()), this, SLOT(emitSearchRequested()));
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

	toggleVertical= new QAction(tr("Числа: вертикально"), this);
	toggleVertical->setStatusTip(tr("Показывать числа вертикально в таблице"));
	toggleVertical->setCheckable(true); toggleVertical->setChecked(true);
//	connect(toggleVertical, SIGNAL(triggered()), this, SLOT(emitToggledNumberRepresentation()));
	connect(toggleVertical, &QAction::triggered, [this] {
		emit toggledNumberRepresentation(toggleVertical->isChecked());
	});
	toggleAdvanced= new QAction(tr("Дополнительные опции"), this);
	toggleAdvanced->setStatusTip(tr("Показывать дополнительные опции по управлению базой данных"));
	toggleAdvanced->setCheckable(true); toggleAdvanced->setChecked(false);
	connect(toggleAdvanced, &QAction::triggered, [this] {
		emit toggledAdvancedOptions(toggleAdvanced->isChecked());
	});

	aboutAct = new QAction(tr("О программе"), this);
	aboutAct->setStatusTip(tr("Показать информацию об этой программе"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

//	fileGroup = new QActionGroup(this);
//	fileGroup->addAction(newAct);
//	fileGroup->addAction(openAct);
//	fileGroup->addAction(saveAct);
//	fileGroup->addAction(exitAct);
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("База данных"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
//	fileMenu->addAction(saveAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	viewMenu = menuBar()->addMenu(tr("Представление"));
//	viewMenu->addAction(undoAct);
//	viewMenu->addAction(redoAct);
	viewMenu->addAction(searchAct);
	viewMenu->addSeparator();
	viewMenu->addAction(toggleVertical);
	viewMenu->addAction(toggleAdvanced);

	helpMenu = menuBar()->addMenu(tr("Справка"));
	helpMenu->addAction(aboutAct);
}

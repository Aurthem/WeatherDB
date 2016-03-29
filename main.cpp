//#include <QApplication>

#include "mainwindow.h"
#include "view/tabview.h"

//#include <QtWidgets>
#include <QApplication>
#include <QTranslator>
//#include <QLocale>
//#include <QLibraryInfo>
#include <QFile>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
//	a.setAttribute(Qt::AA_DontShowIconsInMenus);	//doesn't help

	//don't forget to trim .ts file and release it with Tools/External/Linguist/Release...
	//also, be careful with update, because it overwrites .qm and marks all default_ru_RU entries as obsolete
	//you can merge two .ts files with this: "lconvert -i primary.ts secondary.ts -o complete.ts"
	QTranslator *defaultTranslator=new QTranslator(&app);
	if (defaultTranslator->load("default_"+QLocale::system().name(),":/lang"))
		app.installTranslator(defaultTranslator);
	QTranslator *appTranslator=new QTranslator(&app);
	if (appTranslator->load("WeatherDB_"+QLocale::system().name(),":/lang"))
		app.installTranslator(appTranslator);

	MainWindow w;
	TabHolder *tabHolder=new TabHolder();
	w.setCentralWidget(tabHolder);

	QFile file("styles.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	w.setStyleSheet(styleSheet);
//	w.ensurePolished();

	app.connect(&w,SIGNAL(toggledNumberRepresentation(bool)),
					tabHolder,SLOT(handleNumberRepresentationToggle(bool)));

//	a.connect(&a,&QApplication::aboutToQuit,//&Database::getInstance(),&Database::finalize);
//		[=] { Database::getInstance().finalize(); });

	tabHolder->addTab();

	w.show();

	return app.exec();
}

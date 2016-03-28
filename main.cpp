//#include <QApplication>

#include "mainwindow.h"
#include "view/tabview.h"

//#include <QtWidgets>
#include <QApplication>
#include <QTranslator>
//#include <QLocale>
//#include <QLibraryInfo>
#include <QFile>


//#include <intrin.h>
////#include <windows.h>
//#include <dbghelp.h>
//void printStack(void) {
//	unsigned int i;
//	void* stack[100];
//	unsigned short frames;
//	SYMBOL_INFO* symbol;
//	HANDLE process;

//	process = GetCurrentProcess();

//	SymInitialize(process, NULL, TRUE);

//	frames = CaptureStackBackTrace(0, 100, stack, NULL);
//	symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
//	symbol->MaxNameLen = 255;
//	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

//	for(i=0; i<frames; i++) {
//		SymFromAddr(process, (DWORD64)( stack[i] ), 0, symbol);
//		printf( "%i: %s - 0x%0llX\n", frames - i - 1, symbol->Name, symbol->Address );
//	}

//	free(symbol);
//}
//void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
//	QByteArray localMsg = msg.toLocal8Bit();
//	switch (type) {
//	case QtDebugMsg:
//		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//		break;
//	case QtInfoMsg:
//		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//		break;
//	case QtWarningMsg:
//		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//		break;
//	case QtCriticalMsg:
//		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
//		break;
//	case QtFatalMsg:
//		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
////		printStack();
////		__debugbreak();
//		abort();
//	}
//}


int main(int argc, char *argv[])
{
//	qInstallMessageHandler(myMessageOutput);

	QApplication app(argc, argv);
//	a.setAttribute(Qt::AA_DontShowIconsInMenus);	//doesn't help

//	QTranslator *translator = new QTranslator(&app);
//	QString translatorFileName = QLatin1String("qt_");
//	translatorFileName += QLocale::system().name();
//	if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
//		app.installTranslator(translator);
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

//	a.connect(Database::getInstance().model,&DBModel::rowsAboutToBeRemoved,
//						&Database::getInstance(),&Database::handleRowsRemoved);
//	a.connect(Database::getInstance().model,&DBModel::columnsAboutToBeRemoved,
//						&Database::getInstance(),&Database::handleColumnsRemoved);

//	a.connect(&a,&QApplication::aboutToQuit,//&Database::getInstance(),&Database::finalize);
//		[=] { Database::getInstance().finalize(); });

//	if (!createConnection()) return 1;
//	QSqlDatabase::addDatabase("QSQLITE");

//---
//	QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE","testConnection");
//	database.setDatabaseName("test.db");
//	DBModel model(0,database);
//---

//	QueryModelView testModel;
//	initializeModel(&testModel);
//	QString tmp_query("select id,wind,year from weather");
//	model.setQuery_DB(tmp_query);

//	model.setFilter("year<2000");
//	model.select();

//	tabHolder->setModel(model);
//	tabHolder->setDatabase(database);

//	DBView* tmpView=new DBView();
//	tmpView->setModel(&hpm);

//	tmpView->setStyleSheet(
//				"QHeaderView::section:horizontal {"
//													"background: rgb(255,0,0);"
//												 "}");
//		tmpView->horizontalHeader()->hide();
	//	tmpView->verticalHeader()->hide();
//	tabView->adjustSize();

	tabHolder->addTab();

	w.show();

	return app.exec();
}

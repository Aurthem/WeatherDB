#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
QT_BEGIN_NAMESPACE
class QMessageBox;
QT_END_NAMESPACE

class CustomSearchDialog;

class MainWindow : public QMainWindow
{	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
//	void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
	void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
private slots:
	void newFile();
	void open();
//	void save();
	void undo();
	void redo();
	void copy();
	void about();

//	void emitToggledNumberRepresentation(void);
//	void emitSearchRequested(void);

signals:
	void toggledNumberRepresentation(bool vertical);
	void toggledAdvancedOptions(bool advancedOptions);
	void searchRequested(const QMap<QString, QPair<QVariant,QVariant> > &searchQuery);

private:
	void createActions();
	void createMenus();

	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *helpMenu;
//	QActionGroup *fileGroup;	//use group for "choose one" options
	QAction *newAct;
	QAction *openAct;
//	QAction *saveAct;
	QAction *exitAct;
//	QAction *undoAct;
//	QAction *redoAct;
	QAction *searchAct;
	QAction *toggleVertical;
	QAction *toggleAdvanced;
	QAction *aboutAct;

	QMessageBox *msg;

	CustomSearchDialog* searchDialog;
};

#endif // MAINWINDOW_H

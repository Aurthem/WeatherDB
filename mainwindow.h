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
	void copy();
	void about();

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
	QAction *newAct;
	QAction *openAct;
	QAction *exitAct;
	QAction *searchAct;
	QAction *toggleVertical;
	QAction *toggleAdvanced;
	QAction *aboutAct;

	QMessageBox *msg;

	CustomSearchDialog* searchDialog;
};

#endif // MAINWINDOW_H

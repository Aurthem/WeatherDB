#ifndef TABVIEW_H
#define TABVIEW_H

#include <QWidget>
#include <QTabWidget>

#include <QVariant>	//for QSet
#include <QSet>
QT_BEGIN_NAMESPACE
class QStatusBar;
QT_END_NAMESPACE

class FreezeTableWidget;
class DBModel;
class AddRowProxyModel;
class FilterProxyModel;
class HorizontalProxyModel;

class TabView : public QWidget
{	Q_OBJECT
public:
	explicit TabView(QWidget *parent=0);

	void setVerticalNumberRepresentation(bool vertical);

	void restoreVhState(const QPair<QMap<int,int>, QList<int> > &vhState);
	QPair<QMap<int,int>, QList<int> > saveVhState() const;

public slots:
protected:
	virtual bool event(QEvent * event) Q_DECL_OVERRIDE;	//for status tips
private:
	FreezeTableWidget *view;
	DBModel *model;

	AddRowProxyModel *proxy;
	FilterProxyModel *filter;
	HorizontalProxyModel *adapter;	//applied after to switch columns<->rows

	QStatusBar *statusBar;
	QPair<QString, QString> statusMessage;	//Filter: <"first1, first2">; Hidden: <"second1, second2">
	bool showStatusMessage(void) const;

	QSet<int> rowIdsToRemove;	//for marks update after submitAll

	static bool advancedOptionsEnabled;
	QWidget* advancedOptions;
signals:
	void customRowsRemoved(const QSet<int> &rowIds);	//used for updating marks
	void customColumnsRemoved(const QList<int> &columns);	//used for updating marks
private slots:
	void submit();
	bool removeColumns(const QList<int> &columns);
	void handleAlterColumns(bool isDBchanged, const QList<QList<QVariant> > &newHeaders);
	void handleAddColumn(const QList<QVariant> &record);

	void toggleAdvancedOptions(bool isAdvanced);
};

class TabHolder : public QTabWidget
{	Q_OBJECT
public:
	explicit TabHolder(QWidget *parent = 0);

public slots:
	void customTabMenuRequested(const QPoint &pos);

	void addTab(void);

	void removeTabView(int idx);	//will delete tab widget (!)

	void handleNumberRepresentationToggle(bool vert);
private:
	bool vertical;
};

#endif // TABVIEW_H

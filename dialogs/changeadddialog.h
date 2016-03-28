#ifndef CHANGEADDDIALOG_H
#define CHANGEADDDIALOG_H

#include <QDialog>
#include <QVariant>	//for QMap
#include <QMap>
QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

class ChangeAddDialog : public QDialog	//very clunky, heavy refactoring needed
{	Q_OBJECT
	enum State { Unknown=0, Alter=1, Add=2 };
public:
	ChangeAddDialog(const QList<int> &checked, QWidget *parent = 0);
	ChangeAddDialog(const QString &defaultType, QWidget *parent = 0);

	State getState() const { return state; }
public slots:
	virtual void accept() Q_DECL_OVERRIDE;
private:
	State state;
	QCheckBox *enableDBEdit;

	QList<QList<QWidget*> > contents;	//all relevant widgets (to get data from)
	QList<QWidget*> createEntryTitle(const QString &name, int index) const;
	QList<QWidget*> createColorCombo(const QList<QVariant> &parameters) const;
	QList<QWidget*> createNumberEditor(const QString &type, const QList<QVariant> &parameters, bool setDefault) const;

	QList<QList<QVariant> > getResults();
	QMap<int, int> indexMapping;	//(visualIndex, oldIndex) - for name change, etc.
signals:
	void alterColumnsRequested(bool isDBchanged, const QList<QList<QVariant> > &newHeaders);
	//newHeaders: oldIdx, if(isDBchanged)(newIdx,name), limits/colors
	void addColumnRequested(const QList<QVariant> &record);
	//record: newIdx, name, isMark, type, limits/colors, default
};

#endif // CHANGEADDDIALOG_H

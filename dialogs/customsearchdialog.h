#ifndef CUSTOMSEARCHDIALOG_H
#define CUSTOMSEARCHDIALOG_H

#include <QDialog>
#include <QVariant>	//for QMap
#include <QMap>
QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

class CustomSearchDialog : public QDialog
{	Q_OBJECT
public:
	CustomSearchDialog(QWidget *parent = 0);

	QMap<QString, QPair<QVariant,QVariant> > getSearchQuery() const;
//public slots:
//	virtual void accept() Q_DECL_OVERRIDE;
private:
	QHBoxLayout* createEntry(const QString &name, const QList<QVariant> &parameters) const;
};

#endif // CUSTOMSEARCHDIALOG_H

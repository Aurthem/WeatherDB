#ifndef DBMODEL_H
#define DBMODEL_H

#include <QSqlTableModel>

class DBModel: public QSqlTableModel
{	Q_OBJECT
public:
	explicit DBModel(QObject *parent = 0,QSqlDatabase db = QSqlDatabase());
	QVariant data(const QModelIndex &idx, int role) const Q_DECL_OVERRIDE;
};

#endif // DBMODEL_H

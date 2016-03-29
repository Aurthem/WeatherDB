//#include <QtWidgets>
#include <QMessageBox>

#include "dbmodel.h"
#include "database.h"

DBModel::DBModel(QObject *parent,QSqlDatabase db)
	: QSqlTableModel(parent,db)
{
	if (!database().isOpen()) {
		QMessageBox::critical(0, tr("Database is closed"),
			tr("Could not establish a connection"), QMessageBox::Cancel);
		return;
	}

	this->setTable("weather");
	this->setEditStrategy(QSqlTableModel::OnManualSubmit);
	this->select();
}
QVariant DBModel::data(const QModelIndex &idx, int role) const {
	if (role == Qt::UserRole && isDirty(idx))
//		return QBrush(QColor(Qt::yellow));
		return QString();	//special type for delegate to handle
	if(role==Qt::BackgroundRole) {
		QColor tmp_color=Database::getInstance().getMarkColor(idx);
		if(tmp_color.isValid()) return QBrush(tmp_color);
		return QBrush();
	}
//	if(role==Qt::DisplayRole) {	//checked SQLite version, don't use normally
//		QSqlQuery tmp("SELECT sqlite_version()",database());
//		while(tmp.next()) {
//			return tmp.value(0);
//		}
//	}
	return QSqlTableModel::data(idx, role);
}

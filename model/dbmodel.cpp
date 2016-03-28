//#include <QtWidgets>
#include <QMessageBox>

#include "dbmodel.h"
#include "database.h"

DBModel::DBModel(QObject *parent,QSqlDatabase db)
	: QSqlTableModel(parent,db)
{
//	QSqlDatabase::removeDatabase(db.connectionName());

//	database = QSqlDatabase::addDatabase("QSQLITE","testConnection");
//	database.setDatabaseName("test.db");

	if (!database().isOpen()) {
		QMessageBox::critical(0, tr("База данных закрыта"),
			tr("Не получилось установить соединение"), QMessageBox::Cancel);
		return;
	}

//	model = new QSqlTableModel(this);
//	QSqlTableModel(parent,db);

	this->setTable("weather");
	this->setEditStrategy(QSqlTableModel::OnManualSubmit);

	this->select();
//	for(int idx=0; idx<columnCount(); ++idx) {	//space for checkboxes
//		setHeaderData(idx,Qt::Horizontal,QString("     ").append(headerData(idx,Qt::Horizontal).toString()));
//	}

/*	QString inifile("settings.ini");
	QSettings myini(inifile,QSettings::IniFormat);
	myini.clear();
	myini.beginGroup("DB");

	QList<QList<QVariant> > tmpVars;
	tmpVars.append({"year",1,0});
	tmpVars.append({"month",2,1.1});
	myini.beginWriteArray("fields");
	for(int idx=0;idx<tmpVars.size();++idx) {
		myini.setArrayIndex(idx);
		myini.setValue("field",tmpVars.value(idx));
	}
//	QList<QString> tmplist(
//			{"year","month","day","pressure","humidity","wind","coeff"});
//	myini.beginWriteArray("fields");
//	for(int idx=0; idx<tmplist.size(); ++idx) {
//		myini.setArrayIndex(idx);
//		myini.setValue(tmplist.at(idx),idx);
//	}
	myini.endArray();
	myini.endGroup();
//	myini.setValue("fields",QVariant());

	int tmp_size=myini.beginReadArray("DB/fields");
//	QList<QString> tmp_list;
//	for(int idx=0; idx<tmp_size; ++idx) {
//		myini.setArrayIndex(idx);
//		tmp_list.append(myini.value(tmplist.value(idx)).toString());
//	}
	myini.setArrayIndex(1);
	QList<QVariant> tmp_list=myini.value("field").toList();
	myini.endArray();
	QMessageBox *qmb=new QMessageBox(QMessageBox::Information,"Information",QString::number(tmp_list.value(2).toDouble()));
	qmb->show();	//.canConvert(QMetaType::Int)?"true":"false"
	*/
}
QVariant DBModel::data(const QModelIndex &idx, int role) const {
	if (role == Qt::UserRole && isDirty(idx))
//			return QBrush(QColor(Qt::yellow));
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

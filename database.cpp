//#include <QtWidgets>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QItemSelectionRange>
#include <QCoreApplication>

//#include <QtSql>
#include <QSqlQuery>
#include <QSqlRecord>

//#include <algorithm>	//for sorting

#include "database.h"
#include "model/dbmodel.h"

//Database* Database::instance=0;
Database::Database(QObject *parent) : QObject(parent), model(0), marks(0) {
	database = QSqlDatabase::addDatabase("QSQLITE");	//using default connection
	setupDatabase(QCoreApplication::applicationDirPath()+"/test.db");

	model=new DBModel(0,database);

	marks=new QSqlTableModel(0,database);
	marks->setEditStrategy(QSqlTableModel::OnManualSubmit);
	marks->setTable("marks");
	marks->select();

	settings.refreshDatabase(model);
}

Database::~Database() {
	delete marks;
	delete model;
	database.close();
	QSqlDatabase::removeDatabase(database.connectionName());	//connection is still in use?
}
void Database::finalize(void) {
	settings.write();
	marks->submitAll();
}

void Database::setupDatabase(const QString &fileName) {
	if(database.isOpen()) database.close();
	if(fileName.isEmpty() || !fileName.compare(database.databaseName())) return;
	database.setDatabaseName(fileName);

	if (!database.open()) {
		QMessageBox::critical(0, tr("Database could not be opened"),
			tr("Failed to establish a connection to the database"), QMessageBox::Cancel);
		exit(2);
	}
	QSqlQuery query(database);
//	query.exec("drop table weather");
//	query.exec("drop table marks");
	if(!database.tables().contains(QLatin1String("weather"))) {
		query.exec(QString("create table weather (").append(settings.db_create["weather"]).append(")"));
		query.exec("insert into weather values("
							 "null, 0, 1981, 11, 27, 755, 45, 10, 0.7)");
		query.exec("insert into weather values("
							 "null, 1, 1986, 08, 13, 765, 83, 25, 2.7)");
		query.exec("insert into weather values("
							 "null, 0, 2012, 03, 05, 771, 25, 17, 1.3)");
	}
	if(!database.tables().contains(QLatin1String("marks"))) {
		query.exec(QString("create table marks (").append(settings.db_create["marks"]).append(")"));
		query.exec("insert into marks values("
							 "null, 1, 2,5, 'FF0000')");
		query.exec("insert into marks values("
							 "null, 2, 4,4, '3300FF00')");
		query.exec("insert into marks values("
							 "null, 3, 1,2, '550000FF')");
	}
	if(model) { model->setTable("weather"); model->select(); }
	if(marks) { marks->setTable("marks"); marks->select(); }
}

Database::Settings::Settings() : myini(new QSettings("settings.ini",QSettings::IniFormat)) {
	myini->setIniCodec("UTF-8");
	int tmp_size=myini->beginReadArray("DB/fields");
	if(tmp_size<1) {
		database["id"]				={0,QString("integer primary key"),0};
		database["Метка"]			={1,QString("integer"),1,"#ffffff","#555555"};
		database["Год"]				={2,QString("integer"),0,1000,3000};
		database["Месяц"]			={3,QString("integer"),0,1,12};
		database["День"]			={4,QString("integer"),0,1,31};
		database["Давление"]	={5,QString("integer"),0,500,999};
		database["Влажность"]	={6,QString("integer"),0,0,100};
		database["Ветер"]			={7,QString("integer"),0,0,99};
		database["Коэфф"]			={8,QString("real"),0,-9.9,9.9};
	}
	for(int idx=0; idx<tmp_size; ++idx) {
		myini->setArrayIndex(idx);
		QList<QVariant> tmp=myini->value("field").toList();
		database[tmp.value(0).toString()]=tmp.mid(1);
	}
	myini->endArray();

	tmp_size=myini->beginReadArray("DB/create");
	if(tmp_size<2) {
		db_create["weather"]="id integer primary key, "
			"Метка integer, Год integer, Месяц integer, День integer, Давление integer, "
			"Влажность integer, Ветер integer, Коэфф real";
		db_create["marks"]="id integer primary key, "
			"other_id int, row_top int, row_bottom int, color varchar(8)";
	} else {
		myini->setArrayIndex(0);
		db_create["weather"]=myini->value("weather").toString();
		myini->setArrayIndex(1);
		db_create["marks"]=myini->value("marks").toString();
	}
	myini->endArray();
}
void Database::Settings::write() const {
	myini->beginGroup("DB");
	myini->remove("");	//clear this group
	myini->beginWriteArray("fields");
	int idx=0;
	for(QMap<QString,QList<QVariant> >::const_iterator itr=database.begin();
			itr!=database.end();++itr) {
		QList<QVariant> tmp=itr.value();
		tmp.prepend(QVariant(itr.key()));
		myini->setArrayIndex(idx);
		myini->setValue("field",tmp);
		++idx;
	}
	myini->endArray();

	myini->beginWriteArray("create");
	myini->setArrayIndex(0);
	myini->setValue("weather",db_create["weather"]);
	myini->setArrayIndex(1);
	myini->setValue("marks",db_create["marks"]);
	myini->endArray();

	myini->endGroup();
}

void Database::Settings::refreshDatabase(const DBModel *sourceModel) {
	QList<QString> newHeaders;
	for(int idx=0; idx<sourceModel->columnCount(); ++idx) {
		QString columnName=sourceModel->headerData(idx,Qt::Horizontal,Qt::DisplayRole).toString();
		if(database.contains(columnName) && !database[columnName].isEmpty()) {
			database[columnName][0]=idx;
		} else {
			database[columnName]={QVariant(idx),QString("integer"),0};	//shouldn't go here if used correctly
		}
		newHeaders.append(columnName);
	}
	for(QMap<QString,QList<QVariant> >::iterator itr=database.begin(); itr!=database.end();) {
		if(newHeaders.contains(itr.key())) ++itr;
		else itr=database.erase(itr);
	}	//in .ini file the last record is not erased, will be hanging there
}

QColor Database::getMarkColor(const QModelIndex &idx) const {
	//faster than query method
	for(int index=0; index<marks->rowCount(); ++index) {
		QSqlRecord tmp=marks->record(index);
		if(	tmp.value(1)==model->data(model->index(idx.row(),0),Qt::DisplayRole)
		&&	tmp.value(2)<=idx.column() && tmp.value(3)>=idx.column()) {
			return QColor(tmp.value(4).toString().prepend("#"));
		}
	}
	return QColor();
}
void Database::setMarkColor(const QItemSelection &rangesSelected, const QColor &color) {
	QString colorString;
	if(color.isValid()) {	//empty colorString will be used to erase marks
		if(color.alpha()!=0 && color.name(QColor::HexRgb).compare("#ffffff")) {
			if(color.alpha()==255) colorString=color.name(QColor::HexRgb).mid(1);
			else colorString=color.name(QColor::HexArgb).mid(1);
		}
	}
	QSqlRecord recordWrite=marks->record();
	int markCount=marks->rowCount();	//insertMarkRecord() changes rowCount();
	for(const QItemSelectionRange &range: rangesSelected) {
		//HorizontalProxyModel specific implementation
		for(int column=range.left();column<=range.right();++column) {
			QVariant tmpID=range.model()->index(0,column).data(Qt::EditRole);
			if(!tmpID.isNull()) {
				recordWrite.setValue(1,tmpID);
				recordWrite.setValue(2,range.top());	//hidden rows - solved by splitting selection in the view
				recordWrite.setValue(3,range.bottom());
				recordWrite.setValue(4,colorString);
				insertMarkRecord(recordWrite,markCount);
			}
		}
	}

	sanitizeMarks();
	marks->submitAll();
//	for(int index=0;index<marks->rowCount();++index) { qDebug()<<marks->record(index); }
}
void Database::insertMarkRecord(const QSqlRecord &recordWrite, int markRowCount) {
	for(int index=0;index<markRowCount;++index) {
		if(marks->data(marks->index(index,1))!=recordWrite.value(1)) continue;
		int row_top=marks->data(marks->index(index,2)).toInt(),
				row_bottom=marks->data(marks->index(index,3)).toInt();
		int new_row_top=recordWrite.value(2).toInt(),
				new_row_bottom=recordWrite.value(3).toInt();
		if(row_top<=new_row_bottom && row_bottom>=new_row_top) {
			if(row_top<new_row_top) {
				if(row_bottom>new_row_bottom) {
//					qDebug()<<index<<"inside, need to split";
					QSqlRecord bottomRecord=marks->record();
					bottomRecord.setValue(1,recordWrite.value(1));
					bottomRecord.setValue(2,recordWrite.value(3).toInt()+1);
					bottomRecord.setValue(3,marks->data(marks->index(index,3)));
					bottomRecord.setValue(4,marks->data(marks->index(index,4)));
					marks->setData(marks->index(index,3),new_row_top-1);
					marks->insertRecord(-1,bottomRecord);
				} else {
//					qDebug()<<index<<"trim bottom";
					marks->setData(marks->index(index,3),new_row_top-1);
				}
			} else {
				if(row_bottom<=new_row_bottom) {
//					qDebug()<<index<<"inside, need to remove";
					marks->removeRow(index);	//not submitted yet, so indexes aren't shifted
				} else {
//					qDebug()<<index<<"trim top";
					marks->setData(marks->index(index,2),new_row_bottom+1);
				}
			}
		}// else { qDebug()<<"no intersection"; }
	}
	if(!recordWrite.value(4).isNull())
		marks->insertRecord(-1,recordWrite);	//insert record at the end
}

void Database::handleRowsRemoved(const QSet<int> &rowIds) {
	for(int index=0; index<marks->rowCount();++index) {
		if(rowIds.contains(marks->data(marks->index(index,1)).toInt())) {
			marks->removeRow(index);
		}
	}
	marks->submitAll();
//	for(int index=0;index<marks->rowCount();++index) { qDebug()<<marks->record(index); }
}
void Database::handleColumnsRemoved(const QList<int> &columns) {
	int markCount=marks->rowCount();	//rowCount changes after insertions
	for(int index=0; index<markCount; ++index) {
		int row_top=marks->data(marks->index(index,2)).toInt(),
				row_bottom=marks->data(marks->index(index,3)).toInt();
		QList<int> interval;
		for(int row=row_top;row<=row_bottom;++row) {
			interval.append(row);
		}
		for(int idx=columns.size();--idx>=0;) {	//should be sorted, and it is - from CheckboxHeader (QMap is sorted by key)
			for(QList<int>::iterator itr=interval.begin();itr!=interval.end();) {
				if(*itr==columns.at(idx)) itr=interval.erase(itr);
				else {
					if(*itr>columns.at(idx)) --(*itr);
					++itr;
				}
			}
		}
		updateMarkRecord(interval,index);
	}

	sanitizeMarks();
	marks->submitAll();
//	for(int index=0;index<marks->rowCount();++index) { qDebug()<<marks->record(index); }
}

void Database::handleColumnsRearranged(const QList<int> &newOrder) {
	int markCount=marks->rowCount();	//rowCount changes after insertions
	for(int index=0; index<markCount; ++index) {
		int row_top=marks->data(marks->index(index,2)).toInt(),
				row_bottom=marks->data(marks->index(index,3)).toInt();
		QList<int> interval;
		int newIdx=0;
		for(int oldIdx: newOrder) {
			if(oldIdx>=row_top && oldIdx<=row_bottom) interval.append(newIdx);
			++newIdx;
		}
//		std::sort(interval.begin(),interval.end());	//already sorted by construction
		updateMarkRecord(interval,index);
	}

	sanitizeMarks();
	marks->submitAll();
//	for(int index=0;index<marks->rowCount();++index) { qDebug()<<marks->record(index); }
}

void Database::updateMarkRecord(const QList<int> &interval, int index) {
	if(interval.isEmpty()) marks->removeRow(index);
	else {
		int row_top=interval.front();
		int row_bottom=row_top;
		bool first_record=true, finalWrite=false;
		QList<int>::const_iterator itr=interval.constBegin();
		while(itr!=interval.constEnd() || !finalWrite) {
			if(itr==interval.constEnd()) finalWrite=true;
			if(*itr-row_bottom>1 || finalWrite) {
				if(first_record) {
					marks->setData(marks->index(index,2),row_top);
					marks->setData(marks->index(index,3),row_bottom);
					first_record=false;
				} else {
					QSqlRecord tmpRecord=marks->record();
					tmpRecord.setValue(1,marks->data(marks->index(index,1)));
					tmpRecord.setValue(2,row_top);
					tmpRecord.setValue(3,row_bottom);
					tmpRecord.setValue(4,marks->data(marks->index(index,4)));
					marks->insertRecord(-1,tmpRecord);
				}
				row_top=row_bottom=*itr;
			} else row_bottom=*itr;
			if(!finalWrite) ++itr;
		}
	}
}

void Database::sanitizeMarks() {
	QList<int> marksToRemove;	//rows might not be submitted, so they could be removed at the spot, thus delayed removal
	QSet<int> skipMarks;	//a set to skip matched marks (order doesn't matter)
	int markCount=marks->rowCount();
	for(int index=0; index<markCount; ++index) {
		if(!marks->headerData(index,Qt::Vertical).toString().compare("!"))
			skipMarks.insert(index);	//to avoid unnecessary submitAll() calls, skip all marked for removal rows
	}
	for(int index=0; index<markCount; ++index) {
		if(skipMarks.remove(index)) continue;	//returns true if contained and removed
		int currentID=marks->data(marks->index(index,1)).toInt();
		QString currentColor=marks->data(marks->index(index,4)).toString();
		QList<int> matchingMarks;	//order is important here
		for(int otherIdx=index+1; otherIdx<markCount; ++otherIdx) {	//longer than loading it all, but less memory needed
			if(skipMarks.contains(otherIdx)) continue;	//might be faster than string compare with matching ids
			if(currentID==marks->data(marks->index(otherIdx,1)).toInt())
				if(!currentColor.compare(marks->data(marks->index(otherIdx,4)).toString()))
					matchingMarks.append(otherIdx);
		}
//		if(matchingMarks.isEmpty()) continue;	//check the only record anyway (in case range is empty)
		skipMarks.unite(matchingMarks.toSet());
		matchingMarks.prepend(index);
		QSet<int> unsortedInterval;	//set for unique entries
		for(int idx : matchingMarks) {
			int row_top=marks->data(marks->index(idx,2)).toInt(),
					row_bottom=marks->data(marks->index(idx,3)).toInt();
			for(int row=row_top;row<=row_bottom;++row) {
				unsortedInterval.insert(row);
			}
		}
		if(unsortedInterval.isEmpty()) {
			marksToRemove.append(matchingMarks);
			continue;
		}
		QList<int> interval=unsortedInterval.toList();
		std::sort(interval.begin(),interval.end());	//sorts in ascending order

		//adapted from updateMarkRecord();
		int row_top=interval.front();
		int row_bottom=row_top;
		int currentMatched=0;
		bool finalWrite=false;
		QList<int>::const_iterator itr=interval.constBegin();
		while(itr!=interval.constEnd() || !finalWrite) {
			if(itr==interval.constEnd()) finalWrite=true;
			if(*itr-row_bottom>1 || finalWrite) {
				marks->setData(marks->index(matchingMarks.at(currentMatched),2),row_top);
				marks->setData(marks->index(matchingMarks.at(currentMatched),3),row_bottom);
				row_top=row_bottom=*itr;
				++currentMatched;
			} else row_bottom=*itr;
			if(!finalWrite) ++itr;
		}
		if(currentMatched<matchingMarks.size()) {
			marksToRemove.append(matchingMarks.mid(currentMatched));
		}
	}
	//iterate from the end, sorted, to avoid numbering issues on removal
	std::sort(marksToRemove.begin(),marksToRemove.end());
	for(QList<int>::const_iterator itr=marksToRemove.end(); itr!=marksToRemove.begin();) {
		--itr;
		marks->removeRow(*itr);
	}
	Q_ASSERT(skipMarks.isEmpty());
}

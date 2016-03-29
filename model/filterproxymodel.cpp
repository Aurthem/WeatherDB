//#include <QtWidgets>
#include <QDate>

#include "filterproxymodel.h"

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
	if(sourceParent.isValid()) return false;
	if(sourceRow==0) return true;
	if(sourceRow>=sourceModel()->rowCount()) return false;
	if(sourceModel()->headerData(sourceRow,Qt::Vertical)=="*") return true;	//show not submitted rows
//	if(filters.empty()) return false;	//hide until filtered - when there's a lot of records, could be useful
	for(QMap<int, QPair<QVariant,QVariant> >::const_iterator itr=filters.begin();
			itr!=filters.end();++itr) {
		QModelIndex source_index=sourceModel()->index(sourceRow,itr.key());
		if(!source_index.isValid()) return true;	//itr.key() points to not existing column
		QVariant tmp=sourceModel()->data(source_index,Qt::EditRole);
		if(!tmp.isNull()) {	//ignore empty
			if(lessThanVariants(tmp,itr.value().first) || lessThanVariants(itr.value().second,tmp)) return false;
		}
	}
	return true;
}
bool FilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
	if(left.row()==0) return true;
	if(right.row()==0) return false;
	if(sourceModel()->headerData(left.row(),Qt::Vertical)=="*") {	//compare not submitted rows with each other
		if(sourceModel()->headerData(right.row(),Qt::Vertical)!="*") return true;
	} else if(sourceModel()->headerData(right.row(),Qt::Vertical)=="*") return false;

	for(int compared_column=left.column(); compared_column<sourceModel()->columnCount();++compared_column) {
		QVariant leftData = (left.model() ?
			left.model()->data(left.model()->index(left.row(),compared_column), Qt::EditRole) : QVariant());
		QVariant rightData = (right.model() ?
			right.model()->data(right.model()->index(right.row(),compared_column), Qt::EditRole) : QVariant());
		if(lessThanVariants(leftData,rightData)) return true;
		if(lessThanVariants(rightData,leftData)) return false;
	}
	return lessThanVariants((left.model() ?
		left.model()->data(left.model()->index(left.row(),0), Qt::EditRole) : QVariant()),
													(right.model() ?
		right.model()->data(right.model()->index(right.row(),0), Qt::EditRole) : QVariant())
													);
//	return false;
}
bool FilterProxyModel::lessThanVariants(const QVariant &left, const QVariant &right) const {
	if (left.userType() == QVariant::Invalid) return false;
	if (right.userType() == QVariant::Invalid) return true;
	switch (left.userType()) {
	case QVariant::Int:				return left.toInt() < right.toInt();
	case QVariant::UInt:			return left.toUInt() < right.toUInt();
	case QVariant::LongLong:	return left.toLongLong() < right.toLongLong();
	case QVariant::ULongLong:	return left.toULongLong() < right.toULongLong();
	case QMetaType::Float:		return left.toFloat() < right.toFloat();
	case QVariant::Double:		return left.toDouble() < right.toDouble();
	case QVariant::Char:			return left.toChar() < right.toChar();
	case QVariant::Date:			return left.toDate() < right.toDate();
	case QVariant::Time:			return left.toTime() < right.toTime();
	case QVariant::DateTime:	return left.toDateTime() < right.toDateTime();
	case QVariant::String:
	default:	return left.toString().localeAwareCompare(right.toString()) < 0;
	}
}

void FilterProxyModel::compileFilter(const QMap<int, QVariant> &source) {
	if(source.empty()) filters.clear();
	for(QMap<int, QVariant>::const_iterator itr=source.begin();
			itr!=source.end();++itr) {
		filters[itr.key()]=QPair<QVariant,QVariant>(itr.value(),itr.value());
	}
	invalidateFilter();
//	invalidate();
}
void FilterProxyModel::compileNamedFilters(const QMap<QString,QPair<QVariant,QVariant> > &source) {
	if(source.empty()) filters.clear();
	for(QMap<QString,QPair<QVariant,QVariant> >::const_iterator itr=source.constBegin(); itr!=source.constEnd(); ++itr) {
		int idx=findIndexByName(itr.key());
		if(idx>=0) {
			if(!itr.value().first.isNull() && !itr.value().second.isNull())
				filters[idx]=itr.value();
		}
	}
	invalidateFilter();
}
int FilterProxyModel::findIndexByName(const QString &name) const {
	for(int idx=0; idx<columnCount(); ++idx) {
		if(!headerData(idx,Qt::Horizontal,Qt::DisplayRole).toString().compare(name)) return idx;
	}
	return -1;	//didn't find
}

//bool FilterProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const {
//	if(sourceParent.isValid()) return false;
//	if(sourceColumn==2) return false;
//	for(QMap<int, QPair<QVariant,QVariant> >::const_iterator itr=filters.begin();
//			itr!=filters.end();++itr) {
//		if(itr.key()>=rowCount()) return false;
//		QVariant tmp=data(index(itr.key(),sourceColumn));
//		if(tmp<itr.value().first || tmp>itr.value().second) return false;
//	}
//	return true;
//}

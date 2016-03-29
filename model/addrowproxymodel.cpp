//#include <QtWidgets>
#include <QLocale>

#include <QItemSelection>
#include "addrowproxymodel.h"
#include "database.h"

AddRowProxyModel::AddRowProxyModel(QObject *parent)
	: QAbstractProxyModel(parent) {
}

int AddRowProxyModel::columnCount(const QModelIndex &parent) const {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->columnCount(mapToSource(parent));
//	return sourceModel() ? sourceModel()->columnCount() : 0;
}
int AddRowProxyModel::rowCount(const QModelIndex &parent) const {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->rowCount(mapToSource(parent))+1;
//	return sourceModel() ? sourceModel()->rowCount()+1 : 0;
}
QModelIndex AddRowProxyModel::index(int row, int column, const QModelIndex &parent) const {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
//	const QModelIndex sourceParent = mapToSource(parent);
//	const QModelIndex sourceIndex = sourceModel()->index(row, column, sourceParent);
//	return mapFromSource(sourceIndex);
		return createIndex(row,column,	//produce index this way to keep workaround useful
			sourceModel()?sourceModel()->index(row,column,mapToSource(parent)).internalPointer():(void*)0);
}
QModelIndex AddRowProxyModel::parent(const QModelIndex &child) const {
	Q_UNUSED(child)
	return QModelIndex();
//	Q_ASSERT(child.isValid() ? child.model() == this : true);
//	const QModelIndex sourceIndex = mapToSource(child);
//	const QModelIndex sourceParent = sourceIndex.parent();
//	return mapFromSource(sourceParent);
}
QModelIndex AddRowProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
	if(!sourceModel()) return QModelIndex();
	if(!sourceIndex.isValid()) {
		if(	sourceIndex.row()==-1 && sourceIndex.column()>-1	//workaround cont.
		&&	sourceIndex.internalId()==static_cast<quintptr>(-1)) {
			return index(0,sourceIndex.column());
		}
		return QModelIndex();
	}
	Q_ASSERT(sourceIndex.model() == sourceModel());
	return createIndex(sourceIndex.row()+1, sourceIndex.column(), sourceIndex.internalPointer());
}
QModelIndex AddRowProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
	if(!proxyIndex.isValid() || !sourceModel()) return QModelIndex();
	Q_ASSERT(proxyIndex.model() == this);
	if(proxyIndex.row()==0)	//workaround, should work
		return createIndex(-1,proxyIndex.column(),static_cast<quintptr>(-1));
//	return sourceModel()->createIndex(proxyIndex.row()-1, proxyIndex.column(), proxyIndex.internalPointer());	//protected!
	//hacks:
	const AbstractItemModelExposer *hackedModel = static_cast<const AbstractItemModelExposer*>(sourceModel());
	return hackedModel->createIndexHack(proxyIndex.row()-1, proxyIndex.column(), proxyIndex.internalId());
}

bool AddRowProxyModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) {
	if(row==0) return false;
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->dropMimeData(data, action, row-1, column, mapToSource(parent));
}
QModelIndex AddRowProxyModel::sibling(int row, int column, const QModelIndex &idx) const {
	if(row==0) return QModelIndex();
	return mapFromSource(sourceModel()->sibling(row-1, column, mapToSource(idx)));
}
QItemSelection AddRowProxyModel::mapSelectionFromSource(const QItemSelection & selection) const {
	QItemSelection proxySelection;
	if(!sourceModel()) return proxySelection;

	QItemSelection::const_iterator itr = selection.constBegin();
	const QItemSelection::const_iterator end = selection.constEnd();
	for ( ; itr!=end; ++itr) {
		Q_ASSERT(itr->model() == sourceModel());
		const QItemSelectionRange range(mapFromSource(itr->topLeft()), mapFromSource(itr->bottomRight()));
		proxySelection.append(range);
	}

	return proxySelection;
}
QItemSelection AddRowProxyModel::mapSelectionToSource(const QItemSelection & selection) const {
	QItemSelection sourceSelection;
	if(!sourceModel()) return sourceSelection;

	QItemSelection::const_iterator itr = selection.constBegin();
	const QItemSelection::const_iterator end = selection.constEnd();
	for ( ; itr!=end; ++itr) {
		Q_ASSERT(itr->model() == this);
		const QItemSelectionRange range(mapToSource(itr->topLeft()), mapToSource(itr->bottomRight()));
		sourceSelection.append(range);
	}

	return sourceSelection;
}
QModelIndexList	AddRowProxyModel::match(const QModelIndex & start, int role, const QVariant & value, int hits, Qt::MatchFlags flags) const {
	Q_ASSERT(start.isValid() ? start.model() == this : true);
	if(!sourceModel()) return QModelIndexList();

	const QModelIndexList sourceList = sourceModel()->match(mapToSource(start), role, value, hits, flags);
	QModelIndexList::const_iterator itr = sourceList.constBegin();
	const QModelIndexList::const_iterator end = sourceList.constEnd();
	QModelIndexList proxyList;
	for ( ; itr!=end; ++itr)
		proxyList.append(mapFromSource(*itr));
	return proxyList;
}

bool AddRowProxyModel::insertColumns(int column, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->insertColumns(column, count, mapToSource(parent));
}
bool AddRowProxyModel::insertRows(int row, int count, const QModelIndex & parent) {
	if(row==0) return false;
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->insertRows(row-1,count,parent);
}
bool AddRowProxyModel::removeColumns(int column, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->removeColumns(column, count, mapToSource(parent));
}
bool AddRowProxyModel::removeRows(int row, int count, const QModelIndex & parent) {
	if(row==0) return false;
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->removeRows(row-1,count,parent);
}

QVariant AddRowProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if(orientation==Qt::Vertical) {
		if(section==0) {
			if(role==Qt::DisplayRole)
				return QString(tr("New"));
			return QVariant();
		}
		return sourceModel()->headerData(section-1,orientation,role);
	}
	return sourceModel()->headerData(section,orientation,role);
}
QVariant AddRowProxyModel::data(const QModelIndex &index, int role) const {
//	if(!index.isValid()) return QVariant();	//not needed, probably
	if(index.row()==0) {
		if(role==Qt::DisplayRole || role==Qt::EditRole) {
			return record.value(index.column());	//should work like this
		}
		return QVariant();
	}
	return sourceModel()->data(mapToSource(index),role);
//	return sourceModel() ? sourceModel()->data(mapToSource(index),role) : QVariant();
}
bool AddRowProxyModel::setData(const QModelIndex &index, const QVariant &value, int role) {
//	if(!index.isValid()) return false;	//not needed, probably
	if(index.row()==0) {
		if(role==Qt::DisplayRole || role==Qt::EditRole) {
//			sourceModel()->insertRow(sourceModel()->rowCount());
			if(!Database::getInstance().settings.database.value(
							headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString()
							).value(1).toString().compare("real")
					) {
				bool isSuccessful;
				QLocale defaultLocale;
				record[index.column()]=defaultLocale.toDouble(value.toString(),&isSuccessful);
				if(!isSuccessful) record[index.column()]=value;
			} else
				record[index.column()]=value;	//localeConvert(value);
			emit dataChanged(index,index);
			return true;
		}
		return false;
	}
	return sourceModel()->setData(mapToSource(index),value,role);
//	return sourceModel() ? sourceModel()->setData(mapToSource(index),value,role) : false;
}
Qt::ItemFlags	AddRowProxyModel::flags(const QModelIndex &index) const {
//	if(!index.isValid()) return Qt::NoItemFlags;	//not needed, probably
	if(index.row()==0) {
		return (Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
	}
	return sourceModel()->flags(mapToSource(index));
//	return sourceModel() ? sourceModel()->flags(mapToSource(index)) : Qt::NoItemFlags;
}

void AddRowProxyModel::setSourceModel(QAbstractItemModel * newSourceModel) {
	beginResetModel();

	if (sourceModel()) {
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeInserted,
							 this,&AddRowProxyModel::sourceRowsAboutToBeInserted);
		disconnect(sourceModel(),&QAbstractItemModel::rowsInserted,
							 this,&AddRowProxyModel::sourceRowsInserted);
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeRemoved,
							 this,&AddRowProxyModel::sourceRowsAboutToBeRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsRemoved,
							 this,&AddRowProxyModel::sourceRowsRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeMoved,
							 this,&AddRowProxyModel::sourceRowsAboutToBeMoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsMoved,
							 this,&AddRowProxyModel::sourceRowsMoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeInserted,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeInserted);
		disconnect(sourceModel(),&QAbstractItemModel::columnsInserted,
							 this,&AddRowProxyModel::sourceColumnsInserted);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeRemoved,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsRemoved,
							 this,&AddRowProxyModel::sourceColumnsRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeMoved,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeMoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsMoved,
							 this,&AddRowProxyModel::sourceColumnsMoved);
		disconnect(sourceModel(),&QAbstractItemModel::modelAboutToBeReset,
							 this,&AddRowProxyModel::sourceModelAboutToBeReset);
		disconnect(sourceModel(),&QAbstractItemModel::modelReset,
							 this,&AddRowProxyModel::sourceModelReset);
		disconnect(sourceModel(),&QAbstractItemModel::dataChanged,
							 this,&AddRowProxyModel::sourceDataChanged);
		disconnect(sourceModel(),&QAbstractItemModel::headerDataChanged,
							 this,&AddRowProxyModel::sourceHeaderDataChanged);
		disconnect(sourceModel(),&QAbstractItemModel::layoutAboutToBeChanged,
							 this,&AddRowProxyModel::sourceLayoutAboutToBeChanged);
		disconnect(sourceModel(),&QAbstractItemModel::layoutChanged,
							 this,&AddRowProxyModel::sourceLayoutChanged);
	}

	QAbstractProxyModel::setSourceModel(newSourceModel);

	if (sourceModel()) {
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeInserted,
							 this,&AddRowProxyModel::sourceRowsAboutToBeInserted);
		connect(sourceModel(),&QAbstractItemModel::rowsInserted,
							 this,&AddRowProxyModel::sourceRowsInserted);
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeRemoved,
							 this,&AddRowProxyModel::sourceRowsAboutToBeRemoved);
		connect(sourceModel(),&QAbstractItemModel::rowsRemoved,
							 this,&AddRowProxyModel::sourceRowsRemoved);
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeMoved,
							 this,&AddRowProxyModel::sourceRowsAboutToBeMoved);
		connect(sourceModel(),&QAbstractItemModel::rowsMoved,
							 this,&AddRowProxyModel::sourceRowsMoved);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeInserted,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeInserted);
		connect(sourceModel(),&QAbstractItemModel::columnsInserted,
							 this,&AddRowProxyModel::sourceColumnsInserted);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeRemoved,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeRemoved);
		connect(sourceModel(),&QAbstractItemModel::columnsRemoved,
							 this,&AddRowProxyModel::sourceColumnsRemoved);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeMoved,
							 this,&AddRowProxyModel::sourceColumnsAboutToBeMoved);
		connect(sourceModel(),&QAbstractItemModel::columnsMoved,
							 this,&AddRowProxyModel::sourceColumnsMoved);
		connect(sourceModel(),&QAbstractItemModel::modelAboutToBeReset,
							 this,&AddRowProxyModel::sourceModelAboutToBeReset);
		connect(sourceModel(),&QAbstractItemModel::modelReset,
							 this,&AddRowProxyModel::sourceModelReset);
		connect(sourceModel(),&QAbstractItemModel::dataChanged,
							 this,&AddRowProxyModel::sourceDataChanged);
		connect(sourceModel(),&QAbstractItemModel::headerDataChanged,
							 this,&AddRowProxyModel::sourceHeaderDataChanged);
		connect(sourceModel(),&QAbstractItemModel::layoutAboutToBeChanged,
							 this,&AddRowProxyModel::sourceLayoutAboutToBeChanged);
		connect(sourceModel(),&QAbstractItemModel::layoutChanged,
							 this,&AddRowProxyModel::sourceLayoutChanged);
	}

	endResetModel();
}
void AddRowProxyModel::sourceRowsAboutToBeInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginInsertRows(mapFromSource(parent),first+1,last+1);
}
void AddRowProxyModel::sourceRowsInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endInsertRows();
}
void AddRowProxyModel::sourceRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	beginMoveRows(mapFromSource(sourceParent), sourceStart+1, sourceEnd+1, mapFromSource(destinationParent), destinationRow+1);
}
void AddRowProxyModel::sourceRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	Q_UNUSED(sourceParent)
	Q_UNUSED(sourceStart)
	Q_UNUSED(sourceEnd)
	Q_UNUSED(destinationParent)
	Q_UNUSED(destinationRow)
	endMoveRows();
}
void AddRowProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginRemoveRows(mapFromSource(parent),first+1,last+1);
}
void AddRowProxyModel::sourceRowsRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endRemoveRows();
}
void AddRowProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginInsertColumns(mapFromSource(parent), first, last);
}
void AddRowProxyModel::sourceColumnsInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endInsertColumns();
}
void AddRowProxyModel::sourceColumnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn);
}
void AddRowProxyModel::sourceColumnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	Q_UNUSED(sourceParent)
	Q_UNUSED(sourceStart)
	Q_UNUSED(sourceEnd)
	Q_UNUSED(destinationParent)
	Q_UNUSED(destinationColumn)
	endMoveColumns();
}
void AddRowProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginRemoveColumns(mapFromSource(parent), first, last);
}
void AddRowProxyModel::sourceColumnsRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endRemoveColumns();
}

void AddRowProxyModel::sourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles) {
	Q_ASSERT(topLeft.isValid() ? topLeft.model() == sourceModel() : true);
	Q_ASSERT(bottomRight.isValid() ? bottomRight.model() == sourceModel() : true);
	emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}
void AddRowProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last) {
	if(orientation==Qt::Vertical)
		emit headerDataChanged(orientation,first+1,last+1);
	else
		emit headerDataChanged(orientation,first,last);
}

void AddRowProxyModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint) {
	for(const QPersistentModelIndex &proxyPersistentIndex: persistentIndexList()) {
		proxyIndexes << proxyPersistentIndex;
		Q_ASSERT(proxyPersistentIndex.isValid());
		const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
		Q_ASSERT(srcPersistentIndex.isValid());
		layoutChangePersistentIndexes << srcPersistentIndex;
	}

	QList<QPersistentModelIndex> parents;
	parents.reserve(sourceParents.size());
	for(const QPersistentModelIndex &parent: sourceParents) {
		if (!parent.isValid()) {
			parents << QPersistentModelIndex();
			continue;
		}
		const QModelIndex mappedParent = mapFromSource(parent);
		Q_ASSERT(mappedParent.isValid());
		parents << mappedParent;
	}
	layoutAboutToBeChanged(parents, hint);
}
void AddRowProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint) {
	for(int idx = 0; idx<proxyIndexes.size(); ++idx) {
		changePersistentIndex(proxyIndexes.at(idx), mapFromSource(layoutChangePersistentIndexes.at(idx)));
	}
	layoutChangePersistentIndexes.clear();
	proxyIndexes.clear();

	QList<QPersistentModelIndex> parents;
	parents.reserve(sourceParents.size());
	for(const QPersistentModelIndex &parent: sourceParents) {
		if (!parent.isValid()) {
			parents << QPersistentModelIndex();
			continue;
		}
		const QModelIndex mappedParent = mapFromSource(parent);
		Q_ASSERT(mappedParent.isValid());
		parents << mappedParent;
	}
	layoutChanged(parents, hint);
}
void AddRowProxyModel::sourceModelAboutToBeReset() {
	beginResetModel();
//	record.clear();
}
void AddRowProxyModel::sourceModelReset() { endResetModel(); }

QMap<int, QVariant> AddRowProxyModel::constructFilter(const QList<int> &checked) const {
	QMap<int, QVariant> result;
	for(int idx: checked) {
		if(record.contains(idx))
			result[idx]=record[idx];
	}
	return result;
}
QVariant AddRowProxyModel::localeConvert(const QVariant &source) const {
	QVariant result;
	QLocale localConv;
	bool success;
	result=localConv.toLongLong(source.toString(),&success);
	if(success) return result;
	result=localConv.toDouble(source.toString(),&success);
	if(success) return result;

	return source;
}

void AddRowProxyModel::submitRow() {
//	emit layoutAboutToBeChanged();

	insertRows(1,1);

	//setRecord is faster, but without model cast this is the solution
	for(int idx=1; idx<sourceModel()->columnCount(); ++idx) {	//don't set id
		setData(index(1,idx),record[idx],Qt::EditRole);
	}
//	emit layoutChanged();
//	emit dataChanged(createIndex(1,0),QModelIndex());
}

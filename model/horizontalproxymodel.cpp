//#include <QtWidgets>

#include <QItemSelection>
#include "horizontalproxymodel.h"
#include "database.h"

HorizontalProxyModel::HorizontalProxyModel(QObject *parent)
	: QAbstractProxyModel(parent), vertical(true) {
}

int HorizontalProxyModel::columnCount(const QModelIndex &parent) const {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->rowCount(mapToSource(parent));
//	return (sourceModel() ? sourceModel()->rowCount(mapToSource(parent)) : 0);
}
int HorizontalProxyModel::rowCount(const QModelIndex &parent) const {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->columnCount(mapToSource(parent));
//	return (sourceModel() ? sourceModel()->columnCount(mapToSource(parent)) : 0);
}
QModelIndex HorizontalProxyModel::index(int row, int column, const QModelIndex &parent) const {
//	return createIndex(row,column,
//		sourceModel() ? sourceModel()->index(column,row,mapToSource(parent)).internalPointer():(void*)0);
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	const QModelIndex sourceParent = mapToSource(parent);
	const QModelIndex sourceIndex = sourceModel()->index(column, row, sourceParent);
	return mapFromSource(sourceIndex);
}
QModelIndex HorizontalProxyModel::parent(const QModelIndex &child) const {
	Q_UNUSED(child)
	return QModelIndex();
//	Q_ASSERT(child.isValid() ? child.model() == this : true);
//	const QModelIndex sourceIndex = mapToSource(child);
//	const QModelIndex sourceParent = sourceIndex.parent();
//	return mapFromSource(sourceParent);
}
QModelIndex HorizontalProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
	if (!sourceModel() || !sourceIndex.isValid()) return QModelIndex();
	Q_ASSERT(sourceIndex.model() == sourceModel());
	return createIndex(sourceIndex.column(), sourceIndex.row(), sourceIndex.internalPointer());
}
QModelIndex HorizontalProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
	if (!sourceModel() || !proxyIndex.isValid()) return QModelIndex();
	Q_ASSERT(proxyIndex.model() == this);
//	return sourceModel()->createIndex(proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer());	//protected!
	//hacks:
	const AbstractItemModelExposer *hackedModel = static_cast<const AbstractItemModelExposer*>(sourceModel());
	return hackedModel->createIndexHack(proxyIndex.column(), proxyIndex.row(), proxyIndex.internalId());
}

bool HorizontalProxyModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->dropMimeData(data,action,column,row,mapToSource(parent));
}
QModelIndex HorizontalProxyModel::sibling(int row, int column, const QModelIndex &idx) const {
	return mapFromSource(sourceModel()->sibling(column, row, mapToSource(idx)));
}
QItemSelection HorizontalProxyModel::mapSelectionFromSource(const QItemSelection & selection) const {
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
QItemSelection HorizontalProxyModel::mapSelectionToSource(const QItemSelection & selection) const {
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
QModelIndexList	HorizontalProxyModel::match(const QModelIndex & start, int role, const QVariant & value, int hits, Qt::MatchFlags flags) const {
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

bool HorizontalProxyModel::insertColumns(int column, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->insertRows(column, count, mapToSource(parent));
}
bool HorizontalProxyModel::insertRows(int row, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->insertColumns(row, count, mapToSource(parent));
}
bool HorizontalProxyModel::removeColumns(int column, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->removeRows(column, count, mapToSource(parent));
}
bool HorizontalProxyModel::removeRows(int row, int count, const QModelIndex & parent) {
	Q_ASSERT(parent.isValid() ? parent.model() == this : true);
	return sourceModel()->removeColumns(row, count, mapToSource(parent));
}

QVariant HorizontalProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
//	if (!sourceModel()) { return QVariant(); }
	Qt::Orientation new_orientation = (orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
	QVariant result=sourceModel()->headerData(section, new_orientation, role);
	if(section!=0 && orientation==Qt::Horizontal && role==Qt::DisplayRole) {
		if(result.toString().compare("*") && result.toString().compare("!")) return QString();	//leave "*","!" unchanged
	}
	if(orientation==Qt::Vertical && role==Qt::DisplayRole) {
		return result.toString().prepend("     ");	//space for CheckboxHeader's checkboxes
	}
	if(orientation==Qt::Vertical && role==Qt::UserRole) {	//without spaces, for delegate
		return sourceModel()->headerData(section, new_orientation, Qt::DisplayRole);
	}

	return result;
}
QVariant HorizontalProxyModel::data(const QModelIndex &proxyIndex, int role) const {
	QVariant result=sourceModel()->data(mapToSource(proxyIndex),role);
	if(role==Qt::DisplayRole && result.isNull()) return result;	//don't process empty fields
	if(role==Qt::DisplayRole && vertical) {
		const QList<QVariant> &dbList=Database::getInstance().settings.database.value(
						headerData(proxyIndex.row(),Qt::Vertical,Qt::UserRole).toString()
					);
		if(!dbList.value(1).toString().compare("integer")
		|| !dbList.value(1).toString().compare("string")) {
//		if(	result.type()==QVariant::String
//		||	result.type()==QVariant::LongLong)
//		{
			return makeVertical(result.toString());
		}
		if(!dbList.value(1).toString().compare("real")) {
			return makeVertical(QString("%L1").arg(result.toDouble(), 0, 'f', 3));
		}
	}
	if(role==Qt::TextAlignmentRole) {
		return QVariant(Qt::AlignRight|Qt::AlignVCenter);
	}
	return result;
}
QString HorizontalProxyModel::makeVertical(QString source) const {
	QString result;
	for(int idx=0; idx<source.size(); ++idx) {	//strongly depends on locale for doubles (!)
		if(idx && source.at(idx)!=',' && source.at(idx-1)!='-') result.append("\n");
		result.append(source.at(idx));
	}
	return result;
}

void HorizontalProxyModel::setSourceModel(QAbstractItemModel * newSourceModel) {
	beginResetModel();

	if (sourceModel()) {
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeInserted,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeInserted);
		disconnect(sourceModel(),&QAbstractItemModel::rowsInserted,
							 this,&HorizontalProxyModel::sourceRowsInserted);
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeRemoved,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsRemoved,
							 this,&HorizontalProxyModel::sourceRowsRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsAboutToBeMoved,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeMoved);
		disconnect(sourceModel(),&QAbstractItemModel::rowsMoved,
							 this,&HorizontalProxyModel::sourceRowsMoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeInserted,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeInserted);
		disconnect(sourceModel(),&QAbstractItemModel::columnsInserted,
							 this,&HorizontalProxyModel::sourceColumnsInserted);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeRemoved,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsRemoved,
							 this,&HorizontalProxyModel::sourceColumnsRemoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsAboutToBeMoved,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeMoved);
		disconnect(sourceModel(),&QAbstractItemModel::columnsMoved,
							 this,&HorizontalProxyModel::sourceColumnsMoved);
		disconnect(sourceModel(),&QAbstractItemModel::modelAboutToBeReset,
							 this,&HorizontalProxyModel::sourceModelAboutToBeReset);
		disconnect(sourceModel(),&QAbstractItemModel::modelReset,
							 this,&HorizontalProxyModel::sourceModelReset);
		disconnect(sourceModel(),&QAbstractItemModel::dataChanged,
							 this,&HorizontalProxyModel::sourceDataChanged);
		disconnect(sourceModel(),&QAbstractItemModel::headerDataChanged,
							 this,&HorizontalProxyModel::sourceHeaderDataChanged);
		disconnect(sourceModel(),&QAbstractItemModel::layoutAboutToBeChanged,
							 this,&HorizontalProxyModel::sourceLayoutAboutToBeChanged);
		disconnect(sourceModel(),&QAbstractItemModel::layoutChanged,
							 this,&HorizontalProxyModel::sourceLayoutChanged);
	}

	QAbstractProxyModel::setSourceModel(newSourceModel);

	if (sourceModel()) {
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeInserted,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeInserted);
		connect(sourceModel(),&QAbstractItemModel::rowsInserted,
							 this,&HorizontalProxyModel::sourceRowsInserted);
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeRemoved,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeRemoved);
		connect(sourceModel(),&QAbstractItemModel::rowsRemoved,
							 this,&HorizontalProxyModel::sourceRowsRemoved);
		connect(sourceModel(),&QAbstractItemModel::rowsAboutToBeMoved,
							 this,&HorizontalProxyModel::sourceRowsAboutToBeMoved);
		connect(sourceModel(),&QAbstractItemModel::rowsMoved,
							 this,&HorizontalProxyModel::sourceRowsMoved);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeInserted,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeInserted);
		connect(sourceModel(),&QAbstractItemModel::columnsInserted,
							 this,&HorizontalProxyModel::sourceColumnsInserted);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeRemoved,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeRemoved);
		connect(sourceModel(),&QAbstractItemModel::columnsRemoved,
							 this,&HorizontalProxyModel::sourceColumnsRemoved);
		connect(sourceModel(),&QAbstractItemModel::columnsAboutToBeMoved,
							 this,&HorizontalProxyModel::sourceColumnsAboutToBeMoved);
		connect(sourceModel(),&QAbstractItemModel::columnsMoved,
							 this,&HorizontalProxyModel::sourceColumnsMoved);
		connect(sourceModel(),&QAbstractItemModel::modelAboutToBeReset,
							 this,&HorizontalProxyModel::sourceModelAboutToBeReset);
		connect(sourceModel(),&QAbstractItemModel::modelReset,
							 this,&HorizontalProxyModel::sourceModelReset);
		connect(sourceModel(),&QAbstractItemModel::dataChanged,
							 this,&HorizontalProxyModel::sourceDataChanged);
		connect(sourceModel(),&QAbstractItemModel::headerDataChanged,
							 this,&HorizontalProxyModel::sourceHeaderDataChanged);
		connect(sourceModel(),&QAbstractItemModel::layoutAboutToBeChanged,
							 this,&HorizontalProxyModel::sourceLayoutAboutToBeChanged);
		connect(sourceModel(),&QAbstractItemModel::layoutChanged,
							 this,&HorizontalProxyModel::sourceLayoutChanged);
	}

	endResetModel();
}
void HorizontalProxyModel::sourceRowsAboutToBeInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginInsertColumns(mapFromSource(parent), first, last);
}
void HorizontalProxyModel::sourceRowsInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endInsertColumns();
}
void HorizontalProxyModel::sourceRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	beginMoveColumns(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationRow);
}
void HorizontalProxyModel::sourceRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	Q_UNUSED(sourceParent)
	Q_UNUSED(sourceStart)
	Q_UNUSED(sourceEnd)
	Q_UNUSED(destinationParent)
	Q_UNUSED(destinationRow)
	endMoveColumns();
}
void HorizontalProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginRemoveColumns(mapFromSource(parent), first, last);
}
void HorizontalProxyModel::sourceRowsRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endRemoveColumns();
}
void HorizontalProxyModel::sourceColumnsAboutToBeInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginInsertRows(mapFromSource(parent), first, last);
}
void HorizontalProxyModel::sourceColumnsInserted(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endInsertRows();
}
void HorizontalProxyModel::sourceColumnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	beginMoveRows(mapFromSource(sourceParent), sourceStart, sourceEnd, mapFromSource(destinationParent), destinationColumn);
}
void HorizontalProxyModel::sourceColumnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn) {
	Q_ASSERT(sourceParent.isValid() ? sourceParent.model() == sourceModel() : true);
	Q_ASSERT(destinationParent.isValid() ? destinationParent.model() == sourceModel() : true);
	Q_UNUSED(sourceParent)
	Q_UNUSED(sourceStart)
	Q_UNUSED(sourceEnd)
	Q_UNUSED(destinationParent)
	Q_UNUSED(destinationColumn)
	endMoveRows();
}
void HorizontalProxyModel::sourceColumnsAboutToBeRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	beginRemoveRows(mapFromSource(parent), first, last);
}
void HorizontalProxyModel::sourceColumnsRemoved(const QModelIndex & parent, int first, int last) {
	Q_ASSERT(parent.isValid() ? parent.model() == sourceModel() : true);
	Q_UNUSED(parent)
	Q_UNUSED(first)
	Q_UNUSED(last)
	endRemoveRows();
}

void HorizontalProxyModel::sourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles) {
	Q_ASSERT(topLeft.isValid() ? topLeft.model() == sourceModel() : true);
	Q_ASSERT(bottomRight.isValid() ? bottomRight.model() == sourceModel() : true);
	emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight), roles);
}
void HorizontalProxyModel::sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last) {
	Qt::Orientation new_orientation = (orientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;
	emit headerDataChanged(new_orientation, first, last);
}

void HorizontalProxyModel::sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint) {
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
void HorizontalProxyModel::sourceLayoutChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint) {
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
void HorizontalProxyModel::sourceModelAboutToBeReset() { beginResetModel(); }
void HorizontalProxyModel::sourceModelReset() { endResetModel(); }

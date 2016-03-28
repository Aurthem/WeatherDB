#ifndef HORIZONTALPROXYMODEL_H
#define HORIZONTALPROXYMODEL_H

#include <QAbstractProxyModel>

class HorizontalProxyModel : public QAbstractProxyModel//QIdentityProxyModel
{	Q_OBJECT
public:
	HorizontalProxyModel(QObject * parent = 0);

	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
	virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
	virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const Q_DECL_OVERRIDE;
	virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const Q_DECL_OVERRIDE;

	virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) Q_DECL_OVERRIDE;
	virtual QModelIndex sibling(int row, int column, const QModelIndex &idx) const Q_DECL_OVERRIDE;
	virtual QItemSelection mapSelectionFromSource(const QItemSelection & selection) const Q_DECL_OVERRIDE;
	virtual QItemSelection mapSelectionToSource(const QItemSelection & selection) const Q_DECL_OVERRIDE;
	virtual QModelIndexList	match(const QModelIndex & start, int role, const QVariant & value,
												int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const Q_DECL_OVERRIDE;

	virtual bool insertColumns(int column, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual bool removeColumns(int column, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;
	virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) Q_DECL_OVERRIDE;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
	virtual QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

	void setVerticalRepresentation(bool vert) {
		if(vertical!=vert) {
			vertical=vert;
			emit dataChanged(index(0,0),
				index(rowCount()?rowCount()-1:0,columnCount()?columnCount()-1:0));
		}
	}

	virtual void setSourceModel(QAbstractItemModel * newSourceModel) Q_DECL_OVERRIDE;
private slots:
	void sourceRowsAboutToBeInserted(const QModelIndex & parent, int first, int last);
	void sourceRowsInserted(const QModelIndex & parent, int first, int last);
	void sourceRowsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
	void sourceRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
	void sourceRowsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
	void sourceRowsRemoved(const QModelIndex & parent, int first, int last);
	void sourceColumnsAboutToBeInserted(const QModelIndex & parent, int first, int last);
	void sourceColumnsInserted(const QModelIndex & parent, int first, int last);
	void sourceColumnsAboutToBeMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
	void sourceColumnsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationColumn);
	void sourceColumnsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
	void sourceColumnsRemoved(const QModelIndex & parent, int first, int last);

	void sourceDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles);
	void sourceHeaderDataChanged(Qt::Orientation orientation, int first, int last);

	void sourceLayoutAboutToBeChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint);
	void sourceLayoutChanged(const QList<QPersistentModelIndex> & sourceParents, QAbstractItemModel::LayoutChangeHint hint);
	void sourceModelAboutToBeReset();
	void sourceModelReset();
private:
	QList<QPersistentModelIndex> layoutChangePersistentIndexes;
	QModelIndexList proxyIndexes;
	class AbstractItemModelExposer : public QAbstractItemModel {
	public:
		QModelIndex createIndexHack(int row, int column, quintptr internalId) const { return createIndex(row, column, internalId); }
	};

	bool vertical;
	QString makeVertical(QString source) const;
};

#endif // HORIZONTALPROXYMODEL_H

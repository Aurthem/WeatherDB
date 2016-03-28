#ifndef FILTERPROXYMODEL_H
#define FILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class FilterProxyModel : public QSortFilterProxyModel
{	Q_OBJECT
public:
	FilterProxyModel(QObject *parent=0) : QSortFilterProxyModel(parent) { }

	void compileFilter(const QMap<int, QVariant> &source);
	//after filter is changed or applied, selection is corrupted --fixed (solution: invalidateFilter instead of invalidate)
	//when the column is selected by clicking the header, it leads to QPersistentModelIndex corruption (failure) --fixed
	//frozenTableColumn is repopulated when filter is reset (no columnsInserted signal?) --fixed
public slots:
	void compileNamedFilters(const QMap<QString,QPair<QVariant,QVariant> > &source);
protected:
//	bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const Q_DECL_OVERRIDE {
//		return true;
//	}
	bool filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const Q_DECL_OVERRIDE;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const Q_DECL_OVERRIDE;
private:
	bool lessThanVariants(const QVariant &left, const QVariant &right) const;

	void setFilterValue(int index, const QVariant &left, const QVariant &right) {
		filters[index]=QPair<QVariant,QVariant>(left,right);
		invalidateFilter();
//		invalidate();
	}
	int findIndexByName(const QString &name) const;

	QMap<int, QPair<QVariant,QVariant> > filters;
};

#endif // FILTERPROXYMODEL_H

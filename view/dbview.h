#ifndef DBVIEW_H
#define DBVIEW_H

#include <QTableView>

class DBView: public QTableView
{	Q_OBJECT
public:
	DBView(QWidget * parent = 0);

	virtual void keyboardSearch(const QString & search) Q_DECL_OVERRIDE {
		Q_UNUSED(search);
		//empty to disable keyboard search activated in read-only column
	}
//public slots:
//	virtual void reset() Q_DECL_OVERRIDE;
protected:
	void scrollContentsBy(int dx, int dy) Q_DECL_OVERRIDE;
};

class FreezeTableWidget : public DBView
{	Q_OBJECT
public:
	FreezeTableWidget(QAbstractItemModel *model, QWidget *parent=0);
	~FreezeTableWidget();

	//if the row is moved, column selection is no longer possible
	//happens because first row is hidden - first visual index, if hidden, breaks selection!
	//same with the last row
	//reimplement selectColumn?
	//try to set columns movable to reproduce the issue for row selection - not reproduced

	void setRowHidden(int row, bool hide);
	QList<int> getCheckedRows(void) const;
//	void unhideFrozenTableGeometry();

	QAbstractItemDelegate * getFrozenItemDelegate() const {	//need to connect to delegate from TabView
		return frozenTableView->itemDelegate();
	}
public slots:
	void updateFrozenTableGeometry();
	void handleColumnsInserted(const QModelIndex & parent, int first, int last);
	void handleRowsHidden(void);
	void handleRowsShown(void);
//	void updateHorizontalHeaderLayout(void);
	void setFrozenVisible(bool visible) {
		frozenTableView->setVisible(visible);
		//assuming that AddRowProxyModel and HorizontalProxyModel are both there:
		setColumnHidden(0,!visible);
	}
//	void handleRowsInserted(const QModelIndex &parent, int logicalFirst, int logicalLast) {
//		horizontalHeader()->sectionsInserted(parent, logicalFirst, logicalLast);
//	}
//	void updateGeometry() {
//		updateFrozenTableGeometry();
//		DBView::updateGeometry();
//	}
	void resizeRowsToContents(void);

	void customCellMenuRequested(const QPoint &pos);
	void customVHeaderMenuRequested(const QPoint &pos);
	void customHHeaderMenuRequested(const QPoint &pos);

//	void handleSelectionOnRemove(void);
//	virtual void reset() Q_DECL_OVERRIDE;

protected:
	virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
	virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) Q_DECL_OVERRIDE;
	void scrollTo(const QModelIndex & index, ScrollHint hint = EnsureVisible) Q_DECL_OVERRIDE;

	virtual bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) Q_DECL_OVERRIDE;
private:
	DBView *frozenTableView;
	void init();

	QAction *copyAction;
	QAction *deleteRowAction;
	QString getSelectedContents(void) const;

	void handleSetMarkColor(const QColor& color) const;

	int columnSectionAnchor;
	QItemSelectionModel::SelectionFlag ctrlDragSelectionFlag;

signals:
	void markRowsForRemoval(const QSet<int> &rowIdsRemoved);

private slots:
	void updateSectionWidth(int logicalIndex, int oldSize, int newSize);
	void updateSectionHeight(int logicalIndex, int oldSize, int newSize);
	void handleRowsMoved(int logical, int oldVisualIndex, int newVisualIndex);
	void handleColumnsMoved(int logical, int oldVisualIndex, int newVisualIndex);
//	void handleColumnCountChanged(int oldCount, int newCount);

	void slotSectionsSetHidden(const QList<int> &index_list,bool hide);

	void customSelectColumn(int column, bool anchor);
	bool customIsColumnSelected(int column,const QModelIndex &parent) const;
	QModelIndexList customSelectedColumns(int row) const;

//	void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
//		resizeRowsToContents();
//	}
};

#endif // DBVIEW_H

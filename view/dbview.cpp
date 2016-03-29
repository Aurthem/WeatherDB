//#include <QtWidgets>
#include <QScrollBar>
//#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QApplication>
#include <QColorDialog>
#include <QDebug>

#include "dbview.h"
#include "database.h"
#include "dbdelegates.h"
#include "checkboxheader.h"

DBView::DBView(QWidget * parent) : QTableView(parent) {
	QTableView::setItemDelegate(new DBDelegate(this));
	setWordWrap(true);
	setTextElideMode(Qt::ElideRight);
	CheckboxHeader* header = new CheckboxHeader(Qt::Vertical, this);
	setVerticalHeader(header);

	horizontalHeader()->setMinimumSectionSize(fontInfo().pixelSize()+2);	//13=11+2 for margins
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void DBView::scrollContentsBy(int dx, int dy) {
	QTableView::scrollContentsBy(dx,dy);
	CheckboxHeader* tmp=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(dy && tmp) {
		tmp->fixBoxPositions();
	}
}

FreezeTableWidget::FreezeTableWidget(QAbstractItemModel *model, QWidget *parent)
	: DBView(parent),frozenTableView(0),
//for customSelectColumn - these mimic corresponding fields of QTableViewPrivate:
		columnSectionAnchor(-1),ctrlDragSelectionFlag(QItemSelectionModel::NoUpdate)
{
	frozenTableView = new DBView(this);
	frozenTableView->setObjectName("frozenRow");	//for styling

	setModel(model);	//calls reset() after connections
	frozenTableView->setModel(model);	//try DBView::reset()
	init();

	disconnect(horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
	disconnect(horizontalHeader(), SIGNAL(sectionEntered(int)), this, SLOT(_q_selectColumn(int)));
	connect(horizontalHeader(),&QHeaderView::sectionPressed,[this](int logicalIndex){
		customSelectColumn(logicalIndex,true);
	});
	connect(horizontalHeader(),&QHeaderView::sectionEntered,[this](int logicalIndex){
		customSelectColumn(logicalIndex,false);
	});

	connect(model,&QAbstractItemModel::dataChanged,
					this,&FreezeTableWidget::resizeRowsToContents);
	connect(model,&QAbstractItemModel::columnsInserted,
					this,&FreezeTableWidget::handleColumnsInserted);	//update on insertion

	connect(model,&QAbstractItemModel::modelReset,
					this,&FreezeTableWidget::resizeRowsToContents);	//resize when rows are removed, etc.
	connect(model,&QAbstractItemModel::modelReset,[this]{
		handleColumnsInserted(QModelIndex(),1,horizontalHeader()->count()-1);
	});	//fix for model reset unhiding frozenTableView columns

	//connect the headers and scrollbars of both tableviews together
	connect(horizontalHeader(),&QHeaderView::sectionResized,
					this,&FreezeTableWidget::updateSectionWidth);
	connect(verticalHeader(),&QHeaderView::sectionResized,
					this,&FreezeTableWidget::updateSectionHeight);
	connect(horizontalHeader(),&QHeaderView::sectionMoved,
					this,&FreezeTableWidget::handleColumnsMoved);
	connect(verticalHeader(),&QHeaderView::sectionMoved,
					this,&FreezeTableWidget::handleRowsMoved);

	connect(frozenTableView->verticalScrollBar(),&QScrollBar::valueChanged,
					verticalScrollBar(),&QScrollBar::setValue);
	connect(verticalScrollBar(),&QScrollBar::valueChanged,
					frozenTableView->verticalScrollBar(),&QScrollBar::setValue);

//after connections were established:
	setRowHidden(0,true);
	frozenTableView->setRowHidden(0,true);
	resizeRowsToContents();

	CheckboxHeader* tmp_header=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(tmp_header) {
		connect(tmp_header,&CheckboxHeader::sectionsSetHidden,this,&FreezeTableWidget::slotSectionsSetHidden);
	}

	copyAction=new QAction(tr("Copy"), this);
	copyAction->setShortcuts(QKeySequence::Copy);
	copyAction->setStatusTip(tr("Copy cell data"));
	connect(copyAction, &QAction::triggered, [this]{
		QApplication::clipboard()->setText(getSelectedContents());
	});
	addAction(copyAction);	//to utilize shortcut

	deleteRowAction=new QAction(tr("Remove columns"), this);
	deleteRowAction->setStatusTip(tr("Remove selected columns from the table"));
	connect(deleteRowAction,&QAction::triggered,[this]{
		const QModelIndexList selectedColumns=customSelectedColumns(0);
		QSet<int> removedRowIds;
		for(const QModelIndex &idx: selectedColumns) {
			removedRowIds.insert(this->model()->data(this->model()->index(0,idx.column()),Qt::EditRole).toInt());
			this->model()->removeColumns(idx.column(),1);	//marks for removal, removes after submitAll() call
		}
		emit markRowsForRemoval(removedRowIds);
	});

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this,&FreezeTableWidget::customContextMenuRequested,
					this,&FreezeTableWidget::customCellMenuRequested);

	verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(verticalHeader(),&QHeaderView::customContextMenuRequested,
					this,&FreezeTableWidget::customVHeaderMenuRequested);

	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(horizontalHeader(),&QHeaderView::customContextMenuRequested,
					this,&FreezeTableWidget::customHHeaderMenuRequested);
}
FreezeTableWidget::~FreezeTableWidget() {
	delete frozenTableView;
}

void FreezeTableWidget::init() {
	frozenTableView->setFocusPolicy(Qt::NoFocus);
	frozenTableView->verticalHeader()->hide();
	frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	viewport()->stackUnder(frozenTableView);

	frozenTableView->setSelectionModel(selectionModel());

	frozenTableView->setColumnWidth(0, columnWidth(0));

	frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	frozenTableView->show();

	updateFrozenTableGeometry();
	for (int col = 1; col < model()->columnCount(); ++col)
		frozenTableView->setColumnHidden(col, true);	//should hide more when inserted

	setHorizontalScrollMode(ScrollPerPixel);
	setVerticalScrollMode(ScrollPerPixel);
	frozenTableView->setVerticalScrollMode(ScrollPerPixel);

//	frozenTableView->setEditTriggers(QTableView::DoubleClicked|QTableView::AnyKeyPressed);
}

QString FreezeTableWidget::getSelectedContents(void) const {
	QString result;
	const QModelIndexList selected=selectedIndexes();
	bool new_line=true;
	int current_row=-1;
	QLocale defaultLocale;	//locale for double delimiters
	for(const QModelIndex &index: selected) {
		if(index.row()!=current_row) {
			if(current_row>=0) {
				result.append("\n");
				new_line=true;
			}
			current_row=index.row();
		}
		if(!new_line) result.append("\t");
		QVariant tmp=index.data(Qt::EditRole);
		result.append( (tmp.type()==QVariant::Double) ?
			defaultLocale.toString(tmp.toDouble()) : tmp.toString());
		new_line=false;
	}
	return result;
}

void FreezeTableWidget::updateSectionWidth(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	if (logicalIndex == 0){
		frozenTableView->setColumnWidth(0, newSize);
		updateFrozenTableGeometry();
	} else
		updateFrozenTableGeometry();	//fix for frozenColumn resize when inserting column
}
void FreezeTableWidget::updateSectionHeight(int logicalIndex, int oldSize, int newSize) {
	Q_UNUSED(oldSize)
	frozenTableView->setRowHeight(logicalIndex, newSize);
}
void FreezeTableWidget::handleRowsMoved(int logical, int oldVisualIndex, int newVisualIndex) {
	Q_UNUSED(logical)
	frozenTableView->verticalHeader()->moveSection(oldVisualIndex,newVisualIndex);
}
void FreezeTableWidget::handleColumnsMoved(int logical, int oldVisualIndex, int newVisualIndex) {
	Q_UNUSED(logical)
	frozenTableView->horizontalHeader()->moveSection(oldVisualIndex,newVisualIndex);
}
void FreezeTableWidget::setRowHidden(int row, bool hide) {
	if (row < 0 || row >= verticalHeader()->count()) return;
	verticalHeader()->setSectionHidden(row, hide);
	if(frozenTableView) frozenTableView->verticalHeader()->setSectionHidden(row,hide);
}

QList<int> FreezeTableWidget::getCheckedRows(void) const {
	CheckboxHeader* hdr=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(hdr) return hdr->getCheckedBoxes();
	return QList<int>();
}

void FreezeTableWidget::handleRowsHidden() {
	CheckboxHeader* hdr=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(hdr) hdr->handleRowsHidden();
	//frozenTableView hides its own columns

	resizeColumnsToContents();
	updateFrozenTableGeometry();
	//issue: when vertical header is resized after some horizontal header sections are hidden,
	//frozenTableView remains shifted in its old position
}
void FreezeTableWidget::handleRowsShown() {
	CheckboxHeader* hdr=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(hdr) hdr->handleRowsShown();
	resizeColumnsToContents();
	updateFrozenTableGeometry();
}
void FreezeTableWidget::resizeRowsToContents(void) {
	QTableView::resizeRowsToContents();
	CheckboxHeader *tmp=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(tmp) tmp->fixBoxPositions();
}

void FreezeTableWidget::slotSectionsSetHidden(const QList<int> &index_list,bool hide) {
	for(const int &idx: index_list) {
		frozenTableView->verticalHeader()->setSectionHidden(idx,hide);
	}
}

void FreezeTableWidget::customCellMenuRequested(const QPoint &pos) {
	if(!indexAt(pos).isValid()) return;
	QMenu contextMenu(tr("Cell context menu"), this);
	QAction markAction(tr("Mark"), this);
	markAction.setStatusTip(tr("Mark selected cells with a color"));
	connect(&markAction, &QAction::triggered, [this]{
		const QColorDialog::ColorDialogOptions options=QColorDialog::ShowAlphaChannel;
		const QColor color = QColorDialog::getColor(Qt::white, this, tr("Select mark color"), options);
		if(color.isValid()) {
			handleSetMarkColor(color);
		}
	});
	QAction clearAction(tr("Remove marks"), this);
	clearAction.setStatusTip(tr("Remove color marks of selected cells"));
	connect(&clearAction, &QAction::triggered, [this]{ handleSetMarkColor(QColor()); });

	contextMenu.addAction(copyAction);	//ownership is not transferred (!)
	contextMenu.addSeparator();
	contextMenu.addAction(&markAction);
	contextMenu.addAction(&clearAction);
	if(!customSelectedColumns(0).isEmpty()) {	//overhead in double call (inside action too)
		contextMenu.addSeparator();
		contextMenu.addAction(deleteRowAction);
	}
	contextMenu.exec(viewport()->mapToGlobal(pos));
}
void FreezeTableWidget::handleSetMarkColor(const QColor& color) const {
	//split selections if there are hidden sections inside
	QItemSelection selectionWithHidden;
	const QItemSelection selectionDefault=selectionModel()->selection();
	for(const QItemSelectionRange &range: selectionDefault) {
		int tmp_top=range.top(), tmp_bottom=range.top();
		for(int idx=range.top();idx<=range.bottom();) {
			if(verticalHeader()->isSectionHidden(idx)) {	//top and bottom can't be hidden anyway
				if(idx!=tmp_top)
					selectionWithHidden.append(QItemSelectionRange(range.model()->index(tmp_top,range.left(),range.parent()),
																											 range.model()->index(tmp_bottom,range.right(),range.parent())));
				tmp_top=tmp_bottom=++idx;
			} else {
				tmp_bottom=idx++;
			}
		}
		if(tmp_top<=range.bottom())
			selectionWithHidden.append(QItemSelectionRange(range.model()->index(tmp_top,range.left(),range.parent()),
																									 range.model()->index(range.bottom(),range.right(),range.parent())));
	}

	Database::getInstance().setMarkColor(selectionWithHidden,color);
}

void FreezeTableWidget::customVHeaderMenuRequested(const QPoint &pos) {
	QMenu contextMenu(tr("Vertical header context menu"), this);
	QAction lockAction(tr("Locked"), this);
	lockAction.setStatusTip(tr("Lock or unlock row movement"));
	lockAction.setCheckable(true); lockAction.setChecked(!verticalHeader()->sectionsMovable());
	connect(&lockAction, &QAction::triggered, [this](bool checked){
		verticalHeader()->setSectionsMovable(!checked);
	});
	contextMenu.addAction(&lockAction);
	contextMenu.exec(verticalHeader()->viewport()->mapToGlobal(pos));
}
void FreezeTableWidget::customHHeaderMenuRequested(const QPoint &pos) {
	if(customSelectedColumns(0).isEmpty()) return;
	QMenu contextMenu(tr("Horizontal header context menu"), this);
	contextMenu.addAction(deleteRowAction);
	contextMenu.exec(horizontalHeader()->viewport()->mapToGlobal(pos));
}

void FreezeTableWidget::resizeEvent(QResizeEvent * event) {
	QTableView::resizeEvent(event);
	updateFrozenTableGeometry();
}
QModelIndex FreezeTableWidget::moveCursor(CursorAction cursorAction,
																					Qt::KeyboardModifiers modifiers)
{
	QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

	if (cursorAction == MoveLeft && current.column() > 0
			&& visualRect(current).topLeft().x() < frozenTableView->columnWidth(0) ){
		const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x()
				- frozenTableView->columnWidth(0);
		horizontalScrollBar()->setValue(newValue);
	}
	return current;
}
void FreezeTableWidget::scrollTo (const QModelIndex & index, ScrollHint hint) {
	if (index.column() > 0)
		QTableView::scrollTo(index, hint);
}
bool FreezeTableWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
	if(!index.isValid()) return false;
	if(index.column()==0) {
		if(trigger&QAbstractItemView::AnyKeyPressed) {
			QCoreApplication::sendEvent(frozenTableView,event);
//			QKeyEvent* key=qobject_cast<QKeyEvent*>(event);
//			return true;
		}
		return false;
	}
	return DBView::edit(index,trigger,event);
}

void FreezeTableWidget::updateFrozenTableGeometry() {
	frozenTableView->setGeometry(verticalHeader()->width() + frameWidth(),
															 frameWidth(), columnWidth(0),
															 viewport()->height()+horizontalHeader()->height());
//	for (int col = 1; col < model()->columnCount(); ++col)
//		frozenTableView->setColumnHidden(col, true);	//moved to a slot to avoid unnecessary actions
}
void FreezeTableWidget::handleColumnsInserted(const QModelIndex &parent, int first, int last) {
	Q_UNUSED(parent);
	for (int col = first; col<=last; ++col)
		frozenTableView->setColumnHidden(col, true);//should hide more when inserted
}

//the following is a workaround for https://bugreports.qt.io/browse/QTBUG-50171
//check if it's relevant after version 5.5.1
void FreezeTableWidget::customSelectColumn(int column, bool anchor) {
	if (selectionBehavior() == QTableView::SelectRows
			|| (selectionMode() == QTableView::SingleSelection
					&& selectionBehavior() == QTableView::SelectItems))
		return;

	if (column >= 0 && column < model()->columnCount()) {
		int row = verticalHeader()->logicalIndexAt(0);
		QModelIndex index = model()->index(row, column);
		QItemSelectionModel::SelectionFlags command = selectionCommand(index);
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
		if ((anchor && !(command & QItemSelectionModel::Current))
				|| (selectionMode() == QTableView::SingleSelection))
			columnSectionAnchor = column;

		if (selectionMode() != QTableView::SingleSelection
				&& command.testFlag(QItemSelectionModel::Toggle)) {
			if (anchor)	//horizontalHeader()->selectionModel()->selectedColumns(row) if rows moved so the first visual is not row=0
				ctrlDragSelectionFlag = customSelectedColumns(row).contains(index)
						? QItemSelectionModel::Deselect : QItemSelectionModel::Select;
			command &= ~QItemSelectionModel::Toggle;
			command |= ctrlDragSelectionFlag;
			if (!anchor)
				command |= QItemSelectionModel::Current;
		}

		//logicalIndex(int) doesn't play well with moved and hidden sections, thus logicalIndexAt(int) is better
		QModelIndex tl = model()->index(verticalHeader()->logicalIndexAt(0), qMin(columnSectionAnchor, column));
		QModelIndex br = model()->index(verticalHeader()->logicalIndexAt(verticalHeader()->length()-1),
																		qMax(columnSectionAnchor, column));
		if ((horizontalHeader()->sectionsMoved() && tl.column() != br.column())
				|| verticalHeader()->sectionsMoved()) {
			setSelection(visualRect(tl)|visualRect(br), command);
		} else {
			selectionModel()->select(QItemSelection(tl, br), command);
		}
	}
}

bool FreezeTableWidget::customIsColumnSelected(int column, const QModelIndex &parent) const {
	if (!selectionModel()->model()) return false;
	if (parent.isValid() && selectionModel()->model() != parent.model()) return false;

	const QItemSelection currentSelection=selectionModel()->selection();

	QList<QItemSelectionRange>::const_iterator it;
	int rowCount = model()->rowCount(parent);
	for (int row = 0; row < rowCount; ++row) {
		if(verticalHeader()->isSectionHidden(row)) continue;
		for (it = currentSelection.constBegin(); it != currentSelection.constEnd(); ++it) {
			if (it->contains(row, column, parent)) {
				Qt::ItemFlags flags = model()->index(row, column, parent).flags();
				if ((flags & Qt::ItemIsSelectable) && (flags & Qt::ItemIsEnabled)) {
					row = qMax(row, it->bottom());
					break;
				}
			}
		}
		if (it == currentSelection.constEnd())
			return false;
	}
	return rowCount > 0; // no rows means no selected items
}
QModelIndexList FreezeTableWidget::customSelectedColumns(int row) const {
	QModelIndexList indexes;
	//the QSet contains pairs of parent modelIndex and column number
	QSet< QPair<QModelIndex, int> > columnsSeen;

	const QItemSelection ranges = selectionModel()->selection();
	for (int i = 0; i < ranges.count(); ++i) {
		const QItemSelectionRange &range = ranges.at(i);
		QModelIndex parent = range.parent();
		for (int column = range.left(); column <= range.right(); ++column) {
			QPair<QModelIndex, int> columnDef = qMakePair(parent, column);
			if (!columnsSeen.contains(columnDef)) {
				columnsSeen << columnDef;
				if (customIsColumnSelected(column, parent)) {
					indexes.append(model()->index(row, column, parent));
				}
			}
		}
	}

	return indexes;
}

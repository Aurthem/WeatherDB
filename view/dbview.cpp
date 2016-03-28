//#include <QtWidgets>
#include <QScrollBar>
//#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QApplication>
#include <QColorDialog>

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
//	verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
//	setLayoutDirection(Qt::RightToLeft);
}

void DBView::scrollContentsBy(int dx, int dy) {
	QTableView::scrollContentsBy(dx,dy);
//	CheckboxHeader* tmp=dynamic_cast<CheckboxHeader*>(verticalHeader());
	CheckboxHeader* tmp=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(dy && tmp) {
		tmp->fixBoxPositions();
	}
}
//void DBView::reset() {
//	qDebug()<<"dbv reset";
//	qDebug()<<verticalHeader()->count()<<verticalHeader()->isSectionHidden(0)
//				 <<isRowHidden(0);
//	QTableView::reset();
////	setRowHidden(0,true);
//	verticalHeader()->setSectionHidden(0,true);
//	qDebug()<<verticalHeader()->count()<<verticalHeader()->isSectionHidden(0)
//				 <<isRowHidden(0);
//}

FreezeTableWidget::FreezeTableWidget(QAbstractItemModel *model, QWidget *parent)
//for customSelectColumn - these mimic corresponding fields of QTableViewPrivate:
	: DBView(parent),frozenTableView(0),
		columnSectionAnchor(-1),ctrlDragSelectionFlag(QItemSelectionModel::NoUpdate)
{
//	frozenTableView = new QTableView(this);
	frozenTableView = new DBView(this);
//	frozenTableView = new QTableView(this);
	frozenTableView->setObjectName("frozenRow");

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

//	connect(model,&QAbstractItemModel::layoutChanged,
//					this,&FreezeTableWidget::resizeColumnsToContents);	//on sort filter change
//	connect(model,&QAbstractItemModel::columnsInserted,
//					horizontalHeader(),&QHeaderView::doItemsLayout);
//	connect(model,&QAbstractItemModel::columnsRemoved,
//					horizontalHeader(),&QHeaderView::doItemsLayout);	//works well without these now

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
//	verticalHeader()->hideSection(0);	//handle checkboxes in checkboxheader (!)
//	frozenTableView->verticalHeader()->hideSection(0);	//keyboard navigation will be able to go there otherwise
	setRowHidden(0,true);
	frozenTableView->setRowHidden(0,true);
	resizeRowsToContents();

	CheckboxHeader* tmp_header=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(tmp_header) {
		connect(tmp_header,&CheckboxHeader::sectionsSetHidden,this,&FreezeTableWidget::slotSectionsSetHidden);
	}

	copyAction=new QAction("Копировать", this);
	copyAction->setShortcuts(QKeySequence::Copy);
	copyAction->setStatusTip(tr("Копировать данные ячеек"));
	connect(copyAction, &QAction::triggered, [this]{
		QApplication::clipboard()->setText(getSelectedContents());
	});
	addAction(copyAction);	//to utilize shortcut

	deleteRowAction=new QAction(tr("Удалить столбцы"), this);
	deleteRowAction->setStatusTip(tr("Удалить выделенные столбцы из таблицы"));
	connect(deleteRowAction,&QAction::triggered,[this]{
		const QModelIndexList selectedColumns=customSelectedColumns(0);
//		qDebug()<<selectedColumns;
		QSet<int> removedRowIds;
		for(const QModelIndex &idx: selectedColumns) {
//			qDebug()<<this->model()->data(this->model()->index(0,idx.column()),Qt::EditRole);
			removedRowIds.insert(this->model()->data(this->model()->index(0,idx.column()),Qt::EditRole).toInt());
			this->model()->removeColumns(idx.column(),1);	//marks for removal, removes after submitAll() call
		}
		emit markRowsForRemoval(removedRowIds);
//		qDebug()<<removedRowIds;
//		model()->submit();
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

//	connect(horizontalHeader(),&QHeaderView::sectionPressed,this,&FreezeTableWidget::selectColumn);

//	connect(this->model(),&QAbstractItemModel::layoutChanged,[this]{
//		QItemSelection s = selectionModel()->selection();
//		setCurrentIndex( currentIndex() );
//		selectionModel()->select( s, QItemSelectionModel::SelectCurrent );
//	});
}
FreezeTableWidget::~FreezeTableWidget()
{
	delete frozenTableView;
}

void FreezeTableWidget::init()
{
	frozenTableView->setFocusPolicy(Qt::NoFocus);
	frozenTableView->verticalHeader()->hide();
	frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
//	frozenTableView->setColumnWidth(0,columnWidth(0));
//	frozenTableView->setBaseSize(frozenTableView->width()+20,frozenTableView->height());
//	frozenTableView->setFixedWidth(columnWidth(0));

	viewport()->stackUnder(frozenTableView);
//	frozenTableView->viewport()->stackUnder(this);
//	setColumnHidden(0,true);
//	QHBoxLayout *tmp_layout=new QHBoxLayout();
//	viewport()->layout()->setContentsMargins(50,0,0,0);
//	tmp_layout->setContentsMargins(50,50,50,50);
//	viewport()->setLayout(tmp_layout);
//	findChild<QWidget*>(viewport()->objectName())->setLayout(tmp_layout);

//	frozenTableView->setStyleSheet("QTableView { border: none;"
//																 "background-color: rgb(225,255,225);"
//																 "selection-background-color: lightblue"
//																 "}"); //for demo purposes

	frozenTableView->setSelectionModel(selectionModel());

	frozenTableView->setColumnWidth(0, columnWidth(0) );

	frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	frozenTableView->show();

	updateFrozenTableGeometry();
	for (int col = 1; col < model()->columnCount(); ++col)	//inside now (or is it?)
		frozenTableView->setColumnHidden(col, true);//should hide more when inserted

	setHorizontalScrollMode(ScrollPerPixel);
	setVerticalScrollMode(ScrollPerPixel);
	frozenTableView->setVerticalScrollMode(ScrollPerPixel);

//	frozenTableView->setEditTriggers(QTableView::DoubleClicked|QTableView::AnyKeyPressed);
}

QString FreezeTableWidget::getSelectedContents(void) const {
	QString result;
//	const QModelIndexList selected=selectionModel()->selectedIndexes();
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
//		result.append(index.data(Qt::EditRole).toString());
		QVariant tmp=index.data(Qt::EditRole);
		result.append( (tmp.type()==QVariant::Double) ?
			defaultLocale.toString(tmp.toDouble()) : tmp.toString());
		new_line=false;
	}
	return result;
}

void FreezeTableWidget::updateSectionWidth(int logicalIndex, int /* oldSize */, int newSize)
{
	if (logicalIndex == 0){
		frozenTableView->setColumnWidth(0, newSize);
		updateFrozenTableGeometry();
	} else
		updateFrozenTableGeometry();	//fix for frozenColumn resize when inserting column
}
void FreezeTableWidget::updateSectionHeight(int logicalIndex, int /* oldSize */, int newSize)
{
	frozenTableView->setRowHeight(logicalIndex, newSize);
}
void FreezeTableWidget::handleRowsMoved(int /*logical*/, int oldVisualIndex, int newVisualIndex) {
	frozenTableView->verticalHeader()->moveSection(oldVisualIndex,newVisualIndex);
}
void FreezeTableWidget::handleColumnsMoved(int /*logical*/, int oldVisualIndex, int newVisualIndex) {
	frozenTableView->horizontalHeader()->moveSection(oldVisualIndex,newVisualIndex);
}
//void FreezeTableWidget::handleColumnCountChanged(int oldCount, int newCount) {
//	frozenTableView->columnCountChanged(oldCount,newCount);
//}
//void FreezeTableWidget::updateHorizontalHeaderLayout(void) {
////	horizontalHeader()->doItemsLayout();
////	frozenTableView->horizontalHeader()->doItemsLayout();
//	frozenTableView->resizeColumnsToContents();
//	updateFrozenTableGeometry();
//}
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
	//frozenTableView should hide its own columns
//	QList<int> rows_to_hide=getCheckedRows();
//	for(int idx: rows_to_hide) {
//		frozenTableView->verticalHeader()->setSectionHidden(idx,true);
//	}

	resizeColumnsToContents();
	updateFrozenTableGeometry();
//	frozenTableView->resizeColumnsToContents();
//	verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
//	horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
//	frozenTableView->verticalHeader()->show();
//	setFrozenVisible(false);
//	setFrozenVisible(true);
//	frozenTableView->horizontalHeader()->doItemsLayout();
//	frozenTableView->verticalHeader()->updateGeometry();
//	frozenTableView->hide();
//	frozenTableView->show();
//	horizontalHeader()->resizeSections();
//	updateFrozenTableGeometry();
//	horizontalHeader()->reset();
//	frozenTableView->doItemsLayout();
//	frozenTableView->horizontalHeader()->doItemsLayout();
//	updateFrozenTableGeometry();
//	frozenTableView->update();
//	frozenTableView->setHidden(true);
//	frozenTableView->setHidden(false);

	//when vertical header is resized after some sections are hidden,
	//frozenTableView remains shifted in its old position (no idea how to fix)
}
void FreezeTableWidget::handleRowsShown() {
	CheckboxHeader* hdr=qobject_cast<CheckboxHeader*>(verticalHeader());
	if(hdr) hdr->handleRowsShown();
	resizeColumnsToContents();
	updateFrozenTableGeometry();
//	frozenTableView->resizeColumnsToContents();
//	frozenTableView->verticalHeader()->resizeSections();

//	frozenTableView->verticalHeader()->show();
//	setFrozenVisible(false);
//	setFrozenVisible(true);
//	updateFrozenTableGeometry();
//	frozenTableView->verticalHeader()->updateGeometry();
//	frozenTableView->hide();
//	frozenTableView->show();
//	horizontalHeader()->resizeSections();
//	horizontalHeader()->reset();
//	updateFrozenTableGeometry();
//	frozenTableView->update();
//	frozenTableView->setHidden(true);
//	frozenTableView->setHidden(false);
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
//	qDebug()<<"slotSectionsSetHidden";
}

void FreezeTableWidget::customCellMenuRequested(const QPoint &pos) {
	if(!indexAt(pos).isValid()) return;
	QMenu contextMenu(tr("Cell context menu"), this);
	QAction markAction(tr("Пометить"), this);
	markAction.setStatusTip(tr("Пометить выделенные ячейки цветом"));
	connect(&markAction, &QAction::triggered, [this]{
		const QColorDialog::ColorDialogOptions options=QColorDialog::ShowAlphaChannel;
		const QColor color = QColorDialog::getColor(Qt::white, this, tr("Выберите цвет метки"), options);
		if(color.isValid()) {
			handleSetMarkColor(color);
		}
	});
	QAction clearAction(tr("Убрать метки"), this);
	clearAction.setStatusTip(tr("Убрать цветовые метки выделенных ячеек"));
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
	QAction lockAction(tr("Закреплено"), this);
	lockAction.setStatusTip(tr("Разрешить или запретить перемещение строк"));
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

//void FreezeTableWidget::reset() {
////	qDebug()<<"ftw reset";
////	qDebug()<<verticalHeader()->count()<<verticalHeader()->isSectionHidden(0)
////				 <<isRowHidden(0);
//	DBView::reset();
////	resizeRowsToContents();
////	updateFrozenTableGeometry();	//hangs, recursion
//}

void FreezeTableWidget::resizeEvent(QResizeEvent * event)
{
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
void FreezeTableWidget::scrollTo (const QModelIndex & index, ScrollHint hint){
	if (index.column() > 0)
		QTableView::scrollTo(index, hint);
}
bool FreezeTableWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
	if(!index.isValid()) return false;
	if(index.column()==0) {
		if(trigger&QAbstractItemView::AnyKeyPressed) {
//			event->setAccepted(false);
//			frozenTableView->edit(index);
			QCoreApplication::sendEvent(frozenTableView,event);
//			frozenTableView->event(event);
//			QKeyEvent* key=qobject_cast<QKeyEvent*>(event);
//			return true;
		}
		return false;
	}
	return DBView::edit(index,trigger,event);
}

//void FreezeTableWidget::unhideFrozenTableGeometry()
//{
//	for (int col = 1; col < model()->columnCount(); ++col)
//		frozenTableView->setColumnHidden(col, false);//should hide more when inserted
//}
void FreezeTableWidget::updateFrozenTableGeometry() {
	frozenTableView->setGeometry(verticalHeader()->width() + frameWidth(),
															 frameWidth(), columnWidth(0),
															 viewport()->height()+horizontalHeader()->height());
//	qDebug()<<verticalHeader()->width()<<"|"<<frameWidth()<<"|"<<columnWidth(0)<<"|"<<viewport()->height()<<"|"<<horizontalHeader()->height();
//	for (int col = 1; col < model()->columnCount(); ++col)
//		frozenTableView->setColumnHidden(col, true);//should hide more when inserted
}
void FreezeTableWidget::handleColumnsInserted(const QModelIndex & parent, int first, int last) {
	Q_UNUSED(parent);
//	horizontalHeader()->rowsAboutToBeRemoved(parent,first,last);
//	int last_idx=horizontalHeader()->count()-last+first-1;
//	for(int idx=0; idx<horizontalHeader()->count(); ++idx) {
//		if(idx>=first && idx<=last) horizontalHeader()->setSectionHidden(idx,true);
//		if(idx>=last_idx) horizontalHeader()->setSectionHidden(idx,true);
//	}
//	horizontalHeader()->sectionCountChanged(horizontalHeader()->count(),horizontalHeader()->count()-last+first-1);
//	horizontalHeader()->headerDataChanged(Qt::Horizontal,0,horizontalHeader()->count()-1);
//	layoutChanged();
//	frozenTableView->model()->resetInternalData();
//	frozenTableView->;
//	int count=1+last-first;
	for (int col = first; col<=last; ++col)
		frozenTableView->setColumnHidden(col, true);//should hide more when inserted
}

//void FreezeTableWidget::handleSelectionOnRemove(void) {
//	selectionModel()->setCurrentIndex(model()->index(0,0),QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
//	frozenTableView->selectionModel()->setCurrentIndex(model()->index(0,0),QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
//	unhideFrozenTableGeometry();
//	moveCursor(QTableView::MoveHome,Qt::NoModifier);
//	setSelection(QRect(0,0,10,10),QItemSelectionModel::SelectCurrent);
//	selectionChanged(QItemSelection(),QItemSelection(model()->index(0,0),model()->index(model()->rowCount()-1,model()->columnCount()-1)));
//	setCurrentIndex(model()->index(0,0));
//	selectionModel()->clearCurrentIndex();
//	selectionModel()->reset();
//	frozenTableView->selectionModel()->reset();
//	qDebug()<<selectionModel()->hasSelection();
//}

//the following is a workaround for https://bugreports.qt.io/browse/QTBUG-50171
//check if it's relevant after version 5.5.1
void FreezeTableWidget::customSelectColumn(int column, bool anchor) {
	if (selectionBehavior() == QTableView::SelectRows
			|| (selectionMode() == QTableView::SingleSelection
					&& selectionBehavior() == QTableView::SelectItems))
		return;

//	int columnSectionAnchor=-1;
//	QItemSelectionModel::SelectionFlag ctrlDragSelectionFlag=QItemSelectionModel::NoUpdate;
	if (column >= 0 && column < model()->columnCount()) {
		int row = verticalHeader()->logicalIndexAt(0);
		QModelIndex index = model()->index(row, column);
		QItemSelectionModel::SelectionFlags command = selectionCommand(index);
		selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
		if ((anchor && !(command & QItemSelectionModel::Current))
				|| (selectionMode() == QTableView::SingleSelection))
			columnSectionAnchor = column;

//		qDebug()<<ctrlDragSelectionFlag<<command;
//		const QItemSelection selected_ranges=selectionModel()->selection();
//		QSet<int> selected_columns_to_check;
//		for(const QItemSelectionRange &range: selected_ranges) {
//			for(int idx=range.left();idx<=range.right();++idx) {
//				selected_columns_to_check.insert(idx);
//			}
//		}
//		const QModelIndexList selected_indexes=selectionModel()->selectedIndexes();
//		for(const QModelIndex &idx: selected_indexes) {
////			for(int model_row=0; model_row<model()->rowCount();++model_row) {
//				if(!verticalHeader()->isSectionHidden(model_row)) {
//					if(model()->index(model_row,idx.column())) {

//					}
//				}
////			}
//		}
//		qDebug()<<index;
//		qDebug()<<customSelectedColumns(row);
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
//		qDebug()<<ctrlDragSelectionFlag<<command;

//		qDebug()<<verticalHeader()->length();
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

bool FreezeTableWidget::customIsColumnSelected(int column, const QModelIndex &parent) const
{
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
QModelIndexList FreezeTableWidget::customSelectedColumns(int row) const
{
	QModelIndexList indexes;
	//the QSet contains pairs of parent modelIndex
	//and column number
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

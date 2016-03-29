//#include <QtWidgets>
#include <QCheckBox>

#include "checkboxheader.h"

CheckboxHeader::CheckboxHeader(Qt::Orientation orientation, QWidget* parent)
	: QHeaderView(orientation, parent)
{
	connect(this, SIGNAL(sectionResized(int, int, int)), this,
					SLOT(handleSectionResized(int)));
	connect(this, SIGNAL(sectionMoved(int, int, int)), this,
					SLOT(handleSectionMoved(int, int, int)));
//	setSectionsMovable(true);
//	setSectionsClickable(true);
}

QList<int> CheckboxHeader::getCheckedBoxes(void) const {
	QList<int> result;
	for(QMap<int,QCheckBox*>::const_iterator itr=boxes.begin();
			itr!=boxes.end();++itr) {
		if(itr.value()->isChecked()) result.append(itr.key());
	}
	return result;
}

void CheckboxHeader::handleSectionResized(int idx) {
	for (int j=visualIndex(idx); j<count(); ++j) {
		int logical = logicalIndex(j);
		if(logical==0) continue;	//skip id row (model specific)
		updateBox(logical);
		if(orientation()==Qt::Horizontal) {
			boxes[logical]->setGeometry(sectionViewportPosition(logical)+(sectionSize(logical)-boxes[logical]->minimumSizeHint().width())/2, 2,
				boxes[logical]->minimumSizeHint().width(), boxes[logical]->minimumSizeHint().height());
		} else {
			boxes[logical]->setGeometry(2, sectionViewportPosition(logical)+(sectionSize(logical)-boxes[logical]->minimumSizeHint().height())/2,
				boxes[logical]->minimumSizeHint().width(), boxes[logical]->minimumSizeHint().height());
		}
	}
}
void CheckboxHeader::handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex) {
	Q_UNUSED(logical)
	for (int idx=qMin(oldVisualIndex, newVisualIndex);idx<count();++idx){
		int logical = logicalIndex(idx);
		if(logical==0) continue;	//skip id row (model specific)
		updateBox(logical);
		if(orientation()==Qt::Horizontal) {
			boxes[logical]->setGeometry(sectionViewportPosition(logical)+(sectionSize(logical)-boxes[logical]->minimumSizeHint().width())/2, 2,
				boxes[logical]->minimumSizeHint().width(), boxes[logical]->minimumSizeHint().height());
		} else {
			boxes[logical]->setGeometry(2, sectionViewportPosition(logical)+(sectionSize(logical)-boxes[logical]->minimumSizeHint().height())/2,
				boxes[logical]->minimumSizeHint().width(), boxes[logical]->minimumSizeHint().height());
		}
	}
}
void CheckboxHeader::handleRowsHidden() {
	QList<int> indexHidden;
	for(QMap<int,QCheckBox*>::iterator itr=boxes.begin();
			itr!=boxes.end();++itr) {
		if(itr.value()->isChecked()) {
			hideSection(itr.key());
			indexHidden.append(itr.key());
			itr.value()->setHidden(true);
			itr.value()->setChecked(false);
		}
	}
	emit sectionsSetHidden(indexHidden,true);
	emit geometriesChanged();	//to update vertical header width
//	fixBoxPositions();
}
void CheckboxHeader::handleRowsShown() {
	QList<int> indexHidden;
	for(QMap<int,QCheckBox*>::iterator itr=boxes.begin();
			itr!=boxes.end();++itr) {
//		if(itr.key()==0) continue;	//skip id row (model specific)
//		itr.value()->setChecked(false);
		showSection(itr.key());
		indexHidden.append(itr.key());
		itr.value()->setHidden(false);
	}
	emit sectionsSetHidden(indexHidden,false);
	emit geometriesChanged();	//to update vertical header width
//	fixBoxPositions();
}

void CheckboxHeader::fixBoxPositions() {
	for (int idx=1; idx<count(); ++idx) {	//start from 1 to exclude id row
		updateBox(idx);
		if(orientation()==Qt::Horizontal) {
			boxes[idx]->setGeometry(sectionViewportPosition(idx)+(sectionSize(idx)-boxes[idx]->minimumSizeHint().width())/2, 2,
				boxes[idx]->minimumSizeHint().width(), boxes[idx]->minimumSizeHint().height());
		} else {
			boxes[idx]->setGeometry(2, sectionViewportPosition(idx)+(sectionSize(idx)-boxes[idx]->minimumSizeHint().height())/2,
					boxes[idx]->minimumSizeHint().width(), boxes[idx]->minimumSizeHint().height());
		}
//		if(isSectionHidden(idx)) boxes[idx]->hide();
	}
}

void CheckboxHeader::reset() {
	QHeaderView::reset();
	int idx=0;
	while(++idx<count()) {	//skip id row
		updateBox(idx);
		boxes[idx]->setChecked(false);
	}
	for(QMap<int,QCheckBox*>::iterator itr=boxes.begin();itr!=boxes.end();) {
		if(itr.key()<idx) ++itr;
		else {
			delete itr.value();
			itr=boxes.erase(itr);
		}
	}
	handleRowsShown();	//show all
	fixBoxPositions();

	setSectionHidden(0,false);	//!fixed - if first section is hidden, it messes up hide/show for QTableView
	setSectionHidden(0,true);		//!while not obvious, this actually solves this issue:
	//after row removal there's hidden id row (which is not hidden, but not shown) on top which can be navigated to with keyboard
	//it also messes up column selection; but if any column is hidden, it's correct until all columns are shown
	//after tab reopens, everything is ok again
}

/*void CheckboxHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
	painter->save();
	QHeaderView::paintSection(painter, rect, logicalIndex);
	painter->restore();
//	if (logicalIndex == 0) {
		QStyleOptionButton option;
		option.rect = QRect(10,30,20,100);
		option.text="Test string";
//		QVariant single_header=QAbstractItemModel::headerData(0,Qt::Vertical);
		sizeHint();
		option.state = QStyle::State_Enabled | QStyle::State_Active;

		if (isChecked_)
			option.state |= QStyle::State_On;
		else
			option.state |= QStyle::State_Off;

//		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
		style()->drawControl(QStyle::CE_CheckBox, &option, painter);
//	}
}*/

void CheckboxHeader::showEvent(QShowEvent *event) {
	for (int idx=1; idx<count(); ++idx) {	//start from 1 to exclude id row
		updateBox(idx);
		if(orientation()==Qt::Horizontal) {
			boxes[idx]->setGeometry(sectionViewportPosition(idx)+(sectionSize(idx)-boxes[idx]->minimumSizeHint().width())/2, 2,
				boxes[idx]->minimumSizeHint().width(), boxes[idx]->minimumSizeHint().height());
		} else {
			boxes[idx]->setGeometry(2, sectionViewportPosition(idx)+(sectionSize(idx)-boxes[idx]->minimumSizeHint().height())/2,
					boxes[idx]->minimumSizeHint().width(), boxes[idx]->minimumSizeHint().height());
		}
//		if(!boxes[idx]->isHidden()) boxes[idx]->show();
		if(isSectionHidden(idx)) boxes[idx]->hide();
	}
	QHeaderView::showEvent(event);
}

void CheckboxHeader::updateBox(int index) {
	if (!boxes.contains(index)) {
		boxes.insert(index, new QCheckBox(this));
	}
}

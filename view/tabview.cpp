//#include <QtWidgets>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QPushButton>
#include <QToolButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
//#include <QEvent>
#include <QStatusTipEvent>
#include <QApplication>

#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>

#include "tabview.h"
#include "dbview.h"
#include "dbdelegates.h"
#include "checkboxheader.h"
#include "collapsiblegroupbox.h"

#include "model/dbmodel.h"
#include "model/addrowproxymodel.h"
#include "model/filterproxymodel.h"
#include "model/horizontalproxymodel.h"

#include "dialogs/changeadddialog.h"
#include "database.h"
#include "mainwindow.h"

bool TabView::advancedOptionsEnabled=false;	//static

TabView::TabView(QWidget *parent)
	: QWidget(parent)
{
	model=Database::getInstance().model;

	proxy=new AddRowProxyModel(this);
	proxy->setSourceModel(model);
	filter=new FilterProxyModel(proxy);
	filter->setSourceModel(proxy);
	adapter=new HorizontalProxyModel(filter);
	adapter->setSourceModel(filter);

	view=new FreezeTableWidget(adapter,this);

	view->verticalHeader()->setDragEnabled(true);
	view->verticalHeader()->setSectionsMovable(false);	//can be changed in UI
	view->verticalHeader()->setSectionsClickable(true);
	view->verticalHeader()->setDragDropMode(QAbstractItemView::InternalMove);

	statusBar=new QStatusBar(this);	//subclass this and join all lambdas there (maybe?)
	statusBar->setSizeGripEnabled(false);
	statusBar->hide();
	connect(statusBar,&QStatusBar::messageChanged,[this](const QString & message){
		if(message.isEmpty()) {
			if(!showStatusMessage()) statusBar->hide();
		} else statusBar->show();
	});
	//status bar should show: Filters: <Names> | Hidden: <Names>
	//tabs could be promoted to separate windows - should still have status bars

	DBDelegate *dbDelegate;
	dbDelegate=qobject_cast<DBDelegate*>(view->itemDelegate());
	if(dbDelegate) {
		connect(dbDelegate,&DBDelegate::validatedEditorCreated,[this](const QString &validatorMessage){
//			statusBar->showMessage(tr("Limits: %1 %2").arg(limits.first.toInt()).arg(limits.second.toInt()));
			statusBar->showMessage(tr("Limits: %1").arg(validatorMessage));
		});
		connect(dbDelegate,&DBDelegate::closeEditor,[this]{
			statusBar->showMessage(QString());
		});
	}	//repeat for frozenTableView:
	dbDelegate=qobject_cast<DBDelegate*>(view->getFrozenItemDelegate());
	if(dbDelegate) {
		connect(dbDelegate,&DBDelegate::validatedEditorCreated,[this](const QString &validatorMessage){
			statusBar->showMessage(tr("Limits: %1").arg(validatorMessage));
		});
		connect(dbDelegate,&DBDelegate::closeEditor,[this]{
			statusBar->showMessage(QString());
		});
	}

	filter->setDynamicSortFilter(true);
	filter->sort(2,Qt::AscendingOrder);

//---main menu dialog---
	QPushButton *submitButton = new QPushButton(tr("Save"));
	submitButton->setDefault(true);
	submitButton->setShortcut(QKeySequence::Save);//Qt::CTRL+Qt::Key_S);
	submitButton->setStatusTip(tr("Save changes to the database"));
	QPushButton *revertButton = new QPushButton(tr("Discard"));
	revertButton->setShortcut(Qt::CTRL+Qt::Key_R);
	revertButton->setStatusTip(tr("Discard unsaved changes"));

	connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
	connect(revertButton,&QPushButton::clicked, [this]{ //model,&QSqlTableModel::revertAll);
		rowIdsToRemove.clear();
		model->revertAll();
	});

	QPushButton *searchButton = new QPushButton(tr("Search"));
	searchButton->setShortcut(QKeySequence::Find);//Qt::CTRL+Qt::Key_F);
	searchButton->setStatusTip(tr("Show columns with matching values"));
	QPushButton *hideButton = new QPushButton(tr("Hide"));
	hideButton->setShortcut(Qt::CTRL+Qt::Key_H);
	hideButton->setStatusTip(tr("Hide selected rows"));
	QPushButton *showAllButton = new QPushButton(tr("Show all"));
	showAllButton->setStatusTip(tr("Show all hidden rows"));
	QPushButton *alterRowButton=new QPushButton(tr("Alter"));
	alterRowButton->setStatusTip(tr("Alter parameters of selected rows"));
	QPushButton *addRowButton=new QPushButton(tr("Add"));
	addRowButton->setStatusTip(tr("Add new row to the database"));
	QPushButton *removeRowButton=new QPushButton(tr("Remove"));
	removeRowButton->setStatusTip(tr("Remove selected rows from the database"));

	connect(hideButton,SIGNAL(clicked(bool)), view,SLOT(handleRowsHidden()));
	connect(showAllButton,SIGNAL(clicked(bool)), view,SLOT(handleRowsShown()));
	CheckboxHeader* checkboxHeader=qobject_cast<CheckboxHeader*>(view->verticalHeader());
	if(checkboxHeader) {
		connect(checkboxHeader,&CheckboxHeader::sectionsSetHidden,[this](const QList<int> &indexList,bool hide){
			if(hide) for(int idx: indexList) {
				if(!statusMessage.second.isNull()) statusMessage.second.append(", ");
				statusMessage.second.append(proxy->headerData(idx,Qt::Horizontal,Qt::DisplayRole).toString());
			} else statusMessage.second.clear();	//when shown, all indexes are shown
		});
	}

	connect(searchButton,&QPushButton::clicked,[this]{
		const QList<int> checkedList=view->getCheckedRows();
		filter->compileFilter(proxy->constructFilter(checkedList));
		view->updateFrozenTableGeometry();

		statusMessage.first.clear();
		for(int idx: checkedList) {
			if(!statusMessage.first.isNull()) statusMessage.first.append(", ");
			statusMessage.first.append(proxy->headerData(idx,Qt::Horizontal,Qt::DisplayRole).toString());
		}
	});
	MainWindow* mainWindow=0;
	for(QWidget* tmp: QApplication::topLevelWidgets()) {
		if(	(mainWindow=qobject_cast<MainWindow*>(tmp))	) break;
	}
	if(mainWindow) {
		connect(mainWindow,&MainWindow::searchRequested,[this,mainWindow](const QMap<QString,QPair<QVariant,QVariant> > &source){
			TabHolder* tabHolder=qobject_cast<TabHolder*>(mainWindow->centralWidget());
			if(tabHolder && tabHolder->currentWidget()==this) {
				filter->compileNamedFilters(source);
				statusBar->showMessage(tr("Filter: compound"));	//is erased with overlapping filters, change later
			}	//only for active tab, don't search in others
		});
	}

	connect(alterRowButton,&QPushButton::clicked,[this]{
		const QList<int> checkedList=view->getCheckedRows();
		if(checkedList.isEmpty()) return;
		ChangeAddDialog changeDialog(checkedList,this);
		connect(&changeDialog,&ChangeAddDialog::alterColumnsRequested,this,&TabView::handleAlterColumns);
		changeDialog.exec();
	});
	connect(addRowButton,&QPushButton::clicked,[this]{
		ChangeAddDialog changeDialog("integer",this);
		connect(&changeDialog,&ChangeAddDialog::addColumnRequested,this,&TabView::handleAddColumn);
		changeDialog.exec();
	});

	connect(removeRowButton,&QPushButton::clicked,[this]{
		QMessageBox::StandardButton clicked = QMessageBox::warning(this,
			tr("Confirm removal"),tr("<center>This operation will remove rows from the database.<br>"
																			"All data in the corresponding field of <strong>every</strong> column<br>"
																			"will be <strong><u>irrevocably</u></strong> lost. Continue?</center>"),
																	 QMessageBox::Ok | QMessageBox::Cancel,
																	 QMessageBox::Cancel);
		if(clicked!=QMessageBox::Ok) return;
		const QList<int> checkedList=view->getCheckedRows();
		if(removeColumns(checkedList)) {
			emit customColumnsRemoved(checkedList);
			proxy->clearRowData();
		}
	});
	connect(view,&FreezeTableWidget::markRowsForRemoval,[this](const QSet<int> &rowIdsRemoved){
		rowIdsToRemove.unite(rowIdsRemoved);
	});
	connect(this,&TabView::customRowsRemoved,
					&Database::getInstance(), &Database::handleRowsRemoved);
	connect(this,&TabView::customColumnsRemoved,
					&Database::getInstance(), &Database::handleColumnsRemoved);

	QVBoxLayout* dialogDBLayout=new QVBoxLayout();
	dialogDBLayout->addWidget(submitButton);
	dialogDBLayout->addWidget(revertButton);
	CollapsibleGroupBox *dbControls=new CollapsibleGroupBox(tr("Database"));
	dbControls->setCollapsive(true);
	dbControls->setLayout(dialogDBLayout);
	dbControls->setStatusTip(tr("Database controls"));

	QVBoxLayout* dialogLayout=new QVBoxLayout();
	dialogLayout->addWidget(searchButton);
	dialogLayout->addWidget(hideButton);
	dialogLayout->addWidget(showAllButton);

	advancedOptions=new QWidget;
	advancedOptions->setObjectName("advancedOptions");	//for collapsible group visibility
	advancedOptions->setProperty("isAdvancedVisible",advancedOptionsEnabled);	//there is property 'visible' already
//	advancedOptions->setVisible(advancedOptionsEnabled);
	//the geometry should be known on setLayout (to setGeometry properly)
	//but setVisible flashes widget on screen as it has no parent at this point - thus hide()
	if(!advancedOptionsEnabled) advancedOptions->hide();
	QVBoxLayout* advancedLayout=new QVBoxLayout;
	advancedLayout->setContentsMargins(0,2,0,0);
	QFrame *lineSeparator=new QFrame();
	lineSeparator->setFrameShape(QFrame::HLine);
	lineSeparator->setFrameShadow(QFrame::Sunken);
	advancedLayout->addWidget(lineSeparator);
	advancedLayout->addWidget(alterRowButton);
	advancedLayout->addWidget(addRowButton);
	advancedLayout->addWidget(removeRowButton);
	advancedOptions->setLayout(advancedLayout);
	dialogLayout->addWidget(advancedOptions);

	if(mainWindow) {
		connect(mainWindow,&MainWindow::toggledAdvancedOptions,this,&TabView::toggleAdvancedOptions);
	}

	CollapsibleGroupBox *allColumnsControls=new CollapsibleGroupBox(tr("Rows"));
	allColumnsControls->setCollapsive(true);
	allColumnsControls->setLayout(dialogLayout);
	allColumnsControls->setStatusTip(tr("Row controls"));

//---new row submenu---
	QPushButton *submitRowButton = new QPushButton(tr("Write"));
	submitRowButton->setStatusTip(tr("Write new column to the table"));
	QPushButton *clearRowButton = new QPushButton(tr("Clear"));
	clearRowButton->setStatusTip(tr("Clear entered data from the new column"));

	connect(submitRowButton, SIGNAL(clicked()), proxy, SLOT(submitRow()));

//	not great - multiple proxies from tabs listen to one signal, latest writes
//	connect(model,&QSqlTableModel::primeInsert,proxy,&AddRowProxyModel::populateRecord);

	connect(clearRowButton, SIGNAL(clicked()), proxy, SLOT(clearRowData()));

	CollapsibleGroupBox *newRowControls=new CollapsibleGroupBox(tr("New column"));
	newRowControls->setCollapsive(true);
	newRowControls->setStatusTip(tr("New column controls (collapsible)"));
	QVBoxLayout *innerLayout=new QVBoxLayout();
	innerLayout->addWidget(submitRowButton);
	innerLayout->addWidget(clearRowButton);
	newRowControls->setLayout(innerLayout);
	connect(newRowControls, SIGNAL(toggled(bool)), view, SLOT(setFrozenVisible(bool)));

//---layout---
	QVBoxLayout *sidebarLayout = new QVBoxLayout;
	newRowControls->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	sidebarLayout->addWidget(newRowControls);
	sidebarLayout->addStrut(fontInfo().pixelSize()*10.5);	//115 - to avoid size change when groupboxes are collapsed
	sidebarLayout->addStretch(0);
//	sidebarLayout->addSpacerItem(new QSpacerItem(115,10,QSizePolicy::Minimum,QSizePolicy::Expanding));
//	buttonBox->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
	sidebarLayout->addWidget(allColumnsControls);
	sidebarLayout->addWidget(dbControls);
//	sidebarLayout->setSpacing(10);

	QVBoxLayout *viewLayout=new QVBoxLayout;
	viewLayout->addWidget(view);
	viewLayout->addWidget(statusBar);
	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->addLayout(sidebarLayout);
	mainLayout->addLayout(viewLayout);
	setLayout(mainLayout);
}
void TabView::setVerticalNumberRepresentation(bool vertical) {
	adapter->setVerticalRepresentation(vertical);
}
void TabView::toggleAdvancedOptions(bool isAdvanced) {
//	if(advancedOptionsEnabled!=isAdvanced) {	//other tabs need to update
			advancedOptionsEnabled=isAdvanced;
			advancedOptions->setVisible(advancedOptionsEnabled);
			//when the group box is collapsed, it yields greyed out advancedOptions shown
			//actually a feature (easy to solve, but better for user this way)
			advancedOptions->setProperty("isAdvancedVisible",advancedOptionsEnabled);
//	}
}
void TabView::restoreVhState(const QPair<QMap<int,int>, QList<int> > &vhState) {
	for(int visualIdx=0; visualIdx<vhState.first.size();++visualIdx) {	//map is sorted by key
		view->verticalHeader()->moveSection(
					view->verticalHeader()->visualIndex(vhState.first.value(visualIdx)),visualIdx);
	}
	statusMessage.second.clear();
	for(int logicalIdx: vhState.second) {
		view->setRowHidden(logicalIdx,true);	//also hides in frozenTableView
		if(logicalIdx==0) continue;	//skip id
		if(!statusMessage.second.isNull()) statusMessage.second.append(", ");
		statusMessage.second.append(proxy->headerData(logicalIdx,Qt::Horizontal,Qt::DisplayRole).toString());
	}
	showStatusMessage();
}
QPair<QMap<int,int>, QList<int> > TabView::saveVhState() const {
	QPair<QMap<int,int>, QList<int> > result;	//visual, logical, hiddenLogical
	for(int logicalIdx=0; logicalIdx<view->verticalHeader()->count(); ++logicalIdx) {
		result.first.insert(view->verticalHeader()->visualIndex(logicalIdx),logicalIdx);
		if(view->verticalHeader()->isSectionHidden(logicalIdx)) result.second.append(logicalIdx);
	}
	return result;
}

bool TabView::event(QEvent * event) {
	if(event->type()==QEvent::StatusTip) {
		statusBar->showMessage(static_cast<QStatusTipEvent*>(event)->tip());
		return true;
	}
	return QWidget::event(event);
}
bool TabView::showStatusMessage(void) const {
	if(statusMessage.first.isNull() && statusMessage.second.isNull()) return false;
	QString tmpMessage;
	if(!statusMessage.first.isNull())
		tmpMessage.append(tr("Filter: ")+statusMessage.first
											+(!statusMessage.second.isNull()?"; ":""));
	if(!statusMessage.second.isNull())
		tmpMessage.append(tr("Hidden: ")+statusMessage.second);
	statusBar->showMessage(tmpMessage);
	return true;
}

void TabView::submit() {
	model->database().transaction();
	if (model->submitAll()) {
		model->database().commit();
		if(!rowIdsToRemove.isEmpty()) {
			emit customRowsRemoved(rowIdsToRemove);
			rowIdsToRemove.clear();
		}
	} else {
		model->database().rollback();
		QMessageBox::warning(this, tr("Tab view"),
			tr("The database reported an error: %1")
			.arg(model->lastError().text()));
	}
}
bool TabView::removeColumns(const QList<int> &columns) {	//SQLite doesn't support direct column removal
	if(columns.empty()) return false;
	QSqlRecord emptyRecord=model->record();
	QString newFields, newTypedFields;
	const QMap<QString,QList<QVariant> > &settingsDB=Database::getInstance().settings.database;
	for(int idx=0; idx<emptyRecord.count();++idx) {
		if(!columns.contains(idx)) {
			if(!newFields.isEmpty()) {
				newFields.append(",");
				newTypedFields.append(", ");
			}
			newFields.append(emptyRecord.fieldName(idx));
			newTypedFields.append(emptyRecord.fieldName(idx));
//			if(!emptyRecord.fieldName(idx).compare("id")) newTypedFields.append(" integer primary key");
			if(settingsDB.contains(emptyRecord.fieldName(idx))) {
				newTypedFields.append(" ");
				newTypedFields.append(settingsDB.value(emptyRecord.fieldName(idx)).value(1).toString());
			}
		}
	}
	model->database().transaction();
	QSqlQuery query(model->database());
	bool queryResult=true;
	queryResult&=query.exec(QString("CREATE TABLE weather_temp(").append(newTypedFields).append(")"));
	queryResult&=query.exec(QString("INSERT INTO weather_temp SELECT ").append(newFields).append(" FROM weather"));
	queryResult&=query.exec("DROP TABLE weather");
	queryResult&=query.exec("ALTER TABLE weather_temp RENAME TO weather");
	if (queryResult) {
		model->database().commit();
		model->setTable("weather");
		model->select();	//emits modelAboutToBeReset and modelReset afterwards
		//doesn't emit columnsAboutToBeRemoved, thus marks are not updated from here

		Database::getInstance().settings.db_create["weather"]=newTypedFields;
	} else {
		model->database().rollback();
		QMessageBox::warning(this, tr("Tab view"),
			tr("The database reported an error: %1")
			.arg(model->lastError().text()));
	}
	Database::getInstance().settings.refreshDatabase(model);	//remove headers from settings
	return queryResult;
}
void TabView::handleAlterColumns(bool isDBchanged, const QList<QList<QVariant> > &newHeaders) {
//newHeaders: oldIdx, if(isDBchanged)(newIdx,name), limits/colors
	if(newHeaders.isEmpty()) return;
	QMap<QString,QList<QVariant> > &settingsDB=Database::getInstance().settings.database;	//could change
	QMap<QString,int> limitsMapping;	//needed for changing limits after indexes had been changed
	QSqlRecord emptyRecord=model->record();
	if(isDBchanged) {
		QString newFields, newTypedFields, oldFields;

		QMap<int, int> indexMapping;	//used to get new ordering
		QList<QString> uniqueNames;	//lower case because DB is case-insensitive (for ASCII, not for Unicode)
		for(int idx=0; idx<emptyRecord.count();++idx) {
			indexMapping.insert(idx,idx);
			uniqueNames.append(emptyRecord.fieldName(idx).toLower());
		}
		QMap<int, int> headerMapping;	//(oldIdx -> headerIdx)
		int headerIndex=0;
		QMap<int,int> newToOld;	//for sorting
		//insertMulti reverses indexes with the same newIdx, but second insertMulti fixes it
		for(const QList<QVariant> &entry: newHeaders) {
			int oldIdx=entry.value(0).toInt();
			headerMapping.insert(oldIdx, headerIndex++);
			newToOld.insertMulti(entry.value(1).toInt(), oldIdx);
		}
		for(int idx: newToOld) indexMapping.remove(idx);	//should return 1
		for(QMap<int,int>::iterator itr=newToOld.begin(); itr!=newToOld.end(); ++itr) {
			int newIdx=itr.key();
			if(newIdx<1) newIdx=1;	//don't allow to go above ID column
			indexMapping.insertMulti(newIdx+1,itr.value());	//newest inserted go first in values()
			//+1 here because new item should go below old one (more intuitive)
//			uniqueNames[itr.value()].clear();	//switching names is too complicated of a case, two calls with temp names should do
		}
		QList<int> ordering=indexMapping.values();
		QMap<int, QString> changedNames;	//to update settings after commit
		for(int idx=0; idx<emptyRecord.count();++idx) {
			if(!oldFields.isEmpty()) oldFields.append(",");
			QString newFieldName=emptyRecord.fieldName(ordering.value(idx));
			oldFields.append(newFieldName);	//rearrange fields
			QString fieldType=settingsDB.value(newFieldName).value(1).toString();
			if(!newFields.isEmpty()) {
				newFields.append(",");
				newTypedFields.append(", ");
			}
			if(headerMapping.contains(ordering.value(idx))) {
				QString fieldNameCandidate=newHeaders.value(headerMapping.value(ordering.value(idx))).value(2).toString();
				if(!fieldNameCandidate.isEmpty()
				&& uniqueNames.value(ordering.value(idx)).compare(fieldNameCandidate,Qt::CaseInsensitive)
				&& !uniqueNames.contains(fieldNameCandidate.toLower())) {
					newFieldName=fieldNameCandidate;
					uniqueNames[ordering.value(idx)]=newFieldName.toLower();
					changedNames[idx]=newFieldName;
				}
				limitsMapping.insert(newFieldName,headerMapping.value(ordering.value(idx)));
			}
			newFields.append(newFieldName);
			newTypedFields.append(newFieldName);
//			if(settingsDB.contains(emptyRecord.fieldName(idx))) {
			if(!fieldType.isNull()) {
				newTypedFields.append(" ");
				newTypedFields.append(fieldType);
			}
		}

		model->database().transaction();
		QSqlQuery query(model->database());
		bool queryResult=true;
		queryResult&=query.exec(QString("CREATE TABLE weather_temp(").append(newTypedFields).append(")"));
		queryResult&=query.exec(QString("INSERT INTO weather_temp(").append(newFields).append(") SELECT ")
														.append(oldFields).append(" FROM weather"));
		queryResult&=query.exec("DROP TABLE weather");
		queryResult&=query.exec("ALTER TABLE weather_temp RENAME TO weather");
		if (queryResult) {
			model->database().commit();
			model->setTable("weather");
			model->select();	//emits modelAboutToBeReset and modelReset afterwards

			Database::getInstance().handleColumnsRearranged(ordering);	//update marks

			QMap<QString,QList<QVariant> > valuesToInsert;
			for(QMap<QString,QList<QVariant> >::iterator itr=settingsDB.begin();itr!=settingsDB.end();) {
				int newIdx=ordering.indexOf(itr.value().value(0).toInt());
				itr.value()[0]=newIdx;
				if(changedNames.contains(newIdx)) {
					valuesToInsert.insert(changedNames.value(newIdx),itr.value());
					itr=settingsDB.erase(itr);
				} else ++itr;
			}
			settingsDB.unite(valuesToInsert);

			Database::getInstance().settings.db_create["weather"]=newTypedFields;
		} else {
			model->database().rollback();
			QMessageBox::warning(this, tr("Tab view"),
				tr("The database reported an error: %1")
				.arg(model->lastError().text()));
		}
	}
	//to change limits in settings
	if(limitsMapping.isEmpty()) {
		int headerIdx=0;
		for(const QList<QVariant> &entry: newHeaders) {
			limitsMapping.insert(emptyRecord.fieldName(entry.value(0).toInt()),headerIdx++);
		}
	}
	for(QMap<QString,int>::const_iterator itr=limitsMapping.constBegin(); itr!=limitsMapping.constEnd();++itr) {
		QList<QVariant> &dbRecord=settingsDB[itr.key()];
		int sizeBeforeLimits=3;
		while(dbRecord.size()>sizeBeforeLimits) dbRecord.removeLast();
		const QList<QVariant> &dbList=newHeaders.value(itr.value()).mid(isDBchanged ? 3 : 1);
		for(const QVariant &data: dbList) {
			dbRecord.append(data);
		}
	}
}
void TabView::handleAddColumn(const QList<QVariant> &record) {
//record: newIdx, name, isMark, type, limits/colors, default
	QString columnName=record.value(1).toString();
	if(columnName.isEmpty()) return;
	QSqlRecord emptyRecord=model->record();
	for(int idx=0; idx<emptyRecord.count();++idx) {
		if(!columnName.compare(emptyRecord.fieldName(idx),Qt::CaseInsensitive)) return;
	}
	QList<QVariant> dbList;
	dbList.append(record.value(0));
	if(dbList.last().toInt()!=emptyRecord.count()) return;	//append at the end
	dbList.append(record.value(3));
	dbList.append(record.value(2));
	for(const QVariant &data: record.mid(4,record.size()-5)) {	//leave last element
		dbList.append(data);
	}
	QVariant defaultValue=record.last();

	model->database().transaction();
	QSqlQuery query(model->database());
	bool queryResult=true;
	QString queryString=QString("ALTER TABLE weather ADD COLUMN ").append(columnName).append(" ")
			.append(dbList.value(1).toString());
	if(!defaultValue.isNull()) {
		queryString.append(" DEFAULT ").append(defaultValue.toString());
	}
	queryResult&=query.exec(queryString);
	if (queryResult) {
		model->database().commit();
		model->setTable("weather");
		model->select();	//to reset model

		QString &createSetting=Database::getInstance().settings.db_create["weather"];
		if(!createSetting.isEmpty()) createSetting.append(", ");
		createSetting.append(columnName).append(" ").append(dbList.value(1).toString());
	} else {
		model->database().rollback();
		QMessageBox::warning(this, tr("Tab view"),
			tr("The database reported an error: %1")
			.arg(model->lastError().text()));
	}
	if(queryResult) {
		Database::getInstance().settings.database.insert(columnName,dbList);
	}
}

TabHolder::TabHolder(QWidget *parent) : QTabWidget(parent), vertical(true) {
	setMovable(true);
	setTabsClosable(true);
	setUsesScrollButtons(true);
	setTabShape(QTabWidget::Rounded);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTabView(int)));

	QToolButton *tb = new QToolButton();
	tb->setText("+");
	tb->setAutoRaise(true);
	setCornerWidget(tb,Qt::TopRightCorner);
	connect(tb, SIGNAL(clicked()), this, SLOT(addTab()));

	tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(tabBar(),&QTabBar::customContextMenuRequested,this,&TabHolder::customTabMenuRequested);
}
void TabHolder::customTabMenuRequested(const QPoint &pos) {
	QMenu contextMenu(tr("Tab context menu"), this);
	QAction action1(tr("Rename"), this);
	connect(&action1, &QAction::triggered, [this,&pos]{
		int idx=tabBar()->tabAt(pos);
		bool ok;
		QString tmp_str=QInputDialog::getText(this,tr("Input tab name"),tr("Tab name"),
																					QLineEdit::Normal,tabText(idx),&ok);
		if(ok && !tmp_str.isEmpty())
			setTabText(idx,tmp_str);
	});
	contextMenu.addAction(&action1);
	contextMenu.exec(mapToGlobal(pos));
}
void TabHolder::addTab(void) {
	TabView* tmp=new TabView();
	tmp->setVerticalNumberRepresentation(vertical);

	TabView* currentTab=qobject_cast<TabView*>(currentWidget());
	if(currentTab) {
		tmp->restoreVhState(currentTab->saveVhState());
	}

	QTabWidget::insertTab(currentIndex()+1,tmp,tr("New tab"));
}

void TabHolder::removeTabView(int idx) {
	QWidget* tmp=widget(idx);
	removeTab(idx);
	delete tmp;

	//if no tabs left
	if(!count()) {
		addTab();	//revise later
	}
}

void TabHolder::handleNumberRepresentationToggle(bool vert) {
	for(int idx=0; idx<count(); ++idx) {
		TabView* tmp=qobject_cast<TabView*>(widget(idx));	//TabView should have Q_OBJECT
		if(tmp) tmp->setVerticalNumberRepresentation(vert);
	}
	vertical=vert;
}

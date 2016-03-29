//#include <QtWidgets>
//components used (from QtWidgets):
#include <QGridLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QColorDialog>

#include <limits>

#include "changeadddialog.h"
#include "view/dbdelegates.h"
#include "database.h"

ChangeAddDialog::ChangeAddDialog(const QList<int> &checked, QWidget *parent)
	: QDialog(parent), state(Alter), enableDBEdit(0)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QGridLayout *mainLayout=new QGridLayout;

	const QMap<QString,QList<QVariant> > &dbMap=Database::getInstance().settings.database;
	QMap<int,QString> ordering, orderingMarked;
	for(QMap<QString,QList<QVariant> >::const_iterator itr=dbMap.constBegin(); itr!=dbMap.constEnd(); ++itr) {
		int key=itr.value().value(0).toInt();
		if(key && checked.contains(key)) {	//skip id and invalid entries, add only checked
			if(itr.value().value(2).toInt()>0) orderingMarked.insert(key,itr.key());
			else ordering.insert(key,itr.key());
		}
	}
	int row=0;
	enableDBEdit = new QCheckBox(tr("Allow database to be changed"));
	enableDBEdit->setChecked(false);
	mainLayout->addWidget(enableDBEdit,row++,0,1,-1,Qt::AlignLeft);
	for(const QString &name: orderingMarked) {
		QList<QWidget*> record;
		int column=0;
		for(QWidget* item: createEntryTitle(name,dbMap.value(name).value(0).toInt())) {
			mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
			record.append(item);
		}
		indexMapping[row]=dbMap.value(name).value(0).toInt();
		QHBoxLayout *markLayout=new QHBoxLayout;
		for(QWidget* item: createColorCombo(dbMap.value(name).mid(3))) {
			markLayout->addWidget(item);
			record.append(item);
		}
		mainLayout->addLayout(markLayout,row,column,1,-1,Qt::AlignLeft);
		++row;
		contents.append(record);
	}
	if(!ordering.isEmpty()) {
		mainLayout->addWidget(new QLabel(tr("Number")),row,0,Qt::AlignLeft);
		mainLayout->addWidget(new QLabel(tr("Row name")),row,1,Qt::AlignLeft);
		mainLayout->addWidget(new QLabel(tr("Minimum")),row,2,Qt::AlignLeft);
		mainLayout->addWidget(new QLabel(tr("Maximum")),row,3,Qt::AlignLeft);
		++row;
	}
	for(const QString &name: ordering) {
		QList<QWidget*> record;
		int column=0;
		for(QWidget* item: createEntryTitle(name,dbMap.value(name).value(0).toInt())) {
			mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
			record.append(item);
		}
		indexMapping[row-1]=dbMap.value(name).value(0).toInt();
		for(QWidget* item: createNumberEditor(dbMap.value(name).value(1).toString(),dbMap.value(name).mid(3),false)) {
			mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
			record.append(item);
		}
		++row;
		contents.append(record);
	}

	QPushButton *confirm=new QPushButton(tr("Write"));
	confirm->setDefault(true);
	QPushButton *reject=new QPushButton(tr("Cancel"));
	QDialogButtonBox *buttons=new QDialogButtonBox(Qt::Horizontal);
	buttons->addButton(confirm,QDialogButtonBox::AcceptRole);
	buttons->addButton(reject,QDialogButtonBox::RejectRole);
	connect(confirm,&QPushButton::clicked,this,&ChangeAddDialog::accept);
	connect(reject,&QPushButton::clicked,this,&ChangeAddDialog::reject);

	mainLayout->addWidget(buttons,row,0,1,-1,Qt::AlignRight);
	setLayout(mainLayout);
//	layout()->setSizeConstraint(QLayout::SetFixedSize);
	setWindowTitle(tr("Change rows"));
}
ChangeAddDialog::ChangeAddDialog(const QString &defaultType, QWidget *parent)
	: QDialog(parent), state(Add), enableDBEdit(0)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QGridLayout *mainLayout=new QGridLayout;

	int row=0, column=0;
	mainLayout->addWidget(new QLabel(tr("Number")),row,0,Qt::AlignLeft);
	mainLayout->addWidget(new QLabel(tr("Row name")),row,1,Qt::AlignLeft);
	mainLayout->addWidget(new QLabel(tr("Mark")),row,2,Qt::AlignLeft);
	QLabel *numberLabel=new QLabel(tr("Value"));
	mainLayout->addWidget(numberLabel,row,3,Qt::AlignLeft);
	QLabel *minLabel=new QLabel(tr("Minimum"));
	mainLayout->addWidget(minLabel,row,4,Qt::AlignLeft);
	QLabel *maxLabel=new QLabel(tr("Maximum"));
	mainLayout->addWidget(maxLabel,row,5,Qt::AlignLeft);
	QLabel *defaultLabel=new QLabel(tr("Default"));
	mainLayout->addWidget(defaultLabel,row,6,Qt::AlignLeft);
	++row;

	QList<QWidget*> record;
	bool tmpFirstItem=true;
	for(QWidget* item: createEntryTitle(QString(),Database::getInstance().settings.database.size())) {
		mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
		record.append(item);
		if(tmpFirstItem) {
			item->setEnabled(false);
			tmpFirstItem=false;
		}
	}
	QCheckBox *checkMark=new QCheckBox;
	checkMark->setChecked(false);
	mainLayout->addWidget(checkMark,row,column++,Qt::AlignCenter);
	record.append(checkMark);
	QComboBox *typeCombo=new QComboBox;
	typeCombo->addItem(tr("Integer"),QString("integer"));
	typeCombo->addItem(tr("Real"),QString("real"));
	int currentIndex=-1;
	for(int idx=0; idx<typeCombo->count(); ++idx) {
		if(!typeCombo->itemData(idx).toString().compare(defaultType)) {
			currentIndex=idx;
			break;
		}
	}
	typeCombo->setCurrentIndex(currentIndex);
	mainLayout->addWidget(typeCombo,row,column++,Qt::AlignLeft);
	record.append(typeCombo);
//	++row;
	contents.append(record); record.clear();
	int columnCut=column;
	for(QWidget* item: createNumberEditor("integer",QList<QVariant>(),true)) {
		mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
		record.append(item);
	}
	if(typeCombo->currentData().toString().compare("integer")) {
		for(QWidget* item: record) {
			item->hide();
		}
	}
//	++row;
	column=columnCut;
	contents.append(record); record.clear();
	for(QWidget* item: createNumberEditor("real",QList<QVariant>(),true)) {
		mainLayout->addWidget(item,row,column++,Qt::AlignLeft);
		record.append(item);
	}
	if(typeCombo->currentData().toString().compare("real")) {
		for(QWidget* item: record) {
			item->hide();
		}
	}
//	++row;
	column=columnCut;
	contents.append(record); record.clear();

	connect(typeCombo,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),	//because of overload
	[this](int newIdx) {
		for(QWidget* item: contents.value(newIdx+1)) {	//show first to avoid dialog shrinking
			item->show();
		}
		for(int listIdx=0; listIdx<2; ++listIdx) if(listIdx!=newIdx) {
			for(QWidget* item: contents.value(listIdx+1)) {
				item->hide();
			}
		}
	});

	QHBoxLayout *markLayout=new QHBoxLayout;
	for(QWidget* item: createColorCombo(QList<QVariant>())) {
		markLayout->addWidget(item);
		record.append(item);
	}
	mainLayout->addLayout(markLayout,row,column,1,-1,Qt::AlignLeft);
	if(checkMark->isChecked()) {
		numberLabel->hide();
		minLabel->hide();
		maxLabel->hide();
		defaultLabel->hide();
		typeCombo->hide();
		for(int listIdx=0; listIdx<2; ++listIdx) {
			for(QWidget* item: contents.value(listIdx+1)) {
				item->hide();
			}
		}
	} else {
		for(QWidget* item: record) {
			item->hide();
		}
	}

	connect(checkMark,&QCheckBox::toggled,[=](bool isChecked){
		if(isChecked) {
			for(QWidget* item: contents.value(3)) {
				item->show();
			}
			numberLabel->hide();
			minLabel->hide();
			maxLabel->hide();
			defaultLabel->hide();
			typeCombo->hide();
			for(int listIdx=0; listIdx<2; ++listIdx) {
				for(QWidget* item: contents.value(listIdx+1)) {
					item->hide();
				}
			}
		} else {
			numberLabel->show();
			minLabel->show();
			maxLabel->show();
			defaultLabel->show();
			typeCombo->show();
			for(QWidget* item: contents.value(typeCombo->currentIndex()+1)) {
				item->show();
			}
			for(QWidget* item: contents.value(3)) {
				item->hide();
			}
		}
	});

	++row;
	contents.append(record); record.clear();

	QPushButton *confirm=new QPushButton(tr("Write"));
	confirm->setDefault(true);
	QPushButton *reject=new QPushButton(tr("Cancel"));
	QDialogButtonBox *buttons=new QDialogButtonBox(Qt::Horizontal);
	buttons->addButton(confirm,QDialogButtonBox::AcceptRole);
	buttons->addButton(reject,QDialogButtonBox::RejectRole);
	connect(confirm,&QPushButton::clicked,this,&ChangeAddDialog::accept);
	connect(reject,&QPushButton::clicked,this,&ChangeAddDialog::reject);

	mainLayout->addWidget(buttons,row,0,1,-1,Qt::AlignRight);
	setLayout(mainLayout);
	layout()->setSizeConstraint(QLayout::SetFixedSize);	//auto-resize
	setWindowTitle(tr("New row"));
}

QList<QWidget*> ChangeAddDialog::createEntryTitle(const QString &name, int index) const {
	QList<QWidget*> result;

	QSpinBox *number=new QSpinBox;
	number->setRange(1,99);
	number->setValue(index);
	number->setButtonSymbols(QSpinBox::NoButtons);
	result.append(number);
	QLineEdit *label=new QLineEdit(name);
	result.append(label);

	if(enableDBEdit) {
		number->setEnabled(enableDBEdit->isChecked());
		label->setEnabled(enableDBEdit->isChecked());
		connect(enableDBEdit,&QCheckBox::toggled,[=](bool checked){
			number->setEnabled(checked);
			label->setEnabled(checked);
		});
	}
	return result;
}
QList<QWidget*> ChangeAddDialog::createColorCombo(const QList<QVariant> &parameters) const {
	QList<QWidget*> result;
	DBColorCombo *colorCombo=new DBColorCombo;
	for(const QVariant &item: parameters) {
		colorCombo->addItem(item.toString(),QColor(item.toString()));
	}
	result.append(colorCombo);
	QPushButton *plus=new QPushButton(tr("+"));
	QPushButton *minus=new QPushButton(tr("-"));
	plus->setFixedWidth(20);
	minus->setFixedWidth(20);
	result.append(plus); result.append(minus);
	connect(plus,&QPushButton::clicked,[=]{
//		const QColorDialog::ColorDialogOptions options=QColorDialog::ShowAlphaChannel;
		const QColor color = QColorDialog::getColor(Qt::white, colorCombo, tr("Select mark color"));//, options);
		if(color.isValid()) {
			colorCombo->addItem(color.name(),color);
		}
	});
	connect(minus,&QPushButton::clicked,[=]{
		colorCombo->removeItem(colorCombo->currentIndex());
	});
	return result;
}
QList<QWidget*> ChangeAddDialog::createNumberEditor(const QString &type, const QList<QVariant> &parameters, bool setDefault) const {
	QList<QWidget*> result;
	DBEditor *editLeft=new DBEditor(type,parameters);
	editLeft->setValue(parameters.value(0));
//	if(editor->getType()==DBEditor::Unknown); //acts like normal QLineEdit
	DBEditor *editRight=new DBEditor(type,parameters);
	editRight->setValue(parameters.value(1));
	DBEditor *defaultEditor=0;
	if(setDefault) {
		defaultEditor=new DBEditor(type,parameters);
	}

	switch(editLeft->getType()) {	//editRight->getType() is the same
	case DBEditor::DoubleData:
		editLeft->setValidatorRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
		editRight->setValidatorRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
		if(defaultEditor) defaultEditor->setValidatorRange(std::numeric_limits<double>::lowest(),std::numeric_limits<double>::max());
		break;
	case DBEditor::IntData:
		editLeft->setValidatorRange(std::numeric_limits<int>::lowest(),std::numeric_limits<int>::max());
		editRight->setValidatorRange(std::numeric_limits<int>::lowest(),std::numeric_limits<int>::max());
		if(defaultEditor) defaultEditor->setValidatorRange(std::numeric_limits<int>::lowest(),std::numeric_limits<int>::max());
		break;
	case DBEditor::Unknown:
	default:
		break;
	}
	result.append(editLeft);
	result.append(editRight);
	if(defaultEditor) result.append(defaultEditor);
	return result;
}

QList<QList<QVariant> > ChangeAddDialog::getResults() {
	QList<QList<QVariant> > result;
	if(state==Alter) {
		int visualIndex=1;	//starts from 1 in the map (filled this way)
		for(QList<QWidget*> &record: contents) {
			QList<QVariant> resultRecord;
			resultRecord.append(indexMapping.value(visualIndex));
			if(enableDBEdit && enableDBEdit->isChecked()) {
				{	QSpinBox* tmp=qobject_cast<QSpinBox*>(record.value(0));
					if(tmp) resultRecord.append(tmp->value());
				}{QLineEdit* tmp=qobject_cast<QLineEdit*>(record.value(1));
					if(tmp) resultRecord.append(tmp->text().remove(',').simplified().remove(' '));
				}	//avoid commas and whitespace to keep database functional
			}
			{	DBColorCombo* tmp=qobject_cast<DBColorCombo*>(record.value(2));
				if(tmp) {
					for(int idx=0; idx<tmp->count(); ++idx) {
						resultRecord.append(tmp->itemText(idx));
					}
					result.append(resultRecord);
					++visualIndex;
					continue;
				}
			}{DBEditor* tmp=qobject_cast<DBEditor*>(record.value(2));
				if(tmp) resultRecord.append(tmp->getValue());
			}{DBEditor* tmp=qobject_cast<DBEditor*>(record.value(3));
				if(tmp) resultRecord.append(tmp->getValue());
			}
			result.append(resultRecord);
			++visualIndex;
		}
	} else if(state==Add) {
		QList<QVariant> resultRecord;
		const QList<QWidget*> &record=contents.value(0);
		{	QSpinBox* tmp=qobject_cast<QSpinBox*>(record.value(0));
			if(tmp) resultRecord.append(tmp->value());
		}{QLineEdit* tmp=qobject_cast<QLineEdit*>(record.value(1));
			if(tmp) resultRecord.append(tmp->text().remove(',').simplified().remove(' '));
		}	bool isMark=false;
		{	QCheckBox* tmp=qobject_cast<QCheckBox*>(record.value(2));
			if(tmp) {
				isMark=tmp->isChecked();
				resultRecord.append(isMark ? 1 : 0);
			}
		}
		int currentIdx=-1;
		if(!isMark) {
			QComboBox* tmp=qobject_cast<QComboBox*>(record.value(3));
			if(tmp) {
				currentIdx=tmp->currentIndex();
				resultRecord.append(tmp->currentData());
			}
		} else {
			resultRecord.append(QString("integer"));
		}
		if(currentIdx<0) {
			DBColorCombo* tmp=qobject_cast<DBColorCombo*>(contents.value(3).value(0));
			if(tmp) {
				for(int idx=0; idx<tmp->count(); ++idx) {
					resultRecord.append(tmp->itemText(idx));
				}
				resultRecord.append(tmp->currentIndex());	//default value
			}
		} else {
			{DBEditor* tmp=qobject_cast<DBEditor*>(contents.value(currentIdx+1).value(0));
				if(tmp) resultRecord.append(tmp->getValue());
			}{DBEditor* tmp=qobject_cast<DBEditor*>(contents.value(currentIdx+1).value(1));
				if(tmp) resultRecord.append(tmp->getValue());
			}{DBEditor* tmp=qobject_cast<DBEditor*>(contents.value(currentIdx+1).value(2));
				if(tmp) resultRecord.append(tmp->getValue());	//default value
			}
		}
		result.append(resultRecord);
	}
	return result;
}
void ChangeAddDialog::accept() {
	if(state==Alter) {
		emit alterColumnsRequested((enableDBEdit && enableDBEdit->isChecked()),getResults());
	} else if(state==Add) {
		emit addColumnRequested(getResults().value(0));
	}
	QDialog::accept();
}

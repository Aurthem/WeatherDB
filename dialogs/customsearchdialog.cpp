//#include <QtWidgets>
//components used (from QtWidgets):
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

#include "customsearchdialog.h"
#include "database.h"
#include "view/dbdelegates.h"

CustomSearchDialog::CustomSearchDialog(QWidget *parent)
	: QDialog(parent) {
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout *mainLayout=new QVBoxLayout;

	const QMap<QString,QList<QVariant> > &dbMap=Database::getInstance().settings.database;
	QMap<int,QString> ordering;
	for(QMap<QString,QList<QVariant> >::const_iterator itr=dbMap.constBegin(); itr!=dbMap.constEnd(); ++itr) {
		int key=itr.value().value(0).toInt();
		if(key && itr.value().value(2).toInt()!=1)	//skip id and invalid entries; skip "mark" type entries
			ordering.insert(key,itr.key());
	}
//	for(QMap<QString,QList<QVariant> >::const_iterator itr=dbMap.constBegin(); itr!=dbMap.constEnd(); ++itr) {
//		QHBoxLayout* entry=createEntry(itr.key(),itr.value());
//		if(entry) mainLayout->addLayout(entry);
//	}
	for(const QString &name: ordering) {
//		QHBoxLayout* entry=createEntry(name,dbMap.value(name));
//		if(entry) mainLayout->addLayout(entry);
		mainLayout->addLayout(createEntry(name,dbMap.value(name)));
	}

	QPushButton *confirm=new QPushButton(tr("Найти"));
	confirm->setDefault(true);
	QPushButton *reject=new QPushButton(tr("Отмена"));
	QDialogButtonBox *buttons=new QDialogButtonBox(Qt::Horizontal);
	buttons->addButton(confirm,QDialogButtonBox::AcceptRole);
	buttons->addButton(reject,QDialogButtonBox::RejectRole);
	connect(confirm,&QPushButton::clicked,this,&CustomSearchDialog::accept);
	connect(reject,&QPushButton::clicked,this,&CustomSearchDialog::reject);

	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
	setWindowTitle(tr("Поисковой запрос"));
}
QHBoxLayout* CustomSearchDialog::createEntry(const QString &name, const QList<QVariant> &parameters) const {
//	if(parameters.value(0).toInt()==0) return 0;	//skip id and also invalid entries
//	if(parameters.value(2).toInt()==1) return 0;	//also skip "mark" type entries

	QHBoxLayout *result=new QHBoxLayout;
	QLabel *label=new QLabel(name);
	DBEditor *editLeft=new DBEditor(parameters.value(1).toString(),parameters.mid(3));
//	if(editor->getType()==DBEditor::Unknown); //acts like normal QLineEdit
	DBEditor *editRight=new DBEditor(parameters.value(1).toString(),parameters.mid(3));

	result->addWidget(editLeft);
	result->addWidget(label);
	result->addWidget(editRight);
	return result;
}
QMap<QString, QPair<QVariant,QVariant> > CustomSearchDialog::getSearchQuery() const {
	QMap<QString, QPair<QVariant,QVariant> > result;
//	layout()->children();
//	this->children();
	for(int idx=0; idx<layout()->count(); ++idx) {
		QLayout* entryLayout=layout()->itemAt(idx)->layout();
		if(entryLayout) {
			DBEditor *editLeft=qobject_cast<DBEditor*>(entryLayout->itemAt(0)->widget());
			QLabel *label=qobject_cast<QLabel*>(entryLayout->itemAt(1)->widget());
			DBEditor *editRight=qobject_cast<DBEditor*>(entryLayout->itemAt(2)->widget());
			if(editLeft && label && editRight) {	//correct cast was done
				result.insert(label->text(),QPair<QVariant,QVariant>(editLeft->getValue(),editRight->getValue()));
			}
		}
	}
	return result;
}

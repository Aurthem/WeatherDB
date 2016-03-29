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
	for(const QString &name: ordering) {
		mainLayout->addLayout(createEntry(name,dbMap.value(name)));
	}

	QPushButton *confirm=new QPushButton(tr("Search"));
	confirm->setDefault(true);
	QPushButton *reject=new QPushButton(tr("Cancel"));
	QDialogButtonBox *buttons=new QDialogButtonBox(Qt::Horizontal);
	buttons->addButton(confirm,QDialogButtonBox::AcceptRole);
	buttons->addButton(reject,QDialogButtonBox::RejectRole);
	connect(confirm,&QPushButton::clicked,this,&CustomSearchDialog::accept);
	connect(reject,&QPushButton::clicked,this,&CustomSearchDialog::reject);

	mainLayout->addWidget(buttons);
	setLayout(mainLayout);
	setWindowTitle(tr("Search query"));
}
QHBoxLayout* CustomSearchDialog::createEntry(const QString &name, const QList<QVariant> &parameters) const {
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

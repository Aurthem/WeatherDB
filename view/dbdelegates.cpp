//#include <QtWidgets>
//#include <QPainter>
#include <QStylePainter>
#include <QTimer>
#include <QApplication>

#include "dbdelegates.h"
#include "database.h"

void ComboDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	QItemDelegate::paint(painter, option, index);	//<- calls these (after calculating rectangles):
//	drawBackground(painter, opt, index);
//	drawCheck(painter, opt, checkRect, checkState);
//	drawDecoration(painter, opt, decorationRect, pixmap);
//	drawDisplay(painter, opt, displayRect, text);
//	drawFocus(painter, opt, displayRect);
	//of which latter 4 are virtual, thus to not draw text drawDisplay should be overridden

///---this method renders items inside combobox dropdown list
	QColor tmp_color(index.data().toString());
	if(tmp_color.isValid()) {
		painter->save();
		painter->setBrush(tmp_color);
		painter->setPen(tmp_color);
		painter->setRenderHint(QPainter::Antialiasing,true);
//		painter->drawEllipse(option.rect);
		int diameter=qMin(option.rect.width(),option.rect.height());
		int left=option.rect.left()+(option.rect.width()-diameter)/2;
		int top=option.rect.top()+(option.rect.height()-diameter)/2;
		painter->drawEllipse(left+1,top+1,diameter-2,diameter-2);
		painter->restore();
	}
}
void ComboDelegate::drawDisplay(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text) const {
	//do nothing - don't draw text
	Q_UNUSED(painter)
	Q_UNUSED(option)
	Q_UNUSED(rect)
	Q_UNUSED(text)
}
DBColorCombo::DBColorCombo(QWidget * parent) : QComboBox(parent) {
	setItemDelegate(new ComboDelegate());
}
void DBColorCombo::paintEvent(QPaintEvent * event) {
	Q_UNUSED(event)
	QVariant data=itemData(currentIndex());
	QColor tmp_color(data.toString());
	QStylePainter painter(this);

	QStyleOptionComboBox opt;
	initStyleOption(&opt);

	painter.save();
///---draw selected item (after it was selected in combobox dropdown list)
	if(tmp_color.isValid()) {
		painter.fillRect(opt.rect,tmp_color);
	}
	painter.restore();

	QStyleOptionFrame frOpt;
	frOpt.QStyleOption::operator =(opt);
	frOpt.rect=style()->subControlRect(QStyle::CC_ComboBox,&opt,QStyle::SC_ComboBoxFrame,this);
	style()->drawPrimitive(QStyle::PE_Frame,&frOpt,&painter,this);
	QStyleOption tmpOpt;
	tmpOpt.QStyleOption::operator =(opt);
	int diameter=qMin(opt.rect.width(),opt.rect.height());
	int left=opt.rect.left()+(opt.rect.width()-diameter)/2;
	int top=opt.rect.top()+(opt.rect.height()-diameter)/2;
	tmpOpt.rect=QRect(left+1,top+1,diameter,diameter);
	style()->drawPrimitive(QStyle::PE_IndicatorArrowDown,&tmpOpt,&painter,this);
}

DBEditor::DBEditor(const QString &type, const QList<QVariant> &range, QWidget * parent)
	: QLineEdit(parent), dataType(Unknown), valid(true), currentValidator(0)
{
	if(!type.compare("real")) {
		DBDoubleValidator *validator=new DBDoubleValidator(this);
		validator->setRange(range.value(0).toDouble(),range.value(1).toDouble(),3);
		setValidator(validator);
		currentValidator=validator;
		validatorMessage.append(QString("%L1").arg(range.value(0).toDouble(), 0, 'f', 3))
				.append(" < X < ").append(QString("%L1").arg(range.value(1).toDouble(), 0, 'f', 3));
		dataType=DoubleData;
		connect(validator,&DBDoubleValidator::validationStateChanged,this,&DBEditor::setValidProperty);
	} else if(!type.compare("integer")) {
		DBIntValidator *validator=new DBIntValidator(this);
		validator->setRange(range.value(0).toInt(),range.value(1).toInt());
		setValidator(validator);
		currentValidator=validator;
		validatorMessage.append(range.value(0).toString()).append(" < X < ").append(range.value(1).toString());
		dataType=IntData;
		connect(validator,&DBIntValidator::validationStateChanged,this,&DBEditor::setValidProperty);
	}
	setFixedWidth(70);
}
void DBEditor::setValidProperty(QValidator::State newState) {
	if(newState==QValidator::Acceptable) setValid(true);
	else setValid(false);
//	style()->unpolish(this);
	style()->polish(this);	//to reapply style
	update();
	QTimer::singleShot(500,this,[this]{	//check if the input was changed by validator after a delay
		setValid(hasAcceptableInput());
		style()->polish(this);
		update();
	});
}
bool DBEditor::setValue(const QVariant &value) {
	switch(dataType) {
	case DoubleData:
		if(!value.isNull()) setText(QString("%L1").arg(value.toDouble(), 0, 'f', 3));
		return true;
	case IntData:
		setText(value.toString());
		return true;
	default:
		return false;
	}
}
QVariant DBEditor::getValue() const {
	switch(dataType) {
	case DoubleData: {
		QLocale defaultLocale; bool isSuccessful;
		double value=defaultLocale.toDouble(text(),&isSuccessful);
		if(isSuccessful) return value;
		else return QVariant();
	}
	case IntData: {
		QLocale defaultLocale; bool isSuccessful;
		int value=defaultLocale.toInt(text(),&isSuccessful);
		if(isSuccessful) return value;
		else return QVariant();
	}
	default:
		return QVariant();
	}
}
void DBEditor::setValidatorRange(const QVariant &left, const QVariant &right) {
	switch(dataType) {
	case DoubleData: {
		DBDoubleValidator *tmpValidator=qobject_cast<DBDoubleValidator*>(currentValidator);
		tmpValidator->setRange(left.toDouble(),right.toDouble(),3);
	} break;
	case IntData: {
		DBIntValidator *tmpValidator=qobject_cast<DBIntValidator*>(currentValidator);
		tmpValidator->setRange(left.toInt(),right.toInt());
	} break;
	default:
		break;
	}
}

DBIntValidator::DBIntValidator(QObject * parent) : QIntValidator(parent) {
}
QValidator::State	DBIntValidator::validate(QString & input, int & pos) const {
	State result;
	if(input.isEmpty()) result=QValidator::Acceptable;
	else result=QIntValidator::validate(input,pos);
	if(result!=currentState) {
		currentState=result;
		emit validationStateChanged(result);
	}
	return result;
}
//void DBIntValidator::fixup(QString & input) const {
//	qDebug()<<"fixup int";
//	QIntValidator::fixup(input);
//}
DBDoubleValidator::DBDoubleValidator(QObject * parent) : QDoubleValidator(parent) {
}
QValidator::State	DBDoubleValidator::validate(QString & input, int & pos) const {
	State result;
	if(input.isEmpty()) result=QValidator::Acceptable;
	else result=QDoubleValidator::validate(input,pos);
	if(result!=currentState) {
		currentState=result;
		emit validationStateChanged(result);
	}
	return result;
}
//void DBDoubleValidator::fixup(QString & input) const {
//	qDebug()<<"fixup double";
//	QDoubleValidator::fixup(input);
//}

DBDelegate::DBDelegate(QWidget *parent)
	: QStyledItemDelegate(parent) {
}
QWidget* DBDelegate::createEditor(QWidget *parent,
																	const QStyleOptionViewItem &option,
																	const QModelIndex &index) const
{
	const QList<QVariant> &dbList=Database::getInstance().settings.database.value(
					index.model()->headerData(index.row(),Qt::Vertical,Qt::UserRole).toString()
				);
	if(dbList.value(2).toInt()>0) {
		DBColorCombo *editor=new DBColorCombo(parent);
		for(QVariant &item: dbList.mid(3)) {
			editor->addItem(item.toString(),QColor(item.toString()));
		}
		return editor;
	}
	DBEditor *editor=new DBEditor(dbList.value(1).toString(),dbList.mid(3),parent);
	if(editor->getType()!=DBEditor::Unknown) {
		emit validatedEditorCreated(editor->getValidatorMessage());
		return editor;
	} else {
		delete editor;	//created, but failed, prevent memory leak
	}

	return QStyledItemDelegate::createEditor(parent, option, index);
}
void DBDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	QComboBox *combo=qobject_cast<QComboBox*>(editor);
	if(combo) {
		int value=index.data(Qt::EditRole).toInt();
		combo->setCurrentIndex(value);
		return;
	}
	DBEditor *dbEditor=qobject_cast<DBEditor*>(editor);
	if(dbEditor) {
		if(dbEditor->setValue(index.data(Qt::EditRole))) return;
	}

	QStyledItemDelegate::setEditorData(editor,index);
}
void DBDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	QComboBox *combo=qobject_cast<QComboBox*>(editor);
	if(combo) {
		int value=combo->currentIndex();
		model->setData(index,value,Qt::EditRole);
		return;
	}
	DBEditor *dbEditor=qobject_cast<DBEditor*>(editor);
	if(dbEditor) {
		QVariant value=dbEditor->getValue();
//		if(!value.isNull()) {
			model->setData(index,value,Qt::EditRole);
			return;
//		}
	}
	QStyledItemDelegate::setModelData(editor,model,index);
}
void DBDelegate::updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if(Database::getInstance().settings.database.value(
					index.model()->headerData(index.row(),Qt::Vertical,Qt::UserRole).toString()
					).value(2).toInt()>0
				) {
		editor->setGeometry(option.rect);
	} else
		QStyledItemDelegate::updateEditorGeometry(editor,option,index);
}

void DBDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//	QStyledItemDelegate::paint(painter,option,index);
	const QList<QVariant> &dbList=Database::getInstance().settings.database.value(
					index.model()->headerData(index.row(),Qt::Vertical,Qt::UserRole).toString()
				);

	QStyleOptionViewItem opt = option;
	initStyleOption(&opt, index);

	if(dbList.value(2).toInt()>0) opt.text.clear();	//don't show text on "mark" type fields

	const QWidget* widget=option.widget;
	QStyle *style=widget ? widget->style() : QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

	if(dbList.value(2).toInt()>0 && !index.data(Qt::EditRole).isNull()) {	//don't draw in the new column if the value is not set
//		QBrush tmp_brush=index.data(Qt::BackgroundRole).value<QBrush>();
		QColor tmp_color(dbList.value(3+index.data(Qt::EditRole).toInt()).toString());
///---draw item in the table cell
		if(tmp_color.isValid()) {
			painter->save();
			painter->setBrush(tmp_color);
			painter->setPen(tmp_color);
			painter->setRenderHint(QPainter::Antialiasing,true);
			int diameter=qMin(option.rect.width(),option.rect.height());
			int left=option.rect.left()+(option.rect.width()-diameter)/2;
			int top=option.rect.top()+(option.rect.height()-diameter)/2;
			painter->drawEllipse(left+1,top+1,diameter-2,diameter-2);
			painter->restore();
		}
	}

	if(index.model()->data(index,Qt::UserRole).canConvert(QMetaType::QString)) {
		QPolygon triangle(3);
		triangle.setPoint(0,QPoint(option.rect.x()+5,option.rect.y()));
		triangle.setPoint(1,QPoint(option.rect.x(),option.rect.y()));
		triangle.setPoint(2,QPoint(option.rect.x(),option.rect.y()+5));
		painter->save();
		painter->setRenderHint(painter->Antialiasing);
		QColor tmpColor(index.model()->headerData(index.column(),Qt::Horizontal,Qt::DisplayRole).toString().compare("!")
										? Qt::darkGreen : Qt::darkRed);	//red for removed, but not submitted rows
		painter->setBrush(QBrush(tmpColor));
		painter->setPen(QPen(tmpColor));
		painter->drawPolygon(triangle);
		painter->restore();
	}
}
QSize	DBDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
	QSize result=QStyledItemDelegate::sizeHint(option,index);
	if(Database::getInstance().settings.database.value(
					index.model()->headerData(index.row(),Qt::Vertical,Qt::UserRole).toString()
					).value(2).toInt()>0
				) {
		int diameter=qMax(result.width(),result.height());	//for bigger circles
		result.setHeight(diameter);
		result.setWidth(diameter);
	}
	return result;
}
/*---
QString DBDelegate::displayText(const QVariant &value, const QLocale &locale) const {
	if(	value.type()==QVariant::String) {
		return makeVertical(value.toString());
	} else if(value.type()==QVariant::LongLong) {
		return makeVertical(locale.toString(value.toInt()));
	}
	return QStyledItemDelegate::displayText(value,locale);
}
---*/

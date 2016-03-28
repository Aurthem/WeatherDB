#ifndef DBDELEGATES_H
#define DBDELEGATES_H

#include <QItemDelegate>
#include <QComboBox>
#include <QLineEdit>
#include <QStyledItemDelegate>

class ComboDelegate : public QItemDelegate {
public:
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
protected:
	virtual void drawDisplay(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QString & text) const Q_DECL_OVERRIDE;
};
class DBColorCombo : public QComboBox
{	Q_OBJECT
public:
	DBColorCombo(QWidget * parent = 0);
protected:
	virtual void paintEvent(QPaintEvent * event) Q_DECL_OVERRIDE;
};

class DBEditor: public QLineEdit
{ Q_OBJECT
//	Q_DECLARE_METATYPE(QValidator::State)
//	Q_ENUM(QValidator::State)
public:
	DBEditor(const QString &type, const QList<QVariant> &range, QWidget * parent = 0);

	enum Type { Unknown=0, IntData=1, DoubleData=2 };
	Type getType() const { return dataType; }

	Q_PROPERTY(bool valid READ isValid WRITE setValid)// NOTIFY validityChanged)
	bool isValid() const { return valid; }
	void setValid(bool newValid) { valid=newValid; }

	bool setValue(const QVariant &value);
	QVariant getValue() const;

//	QPair<QVariant,QVariant> getValidatorLimits() const;
	QString getValidatorMessage() const { return validatorMessage; }
	void setValidatorRange(const QVariant &left, const QVariant &right);
public slots:
	void setValidProperty(QValidator::State newState);
private:
	Type dataType;
	bool valid;
	QValidator* currentValidator;
	QString validatorMessage;
};
class DBIntValidator: public QIntValidator
{ Q_OBJECT
public:
	DBIntValidator(QObject * parent = 0);
	virtual QValidator::State	validate(QString & input, int & pos) const Q_DECL_OVERRIDE;
//	virtual void fixup(QString & input) const Q_DECL_OVERRIDE;
signals:
	void validationStateChanged(QValidator::State newState) const;
private:
	mutable State currentState;
};
class DBDoubleValidator: public QDoubleValidator
{	Q_OBJECT
public:
	DBDoubleValidator(QObject * parent = 0);
	virtual QValidator::State	validate(QString & input, int & pos) const Q_DECL_OVERRIDE;
//	virtual void fixup(QString & input) const Q_DECL_OVERRIDE;
signals:
	void validationStateChanged(QValidator::State newState) const;
private:
	mutable State currentState;
};

class DBDelegate: public QStyledItemDelegate
{	Q_OBJECT
public:
	DBDelegate(QWidget *parent = 0);
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
												const QModelIndex &index) const Q_DECL_OVERRIDE;
	void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
										const QModelIndex &index) const Q_DECL_OVERRIDE;
	void updateEditorGeometry(QWidget *editor,
														const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
	//	QString	displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;
//	QSize	sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
	virtual QSize	sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const Q_DECL_OVERRIDE;
signals:
//	void validatedEditorCreated(const QPair<QVariant,QVariant> &limits) const;
	void validatedEditorCreated(const QString &validatorMessage) const;
private:
//	QString makeVertical(QString source) const;
};

#endif // DBDELEGATES_H

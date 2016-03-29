#ifndef COLLAPSIBLEGROUPBOX_H
#define COLLAPSIBLEGROUPBOX_H
//Adapted from the LibQxt project: QxtGroupBox
//https://bitbucket.org/libqxt/libqxt/src/696423b68972fc9edae318558b8ce26dc187cc40/src/widgets/qxtgroupbox.h
//https://bitbucket.org/libqxt/libqxt/src/696423b68972fc9edae318558b8ce26dc187cc40/src/widgets/qxtgroupbox.cpp

#include <QGroupBox>

class CollapsibleGroupBox : public QGroupBox
{	Q_OBJECT
	Q_PROPERTY(bool collapsive READ isCollapsive WRITE setCollapsive)
public:
	explicit CollapsibleGroupBox(QWidget *parent = 0);
	explicit CollapsibleGroupBox(const QString& title, QWidget *parent = 0);

	bool isCollapsive() const { return collapsive; }
	void setCollapsive(bool enabled);

public slots:
	void setCollapsed(bool collapsed = true) { setExpanded(!collapsed); }
	void setExpanded(bool expanded = true);
protected:
	virtual void childEvent(QChildEvent* event) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
private:
	bool collapsive;
	bool flat;

	bool advancedOptionsEnabled;	//quick fix for advanced options showing after the groupbox is collapsed
};

#endif // COLLAPSIBLEGROUPBOX_H

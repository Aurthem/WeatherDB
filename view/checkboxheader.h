#ifndef CHECKBOXHEADER_H
#define CHECKBOXHEADER_H

#include <QHeaderView>
QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

class CheckboxHeader : public QHeaderView
{	Q_OBJECT
public:
	CheckboxHeader(Qt::Orientation orientation, QWidget* parent = 0);
//	virtual ~CheckboxHeader() {}

//	void setSectionHidden(int logicalIndex, bool hide);
//	bool isChecked() const { return isChecked_; }
//	void setIsChecked(bool val);
	QList<int> getCheckedBoxes(void) const;

//	virtual void setModel(QAbstractItemModel * model) Q_DECL_OVERRIDE;
signals:
//	void checkBoxClicked(bool state);
	void sectionsSetHidden(const QList<int> &index_list,bool hide);

public slots:
	void handleSectionResized(int idx);
	void handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex);
//	void update();
	void handleRowsHidden(void);
	void handleRowsShown(void);
	void fixBoxPositions();

	virtual void reset() Q_DECL_OVERRIDE;	//clear boxes when reset happened
protected:
//	void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const Q_DECL_OVERRIDE;
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

//	void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
//mousePressEvent
//private slots:
//	void onResetModel(void);	//slot to hide id row (very model specific)
private:
//	bool isChecked_;

//	void redrawCheckBox();
	QMap<int, QCheckBox*> boxes;
//	void updateBoxes();
	void updateBox(int index);	//just checks that the box is there
};

#endif // CHECKBOXHEADER_H

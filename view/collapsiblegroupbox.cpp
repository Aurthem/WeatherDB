#include <QChildEvent>
#include <QStylePainter>
#include <QStyleOption>
//#include <QStyleOptionGroupBox>

#include "collapsiblegroupbox.h"

CollapsibleGroupBox::CollapsibleGroupBox(QWidget *parent)
	: QGroupBox(parent), collapsive(false), advancedOptionsEnabled(true)
{
	setCheckable(true);
	setChecked(true);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));
}
CollapsibleGroupBox::CollapsibleGroupBox(const QString& title,QWidget *parent)
	: QGroupBox(title,parent), collapsive(false), advancedOptionsEnabled(true)
{
	setCheckable(true);
	setChecked(true);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(setExpanded(bool)));
}
void CollapsibleGroupBox::setCollapsive(bool enabled) {
	if(collapsive!=enabled) {
		collapsive=enabled;
		if(!enabled)
			setExpanded(true);
		else if(!isChecked())
			setExpanded(false);
	}
}
void CollapsibleGroupBox::setExpanded(bool expanded) {
	if(collapsive || expanded) {
		for(QObject* child: children()) {
			if(child->isWidgetType()) {
				//added for advancedOptions hiding
				if(expanded && !child->objectName().compare("advancedOptions")) {
					QVariant isAdvancedVisible=child->property("isAdvancedVisible");
					if(!isAdvancedVisible.isNull()) {
						static_cast<QWidget*>(child)->setVisible(isAdvancedVisible.toBool());
						continue;
					}
				}

				static_cast<QWidget*>(child)->setVisible(expanded);
			}
		}
		if(expanded) {
			setFlat(flat);
		} else {
			flat=isFlat();
			setFlat(true);
		}
	}
}
void CollapsibleGroupBox::childEvent(QChildEvent* event) {
	QObject* child = event->child();
	if (event->added() && child->isWidgetType()) {
		QWidget* widget = static_cast<QWidget*>(child);
		if (collapsive && !isChecked())
			widget->hide();
	}
}
void CollapsibleGroupBox::paintEvent(QPaintEvent */*event*/) {
	QStylePainter paint( this );
	QStyleOptionGroupBox option;
	initStyleOption( &option );
//	paint.drawPrimitive(QStyle::PE_IndicatorSpinDown,option);
//	option.subControls &= ~QStyle::SC_GroupBoxCheckBox;
//	option.subControls |= QStyle::SC_SpinBoxFrame;

//	paint.drawComplexControl( QStyle::CC_GroupBox, option );

//	option.rect = style()->subControlRect( QStyle::CC_GroupBox, &option,
//																				 QStyle::SC_GroupBoxCheckBox, this );
//	SC_ComboBoxArrow;
////now erase the checkbox
//	paint.save();
//	QPixmap px( option.rect.width(), option.rect.height() );
//	px.fill( this, option.rect.left(), option.rect.top() );
//	QBrush brush( px );
//	paint.fillRect( option.rect, brush );
//	paint.restore();
//// and replace it with a radio button
//	paint.drawPrimitive( QStyle::PE_IndicatorRadioButton, option );

//	option.rect=QRect(0,0,10,10);// rect.adjusted(0, 0, -10, 0);
//	paint.drawPrimitive(QStyle::PE_IndicatorSpinDown,option);
//	paint.drawItemPixmap(
//				option.rect.adjusted( 0, 0, -10, 0 ),
//				Qt::AlignTop | Qt::AlignRight,
//				QPixmap( m_collapsed ?
//									 ":/images/images/navigate_down2_16x16.png" :
//									 ":/images/images/navigate_up2_16x16.png" ) );

	//directly from qcommonstyle.cpp
	QRect textRect = style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxLabel, this);
	QRect checkBoxRect = style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxCheckBox, this);
	if (option.subControls & QStyle::SC_GroupBoxFrame) {
		QStyleOptionFrame frame;
		frame.QStyleOption::operator=(option);
		frame.features = option.features;
		frame.lineWidth = option.lineWidth;
		frame.midLineWidth = option.midLineWidth;
		frame.rect = style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxFrame, this);
		paint.save();
		QRegion region(option.rect);
		if (!option.text.isEmpty()) {
			bool ltr = option.direction == Qt::LeftToRight;
			QRect finalRect;
			if (option.subControls & QStyle::SC_GroupBoxCheckBox) {
				finalRect = checkBoxRect.united(textRect);
				finalRect.adjust(ltr ? -4 : 0, 0, ltr ? 0 : 4, 0);
			} else {
				finalRect = textRect;
			}
			region -= finalRect;
		}
		paint.setClipRegion(region);
		style()->drawPrimitive(QStyle::PE_FrameGroupBox, &frame, &paint, this);
		paint.restore();
	}

	// Draw title
	if ((option.subControls & QStyle::SC_GroupBoxLabel) && !option.text.isEmpty()) {
		QColor textColor = option.textColor;
		if (textColor.isValid())
			paint.setPen(textColor);
		int alignment = int(option.textAlignment);
		if (!style()->styleHint(QStyle::SH_UnderlineShortcut, &option, this))
			alignment |= Qt::TextHideMnemonic;

		style()->drawItemText(&paint, textRect,  Qt::TextShowMnemonic | Qt::AlignHCenter | alignment,
													option.palette, option.state & QStyle::State_Enabled, option.text,
													textColor.isValid() ? QPalette::NoRole : QPalette::WindowText);

		if (option.state & QStyle::State_HasFocus) {
			QStyleOptionFocusRect fropt;
			fropt.QStyleOption::operator=(option);
			fropt.rect = textRect;
			style()->drawPrimitive(QStyle::PE_FrameFocusRect, &fropt, &paint, this);
		}
	}

	// Draw checkbox
	if (option.subControls & QStyle::SC_GroupBoxCheckBox) {
		QStyleOptionButton box;
		box.QStyleOption::operator=(option);
		box.rect = checkBoxRect;
		box.rect.adjust(1,1,-2,-2);
//		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &box, &paint, this);
		if(isChecked())
			style()->drawPrimitive(QStyle::PE_IndicatorSpinMinus, &box, &paint, this);
		else
			style()->drawPrimitive(QStyle::PE_IndicatorSpinPlus, &box, &paint, this);
//		style()->drawPrimitive(QStyle::PE_IndicatorBranch, &box, &paint, this);
	}

}

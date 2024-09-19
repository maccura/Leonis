/***************************************************************************
*   This file is part of the leonis project                               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   zhouguangming@maccura.com                                             *
*                                                                         *
**                   GNU General Public License Usage                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
**                  GNU Lesser General Public License                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 3 of the    *
*   License, or (at your option) any later version.                       *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this library.                                      *
*   If not, see <http://www.gnu.org/licenses/>.                           *
*                                                                         *
*   This library is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// @file     QComDelegate.h
/// @brief    各类通用代理类
///
/// @author   5774/WuHongTao
/// @date     2022年5月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QComDelegate.h"
#include "QPainter"
#include <QComboBox>
#include <QMouseEvent>
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>
#include <QCalendarWidget>
#include <QKeyEvent>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QStylePainter>
#include "src/common/common.h"

#define DATEFORMAT "yyyy-MM-dd hh:mm:ss"

///
/// @brief 下拉框代理构造函数
///
/// @param[in]  parent	   父节点
/// @param[in]  valueList  combox的初始值
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
ComBoxDelegate::ComBoxDelegate(QObject *parent, QStringList valueList)
    : QStyledItemDelegate(parent)
{
	m_valueList = valueList;
}

///
/// @brief
///     创建combox的widgets
///
/// @param[in]  parent  父节点
/// @param[in]  option  创建选项，显示的一些细节和控制
/// @param[in]  index  model中的位置
///
/// @return 创建的combox的widget指针
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
QWidget *ComBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    //创建编辑的控件
	QComboBox *editor = new QComboBox(parent);
    editor->setFrame(true);

    //设置属性
	int i = 0;
	for (const QString& value : m_valueList)
	{
		editor->addItem(value, i++);
	}

    return editor;
}

///
/// @brief
///     设置editor的显示值
///
/// @param[in]  editor  editor的空间Handler
/// @param[in]  index  位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void ComBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int level = index.model()->data(index, Qt::EditRole).toInt();
	QComboBox *comBox = static_cast<QComboBox*>(editor);
    //设置编号
	comBox->setCurrentIndex(level);
}

///
/// @brief
///     更新Modle的对应值
///
/// @param[in]  editor  位置对应的控件
/// @param[in]  model  模式
/// @param[in]  index  位置
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void ComBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QComboBox *comBox = static_cast<QComboBox*>(editor);
	int level = comBox->currentIndex();
    model->setData(index, level, Qt::EditRole);
}

///
/// @brief
///     更新的区域，控件
///
/// @param[in]  editor  控件指针
/// @param[in]  option  绘制选项
/// @param[in]  index  位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void ComBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父节点
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
LineEditDelegate::LineEditDelegate(QObject *parent)
{

}

///
/// @brief
///     绘制displaymode的时候的函数
///
/// @param[in]  painter  图形绘制句柄
/// @param[in]  option  选项
/// @param[in]  index  索引
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月21日，新建函数
///
void LineEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter, option, index);
}

///
/// @brief
///     创建combox的widgets
///
/// @param[in]  parent  父节点
/// @param[in]  option  创建选项，显示的一些细节和控制
/// @param[in]  index  model中的位置
///
/// @return 创建的LineEdit的widget指针
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
QWidget * LineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QLineEdit *editor = new QLineEdit(parent);
    connect(editor, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
    connect(editor, SIGNAL(textChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
    editor->setFrame(true);
	editor->setValidator(new QDoubleValidator(editor));
	return editor;
}

///
/// @brief
///     设置editor的显示值
///
/// @param[in]  editor  editor的空间Handler
/// @param[in]  index  位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void LineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QString value = index.model()->data(index, Qt::EditRole).toString();
	QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
	lineEdit->setText(value);
}

///
/// @brief
///     更新Modle的对应值
///
/// @param[in]  editor  位置对应的控件
/// @param[in]  model  模式
/// @param[in]  index  位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
	QString text = lineEdit->text();

    // 如果设置了验证函数，则执行验证
    if (m_validateFunc && !m_validateFunc(text))
    {
        return;
    }
	model->setData(index, text, Qt::EditRole);
}

///
/// @brief
///     更新的区域，控件
///
/// @param[in]  editor  控件指针
/// @param[in]  option  绘制选项
/// @param[in]  index  位置
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
void LineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
}

void LineEditDelegate::SetValidateFunc(std::function<bool(const QString&)> func)
{
    m_validateFunc = func;
}

///
/// @brief
///     构造函数
///
/// @param[in]  parent  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月12日，新建函数
///
CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
	: QStyledItemDelegate(parent)
{
    isBase = false;
	selectedPixmap = QPixmap(":/Leonis/resource/image/icon-select.png");
}

CheckBoxDelegate::CheckBoxDelegate(QString path, QObject *parent /*= 0*/)
    : QStyledItemDelegate(parent)
{
    isBase = false;
    selectedPixmap = QPixmap(path);
}

CheckBoxDelegate::CheckBoxDelegate(bool base, QObject *parent /*= 0*/)
    : QStyledItemDelegate(parent)
{
    isBase = base;
}

///
/// @brief
///     不创建editor
///
/// @param[in]  parent  父节点
/// @param[in]  option  创建选项，显示的一些细节和控制
/// @param[in]  index  model中的位置
///
/// @return NULL
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月29日，新建函数
///
QWidget *CheckBoxDelegate::createEditor(QWidget *parent,
	const QStyleOptionViewItem & option,
	const QModelIndex & index) const
{
	Q_UNUSED(parent);
	Q_UNUSED(option);
	Q_UNUSED(index);
	return NULL;
}

///
/// @brief
///     处理鼠标点击事件
///
/// @param[in]  event  鼠标事件
/// @param[in]  model  数据模型
/// @param[in]  option  绘制选项
/// @param[in]  index  位置
///
/// @return true成功
///
/// @par History:
/// @li 5774/WuHongTao，2020年10月10日，新建函数
///
bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	auto decorationRect = option.rect;

	// 根据事件来做不同操作
	QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
	if (event->type() == QEvent::MouseButtonPress && decorationRect.contains(mouseEvent->pos()))
	{
		// 获取设置数据
        if (isBase)
        {
            // 没有使能，则不能点击
            auto flagEnable = index.data(Qt::UserRole + 2);
            if (flagEnable.isValid() && !flagEnable.toBool())
            {
                return false;
            }

            auto dataCheck = index.data(Qt::UserRole + 1).toBool();
            model->setData(index, !dataCheck, Qt::UserRole + 1);
        }
        else
        {
            bool IsSelect = model->data(index, Qt::UserRole + 1).toBool();
            model->setData(index, IsSelect ? false : true, Qt::UserRole + 1);
        }
	}

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

///
/// @brief
///     绘制displaymode的时候的函数
///
/// @param[in]  painter  图形绘制句柄
/// @param[in]  option  选项
/// @param[in]  index  索引
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月21日，新建函数
///
void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // 原始对话框
    if (isBase)
    {
        // 是否勾选
        auto flag = index.data(Qt::UserRole + 1);
        if (!flag.isValid())
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        QStyleOptionButton check_box_style_option;
        if (flag.toBool())
        {
            check_box_style_option.state |= QStyle::State_On;
        }

        // 是否开启
        auto flagEnable = index.data(Qt::UserRole + 2);
        check_box_style_option.state |= (!flagEnable.isValid() || flagEnable.toBool()) ? QStyle::State_Enabled : QStyle::State_None;

        // 计算 checkbox 的位置，使其居中显示
        QCheckBox checkbox;
        QSize check_box_size = QApplication::style()->sizeFromContents(QStyle::CT_CheckBox, &check_box_style_option, QSize(), nullptr);
        int check_box_x = option.rect.x() + option.rect.width() / 2 - check_box_size.width() / 2 -3;
        int check_box_y = option.rect.y() + option.rect.height() / 2 - check_box_size.height() / 2;
        check_box_style_option.rect = QRect(check_box_x, check_box_y, check_box_size.width(), check_box_size.height());
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &check_box_style_option, painter, &checkbox);
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

	QStyledItemDelegate::paint(painter, option, index);

    QString strColor = index.model()->data(index, VIEW_ITEM_DISABLE_ROLE).toString();
    if (!strColor.isEmpty())
    {
        painter->fillRect(QRect(option.rect.x() + 1, option.rect.y(), option.rect.width() - 1, option.rect.height() - 1), QColor(strColor));
        return;
    }

	if (index.model()->data(index, Qt::UserRole + 1).toBool())
	{
        if (selectedBkColor.isValid())
        {
            painter->fillRect(QRect(option.rect.x() + 1, option.rect.y(), option.rect.width() - 1, option.rect.height() - 1), selectedBkColor);
        }
        
		QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignHCenter, selectedPixmap);
	}
}

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父节点
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月5日，新建函数
///
PositiveNumDelegate::PositiveNumDelegate(QObject *parent)
    :m_decimalsPlace(6)
{

}

///
/// @brief
///     绘制displaymode的时候的函数
///
/// @param[in]  painter  图形绘制句柄
/// @param[in]  option  选项
/// @param[in]  index  索引
///
/// @par History:
/// @li 8090/YeHuaNing，2020年9月21日，新建函数
///
void PositiveNumDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

///
/// @brief
///     创建combox的widgets
///
/// @param[in]  parent  父节点
/// @param[in]  option  创建选项，显示的一些细节和控制
/// @param[in]  index  model中的位置
///
/// @return 创建的LineEdit的widget指针
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月5日，新建函数
///
QWidget * PositiveNumDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    editor->setFrame(true);
    QDoubleValidator*  validator = new ChDoubleValidator(editor, m_decimalsPlace);
    validator->setBottom(0);
    editor->setValidator(validator);
    connect(editor, SIGNAL(textChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
    return editor;
}

///
/// @brief
///     设置editor的显示值
///
/// @param[in]  editor  editor的空间Handler
/// @param[in]  index  位置
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月5日，新建函数
///
void PositiveNumDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);
}

///
/// @brief
///     更新Modle的对应值
///
/// @param[in]  editor  位置对应的控件
/// @param[in]  model  模式
/// @param[in]  index  位置
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月5日，新建函数
///
void PositiveNumDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString text = lineEdit->text();
   
	// 如果设置了验证函数，则执行验证
	if (m_validateFunc && !m_validateFunc(text))
	{
		return;
	}
	model->setData(index, text, Qt::EditRole);
}

///
/// @brief
///     更新的区域，控件
///
/// @param[in]  editor  控件指针
/// @param[in]  option  绘制选项
/// @param[in]  index  位置
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月5日，新建函数
///
void PositiveNumDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void PositiveNumDelegate::SetValidateFunc(std::function<bool(const QString&)> func)
{
	m_validateFunc = func;
}

void PositiveNumDelegate::SetDecimalsPlace(int decimalsPlace)
{
    // 超出范围则恢复为默认值
    if (decimalsPlace < 1 || m_decimalsPlace > 6)
        m_decimalsPlace = 6;
    else
        m_decimalsPlace = decimalsPlace;
}

QBaseDateEdit::QBaseDateEdit(QWidget *parent)
    : QDateEdit(parent)
{
    initControl();
}

QBaseDateEdit::QBaseDateEdit(const QDate& date, QWidget* parent)
    : QDateEdit(date, parent)
{
    initControl();
}

QBaseDateEdit::~QBaseDateEdit()
{
}

void QBaseDateEdit::initControl()
{
    minDate = QDate(2000, 1, 1);//qt允许的最小日期
    setMinimumDate(minDate);
	setDate(QDate::currentDate());
    setCalendarPopup(true);
    connect(calendarWidget(), &QCalendarWidget::clicked, this, &QBaseDateEdit::slots_clicked);
    calendarWidget()->setMinimumWidth(288);
    calendarWidget()->setMinimumHeight(202);
    setDisplayFormat(DATEFORMAT);
}

void QBaseDateEdit::setNull()
{
    //setDate(minDate);
    lineEdit()->setText("");
}

bool QBaseDateEdit::isNull()
{
    return lineEdit()->text().isEmpty();
}

void QBaseDateEdit::setDateTimeStr(const QString& dateTimeStr)
{
    dateTimeStr.isEmpty() ? setNull() : setDateTime(QDateTime::fromString(dateTimeStr, DATEFORMAT));
}

QString QBaseDateEdit::getDateTimeStr(bool isStart)
{
	if (isStart)
	{
		auto dateTimeValue = dateTime();
		dateTimeValue.setTime(QTime(0, 0, 0));
		return isNull() ? "" : dateTimeValue.toString(DATEFORMAT);
	}
	else
	{
		auto dateTimeValue = dateTime();
		dateTimeValue.setTime(QTime(23, 59, 59));
		return isNull() ? "" : dateTimeValue.toString(DATEFORMAT);
	}
}

void QBaseDateEdit::keyPressEvent(QKeyEvent* event)
{
    if (isNull())
    {
		setDate(QDate::currentDate());
    }

    QDateEdit::keyPressEvent(event);
}

void QBaseDateEdit::mousePressEvent(QMouseEvent* event)
{
    if (isNull()) setDate(QDate::currentDate());
    calendarWidget()->setFocus();
    QDateEdit::mousePressEvent(event);
}

QValidator::State QBaseDateEdit::validate(QString& input, int& pos) const
{
    return input.isEmpty() ? QValidator::Acceptable : QDateTimeEdit::validate(input, pos);
}

QString QBaseDateEdit::textFromDateTime(const QDateTime& dt) const
{
    return dt.date() == minDate ? "" : QDateTimeEdit::textFromDateTime(dt);
}

void QBaseDateEdit::slots_clicked(const QDate& date)
{
    setDate(date);
}

void CustomButton::enterEvent(QEvent* event)
{
    Q_UNUSED(event);

    QPoint globalPos = mapToGlobal(rect().bottomLeft());
    globalPos.setY(globalPos.y() - 10); // 调整窗口的垂直位置
    globalPos.setX(globalPos.x() - 45);
    QToolTip::showText(globalPos, toolTip(), this);
}

CenterComboBox::CenterComboBox(QWidget *parent /*= Q_NULLPTR*/)
    : QComboBox(parent)
{

}

CenterComboBox::~CenterComboBox()
{

}

void CenterComboBox::SetTextAlignment(Qt::Alignment flag)
{
    // 设置下拉菜单内容居中
    QStandardItemModel* pItemMode = static_cast<QStandardItemModel*>(this->view()->model());
    if (pItemMode == Q_NULLPTR)
    {
        return;
    }

    for (int i = 0; i < pItemMode->rowCount(); i++)
    {
        QStandardItem* qItem = pItemMode->item(i);
        if (qItem != Q_NULLPTR)
        {
            qItem->setTextAlignment(flag);
        }
    }
}

void CenterComboBox::paintEvent(QPaintEvent *e)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    if (currentIndex() < 0)
        opt.palette.setBrush(QPalette::ButtonText, opt.palette.brush(QPalette::ButtonText).color().lighter());

    painter.end();

    // 在文本区域绘制按钮文本
    QPainter painter2(this);
    QStyleOptionButton buttonOpt;

    // 窗口状态
    buttonOpt.initFrom(this);
    QRect editRect = this->style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);

    // 设置按钮区域为QComboBox的文本区域
    buttonOpt.rect = editRect;

    // 设置文本
    buttonOpt.text = opt.currentText;

    // 绘制QStyle::CE_PushButtonLabel，内容居中
    this->style()->drawControl(QStyle::CE_PushButtonLabel, &buttonOpt, &painter2, this);
}

///
/// @brief
///     构造函数
///
/// @param[in]  strRegValidator     正则表达式（输入约束）
/// @param[in]  parent              父对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
InputLimitDelegate::InputLimitDelegate(QString strRegValidator, QObject *parent /*= Q_NULLPTR*/)
    : QStyledItemDelegate(parent),
      m_strRegValidator(strRegValidator)
{

}

///
/// @brief
///     创建combox的widgets
///
/// @param[in]  parent  父节点
/// @param[in]  option  创建选项，显示的一些细节和控制
/// @param[in]  index  model中的位置
///
/// @return 创建的LineEdit的widget指针
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
QWidget * InputLimitDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    // 创建编辑框并设置正则表达式
    QLineEdit *pEdit = new QLineEdit(parent);
    pEdit->setValidator(new QRegExpValidator(QRegExp(m_strRegValidator)));
    return pEdit;
}

///
/// @brief
///     设置editor的显示值
///
/// @param[in]  editor  editor的空间Handler
/// @param[in]  index  位置
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月8日，新建函数
///
void InputLimitDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString strValue = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(editor);
    if (pEdit == Q_NULLPTR)
    {
        return;
    }
    pEdit->setText(strValue);
}

///
/// @brief
///     更新Modle的对应值
///
/// @param[in]  editor  位置对应的控件
/// @param[in]  model  模式
/// @param[in]  index  位置
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月8日，新建函数
///
void InputLimitDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *pEdit = qobject_cast<QLineEdit*>(editor);
    if (pEdit == Q_NULLPTR)
    {
        return;
    }
    model->setData(index, pEdit->text(), Qt::EditRole);
}

QString ChDoubleValidator::GetValidNumber(const QString& doubleToString, double number)
{
    if (doubleToString.isEmpty())
        return doubleToString;

    const QStringList& list = doubleToString.split('.');
    int listSize = list.size();
    int integerCnt = 0;
    int decimalCnt = 0;

    if (listSize == 1)
    {
        integerCnt = list.front().size();
    }
    else if (listSize == 2)
    {
        integerCnt = list.front().size();
        decimalCnt = list.back().size();
    }
    // 不符合规则，直接返回自身
    else
    {
        return doubleToString;
    }

    QString dest;
    int decimals = 0; // 有效小数位数
                      // 如果数字区域已经大于了有效位数，就返回前段
    if (integerCnt > m_numbers)
        dest = list.front().left(m_numbers);
    // 总位数超过 m_numbers
    else if (integerCnt + decimalCnt > m_numbers)
    {
        decimals = m_numbers - integerCnt;
    }
    // 小数不能超过m_decimals
    else if (decimalCnt > m_decimals)
        decimals = m_decimals;
    else
        dest = doubleToString;

    // 进行小数有效位获取
    if (decimals > 0)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(decimals) << number;
        dest = QString::fromStdString(ss.str());
        dest.remove(QRegExp("0*$"));
    }

    return dest;
}

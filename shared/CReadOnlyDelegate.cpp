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
/// @file     CReadOnlyDelegate.h
/// @brief    只读
///
/// @author   5774/WuHongTao
/// @date     2020年5月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include <QStandardItemModel>
#include "CReadOnlyDelegate.h"
#include <QPainter>
#include <QEvent>
#include <QComboBox>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QPushButton>
#include <QMouseEvent>
#include "QC/ch/QQcSampleModel.h"
#include "src/common/TimeUtil.h"
#include "shared/uidef.h"
#include "manager/DictionaryQueryManager.h"
#include <bitset>
#include "SortHeaderView.h"
#include "datetimefmttool.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QTableView>

///
/// @brief
///     只读代理的构造函数
///
/// @param[in]  parent  父节点
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
CReadOnlyDelegate::CReadOnlyDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
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
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
QWidget *CReadOnlyDelegate::createEditor(QWidget *parent,
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
///     重写显示问题（主要用于可配置的日期时间格式显示）
///
/// @param[in]  value   显示值
/// @param[in]  locale  本地
///
/// @return 显示字符串
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月19日，新建函数
///
QString CReadOnlyDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    return QStyledItemDelegate::displayText(ToCfgFmtDateTime(value.toString()), locale);
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
void CReadOnlyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS).isValid())
    {
        QString color = index.data(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS).toString();
        painter->fillRect(option.rect, color);
    }

	// 特殊背景的需求
	auto borderPic = index.model()->data(index, Qt::UserRole + 2);
	if (borderPic.canConvert<QPixmap>())
	{
		auto border = borderPic.value<QPixmap>();
		QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignLeft, border);
	}

    auto framePixmap = index.model()->data(index, Qt::UserRole);
    if (framePixmap.canConvert<QPixmap>())
    {
        // 绘制内容和背景
        QString text = index.data(Qt::DisplayRole).toString();
        if (!text.isEmpty())
        {
            QStyledItemDelegate::paint(painter, option, index);
        }

        // 绘制特殊背景
        bool isSelected = option.state & QStyle::State_Selected;
        if (isSelected && !text.isEmpty())
        {
            auto flagPix = framePixmap.value<QPixmap>();
            QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignLeft, flagPix);
        }

        return;
    }

	// 居中图片
	auto centerPic = index.model()->data(index, Qt::UserRole + 3);
	if (centerPic.canConvert<QPixmap>())
	{
		auto center = centerPic.value<QPixmap>();
		QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignCenter, center);
	}

    QStyleOptionViewItem optionNew = option;
	optionNew.textElideMode = Qt::ElideLeft;
    // 设置字体颜色
    auto fontColorType = index.model()->data(index, Qt::UserRole + 1);
    if (!fontColorType.isValid())
    {
        // 如果没取到则尝试在textColorRole中取
        fontColorType = index.model()->data(index, Qt::TextColorRole);
    }

    if (fontColorType.isValid())
    {
        if (fontColorType.canConvert<QColor>())
        {
            const auto& color = fontColorType.value<QColor>();
            optionNew.palette.setColor(QPalette::HighlightedText, color);
            optionNew.palette.setColor(QPalette::Text, color);
        }
        else
        {
            auto colortype = ColorType(fontColorType.toInt());
            if (colortype == ColorType::UPPERCOLOR)
            {
                optionNew.palette.setColor(QPalette::HighlightedText, QColor("#FA3741"));
                optionNew.palette.setColor(QPalette::Text, QColor("#FA3741"));
            }
            else if (colortype == ColorType::LOWERCOLOR)
            {
                optionNew.palette.setColor(QPalette::HighlightedText, QColor("#459FFF"));
                optionNew.palette.setColor(QPalette::Text, QColor("#459FFF"));
            }
        }
    }

	QStyledItemDelegate::paint(painter, optionNew, index);
}

QCircleButton::QCircleButton(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void QCircleButton::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    auto dataButtonPixMap = index.model()->data(index, Qt::DisplayRole);
    if (!dataButtonPixMap.canConvert<QPixmap>())
    {
        return;
    }

    // 是否显示自定义背景色
    bool specialBackGroud = index.model()->data(index, Qt::UserRole + 1).toBool();
    if (specialBackGroud)
    {
        painter->fillRect(QRect(option.rect.x() + 1, option.rect.y(), option.rect.width() - 1, option.rect.height() - 1), Qt::red);
    }
    // 普通绘制背景色
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }

    bool isSelected = option.state & QStyle::State_Selected;
    if (isSelected)
    {
        // 若需要叠加边框
        auto borderPic = index.model()->data(index, Qt::UserRole);
        if (borderPic.canConvert<QPixmap>())
        {
            auto border = borderPic.value<QPixmap>();
            QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignRight, border);
        }
    }

	// 特殊背景的需求
	auto borderPic = index.model()->data(index, Qt::UserRole + 2);
	if (borderPic.canConvert<QPixmap>())
	{
		auto border = borderPic.value<QPixmap>();
		QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignRight, border);
	}

    // button的绘制
    auto flagPix = dataButtonPixMap.value<QPixmap>();
    QApplication::style()->drawItemPixmap(painter, option.rect, Qt::AlignVCenter | Qt::AlignHCenter, flagPix);
}

///
/// @brief 结果的显示
///
/// @param[in]  parent  父控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月6日，新建函数
///
QResultDelegate::QResultDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QString QResultDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    return QString();
}

void QResultDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    // modify for bug2185 by wuht
    auto flag = index.data(Qt::UserRole + 1);
    auto text = index.data(Qt::DisplayRole).toString();
    int width = option.fontMetrics.width(text);
    if (flag.isValid())
    {
        painter->save();
        painter->setClipRect(option.rect);

        // 绘制标志
        if (flag.canConvert<QPixmap>())
        {
            auto flagPix = flag.value<QPixmap>();
            auto height = flagPix.height();
            auto flagWidth = flagPix.width();
            auto move = (option.rect.height() - height) / 2;

            auto needWidth = flagWidth + width;
            auto realWidth = option.rect.width();
            // 若需要的宽度大于实际的宽度，则从左边头开始绘制
            if (needWidth > realWidth)
            {
                // 1：绘制名称
                painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
                // 2：绘制稀释类型flag
                painter->drawPixmap(option.rect.left() + width + 1, option.rect.top() + move, flagPix);
            }
            // 若需要的宽度小于实际宽度，则需要居中绘制
            else
            {
                // 计算需要离左边位置的像素点
                auto blance = realWidth - needWidth;
                auto startPos = blance / 2;
                QRect tmpRect = option.rect;
                tmpRect.setLeft(option.rect.left() + startPos + 1);
                tmpRect.setWidth(width);

                // 1：绘制名称
                painter->drawText(tmpRect, Qt::AlignLeft | Qt::AlignVCenter, text);
                // 2：绘制稀释类型flag
                painter->drawPixmap(tmpRect.left() + width + 1, option.rect.top() + move, flagPix);
            }
        }
        // 绘制数字
        else if (flag.canConvert<int32_t>())
        {
            auto flagNumber = flag.value<int32_t>();
            int textWidth = option.fontMetrics.width(flagNumber);
            auto needWidth = textWidth + width;
            auto realWidth = option.rect.width();

            if (needWidth > realWidth)
            {
                // 1：绘制名称
                painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
                // 2：绘制数字
                QRect tmpRect = option.rect;
                tmpRect.setLeft(option.rect.left() + width + 1);
                tmpRect.setWidth(textWidth);
                painter->save();
                painter->setPen(QColor("#0C9586"));
                painter->drawText(tmpRect, Qt::AlignLeft | Qt::AlignVCenter, QString("(%1)").arg(flagNumber));
                painter->restore();
            }
            else
            {
                int textWidth = option.fontMetrics.width(flagNumber);
                // 计算需要离左边位置的像素点
                auto blance = realWidth - needWidth;
                auto startPos = blance / 2;

                QRect tmpRect = option.rect;
                tmpRect.setLeft(option.rect.left() + startPos + 1);
                tmpRect.setWidth(width);
                // 1：绘制名称
                painter->drawText(tmpRect, Qt::AlignLeft | Qt::AlignVCenter, text);

                tmpRect.setLeft(tmpRect.left() + 1 + width);
                tmpRect.setWidth(realWidth - width - startPos);
                // 2：绘制数字
                painter->save();
                painter->setPen(QColor("#0C9586"));
                painter->drawText(tmpRect, Qt::AlignLeft | Qt::AlignVCenter, QString("(%1)").arg(flagNumber));
                painter->restore();
            }
        }

        painter->restore();
    }
    else
    {
        if (width > option.rect.width())
        {
            painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
        else
        {
            painter->drawText(option.rect, Qt::AlignCenter, text);
        }

    }
    // 设置颜色
    QStyledItemDelegate::paint(painter, option, index);
}

///
/// @brief 校准品显示代理
///
/// @param[in]  parent  父控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年4月7日，新建函数
///
QPositionDelegate::QPositionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QString QPositionDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    return QString();
}

void QPositionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    auto flag = index.data(Qt::UserRole + 1);
    auto text = index.data(Qt::DisplayRole).toString();

    if (flag.isValid())
    {
        int width = option.fontMetrics.width(text);
        int rectwid = option.rect.width() / 2;

        QRect textrect = QRect(option.rect.left() + 5, option.rect.top() + 1, option.rect.width() / 2, option.rect.height() - 2);
        painter->drawText(textrect, Qt::AlignRight | Qt::AlignVCenter, text);

        // 绘制标志
        if (flag.canConvert<QPixmap>())
        {
            auto flagPix = flag.value<QPixmap>();
            auto height = flagPix.height();
            auto move = (option.rect.height() - height) / 2;
            QRect otherRect = QRect(option.rect.left() + option.rect.width() / 2, option.rect.top() + 1, option.rect.width() / 2, option.rect.height() - 2);
            painter->drawPixmap(otherRect.left(), otherRect.top() + move, flagPix);

        }
    }
    else
    {
        painter->save();
        if (text == QString(tr("请设置位置"))) painter->setPen(QColor(Qt::gray));
        painter->drawText(option.rect, Qt::AlignCenter, text);
        painter->restore();
    }

    // 设置颜色
    QStyledItemDelegate::paint(painter, option, index);
}

QStatusDelegate::QStatusDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void QStatusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    int adjustHeight = 17;
    int adjustWidth = 26;

    int leftX = option.rect.left() + 5;
    int rightX = option.rect.left() + adjustWidth + 7;
    int topY = option.rect.top() + 4;
    int bottomY = option.rect.top() + adjustHeight + 6;

    // 左上状态
    {
        const QVariant& flag = index.data(Qt::UserRole + 1);
        if (flag.isValid())
        {
            // 绘制标志
            if (flag.canConvert<QPixmap>())
            {
                painter->drawPixmap(leftX, topY, flag.value<QPixmap>());
            }
        }
    }

    {
        // 右上状态
        const QVariant& flag = index.data(Qt::UserRole + 2);
        if (flag.isValid())
        {
            // 绘制标志
            if (flag.canConvert<QPixmap>())
            {
                painter->drawPixmap(rightX, topY, flag.value<QPixmap>());
            }
        }
    }

    {
        // 底右状态
        const QVariant& flag = index.data(Qt::UserRole + 4);
        if (flag.isValid())
        {
            // 绘制标志
            if (flag.canConvert<QPixmap>())
            {
                painter->drawPixmap(rightX, bottomY, flag.value<QPixmap>());
            }
        }
    }

    {
        // 底左状态
        const QVariant& flag = index.data(Qt::UserRole + 3);
        if (flag.isValid())
        {
            // 绘制标志
            if (flag.canConvert<QPixmap>())
            {
                painter->drawPixmap(leftX, bottomY, flag.value<QPixmap>());
            }
        }
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QVariant QColorHeaderModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    int rCnt = rowCount();
    if ((role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        auto iter = m_mapRowAndColor.find(section);
        if (iter != m_mapRowAndColor.end())
        {
            return iter->second;
        }
    }

    return QStandardItemModel::headerData(section, orientation, role);
}

void QColorHeaderModel::SetColunmColor(int row, QColor color)
{
    m_mapRowAndColor[row] = color;
}

///
/// @brief 构造函数
///
/// @param[in]  parent  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月16日，新建函数
///
LineEdit::LineEdit(QWidget* parent /*= nullptr*/) : QLineEdit(parent), m_clearButton(nullptr)
{
    m_clearButton = new QPushButton(this);
    m_clearButton->setProperty("buttons", "share_cancel");

    connect(m_clearButton, &QPushButton::clicked, this, [&] {this->clear(); });
    //m_clearButton->setIcon(QIcon(QString(":/Leonis/resource/image/icon-cancel.png")));
    m_clearButton->setFixedSize(32, 32);

    m_clearButton->move(this->width() - m_clearButton->width(), (this->height() - m_clearButton->height()) / 2);

    this->setTextMargins(0, 0, m_clearButton->width(), 0);

    m_clearButton->installEventFilter(this);
}

///
/// @brief 窗口尺寸改变事件
///
/// @param[in]  event  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月16日，新建函数
///
void LineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);

    m_clearButton->move(this->width() - m_clearButton->width(), (this->height() - m_clearButton->height()) / 2);
}

///
/// @brief 事件过滤器
///
/// @param[in]  obj  
/// @param[in]  event  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月16日，新建函数
///
bool LineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_clearButton && event->type() == QEvent::HoverEnter)
    {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        m_clearButton->setFocus();
        return true;
    }
    else if (obj == m_clearButton && event->type() == QEvent::HoverLeave)
    {
        QApplication::restoreOverrideCursor();
        this->setFocus();
        return true;
    }
    return QLineEdit::eventFilter(obj, event);
}

QWidget * CIPDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto edit = new QLineEdit(parent);
    QRegExp regExpIp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    edit->setValidator(new QRegExpValidator(regExpIp));
    return qobject_cast<QWidget*>(edit);
}

QWidget * CPortDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto edit = new QLineEdit(parent);
    QRegExp regExpPort("([0-9]|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{4}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])");
    edit->setValidator(new QRegExpValidator(regExpPort));
    return qobject_cast<QWidget*>(edit);
}

TabPageCorner::TabPageCorner(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
    m_prePageButton = new QPushButton(this);
    m_nextPageButton = new QPushButton(this);

    m_prePageButton->setObjectName("prePageButton");
    m_nextPageButton->setObjectName("nextPageButton");

    m_curPageLabel = new QLabel(this);
    m_totalPageLabel = new QLabel(this);

    auto layout = new QHBoxLayout(this);
    layout->addWidget(m_prePageButton);
    layout->addWidget(m_curPageLabel);
    layout->addWidget(new QLabel("/"));
    layout->addWidget(m_totalPageLabel);
    layout->addWidget(m_nextPageButton);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);

    connect(m_prePageButton, &QPushButton::clicked, this, &TabPageCorner::SignalPrePage);
    connect(m_nextPageButton, &QPushButton::clicked, this, &TabPageCorner::SignalNextPage);
}

///
/// @brief 设置当前页和总页数，1-开始计数
///
/// @param[in]  curPage		当前页数
/// @param[in]  totalPage	总页数
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
void TabPageCorner::SetPage(int curPage, int totalPage)
{
    m_curPageLabel->setNum(curPage);
    m_totalPageLabel->setNum(totalPage);

    m_prePageButton->setEnabled(curPage > 1 && curPage <= totalPage);
    m_nextPageButton->setEnabled(curPage >= 1 && curPage < totalPage);
}

const int COUNT_PER_PAGE = 9;// 每页最多tab数
RFlagTabWidget::RFlagTabWidget(QWidget* parent) : QTabWidget(parent)
{
    m_tabBar = new RFlagTabBar(this);
    setTabBar(m_tabBar);

    m_curPageIndex = 0;
    m_totalPage = 1;

    this->setUsesScrollButtons(false);
    m_tabPageCorner = new TabPageCorner(this);
    this->setCornerWidget(m_tabPageCorner);
    connect(m_tabPageCorner, &TabPageCorner::SignalPrePage, this, [&]() {
        SetPage(m_curPageIndex - 1);
    });
    connect(m_tabPageCorner, &TabPageCorner::SignalNextPage, this, [&]() {
        SetPage(m_curPageIndex + 1);
    });

	connect(this, &RFlagTabWidget::currentChanged, this, [&](int index) {
		// index为窗口内的索引，不代表在容器内的索引
		auto index_vec = index + m_curPageIndex*COUNT_PER_PAGE;
		// 第一个tab默认是带Rflag(R标志认为是复查标志，第一个tab设计上为初查，当做带标志处理)
		if (index_vec <= 0 || std::find(m_vecFlag.cbegin(), m_vecFlag.cend(), index_vec) != m_vecFlag.cend())
		{
			emit SignalCurrentTabIsRFlag(true);
		}
		else
		{
			emit SignalCurrentTabIsRFlag(false);
		}
	});
}

void RFlagTabWidget::SetFlag(const std::vector<int>& vecFlag)
{
	m_vecFlag = vecFlag;
    m_tabBar->SetFlag(vecFlag);
    m_tabBar->update();
}

RotatedTabBar::RotatedTabBar(QWidget* parent /*= nullptr*/)
	: QTabBar(parent) 
{
	hoverIndex = -1;
	setMouseTracking(true);
}

void RotatedTabBar::paintEvent(QPaintEvent* event)
{
    QStylePainter painter(this);
    QStyleOptionTab tabOption;
    for (int index = 0; index < count(); ++index) {
        initStyleOption(&tabOption, index);
        painter.drawControl(QStyle::CE_TabBarTabShape, tabOption);
        painter.save();

        QPoint center = tabOption.rect.center();
        painter.translate(center);
        //painter.rotate(0);
        painter.translate(-center);
		if (index == currentIndex() || index == hoverIndex)
		{
			painter.setPen(Qt::white);
		}
		else
		{
			painter.setPen(Qt::black);
		}

        painter.drawText(tabOption.rect, Qt::AlignCenter, tabText(index));
        painter.restore();
    }
}

void RotatedTabBar::mouseMoveEvent(QMouseEvent* event)
{
	QTabBar::mouseMoveEvent(event);
	int index = tabAt((event->pos()));
	if (index != hoverIndex)
	{
		hoverIndex = index;
		update();
	}
}

void RotatedTabBar::enterEvent(QEvent* event)
{
	Q_UNUSED(event);
	auto potion = QCursor::pos();
	int index = tabAt(this->mapFromGlobal(potion));
	if (index != hoverIndex)
	{
		hoverIndex = index;
		update();
	}
}

void RotatedTabBar::leaveEvent(QEvent* event)
{
	QTabBar::leaveEvent(event);
	hoverIndex = -1;
	update();
// 	auto potion = QCursor::pos();
// 	int index = tabAt(this->mapFromGlobal(potion));
// 	if (index == -1)
// 	{
// 		hoverIndex = index;
// 		update();
// 	}
}

void RFlagTabBar::paintEvent(QPaintEvent* event)
{
    QTabBar::paintEvent(event);
    if (m_parentRFlagTabWidget == nullptr)
    {
        return;
    }

    QPainter painter(this);
    for (int i = 0; i < m_parentRFlagTabWidget->Count(); ++i)
    {
        if (std::find(m_vecFlag.cbegin(), m_vecFlag.cend(), i) == m_vecFlag.cend())
        {
            continue;
        }

        int pageIndex = -1;
        int indexInPage = -1;
        if (!m_parentRFlagTabWidget->GetPageIndex(i, indexInPage, pageIndex))
        {
            return;
        }
        // 不是当前页
        if (pageIndex != m_parentRFlagTabWidget->GetCurPageIndex())
        {
            continue;
        }

        auto rect = tabRect(indexInPage);
        painter.drawImage(QPoint(rect.topRight().x() - 26, rect.topRight().y()), QImage(":/Leonis/resource/image/R_flag.png"));
    }
}

///
/// @brief 根据在容器内的索引获取在当前页内的索引和页索引
///
/// @param[in]		index			tab在容器内的索引
/// @param[out]		indexInPage		当前页内的索引
/// @param[out]		pageIndex		页索引
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
bool RFlagTabWidget::GetPageIndex(int index, int& indexInPage, int& pageIndex)
{
    if (index < 0 || index >= m_vecTabs.size())
    {
        return false;
    }
    indexInPage = index % COUNT_PER_PAGE;
    pageIndex = index / COUNT_PER_PAGE;
    return true;
}

///
/// @brief 获取当前页索引
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
int RFlagTabWidget::GetCurPageIndex()
{
    return m_curPageIndex;
}

///
/// @brief 添加tab窗口
///
/// @param[in]  tab		tab窗口
/// @param[in]  name	tab窗口名称
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
void RFlagTabWidget::AddTab(QWidget* tab, QString name)
{
    if (tab == nullptr)
    {
        return;
    }

    m_vecTabs.append(std::tuple<QWidget*, QString>(tab, name));
    // 设置当前页数和总页数
	// bug0012620 modify by wuht
    m_totalPage = std::ceil(m_vecTabs.size()*1.0 / COUNT_PER_PAGE);
	if (m_totalPage < 2)
	{
		m_tabPageCorner->hide();
	}
	else
	{
		m_tabPageCorner->show();
	}

    SetPage(m_curPageIndex);
}

///
/// @brief	清除所有tab
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
void RFlagTabWidget::Clear()
{
	this->clear();
	for (auto& page : m_vecTabs)
	{
		auto pagePointer = std::get<0>(page);
		if (pagePointer != Q_NULLPTR)
		{
			delete pagePointer;
		}
	}

    m_vecTabs.clear();
    m_curPageIndex = 0;
    m_totalPage = 1;
}

///
/// @brief 获取所有tab数量，注意并不是所有tab都显示出来了
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
int RFlagTabWidget::Count()
{
    return m_vecTabs.size();
}

///
/// @brief 设置当前页
///
/// @param[in]  curPageIndex  当前页索引
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月11日，新建函数
///
void RFlagTabWidget::SetPage(int curPageIndex)
{
    if (curPageIndex < 0 || curPageIndex >= m_totalPage)
    {
        return;
    }

	// 判断页面是否在显示中
	auto IsInCurrentPage = [&](int currentIndex, int pageIndex)->bool
	{
		// 本页的开始序号
		int startIndex = m_curPageIndex*COUNT_PER_PAGE;
		// 本页的结束序号
		int endIndex = (m_curPageIndex + 1)*COUNT_PER_PAGE;

		if (currentIndex < startIndex)
		{
			return false;
		}

		if (currentIndex >= endIndex)
		{
			return false;
		}

		return true;
	};

    m_curPageIndex = curPageIndex;
    this->clear();
    for (auto index = 0; index < m_vecTabs.size(); ++index)
    {
		// 修改Bug0011227,当页面未被加入到显示中，此时页面显示不受控
		// 需要手动隐藏页面
		auto& tab = m_vecTabs[index];
		if (!IsInCurrentPage(index, m_curPageIndex))
		{
			auto pageWidget = std::get<0>(tab);
			if (pageWidget == Q_NULLPTR)
			{
				continue;
			}

			pageWidget->hide();
			continue;
		}

        this->addTab(std::get<0>(tab), std::get<1>(tab));
    }
    m_tabPageCorner->SetPage(m_curPageIndex + 1, m_totalPage);
}

QUtilitySortFilterProxyModel::QUtilitySortFilterProxyModel(QObject* obj)
    :QSortFilterProxyModel(obj)
{
    setSortRole(Qt::InitialSortOrderRole);
}

bool QUtilitySortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString strLeft = source_left.data().toString();
    QString strRight = source_right.data().toString();

    if (strLeft.isEmpty())
    {
        return sortOrder() == Qt::AscendingOrder ? false : true;
    }

    if (strRight.isEmpty())
    {
        return sortOrder() == Qt::AscendingOrder ? true : false;
    }

    return strLeft < strRight;
}

void QUtilitySortFilterProxyModel::SetTableViewSort(QTableView* view, const QVector<int>& vecSortColumn)
{
    m_vecSortColumn = vecSortColumn;

    auto sortHeader = new SortHeaderView(Qt::Horizontal, view);
    view->setHorizontalHeader(sortHeader);
    connect(sortHeader, &SortHeaderView::SortOrderChanged, this, [this, view, sortHeader](int logicIndex, SortHeaderView::SortOrder order)\
    {
        // 未设置排序的列不处理
        if (logicIndex >= 0 && !m_vecSortColumn.contains(logicIndex))
        {
            return;
        }

        // 无效列恢复默认
        if (logicIndex < 0 || order == SortHeaderView::NoOrder)
        {
            sortHeader->setSortIndicator(-1, Qt::DescendingOrder);  //去掉排序三角样式
            sort(-1, Qt::DescendingOrder);                          //-1则还原model默认顺序           
        }
        else
        {
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            view->sortByColumn(logicIndex, qOrderFlag);
        }
    });

    view->setSortingEnabled(true);
}

void AutoExpriteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex & index) const
{
    const auto& date = index.data(Qt::UserRole + DELEGATE_DATATIME_EXPIRE);
    QString text = index.data(Qt::DisplayRole).toString();
    QAbstractItemModel* pModel = (QAbstractItemModel*)index.model();

    // 如果不需要特殊处理的直接父类处理
    if (!date.isValid() || pModel == Q_NULLPTR || text.isEmpty())
        return CReadOnlyDelegate::paint(painter, option, index);

    // 不是有效时间，需要返回
    const auto& expirateDateTime = TimeStringToPosixTime(date.toString().toStdString());
    if (expirateDateTime.is_not_a_date_time())
        return CReadOnlyDelegate::paint(painter, option, index);

    // 绘制内容和背景
    int textColorRole = Qt::UserRole + 1;
    int bgColorRole = Qt::UserRole + DELEGATE_COLOR_OFFSET_POS;
    
    const auto& curDateTime = TimeStringToPosixTime(GetCurrentLocalTimeString());
    int daysValid = 0;

    if (expirateDateTime > curDateTime)
    {
        boost::posix_time::time_duration duration = expirateDateTime - curDateTime;
        // 过期时间按照24小时一天，并且向上取整（参照bug1315)
        daysValid = ceil((1.0 *  duration.total_seconds()) / (3600 * 24));
    }

    const QChar& type = text.at(0);
    //有类型
    if ((type > 'a' && type < "z") || (type > 'A' && type < "Z"))
    {
        text = QString("%1:%2").arg(type).arg(QString::number(daysValid));
    }
    else
    {
        text = QString::number(daysValid);
    }

    pModel->blockSignals(true);
    // 重新设置文字
    pModel->setData(index, text, Qt::DisplayRole);
    // 备注:2024/06/27 根据免疫Mantis29845,和周晓锋确认，只是不显示红色颜色，不是不显示过期天数
    if (daysValid <= 0 && DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
    {
        // 重新设置文字颜色
         pModel->setData(index, QColor(UI_REAGENT_NOTEFONT), textColorRole);
        // 重新设置背景颜色
        //pModel->setData(index, QColor(UI_REAGENT_WARNFONT), bgColorRole);
    }
    // 
    else if (!DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
    {
        // 重新设置文字颜色
        pModel->setData(index, QVariant(), textColorRole);
    }
    pModel->blockSignals(false);

    CReadOnlyDelegate::paint(painter, option, index);
}

CustomTabWidget::CustomTabWidget(QWidget* parent /*= nullptr*/) 
    : QTabWidget(parent)
{
    RotatedTabBar* pRotBar = new RotatedTabBar();

    connect(pRotBar, &RotatedTabBar::currentChanged, this, &CustomTabWidget::currentChanged);
    setTabBar(pRotBar);
}

QWidget * ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox* editor = new QComboBox(parent);
    editor->addItems(m_listItemText);

    QVariant data = index.model()->data(index, Qt::EditRole);
    if (!data.isNull())
    {
        int currentIndex = editor->findText(data.toString());
        if (currentIndex)
        {
            editor->setCurrentIndex(currentIndex);
        }
    }

    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
    if (comboBox)
    {
        comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
    if (comboBox)
    {
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

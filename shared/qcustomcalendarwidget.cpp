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
/// @file     qcustomcalendarwidget.cpp
/// @brief    自定义日历控件
///
/// @author   4170/TangChuXian
/// @date     2022年2月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年2月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qcustomcalendarwidget.h"
#include <QLocale> 
#include <QPainter>
#include <QTextCharFormat>
#include <QProxyStyle>
#include <QTableView>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSignalMapper>
#include <QMouseEvent>

QCustomCalendarWidget::QCustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent),
      m_enViewType(VIEW_TYPE_DAY),
      m_pCalendarWgt(Q_NULLPTR)
{
    initControl();
}

QCustomCalendarWidget::~QCustomCalendarWidget()
{

}

void QCustomCalendarWidget::initControl()
{
    setFixedSize(384, 394);
    setStyleSheet("border:none;border-top-left-radius:4px;border-top-right-radius:4px;background:white;");

    //layout()->setSizeConstraint(QLayout::SetFixedSize);
    //setLocale(QLocale(QLocale::system()));
    setLocale(QLocale(QLocale::system().language()));
    setNavigationBarVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
    setFirstDayOfWeek(Qt::Monday);
    setDateEditEnabled(false);
    //grabMouse();
    //setStyle(new QCustomStyle(this));

    QTextCharFormat format;
    format.setForeground(QColor(0x56, 0x56, 0x56));
    format.setBackground(QColor(255, 255, 255));

    setHeaderTextFormat(format);
    setWeekdayTextFormat(Qt::Saturday, format);
    setWeekdayTextFormat(Qt::Sunday,   format);
    setWeekdayTextFormat(Qt::Monday,   format);
    setWeekdayTextFormat(Qt::Tuesday,  format);
    setWeekdayTextFormat(Qt::Wednesday,format);
    setWeekdayTextFormat(Qt::Thursday, format);
    setWeekdayTextFormat(Qt::Friday,   format);

    // 获取日历界面
    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    if (vBodyLayout != Q_NULLPTR)
    {
        m_pCalendarWgt = vBodyLayout->itemAt(1)->widget();
        if (m_pCalendarWgt != Q_NULLPTR)
        {
            m_pCalendarWgt->setMouseTracking(true);
            m_pCalendarWgt->setStyleSheet("border-bottom-left-radius:4px;border-bottom-right-radius:4px;");
            m_pCalendarWgt->installEventFilter(this);
        }
    }

    initTopWidget();
    initMonthWidget();
    initYearWidget();

    connect(this, &QCalendarWidget::currentPageChanged, [this](int year, int month){
        setDataLabelTimeText(year, month);
    });
}

void QCustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    // 获取背景区域
    QRect bkRect(0, 0, 32, 32);
    bkRect.moveCenter(rect.center());

    // 画白色背景底色
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0xff, 0xff, 0xff));
    painter->drawRect(rect);
    painter->restore();

    // 如果鼠标悬停，画边框
    if (rect.contains(m_mousePoint))
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QColor(2, 91, 199));
        painter->setBrush(Qt::NoBrush);

        painter->drawRoundRect(bkRect, 2, 2);
        painter->restore();
    }

    if (date == selectedDate())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0x02, 0x5b, 0xc7));

        painter->drawRoundedRect(bkRect.x(), bkRect.y(), bkRect.width(), bkRect.height(), 2, 2);
        painter->setPen(QColor(255, 255, 255));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else if (date == QDate::currentDate())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        //painter->setBrush(QColor(0, 161, 255));
        //painter->drawRoundedRect(rect.x(), rect.y() + 3, rect.width(), rect.height() - 6, 3, 3);
        painter->setBrush(QColor(0xe5, 0xf2, 0xff));
        painter->drawRoundedRect(bkRect.x(), bkRect.y(), bkRect.width(), bkRect.height(), 2, 2);
        painter->setPen(QColor(0x56, 0x56, 0x56));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else if (date.month() == monthShown())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QColor(0x56, 0x56, 0x56));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else if (date.month() != monthShown())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QColor(0xb7, 0xb7, 0xb7));

        painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
        painter->restore();
    }
    else
    {
        QCalendarWidget::paintCell(painter, rect, date);
    }
}

void QCustomCalendarWidget::showEvent(QShowEvent *event)
{
    setLocale(QLocale(QLocale::system().language()));
    QCalendarWidget::showEvent(event);
    m_pMonthWidget->hide();
    m_pYearWidget->hide();
    m_enViewType = VIEW_TYPE_DAY;
    setDataLabelTimeText(yearShown(), monthShown());
}

bool QCustomCalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pCalendarWgt && event->type() == QEvent::MouseMove)
    {
        // 事件转换
        QMouseEvent* pMousEvt = dynamic_cast<QMouseEvent*>(event);
        if (pMousEvt == Q_NULLPTR)
        {
            return QCalendarWidget::eventFilter(watched, event);
        }

        // 更新坐标
        m_mousePoint = pMousEvt->pos();
        updateCells();
    }

    if (watched == m_pCalendarWgt && event->type() == QEvent::Leave)
    {
        // 重置坐标
        m_mousePoint = QPoint();
        updateCells();
    }

    return QCalendarWidget::eventFilter(watched, event);
}

void QCustomCalendarWidget::initTopWidget()
{
    QWidget* topWidget = new QWidget(this);
    topWidget->setObjectName("CalendarTopWidget");
    topWidget->setFixedHeight(44);
    topWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    topWidget->setStyleSheet("font-size:18px;\
    background-color:#025bc7;\
    color:#fff;");

    QHBoxLayout* hboxLayout = new QHBoxLayout();
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    hboxLayout->setSpacing(0);

    m_leftBtn   = new QPushButton(this);
    m_rightBtn  = new QPushButton(this);
    m_selDateBtn = new QPushButton(this);

    m_leftBtn->setObjectName("CalendarLeftBtn");
    m_rightBtn->setObjectName("CalendarRightBtn");
    m_selDateBtn->setObjectName("CalendarSelDateBtn");

    m_leftBtn->setFixedSize(50, 44);
    m_rightBtn->setFixedSize(50, 44);

    m_leftBtn->setStyleSheet("background:url(:/Leonis/resource/image/icon-left-arrow.png) center no-repeat;");
    m_rightBtn->setStyleSheet("background:url(:/Leonis/resource/image/icon-right-arrow.png) center no-repeat;");
    m_selDateBtn->setStyleSheet("QPushButton{background:transparent;color:white;}QPushButton:hover{color:rgb(141,181,230);}");
    //m_selDateBtn->setStyleSheet("QPushButton{background:rgb(231,139,139);color:white;}QPushButton:hover{color:rgb(141,181,230);}");

    hboxLayout->addWidget(m_leftBtn);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_selDateBtn);
    hboxLayout->addStretch();
    hboxLayout->addWidget(m_rightBtn);
    topWidget->setLayout(hboxLayout);

    //这里见下图1
    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(0, topWidget);
    vBodyLayout->setContentsMargins(1, 0, 0, 0);

    connect(m_leftBtn,   SIGNAL(clicked()),  this, SLOT(onbtnClicked()));
    connect(m_rightBtn,  SIGNAL(clicked()),  this, SLOT(onbtnClicked()));
    connect(m_selDateBtn, SIGNAL(clicked()), this, SLOT(OnSelDateBtnClicked()));

    setDataLabelTimeText(selectedDate().year(), selectedDate().month());
}

void QCustomCalendarWidget::initYearWidget()
{
    m_pYearWidget = new QWidget(this);
    m_pYearWidget->setFixedSize(384, 350);

    QGridLayout* pGridLayout = new QGridLayout();
    pGridLayout->setContentsMargins(0, 0, 0, 0);

    for (int iRow = 0; iRow < 4; iRow++)
    {
        for (int iCol = 0; iCol < 3 ; iCol++)
        {
            QPushButton* pYearBtn = new QPushButton;
            pGridLayout->addWidget(pYearBtn, iRow, iCol, Qt::AlignCenter);

            pYearBtn->setFixedSize(90, 40);

            connect(pYearBtn, SIGNAL(clicked()), this, SLOT(OnYearBtnClicked()));
        }
    }

    m_pYearWidget->setLayout(pGridLayout);
    m_pYearWidget->setStyleSheet("QPushButton:hover{border:1px solid rgb(2,91,199);border-bottom-left-radius:4px;border-bottom-right-radius:4px;}");

    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(1, m_pYearWidget);
    m_pYearWidget->hide();
}

void QCustomCalendarWidget::initMonthWidget()
{
    m_pMonthWidget = new QWidget(this);
    m_pMonthWidget->setFixedSize(384, 350);

    QGridLayout* pGridLayout = new QGridLayout();
    pGridLayout->setContentsMargins(0, 0, 0, 0);

    QSignalMapper *pSignalMapper = new QSignalMapper(this);
    for (int iRow = 0; iRow < 4; iRow++)
    {
        for (int iCol = 0; iCol < 3; iCol++)
        {
            QPushButton* pMonthBtn = new QPushButton;
            pGridLayout->addWidget(pMonthBtn, iRow, iCol, Qt::AlignCenter);

            pMonthBtn->setFixedSize(80, 40);
            pMonthBtn->setText(QDate::longMonthName(iRow * 3 + iCol + 1));

            pSignalMapper->setMapping(pMonthBtn, iRow * 3 + iCol + 1);
            connect(pMonthBtn, SIGNAL(clicked()), pSignalMapper, SLOT(map()));
        }
    }
    connect(pSignalMapper, SIGNAL(mapped(int)), this, SLOT(OnMonthBtnClicked(int)));

    m_pMonthWidget->setLayout(pGridLayout);
    m_pMonthWidget->setStyleSheet("QPushButton:hover{border:1px solid rgb(2,91,199);border-bottom-left-radius:4px;border-bottom-right-radius:4px;}");

    QVBoxLayout *vBodyLayout = qobject_cast<QVBoxLayout *>(layout());
    vBodyLayout->insertWidget(1, m_pMonthWidget);
    m_pMonthWidget->hide();
}

void QCustomCalendarWidget::setDataLabelTimeText(int year, int month)
{
    if (m_enViewType == VIEW_TYPE_DAY)
    {
        // remove(QDate(year, month, 1).toString("dd")
        //m_selDateBtn->setText(QStringLiteral("%1年%2月").arg(year).arg(month));
        m_selDateBtn->setText(GetGeneralYearMonth(year, month));
    }
    else if (m_enViewType == VIEW_TYPE_MONTH)
    {
        m_selDateBtn->setText(QString::number(year));
    }
    else if (m_enViewType == VIEW_TYPE_YEAR)
    {
        m_selDateBtn->setText(QStringLiteral("%1-%2").arg(year - (year % 10)).arg(year - (year % 10) + 9));
    }
}

///
/// @brief
///     获取国际化的年月字符串
///
/// @param[in]  iYear   年
/// @param[in]  iMmnth  月
///
/// @return 国际化年月字符串
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月17日，新建函数
///
QString QCustomCalendarWidget::GetGeneralYearMonth(int iYear, int iMmnth)
{
    QDate tmpDate(iYear, iMmnth, 20);
    QString strLongDate = tmpDate.toString(Qt::SystemLocaleLongDate);
    QString strMaskYearDate = strLongDate;
    strMaskYearDate.replace(tmpDate.toString("yyyy"), "xxxx");
    int iPosOfDay = strMaskYearDate.indexOf(tmpDate.toString("d"));
    if (iPosOfDay < 0)
    {
        return "";
    }

    int iPosOfMonth = strLongDate.indexOf(tmpDate.toString("MMMM"));
    int iPosOfYear = strLongDate.indexOf(tmpDate.toString("yyyy"));
    int iEndPosOfDay = -1;
    if (qMin(iPosOfMonth, iPosOfYear) > iPosOfDay)
    {
        iEndPosOfDay = qMin(iPosOfMonth, iPosOfYear);
    }
    else if (qMax(iPosOfMonth, iPosOfYear) > iPosOfDay)
    {
        iEndPosOfDay = qMax(iPosOfMonth, iPosOfYear);
    }

    // 移除日期
    if (iEndPosOfDay < 0)
    {
        return strLongDate.left(iPosOfDay);
    }

    return strLongDate.remove(iPosOfDay, iEndPosOfDay - iPosOfDay);
}

void QCustomCalendarWidget::onbtnClicked()
{
    QPushButton *senderBtn = qobject_cast<QPushButton *>(sender());
    if (senderBtn == m_leftBtn)
    {
        if (m_enViewType == VIEW_TYPE_DAY)
        {
            showPreviousMonth();
        }
        else if (m_enViewType == VIEW_TYPE_MONTH)
        {
            showPreviousYear();
            int iSelMonth = selectedDate().month();
            QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pMonthWidget->layout());
            for (int iRow = 0; iRow < 4; iRow++)
            {
                for (int iCol = 0; iCol < 3; iCol++)
                {
                    QPushButton* pMonthBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                    if (((iRow * 3 + iCol + 1) == iSelMonth) && (yearShown() == selectedDate().year()))
                    {
                        pMonthBtn->setStyleSheet("background:#025bc7;color:white;");
                        continue;
                    }
                    pMonthBtn->setText(QDate::longMonthName(iRow * 3 + iCol + 1));
                    pMonthBtn->setStyleSheet("background:white;color:0x565656;");
                }
            }
        }
        else if (m_enViewType == VIEW_TYPE_YEAR)
        {
            setCurrentPage(yearShown() - 10, monthShown());
            int iSelYear = selectedDate().year();
            int iStartYear = yearShown() - (yearShown() % 10) - 1;
            QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pYearWidget->layout());
            for (int iRow = 0; iRow < 4; iRow++)
            {
                for (int iCol = 0; iCol < 3; iCol++)
                {
                    QPushButton* pYearBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                    pYearBtn->setText(QString::number(iStartYear + (iRow * 3) + iCol));

                    if (((iRow * 3) + iCol == 0) || ((iRow * 3) + iCol == 11))
                    {
                        pYearBtn->setStyleSheet("background:white;color:#b7b7b7;");
                        continue;
                    }

                    if ((iStartYear + (iRow * 3) + iCol) == iSelYear)
                    {
                        pYearBtn->setStyleSheet("background:#025bc7;color:white;");
                        continue;
                    }

                    pYearBtn->setStyleSheet("background:white;color:0x565656");
                }
            }
        }
    }
    else if (senderBtn == m_rightBtn)
    {
        if (m_enViewType == VIEW_TYPE_DAY)
        {
            showNextMonth();
        }
        else if (m_enViewType == VIEW_TYPE_MONTH)
        {
            showNextYear();
            int iSelMonth = selectedDate().month();
            QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pMonthWidget->layout());
            for (int iRow = 0; iRow < 4; iRow++)
            {
                for (int iCol = 0; iCol < 3; iCol++)
                {
                    QPushButton* pMonthBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                    if (((iRow * 3 + iCol + 1) == iSelMonth) && (yearShown() == selectedDate().year()))
                    {
                        pMonthBtn->setStyleSheet("background:#025bc7;color:white;");
                        continue;
                    }
                    pMonthBtn->setText(QDate::longMonthName(iRow * 3 + iCol + 1));
                    pMonthBtn->setStyleSheet("background:white;color:0x565656;");
                }
            }
        }
        else if (m_enViewType == VIEW_TYPE_YEAR)
        {
            setCurrentPage(yearShown() + 10, monthShown());
            int iSelYear = selectedDate().year();
            int iStartYear = yearShown() - (yearShown() % 10) - 1;
            QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pYearWidget->layout());
            for (int iRow = 0; iRow < 4; iRow++)
            {
                for (int iCol = 0; iCol < 3; iCol++)
                {
                    QPushButton* pYearBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                    pYearBtn->setText(QString::number(iStartYear + (iRow * 3) + iCol));

                    if (((iRow * 3) + iCol == 0) || ((iRow * 3) + iCol == 11))
                    {
                        pYearBtn->setStyleSheet("background:white;color:#b7b7b7;");
                        continue;
                    }

                    if ((iStartYear + (iRow * 3) + iCol) == iSelYear)
                    {
                        pYearBtn->setStyleSheet("background:#025bc7;color:white;");
                        continue;
                    }

                    pYearBtn->setStyleSheet("background:white;color:0x565656");
                }
            }
        }
    }
}

void QCustomCalendarWidget::OnMonthBtnClicked(int iMonth)
{
    setCurrentPage(yearShown(), iMonth);
    m_pMonthWidget->hide();
    m_enViewType = VIEW_TYPE_DAY;
    setDataLabelTimeText(yearShown(), monthShown());
}

void QCustomCalendarWidget::OnYearBtnClicked()
{
    QPushButton *senderBtn = qobject_cast<QPushButton *>(sender());
    if (senderBtn == Q_NULLPTR)
    {
        return;
    }
    setCurrentPage(senderBtn->text().toInt(), monthShown());
    m_pYearWidget->hide();
    m_enViewType = VIEW_TYPE_MONTH;
    setDataLabelTimeText(yearShown(), monthShown());
    int iSelMonth = selectedDate().month();
    QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pMonthWidget->layout());
    for (int iRow = 0; iRow < 4; iRow++)
    {
        for (int iCol = 0; iCol < 3; iCol++)
        {
            QPushButton* pMonthBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
            if (((iRow * 3 + iCol + 1) == iSelMonth) && (yearShown() == selectedDate().year()))
            {
                pMonthBtn->setStyleSheet("background:#025bc7;color:white;");
                continue;
            }
            pMonthBtn->setText(QDate::longMonthName(iRow * 3 + iCol + 1));
            pMonthBtn->setStyleSheet("background:white;color:0x565656;");
        }
    }
}

void QCustomCalendarWidget::OnSelDateBtnClicked()
{
    if (m_enViewType == VIEW_TYPE_YEAR)
    {
        return;
    }

    if (m_enViewType == VIEW_TYPE_DAY)
    {
        m_pMonthWidget->show();
        m_enViewType = VIEW_TYPE_MONTH;
        //m_selDateBtn->setText();
        int iSelMonth = selectedDate().month();
        QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pMonthWidget->layout());
        for (int iRow = 0; iRow < 4; iRow++)
        {
            for (int iCol = 0; iCol < 3; iCol++)
            {
                QPushButton* pMonthBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                if (((iRow * 3 + iCol + 1) == iSelMonth) && (yearShown() == selectedDate().year()))
                {
                    pMonthBtn->setStyleSheet("background:#025bc7;color:white;");
                    continue;
                }
                pMonthBtn->setText(QDate::longMonthName(iRow * 3 + iCol + 1));
                pMonthBtn->setStyleSheet("background:white;color:0x565656;");
            }
        }

        setDataLabelTimeText(yearShown(), monthShown());
        return;
    }

    if (m_enViewType == VIEW_TYPE_MONTH)
    {
        m_pYearWidget->show();
        m_enViewType = VIEW_TYPE_YEAR;
        //m_selDateBtn->setStyleSheet("QPushButton{background:transparent;color:white;}");
        int iSelYear = selectedDate().year();
        int iStartYear = yearShown() - (yearShown() % 10) - 1;
        QGridLayout* pGridLayout = qobject_cast<QGridLayout*>(m_pYearWidget->layout());
        for (int iRow = 0; iRow < 4; iRow++)
        {
            for (int iCol = 0; iCol < 3; iCol++)
            {
                QPushButton* pYearBtn = qobject_cast<QPushButton*>(pGridLayout->itemAtPosition(iRow, iCol)->widget());
                pYearBtn->setText(QString::number(iStartYear + (iRow * 3) + iCol));

                if (((iRow * 3) + iCol == 0) || ((iRow * 3) + iCol == 11))
                {
                    pYearBtn->setStyleSheet("background:white;color:#b7b7b7;");
                    continue;
                }

                if ((iStartYear + (iRow * 3) + iCol) == iSelYear)
                {
                    pYearBtn->setStyleSheet("background:#025bc7;color:white;");
                    continue;
                }

                pYearBtn->setStyleSheet("background:white;color:0x565656");
            }
        }
        setDataLabelTimeText(yearShown(), monthShown());
        return;
    }
}

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
/// @file     qcustomcalendarwidget.h
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

#pragma once

#include <QCalendarWidget>
#include <QPoint>

class QPushButton;
class QLabel;
class QCustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT

public:
    QCustomCalendarWidget(QWidget *parent = Q_NULLPTR);
    ~QCustomCalendarWidget();

protected:
    // 视图类型
    enum VIEW_TYPE
    {
        VIEW_TYPE_DAY = 0,
        VIEW_TYPE_MONTH,
        VIEW_TYPE_YEAR
    };

private:
    void initControl();
    void initTopWidget();
    void initYearWidget();
    void initMonthWidget();
    void setDataLabelTimeText(int year, int month);

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
    QString GetGeneralYearMonth(int iYear, int iMmnth);

signals:
    void signalSetCalendarTime(const QDate& data);

private slots:
    void onbtnClicked();
    void OnMonthBtnClicked(int iMonth);
    void OnYearBtnClicked();
    void OnSelDateBtnClicked();

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void showEvent(QShowEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QPushButton* m_leftBtn;
    QPushButton* m_rightBtn;
    QPushButton* m_selDateBtn;

    QWidget*     m_pYearWidget;
    QWidget*     m_pMonthWidget;

    VIEW_TYPE    m_enViewType;

    QPoint       m_mousePoint;
    QWidget*     m_pCalendarWgt;
};

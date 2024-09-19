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
/// @file     QCustomCard.cpp
/// @brief 	  自定义下拉列表
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include <set>
#include <map>
#include <QWidget>
#include <QLabel>

namespace Ui {
    class QCustomCard;
}
class QHBoxLayout;
class QPushButton;
class QCustomLabel;
class QCheckBox;

///
/// @bref
///		用于缓存界面控件
///
struct CardCtrls
{
    QCustomLabel *label;
    QCheckBox *checkBox;

    CardCtrls()
        : label(nullptr)
        , checkBox(nullptr)
    {}
};

class QCustomCard : public QWidget
{
    Q_OBJECT

public:
    explicit QCustomCard(QWidget *parent = nullptr);
    ~QCustomCard();

    ///
    /// @bref
    ///		初始化时间控件数据
    ///
    /// @param[in] strTime 时间
    /// @param[in] selectedWeek 周
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    void UpdateData(const std::string& strTime, const std::vector<int>& selectedWeek);

    ///
    /// @bref
    ///		获取选择的天（0-6）
    ///
    /// @param[out] choisedDays 输出的选择
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    void GetSelectedDays(std::vector<int>& choisedDays);

    ///
    /// @bref
    ///		获取时间字符串 hh:MM:ss
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    QString GetTimeString();

	
	bool IsEmpty();

signals:
	// 清空信号
	void OnClearSignal();

	// 输入改变
	void InputChanged();

public slots:

    ///
    /// @bref
    ///		清空内容的槽函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    void OnClear();

    ///
    ///  @brief 复选框状态改变
    ///
    ///  @param[in]   state  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
    ///
    void OnChecked(int state);

    ///
    ///  @brief 点击标签删除按钮
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
    ///
    void OnLabelDelBtnClicked();
private:

    ///
    ///  @brief 初始化
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
    ///
    void InitWeekComb();
private:
    Ui::QCustomCard *ui;
    QHBoxLayout *m_hLayout;

    std::map<int, CardCtrls> m_labels; // 周下拉列表

    QStringList m_weekDays;     // 周1-7.
};

///////////////////////////////////////////////////////////////////////////
/// @file     QCustomCard.h
/// @brief    自定义标签
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class QCustomLabel : public QLabel
{
    Q_OBJECT

public:
    QCustomLabel(int day, QWidget *parent = Q_NULLPTR);

    ///
    ///  @brief 设置按钮大小
    ///
    ///  @param[in]   size  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
    ///
    void setBtnSize(int size);

private:

    ///
    ///  @brief 初始化
    ///  @param[in]   day  周几
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
    ///
    void Init(int day);

signals:
    void checked();

private:
    QPushButton*		m_btn;
    int					m_day;			// 周几
};


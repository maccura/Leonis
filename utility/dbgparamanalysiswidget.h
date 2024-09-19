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
/// @file     analysisparamwidget.h
/// @brief    应用界面->工程师调试->参数（免疫）分析
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <memory>

class QStandardItemModel;


namespace Ui
{
    class DbgParamAnalysisWidget;
};

namespace tf
{
    class GeneralAssayInfo;
};


class DbgParamAnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    DbgParamAnalysisWidget(QWidget *parent = Q_NULLPTR);
    ~DbgParamAnalysisWidget();

    ///
    /// @brief
    ///     加载指定项目的分析参数
    ///
    /// @param[in]  item  数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void LoadAnalysisParam(int iAssayCode);

    ///
    /// @bref
    ///		保存参数信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    bool SaveAnalysisInfo();

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
    ///
    /// @bref
    ///		用于过滤鼠标滚轮事件等
    ///
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    /// @brief
    ///     反应方式改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月11日，新建函数
    ///
    void OnReactionMethodChanged();

private:
    // 试剂参数，表头枚举
    enum RgntParamHeader {
        Rph_RgntSubPos = 0,     // 试剂子位置
        Rph_RgntVol,            // 试剂量（ul）
    };

    ///
    /// @bref
    ///		清空UI上的免疫相关参数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月12日，新建函数
    ///
    void CleanImParameters();

private:
    Ui::DbgParamAnalysisWidget			*ui;                            // UI指针
    bool							    m_bInit;                        // 是否已经初始化
    QStandardItemModel* m_tabModel;
    QStandardItemModel* m_tabModel2;
};

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
/// @file     dbgparamcaliwidget.h
/// @brief    应用界面->工程师调试->参数（免疫）校准
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
#include "src/thrift/im/gen-cpp/im_types.h"
#include "../../src/alg/im/IMM_ALG/imm_alg.h"

namespace Ui
{
    class DbgParamCaliWidget;
};

namespace tf
{
    class GeneralAssayInfo;
};
class QStandardItemModel;

namespace utilcomm{ 
    class CaliCurvParam;
};


class DbgParamCaliWidget : public QWidget
{
    Q_OBJECT

public:
    DbgParamCaliWidget(QWidget *parent = Q_NULLPTR);
    ~DbgParamCaliWidget();

    ///
    /// @bref
    ///		加质校准信息
    ///
    /// @param[in] iAssayCode 项目ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void LoadAnalysisParam(const utilcomm::CaliCurvParam& curvParm);

    ///
    /// @bref
    ///		获取校准信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    bool TakeCaliInfo(utilcomm::CaliCurvParam& uiParam);

protected:
    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @bref
    ///		对表格赋值
    ///
    /// @param[in] spanFix2Segment 曲线信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    void AssignCurvTable(const std::shared_ptr<im::SpanFix2Segment>& spanFix2Segment);

    ///
    /// @bref
    ///		清除界面的控件上的值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void CleanUi();

private:
    // 校准参数列表，表头枚举
    enum CaliParamHeader {
        Cph_SeqNo = 0,              // 序号
        Cph_CaliStartPoint,         // 校准起始点
        Cph_CaliEndPoint,           // 校准终止点
        Cph_MajorCurveStartPoint,   // 主标曲起始点
        Cph_MajorCurveEndPoint,     // 主标曲终止点
        Cph_CaliMethod,             // 校准方法
    };

private:
    Ui::DbgParamCaliWidget			*ui;                                // UI指针
    bool							m_bInit;                            // 是否已经初始化
    QStandardItemModel*             m_tabModel; // 曲线信息表格
};

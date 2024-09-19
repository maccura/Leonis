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
/// @file     QCalibrateTrack.h
/// @brief    校准追踪
///
/// @author   5774/WuHongTao
/// @date     2022年3月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "curveDraw/common.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class QStandardItemModel;
namespace Ui {
    class QCalibrateTrack;
};


class QCalibrateTrack : public BaseDlg
{
    Q_OBJECT

public:
    QCalibrateTrack(QWidget *parent = Q_NULLPTR);
    ~QCalibrateTrack();

    ///
    /// @brief
    ///     设置设备列表
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void SetDeivceList(std::vector<std::string>& devices);

private:
    ///
    /// @brief
    ///     初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月1日，新建函数
    ///
    void InitCurveAttribute();

    ///
    /// @brief
    ///     初始化项目列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void InitProjectCombox();

    ///
    /// @brief
    ///     重置校准品结构跟踪表
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void ResetCalibrateLevelResultTable();

    ///
    /// @brief
    ///     显示曲线的信息--以表格形式
    ///
    /// @param[in]  curve  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void ShowCurveTable(int row, const ch::tf::CaliCurve& curve, curveSingleDataStore& ContinueCurve);

    ///
    /// @brief
    ///     显示曲线
    ///
    /// @param[in]  curve  曲线的数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void ShowCurveData(curveSingleDataStore& curve);

private slots:
    ///
    /// @brief
    ///     坐标系自动跟随
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月4日，新建函数
    ///
    void OnAutoAxisUpdate();

    ///
    /// @brief
    ///     根据设置的手动数据更新坐标系
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月4日，新建函数
    ///
    void OnManualAxisUpdate();

    ///
    /// @brief
    ///     改变曲线的Y轴的最大值
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void OnChangeCurveMaxy(QString text);

    ///
    /// @brief
    ///     改变曲线的Y轴的最小值
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void OnChangeCurveMiny(QString text);

    ///
    /// @brief
    ///     选择不同的水平的校准品
    ///
    /// @param[in]  index  序号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void OnSelectAssay(int index);

    ///
    /// @brief
    ///     选择不同水准的校准品
    ///
    /// @param[in]  level  校准品水平
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月7日，新建函数
    ///
    void OnSelectCaliLevel(int level);

private:
    Ui::QCalibrateTrack*            ui;
    std::map<std::string, std::shared_ptr<ch::tf::GeneralAssayInfo>>      m_assayMap;
    QStandardItemModel*             m_CalibrateTrackResultMode; ///< 校准品的数据跟踪
    std::vector<std::string>        m_StrDevs;                  ///< 对应设备列表
    curveSingleDataStore            m_AbsorbCurve;              ///< 曲线对应的吸光度
};

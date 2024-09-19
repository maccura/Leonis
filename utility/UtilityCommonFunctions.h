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
/// @file     UtilityCommonFunctions.h
/// @brief    应用的通用工具函数
///
/// @author   8276/huchunli
/// @date     2023年7月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年7月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _UTILITYCOMMONFUNCTIONS_INCLUDE_H_
#define _UTILITYCOMMONFUNCTIONS_INCLUDE_H_

#include <QStringList>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "../../src/alg/im/IMM_ALG/imm_alg.h"

class QStandardItemModel;
class QTableView;
class QWidget;

namespace utilcomm
{
    ///
    /// @bref
    ///		用于缓存指定项目的校准曲线
    ///
    class CaliCurvCache
    {
    public:
        ///
        /// @bref
        ///		使用指定的项目编号初始化对应的校准曲线缓存
        ///
        /// @param[in] iAssayCode 项目编号
        ///
        /// @par History:
        /// @li 8276/huchunli, 2024年4月8日，新建函数
        ///
        bool Init(int iAssayCode);

        ///
        /// @bref
        ///		获取校准曲线的批号列表
        ///
        /// @param[out] vecLots 获取到的批号列表，如果是用于STAT模式的，则后后面添加“-STAT”
        ///
        /// @par History:
        /// @li 8276/huchunli, 2024年4月8日，新建函数
        ///
        void GetLotList(std::vector<std::string>& vecLots);

        ///
        /// @bref
        ///		获取一条最新的曲线
        ///
        /// @param[out] outCurv 最新的一条校准曲线
        /// @param[out] strLot 曲线对应的Lot标识
        ///
        /// @par History:
        /// @li 8276/huchunli, 2024年4月8日，新建函数
        ///
        bool GetLateastCurve(std::shared_ptr<im::tf::CaliCurve>& outCurv, std::string& strLot);

        ///
        /// @bref
        ///		通过校准批号获取曲线
        ///
        /// @param[in] strLot 校准批号
        ///
        /// @par History:
        /// @li 8276/huchunli, 2024年4月8日，新建函数
        ///
        std::shared_ptr<im::tf::CaliCurve> GetCurveByLot(const std::string& strLot);

    private:
        std::vector<std::shared_ptr<::im::tf::CaliCurve>> m_curvs;
        std::map<std::string, std::shared_ptr<::im::tf::CaliCurve>> m_curvsMap;
    };

     ///
     /// @bref
     ///		用于减少校准曲线解析的传出参数
     ///
    struct CaliCurvParam
    {
        bool isQuantita = false;
        std::shared_ptr<im::CalibratePoints> calibratePoints = nullptr;
        std::shared_ptr<im::Fit2Segment> fit2Segment = nullptr;
        std::shared_ptr<im::CutOffParams> cutoff = nullptr;
        std::shared_ptr<im::SpanFix2Segment> spanFix2Segment = nullptr;
    };

    ///
    /// @bref
    ///		获取免疫的稀释倍数列表.(如果不能从配置中获取，则采用默认列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    QStringList GetDuliRatioListIm();

    ///
    /// @bref
    ///		由于校准算法库中没有暴露calibratePoints的解析接口，暂时在此处做临时解析
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    bool ParseCaliJson(const std::string& json, CaliCurvParam& outCaliParam);
    bool ModifyCaliJson(std::string& json, const CaliCurvParam& caliParam);

    ///
    /// @bref
    ///		累加一步中的吸试剂量
    ///
    /// @param[in] suckList 吸取列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月19日，新建函数
    ///
    int GetSuckVol(const std::vector<::im::tf::SuckReagent>& suckList);

    ///
    /// @bref
    ///		对试剂表赋值
    ///
    /// @param[in] pView 表格
    /// @param[in] tbModel 表格模型
    /// @param[in] startIdx 第一步序号
    /// @param[in] suckList 吸取数据参数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月16日，新建函数
    ///
    void AssignReagentSuckTb(QWidget* parent, QTableView* pView, QStandardItemModel* tbModel, 
        int startIdx, const std::vector<::im::tf::reactReagInfo>& suckList);

    ///
    /// @bref
    ///		从试剂表格获取吸取信息
    ///
    /// @param[in] pView 表格
    /// @param[in] tbModel 表格模型
    /// @param[in] startIdx 第一步序号
    /// @param[out] suckList 吸取数据参数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月29日，新建函数
    ///
    void GetReagentSuckFromTb(QTableView* pView, QStandardItemModel* tbModel,
        int startIdx, std::vector<::im::tf::reactReagInfo>& suckList);
};

#endif

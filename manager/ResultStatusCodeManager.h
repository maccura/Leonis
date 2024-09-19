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
/// @file     ResultStatusCodeManager.h
/// @brief    结果状态码的映射与处理
///
/// @author   8276/huchunli
/// @date     2023年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _INCLUDE_RESULTSTATUSCODEMANAGER_H_
#define _INCLUDE_RESULTSTATUSCODEMANAGER_H_

#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <QObject>
#include "src/thrift/im/gen-cpp/im_types.h"

typedef std::map<std::string, im::tf::ResultCodeSet> ImResCodeMaps;


class ResultStatusCodeManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ResultStatusCodeManager> GetInstance();

    ~ResultStatusCodeManager();

    ///
    /// @brief  获取免疫结果状态设置表
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年4月19日，新建函数
    ///
    inline ImResCodeMaps GetResCodeSet() { return m_mapImResCodeSet; };

    ///
    /// @bref
    ///		设置是否显示Cali.M数据状态码
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月6日，新建函数
    ///
    inline void SetCaliMShowFlag(bool bShowCaliM) { m_showCaliM = bShowCaliM; };

    ///
    /// @bref
    ///		设置是否显示Cali.E数据状态码
    ///
    /// @par History:
    /// @li 1226/zhangjing，2024年3月14日，新建函数
    ///
    inline void SetCaliEShowFlag(bool bShowCaliE) { m_showCaliE = bShowCaliE; };

    ///
    /// @bref
    ///		获取免疫状态码与其描述的映射
    ///
    /// @param[out]  imDataAlarms  免疫的数据状态码
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月20日，新建函数
    ///
    void GetImResultStatus(std::vector<tf::DataAlarmItem>& imDataAlarms);

    ///
    /// @brief  修改后需要更新map中的记录
    ///
    /// @param[in]  name  名称
    /// @param[in]  rcs   结果状态设置信息
    ///
    /// @return 是否成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年4月19日，新建函数
    ///
    bool UpdateResCode(const std::string& name, const im::tf::ResultCodeSet& rcs);

    ///
    /// @brief  获取显示的结果状态码
    ///
    /// @param[in]  oldCodes  原状态码
    /// @param[in]  newCodes  显示状态码
    /// @param[in]  result    结果显示  
    /// @param[in]  refJud    参考范围标志
    /// @param[in]  criJud    危机范围标志
    /// @param[in]  sampleType 样本类型，不同样本类型对状态码的显示差异
    ///
    /// @return 是否成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年9月21日，新建函数
    ///
    bool GetDisplayResCode(const std::string& oldCodes, std::string& newCodes, 
        const std::string& result, int refJud, int criJud, int sampleType);

	///
	/// @brief 获取生化,ISE和LHI的数据报警信息
	///
	/// @param[in]  dataItem  项目详细信息
	/// @param[in]  isRetest  是否复查
	///
	/// @return 数据报警信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月18日，新建函数
	///
	QString GetChemistryDisplayResCode(const tf::TestItem& dataItem, bool isRetest);

	///
	/// @brief 获取状态码
	///
	/// @param[in]  status  原始状态码
	///
	/// @return 筛选后的状态码
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年6月5日，新建函数
	///
	QString GetResCode(std::string& status);

    ///
    ///  @brief 获取显示结果状态码的详情
    ///
    ///  @param[in]   resCodes  显示状态码
    ///
    ///  @return	显示结果状态码的详情
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年5月31日，新建函数
    ///
    std::string GetResCodesDiscrbe(std::string& resCodes);

    ///
    /// @bref
    ///		移除状态字符串中的质控排除状态码，返回移除后的状态列表
    ///
    /// @param[in] strStatus 原始状态字符串
    /// @param[out] lstStatu 移除后的状态列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月20日，新建函数
    ///
    QStringList EraseQcExcludeSymble(const QString& strStatus);

    ///
    /// @brief
    ///     获取质控排除计算结果状态描述
    ///
    /// @return 结果状态描述
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月29日，新建函数
    ///
    QString GetQCExcludeCodeDiscrbe();

    ///
    /// @brief  获取报警码设置
    ///
    /// @param[in]  name  名称
    /// @param[in]  rcs   结果状态设置信息
    ///
    /// @return 是否成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年12月25日，新建函数
    ///
    bool GetResStatusCodeSet(const std::string& name,im::tf::ResultCodeSet& rcs);

    protected slots:

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月6日，新建函数
    ///
    void OnPermisionChanged();


private:

    ///
    /// @bref
    ///		初始化加载结果状态码设置信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月20日，新建函数
    ///
    void InitResultCodeSet();

private:
    ResultStatusCodeManager();

    // 当前类的单例对象
    static std::shared_ptr<ResultStatusCodeManager> s_instance;
    static std::mutex m_userMgrMutx;

    std::vector<tf::DataAlarmItem> m_allDataAlarms;                 ///< 所有的数据状态码
    std::map<std::string, tf::DataAlarmItem> m_allDataAlarmsMap;    ///< <code, item>

    bool m_showCaliM;                                               ///< 是否显示Cali.M状态码
    bool m_showCaliE;                                               ///< 是否显示Cali.E状态码-设置校准提示功能

    ImResCodeMaps                       m_mapImResCodeSet;          ///< 免疫结果状态码设置
};

#endif

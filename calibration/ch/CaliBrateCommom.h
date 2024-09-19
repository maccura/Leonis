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
/// @file     CaliBrateCommom.h
/// @brief    校准公共函数集合
///
/// @author   5774/WuHongTao
/// @date     2022年3月3日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#define MAXPOINTER 200

struct curveSingleDataStore;
class QString;


class CaliBrateCommom
{
public:
    CaliBrateCommom();
    ~CaliBrateCommom();

    enum TimeType {
        LAST_ONE_DAY = 0,
        LAST_THREE_DAY = 1,
        LAST_ONE_WEEK = 2,
        LAST_ONE_MONTH = 3,
        LAST_HALF_YEAR = 4
    };

    ///
    /// @brief
    ///     创建曲线
    ///
    /// @param[in]  curve  曲线信息
    ///
    /// @return 成功返回曲线句柄否则为空
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    static std::shared_ptr<ch::ICalibrateCurve> GetCurve(const ch::tf::CaliCurve& curve);

    ///
    /// @brief
    ///     根据曲线句柄获取曲线的坐标点（离散）
    ///
    /// @param[in]  curveHandle  曲线句柄
    /// @param[out] ScatterCurve 可曲线
    ///
    /// @return 成功true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月3日，新建函数
    ///
    static bool CreatScatterCurve(curveSingleDataStore& ScatterCurve, std::shared_ptr<ch::ICalibrateCurve> curveHandle);

    ///
    /// @brief
    ///     根据离散曲线获取模拟的连续曲线
    ///
    /// @param[out] ContinueCurve 连续曲线 
    /// @param[in]  ScatterCurve  离散曲线
    ///
    /// @return 成功true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月3日，新建函数
    ///
    static bool CreatContinueCurve(curveSingleDataStore& ContinueCurve, curveSingleDataStore& ScatterCurve, std::shared_ptr<ch::ICalibrateCurve> curveHandle);

    ///
    /// @brief
    ///     根据结果，得到主曲线次曲线和主-次曲线
    ///
    /// @param[in]  result  测试结果
    /// @param[out]  CurveList  曲线列表，依次是主曲线，次曲线，主-次曲线
    ///
    /// @return 成功true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月3日，新建函数
    ///
    static bool GetScatterFromResult(ch::tf::AssayTestResult& result, std::vector<std::shared_ptr<curveSingleDataStore>>& CurveList);

    ///
    /// @brief
    ///     获取校准方法
    ///
    /// @param[in]  cureveHandel  校准曲线的数据句柄
    ///
    /// @return 校准方法字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月23日，新建函数
    ///
    static QString GetCalibrateMethod(std::shared_ptr<ch::ICalibrateCurve> cureveHandel);

	///
	/// @brief 获取校准类型
	///
	/// @param[in]  cureveHandel  校准曲线的数据句柄
	///
	/// @return 校准类型字符串
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月29日，新建函数
	///
    static QString GetCalibrateType(std::shared_ptr<ch::ICalibrateCurve> cureveHandel);

    ///
    /// @brief 获取执行方法
    ///
    /// @param[in]  caliMode  执行方法
    ///
    /// @return 执行方法类型字符串
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年1月11日，新建函数
    ///
    static QString GetCaliBrateMode(int caliMode);

    ///
    /// @brief
    ///     获取校准类型来源
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @return 试剂校准类型来源字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    static QString GetCaliBrateType(const ch::tf::ReagentGroup& reagentInfo);

    ///
    /// @brief
    ///     获取试剂
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    static QString GetCaliBrateStatus(const ch::tf::ReagentGroup& reagentInfo);

    ///
    /// @brief
    ///     获取试剂是开放或者封闭
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @return 试剂类型的字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    static QString GetCaliBrateReagentType(const ch::tf::ReagentGroup& reagentInfo);

    ///
    /// @brief
    ///     获取校准品的位置和杯类型
    ///
    /// @param[in]  doc  校准品信息
    ///
    /// @return 位置和类型字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月24日，新建函数
    ///
    static QString GetPosAndCupType(ch::tf::CaliDoc& doc);

	///
	/// @brief 根据条件获取曲线组
	///
	/// @param[in]  CurveCond  条件组
	///
	/// @return 曲线组
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	static std::vector<std::shared_ptr<ch::tf::CaliCurve>> GetCurvesByCond(const std::vector<::ch::tf::CaliCurveQueryCond>& CurveCond);

	///
	/// @brief 根据名称获取编号
	///
	/// @param[in]  widget  控件句柄
	///
	/// @return 控件编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	template<typename WidgetType>
	static int GetKeyValueFromWidgetName(WidgetType* widget)
	{
		int index = -1;

		if (widget == nullptr)
		{
			return index;
		}

		// 为空或者不是label_ 的对象名则跳过
		if (widget->objectName().isEmpty() || widget->objectName().indexOf("label_") != 0)
		{
			return index;
		}

		// 空则跳过
		QStringList names = widget->objectName().split("_");
		if (names.size() < 2)
		{
			return index;
		}

		bool ok = false;
		index = names.back().toInt(&ok, 10);
		return index;
	}

    ///
    /// @brief 从曲线中获取测试的杯号数据
    ///
    /// @param[in]  curve  曲线
    ///
    /// @return 图索引  key: 校准品水平   value:占四个数据（从0开始 首次杯号、同杯前反应、第二次杯号、第二次同杯前反应，没有数据的为空字符串）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月18日，新建函数
    ///
    static std::map<int64_t, std::vector<std::string>> GetCurveCupInfos(const ch::tf::CaliCurve& curve);
};


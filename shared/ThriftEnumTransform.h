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

#ifndef _INCLUDE_THRIFTENUMTRANSFORM_H_
#define _INCLUDE_THRIFTENUMTRANSFORM_H_

#include <QString>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/gen-cpp/statis_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

class ThriftEnumTrans 
{
public:

    /// @brief 根据类型编号获取类型名称
    ///
    /// @param[in]  sourceType  类型编号
    ///
    /// @return 类型名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年09月05日，新建函数
    ///
    static QString GetSourceTypeName(int32_t sourceType, const QString& defualtValue = "");

    ///
    /// @bref
    ///		把样本类型字符串转换成枚举值
    ///
    /// @param[in] strSampleType 样本类型字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月23日，新建函数
    ///
    static int TransSampleType(const QString& strSampleType);

	///
	/// @brief 获取样本类别
	///
	/// @param[in]  sampleType  样本类型（校准质控病人）
	/// @param[in]  isEmerge	是否急诊样本
	///
	/// @return 样本类别
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月15日，新建函数
	///
	static QString GetSampleTypeStat(const tf::SampleType::type sampleType, const bool isEmerge);

	///
	/// @brief 获取项目校准方法
	///
	/// @param[in]  caliType  项目校准方法的枚举
	///
	/// @return 校准方法的名称
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月15日，新建函数
	///
	static QString GetTestItemMethod(const ch::tf::CalibrationType::type caliType);

    ///
    /// @brief 获取校准状态
    ///
    ///
    /// @par History:
    /// @li  8580/GongZhiQiang，2023年2月2日，新建函数
    ///
    static QString GetCalibrateStatus(::tf::CaliStatus::type caliStatus);

	///
	/// @brief 获取分析方法
	///
	/// @param[in]  method  分析方法
	///
	/// @return 分析方法的名称
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月17日，新建函数
	///
	static QString GetAnalysisMethodName(ch::tf::AnalysisMethod::type& method);

    ///
    /// @brief 根据类型生成耗材名称
    ///
    /// @param[in]  supplyType  耗材类型
    ///
    /// @return 耗材名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月21日，新建函数
    ///
    static QString GetSupplyName(ch::tf::SuppliesType::type supplyType, bool IsReagent);

    ///
    /// @brief 根据类型生成耗材名称(ISE)
    ///
    /// @param[in]  supplyType  耗材类型
    ///
    /// @return 耗材名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月7日，新建函数
    ///
    static QString GetSupplyName(::ise::tf::SuppliesType::type supplyType);

    ///
    /// @brief  根据类型生成耗材名称(免疫)
    ///
    /// @param[in]  supplyType  耗材类型
    ///
    /// @return 耗材名称
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年3月29日，新建函数
    ///
    static QString GetImSupplyName(::im::tf::SuppliesType::type supplyType);

    ///
    /// @brief  根据类型生成耗材名称(免疫)
    ///
    /// @param[in]  supplyType  耗材类型
    ///
    /// @return 耗材名称
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年3月29日，新建函数
    ///
    static QString GetImSupplyAlarmSettingName(::im::tf::SuppliesType::type supplyType);

    ///
    /// @bref
    ///		质控等的登记方式（手工登记、扫描登记、下载登记等）
    ///
    /// @param[in] registType 登记方式枚举
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年4月18日，新建函数
    ///
    static QString GetRegistType(tf::DocRegistType::type registType);

    ///
    /// @bref
    ///		映射样本测试状态字符串
    ///
    /// @param[in] testSatuts 测试状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月14日，新建函数
    ///
    static QString GetSampleTestStatus(::tf::SampleStatus::type testSatuts);

    ///
    /// @brief 样本杯类型(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  type  样本杯类型枚举
    ///
    /// @return 样本杯类型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    static QString GetTubeType(const tf::TubeType::type & type, const QString& defualtValue = "");

    ///
    /// @brief 性别(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  spPatient  病人指针
    ///
    /// @return 性别
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    static QString GetGender(tf::Gender::type gType, const QString& defualtValue = "");

    ///
    /// @bref
    ///		把字符串翻译为性别枚举
    ///
    /// @param[in] strGender 性别字符串
    ///
    /// @return tf::Gender::type, 如果转换失败，则返回 -1
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月20日，新建函数
    ///
    static int TransGender(const QString& strGender);

    ///
    /// @brief 样本类型(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  sampleType  样本类型枚举
    /// @param[in]  stat  是否急诊
    ///
    /// @return 样本类型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    static QString GetOrderSampleType(const tf::SampleType::type& sampleType, bool stat);
    static QString GetOrderSampleType(const tf::HisSampleType::type& sampleType);

    ///
    /// @brief 检测模式(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  type  检测模式枚举
    ///
    /// @return 检测模式
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    static QString GetTestMode(const tf::TestMode::type& type);

    ///
    /// @brief 年龄(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  spPatient  病人指针
    ///
    /// @return 年龄
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    static QString GetAgeUnit(tf::AgeUnit::type ageUnit, const QString& defualtValue = "");
    static QString GetAge(const std::shared_ptr<tf::PatientInfo>& spPatient)
    {
        return spPatient == nullptr ? " " : (QString::number(spPatient->age) + GetAgeUnit(spPatient->ageUnit));
    }

    ///
    /// @bref
    ///		把年龄单位字符串转换为枚举
    ///
    /// @param[in] strAgeUnit 年龄单位字符串
    ///
    /// @return tf::AgeUnit::type, 如果转换失败，则返回 -1
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月20日，新建函数
    ///
    static int TransAgeUnit(const QString& strAgeUnit);

    ///
    /// @bref
    ///		样本量类型的翻译
    ///
    /// @param[in] sType 样本量类型枚举
    /// @param[in] defualtValue 缺省值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    static QString GetSuckVolType(tf::SuckVolType::type sType, const QString& defualtValue = "");

    ///
    /// @bref
    ///		获取项目类型对应的界面显示名
    ///
    /// @param[in] aType 项目类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月22日，新建函数
    ///
    static QString GetAssayClassfiyName(tf::AssayClassify::type aType, const QString& defualtValue = "");

    ///
    /// @bref
    ///		获取免疫的实验方法类型字符串
    ///
    /// @param[in] imWay 方法枚举
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月12日，新建函数
    ///
    static QString GetTestWayIm(::im::tf::reactPhaseType::type imWay);

    ///
    /// @bref
    ///		反应方式字符串转换成枚举，如果转换失败则返回-1
    ///
    /// @param[in] imWay 反应方式字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    static int TransTestWayIm(const QString& imWay); // ::im::tf::reactPhaseType::type

    ///
    /// @bref
    ///		转换免疫的耗材类型到全局定义的耗材类型(用于im_suppliesinfo的im.thrift转t_statissup的statis)
    ///
    /// @param[in] imSupp 免疫的耗材类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月9日，新建函数
    ///
    static tf::ConsumablesType::type TransImComsum(::im::tf::SuppliesType::type imSupp);
};

#endif

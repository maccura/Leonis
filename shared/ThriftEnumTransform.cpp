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

#include "ThriftEnumTransform.h"
#include "QObject"
#include "src/common/Mlog/mlog.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ise/IseConfigDefine.h"


/// @brief 根据类型编号获取类型名称
///
/// @param[in]  sourceType  类型编号
/// @param[in]  defualtValue  缺省值
///
/// @return 类型名称
///
/// @par History:
/// @li 5774/WuHongTao，2022年09月05日，新建函数
///
QString ThriftEnumTrans::GetSourceTypeName(int32_t sourceType, const QString& defualtValue)
{
    switch (tf::SampleSourceType::type(sourceType))
    {
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_OTHER:
        return QObject::tr("其他");
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_XQXJ:
        return QObject::tr("血清/血浆");
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NY:
        return QObject::tr("尿液");
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_QX:
        return QObject::tr("全血");
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NJY:
        return QObject::tr("脑脊液");
    case tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_JMQJY:
        return QObject::tr("浆膜腔积液");
    default:
        return defualtValue.isEmpty() ? QObject::tr("其他") : defualtValue;
    }
}

///
/// @bref
///		把样本类型字符串转换成枚举值
///
/// @param[in] strSampleType 样本类型字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年10月23日，新建函数
///
int ThriftEnumTrans::TransSampleType(const QString& strSampleType)
{
    if (strSampleType == QObject::tr("其他"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER;
    }
    else if (strSampleType == QObject::tr("血清/血浆"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ;
    }
    else if (strSampleType == QObject::tr("尿液"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY;
    }
    else if (strSampleType == QObject::tr("全血"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX;
    }
    else if (strSampleType == QObject::tr("脑脊液"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY;
    }
    else if (strSampleType == QObject::tr("浆膜腔积液"))
    {
        return tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY;
    }

    return -1;
}

QString ThriftEnumTrans::GetSampleTypeStat(const tf::SampleType::type sampleType, const bool isEmerge)
{
	if (sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
	{
		if (isEmerge)
		{
			return QObject::tr("急诊");
		}
		else
		{
			return QObject::tr("常规");
		}
	}
	else if (sampleType == tf::SampleType::type::SAMPLE_TYPE_CALIBRATOR)
	{
		return QObject::tr("校准");
	}
	else if (sampleType == tf::SampleType::type::SAMPLE_TYPE_QC)
	{
		return QObject::tr("质控");
	}
	else 
	{
		return QObject::tr("未知");
	}
}

QString ThriftEnumTrans::GetTestItemMethod(const ch::tf::CalibrationType::type caliType)
{
	QString caliTypeName = QObject::tr("未知");
	switch (caliType)
	{
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_ONE_POINT:
			caliTypeName = QObject::tr("线性方法一点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_TWO_POINTS:
			caliTypeName = QObject::tr("线性方法二点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_MULTI_POINTS:
			caliTypeName = QObject::tr("线性方法多点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG3P:
			caliTypeName = QObject::tr("logitlog 3点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG4P:
			caliTypeName = QObject::tr("logitlog 4点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG5P:
			caliTypeName = QObject::tr("logitlog 5点法");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_EXPONENTIAL:
			caliTypeName = QObject::tr("指数");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_SPLINE:
			caliTypeName = QObject::tr("样条");
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_AKIMA:
			caliTypeName = "akima";
			break;
		case ch::tf::CalibrationType::CALIBRATION_TYPE_PCHIP:
			caliTypeName = "pchip";
			break;
		default:
			break;
	}
		return caliTypeName;
}

///
/// @brief 获取校准状态
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
QString ThriftEnumTrans::GetCalibrateStatus(::tf::CaliStatus::type caliStatus)
{

    QString CalibrateStatus;
    switch (caliStatus)
    {
    case tf::CaliStatus::CALI_STATUS_DOING: // 正在校准
        CalibrateStatus = QObject::tr("正在校准");
        break;
    case tf::CaliStatus::CALI_STATUS_FAIL: // 校准失败
        CalibrateStatus = QObject::tr("校准失败");
        break;
    case tf::CaliStatus::CALI_STATUS_SUCCESS: // 校准成功
        CalibrateStatus = QObject::tr("校准成功");
        break;
    case tf::CaliStatus::CALI_STATUS_EDIT: // 编辑
        CalibrateStatus = QObject::tr("编辑");
        break;
    case tf::CaliStatus::CALI_STATUS_NOT:
        CalibrateStatus = QObject::tr("未校准");
        break;
    default:
        CalibrateStatus = "";
        break;
    }

    return CalibrateStatus;

}

QString ThriftEnumTrans::GetAnalysisMethodName(ch::tf::AnalysisMethod::type& method)
{
	QString AnalysisMethodName = QObject::tr("未知");
	switch (method)
	{
		case ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A:
			AnalysisMethodName = QObject::tr("速率A法");
			break;
		case ch::tf::AnalysisMethod::ANALYSIS_METHOD_1POINT:
			AnalysisMethodName = QObject::tr("1点法");
			break;
		case ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_END:
			AnalysisMethodName = QObject::tr("2点终点法");
			break;
		case ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_RATE:
			AnalysisMethodName = QObject::tr("2点速率法");
			break;
		case ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK:
			AnalysisMethodName = QObject::tr("速率A带样本空白修正法");
			break;
		default:
			break;
	}

	return  AnalysisMethodName;
}

QString ThriftEnumTrans::GetSupplyName(ch::tf::SuppliesType::type supplyType, bool IsReagent)
{
    switch (supplyType)
    {
        // 反应杯酸性清洗液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY:
        return CH_CUP_DETERGENT_ACIDITY_NAME;
		// 反应杯碱性清洗液
	case ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY:
		return CH_CUP_DETERGENT_ALKALINITY_NAME;
        // 样本/试剂针酸性清洗液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY:
		return IsReagent ? CH_DETERGENT_ACIDITY_NAME : CH_DETERGENT_ACIDITY_NAME;
        // 样本/试剂针碱性清洗液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY:
		return IsReagent ? CH_DETERGENT_ALKALINITY_NAME : CH_DETERGENT_ALKALINITY_NAME;
        // 抑菌剂
    case ch::tf::SuppliesType::SUPPLIES_TYPE_HITERGENT:
		return CH_HITERGENT_NAME;
    default:
        // 稀释液
        if ((supplyType >= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MIN)
            && (supplyType <= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MAX))
        {
			return CH_DILUENT_NAME + QString::number(supplyType);
        }
		return "";
    }
}

QString ThriftEnumTrans::GetSupplyName(::ise::tf::SuppliesType::type supplyType)
{
    switch (supplyType)
    {
        // 缓冲液
    case ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER:
        return ISE_BUF_NAME;
        // 内部标准液
    case ise::tf::SuppliesType::SUPPLIES_TYPE_IS:
		return ISE_IS_NAME;
        // 样本针酸性清洗液
    case ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY:
		return ISE_SAM_DETERGENT_ACIDITY_NAME;
        // 样本针碱性清洗液
    case ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY:
		return ISE_SAM_DETERGENT_ALKALINITY_NAME;
    default:
		return "";
    }
}

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
QString ThriftEnumTrans::GetImSupplyName(::im::tf::SuppliesType::type supplyType)
{
    QString supplyName = QObject::tr("未知");
    switch (supplyType)
    {
    case im::tf::SuppliesType::SUPPLIES_TYPE_WASH_WATER:// 清洗液
        supplyName = QObject::tr("清洗液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A:// 底物A
        supplyName = QObject::tr("底物液A");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B:// 底物B
        supplyName = QObject::tr("底物液B");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_PURE_WATER:// 纯水
        supplyName = QObject::tr("纯水");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS:// 新反应杯
        supplyName = QObject::tr("反应杯");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SOLID:// 固体废物
        supplyName = QObject::tr("废弃反应杯");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID: // 清洗缓冲液
        supplyName = QObject::tr("清洗缓冲液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID:// 废液
        supplyName = QObject::tr("废液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_CONFECT_LIQUID: // 配液桶
        supplyName = QObject::tr("配液桶");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT: // 稀释液
        supplyName = QObject::tr("稀释液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH:// 特殊清洗液
        supplyName = QObject::tr("特殊清洗液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_REAGENT:// 试剂
        supplyName = QObject::tr("试剂");
        break;
    default:
        break;
    }
    return supplyName;
}

///
/// @brief  根据类型生成耗材提示设置名称(免疫)
///
/// @param[in]  supplyType  耗材类型
///
/// @return 耗材名称
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
QString ThriftEnumTrans::GetImSupplyAlarmSettingName(::im::tf::SuppliesType::type supplyType)
{
    QString supplyName = QObject::tr("未知");
    switch (supplyType)
    {
    case im::tf::SuppliesType::SUPPLIES_TYPE_WASH_WATER:// 清洗液
        supplyName = QObject::tr("清洗液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A:// 底物A
        supplyName = QObject::tr("底物液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B:// 底物B
        supplyName = QObject::tr("底物液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_PURE_WATER:// 纯水
        supplyName = QObject::tr("纯水");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS:// 新反应杯
        supplyName = QObject::tr("反应杯");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SOLID:// 固体废物
        supplyName = QObject::tr("废弃反应杯");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID: // 清洗缓冲液
        supplyName = QObject::tr("清洗缓冲液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID:// 废液
        supplyName = QObject::tr("废液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_CONFECT_LIQUID: // 配液桶
        supplyName = QObject::tr("配液桶");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT: // 稀释液
        supplyName = QObject::tr("稀释液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH:// 特殊清洗液
        supplyName = QObject::tr("特殊清洗液");
        break;
    case im::tf::SuppliesType::SUPPLIES_TYPE_REAGENT:// 试剂
        supplyName = QObject::tr("试剂");
        break;
    default:
        break;
    }
    return supplyName;
}

QString ThriftEnumTrans::GetRegistType(tf::DocRegistType::type registType)
{
    switch (registType)
    {
    case tf::DocRegistType::REG_MANUAL:
        return QObject::tr("手工登记");
    case tf::DocRegistType::REG_SCAN:
        return QObject::tr("扫描登记");
    case tf::DocRegistType::REG_DOWNLOAD:
        return QObject::tr("下载登记");
    default:
        break;
    }

    return "";
}

///
/// @bref
///		映射样本测试状态字符串
///
/// @param[in] testSatuts 测试状态
///
/// @par History:
/// @li 8276/huchunli, 2023年8月14日，新建函数
///
QString ThriftEnumTrans::GetSampleTestStatus(::tf::SampleStatus::type testSatuts)
{
    switch (testSatuts)
    {
    case tf::SampleStatus::SAMPLE_STATUS_PENDING:
        return QObject::tr("待测");
    case tf::SampleStatus::SAMPLE_STATUS_TESTING:
        return QObject::tr("检测中");
    case tf::SampleStatus::SAMPLE_STATUS_TESTED:
        return QObject::tr("已完成");
    default:
        break;
    }

    return "";
}

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
QString ThriftEnumTrans::GetTubeType(const tf::TubeType::type & type, const QString& defualtValue /*= ""*/)
{
    switch (type)
    {
    case tf::TubeType::TUBE_TYPE_NORMAL:
        return QObject::tr("常规样本管");
    case tf::TubeType::TUBE_TYPE_MICRO:
        return QObject::tr("微量样本管");
    case tf::TubeType::TUBE_TYPE_ORIGIN:
        return QObject::tr("标准样本管");
    case tf::TubeType::TUBE_TYPE_STORE:
        return QObject::tr("样本贮存管");
    default:
        break;
    }
    return defualtValue;
}

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
QString ThriftEnumTrans::GetGender(tf::Gender::type gType, const QString& defualtValue /*= ""*/)
{
    switch (gType)
    {
    case tf::Gender::GENDER_MALE:
        return QObject::tr("男");
    case tf::Gender::GENDER_FEMALE:
        return QObject::tr("女");
    case tf::Gender::GENDER_UNKNOWN:
        return QObject::tr(" ");
    default:
        break;
    }
    return defualtValue;
}

///
/// @bref
///		把字符串翻译为性别枚举
///
/// @param[in] strGender 性别字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年10月20日，新建函数
///
int ThriftEnumTrans::TransGender(const QString& strGender)
{
    if (strGender == QObject::tr(" "))
    {
        return tf::Gender::type::GENDER_UNKNOWN;
    }
    if (strGender == QObject::tr("女"))
    {
        return tf::Gender::type::GENDER_FEMALE;
    }
    else if (strGender == QObject::tr("男"))
    {
        return tf::Gender::type::GENDER_MALE;
    }

    return -1;
}

QString ThriftEnumTrans::GetOrderSampleType(const tf::SampleType::type & sampleType, bool stat)
{
    QString orderType = " ";
    if (sampleType == tf::SampleType::SAMPLE_TYPE_PATIENT)
    {
        if (stat)
            orderType = QObject::tr("急诊");
        else
            orderType = QObject::tr("常规");
    }
    else if (sampleType == tf::SampleType::SAMPLE_TYPE_QC)
        orderType = QObject::tr("质控");
    else if (sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
        orderType = QObject::tr("校准");

    return orderType;
}

QString ThriftEnumTrans::GetOrderSampleType(const tf::HisSampleType::type & sampleType)
{
    QString orderType = " ";

    if (sampleType == tf::HisSampleType::SAMPLE_SOURCE_EM)
        orderType = QObject::tr("急诊");
    else if (sampleType == tf::HisSampleType::SAMPLE_SOURCE_NM)
        orderType = QObject::tr("常规");
    else if (sampleType == tf::HisSampleType::SAMPLE_SOURCE_QC)
        orderType = QObject::tr("质控");
    else if (sampleType == tf::HisSampleType::SAMPLE_SOURCE_CL)
        orderType = QObject::tr("校准");

    return orderType;
}

QString ThriftEnumTrans::GetTestMode(const tf::TestMode::type & type)
{
    QString mode = " ";
    if (type == tf::TestMode::BARCODE_MODE)
        mode = QObject::tr("条码模式");
    else if (type == tf::TestMode::SEQNO_MODE)
        mode = QObject::tr("样本号模式");
    else if (type == tf::TestMode::RACK_MODE)
        mode = QObject::tr("样本架模式");

    return mode;
}

QString ThriftEnumTrans::GetAgeUnit(tf::AgeUnit::type ageUnit, const QString& defualtValue)
{
    switch (ageUnit)
    {
    case tf::AgeUnit::AGE_UNIT_YEAR:
        return QObject::tr("岁");
    case tf::AgeUnit::AGE_UNIT_MONTH:
        return QObject::tr("月");
    case tf::AgeUnit::AGE_UNIT_DAY:
        return QObject::tr("天");
    case tf::AgeUnit::AGE_UNIT_HOUR:
        return QObject::tr("小时");
    default:
        break;
    }

    return defualtValue;
}

///
/// @bref
///		把年龄单位字符串转换为枚举
///
/// @param[in] strAgeUnit 年龄单位字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年10月20日，新建函数
///
int ThriftEnumTrans::TransAgeUnit(const QString& strAgeUnit)
{
    if (strAgeUnit == QObject::tr("年") || strAgeUnit == QObject::tr("岁"))
    {
        return tf::AgeUnit::type::AGE_UNIT_YEAR;
    }
    else if (strAgeUnit == QObject::tr("月"))
    {
        return tf::AgeUnit::type::AGE_UNIT_MONTH;
    }
    else if (strAgeUnit == QObject::tr("日") || strAgeUnit == QObject::tr("天"))
    {
        return tf::AgeUnit::type::AGE_UNIT_DAY;
    }
    else if (strAgeUnit == QObject::tr("小时"))
    {
        return tf::AgeUnit::type::AGE_UNIT_HOUR;
    }

    return -1;
}

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
QString ThriftEnumTrans::GetSuckVolType(tf::SuckVolType::type sType, const QString& defualtValue /*= ""*/)
{
    switch (sType)
    {
    case tf::SuckVolType::SUCK_VOL_TYPE_STD:
        return QObject::tr("常量");
    case tf::SuckVolType::SUCK_VOL_TYPE_DEC:
        return QObject::tr("减量");
    case tf::SuckVolType::SUCK_VOL_TYPE_INC:
        return QObject::tr("增量");
    default:
        break;
    }

    return defualtValue;
}

QString ThriftEnumTrans::GetAssayClassfiyName(tf::AssayClassify::type aType, const QString& defualtValue/* = ""*/)
{
    switch (aType)
    {
    case tf::AssayClassify::ASSAY_CLASSIFY_OTHER:
        return QObject::tr("其他");
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
        return QObject::tr("免疫");
    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
        return QObject::tr("生化");
    case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        return QObject::tr("电解质");
    case tf::AssayClassify::ASSAY_CLASSIFY_BLOOD:
        return QObject::tr("血球");
    case tf::AssayClassify::ASSAY_CLASSIFY_URINE:
        return QObject::tr("尿液");
    case tf::AssayClassify::ASSAY_CLASSIFY_CRUOR:
        return QObject::tr("凝血");
    default:
        break;
    }

    return defualtValue;
}

QString ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::type imWay)
{
    switch (imWay)
    {
    case im::tf::reactPhaseType::REACT_PHASE_FAST:
        return QObject::tr("快速模式");
    case im::tf::reactPhaseType::REACT_PHASE_ONE:
        return QObject::tr("一步法");
    case im::tf::reactPhaseType::REACT_PHASE_TWO:
        return QObject::tr("两步法");
    case im::tf::reactPhaseType::REACT_PHASE_FTWO:
        return QObject::tr("伪两步法");
    case im::tf::reactPhaseType::REACT_PHASE_PRE:
        return QObject::tr("预处理");
    default:
        break;
    }

    return "";
}

int ThriftEnumTrans::TransTestWayIm(const QString& imWay)
{
    static std::map<QString, ::im::tf::reactPhaseType::type> imTestVy{
        { QObject::tr("快速模式") , im::tf::reactPhaseType::REACT_PHASE_FAST },
        { QObject::tr("一步法") , im::tf::reactPhaseType::REACT_PHASE_ONE },
        { QObject::tr("两步法") ,im::tf::reactPhaseType::REACT_PHASE_TWO },
        { QObject::tr("伪两步法") ,im::tf::reactPhaseType::REACT_PHASE_FTWO },
        { QObject::tr("预处理") ,im::tf::reactPhaseType::REACT_PHASE_PRE }
    };

    std::map<QString, ::im::tf::reactPhaseType::type>::iterator it = imTestVy.find(imWay.trimmed());
    if (it == imTestVy.end())
    {
        ULOG(LOG_ERROR, "Unknown react phasetype! %s.", imWay.toStdString().c_str());
        return -1;
    }

    return it->second;
}

tf::ConsumablesType::type ThriftEnumTrans::TransImComsum(::im::tf::SuppliesType::type imSupp)
{
    switch (imSupp)
    {
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_WASH_WATER:               ///< 清洗液（缓存桶）
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_CONCENTRATE_LIQUID:       ///< 清洗缓冲液
        return tf::ConsumablesType::CONSUMABLES_IM_CONCENTRATE_LIQUID;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_SPECIAL_WASH:             ///< 特殊清洗液
        return tf::ConsumablesType::CONSUMABLES_IM_SPECIAL_WASH;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_SUBSTRATE_A:              ///< 底物A
        return tf::ConsumablesType::CONSUMABLES_IM_SUBSTRATE_A;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_SUBSTRATE_B:              ///< 底物
        return tf::ConsumablesType::CONSUMABLES_IM_SUBSTRATE_B;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_NEW_CUPS:                 ///< 新反应杯
        return tf::ConsumablesType::CONSUMABLES_IM_NEW_CUPS;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_DILUENT:                  ///< 稀释液
        return tf::ConsumablesType::CONSUMABLES_IM_DILUENT;

    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_SOLID:                    ///< 固体废物
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_WASTE_LIQUID:             ///< 废液
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_CONFECT_LIQUID:           ///< 配液桶
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_REAGENT:                  ///< 试剂
    case ::im::tf::SuppliesType::type::SUPPLIES_TYPE_PURE_WATER:               ///< 纯水
    default:
        break;
    }

    return tf::ConsumablesType::CONSUMABLES_IM_BASE;
}

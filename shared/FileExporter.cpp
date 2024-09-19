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
/// @file     FileExporter.cpp
/// @brief    文件导出类
///
/// @author   6889/ChenWei
/// @date     2021年11月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2021年11月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "FileExporter.h"
#include <QColor>
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"
#include <QTextStream>
#include <QtXlsx/QtXlsx>
#include <QApplication>
#include "Serialize.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "printcom.hpp"

#define UI_EXCEL_UNIT_NUM_PER_INCH_X                (18.0)                                  // excel表每英寸像素个数（非真实，用于调整excel表列宽）

std::shared_ptr<SEMetaInfos> SEMetaInfos::s_Instance = nullptr;

SEMetaInfos::SEMetaInfos()
{
    Init();
};

std::shared_ptr<SEMetaInfos> SEMetaInfos::GetInstance()
{
    // 获取实例对象
    if (nullptr == s_Instance)
    {
        s_Instance.reset(new SEMetaInfos());
    }

    return s_Instance;
}

std::shared_ptr<SEMetaInfo> SEMetaInfos::GetMetaInfo(ExportInfoEn key)
{
    auto iter = m_mapSEInfos.find(key);
    if (iter != m_mapSEInfos.end())
    {
        return iter.value();
    }
    else
    {
        return nullptr;
    }
}

///
///  @brief 初始化导出数据浏览/历史数据元信息
///
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月30日，新建函数
///
void SEMetaInfos::Init()
{
    QMap<ExportInfoEn, std::shared_ptr<SEMetaInfo>>& mapInfo = m_mapSEInfos;
    // 普通用户无导出权限
	// 生化
	mapInfo[ExportInfoEn::EIE_CALI_COUNT] = (std::make_shared<SEMetaInfo>(QObject::tr("次数"), &SampleExportInfo::strCaliCount, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_BARCODE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本条码号"), &SampleExportInfo::strBarcode, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_TYPESTAT] = (std::make_shared<SEMetaInfo>(QObject::tr("样本类别"), &SampleExportInfo::strSampleTypeStat, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_RACK] = (std::make_shared<SEMetaInfo>(QObject::tr("架号"), &SampleExportInfo::strRack, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_SEQ_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("样本号"), &SampleExportInfo::strSampleNo, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_POSITION] = (std::make_shared<SEMetaInfo>(QObject::tr("位置"), &SampleExportInfo::strPos, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALIBRATOR_NAME] = (std::make_shared<SEMetaInfo>(QObject::tr("校准品名称"), &SampleExportInfo::strCaliName, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALIBRATOR_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("校准品批号"), &SampleExportInfo::strCaliLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ASSAY_NAME] = (std::make_shared<SEMetaInfo>(QObject::tr("项目名称"), &SampleExportInfo::strItemName, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DILUTION_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("稀释倍数"), &SampleExportInfo::strDilutionFactor, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALCULATE_EMF] = (std::make_shared<SEMetaInfo>(QObject::tr("计算电动势"), &SampleExportInfo::strCalcuLateEmf, tf::UserType::USER_TYPE_ENGINEER));
	mapInfo[ExportInfoEn::EIE_DATA_ALARM] = (std::make_shared<SEMetaInfo>(QObject::tr("数据报警"), &SampleExportInfo::strResultStatus, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_MODULE] = (std::make_shared<SEMetaInfo>(QObject::tr("模块"), &SampleExportInfo::strTestModule, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALI_LEVEL] = (std::make_shared<SEMetaInfo>(QObject::tr("校准水平"), &SampleExportInfo::strCaliLevel, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CONC] = (std::make_shared<SEMetaInfo>(QObject::tr("浓度"), &SampleExportInfo::strConc, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_UNIT] = (std::make_shared<SEMetaInfo>(QObject::tr("单位"), &SampleExportInfo::strUnit, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_TEST_FINISHED_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("检测完成时间"), &SampleExportInfo::strEndTestTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_IS_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("IC批号"), &SampleExportInfo::strIsLOt, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_IS_SN] = (std::make_shared<SEMetaInfo>(QObject::tr("IC瓶号"), &SampleExportInfo::strIsSn, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DILUENT_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("BS批号"), &SampleExportInfo::strDiluentLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DILUENT_SN] = (std::make_shared<SEMetaInfo>(QObject::tr("BS瓶号"), &SampleExportInfo::strDiluentSn, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_APPLY_MODE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本申请模式"), &SampleExportInfo::strSampleSendModle, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本类型"), &SampleExportInfo::strSampleSourceType, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_TUBE_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本管类型"), &SampleExportInfo::strTubeType, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_PRE_DILUTION_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("手工稀释倍数"), &SampleExportInfo::strPreDilutionFactor, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REPEAT_TIMES] = (std::make_shared<SEMetaInfo>(QObject::tr("重复次数"), &SampleExportInfo::strFirstTestCounts, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ORIGINAL_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("原始结果"), &SampleExportInfo::strOrignialTestResult, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("结果"), &SampleExportInfo::strTestResult, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_AI_RECOGNIZE_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("视觉识别结果"), &SampleExportInfo::strAIResult, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_PROBE_ACIDITY_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("针酸性清洗液批号"), &SampleExportInfo::strSampleProbeAcidityLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_SAMPLE_PROBE_ALKALINITY_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("针碱性清洗液批号"), &SampleExportInfo::strSampleProbeAlkalintyLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALI_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("校准时间"), &SampleExportInfo::strCaliTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ADD_REAGENT1_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("加R1时间"), &SampleExportInfo::strAddReagent1Time, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ADD_SAMPLE_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("加样本时间"), &SampleExportInfo::strAddSampleTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_VORTEX_TIME1] = (std::make_shared<SEMetaInfo>(QObject::tr("混匀时间"), &SampleExportInfo::strVortexTime1, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ADD_REAGENT2_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("加R2时间"), &SampleExportInfo::strAddReagent2Time, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_VORTEX_TIME2] = (std::make_shared<SEMetaInfo>(QObject::tr("混匀时间2"), &SampleExportInfo::strVortexTime2, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ABS] = (std::make_shared<SEMetaInfo>(QObject::tr("吸光度"), &SampleExportInfo::strAbs, tf::UserType::USER_TYPE_ENGINEER));
	mapInfo[ExportInfoEn::EIE_SUCK_VOL] = (std::make_shared<SEMetaInfo>(QObject::tr("样本量"), &SampleExportInfo::strSuckVol, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_AUTO_DILUTION_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("机内稀释倍数"), &SampleExportInfo::strDilutionFactor, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REAGENT_DISK] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂盘"), &SampleExportInfo::strReagentDisk, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REACTION_CUP_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("反应杯号"), &SampleExportInfo::strAssayCupBatchNo, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REAGENT_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂批号"), &SampleExportInfo::strReagentBatchNo, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REAGENT_SN] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂瓶号"), &SampleExportInfo::strReagentSerialNo, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REAGENT_REGISTER_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂上机时间"), &SampleExportInfo::strRgtRegisterT, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REAGENT_EXPIRY_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂失效日期"), &SampleExportInfo::strReagentExpTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_OPEN_BOTTLE_EXPIRY_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("开瓶有效期"), &SampleExportInfo::strOpenBottleExpiryTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ACIDITY_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("酸性清洗液批号"), &SampleExportInfo::strAcidityLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ALKALINITY_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("碱性清洗液批号"), &SampleExportInfo::strAlkalintyLot, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ANALYSIS_METHOD] = (std::make_shared<SEMetaInfo>(QObject::tr("分析方法"), &SampleExportInfo::strAnalysisMethod, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_PRIMARY_WAVE] = (std::make_shared<SEMetaInfo>(QObject::tr("主波长"), &SampleExportInfo::strPrimaryWave, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DEPUTY_WAVE] = (std::make_shared<SEMetaInfo>(QObject::tr("次波长"), &SampleExportInfo::strDeputyWave, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_REACTION_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("反应时间"), &SampleExportInfo::strReactionTime, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DETECT_POINT] = (std::make_shared<SEMetaInfo>(QObject::tr("测光点"), &SampleExportInfo::strDetectPoint, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_PRIMARY_WAVE_ABS] = (std::make_shared<SEMetaInfo>(QObject::tr("主波长吸光度"), &SampleExportInfo::strPrimaryWaveAbs, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DEPUTY_WAVE_ABS] = (std::make_shared<SEMetaInfo>(QObject::tr("次波长吸光度"), &SampleExportInfo::strDeputyWaveAbs, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_ALL_WAVE_ABS] = (std::make_shared<SEMetaInfo>(QObject::tr("所有波长吸光度"), &SampleExportInfo::strAllWaveAbs, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALIBRATION_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("校准方法"), &SampleExportInfo::strCaliType, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALIBRATION_POINTS] = (std::make_shared<SEMetaInfo>(QObject::tr("校准点数"), &SampleExportInfo::strCaliPoints, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DILUTION_SAMPLE_VOL] = (std::make_shared<SEMetaInfo>(QObject::tr("稀释样本量"), &SampleExportInfo::strDilutionSampleVol, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_DILUTION_VOL] = (std::make_shared<SEMetaInfo>(QObject::tr("稀释液量"), &SampleExportInfo::strDiluentVol, tf::UserType::USER_TYPE_ADMIN));
	mapInfo[ExportInfoEn::EIE_CALCULATE_ABS] = (std::make_shared<SEMetaInfo>(QObject::tr("计算吸光度"), &SampleExportInfo::strCalculateAbs, tf::UserType::USER_TYPE_ADMIN));

	//-------------免疫-------
    mapInfo[ExportInfoEn::EIE_IM_SAMPLE_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("样本号"), &SampleExportInfo::strSampleNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_BARCODE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本条码"), &SampleExportInfo::strBarcode, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_SAMPLE_SOURCE_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本类型"), &SampleExportInfo::strSampleSourceType, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_TUBE_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("样本管类型"), &SampleExportInfo::strTubeType, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_POS] = (std::make_shared<SEMetaInfo>(QObject::tr("位置"), &SampleExportInfo::strPos, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_END_TEST_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("检测完成时间"), &SampleExportInfo::strEndTestTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_TEST_MODLE] = (std::make_shared<SEMetaInfo>(QObject::tr("检测模式"), &SampleExportInfo::strTestModle, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_SAMPLE_SEND_MODLE] = (std::make_shared<SEMetaInfo>(QObject::tr("进样模式"), &SampleExportInfo::strSampleSendModle, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_ITEM_NAME] = (std::make_shared<SEMetaInfo>(QObject::tr("项目名称"), &SampleExportInfo::strItemName, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RUL] = (std::make_shared<SEMetaInfo>(QObject::tr("信号值"), &SampleExportInfo::strRUL, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_ORIGNIAL_RUL] = (std::make_shared<SEMetaInfo>(QObject::tr("原始信号值"), &SampleExportInfo::strOrignialRUL, tf::UserType::USER_TYPE_ENGINEER)); // 工程师以上权限可导出
    mapInfo[ExportInfoEn::EIE_IM_TEST_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("检测结果"), &SampleExportInfo::strTestResult, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_ORIGNIAL_TEST_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("原始结果"), &SampleExportInfo::strOrignialTestResult, tf::UserType::USER_TYPE_ENGINEER));
    mapInfo[ExportInfoEn::EIE_IM_RESULT_STATUS] = (std::make_shared<SEMetaInfo>(QObject::tr("数据报警"), &SampleExportInfo::strResultStatus, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_AI_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("视觉识别结果"), &SampleExportInfo::strAIResult, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_UNIT] = (std::make_shared<SEMetaInfo>(QObject::tr("单位"), &SampleExportInfo::strUnit, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_TEST_MODULE] = (std::make_shared<SEMetaInfo>(QObject::tr("模块"), &SampleExportInfo::strTestModule, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_PRE_DILUTION_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("手工稀释倍数"), &SampleExportInfo::strPreDilutionFactor, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_DILUTION_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("机内稀释倍数"), &SampleExportInfo::strDilutionFactor, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_REAGENT_BATCHNO] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂批号"), &SampleExportInfo::strReagentBatchNo, tf::UserType::USER_TYPE_ADMIN));

    mapInfo[ExportInfoEn::EIE_IM_REAGENT_SERIAL_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂瓶号"), &SampleExportInfo::strReagentSerialNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_SUBSTRATE_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("底物液批号"), &SampleExportInfo::strSubstrateBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_SUBSTRATE_BOTTLE_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("底物液瓶号"), &SampleExportInfo::strSubstrateBottleNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_CLEAN_FLUID_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("清洗缓冲液批号"), &SampleExportInfo::strCleanFluidBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_ASSAY_CUP_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("反应杯批号"), &SampleExportInfo::strAssayCupBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_DILUENT_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("稀释液批号"), &SampleExportInfo::strDiluentBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RGT_REGISTER_T] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂上机时间"), &SampleExportInfo::strRgtRegisterT, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_REAGENT_EXP_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂失效日期"), &SampleExportInfo::strReagentExpTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_CALI_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("校准品批号"), &SampleExportInfo::strCaliLot, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_FIRST_CUR_CALI_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("当前工作曲线"), &SampleExportInfo::strFirstCurCaliTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_FIRST_TEST_COUNTS] = (std::make_shared<SEMetaInfo>(QObject::tr("项目重复次数"), &SampleExportInfo::strFirstTestCounts, tf::UserType::USER_TYPE_ADMIN));

    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_TUBE_TYPE] = (std::make_shared<SEMetaInfo>(QObject::tr("复查样本管类型"), &SampleExportInfo::strReTestTubeType, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_RUL] = (std::make_shared<SEMetaInfo>(QObject::tr("复查信号值"), &SampleExportInfo::strReTestRUL, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_ORIG_LRUL] = (std::make_shared<SEMetaInfo>(QObject::tr("复查原始信号值"), &SampleExportInfo::strReTestOriglRUL, tf::UserType::USER_TYPE_ENGINEER));    // 工程师以上权限可导出
    mapInfo[ExportInfoEn::EIE_IM_RE_TESTRESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("复查结果"), &SampleExportInfo::strReTestResult, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_ORIGL_TEST_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("复查原始结果"), &SampleExportInfo::strReOriglTestResult, tf::UserType::USER_TYPE_ENGINEER));
    mapInfo[ExportInfoEn::EIE_IM_RE_RESULT_STATUS] = (std::make_shared<SEMetaInfo>(QObject::tr("复查数据报警"), &SampleExportInfo::strReResultStatus, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_AI_RESULT] = (std::make_shared<SEMetaInfo>(QObject::tr("复查视觉识别结果"), &SampleExportInfo::strReAIResult, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_MODULE] = (std::make_shared<SEMetaInfo>(QObject::tr("模块"), &SampleExportInfo::strReTestModule, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_PRE_DIL_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("复查手工稀释倍数"), &SampleExportInfo::strRePreDilFactor, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_DIL_FACTOR] = (std::make_shared<SEMetaInfo>(QObject::tr("复查机内稀释倍数"), &SampleExportInfo::strReDilFactor, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_END_TEST_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("复查完成时间"), &SampleExportInfo::strReEndTestTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_MODLE] = (std::make_shared<SEMetaInfo>(QObject::tr("复查检测模式"), &SampleExportInfo::strReTestModle, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_SAMPLE_SEND_MODLE] = (std::make_shared<SEMetaInfo>(QObject::tr("复查进样模式"), &SampleExportInfo::strReSampleSendModle, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_REAGENT_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂批号"), &SampleExportInfo::strReReagentBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_REAGENT_SERIAL_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂瓶号"), &SampleExportInfo::strReReagentSerialNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_SUB_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("底物液批号"), &SampleExportInfo::strReSubBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_SUB_BOTTLE_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("底物液瓶号"), &SampleExportInfo::strReSubBottleNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_CLEAN_FLUID_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("清洗缓冲液批号"), &SampleExportInfo::strReCleanFluidBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_ASSAY_CUP_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("反应杯批号"), &SampleExportInfo::strReAssayCupBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_DILUENT_BATCH_NO] = (std::make_shared<SEMetaInfo>(QObject::tr("稀释液批号"), &SampleExportInfo::strReDiluentBatchNo, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_RGT_REGISTER_T] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂上机时间"), &SampleExportInfo::strReRgtRegisterT, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_REAGENT_EXP_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("试剂失效日期"), &SampleExportInfo::strReReagentExpTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_CALI_LOT] = (std::make_shared<SEMetaInfo>(QObject::tr("校准品批号"), &SampleExportInfo::strReCaliLot, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_CUR_CALI_TIME] = (std::make_shared<SEMetaInfo>(QObject::tr("当前工作曲线"), &SampleExportInfo::strReTestCurCaliTime, tf::UserType::USER_TYPE_ADMIN));
    mapInfo[ExportInfoEn::EIE_IM_RE_TEST_COUNTS] = (std::make_shared<SEMetaInfo>(QObject::tr("复查项目重复次数"), &SampleExportInfo::strReTestCounts, tf::UserType::USER_TYPE_ADMIN));

    //return mapInfo;
}
//QMap<ExportInfoEn, std::shared_ptr<SEMetaInfo>> SEMetaInfos::m_mapSEInfos = SEMetaInfos::Init();

///
///  @brief 将传入的数据转换成要导出的字符串链表
///
///
///  @param[in]   vecRecord  要导出的数据
///  @param[out]   strExportTextList 生成的字符串链表
///
///  @par History: 
///  @li 6889/ChenWei，2023年12月15日，新建函数
///
template<typename StructType>
void MakeExportList(std::vector<StructType, std::allocator<StructType>>& vecRecord, QStringList& strExportTextList)
{
    // 生成表头
    QString strHeader("");
    std::shared_ptr<Serializer> pSerializer = SerializerMap::GetInstance()->getSerializer(typeid(StructType).name());
    if (pSerializer == nullptr)
        return;

    int iFCount = pSerializer->getFieldsCount();
    for (int i = 0; i < iFCount; i++)
    {
        std::string strName = pSerializer->getDisplayName(i);
        strHeader = strHeader + QString::fromStdString(strName) + "\t";
    }

    strExportTextList.push_back(strHeader);

    // 生成内容
    for (int i = 0; i < vecRecord.size(); i++)
    {
        // 文本行
        QString strRowString("");
        StructType& item = vecRecord.at(i);
        for (int j = 0; j < iFCount; j++)
        {
            if (pSerializer->getFieldType(j) == "std::string")
            {
                std::string strValue = item.*(pSerializer->FieldPtr<StructType, std::string>(j));
                strRowString += QString::fromStdString(strValue) + "\t";
            }
        }

        strExportTextList.push_back(strRowString);
    }
}

/********************************************************************************************/
// 文件导出类
FileExporter::FileExporter()
{
}

FileExporter::~FileExporter()
{
}

///
///  @brief 导出数据浏览/历史数据
///
///
///  @param[in]   vecSampleInfo  试剂信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月30日，新建函数
///
bool FileExporter::ExportSampleInfo(ExpSampleInfoVector& vecSampleInfo, QString strFileName, tf::UserType::type UserType_)
{
    std::map<ExportType, std::set<ExportInfoEn>> savedata;
    if (!DictionaryQueryManager::GetExportConfig(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get export config.");
        return false;
    }

    auto setInfoIter = savedata.find(ExportType::IM_ALL_SAMPLE);
    if (setInfoIter == savedata.end())
    {
        ULOG(LOG_ERROR, "No't find ExportType::IM_ALL_SAMPLE info");
        return false;
    }

    // 获取要导出的设置信息
    std::set<ExportInfoEn> setInfo = setInfoIter->second;

    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    //strTitle1 = QObject::tr("数据浏览");
    strExportTextList.push_back(std::move(strTitle1));

    // 表头
    QString strHeard = "";
    for (auto iter : setInfo)
    {
        auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
        if (pSEInfo != nullptr && pSEInfo->m_UserType <= UserType_)
        {
            strHeard += pSEInfo->m_strName + "\t";
        }
    }
    strExportTextList.push_back(std::move(strHeard));

    // 内容
    for (SAMPLEEXPORTINFO& item : vecSampleInfo)
    {
        QString strItemInfo = "";
        for (auto iter : setInfo)
        {
            auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
            if (pSEInfo != nullptr && pSEInfo->m_UserType <= UserType_)
            {
                strItemInfo += item.*(pSEInfo->m_Field) + "\t";
            }
        }

        strExportTextList.push_back(std::move(strItemInfo));
    }
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 93);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 54, QPageSize::PageSizeId::A0);
    }

    return false;
}

///
///  @brief 导出数据浏览/历史数据为PDF格式
///
///
///  @param[in]   vecSampleInfo  试剂信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月30日，新建函数
///
bool FileExporter::ExportSampleInfoToPdf(ExpSampleInfoVector& vecSampleInfo, SampleExportModule& PDFInfo)
{
    std::map<ExportType, std::set<ExportInfoEn>> savedata;
    if (!DictionaryQueryManager::GetExportConfig(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get export config.");
        return false;
    }

    auto setInfoIter = savedata.find(ExportType::IM_ALL_SAMPLE);
    if (setInfoIter == savedata.end())
    {
        ULOG(LOG_ERROR, "No't find ExportType::IM_ALL_SAMPLE info");
        return false;
    }

    // 获取要导出的设置信息
    std::set<ExportInfoEn> setInfo = setInfoIter->second;
    SAMPLEEXPORTINFO TestInfo;
    SAMPLEEXPORTINFO ReTestInfo;
    bool bHasReTest = false;
    bool bUnit = false;
    for (SAMPLEEXPORTINFO& item : vecSampleInfo)
    {
        for (auto iter : setInfo)
        {
            switch (iter)
            {
                // 首次结果
            case EIE_IM_SAMPLE_NO:
            {
                TestInfo.strSampleNo = item.strSampleNo;
            }
                break;
            case EIE_IM_BARCODE:
            {
                TestInfo.strBarcode = item.strBarcode;
            }
                break;
            case EIE_IM_SAMPLE_SOURCE_TYPE:
            {
                TestInfo.strSampleSourceType = item.strSampleSourceType;
            }
                break;
            case EIE_IM_ITEM_NAME:
            {
                TestInfo.strItemName = item.strItemName;
            }
                break;
            case EIE_IM_TEST_RESULT:
            {
                TestInfo.strTestResult = item.strTestResult;
            }
                break;
            case EIE_IM_UNIT:
            {
                TestInfo.strUnit = item.strUnit;
                bUnit = true;
            }
                break;
            case EIE_IM_RUL:
            {
                TestInfo.strRUL = item.strRUL;
            }
                break;
            case EIE_IM_END_TEST_TIME:
            {
                TestInfo.strEndTestTime = item.strEndTestTime;
            }
                break;
            case EIE_IM_PRE_DILUTION_FACTOR:
            {
                TestInfo.strPreDilutionFactor = item.strPreDilutionFactor;
            }
                break;
            case EIE_IM_DILUTION_FACTOR:
            {
                TestInfo.strDilutionFactor = item.strDilutionFactor;
            }
                break;
            case EIE_IM_REAGENT_BATCHNO:
            {
                TestInfo.strReagentBatchNo = item.strReagentBatchNo;
            }
                break;
            case EIE_IM_SUBSTRATE_BATCH_NO:
            {
                TestInfo.strSubstrateBatchNo = item.strSubstrateBatchNo;
            }
                break;
            case EIE_IM_CLEAN_FLUID_BATCH_NO:
            {
                TestInfo.strCleanFluidBatchNo = item.strCleanFluidBatchNo;
            }
                break;
            case EIE_IM_ASSAY_CUP_BATCH_NO:
            {
                TestInfo.strAssayCupBatchNo = item.strAssayCupBatchNo;
            }
                break;
            case EIE_IM_DILUENT_BATCH_NO:
            {
                TestInfo.strDiluentBatchNo = item.strDiluentBatchNo;
            }
                break;
            case EIE_IM_RGT_REGISTER_T:
            {
                TestInfo.strRgtRegisterT = item.strRgtRegisterT;
            }
                break;
            case EIE_IM_REAGENT_EXP_TIME:
            {
                TestInfo.strReagentExpTime = item.strReagentExpTime;
            }
                break;
            case EIE_IM_CALI_LOT:
            {
                TestInfo.strCaliLot = item.strCaliLot;
            }
                break;
            case EIE_IM_FIRST_CUR_CALI_TIME:
            {
                TestInfo.strFirstCurCaliTime = item.strFirstCurCaliTime;
            }
                break;

                // 复查结果
            case EIE_IM_RE_TEST_RUL:
            {
                if (!item.strReTestRUL.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strRUL = item.strReTestRUL + "R";
                }
            }
                break;
            case EIE_IM_RE_TESTRESULT:
            {
                if (!item.strReTestResult.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strTestResult = item.strReTestResult + "R";
                    if (bUnit)
                    {
                        ReTestInfo.strUnit = item.strUnit;                      // 有结果且勾选单位时带上复查单位
                    }
                }
            }
                break;
            case EIE_IM_RE_PRE_DIL_FACTOR:
            {
                if (!item.strRePreDilFactor.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strPreDilutionFactor = item.strRePreDilFactor;
                }
            }
                break;
            case EIE_IM_RE_DIL_FACTOR:
            {
                if (!item.strReDilFactor.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strDilutionFactor = item.strReDilFactor;
                }
            }
                break;
            case EIE_IM_RE_END_TEST_TIME:
            {
                if (!item.strReEndTestTime.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strEndTestTime = item.strReEndTestTime;
                }
            }
                break;
            case EIE_IM_RE_REAGENT_BATCH_NO:
            {
                if (!item.strReReagentBatchNo.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strReagentBatchNo = item.strReReagentBatchNo;
                }
            }
                break;
            case EIE_IM_RE_SUB_BATCH_NO:
            {
                if (!item.strReSubBatchNo.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strSubstrateBatchNo = item.strReSubBatchNo;
                }
            }
                break;
            case EIE_IM_RE_CLEAN_FLUID_BATCH_NO:
            {
                if (!item.strReCleanFluidBatchNo.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strCleanFluidBatchNo = item.strReCleanFluidBatchNo;
                }
            }
                break;
            case EIE_IM_RE_ASSAY_CUP_BATCH_NO:
            {
                if (!item.strReAssayCupBatchNo.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strAssayCupBatchNo = item.strReAssayCupBatchNo;
                }
            }
                break;
            case EIE_IM_RE_DILUENT_BATCH_NO:
            {
                if (!item.strReDiluentBatchNo.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strDiluentBatchNo = item.strReDiluentBatchNo;
                }
            }
                break;
            case EIE_IM_RE_RGT_REGISTER_T:
            {
                if (!item.strReRgtRegisterT.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strRgtRegisterT = item.strReRgtRegisterT;
                }
            }
                break;
            case EIE_IM_RE_REAGENT_EXP_TIME:
            {
                if (!item.strReReagentExpTime.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strReagentExpTime = item.strReReagentExpTime;
                }
            }
                break;
            case EIE_IM_RE_CALI_LOT:
            {
                if (!item.strReCaliLot.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strCaliLot = item.strReCaliLot;
                }
            }
                break;
            case EIE_IM_RE_TEST_CUR_CALI_TIME:
            {
                if (!item.strReTestCurCaliTime.isEmpty())
                {
                    bHasReTest = true;
                    ReTestInfo.strFirstCurCaliTime = item.strReTestCurCaliTime;
                }
            }
                break;
            default:
                break;
            }
        }

        PDFInfo.vecResults.push_back(std::move(TestInfo));
        if (bHasReTest)
        {
            PDFInfo.vecResults.push_back(std::move(ReTestInfo)); // 复查结果另起一行
        }
    }
    
    return true;
}

///
///  @brief 导出试剂信息
///
///
///  @param[in]   vecLogDatas  试剂信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportReagentInfo(ReagentItemVector& vecLogDatas, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("试剂信息");
    strExportTextList.push_back(std::move(strTitle1));
    MakeExportList<ReagentItem>(vecLogDatas, strExportTextList);

    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 12, QPageSize::PageSizeId::A3);
    }

    return false;
}

///
///  @brief 导出耗材信息
///
///
///  @param[in]   vecLogDatas  耗材信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportSupplyInfo(SupplyItemVector& vecLogDatas, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("耗材信息");
    strExportTextList.push_back(std::move(strTitle1));
    MakeExportList<SupplyItem>(vecLogDatas, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 9, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出维护日志
///
///
///  @param[in]   vecLogDatas  维护日志数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportMaintainLog(MaintenanceLog& Log, QString strFileName)
{
    // 文件名为空直接返回
    if (strFileName.isEmpty())
    {
        ULOG(LOG_ERROR, "Export canceled!");
        return false;
    }

    // 记录导出文本
    QStringList strExportTextList;

    // 记录文本标题
    QString strTitle("");
    strTitle = strTitle + QObject::tr("维护日志") + "\t";
    strExportTextList.push_back(strTitle);
    std::vector<MaintenanceItem>& vecRecord = Log.vecRecord;
    MakeExportList<MaintenanceItem>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 7, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出报警信息
///
///
///  @param[in]   vecLogDatas  报警信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportAlarmInfo(AlarmInfoLog& Log, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("报警日志");
    strExportTextList.push_back(std::move(strTitle1));
    std::vector<AlarmInfo>& vecLogDatas = Log.vecRecord;
    MakeExportList<AlarmInfo>(vecLogDatas, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 6, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出操作日志
///
///
///  @param[in]   vecOperationLog  操作日志数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportOperationLog(OperationLog& Log, QString strFileName)
{
    OperationLog& OpLog = Log;

    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("操作日志");
    strExportTextList.push_back(std::move(strTitle1));

    // 空一行
    strExportTextList.push_back(QString(" "));
    MakeExportList<OperationLogItem>(OpLog.vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 5, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出耗材更换日志
///
///
///  @param[in]   vecReaChangeLog  耗材更换日志数据列表
///  @param[in]   strFileName  文件名
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportChangeLog(ReagentChangeLog& Log, QString strFileName)
{
    ReagentChangeLog& ChangeLog = Log;

    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("试剂/耗材更换日志");
    strExportTextList.push_back(std::move(strTitle1));

    // 空一行
    strExportTextList.push_back(QString(" "));
    MakeExportList<ChangeRecord>(ChangeLog.vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 9, QPageSize::PageSizeId::A4);
    }

    return false;
}

bool FileExporter::ExportCupHistoryInfo(CupHistoryInfo& info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("反应杯历史");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<CupHistoryItem>& vecRecord = info.vecRecord;
    MakeExportList<CupHistoryItem>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 5, QPageSize::PageSizeId::A4);
    }

    return false;
}

bool FileExporter::ExportLiquidInfo(LiquidInfo& info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("液路");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<LiquidItem>& vecRecord = info.vecRecord;
    MakeExportList<LiquidItem>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 5, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出温度信息
///
///
///  @param[in]   vecTemInfo  温度信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportTemperatureInfo(TemperatureLog& Log, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("温度");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<TemperatureInfo>& vecRecord = Log.vecRecord;
    MakeExportList<TemperatureInfo>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 5, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出计数信息
///
///
///  @param[in]   Info         计数信息
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportCountInfo(StatisticalInfo& Info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("计数");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<StatisticalUnit>& vecRecord = Info.vecRecord;
    MakeExportList<StatisticalUnit>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 8, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出下位机单元日志
///
///
///  @param[in]   Log          下位机单元信息
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportUnitLog(LowerCpLog& Log, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("下位机单元");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<LowerCpUnit>& vecRecord = Log.vecRecord;
    MakeExportList<LowerCpUnit>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 4, QPageSize::PageSizeId::A4);
    }

    return false;
}
///
///  @brief 导出样本统计信息
///
///
///  @param[in]   Info          样本统计信息
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportSampleStatisticsInfo(SampleStatisticsInfo& Info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    //QString strTitle1("");
    //strTitle1 = QObject::tr("样本量统计");
    //strExportTextList.push_back(std::move(strTitle1));

    // 第一行
    QString strFirstRow;
    strFirstRow += QObject::tr("日期") + "\t";
    strFirstRow += QObject::tr("模块") + "\t";
    strFirstRow += QObject::tr("试剂批号") + "\t";
    strExportTextList.push_back(std::move(strFirstRow));

    QString strFirstValue;
    strFirstValue += QString::fromStdString(Info.strDate) + "\t";
    strFirstValue += QString::fromStdString(Info.strModule) + "\t";
    strFirstValue += QString::fromStdString(Info.strReagentLot) + "\t";
    strExportTextList.push_back(std::move(strFirstValue));

    // 空一行
    strExportTextList.push_back(QString(" "));

    // 第二行
    QString strSecond;
    strSecond += QObject::tr(" ") + "\t";
    strSecond += QObject::tr("样本量") + "\t";
    strSecond += QObject::tr("常规") + "\t";
    strSecond += QObject::tr("急诊") + "\t";
    strSecond += QObject::tr("质控") + "\t";
    strSecond += QObject::tr("校准") + "\t";
    strExportTextList.push_back(std::move(strSecond));

    for (auto item : Info.vecRecordSum)
    {
        QString strSecondValue;
        strSecondValue += QString::fromStdString(item.strType) + "\t";
        strSecondValue += QString::fromStdString(item.strTotalOrder) + "\t";
        strSecondValue += QString::fromStdString(item.strConvSample) + "\t";
        strSecondValue += QString::fromStdString(item.strEmSample) + "\t";
        strSecondValue += QString::fromStdString(item.strQc) + "\t";
        strSecondValue += QString::fromStdString(item.strCali) + "\t";
        strExportTextList.push_back(std::move(strSecondValue));
    }
    

    // 表格内容
    std::vector<SampleStatistics>& vecRecord = Info.vecRecord;
    MakeExportList<SampleStatistics>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 8, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出试剂统计信息
///
///
///  @param[in]   Info          试剂统计信息
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportReagentStatisticsInfo(ReagentStatisticsInfo& Info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("试剂统计");
    strExportTextList.push_back(std::move(strTitle1));

    // 第一行
    QString strFirstRow;
    strFirstRow += QObject::tr("日期") + "\t";
    strFirstRow += QObject::tr("模块") + "\t";
    strFirstRow += QObject::tr("试剂名称") + "\t";
    strExportTextList.push_back(std::move(strFirstRow));

    QString strFirstValue;
    strFirstValue += QString::fromStdString(Info.strDate) + "\t";
    strFirstValue += QString::fromStdString(Info.strModule) + "\t";
    strFirstValue += QString::fromStdString(Info.strReagentName) + "\t";
    strExportTextList.push_back(std::move(strFirstValue));

    // 内容表头
    QString strHead;
    MakeExportList<ReagentStatistics>(Info.vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 13, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出校准统计信息
///
///
///  @param[in]   Info          校准统计信息
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportCaliStatisticsInfo(CaliStatisticsInfo& Info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("校准统计");
    strExportTextList.push_back(std::move(strTitle1));

    // 第一行
    QString strFirstRow;
    strFirstRow += QObject::tr("日期") + "\t";
    strFirstRow += QObject::tr("项目名称") + "\t";
    strFirstRow += QObject::tr("试剂批号") + "\t";
    strFirstRow += QObject::tr("模块") + "\t";
    strExportTextList.push_back(std::move(strFirstRow));

    QString strFirstValue;
    strFirstValue += QString::fromStdString(Info.strDate) + "\t";
    strFirstValue += QString::fromStdString(Info.strItemName) + "\t";
    strFirstValue += QString::fromStdString(Info.strReagentLot) + "\t";
    strFirstValue += QString::fromStdString(Info.strModule) + "\t";
    strExportTextList.push_back(std::move(strFirstValue));

    // 表格内容
    std::vector<CaliStatistics>& vecRecord = Info.vecRecord;
    MakeExportList<CaliStatistics>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 8, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出校准信息
///
///
///  @param[in]   strExportTextList  校准信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportCaliInfo(CaliResultInfo& CaliInfo, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;
    QString strTitle = QObject::tr("校准结果") + "\t";
    strExportTextList.push_back(strTitle);
    QString strHeard;
    strHeard += QObject::tr("项目名称") + "\t";
    strHeard += QObject::tr("仪器编号") + "\t";
    strHeard += QObject::tr("软件版本") + "\t";
    strHeard += QObject::tr("校准时间") + "\t";
    strHeard += QObject::tr("模块") + "\t";
    strExportTextList.push_back(strHeard);
    QString strHeardValue;
    strHeardValue += QString::fromStdString(CaliInfo.strName) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strDeviceNum) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strSoftVersion) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strCalibrateDate) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strModelName) + "\t";
    strExportTextList.push_back(strHeardValue);
    strExportTextList.push_back("  \t");            // 空一行

    QString strSupply;
    strSupply += QObject::tr("试剂批号") + "\t";
    strSupply += QObject::tr("试剂瓶号") + "\t";
    strSupply += QObject::tr("校准品批号") + "\t";
    strSupply += QObject::tr("校准品位置") + "\t";
    strSupply += QObject::tr("底物液批号") + "\t";
    strSupply += QObject::tr("底物液瓶号") + "\t";
    strSupply += QObject::tr("清洗缓冲液批号") + "\t";
    strSupply += QObject::tr("清洗缓冲液瓶号") + "\t";
    strSupply += QObject::tr("反应杯批号") + "\t";
    strSupply += QObject::tr("反应杯序列号") + "\t";
    strExportTextList.push_back(strSupply);
    QString strSupplyInfo;
    strSupplyInfo += QString::fromStdString(CaliInfo.strReagentLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strReagentSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCalibratorLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCalibratorPos) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strSubstrateLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strSubstrateSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCleanFluidLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCleanFluidSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCupLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCupSN) + "\t";
    strExportTextList.push_back(strSupplyInfo);
    strExportTextList.push_back("  \t");            // 空一行

    QString strCaliRLU;
    strCaliRLU += QObject::tr("校准品") + "\t";
    strCaliRLU += QObject::tr("浓度") + "\t";
    strCaliRLU += QObject::tr("RLU1") + "\t";
    strCaliRLU += QObject::tr("RLU2") + "\t";
    strCaliRLU += QObject::tr("RLU") + "\t";
    strCaliRLU += QObject::tr("CV") + "\t";
    strCaliRLU += QObject::tr("K1") + "\t";
    strCaliRLU += QObject::tr("K2") + "\t";
    strCaliRLU += QObject::tr("K") + "\t";
    strExportTextList.push_back(strCaliRLU);
    for (int i = 0; i < CaliInfo.vecCaliRLUInfo.size(); i++)
    {
        QString strCaliRLUValue;
        strCaliRLUValue += QString::number(i + 1) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strConc) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strRLU1) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strRLU2) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strRLU) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strCV) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strK1) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strK2) + "\t";
        strCaliRLUValue += QString::fromStdString(CaliInfo.vecCaliRLUInfo[i].strK) + "\t";
        strExportTextList.push_back(strCaliRLUValue);
    }

    strExportTextList.push_back("  \t");            // 空一行
    //QString strCurve;
    //strCurve = QObject::tr("拟合曲线") + "\t";
    //strExportTextList.push_back(strCurve);
    //int iCurRow = strExportTextList.size();         // 图片插入位置
    QString strLevel = QObject::tr("校准品水平") + "\t";
    QString strSign = QObject::tr("信号值") + "\t";
    QString strConc = QObject::tr("浓度") + "\t";
    for (int i = 0; i < CaliInfo.vecCaliResultData.size(); i++)
    {
        strLevel += QString::fromStdString(CaliInfo.vecCaliResultData[i].strLevel) + "\t";
        strSign += QString::fromStdString(CaliInfo.vecCaliResultData[i].strSign) + "\t";
        strConc += QString::fromStdString(CaliInfo.vecCaliResultData[i].strConc) + "\t";
    }

    strExportTextList.push_back(strLevel);
    strExportTextList.push_back(strConc);
    strExportTextList.push_back(strSign);

    strExportTextList.push_back("  \t");            // 空一行
    QString strCaliResult;
    strCaliResult += QObject::tr("校准结果") + "\t" + QString::fromStdString(CaliInfo.strCaliResult) + "\t";
    strCaliResult += QObject::tr("数据报警") + "\t" + QString::fromStdString(CaliInfo.strAlarm) + "\t";
    strCaliResult += QObject::tr("cutoff值") + "\t" + QString::fromStdString(CaliInfo.strCutoff) + "\t";
    strExportTextList.push_back(strCaliResult);

    //QImage Curve;
    //Curve.loadFromData(QByteArray::fromBase64(CaliInfo.strCaliCurveImage.c_str()), "PNG");

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        //QImage image = Curve.scaled(500, 240, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        return SaveToXlsx(strExportTextList,/* image, iCurRow, 3, */strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        //QImage image = Curve.scaled(500, 240, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        return SaveToPdf(strExportTextList,/* image, iCurRow - 2, 3, */strFileName, 11, QPageSize::PageSizeId::A3);
    }

    return false;
}

bool FileExporter::ExportCaliHistoryInfo(CaliHistoryInfo& CaliInfo, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;
    QString strTitle = QObject::tr("校准历史") + "\t";
    strExportTextList.push_back(strTitle);
    QString strHeard;
    strHeard += QObject::tr("项目名称") + "\t";
    strHeard += QObject::tr("校准时间") + "\t";
    strHeard += QObject::tr("模块") + "\t";
    strExportTextList.push_back(strHeard);

    QString strHeardValue;
    strHeardValue += QString::fromStdString(CaliInfo.strName) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strCalibrateDate) + "\t";
    strHeardValue += QString::fromStdString(CaliInfo.strModelName) + "\t";
    strExportTextList.push_back(strHeardValue);

    QString strSupply;
    strSupply += QObject::tr("校准品批号") + "\t";
    strSupply += QObject::tr("试剂批号") + "\t";
    strSupply += QObject::tr("试剂瓶号") + "\t";
    strSupply += QObject::tr("底物液批号") + "\t";
    strSupply += QObject::tr("底物液瓶号") + "\t";
    strSupply += QObject::tr("清洗缓冲液批号") + "\t";
    strSupply += QObject::tr("清洗缓冲液瓶号") + "\t";
    strSupply += QObject::tr("反应杯批号") + "\t";
    strSupply += QObject::tr("反应杯序列号") + "\t";
    strSupply += QObject::tr("试剂上机时间") + "\t";
    strExportTextList.push_back(strSupply);

    QString strSupplyInfo;
    strSupplyInfo += QString::fromStdString(CaliInfo.strCalibratorLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strReagentLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strReagentSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strSubstrateLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strSubstrateSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCleanFluidLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCleanFluidSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCupLot) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strCupSN) + "\t";
    strSupplyInfo += QString::fromStdString(CaliInfo.strRegisterDate) + "\t";
    strExportTextList.push_back(strSupplyInfo);
    strExportTextList.push_back("  \t");            // 空一行

    QString strCaliInfo;
    strCaliInfo = QObject::tr("校准品1") + "\t" + " " + "\t" + QObject::tr("校准品2") + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));
    strCaliInfo = QObject::tr("Cal-1浓度") + "\t" + QString::fromStdString(CaliInfo.strConc1) + "\t";
    strCaliInfo += QObject::tr("Cal-2浓度") + "\t" + QString::fromStdString(CaliInfo.strConc2) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));
    strCaliInfo = QObject::tr("Cal-1主标信号值") + "\t" + QString::fromStdString(CaliInfo.strSignalValue1) + "\t";
    strCaliInfo += QObject::tr("Cal-2主标信号值") + "\t" + QString::fromStdString(CaliInfo.strSignalValue2) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));
    strCaliInfo = QObject::tr("Cal-1 RLU1") + "\t" + QString::fromStdString(CaliInfo.strC1_RUL1) + "\t";
    strCaliInfo += QObject::tr("Cal-2 RLU1") + "\t" + QString::fromStdString(CaliInfo.strC2_RUL1) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));
    strCaliInfo = QObject::tr("Cal-1 RLU2") + "\t" + QString::fromStdString(CaliInfo.strC1_RUL2) + "\t";
    strCaliInfo += QObject::tr("Cal-2 RLU2") + "\t" + QString::fromStdString(CaliInfo.strC2_RUL2) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));
    strCaliInfo = QObject::tr("Cal-1 RLU") + "\t" + QString::fromStdString(CaliInfo.strC1_RUL) + "\t";
    strCaliInfo += QObject::tr("Cal-2 RLU") + "\t" + QString::fromStdString(CaliInfo.strC2_RUL) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));

    // 校准失败原因
    strCaliInfo = QString::fromStdString(CaliInfo.strFailureReason) + "\t";
    strExportTextList.push_back(std::move(strCaliInfo));

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 10, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出校准品记录
///
///
///  @param[in]   strExportTextList  校准信息数据列表
///  @param[in]   strFileName        文件名
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportCaliRecordInfo(CaliRecordInfo& RecordInfo, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("校准品记录");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<CaliRecord>& vecRecord = RecordInfo.vecRecord;
    MakeExportList<CaliRecord>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 10, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出校准架概况记录
///
///
///  @param[in]   RackInfo  校准信息数据列表
///  @param[in]   strFileName        文件名
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportCaliRackInfo(CaliRackInfo& RackInfo, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 记录导出标题
    QString strTitle1("");
    strTitle1 = QObject::tr("校准架概况");
    strExportTextList.push_back(std::move(strTitle1));

    std::vector<CaliRack>& vecRecord = RackInfo.vecCaliRack;
    MakeExportList<CaliRack>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 10, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出LJ质控信息
///
///
///  @param[in]   QcLjInfo  LJ质控信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportLJInfo(QCLJInfo& QcLjInfo, QString strFileName)
{
    QCLJInfo& Info = QcLjInfo;
    
    // 记录导出文本
    QStringList strExportTextList;

    // 标题
    QString strTitle = QObject::tr("项目质控信息");
    strExportTextList.push_back(std::move(strTitle));

    // 质控项目
    QString strHeard;
    strHeard += QObject::tr("项目名称") + "\t";
    strHeard += QObject::tr("质控日期") + "\t";
    //strHeard += QObject::tr("图表") + "\t";
    strHeard += QObject::tr("质控品编号") + "\t";
    strHeard += QObject::tr("质控品名称") + "\t";
    strHeard += QObject::tr("质控品简称") + "\t";
    strHeard += QObject::tr("质控品类型") + "\t";
    strHeard += QObject::tr("质控品水平") + "\t";
    strHeard += QObject::tr("质控品批号") + "\t";
    strHeard += QObject::tr("靶值") + "\t";
    strHeard += QObject::tr("SD") + "\t";
    strHeard += QObject::tr("CV%") + "\t";
    strHeard += QObject::tr("计算靶值") + "\t";
    strHeard += QObject::tr("计算SD") + "\t";
    strHeard += QObject::tr("计算CV%") + "\t";
    strHeard += QObject::tr("数量") + "\t";
    strHeard += QObject::tr("模块") + "\t";
    strExportTextList.push_back(strHeard);

    QString strItem("");
    strItem += QString::fromStdString(Info.strItemName) + "\t";
    strItem += QString::fromStdString(Info.strQCDate) + "\t";
    //strItem += QString::fromStdString(Info.strChart) + "\t";
    strItem += QString::fromStdString(Info.strQCID) + "\t";
    strItem += QString::fromStdString(Info.strQCName) + "\t";
    strItem += QString::fromStdString(Info.strQcBriefName) + "\t";
    strItem += QString::fromStdString(Info.strQcSourceType) + "\t";
    strItem += QString::fromStdString(Info.strQcLevel) + "\t";
    strItem += QString::fromStdString(Info.strQCLot) + "\t";
    strItem += QString::fromStdString(Info.strTargetMean) + "\t";
    strItem += QString::fromStdString(Info.strTargetSD) + "\t";
    strItem += QString::fromStdString(Info.strTargetCV) + "\t";
    strItem += QString::fromStdString(Info.strActualMean) + "\t";
    strItem += QString::fromStdString(Info.strActualSD) + "\t";
    strItem += QString::fromStdString(Info.strActualCV) + "\t";
    strItem += QString::fromStdString(Info.strQcRltCount) + "\t";
    strItem += QString::fromStdString(Info.strModelName) + "\t";
    strExportTextList.push_back(std::move(strItem));

    // 空一行
    strExportTextList.push_back(QString(" "));

    // 质控结果
    std::vector<QCResult>& vecRecord = Info.vecResult;
    MakeExportList<QCResult>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 17, QPageSize::PageSizeId::A3);
    }

    return false;
}

///
///  @brief 导出Twin Plot 图信息
///
///
///  @param[in]   QcYDInfo  Twin Plot 图信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportTpInfo(QCYDInfo& info, QString strFileName)
{
    QCYDInfo& Info = info;

    // 记录导出文本
    QStringList strExportTextList;

    // 标题
    QString strTitle = QObject::tr("项目质控信息");
    strExportTextList.push_back(std::move(strTitle));

    // 质控项目
    QString strItemHead("");	
    strItemHead += QObject::tr("项目名称") + "\t";
    strItemHead += QObject::tr("质控日期") + "\t";
    strItemHead += QObject::tr("模块") + "\t";
    strExportTextList.push_back(std::move(strItemHead));

    QString strItem;
    strItem += QString::fromStdString(Info.strItemName) + "\t";
    strItem += QString::fromStdString(Info.strTimeQuantum) + "\t";
    strItem += QString::fromStdString(Info.strModelName) + "\t";
    strExportTextList.push_back(std::move(strItem));

    // 空一行
    strExportTextList.push_back(QString(" "));

    // 质控结果
    std::vector<QCResult>& vecRecord = Info.vecResult;
    MakeExportList<QCResult>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 9, QPageSize::PageSizeId::A4);
    }

    return false;
}


///
///  @brief 导出质控品记录
///
///
///  @param[in]   QcYDInfo  Twin Plot 图信息数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::ExportQCItemRecord(QCItemRecord& info, QString strFileName)
{
    // 记录导出文本
    QStringList strExportTextList;

    // 标题
    QString strTitle = QObject::tr("质控品记录");
    strExportTextList.push_back(std::move(strTitle));

    // 质控结果
    std::vector<QCRecord>& vecRecord = info.vecRecord;
    MakeExportList<QCRecord>(vecRecord, strExportTextList);

    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为txt文本
    if (strSuffix == "txt")
    {
        return SaveToTxt(strExportTextList, strFileName);
    }

    // 导出为csv
    if (strSuffix == "csv")
    {
        return SaveToCsv(strExportTextList, strFileName);
    }

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsx(strExportTextList, strFileName);
    }

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
        return SaveToPdf(strExportTextList, strFileName, 10, QPageSize::PageSizeId::A4);
    }

    return false;
}

///
///  @brief 导出数据浏览/历史数据到文件
///
///
///  @param[in]   strExportTextList  数据浏览/历史数据数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool FileExporter::ExportInfoToFile(QStringList & strExportTextList, QString strFileName)
{
	// 判断是否导出为excel表，否则作为文本文件写入数据
	QFileInfo FileInfo(strFileName);
	QString strSuffix = FileInfo.suffix();

	// 导出为txt文本
	if (strSuffix == "txt")
	{
		return SaveToTxt(strExportTextList, strFileName);
	}

	// 导出为csv
	if (strSuffix == "csv")
	{
		return SaveToCsv(strExportTextList, strFileName);
	}

	// 导出为xlsx表格
	if (strSuffix == "xlsx")
	{
		return SaveToXlsx(strExportTextList, strFileName);
	}

	// 导出为pdf文件
	if (strSuffix == "pdf")
	{
		return SaveToPdf(strExportTextList, strFileName, 20, QPageSize::PageSizeId::A2);
	}

	return false;
}

bool FileExporter::ExportInfoToFileByClassify(QString strFileName, const QList<ExTaskType>& exTask, const ExpSampleInfoVector& pdfData)
{
    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();

    // 导出为xlsx表格
    if (strSuffix == "xlsx")
    {
        return SaveToXlsxByClassify(strFileName);
    }

	auto fjoin = [](const QVariantList& data, const QString flag)->QString
	{
		QStringList exportTmpList;
		for (auto& item  : data)
		{
			exportTmpList << item.toString();
		}

		return (exportTmpList.join(flag));
	};

	bool isSaveOk = false;
	for (const auto& task : exTask)
	{
		QStringList strExportTextList;
		strExportTextList.push_back(fjoin(task.title, ""));

		for (auto& classify : task.contents)
		{
			strExportTextList.push_back(fjoin(classify.toList(), ""));
		}

		if (strExportTextList.isEmpty())
		{
			continue;
		}

		// 导出为csv
		if (strSuffix == "csv")
		{
			QString tmpFileName = strFileName;
			if (tmpFileName.endsWith(strSuffix))
			{
				int postion = tmpFileName.indexOf(strSuffix);
				if (postion > 0)
				{
					tmpFileName.insert(postion -1, ("_" + task.sheetName));
				}
			}

			bool isSave =  SaveToCsv(strExportTextList, tmpFileName);
			if (!isSave)
			{
				continue;
			}
			else
			{
				isSaveOk = true;
			}
		}
	}

    //// 导出为txt文本
    //if (strSuffix == "txt")
    //{
    //    return SaveToTxt(strExportTextList, strFileName);
    //}

    // 导出为pdf文件
    if (strSuffix == "pdf")
    {
		SampleExportModule info;
		QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
		info.strExportTime = strPrintTime.toStdString();
		info.vecResults = pdfData;
		std::string strInfo = GetJsonString(info);
		ULOG(LOG_INFO, "Print datas : %s", strInfo);
		std::string unique_id;
		QString strDirPath = QCoreApplication::applicationDirPath();
		QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportSampleInfo.lrxml";
		int irect = printcom::printcom_async_assign_export(strInfo, strFileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
		ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
		isSaveOk = true;
    }

    return isSaveOk;
}

///
///  @brief 将数据写入到文件
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName  文件名
///
///  @return	true 写入成功 false 写入失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年5月22日，新建函数
///
bool FileExporter::SaveInfoToFile(QStringList& strExportTextList, QString strFileName)
{
    // 判断是否导出为excel表，否则作为文本文件写入数据
    QFileInfo FileInfo(strFileName);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "txt")
    {
        QFile eptFile(strFileName);
        if (eptFile.open(QFile::WriteOnly | QIODevice::Text))
        {
            QTextStream ts(&eptFile);
            ts.setCodec("utf-8");
            ts << GetAlignTableText(strExportTextList);
            eptFile.close();
            return true;
        }
        else
            return false;
    }
    else if (strSuffix == "csv")
    {
        QFile eptFile(strFileName);
        for (QString& strRowData : strExportTextList)
        {
            strRowData.replace('\t', ',');
        }

        if (eptFile.open(QFile::WriteOnly | QIODevice::Text))
        {
            QTextStream ts(&eptFile);
            ts << strExportTextList.join("\n");
            eptFile.close();
        }

        return true;
    }
    else if (strSuffix == "xlsx")
    {
        // 将导出内容写入文件中
        QXlsx::Document eptFile;

        // 设置样式
        QXlsx::Format textFormat;
        textFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
        QFont defFont = textFormat.font();
        defFont.setFamily(FONT_FAMILY);
        defFont.setPixelSize(FONT_SIZE_GENERAL);
        QFontMetricsF fontMtcs(defFont);
        textFormat.setFont(defFont);
        QScreen* pScreen = QApplication::screens().at(0);
        if (pScreen == Q_NULLPTR)
            return false;

        double dhDpi = pScreen->logicalDotsPerInchX();

        // 第一列是备注信息，第一行是表头，模板上是从第二行开始的
        int iRow = 1;
        for (const QString& strRowText : strExportTextList)
        {
            // 按列区分
            int iCol = 1;
            QStringList strColTextList = strRowText.split("\t");
            for (const QString& strColText : strColTextList)
            {
                double dTextWidth = fontMtcs.width(strColText) / dhDpi * UI_EXCEL_UNIT_NUM_PER_INCH_X;
                double dColWidth = eptFile.columnWidth(iCol);
                if (dTextWidth > dColWidth)
                    eptFile.setColumnWidth(iCol, dTextWidth);

                eptFile.write(iRow, iCol, strColText, textFormat);
                ++iCol;
            }
            ++iRow;
        }

        eptFile.saveAs(strFileName);
        return true;
    }
    else if (strSuffix == "pdf")
    {
        QTextDocument textdoc;
        textdoc.setDocumentMargin(1.0);
        QTextCursor Corsor(&textdoc);

        // 插入标题
        QTextBlockFormat titleBlockFormat;
        titleBlockFormat.setAlignment(Qt::AlignCenter);
        titleBlockFormat.setTopMargin(0);
        Corsor.insertText(strExportTextList.at(0));
        Corsor.insertBlock();
        strExportTextList.pop_front();

        // 统计列数
        QString strFirstText = strExportTextList.constFirst();
        QStringList strFirstList = strFirstText.split("\t");
        QString strLastText = strExportTextList.constLast();
        QStringList strLastTextList = strLastText.split("\t");
        int iColumn = std::max(strLastTextList.count(), strFirstList.count());

        // 插入内容
        QTextDocument Tabledoc;
        Tabledoc.setDocumentMargin(1.0);
        QTextCursor TableCorsor(&Tabledoc);
        QTextTable *pTable = TableCorsor.insertTable(strExportTextList.count(), iColumn);
        QTextTableFormat tableFormat;
        tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        tableFormat.setBorder(0.0);
        tableFormat.setCellSpacing(10);
        tableFormat.setCellPadding(1);
        QVector<QTextLength> cellWidths;
        for (int i = 0; i < iColumn; i++)
        {
            cellWidths << QTextLength(QTextLength::VariableLength, 5.0);

        }

        tableFormat.setColumnWidthConstraints(cellWidths);
        pTable->setFormat(tableFormat);
        for (int i = 0; i < strExportTextList.count(); i++)
        {
            QString strRowText = strExportTextList.at(i);
            QStringList strColTextList = strRowText.split("\t");
            for (int j = 0; j < strColTextList.count() && j < iColumn; j++)
            {
                QTextTableCell cell = pTable->cellAt(i, j);
                QTextCursor cellCursor = cell.firstCursorPosition();
                cellCursor.insertText(strColTextList.at(j));
            }

            //if (i % 10 == 0)
            //{
            //    int iPoss = 45 * i / strExportTextList.count();
            //    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 50 + iPoss);
            //}
        }

        Corsor.insertFragment(QTextDocumentFragment(&Tabledoc));

        QPdfWriter pdfw(strFileName);
        QPageSize ps(QPageSize::A4);
        pdfw.setPageSize(ps);
        pdfw.setPageMargins(QMarginsF(1.0, 1.0, 1.0, 1.0));
        textdoc.print(&pdfw);
        //POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 100);
        return true;
    }
    else
    {
        return false;
    }
}

///
///  @brief 导出为txt文本
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName        文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool FileExporter::SaveToTxt(QStringList & strExportTextList, QString strFileName)
{
	QFile eptFile(strFileName);
	if (eptFile.open(QFile::WriteOnly | QIODevice::Text))
	{
		QTextStream ts(&eptFile);
		ts.setCodec("utf-8");
		ts << GetAlignTableText(strExportTextList);
		eptFile.close();
		return true;
	}

	return false;
}

///
///  @brief 导出为csv
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName        文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool FileExporter::SaveToCsv(QStringList & strExportTextList, QString strFileName)
{
	QFile eptFile(strFileName);
	for (QString& strRowData : strExportTextList)
	{
		strRowData.replace('\t', ',');
	}

	if (eptFile.open(QFile::WriteOnly | QIODevice::Text))
	{
		QTextStream ts(&eptFile);
		ts << strExportTextList.join("\n");
		eptFile.close();
		return true;
	}

    // 刷新进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 99);
	return false;
}

///
///  @brief 导出为xlsx表格
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName        文件名
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool FileExporter::SaveToXlsx(QStringList & strExportTextList, QString strFileName)
{
	// 将导出内容写入文件中
	QXlsx::Document eptFile;

	// 设置样式
	QXlsx::Format textFormat;
	textFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
	QFont defFont = textFormat.font();
	defFont.setFamily(FONT_FAMILY);
	defFont.setPixelSize(FONT_SIZE_GENERAL);
	QFontMetricsF fontMtcs(defFont);
	textFormat.setFont(defFont);
	QScreen* pScreen = QApplication::screens().at(0);
	if (pScreen == Q_NULLPTR)
		return false;

	double dhDpi = pScreen->logicalDotsPerInchX();

	// 第一列是备注信息，第一行是表头，模板上是从第二行开始的
	int iRow = 1;
	for (const QString& strRowText : strExportTextList)
	{
		// 按列区分
		int iCol = 1;
		QStringList strColTextList = strRowText.split("\t");
		for (const QString& strColText : strColTextList)
		{
			double dTextWidth = fontMtcs.width(strColText) / dhDpi * UI_EXCEL_UNIT_NUM_PER_INCH_X;
			double dColWidth = eptFile.columnWidth(iCol);
			if (dTextWidth > dColWidth)
				eptFile.setColumnWidth(iCol, dTextWidth);

			eptFile.write(iRow, iCol, strColText, textFormat);
			++iCol;
		}
		++iRow;
	}

    // 刷新进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 99);
	return eptFile.saveAs(strFileName);
}

bool FileExporter::SaveToXlsxByClassify(QString strFileName)
{
    if (m_dataExTask.empty())
        return false;

    // 将导出内容写入文件中
    QXlsx::Document eptFile;

    // 设置样式
    QXlsx::Format textFormat;
    textFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    QFont defFont = textFormat.font();
    defFont.setFamily(FONT_FAMILY);
    defFont.setPixelSize(FONT_SIZE_GENERAL);
    QFontMetricsF fontMtcs(defFont);
    textFormat.setFont(defFont);
    QScreen* pScreen = QApplication::screens().at(0);
    if (pScreen == Q_NULLPTR)
        return false;

    double dhDpi = pScreen->logicalDotsPerInchX();

    for (auto task : m_dataExTask)
    {
        // 第一列是备注信息，第一行是表头，模板上是从第二行开始的
        int iRow = 1;
        // 获取有多少列数据
        int colCnt = 1;
        // 增加sheet页
        eptFile.addSheet(task.sheetName);

        // 输出表头
        for (const auto& ti : task.title)
        {
            // 按列区分
            double dTextWidth = fontMtcs.width(ti.toString()) / dhDpi * UI_EXCEL_UNIT_NUM_PER_INCH_X;
            double dColWidth = eptFile.columnWidth(colCnt);
            if (dTextWidth > dColWidth)
                eptFile.setColumnWidth(colCnt, dTextWidth);

            eptFile.write(iRow, colCnt, ti, textFormat);
            ++colCnt;
        }
        ++iRow;

        int iCol = 1;
        for (const auto& dataRow : task.contents)
        {
			if (!dataRow.canConvert<QVariantList>())
			{
				continue;
			}

			iCol = 1;
			for (const auto& data : dataRow.toList())
			{
				// 按列区分
				double dTextWidth = fontMtcs.width(data.toString()) / dhDpi * UI_EXCEL_UNIT_NUM_PER_INCH_X;
				double dColWidth = eptFile.columnWidth(iCol);
				if (dTextWidth > dColWidth)
				{
					eptFile.setColumnWidth(iCol, dTextWidth);
				}
				eptFile.write(iRow, iCol, data, textFormat);

				++iCol;
				if (iCol == colCnt)
				{
					break;
				}
			}
			++iRow;
        }
    }

	// 刷新进度条
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 99);
    eptFile.saveAs(strFileName);

    return true;
}

///
///  @brief 导出为xlsx表格
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName        文件名
///  @param[in]   Image              图片
///  @param[in]   iX.iY               插入位置
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
bool FileExporter::SaveToXlsx(QStringList& strExportTextList, QImage& Image, int iX, int iY, QString strFileName)
{
    // 将导出内容写入文件中
    QXlsx::Document eptFile;

    // 设置样式
    QXlsx::Format textFormat;
    textFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    QFont defFont = textFormat.font();
    defFont.setFamily(FONT_FAMILY);
    defFont.setPixelSize(FONT_SIZE_GENERAL);
    QFontMetricsF fontMtcs(defFont);
    textFormat.setFont(defFont);
    QScreen* pScreen = QApplication::screens().at(0);
    if (pScreen == Q_NULLPTR)
        return false;

    double dhDpi = pScreen->logicalDotsPerInchX();
    
    // 第一列是备注信息，第一行是表头，模板上是从第二行开始的
    int iRow = 1;
    for (const QString& strRowText : strExportTextList)
    {
        // 按列区分
        int iCol = 1;
        QStringList strColTextList = strRowText.split("\t");
        for (const QString& strColText : strColTextList)
        {
            double dTextWidth = fontMtcs.width(strColText) / dhDpi * UI_EXCEL_UNIT_NUM_PER_INCH_X;
            double dColWidth = eptFile.columnWidth(iCol);
            if (dTextWidth > dColWidth)
                eptFile.setColumnWidth(iCol, dTextWidth);

            eptFile.write(iRow, iCol, strColText, textFormat);
            ++iCol;
        }
        ++iRow;
    }

    eptFile.insertImage(iX, iY, Image);

    eptFile.saveAs(strFileName);

    return true;
}

///
///  @brief 导出为pdf文件
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   strFileName        文件名
///  @param[in]   iColumn			 文本列数
///  @param[in]   pageSize           纸张大小（A0、A1、A3、A4 ...）
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool FileExporter::SaveToPdf(QStringList & strExportTextList, QString strFileName, int iColumn, QPageSize::PageSizeId pageSize)
{
	QTextDocument textdoc;
	textdoc.setDocumentMargin(1.0);
	QTextCursor Corsor(&textdoc);

	// 插入标题
	QTextBlockFormat titleBlockFormat;
	titleBlockFormat.setAlignment(Qt::AlignCenter);
	titleBlockFormat.setTopMargin(0);
	Corsor.insertText(strExportTextList.at(0));
	Corsor.insertBlock();
	strExportTextList.pop_front();

	// 插入内容
	QTextDocument Tabledoc;
	Tabledoc.setDocumentMargin(1.0);
	QTextCursor TableCorsor(&Tabledoc);
	QTextTable *pTable = TableCorsor.insertTable(strExportTextList.count(), iColumn);
	QTextTableFormat tableFormat;
	tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
	tableFormat.setBorder(0.0);
	tableFormat.setCellSpacing(10);
	tableFormat.setCellPadding(1);
	QVector<QTextLength> cellWidths;
	for (int i = 0; i < iColumn; i++)
	{
		cellWidths << QTextLength(QTextLength::VariableLength, 5.0);

	}

	tableFormat.setColumnWidthConstraints(cellWidths);
	pTable->setFormat(tableFormat);
	for (int i = 0; i < strExportTextList.count(); i++)
	{
		QString strRowText = strExportTextList.at(i);
		QStringList strColTextList = strRowText.split("\t");
		for (int j = 0; j < strColTextList.count() && j < iColumn; j++)
		{
			QTextTableCell cell = pTable->cellAt(i, j);
			QTextCursor cellCursor = cell.firstCursorPosition();
			cellCursor.insertText(strColTextList.at(j));
		}
	}

	Corsor.insertFragment(QTextDocumentFragment(&Tabledoc));

	QPdfWriter pdfw(strFileName);
	QPageSize ps(pageSize);
	pdfw.setPageSize(ps);
	pdfw.setPageMargins(QMarginsF(1.0, 1.0, 1.0, 1.0));
	textdoc.print(&pdfw);
	return true;
}

///
///  @brief 导出为pdf文件
///
///
///  @param[in]   strExportTextList  数据列表
///  @param[in]   Image              图片
///  @param[in]   iX.iY              图片插入位置
///  @param[in]   strFileName        文件名
///  @param[in]   iColumn			 文本列数
///  @param[in]   pageSize           纸张大小（A0、A1、A3、A4 ...）
///
///  @return	true 导出成功 false 导出失败
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
bool FileExporter::SaveToPdf(QStringList& strExportTextList, QImage& Image, int iX, int iY, QString strFileName, int iColumn, QPageSize::PageSizeId pageSize)
{
    QTextDocument textdoc;
    textdoc.setDocumentMargin(1.0);
    QTextCursor Corsor(&textdoc);

    // 插入标题
    QTextBlockFormat titleBlockFormat;
    titleBlockFormat.setAlignment(Qt::AlignCenter);
    titleBlockFormat.setTopMargin(0);
    Corsor.insertText(strExportTextList.at(0));
    Corsor.insertBlock();
    strExportTextList.pop_front();

    // 统计列数
    int iImageInsertRow = (iX <= strExportTextList.count() ? iX : strExportTextList.count());

    // 插入内容
    QTextDocument Tabledoc;
    Tabledoc.setDocumentMargin(1.0);
    QTextCursor TableCorsor(&Tabledoc);
    TableCorsor.insertBlock();
    TableCorsor.beginEditBlock();
    QTextTable *pFirstTable = TableCorsor.insertTable(iImageInsertRow, iColumn);
    if (pFirstTable == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "pFirstTable is Q_NULLPTR!");
        return false;
    }

    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableFormat.setBorder(0.0);
    tableFormat.setCellSpacing(10);
    tableFormat.setCellPadding(1);
    QVector<QTextLength> cellWidths;
    for (int i = 0; i < iColumn; i++)
    {
        cellWidths << QTextLength(QTextLength::VariableLength, 5.0);

    }

    tableFormat.setColumnWidthConstraints(cellWidths);
    pFirstTable->setFormat(tableFormat);
    for (int i = 0; i < iImageInsertRow; i++)
    {
        QString strRowText = strExportTextList.at(i);
        QStringList strColTextList = strRowText.split("\t");
        for (int j = 0; j < strColTextList.count() && j < iColumn; j++)
        {
            QTextTableCell cell = pFirstTable->cellAt(i, j);
            QTextCursor cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(strColTextList.at(j));
        }

        // 光标跟随移动到下一行开始
        TableCorsor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, iColumn);
    }

    TableCorsor.endEditBlock();
    TableCorsor.joinPreviousEditBlock();
    QString strImageName("PdfExportTemporary.png");
    Image.save(strImageName);
    QTextImageFormat ImFormat;
    ImFormat.setName(strImageName);
    if (iImageInsertRow < strExportTextList.count())
    {
        QTextTable *pSecondTable = TableCorsor.insertTable(strExportTextList.count() - iImageInsertRow, iColumn);
        if (pSecondTable == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "pSecondTable is Q_NULLPTR!");
            return false;
        }

        pSecondTable->setFormat(tableFormat);
        for (int i = 0; i < strExportTextList.count() - iImageInsertRow; i++)
        {
            QString strRowText = strExportTextList.at(i + iImageInsertRow);
            QStringList strColTextList = strRowText.split("\t");
            for (int j = 0; j < strColTextList.count() && j < iColumn; j++)
            {
                QTextTableCell cell = pSecondTable->cellAt(i, j);
                QTextCursor cellCursor = cell.firstCursorPosition();
                cellCursor.insertText(strColTextList.at(j));
            }
        }

        QTextTableCell cell = pSecondTable->cellAt(0, iY);
        QTextCursor cellCursor = cell.firstCursorPosition();
        cellCursor.insertImage(ImFormat);
        pSecondTable->mergeCells(0, iY, strExportTextList.count() - iImageInsertRow, iColumn - iY);
    }
    else
    {
        TableCorsor.insertImage(ImFormat);
    }

    Corsor.insertFragment(QTextDocumentFragment(&Tabledoc));

    QPdfWriter pdfw(strFileName);
    QPageSize ps(pageSize);
    pdfw.setPageSize(ps);
    pdfw.setPageMargins(QMarginsF(1.0, 1.0, 1.0, 1.0));
    textdoc.print(&pdfw);
    QFile::remove(strImageName);
    return true;
}

const QString FileExporter::GetFileName(bool isCh, const QString& filePath)
{
	QFileInfo FileInfo(filePath);
	QString strSuffix = FileInfo.suffix();
	QString path = FileInfo.absolutePath() + "/";
	QString suffix;

	if (isCh)
	{
		suffix = "_CH.";
	}
	else
	{
		suffix = "_IM.";
	}

	QString fileName = path + FileInfo.completeBaseName() + suffix + strSuffix;
	return fileName;
}

///
/// @brief
///     获取对齐的表格文本
///
/// @param[in]  notAlignTblText  未对齐的表格文本
///
/// @return 对齐的表格文本
///
/// @par History:
/// @li 4170/TangChuXian，2022年4月2日，新建函数
///
const QString FileExporter::GetAlignTableText(const QString& notAlignTblText)
{
    // 记录每列最长的字符串长度
    QMap<int, int> mapColSTrMaxLen;

    // 获取行文本
    QStringList strExportTextList = notAlignTblText.split("\n");

    // 第一列是备注信息，第一行是表头，模板上是从第二行开始的
    int iRow = 1;
    for (const QString& strRowText : strExportTextList)
    {
        // 按列区分
        int iCol = 1;
        QStringList strColTextList = strRowText.split("\t");
        for (const QString& strColText : strColTextList)
        {
            int iColSTrMaxLen = 0;
            auto it = mapColSTrMaxLen.find(iCol);
            if (it != mapColSTrMaxLen.end())
            {
                iColSTrMaxLen = it.value();
            }

            int iCharSize = strColText.toLocal8Bit().size();
            if (iCharSize > iColSTrMaxLen)
            {
                iColSTrMaxLen = iCharSize;
            }

            mapColSTrMaxLen.insert(iCol, iColSTrMaxLen);

            // 列号自增
            ++iCol;
        }

        // 行号自增
        ++iRow;
    }

    // 对齐文本
    iRow = 1;
    for (QString& strRowText : strExportTextList)
    {
        // 按列区分
        int iCol = 1;

        QStringList strColTextList = strRowText.split("\t");
        for (QString& strColText : strColTextList)
        {
            int iColSTrMaxLen = 0;
            auto it = mapColSTrMaxLen.find(iCol);
            if (it == mapColSTrMaxLen.end())
            {
                return notAlignTblText;
            }

            iColSTrMaxLen = it.value();
            int nT = (iColSTrMaxLen - 1) / 8 + 1;

            int iCharSize = strColText.toLocal8Bit().size();
            if ((iCharSize - 1) / 8 + 1 < nT)
            {
                int subNT = nT - ((iCharSize - 1) / 8 + 1);
                strColText.append(QString(subNT * 8, ' '));
            }

            // 列号自增
            ++iCol;
        }

        // 改变行文本
        strRowText = strColTextList.join("\t");

        // 行号自增
        ++iRow;
    }

    return strExportTextList.join("\n");
}

///
/// @brief
///     对齐表格文本
///
/// @param[in]  notAlignTblText  未对齐的表格文本
///
/// @return 对齐的表格文本
///
/// @par History:
/// @li 4170/TangChuXian，2022年4月2日，新建函数
///
const QString FileExporter::GetAlignTableText(QStringList& notAlignTblText)
{
    // 记录每列最长的字符串长度
    QMap<int, int> mapColSTrMaxLen;

    // 获取行文本
    QStringList& strExportTextList = notAlignTblText;

    // 第一列是备注信息，第一行是表头，模板上是从第二行开始的
    int iRow = 1;
    for (const QString& strRowText : strExportTextList)
    {
        // 按列区分
        int iCol = 1;

        QStringList strColTextList = strRowText.split("\t");
        for (const QString& strColText : strColTextList)
        {
            int iColSTrMaxLen = 0;
            auto it = mapColSTrMaxLen.find(iCol);
            if (it != mapColSTrMaxLen.end())
            {
                iColSTrMaxLen = it.value();
            }

            int iCharSize = strColText.toLocal8Bit().size();
            if (iCharSize > iColSTrMaxLen)
            {
                iColSTrMaxLen = iCharSize;
            }

            mapColSTrMaxLen.insert(iCol, iColSTrMaxLen);

            // 列号自增
            ++iCol;
        }

        // 行号自增
        ++iRow;
    }

    // 对齐文本
    iRow = 1;
    for (QString& strRowText : strExportTextList)
    {
        // 按列区分
        int iCol = 1;

        QStringList strColTextList = strRowText.split("\t");
        for (QString& strColText : strColTextList)
        {
            int iColSTrMaxLen = 0;
            auto it = mapColSTrMaxLen.find(iCol);
            if (it == mapColSTrMaxLen.end())
            {
                return notAlignTblText.join("\n");
            }

            iColSTrMaxLen = it.value();
            int nT = iColSTrMaxLen / 8 + 1;

            int iCharSize = strColText.toLocal8Bit().size();
            if (iCharSize / 8 + 1 < nT)
            {
                int subNT = nT - (iCharSize / 8 + 1);
                strColText.append(QString(subNT * 8, ' '));
            }

            // 列号自增
            ++iCol;
        }

        // 改变行文本
        strRowText = strColTextList.join("\t");

        // 行号自增
        ++iRow;
    }

    return strExportTextList.join("\n");
}

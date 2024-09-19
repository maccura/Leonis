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
/// @file     uicommon.cpp
/// @brief    ui通用接口
///
/// @author   4170/TangChuXian
/// @date     2020年5月21日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月21日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "uicommon.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_types.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/StringUtil.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ise/IseConfigDefine.h"
#include "uidcsadapter/adaptertypedef.h"

#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/datetimefmttool.h"
#include "shared/QFilterComboBox.h"
#include "shared/qcustomcalendarwidget.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QListView>
#include <QRadioButton>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QTableView>
#include <QFontMetrics>
#include <QPushButton>
#include <algorithm>

#define QC_QUERY_MAX_INTERVAL_DAYS                             59                   // 质控查询起止日期最大间隔（59天)


//////////////////////////////////////////////////////////////////////////
/// 自定义下拉框枚举值
//////////////////////////////////////////////////////////////////////////
// 样本量/稀释
enum SUCK_VOL_OR_DILLUTE
{
    SUCK_VOL_STD = 0,                                               // 常量
    SUCK_VOL_INC,                                                   // 增量
    SUCK_VOL_DEC,                                                   // 减量
    DILLUTE,                                                        // 稀释
    PRE_DILLUTE                                                     // 预稀释
};

// 稀释倍数
enum DILLUTE_FACTOR
{
    DILLUTE_FACTOR_3,                                               // 稀释3倍
    DILLUTE_FACTOR_5,                                               // 稀释5倍
    DILLUTE_FACTOR_10,                                              // 稀释10倍
    DILLUTE_FACTOR_20,                                              // 稀释20倍
    DILLUTE_FACTOR_50                                               // 稀释50倍
};

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

UiCommon::UiCommon(QObject *parent)
    : QObject(parent),
      m_cStrEmpty("")
{
    // 初始化thrift枚举名，下拉框文本的映射
    InitEnNameMap();

    // 初始化字典值，thrift枚举名的映射
    InitDiValMap();

    // 初始化当前日期更新监听器
    InitCurDateUpdateMonitor();

    // 安装事件过滤器
    qApp->installEventFilter(this);

    // 通过信号触发列宽调整AddAvgColWidth
    connect(this, SIGNAL(SigTblAddColWidth(QTableView*, int)), this, SLOT(AddAvgColWidth(QTableView*, int)), Qt::QueuedConnection);

    // 监听系统时间手动更新
    REGISTER_HANDLER(MSG_ID_SYSTEM_DATE_MANUAL_CHANGED, this, OnNewDayArrived);
}

///
/// @brief
///     初始化thrift枚举名映射
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月21日，新建函数
///
void UiCommon::InitEnNameMap()
{
	// 单机与联机特殊处理项(可根据该标识动态设置)
	bool isPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();

    // 设备状态
    m_enNameMap.insert("DEVICE_STATUS_DISCONNECT", tr("--"));
    m_enNameMap.insert("DEVICE_STATUS_MAINTAIN", tr("维护"));
    // 免疫预热显示为维护
    m_enNameMap.insert("DEVICE_STATUS_WARMUP", tr("维护"));

    m_enNameMap.insert("DEVICE_STATUS_STANDBY", tr("待机"));
    m_enNameMap.insert("DEVICE_STATUS_RUNNING", tr("运行"));
    m_enNameMap.insert("DEVICE_STATUS_HALT", tr("停止"));
    m_enNameMap.insert("DEVICE_STATUS_SAMPSTOP", tr("暂停"));
    m_enNameMap.insert("DEVICE_STATUS_SAMPSTOP_FAULT", tr("暂停"));
    m_enNameMap.insert("DEVICE_STATUS_STOP_PUSH_RACK", tr("暂停"));
    m_enNameMap.insert("DEVICE_STATUS_WAIT_OPEN_LID", tr("申请中"));
    m_enNameMap.insert("DEVICE_STATUS_ALLOW_OPEN_LID", tr("申请成功"));

    // 样本状态
    m_enNameMap.insert("SAMPLE_STATUS_PENDING", "Pending");
    m_enNameMap.insert("SAMPLE_STATUS_TESTED", "Tested");
    m_enNameMap.insert("SAMPLE_STATUS_TESTING", "Testing");
    m_enNameMap.insert("SAMPLE_STATUS_TESTED_WITH_ALARM", "Alarm");

	m_enNameMap.insert("SAMPLE_SOURCE_TYPE_OTHER", tr("其他"));
	m_enNameMap.insert("SAMPLE_SOURCE_TYPE_XQXJ", tr("血清血浆"));
	m_enNameMap.insert("SAMPLE_SOURCE_TYPE_QX", tr("全血"));
    m_enNameMap.insert("SAMPLE_SOURCE_TYPE_NY", tr("尿液"));
	m_enNameMap.insert("SAMPLE_SOURCE_TYPE_NJY", tr("脑脊液"));
	m_enNameMap.insert("SAMPLE_SOURCE_TYPE_JMQJY", tr("浆膜腔积液"));

    // 质控原因
    m_enNameMap.insert("QC_REASON_MANNUAL", tr("手动选择"));
    m_enNameMap.insert("QC_REASON_DEFAULT", tr("默认质控"));
	m_enNameMap.insert("QC_REASON_OVERTIME", tr("自动质控"));
	m_enNameMap.insert("QC_REASON_ON_TIMER", tr("自动质控"));
	m_enNameMap.insert("QC_REASON_OUT_SAMPLE_COUNT", tr("自动质控"));
	m_enNameMap.insert("QC_REASON_AFTER_CALI", tr("自动质控"));

    // 样本源类型
    std::map<std::string, std::string> souceTypeMap = DictionaryQueryManager::GetInstance()->GetMapSouceType();
    std::map<std::string, std::string>::iterator iter;
    for (iter = souceTypeMap.begin(); iter != souceTypeMap.end(); iter++)
    {
        m_enNameMap.insert(("souce_" + iter->first), tr(QString::fromStdString(iter->second).toStdString().c_str()));
    }

    // 单位类型
    std::map<std::string, std::string> unitTypeMap = DictionaryQueryManager::GetInstance()->GetMapUnitType();
    for (iter = unitTypeMap.begin(); iter != unitTypeMap.end(); iter++)
    {
        m_enNameMap.insert(("unit_" + iter->first), tr(QString::fromStdString(iter->second).toStdString().c_str()));
    }

    // 样本类型
    m_enNameMap.insert("SAMPLE_TYPE_SAMPLE", tr("Routine"));
    m_enNameMap.insert("SAMPLE_TYPE_CALIBRATOR", tr("Calibrator"));
    m_enNameMap.insert("SAMPLE_TYPE_QC", tr("QC"));

    // 患者类型
    m_enNameMap.insert("PATIENT_TYPE_OUT_PATIENT", tr("Outpatient"));
    m_enNameMap.insert("PATIENT_TYPE_IN_HOSPITAL", tr("In hospital"));
    m_enNameMap.insert("PATIENT_TYPE_STAT", tr("Stat"));

    // 性别
    m_enNameMap.insert("GENDER_MALE", tr("男"));
    m_enNameMap.insert("GENDER_FEMALE", tr("女"));
    m_enNameMap.insert("GENDER_UNKNOWN", tr(" "));

    // 年龄单位
    m_enNameMap.insert("AGE_UNIT_UNKNOWN", tr(""));
    m_enNameMap.insert("AGE_UNIT_YEAR", tr("岁"));
    m_enNameMap.insert("AGE_UNIT_MONTH", tr("月"));
    m_enNameMap.insert("AGE_UNIT_DAY", tr("天"));
    m_enNameMap.insert("AGE_UNIT_HOUR", tr("小时"));

    // 样本杯
	m_enNameMap.insert("TUBE_TYPE_INVALID", tr("无样本杯"));
    m_enNameMap.insert("TUBE_TYPE_NORMAL", tr("标准样本杯"));
	m_enNameMap.insert("TUBE_TYPE_MICRO", tr("微量样本杯"));
	m_enNameMap.insert("TUBE_TYPE_ORIGIN", tr("常规样本管"));
	m_enNameMap.insert("TUBE_TYPE_STORE", tr("样本贮存管"));

    // 样本量/稀释
    m_enNameMap.insert("SUCK_VOL_STD", tr("Standard"));
    m_enNameMap.insert("SUCK_VOL_INC", tr("Increase"));
    m_enNameMap.insert("SUCK_VOL_DEC", tr("Decrease"));
    m_enNameMap.insert("DILLUTE", tr("Dillute"));
    m_enNameMap.insert("PRE_DILLUTE", tr("Pre-dillute"));

    // 稀释倍数
    m_enNameMap.insert("DILLUTE_FACTOR_3", "1:3");
    m_enNameMap.insert("DILLUTE_FACTOR_5", "1:5");
    m_enNameMap.insert("DILLUTE_FACTOR_10", "1:10");
    m_enNameMap.insert("DILLUTE_FACTOR_20", "1:20");
    m_enNameMap.insert("DILLUTE_FACTOR_50", "1:50");

    // 质控状态
    m_enNameMap.insert("QC_STATE_NONE", "");
    m_enNameMap.insert("QC_STATE_IN_CTRL", tr("在控"));
    m_enNameMap.insert("QC_STATE_WARNING", tr("警告"));
    m_enNameMap.insert("QC_STATE_OUT_OF_CTRL", tr("失控"));

    // 结果状态
    m_enNameMap.insert("TEST_ITEM_STATUS_PENDING", tr("Pending"));
    m_enNameMap.insert("TEST_ITEM_STATUS_TESTING", tr("Testing"));
    m_enNameMap.insert("TEST_ITEM_STATUS_TESTED", tr("Tested"));

    // 试剂阶段
    m_enNameMap.insert("REAGENT_STAGE_R1", "R1");
    m_enNameMap.insert("REAGENT_STAGE_R2", "R2");
    m_enNameMap.insert("REAGENT_STAGE_R3", "R3");
    m_enNameMap.insert("REAGENT_STAGE_R4", "R4");

    // 校准状态
    m_enNameMap.insert("CALI_STATUS_NOT", tr("not"));
	m_enNameMap.insert("CALI_STATUS_DOING", tr("caling"));
    m_enNameMap.insert("CALI_STATUS_FAIL", tr("failed"));
    m_enNameMap.insert("CALI_STATUS_EXPIRE", tr("expired"));
    m_enNameMap.insert("CALI_STATUS_SUCCESS", tr("success"));

    // 执行方法
    m_enNameMap.insert("CALI_MODE_NONE", tr("不设定"));
    m_enNameMap.insert("CALI_MODE_BLANK", tr("空白"));
    m_enNameMap.insert("CALI_MODE_TWO_POINT", tr("2点"));
    m_enNameMap.insert("CALI_MODE_SPAN_POINT", tr("量程点"));
    m_enNameMap.insert("CALI_MODE_FULLDOT", tr("全点"));
    m_enNameMap.insert("CALI_MODE_INTELLIGENT", tr("智能"));

    // 耗材使用状态
    m_enNameMap.insert("USAGE_STATUS_CURRENT", tr("在用"));
    m_enNameMap.insert("USAGE_STATUS_BACKUP", tr("备用"));
    m_enNameMap.insert("USAGE_STATUS_CAN_NOT_USE", tr("不能使用"));

    // 项目分析检查参数
    m_enNameMap.insert("ASSAY_PARAM_SIDE_INSIDE", tr("Inside"));
    m_enNameMap.insert("ASSAY_PARAM_SIDE_OUTSIDE", tr("Outside"));
    m_enNameMap.insert("ASSAY_PARAM_SIGN_INCREASE", tr("Increase"));
    m_enNameMap.insert("ASSAY_PARAM_SIGN_DECREASE", tr("Decrease"));
    m_enNameMap.insert("ASSAY_PARAM_WAVE_SEL_PRIMARY", tr("Primary"));
    m_enNameMap.insert("ASSAY_PARAM_WAVE_SEL_DEPUTY", tr("Deputy"));
    m_enNameMap.insert("ASSAY_PARAM_WAVE_SEL_PRIMARY_SUB_DEPUTY", tr("Primary-Deputy"));

    // 项目校准方法
    m_enNameMap.insert("CALIBRATION_TYPE_LINEAR_ONE_POINT", tr("One Point Linear"));
    m_enNameMap.insert("CALIBRATION_TYPE_LINEAR_TWO_POINTS", tr("Two Point Linear"));
    m_enNameMap.insert("CALIBRATION_TYPE_LINEAR_MULTI_POINTS", tr("Multipoint Linear"));
    m_enNameMap.insert("CALIBRATION_TYPE_LOGIT_LOG3P", "Logistic-Log 3P");
    m_enNameMap.insert("CALIBRATION_TYPE_LOGIT_LOG4P", "Logistic-Log 4P");
    m_enNameMap.insert("CALIBRATION_TYPE_LOGIT_LOG5P", "Logistic-Log 5P");
    m_enNameMap.insert("CALIBRATION_TYPE_EXPONENTIAL", tr("Exponential"));
    m_enNameMap.insert("CALIBRATION_TYPE_SPLINE", tr("Spline"));

	m_enNameMap.insert("TIME_UNIT_HOUR", tr("Hour"));
	m_enNameMap.insert("TIME_UNIT_DAY", tr("Day"));
	m_enNameMap.insert("TIME_UNIT_WEEK", tr("Week"));

    // 质控方法
    m_enNameMap.insert("QC_RULE_1_3S", tr("QC_RULE_1_3S"));
    m_enNameMap.insert("QC_RULE_2_2S", tr("QC_RULE_2_2S"));
    m_enNameMap.insert("QC_RULE_R_4S", tr("QC_RULE_R_4S"));
    m_enNameMap.insert("QC_RULE_4_1S", tr("QC_RULE_4_1S"));
    m_enNameMap.insert("QC_RULE_10X", tr("QC_RULE_10X"));

	m_enNameMap.insert("TIME_UNIT_HOUR", tr("小时"));
	m_enNameMap.insert("TIME_UNIT_DAY", tr("天"));
	m_enNameMap.insert("TIME_UNIT_WEEK", tr("周"));
    m_enNameMap.insert("TIME_UNIT_MONTH", tr("月"));
    m_enNameMap.insert("TIME_UNIT_YEAR", tr("年"));

    // 用户权限
    m_enNameMap.insert("USER_TYPE_GENERAL", tr("普通用户"));
    m_enNameMap.insert("USER_TYPE_ADMIN", tr("管理员"));
    m_enNameMap.insert("USER_TYPE_MAINTAIN_ENGINEER", tr("维护工程师"));
    m_enNameMap.insert("USER_TYPE_PRODUCTION_ENGINEER", tr("产品工程师"));

    // 耗材类型
    m_enNameMap.insert("SUPPLIES_TYPE_SUBSTRATE_A", tr("底物液A"));
    m_enNameMap.insert("SUPPLIES_TYPE_SUBSTRATE_B", tr("底物液B"));
    m_enNameMap.insert("SUPPLIES_TYPE_NEW_CUPS", tr("反应杯"));
    m_enNameMap.insert("SUPPLIES_TYPE_CONCENTRATE_LIQUID", tr("清洗缓冲液"));

    // 校准状态
    m_enNameMap.insert("CALI_STATUS_DOING", tr("正在校准"));
    m_enNameMap.insert("CALI_STATUS_FAIL", tr("校准失败"));
    m_enNameMap.insert("CALI_STATUS_SUCCESS", tr("校准成功"));
    m_enNameMap.insert("CALI_STATUS_NOT", tr("未校准"));
	m_enNameMap.insert("CALI_STATUS_EMPTY", tr(""));

    // 质控申请类型
    m_enNameMap.insert("QC_TYPE_ONUSE", tr("手动选择"));
    m_enNameMap.insert("QC_TYPE_DEFAULT", tr("默认质控"));
    m_enNameMap.insert("QC_TYPE_BACKUP", tr("备用瓶质控"));
    m_enNameMap.insert("QC_TYPE_NONE", tr(""));

    // 质控原因
    m_enNameMap.insert("QC_REASON_NONE", tr(""));
    m_enNameMap.insert("QC_REASON_DEFAULT", tr("默认质控"));
    m_enNameMap.insert("QC_REASON_SCHEDULE", tr("自动质控"));
    m_enNameMap.insert("QC_REASON_MANUAL", tr("手动质控"));

    // 质控失控规则
    m_enNameMap.insert("QC_RULE_1_2S", ("1-2S"));
    m_enNameMap.insert("QC_RULE_1_2_5_S", (tr("1-2.5S")));
    m_enNameMap.insert("QC_RULE_1_3S", (tr("1-3S")));
    m_enNameMap.insert("QC_RULE_2_2S", (tr("2-2S")));
    m_enNameMap.insert("QC_RULE_3_1S", (tr("3-1S")));
    m_enNameMap.insert("QC_RULE_4_1S", (tr("4-1S")));
    m_enNameMap.insert("QC_RULE_R_4S", (tr("R-4S")));
    m_enNameMap.insert("QC_RULE_7_X", (tr("7-X")));
    m_enNameMap.insert("QC_RULE_8_X", (tr("8-X")));
    m_enNameMap.insert("QC_RULE_9_X", (tr("9-X")));
    m_enNameMap.insert("QC_RULE_10_X", (tr("10-X")));
    m_enNameMap.insert("QC_RULE_2_OF_3_2S", (tr("(2 of 3)2S")));
    m_enNameMap.insert("QC_RULE_3_OF_6_2S", (tr("(3 of 6)2S")));
    m_enNameMap.insert("QC_RULE_12_X", (tr("12-X")));
    m_enNameMap.insert("QC_RULE_7T", (tr("7-T")));

    // 免疫试剂使用状态
    m_enNameMap.insert("REAGENT_STATUS_MASK", tr("试剂屏蔽"));
    m_enNameMap.insert("REAGENT_STATUS_CALI_MASK", tr("校准屏蔽"));
    m_enNameMap.insert("REAGENT_STATUS_ON_USE", tr("在用"));
    m_enNameMap.insert("REAGENT_STATUS_BACKUP", tr("备用"));
    m_enNameMap.insert("REAGENT_STATUS_INVALID", tr(""));

    // 免疫检测模式
    m_enNameMap.insert("DETECT_MODE_ROUTINE", tr("常规模式"));
    m_enNameMap.insert("DETECT_MODE_STAT", tr("STAT模式"));
    m_enNameMap.insert("DETECT_MODE_QUICK", tr("快速模式"));

    // 传感器状态
    m_enNameMap.insert("EM_MONITOR_NOT", tr(""));
    m_enNameMap.insert("EM_MONITOR_TRIGGERED_NOT", tr("未触发"));
    m_enNameMap.insert("EM_MONITOR_TRIGGERED", tr("触发"));

    // 调试节点方向
    m_enNameMap.insert("E_ROTATION", tr("旋转方向"));
    m_enNameMap.insert("E_HORIZONTAL", tr("水平方向"));
    m_enNameMap.insert("E_VERTICAL", tr("垂直方向"));
    m_enNameMap.insert("E_EMPTY", tr("/"));

    // 调试流程参数
    m_enNameMap.insert("MODULE_DETECT_NO_CUP", tr("无管噪声检测"));
    m_enNameMap.insert("MODULE_DETECT_NULL_CUP", tr("有管噪声检测"));
    m_enNameMap.insert("MODULE_DETECT_SUB_CUP", tr("加底物噪声检测"));
    m_enNameMap.insert("MODULE_DETECT_SUB_A_ACCURACY", tr("底物A精度检测"));
    m_enNameMap.insert("MODULE_DETECT_SUB_B_ACCURACY", tr("底物B精度检测"));
    m_enNameMap.insert("MODULE_MAGTRAY_INJECT_ACCURACY", tr("注液精度检测"));
    m_enNameMap.insert("MODULE_MAGTRAY_SUCK_ACCURACY", tr("抽液精度检测"));
    m_enNameMap.insert("MODULE_SAMPLE_PMT", tr("样本针发光剂检测"));
    m_enNameMap.insert("MODULE_SAMPLE_ACCURACY", tr("样本针精度检测"));
    m_enNameMap.insert("MODULE_DILUTE_ACCURACY", tr("样本针稀释液精度检测"));
    m_enNameMap.insert("MODULE_REAGENT_PMT", tr("试剂针发光剂检测"));
    m_enNameMap.insert("MODULE_REAGENT_ACCURACY", tr("试剂针精度检测"));

    // 视觉识别结果
    m_enNameMap.insert("AI_NORMAL", tr("正常"));
    m_enNameMap.insert("AI_YELLOWS", tr("黄疸"));
    m_enNameMap.insert("AI_HEMOLYSIS", tr("溶血"));
    m_enNameMap.insert("AI_LIPOHEMIA", tr("脂血"));

    // 比较符号
    m_enNameMap.insert("COMPARE_SYMBOL_EQUAL", "＝");
    m_enNameMap.insert("COMPARE_SYMBOL_MORE_THAN", "＞");
    m_enNameMap.insert("COMPARE_SYMBOL_LESS_THAN", "＜");
    m_enNameMap.insert("COMPARE_SYMBOL_MORE_THAN_OR_EQUAL", "≥");
    m_enNameMap.insert("COMPARE_SYMBOL_LESS_THAN_OR_EQUAL", "≤");

    // 范围符号
    m_enNameMap.insert("RANGE_SYMBOL_LESS_THAN_1", "-");
    m_enNameMap.insert("RANGE_SYMBOL_MORE_THAN_1", "+");
    m_enNameMap.insert("RANGE_SYMBOL_MORE_THAN_2", "++");
    m_enNameMap.insert("RANGE_SYMBOL_MORE_THAN_3", "+++");
    m_enNameMap.insert("RANGE_SYMBOL_MORE_THAN_4", "++++");
    m_enNameMap.insert("RANGE_SYMBOL_MORE_THAN_5", "+++++");

	// 影响追加测试的异常类型
	m_enNameMap.insert("FAAT_BatchRackNotScan", tr("一批样本架未扫描完毕，请确认后重试！"));
	m_enNameMap.insert("FAAT_RackRecycleAreaFull", tr("样本架回收区满，请确认后重试！"));
	m_enNameMap.insert("FAAT_ReagentHatchOpen", tr("试剂仓盖未关闭，请确认后重试！"));
    m_enNameMap.insert("FAAT_ReagentUnScanAfterPowerOn", tr("开机后未成功执行全部试剂位扫描成功，请确认后重试！"));
    m_enNameMap.insert("FAAT_ReagentUnScanAfterHatchOpen", tr("需手动重试执行全部试剂位扫描成功，请确认后重试！"));
	m_enNameMap.insert("FAAT_ReactionDiskTempAbnormal", tr("反应盘温度异常，请确认后重试！"));
    m_enNameMap.insert("FAAT_TrackEnterBasketAbnormal", tr("轨道入口提篮异常（未放置或放置错误），请确认后重试！"));
    m_enNameMap.insert("FAAT_TrackExitBasketAbnormal", tr("轨道出口提篮异常（未放置或放置错误），请确认后重试！"));
    m_enNameMap.insert("FAAT_FrontCoverOpen", tr("仪器前盖开启，请确认后重试！"));
    m_enNameMap.insert("FAAT_BackCoverOpen", tr("仪器后盖开启，请确认后重试！"));
	m_enNameMap.insert("FAAT_WasteLiquidFull", tr("废液桶满，请确认后重试！"));

	m_enNameMap.insert("FAAT_CH_BASE", tr("生化产品线里影响追加测试的异常类型"));
	m_enNameMap.insert("FAAT_CH_CupAlkaDetergentDisable", tr("柜门的%1未启用").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB1NotExist", tr("柜门的%1-1不存在").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB2NotExist", tr("柜门的%1-2不存在").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB1Unusable", tr("柜门的%1-1不可用").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB2Unusable", tr("柜门的%1-2不可用").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB1NotPerfuse", tr("柜门的%1-1未灌注").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB2NotPerfuse", tr("柜门的%1-2未灌注").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB1PerfuseFailed", tr("柜门的%1-1灌注失败").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
	m_enNameMap.insert("FAAT_CH_CupAlkaB2PerfuseFailed", tr("柜门的%1-2灌注失败").arg(CH_CUP_DETERGENT_ALKALINITY_NAME));
    m_enNameMap.insert("FAAT_CH_MaintainReset", tr("复位失败"));
    m_enNameMap.insert("FAAT_CH_MaintainFluidPerfusionFailed", tr("管路填充失败"));
    m_enNameMap.insert("FAAT_CH_MaintainReagentScanFailed", tr("试剂扫描失败"));
    m_enNameMap.insert("FAAT_CH_MaintainIncubationWaterExchangeFailed", tr("反应槽水更换失败"));
    m_enNameMap.insert("FAAT_CH_MaintainPhotometerCheckFailed", tr("光度计检查失败"));
	m_enNameMap.insert("FAAT_CH_NotAddHitergent", tr("未向反应槽里添加抑菌剂"));

	m_enNameMap.insert("FAAT_ISE_BASE", tr("电解质设备里影响追加测试的异常类型"));
	m_enNameMap.insert("FAAT_ISE_Is1NotExist", tr("第一个模块的%1不存在").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf1NotExist", tr("第一个模块的%1不存在").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is1Unusable", tr("第一个模块的%1不可用").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf1Unusable", tr("第一个模块的%1不可用").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is1NotPerfuse", tr("第一个模块的%1未灌注").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf1NotPerfuse", tr("第一个模块的%1未灌注").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is1PerfuseFailed", tr("第一个模块的%1灌注失败").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf1PerfuseFailed", tr("第一个模块的%1灌注失败").arg(ISE_BUF_NAME));
    m_enNameMap.insert("FAAT_ISE_MaintainReset", tr("第一个模块的复位失败"));
	m_enNameMap.insert("FAAT_ISE_MaintainFluidPerfusion", tr("第一个模块管路填充失败"));
	m_enNameMap.insert("FAAT_ISE_Is2NotExist", tr("第二个模块的%1不存在").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf2NotExist", tr("第二个模块的%1不存在").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is2Unusable", tr("第二个模块的%1不可用").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf2Unusable", tr("第二个模块的%1不可用").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is2NotPerfuse", tr("第二个模块的%1未灌注").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf2NotPerfuse", tr("第二个模块的%1未灌注").arg(ISE_BUF_NAME));
	m_enNameMap.insert("FAAT_ISE_Is2PerfuseFailed", tr("第二个模块的%1灌注失败").arg(ISE_IS_NAME));
	m_enNameMap.insert("FAAT_ISE_Buf2PerfuseFailed", tr("第二个模块的%1灌注失败").arg(ISE_BUF_NAME));
    m_enNameMap.insert("FAAT_ISE_Maintain2Reset", tr("第二个模块的复位失败"));
    m_enNameMap.insert("FAAT_ISE_Maintain2FluidPerfusion", tr(" 第二个模块管路填充失败"));

	m_enNameMap.insert("FAAT_IM_BASE", tr("免疫产品线里影响追加测试的异常类型"));
	m_enNameMap.insert("FAAT_IM_CurStatuNotAllow", tr("请等待仪器待机/暂停/运行/测试结束维护时重试！"));
	m_enNameMap.insert("FAAT_IM_CupEmpty", tr("反应杯余量不足（＜10个），请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_SubstrateBothNoUse", tr("两组底物均未启用"));
    m_enNameMap.insert("FAAT_IM_Substrate1AInvalid", tr("底物组1的底物A异常（无底物或底物把手被提起），请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_Substrate1BInvalid", tr("底物组1的底物B异常（无底物或底物把手被提起），请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_Substrate2AInvalid", tr("底物组2的底物A异常（无底物或底物把手被提起），请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_Substrate2BInvalid", tr("底物组2的底物B异常（无底物或底物把手被提起），请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_SubstrateALack", tr("底物A余量不足（＜0.2mL），请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_SubstrateBLack", tr("底物B余量不足（＜0.2mL），请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_WashBufferUnAvailable", tr("无可用清洗缓冲液，请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_ConfectBacketUnderMid", tr("缓存桶低于中位，请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_PureWaterBacketUnderMid", tr("纯水桶低于中位，请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_WasteBucketFull", tr("废料桶满，请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_NotPipelineFilling", tr("需执行底物管路填充或整机管路填充成功，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_NeedWashPIP", tr("需要执行磁分离管路填充或整机管路填充成功，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_CurAbnormalStopSampling", tr("当前存在待处理的异常加样停故障，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_StopMoreThan24Hours", tr("仪器连续停机超过24小时，需先执行整机管路填充，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_SubstrateStopUseButNotDoFill", tr("底物停用后未成功执行底物停用管路维护，请确认后重试！"));
	m_enNameMap.insert("FAAT_IM_IsNeedFullPipe", tr("执行过管路清洗或上次整机管路填充失败，需先执行整机管路填充，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_EnvTempErr", tr("环境温度异常，不能进行样本测试，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_ReagTempErr", tr("试剂储存系统温度异常，不能进行样本测试，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_ConfectBacketError", tr("缓存桶浮子异常，需修复且执行在线配液自检成功，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_PureWaterBacketError", tr("纯水桶浮子异常，需修复且执行在线配液自检成功，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_NeedDoStartMaintian", tr("在设备管理中上电设备后需先执行开机维护，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_LoadUnloadReagentOnStanbyDoing", tr("正在执行试剂装卸载操作，执行完毕才能开始，请确认后重试！"));
    m_enNameMap.insert("FAAT_IM_AtSampleStopNormalStop", tr("当前处于耗材暂停状态，不能追加测试，请检查耗材（底物、反应杯）！"));

	m_enNameMap.insert("FAAT_UR_BASE", tr("尿液产品线里影响追加测试的异常类型"));

    // 设备类型
    m_enNameMap.insert("DEVICE_TYPE_INVALID", ("主应用"));
    m_enNameMap.insert("DEVICE_TYPE_I6000", ("i 6000"));

    m_enNameMap.insert("DEVICE_TYPE_C2000", ("C 2000"));
    m_enNameMap.insert("DEVICE_TYPE_C800", ("C 800"));
    m_enNameMap.insert("DEVICE_TYPE_C200", ("C 200"));
    m_enNameMap.insert("DEVICE_TYPE_C400", ("C 400"));
    m_enNameMap.insert("DEVICE_TYPE_C410", ("C 410"));
    m_enNameMap.insert("DEVICE_TYPE_C600", ("C 600"));
    m_enNameMap.insert("DEVICE_TYPE_C610", ("C 610"));
    m_enNameMap.insert("DEVICE_TYPE_C1000", ("C 1000"));
    m_enNameMap.insert("DEVICE_TYPE_C1100", ("C 1100"));
    m_enNameMap.insert("DEVICE_TYPE_C2200", ("C 2200"));

    m_enNameMap.insert("DEVICE_TYPE_ISE1005", ("ISE"));

    m_enNameMap.insert("DEVICE_TYPE_U3000", ("U3000"));

    m_enNameMap.insert("DEVICE_TYPE_TRACK", tr("轨道"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_IOM01", tr("进样回收模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_ISE", tr("轨道的ISE模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_ST04", tr("免疫运输加样模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_ST05", tr("生化运输加样模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_RWC", tr("复查缓存模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_DQI", tr("视觉识别模块"));
    m_enNameMap.insert("DEVICE_TYPE_TRACK_6008_SOFTWARE", tr("轨道控制系统软件"));

	m_enNameMap.insert("TYPE_BOARD_CHIP_CORE_ARM", tr("核心板ARM"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_CORE_FPGA", tr("核心板FPGA"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_LOWER_BOARD", tr("底板程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_DRIVER1", tr("电机驱动板1程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_DRIVER2", tr("电机驱动板2程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_DRIVER3", tr("电机驱动板3程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_TEMPCTRL", tr("温控板程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_LLD_SAMPLE", tr("样本液面凝块探测板程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_LLD_1STREAG", tr("第一试剂液面探测板程序"));
	m_enNameMap.insert("TYPE_BOARD_CHIP_LLD_2NDREAG", tr("第二试剂液面探测板程序"));
	m_enNameMap.insert("TYPE_CMS_MAIN_VERSION", tr("CMS"));
	m_enNameMap.insert("TYPE_TINYSCREEN_VERSION", tr("小屏幕"));

	// 维护项名称
	AddTfEnNameMap(tf::_MaintainItemType_VALUES_TO_NAMES,
	{	tr("无效类型"),
		tr("仪器复位"),
		tr("仪器关机"), // 0028889: [应用] 维护保养-单项维护界面“仪器关机”显示为“自动关机” added by zhang.changjiang 20240603

		tr("生免联机轨道维护项目起始"), 
		tr("清除样本架"), 

		tr("比色&免疫共同维护项目起始"),

		tr("比色&ISE共同维护项目起始"), 
		tr("排气"),
		tr("针清洗"),
		tr("超声清洗"),

		tr("生化产品线维护项目起始"),
		tr("反应槽水更换"), 
		tr("反应杯清洗"), 
		tr("杯空白测定"), 
		tr("反应杯更换"), 
		tr("光度计检查"), 
		tr("清洗反应槽"), 
		tr("清洗供水桶"),
		tr("排空供水桶"),
		tr("比色管路填充"), 
		tr("搅拌棒清洗"),
		tr("清洗液管路清洗"),
		(isPipeLine ? tr("生化试剂扫描") : tr("试剂扫描")), // 根据单机或联机动态设置

		tr("电解质产品线维护项目起始"),
		tr("ISE管路清洗"),
		tr("电极更换"),
		tr("ISE排液"),
		tr("电极清洗"),
		tr("ISE管路填充"),
		tr("ISE清洗液余量检测"),

		tr("免疫产品线维护项目起始"),
		tr("整机管路填充"),
		tr("整机管路排空"),
		tr("磁分离管路填充"),
		tr("试剂混匀"),
		tr("整机管路清洗"),
		tr("底物管路清洗"),
		tr("磁分离管路清洗"),
		tr("样本针洗针"),
		tr("试剂针洗针"),
		tr("反应传输盘校准"),
		tr("清除反应杯"),
		tr("底物停用管路维护"),
		tr("测试结束维护"),
		tr("底物管路填充"),
		tr("指定位置试剂扫描"),
		tr("洗针池灌注"),
		tr("样本针特殊清洗"),
		tr("抽液针特殊清洗"),
		tr("整机针特殊清洗"),
		tr("整机针浸泡"),
		tr("在线配液自检"),
		tr("在线配液排空"),
		tr("样本针堵针自检"),
		tr("第一磁分离堵针自检"),
		tr("第二磁分离堵针自检"),
		tr("第一磁分离注液量自检"),
		tr("第二磁分离注液量自检"),
		tr("底物A注液量自检"),
		tr("底物B注液量自检"),
		tr("故障自动修复"),
		tr("检测系统调试"),
		tr("试剂在线加载复位"),
		tr("进样系统复位"),
		tr("添加管路填充管"),
		tr("清除管路填充管"),
		//tr("样本针模块调试"),
		tr("注射模块针调试"),
		tr("磁分离模块调试"),
		tr("免疫试剂扫描"),
		tr("第一试剂针注液量自检"),
		tr("第二试剂针注液量自检"),
		tr("尿液产品线维护项目起始")
	});

	// 维护组名称
	AddTfEnNameMap(tf::_MaintainGroupType_VALUES_TO_NAMES, {
		tr("自定义维护组合"),
		tr("单项维护"),
		tr("开机维护"),
		tr("关机维护"),
		tr("日维护"),
		tr("周维护"),
		tr("月维护"),
		tr("每两个月维护"),
		tr("每三个月维护"),
		tr("每六个月维护"),
		tr("测试前维护"),
		tr("测试后维护"),
		tr("其他")
	});

	// 维护结果
	AddTfEnNameMap(tf::_MaintainResult_VALUES_TO_NAMES,
	{ tr("等待"), tr("执行中"), tr("成功"), tr("失败") });

	// 维护失败原因
	AddTfEnNameMap(tf::_MaintainFailCause_VALUES_TO_NAMES, {
		tr("流程无异常"),
		tr("报警停机，详见报警日志"),
		tr("生免联机轨道维护项目失败原因起始"),
		tr("轨道出架托盘未放置"),
		tr("轨道出架托盘放满"),
		tr("轨道进架托盘未放置"),
		tr("生化产品线维护项目失败原因起始"),
		tr("ISE产品线维护项目失败原因起始"),
		tr("免疫产品线维护项目失败原因起始") });

	// 免疫维护失败原因 modify by Chenjianlin 20240522
	AddTfEnNameMap(im::tf::_MaintainFailCause_VALUES_TO_NAMES, {
		tr("用户手动停机"),
		tr("下位机故障"),
		tr("维护项参数不合法"),
		tr("底物液异常,请检查底物液是否启用或余量是否足够！"),
		tr("指定位置试剂扫描失败"),
		tr("泡针时长不足"),
		tr("维护所需反应杯余量不足"),
		tr("执行维护时背板打开"),
		tr("执行维护前试剂仓盖打开"),
		tr("试剂存储系统漏液"),
		tr("清洗液在线配置漏液"),
		tr("液路组件漏液"),
		tr("加样洗针池漏液"),
		tr("第二试剂臂洗针池漏液"),
		tr("试剂加卸载机构正在加卸载试剂时不能执行维护"),
		tr("底物把手提起或底物把手光电异常"),
		tr("废料桶装满"),
		tr("废液桶装满"),
		tr("缓存桶中的清洗液余量不足,请先执行在线配液自检维护"),
		tr("纯水余量不足,请先执行在线配液自检维护"),
		tr("未找到可用的特殊清洗液"),
		tr("特殊清洗液余量不足"),
		tr("没有停用的底物"),
		tr("维护只能在停机和待机状态发起执行！"),
		tr("停机状态执行的维护组第一项不是复位或故障自动修复。"),
		tr("底物组1浮子处于低位"),
		tr("底物组2浮子处于低位"),
		tr("底物组1和2浮子处于低位"),
		tr("执行特殊洗针维护前未执行试剂扫描或试剂扫描失败"),
		tr("试剂仓盖关闭后未执行试剂扫描或试剂扫描失败"),
		tr("检测系统复位失败"),
		tr("检测仓旋转失败"),
		tr("打底物A失败"),
		tr("打底物B失败"),
		tr("检测仓抽废液失败"),
		tr("检测仓填充指令执行失败"),
		tr("反应传输盘复位失败"),
		tr("反应传输盘旋转失败"),
		tr("样本针垂直复位失败"),
		tr("样本抓手垂直复位失败"),
		tr("样本针水平复位失败"),
		tr("加载反应杯失败"),
		tr("反应杯加载机构滑块复位失败"),
		tr("样本盘旋转失败"),
		tr("样本抓手取杯失败"),
		tr("样本抓手放杯失败"),
		tr("样本针取清洗液失败"),
		tr("样本针注液失败"),
		tr("样本针洗针失败"),
		tr("样本针特殊清洗泡针失败"),
		tr("样本针纯水泡针失败"),
		tr("样本针洗针池灌注失败"),
		tr("轨道复位失败"),
		tr("样本针排水失败"),
		tr("第一试剂针取纯水失败"),
		tr("第一试剂针注纯水失败"),
		tr("第一试剂针取试剂失败"),
		tr("第一试剂针注试剂失败"),
		tr("第一试剂针洗针失败"),
		tr("第一试剂针洗针池灌注失败"),
		tr("第一试剂针排水失败"),
		tr("第二试剂针取纯水失败"),
		tr("第二试剂针注纯水失败"),
		tr("第二试剂针取试剂失败"),
		tr("第二试剂针注试剂失败"),
		tr("第二试剂针洗针失败"),
		tr("第二剂针针洗针池灌注失败"),
		tr("第二试剂针排水失败"),
		tr("复位试剂盘并转到试剂孔位失败"),
		tr("复位试剂盘并关闭试剂孔位失败"),
		tr("旋转试剂盘失败"),
		tr("旋转试剂盘到扫描位置失败"),
		tr("扫描指令执行失败"),
		tr("试剂针垂直复位失败"),
		tr("试剂盘复位失败"),
		tr("试剂混匀指令执行失败"),
		tr("第一清洗站抓手复位失败"),
		tr("第一清洗站抓手垂直复位失败"),
		tr("第一清洗站废液针垂直复位失败"),
		tr("第一清洗站抽废液四联泵复位失败"),
		tr("第一清洗站注液针垂直复位失败"),
		tr("第一清洗站抓手取杯失败"),
		tr("第一清洗站抓手放杯失败"),
		tr("第一清洗站转盘复位失败"),
		tr("第一清洗站转盘旋转失败"),
		tr("第一清洗站第二试剂加注盘复位失败"),
		tr("第一清洗站第二试剂加注盘旋转失败"),
		tr("第一清洗站抽废液失败"),
		tr("第一清洗站注液针抽液失败"),
		tr("第一清洗站注液针注液失败"),
		tr("第一磁分离纯水泡废液针失败"),
		tr("第一磁分离纯水泡注液针失败"),
		tr("第一磁分离废液针抽特殊清洗液失败"),
		tr("第一磁分离特殊清洗液泡废液针失败"),
		tr("第一磁分离填充管复位失败"),
		tr("第一磁分离移动填充管失败"),
		tr("第一磁分离填充指令执行失败"),
		tr("第二清洗站抓手复位失败"),
		tr("第二清洗站抓手垂直复位失败"),
		tr("第二清洗站废液针垂直复位失败"),
		tr("第二清洗站抽废液四联泵复位失败"),
		tr("第二清洗站注液针垂直复位失败"),
		tr("第二清洗站抓手取杯失败"),
		tr("第二清洗站抓手放杯失败"),
		tr("第二清洗站转盘复位失败"),
		tr("第二清洗站转盘旋转失败"),
		tr("第二清洗站抽废液失败"),
		tr("第二清洗站注液针抽液失败"),
		tr("第二清洗站注液针注液失败"),
		tr("第二磁分离纯水泡废液针失败"),
		tr("第二磁分离纯水泡注液针失败"),
		tr("第二磁分离废液针抽特殊清洗液失败"),
		tr("第二磁分离特殊清洗液泡废液针失败"),
		tr("第二磁分离填充管复位失败"),
		tr("第二磁分离移动填充管失败"),
		tr("第二磁分离填充指令执行失败"),
		tr("全盘试剂扫描执行指令执行失败"),
		tr("指定位置试剂扫描指令执行失败"),
		tr("试剂在线加载复位指令执行失败"),
		tr("进样系统复位指令执行失败"),
		tr("配液自检指令执行失败"),
		tr("打开纯水三通阀失败"),
		tr("关闭纯水三通阀失败"),
		tr("清理轨道样本架指令执行失败"),
		tr("关闭上位机配液线程失败"),
		tr("启动下位机转存线程失败"),
		tr("关闭下位机转存线程失败"),
		tr("获取纯水桶低位浮子状态失败"),
		tr("获取缓存桶低位浮子状态失败"),
		tr("获取配液桶低位浮子状态失败"),
		tr("获取底物A1信息失败"),
		tr("获取底物B1信息失败"),
		tr("获取底物A2信息失败"),
		tr("获取底物B2信息失败"),
		tr("底物组1启用但底物液状态不可用"),
		tr("底物组2启用但底物液状态不可用"),
		tr("执行停用底物管路维护时未将底物组1换成纯水"),
		tr("执行停用底物管路维护时未将底物组2换成纯水"),
		tr("执行整机管路排空时未将底物组1换成纯水"),
		tr("执行整机管路排空时未将底物组2换成纯水"),
		tr("执行整机管路排空时未将清洗缓冲液A换成纯水"),
		tr("执行整机管路排空时未将清洗缓冲液B换成纯水"),
		tr("清洗缓冲液A把手提起"),
		tr("清洗缓冲液B把手提起"),
		tr("反应杯加载异常处理指令执行失败"),
	});

    // 反应杯清洗
    AddTfEnNameMap(ch::tf::_CupDetergentType_VALUES_TO_NAMES, { 
        CH_CUP_DETERGENT_ACIDITY_NAME, 
        CH_CUP_DETERGENT_ALKALINITY_NAME,
        (QString::fromStdString(CH_CUP_DETERGENT_ACIDITY_NAME) + "+" + CH_CUP_DETERGENT_ALKALINITY_NAME) });

    // 清洗剂类型
    AddTfEnNameMap(::tf::_DetergentType_VALUES_TO_NAMES, {
        tr("未配置清洗液"),
        CH_CUP_DETERGENT_ACIDITY_NAME,
        CH_CUP_DETERGENT_ALKALINITY_NAME,
        (QString::fromStdString(CH_CUP_DETERGENT_ACIDITY_NAME) + "+" + CH_CUP_DETERGENT_ALKALINITY_NAME),
        tr("系统水"),
        tr("超声清洗"),
        tr("超声清洗") + "+" + CH_CUP_DETERGENT_ALKALINITY_NAME });

	// 项目分类
	AddTfEnNameMap(tf::_AssayClassify_VALUES_TO_NAMES,
	{ tr("其他"), tr("免疫"), tr("生化"), tr("ISE"), tr("血球"), tr("尿液") , tr("凝血") });

    // 项目分类
    AddTfEnNameMap(tf::_RerunDispatchType_VALUES_TO_NAMES,
    { tr("原模块"), tr("非原模块"), tr("随机模块"), tr("指定模块") });

	// 报警级别
	AddTfEnNameMap(tf::_AlarmLevel_VALUES_TO_NAMES,
	{ tr("未知"), tr("注意"), tr("暂停"), tr("停止") });

	// 样本源类型
	AddTfEnNameMap(tf::_SampleSourceType_VALUES_TO_NAMES,
	{ tr("其他"), tr("血清/血浆"), tr("全血"), tr("尿液"), tr("脑脊液"), tr("浆膜腔积液") });

	// 样本量类型
	AddTfEnNameMap(tf::_SuckVolType_VALUES_TO_NAMES,
	{ tr("常量"), tr("减量"), tr("增量") });

	// 分析方法
	AddTfEnNameMap(ch::tf::_AnalysisMethod_VALUES_TO_NAMES,
	{ tr("速率A法"), tr("一点终点法"), tr("两点终点法"), tr("两点速率法"), tr("速率A样本空白修正分析法")});

	// 波长类型
	AddTfEnNameMap(ch::tf::_WaveLength_VALUES_TO_NAMES,
	{ "",    "340", "380", "405",   "450",
		"480", "505", "546",   "570", "600",
        "650", "660", "700", "750",   "800"
	});

	// 奇偶校验
	AddTfEnNameMap(::tf::_Parity_VALUES_TO_NAMES, { tr("无校验"), tr("奇校验"), tr("偶校验")});

	// 停止位
	AddTfEnNameMap(::tf::_StopBits_VALUES_TO_NAMES, { "1", "1.5", "2" });

    // 项目反应时间
	AddTfEnNameMap(ch::tf::_ReactionTime_VALUES_TO_NAMES
		, {tr("3分钟"), tr("4分钟"), tr("5分钟"), tr("6分钟"), tr("7分钟"), tr("8分钟"), tr("9分钟"), tr("10分钟")
		, tr("15分钟"), tr("22分钟")});

	// 反应界限吸光度
	AddTfEnNameMap(ch::tf::_AssayParamSign_VALUES_TO_NAMES, { tr("上升"), tr("下降")});

	// 吸光度检查
	AddTfEnNameMap(ch::tf::_AssayParamWaveSel_VALUES_TO_NAMES, {tr(""), tr("主波长"), tr("次波长"), tr("主减次")});

	// 内外侧
	AddTfEnNameMap(ch::tf::_AssayParamSide_VALUES_TO_NAMES, { tr("内侧"), tr("外侧") });

	// 校准类型
	AddTfEnNameMap(ch::tf::_CalibrationType_VALUES_TO_NAMES, { tr("一点法"), tr("两点线性法"), tr("多点线性法"), tr("logitlog 3点法"),
        tr("logitlog 4点法"), tr("logitlog 5点法"), tr("指数法"), tr("样条曲线法"), tr("Akima"), tr("Pchip") });

	// 界面显示试剂或耗材状态
	AddTfEnNameMap(im::tf::_StatusForUI_VALUES_TO_NAMES, { tr("试剂屏蔽"),tr("校准屏蔽"),tr("在用"),tr("备用"),tr(" ") });

	// 免疫耗材使用状态
	AddTfEnNameMap(im::tf::_SuppliesUsingStatus_VALUES_TO_NAMES, { QString(""),tr("在用") ,tr("备用") ,tr("停用") });

	// 免疫耗材类型
	AddTfEnNameMap(im::tf::_SuppliesType_VALUES_TO_NAMES, { tr("缓存桶"), tr("底物液A"), tr("底物液B"), tr("纯水"), tr("反应杯"), tr("废料"), tr("清洗缓冲液"), tr("废液"), tr("配液桶"), tr("稀释液"), tr("特殊清洗液"), tr("试剂") });

	// 复查时样本量
	AddTfEnNameMap(::tf::_RecheckModel_VALUES_TO_NAMES, { tr("不复查"), tr("同量") ,tr("增量") ,tr("减量") ,tr("使用倍率radio参数") });

    // 患者信息
    AddTfEnNameMap(::tf::_PatientFields_VALUES_TO_NAMES, { tr("样本号"), tr("样本条码"), tr("患者ID"), tr("患者类型"), tr("临床诊断"), tr("姓名"), tr("年龄"), tr("性别"), tr("送检时间"), tr("送检科室"), tr("送检医师"),
        tr("采样时间"), tr("检验医师"), tr("住院号"), tr("收费类型"), tr("审核医师"), tr("病区"), tr("医保账户"), tr("主治医师"), tr("床号"), tr("出生日期"), tr("备注"), tr("电话"), "" });
   
    // 反应杯状态
    AddTfEnNameMap(::ch::tf::_ReactionCupStatus_VALUES_TO_NAMES, { tr("空闲"), tr("加样本/R1") ,tr("加R2") ,tr("待清洗"),tr("禁用") });

	// 耗材类型
	AddTfEnNameMap(::tf::_ConsumablesType_VALUES_TO_NAMES,
	{ 
		tr("通用耗材类型起始")
		, tr("稀释液910")
        , tr("稀释液919")
		, tr("酸性清洗液")
        , tr("碱性清洗液")
        , tr("试剂针酸性清洗液")
        , tr("试剂针碱性清洗液")
        , tr("抑菌剂")
        , tr("AT缓冲液")
        , tr("内部标准液")
        , tr("样本针酸性清洗液")
        , tr("样本针碱性清洗液")
		, tr("免疫耗材类型起始")
		, tr("底物液A")
		, tr("底物液B")
		, tr("反应杯")
		, tr("清洗缓冲液")
        , tr("稀释液")
        , tr("特殊清洗液")
        , tr("尿液耗材类型起始")
	});
}

//////////////////////////////////////////////////////////////////////////
/// @brief
///     初始化字典值映射
///
/// @par History:
/// @li 4170/TangChuXian，2020年9月11日，新建函数
///
void UiCommon::InitDiValMap()
{
    // 样本源类型
    m_diValMap.insert("urine", "SAMPLE_SOURCE_TYPE_NY");
    m_diValMap.insert("serum", "SAMPLE_SOURCE_TYPE_XQ");
    m_diValMap.insert("plasma", "SAMPLE_SOURCE_TYPE_XJ");
    m_diValMap.insert("others", "SAMPLE_SOURCE_TYPE_OTHER");

    // 样本杯
    m_diValMap.insert("normal", "TUBE_TYPE_NORMAL");
    m_diValMap.insert("reduced", "TUBE_TYPE_MICRO");
}

///
/// @brief
///     初始化当前日期更新触发器
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月17日，新建函数
///
void UiCommon::InitCurDateUpdateMonitor()
{
    QDateTime curDateTime = QDateTime::currentDateTime();
    QDateTime nextDay(curDateTime.date().addDays(1));
    unsigned int uMSecs = curDateTime.msecsTo(nextDay);

    m_pDateChangeTimer = new QTimer(this);
    m_pDateChangeTimer->setInterval(uMSecs);
    m_pDateChangeTimer->setSingleShot(true);
    connect(m_pDateChangeTimer, SIGNAL(timeout()), this, SLOT(OnNewDayArrived()));

    m_pDateChangeTimer->start();
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj    目标对象
/// @param[in]  event  事件
///
/// @return true表示已处理
///
/// @par History:
/// @li 4170/TangChuXian，2020年9月18日，新建函数
///
bool UiCommon::eventFilter(QObject *obj, QEvent *event)
{
    // 下拉框设置列表视图
    if (obj->inherits("QComboBox") && event->type() == QEvent::Resize)
    {
        // 获取下拉框控件
        QComboBox* pComboBox = qobject_cast<QComboBox*>(obj);

        // 判断类型是否转换成功
        if (pComboBox == Q_NULLPTR)
        {
            return false;
        }

        // 默认视图的下拉框设置为列表视图
        if (QString(pComboBox->view()->metaObject()->className()) == QString("QComboBoxListView"))
        {
            // 设置列表视图
            pComboBox->setView(new QListView());
        }
    }

    // 日期控件按配置日期格式显示
    if (obj->inherits("QTimeEdit") && event->type() == QEvent::Show)
    {
        return false;
    }

    // 日期控件按配置日期格式显示
    if (obj->inherits("QDateEdit") && event->type() == QEvent::Show)
    {
        // 获取日期控件
        QDateEdit* pDateEdit = qobject_cast<QDateEdit*>(obj);

        // 判断类型是否转换成功
        if (pDateEdit == Q_NULLPTR)
        {
            return false;
        }

        // 设置显示格式
        pDateEdit->setDisplayFormat(DateTimeFmtTool::GetInstance()->GetDateFormatStr());
        return false;
    }

    // 日期控件使用自定义日历表
    if (obj->inherits("QDateEdit") && event->type() == QEvent::Resize)
    {
        // 获取日期控件
        QDateEdit* pDateEdit = qobject_cast<QDateEdit*>(obj);

        // 判断类型是否转换成功
        if (pDateEdit == Q_NULLPTR)
        {
            return false;
        }

        // 设置列表视图
        pDateEdit->setCalendarPopup(true);
        pDateEdit->setCalendarWidget(new QCustomCalendarWidget());
        return false;
    }

    // 日期控件按配置日期格式显示
    if (obj->inherits("QDateTimeEdit") && event->type() == QEvent::Show)
    {
        // 获取日期控件
        QDateTimeEdit* pDateTimeEdit = qobject_cast<QDateTimeEdit*>(obj);

        // 判断类型是否转换成功
        if (pDateTimeEdit == Q_NULLPTR)
        {
            return false;
        }

        // 设置显示格式
        pDateTimeEdit->setDisplayFormat(DateTimeFmtTool::GetInstance()->GetDateTimeFormatStr());
        return false;
    }

    // 单选框自动加宽
    if (obj->inherits("QRadioButton") && event->type() == QEvent::Resize)
    {
        QRadioButton* pBtn = qobject_cast<QRadioButton*>(obj);
        if (pBtn == Q_NULLPTR)
        {
            return false;
        }

        if (pBtn->property("shown") == true)
        {
            return false;
        }

        if (pBtn->sizePolicy().horizontalPolicy() == QSizePolicy::Minimum)
        {
            QEvent* newEvent = new QEvent((QEvent::Type)CUSTOM_EVENT_RADIO_BTN_SHOWN);
            qApp->postEvent(pBtn, newEvent);
            pBtn->setProperty("shown", true);
        }
    }
    else if (obj->inherits("QRadioButton") && event->type() == CUSTOM_EVENT_RADIO_BTN_SHOWN)
    {
        QRadioButton* pBtn = qobject_cast<QRadioButton*>(obj);
        if (pBtn == Q_NULLPTR)
        {
            return false;
        }

        int iWidth = pBtn->width();
        pBtn->setMinimumWidth(iWidth + 10);
    }

    // 复选框自动加宽
    if (obj->inherits("QCheckBox") && event->type() == QEvent::Resize)
    {
        QCheckBox* pBtn = qobject_cast<QCheckBox*>(obj);
        if (pBtn == Q_NULLPTR)
        {
            return false;
        }

        if (pBtn->property("shown") == true)
        {
            return false;
        }

        if (pBtn->sizePolicy().horizontalPolicy() == QSizePolicy::Minimum)
        {
            QEvent* newEvent = new QEvent((QEvent::Type)CUSTOM_EVENT_CHECK_BTN_SHOWN);
            qApp->postEvent(pBtn, newEvent);
            pBtn->setProperty("shown", true);
        }
    }
    else if (obj->inherits("QCheckBox") && event->type() == CUSTOM_EVENT_CHECK_BTN_SHOWN)
    {
        QCheckBox* pBtn = qobject_cast<QCheckBox*>(obj);
        if (pBtn == Q_NULLPTR)
        {
            return false;
        }

        int iWidth = pBtn->width();
        pBtn->setMinimumWidth(iWidth + 10);
        pBtn->hide();
        pBtn->show();
    }

    // 转发事件
    return false;
}

bool UiCommon::CheckReagentVolumeIsValid(const std::vector<ch::tf::ReagentAspirateVol>& rav,
        const std::vector<ch::tf::SampleAspirateVol>& sav, bool isCail)
{
    int R1Vol = 0;
    int R2Vol = 0;
    for (const auto& item : rav)
    {
        if (item.stage == ch::tf::ReagentStage::REAGENT_STAGE_R1)
        {
            R1Vol = item.reagent + item.diluent;
        }

        if (item.stage == ch::tf::ReagentStage::REAGENT_STAGE_R3)
        {
            R2Vol = item.reagent + item.diluent;
        }
    }

    QString errStr;
    errStr = isCail ? tr("校准页面稀释浓度设置") : tr("分析页面样本量");
    
    for (const auto& item : sav)
    {
        if (item.originalSample < 1)
        {
            continue;
        }

        int vol = item.sample4Test > 0 ? item.sample4Test : item.originalSample;

        OnErr(item.diluent != 0 && item.originalSample == 0, errStr + tr("值错误：设置稀释液量必须设置样本量"));

        OnErr(item.sample4Test != 0 && item.diluent == 0, errStr + tr("值错误：设置稀释样本量必须设置稀释液量"));

        OnErr(item.diluent != 0 && item.sample4Test == 0, errStr + tr("值错误：设置稀释液量必须设置稀释样本量"));

        // 样本量/稀释样本量+R1 >= 65ul
        OnErr(vol + R1Vol < SAMPLE_MIX_MIN_VOL, errStr + (isCail ? "" : tr("或试剂量")) + 
            tr("值错误：%1≤样本量/稀释样本量+R1").arg(QString::number(SAMPLE_MIX_MIN_VOL / 10)));

        // 75ul <= 样本量/稀释样本量+R1+R2 <= 185ul
        OnErr(vol + R1Vol + R2Vol < SAMPLE_REACTION_MIN_VOL || vol + R1Vol + R2Vol > SAMPLE_REACTION_MAX_VOL,
            errStr + (isCail ? "" : tr("或试剂量")) + tr("值错误：%1≤样本量/稀释样本量+R1+R2≤%2")
            .arg(QString::number(SAMPLE_REACTION_MIN_VOL/10)).arg(QString::number(SAMPLE_REACTION_MAX_VOL/10)));
    }

    return true;
}

bool UiCommon::CheckAssayReferenceIsValid(::tf::AssayReference ar)
{
    // 检查参考区间只能有一个缺省项
    int count = std::count_if(ar.Items.begin(), ar.Items.end(), [](auto& item)
    {
        return item.bAutoDefault;
    });

    OnErr(count > 1, tr("值错误：参考范围中只能设置一个缺省的参考项"));

    for (auto& item : ar.Items)
    {
        // 单位换算为小时
        switch (item.enAgeUnit)
        {
        case ::tf::AgeUnit::AGE_UNIT_YEAR:
            item.iLowerAge = (item.iLowerAge == INT_MIN) ? INT_MIN : (item.iLowerAge * 8760); // 365 * 24;
            item.iUpperAge = (item.iUpperAge == INT_MAX) ? INT_MAX : (item.iUpperAge * 8760);
            break;
        case ::tf::AgeUnit::AGE_UNIT_MONTH:
            item.iLowerAge = (item.iLowerAge == INT_MIN) ? INT_MIN : (item.iLowerAge * 720);  // 30 * 24;
            item.iUpperAge = (item.iUpperAge == INT_MAX) ? INT_MAX : (item.iUpperAge * 720);
            break;
        case ::tf::AgeUnit::AGE_UNIT_DAY:
            item.iLowerAge = (item.iLowerAge == INT_MIN) ? INT_MIN : (item.iLowerAge * 24);
            item.iUpperAge = (item.iUpperAge == INT_MAX) ? INT_MAX : (item.iUpperAge * 24);
            break;
        default:
            break;
        }
    }

    // 样本类型、性别相同、年龄范围重叠即认为是重复项
    for (int i = 0; i < ar.Items.size(); i++)
    {      
        for (int j = 0; j < ar.Items.size(); j++)
        {
            if (i == j)
            {
                continue;
            }

            tf::AssayReferenceItem& refA = ar.Items[i];
            tf::AssayReferenceItem& refB = ar.Items[j];

            // 2024年7月8日讨论确定，性别为空包含男女，重复项优先匹配设置了男女的
            if (refA.sampleSourceType == refB.sampleSourceType
                && refA.enGender == refB.enGender
                && refA.bAutoDefault == refB.bAutoDefault)
            {
                OnErr((refA.iLowerAge >= refB.iLowerAge && refA.iUpperAge <= refB.iUpperAge)
                    || (refA.iLowerAge <= refB.iLowerAge && refA.iUpperAge >= refB.iUpperAge)
                    || (refA.iLowerAge <= refB.iLowerAge && refA.iUpperAge >= refB.iLowerAge)
                    || (refB.iLowerAge <= refA.iLowerAge && refB.iUpperAge >= refA.iLowerAge),
                    tr("值错误：参考范围中的参考项不可设置重复"));
            }

            // 与缺省值区间相等的检测
            OnErr(refA.sampleSourceType == refB.sampleSourceType && refA.enGender == refB.enGender \
                && refA.iLowerAge == refB.iLowerAge && refA.iUpperAge == refB.iUpperAge \
                && refA.fLowerRefRang == refB.fLowerRefRang && refA.fUpperRefRang == refB.fUpperRefRang \
                && refA.fLowerCrisisRang == refB.fLowerCrisisRang && refA.fUpperCrisisRang == refB.fUpperCrisisRang,
                tr("值错误：参考范围中的参考项不可设置重复"));
        }
    }

    return true;
}

bool UiCommon::PromptUserAfterEditGeneralAssayInfo(const ::ch::tf::GeneralAssayInfo& gai)
{
    QString test(tr("变更"));

    return true;
}

bool UiCommon::PromptUserAfterEditSpecialAssayInfo(const::ch::tf::SpecialAssayInfo & sai)
{
    QString test(tr("变更"));

    return true;
}

///
/// @brief 过滤QStringList中的相同项
///
/// @param[in]  strList  需要过滤的字符串列表
///
/// @return 过滤后的字符串列表
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月20日，新建函数
///
QStringList UiCommon::QStringListFiltSameItem(const QStringList& strList)
{
	QSet<QString> uniqueStrings;
	QStringList filteredList;
	foreach (const QString& str,  strList)
	{
		if (!uniqueStrings.contains(str))
		{
			uniqueStrings.insert(str);
			filteredList.append(str);
		}
	}

	return std::move(filteredList);
}

bool UiCommon::SaveBackupOrderConfig(BackupOrderConfig& bkOrder)
{
    BackupOrderConfig oldBkOrder;
    DictionaryQueryManager::TakeBackupOrderConfig(oldBkOrder);
    if (oldBkOrder == bkOrder)
    {
        return true;
    }
    if (!DictionaryQueryManager::SaveBackupOrderConfigAct(bkOrder))
    {
        return false;
    }

    // 自动测定血清指数
    if (oldBkOrder.autoTestSind != bkOrder.autoTestSind &&
        !AddOptLog(::tf::OperationType::MOD, bkOrder.autoTestSind
            ? tr("修改启用备用订单的“自动测定血清指数”") : tr("修改关闭备用订单的“自动测定血清指数”")))
    {
        ULOG(LOG_ERROR, "Add modify autoTestSind operate log failed !");
    }

    // 获取订单失败
    if (oldBkOrder.getOrderFailUseBackupOrder != bkOrder.getOrderFailUseBackupOrder &&
        !AddOptLog(::tf::OperationType::MOD, bkOrder.getOrderFailUseBackupOrder
            ? tr("修改启用备用订单的“获取订单失败”") : tr("修改关闭备用订单的“获取订单失败”")))
    {
        ULOG(LOG_ERROR, "Add modify getOrderFailUseBackupOrder operate log failed !");
    }

    // 样本条码扫描失败
    if (oldBkOrder.scanFailUseBackupOrder != bkOrder.scanFailUseBackupOrder &&
        !AddOptLog(::tf::OperationType::MOD, bkOrder.scanFailUseBackupOrder
            ? tr("修改启用备用订单的“样本条码扫描失败”") : tr("修改关闭备用订单的“样本条码扫描失败”")))
    {
        ULOG(LOG_ERROR, "Add modify scanFailUseBackupOrder operate log failed !");
    }

    // 订单为空
    if (oldBkOrder.orderNullUseBackupOrder != bkOrder.orderNullUseBackupOrder &&
        !AddOptLog(::tf::OperationType::MOD, bkOrder.orderNullUseBackupOrder
            ? tr("修改启用备用订单的“订单为空”") : tr("修改关闭备用订单的“订单为空”")))
    {
        ULOG(LOG_ERROR, "Add modify orderNullUseBackupOrder operate log failed !");
    }

    for (auto each : ::tf::_SampleSourceType_VALUES_TO_NAMES)
    {
        // 添加操作日志 
        QString sampleSource = ConvertTfEnumToQString((::tf::SampleSourceType::type)each.first);
        if (oldBkOrder.sampleSrcType2TestItems[each.first] != bkOrder.sampleSrcType2TestItems[each.first]
            && !AddOptLog(::tf::OperationType::MOD, tr("修改") + sampleSource + tr("的备用订单")))
        {
            ULOG(LOG_ERROR, "Add modify %s backup order operate log failed !", sampleSource);
        }
    }

    return true;
}

bool UiCommon::AddOperateLog(::tf::OperationType::type type, const QString& record)
{
    auto pUim = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (pUim == nullptr)
    {
        return false;
    }

    ::tf::OperationLog ol;
    ol.__set_user(pUim->username);
    ol.__set_operationType(type);
    ol.__set_operationTime(GetCurrentLocalTimeString());
    ol.__set_operationRecord(record.toStdString());

    ::tf::ResultLong ret;
    return DcsControlProxy::GetInstance()->AddOperationLog(ret, ol);
}

bool UiCommon::DeviceIsRunning(const ::tf::DeviceInfo& stuTfDevInfo)
{
    return (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status
            || ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status 
            || ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT == stuTfDevInfo.status 
            || ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK == stuTfDevInfo.status);
}

void UiCommon::SetButtonTextWithEllipsis(QPushButton* pBtn, const QString& strText)
{
    QFontMetrics fontMetrics(pBtn->font());
    int fontSize = fontMetrics.width(strText);

    int txtWidth = pBtn->width() > 16 ? pBtn->width() - 16 : pBtn->width(); // 假设按钮边框宽度16
    if (fontSize> txtWidth)
    {
        pBtn->setText(fontMetrics.elidedText(strText, Qt::ElideRight, txtWidth));
    }
    else
    {
        pBtn->setText(strText);
    }
}

QString UiCommon::AlarmCodeToString(int mainCode, int middleCode, int subCode)
{    
    return QString("%1").arg(mainCode, 2, 10, QChar('0')) + "-" 
            + QString("%1").arg(middleCode, 2, 10, QChar('0')) + "-" 
            + QString("%1").arg(subCode, 3, 10, QChar('0'));
}

///
/// @brief
///     记录字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2024年9月9日，新建函数
///
void UiCommon::RecordResourceString()
{
    // 记录维护项名称
    RecordMatainItemNameString();

    // 记录维护失败的原因
    RecordMatainFailReasonString();
}

///
/// @brief
///     记录维护失败原因字符串
///
/// @par History:
/// @li 4170/TangChuXian，2024年9月6日，新建函数
///
void UiCommon::RecordMatainFailReasonString()
{
    // 构造字符串资源映射
    std::map<int, std::string> mapStringRc;
    int iMaxEnumVal = 0;
    auto it = im::tf::_MaintainFailCause_VALUES_TO_NAMES.rbegin();
    if (it != im::tf::_MaintainFailCause_VALUES_TO_NAMES.rend())
    {
        iMaxEnumVal = it->first;
    }

    // 字符串转换
    QString strRc("");
    for (int i = 0; i <= iMaxEnumVal; i++)
    {
        strRc = Instance()->ConvertMaintainFailCauseToString(i);
        if (strRc.isEmpty())
        {
            continue;
        }

        mapStringRc[i] = strRc.toStdString();
        strRc.clear();
    }

    // 执行记录
    DcsControlProxy::GetInstance()->RecordMaintainFailReasonStringRc(mapStringRc);
}

///
/// @brief
///     记录维护项名称字符串
///
/// @par History:
/// @li 4170/TangChuXian，2024年9月9日，新建函数
///
void UiCommon::RecordMatainItemNameString()
{
    // 构造字符串资源映射
    std::map<int, std::string> mapStringRc;
    int iMaxEnumVal = 0;
    auto it = ::tf::_MaintainItemType_VALUES_TO_NAMES.rbegin();
    if (it != ::tf::_MaintainItemType_VALUES_TO_NAMES.rend())
    {
        iMaxEnumVal = it->first;
    }

    // 字符串转换
    QString strRc("");
    for (int i = 0; i <= iMaxEnumVal; i++)
    {
        strRc = ConvertTfEnumToQString(tf::MaintainItemType::type(i));
        if (strRc.isEmpty())
        {
            continue;
        }

        mapStringRc[i] = strRc.toStdString();
        strRc.clear();
    }

    // 执行记录
    DcsControlProxy::GetInstance()->RecordMaintainItemNameStringRc(mapStringRc);
}

///
/// @brief 维护失败枚举值转字符串信息
///
/// @param[in]  iValue  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年5月22日，新建函数
///
const QString UiCommon::ConvertMaintainFailCauseToString(int32_t iValue)
{
	// 免疫维护项目失败原因
	if (iValue > ::tf::MaintainFailCause::type::CASE_IM_FAIL_CAUSE_BASE)
	{
		return ConvertTfEnumToString((im::tf::MaintainFailCause::type)iValue);
	}
	else if (iValue > ::tf::MaintainFailCause::type::CASE_ISE_FAIL_CAUSE_BASE)	//ISE维护项目失败原因
	{
		return ConvertTfEnumToString(iValue);
	}
	else if (iValue > ::tf::MaintainFailCause::type::CASE_CH_FAIL_CAUSE_BASE)	//生化维护项目失败原因
	{
		return ConvertTfEnumToString(iValue);
	}
	else
	{
		return ConvertTfEnumToString((tf::MaintainFailCause::type)iValue);
	}
	return "";
}

///
/// @brief
///     日期编辑框被销毁
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月19日，新建函数
///
void UiCommon::OnDateEditDistroyed()
{
    // 获取日期控件
    QDateEdit* pDateEdit = qobject_cast<QDateEdit*>(sender());

    // 判断类型是否转换成功
    if (pDateEdit == Q_NULLPTR)
    {
        return;
    }

    // 查找起止日期控件映射
    if (m_dateEditMap.remove(pDateEdit))
    {
        return;
    }

    // 作为终止日期编辑框移除
    m_dateEditMap.remove(m_dateEditMap.key(pDateEdit));
}

///
/// @brief
///     起始日期改变
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月19日，新建函数
///
void UiCommon::OnStartDateChanged()
{
    // 获取起始日期控件
    QDateEdit* pStartDateEdit = qobject_cast<QDateEdit*>(sender());

    // 判断类型是否转换成功
    if (pStartDateEdit == Q_NULLPTR)
    {
        return;
    }

    // 查找起止日期控件映射
    {
        auto it = m_dateEditMap.find(pStartDateEdit);
        if (it != m_dateEditMap.end())
        {
            it.key()->setMaximumDate(it.value()->date());
            it.value()->setMinimumDate(it.key()->date());
            return;
        }
    }

    // 查找起止日期控件映射
    {
        auto it = m_qcDateEditMap.find(pStartDateEdit);
        if (it != m_qcDateEditMap.end())
        {
            // 如果开始日期到结束日期大于90天，则调整结束日期
            if (it.key()->date().daysTo(it.value()->date()) > QC_QUERY_MAX_INTERVAL_DAYS)
            {
                // 断开信号槽连接
                disconnect(it.value(), SIGNAL(dateChanged(const QDate&)), this, SLOT(OnEndDateChanged()));

                it.value()->setMinimumDate(it.key()->date());
                it.value()->setDate(it.key()->date().addDays(QC_QUERY_MAX_INTERVAL_DAYS));

                // 重新连接信号槽
                connect(it.value(), SIGNAL(dateChanged(const QDate&)), this, SLOT(OnEndDateChanged()));
            }

            it.key()->setMaximumDate(it.value()->date());
            it.value()->setMinimumDate(it.key()->date());
            return;
        }
    }
}

///
/// @brief
///     终止日期改变
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月19日，新建函数
///
void UiCommon::OnEndDateChanged()
{
    // 获取终止日期控件
    QDateEdit* pEndDateEdit = qobject_cast<QDateEdit*>(sender());

    // 判断类型是否转换成功
    if (pEndDateEdit == Q_NULLPTR)
    {
        return;
    }

    // 查找起止日期控件映射
    {
        QDateEdit* pStartDateEdit = m_dateEditMap.key(pEndDateEdit);
        if (pStartDateEdit != Q_NULLPTR)
        {
            pStartDateEdit->setMaximumDate(pEndDateEdit->date());
            pEndDateEdit->setMinimumDate(pStartDateEdit->date());
            return;
        }
    }

    // 查找起止日期控件映射
    {
        QDateEdit* pStartDateEdit = m_qcDateEditMap.key(pEndDateEdit);
        if (pStartDateEdit != Q_NULLPTR)
        {
            // 如果开始日期到结束日期大于90天，则调整结束日期
            if (pStartDateEdit->date().daysTo(pEndDateEdit->date()) > QC_QUERY_MAX_INTERVAL_DAYS)
            {
                // 断开信号槽连接
                disconnect(pStartDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnStartDateChanged()));

                pStartDateEdit->setMaximumDate(pEndDateEdit->date());
                pStartDateEdit->setDate(pEndDateEdit->date().addDays(-QC_QUERY_MAX_INTERVAL_DAYS));

                // 重新连接信号槽
                connect(pStartDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnStartDateChanged()));
            }

            pStartDateEdit->setMaximumDate(pEndDateEdit->date());
            pEndDateEdit->setMinimumDate(pStartDateEdit->date());
            return;
        }
    }
}

///
/// @brief
///     新的一天开始
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月17日，新建函数
///
void UiCommon::OnNewDayArrived()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 发送消息
    POST_MESSAGE(MSG_ID_NEW_DAY_ARRIVED);

    // 更新时间间隔
    QDateTime curDateTime = QDateTime::currentDateTime();
    QDateTime nextDay(curDateTime.date().addDays(1));
    unsigned int uMSecs = curDateTime.msecsTo(nextDay);

    m_pDateChangeTimer->setInterval(uMSecs);
    m_pDateChangeTimer->setSingleShot(true);
    if (!m_pDateChangeTimer->isActive())
    {
        m_pDateChangeTimer->start();
    }

    // 刷新日期控件
    for (auto it = m_qcDateEditMap.begin(); it != m_qcDateEditMap.end(); it++)
    {
        it.key()->setMinimumDate(QDate());
        it.key()->setMaximumDate(QDate::currentDate());
        it.value()->setMinimumDate(QDate::currentDate().addDays(-QC_QUERY_MAX_INTERVAL_DAYS));
        it.value()->setMaximumDate(QDate::currentDate());
        it.key()->setDate(QDate::currentDate().addDays(-QC_QUERY_MAX_INTERVAL_DAYS));
        it.value()->setDate(QDate::currentDate());
    }
}

///
/// @brief
///     增加平均列宽
///
/// @param[in]  pTable      表格
/// @param[in]  iAddWidth   增加宽度
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月31日，新建函数
///
void UiCommon::AddAvgColWidth(QTableView* pTable, int iAddWidth)
{
    // 普通表格获取模型
    QAbstractItemModel* pModel = pTable->model();
    if (pModel == Q_NULLPTR)
    {
        return;
    }

    // 获取水平表头
    QHeaderView* pHorHeaderView = pTable->horizontalHeader();
    if (pHorHeaderView == Q_NULLPTR)
    {
        return;
    }

    // 获取最后一个可见列
    int iLastVisibleCol = -1;
    for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
    {
        if (pTable->isColumnHidden(iCol))
        {
            continue;
        }

        iLastVisibleCol = iCol;
    }

    // 如果没有可见列则返回
    if (iLastVisibleCol < 0)
    {
        return;
    }

    for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
    {
        if (pTable->isColumnHidden(iCol))
        {
            continue;
        }

        if (iCol == iLastVisibleCol)
        {
            continue;
        }

        pTable->setColumnWidth(iCol, pTable->columnWidth(iCol) + iAddWidth);
    }

    pHorHeaderView->setStretchLastSection(true);
}

UiCommon::~UiCommon()
{
}

///
/// @brief
///     初始化单例对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年9月18日，新建函数
///
void UiCommon::Init()
{
    Instance();
}

///
/// @brief 获取单例
///     
/// @return 单实例
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月21日，新建函数
///
UiCommon* UiCommon::Instance()
{
    static UiCommon s_obj;
    return &s_obj;
}

///
/// @brief 根据样本类型和样本号生成样本类型
///
/// @param[in]  category  样本类型（急诊，常规，校准等）
/// @param[in]  seq		  样本号
///
/// @return 格式形样本号
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
std::string UiCommon::CombinSeqName(const std::string& category, const std::string& seq)
{
    // 检查是否是纯数字
    int iVal = 0;
    std::string seqTmp = seq;
    if (!stringutil::IsPureDigit(seqTmp))
    {
        return category + seqTmp;
    }

	while (seqTmp.size() < 5)
	{
		seqTmp = "0" + seqTmp;
	}

    std::string seqString = category + seqTmp;
	return seqString;
}

///
/// @brief 分解格式化的样本号
///
/// @param[in]  seqString  格式化样本号
///
/// @return 样本号
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
int UiCommon::UnCombinSeqString(std::string seqString)
{
	if (seqString.empty())
	{
		return -1;
	}

	// 删除第一个字符
	seqString.erase(0, 1);

	if (seqString.empty())
	{
		return -1;
	}
	else
	{
		return std::stoi(seqString);
	}
}

QString UiCommon::DoubleToQString(double value, int precision)
{
    QString result = QString::number(value, 'f', precision);

    // 移除多余的0
    while ((result.contains(".") && result.endsWith("0")) || result.endsWith("."))
    {
        result.chop(1);
    } 

    return result;
}

///
/// @bref
///		赋值样本类型下拉框
///
/// @param[in] pComBo 样本类型下拉框控件
///
/// @par History:
/// @li 8276/huchunli, 2023年7月13日，新建函数
///
void UiCommon::AssignSampleCombox(QComboBox* pComBo)
{
    SOFTWARE_TYPE devType = CommonInformationManager::GetInstance()->GetSoftWareType();

    // 初始化样本类型下拉框(如果存在生化设备，则多添加 脑脊液、浆膜腔积液 两种类型)
    std::vector<tf::SampleSourceType::type> lstSampleType;
    lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ);     // 血清/血浆
    lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX);       // 全血
    lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY);       // 尿液
    lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY);      // 脑脊液
    devType == SOFTWARE_TYPE::IMMUNE ? void(0) : lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY);   // 浆膜腔积液（只有免疫则不加）
    lstSampleType.push_back(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);    // 其它

    for (tf::SampleSourceType::type sType : lstSampleType)
    {
        UiCommon::Instance()->AddComboBoxItem(pComBo, sType);
    }
}

void UiCommon::AddCheckboxToTableView(QTableView* tableView, QCheckBox* checkBox, int column)
{
	QWidget *widget = new QWidget(tableView->horizontalHeader());
	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addWidget(checkBox);
	hLayout->setMargin(0);
	widget->setLayout(hLayout);

	connect(checkBox, &QCheckBox::clicked, this, [=](bool state)
	{
		for (int i = 0; i < tableView->model()->rowCount(); i++)
		{
            QWidget* wid = qobject_cast<QWidget*>(tableView->indexWidget(tableView->model()->index(i, column)));
            QCheckBox* box = wid->findChild<QCheckBox*>();
			box->setChecked(state);
		}
	});
}

///
/// @brief
///     设置表格列宽比
///
/// @param[in]  pTable     表格控件
/// @param[in]  scaleVec   列宽比列表
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月2日，新建函数
///
void UiCommon::SetTableColWidthScale(QTableView* pTable, const QVector<double>& scaleVec)
{
    // 判断参数是否合法
    if (scaleVec.empty() || pTable->model() == Q_NULLPTR)
    {
        return;
    }

    // 获取表格可见列数
    int iVisibleColCnt = 0;
    for (int i = 0; i < pTable->model()->columnCount(); i++)
    {
        if (pTable->isColumnHidden(i))
        {
            continue;
        }

        ++iVisibleColCnt;
    }

    // 列数不匹配则返回
    if (iVisibleColCnt != scaleVec.size())
    {
        return;
    }

    // 计算总份数
    double fTotalScale;
    for (int i = 0; i < scaleVec.size(); i++)
    {
        if (i == 0)
        {
            fTotalScale = scaleVec[i];
        }
        else
        {
            fTotalScale += scaleVec[i];
        }
    }

    // 如果总份数为0，则返回
    if (qAbs(fTotalScale) <= DBL_EPSILON)
    {
        return;
    }

    // 隐藏水平滚动条
    pTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 设置表格列宽用户不可调整
    pTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // 按比例设置列宽
    int iTableWidth  = pTable->width() - pTable->verticalScrollBar()->width();   // 表格宽度（滚动条除外）
    for (int i = 0; i < scaleVec.size(); i++)
    {
        if (i != scaleVec.size() - 1)
        {
            // 不为最后一列，按比例设置列宽
            pTable->setColumnWidth(i, iTableWidth * scaleVec[i] / fTotalScale);
        }
        else
        {
            // 最后一列匹配表格宽度
            pTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        }
    }
}

///
/// @brief
///     表格列宽自适应
///
/// @param[in]  pTable  表格
///
/// @par History:
/// @li 4170/TangChuXian，2022年3月29日，新建函数
///
void UiCommon::ResizeTblColToContents(QTableView* pTable)
{
    // 参数检查
    if (pTable == Q_NULLPTR)
    {
        return;
    }

    // 获取水平表头
    QHeaderView* pHorHeaderView = pTable->horizontalHeader();
    if (pHorHeaderView == Q_NULLPTR)
    {
        return;
    }
    pHorHeaderView->setStretchLastSection(false);

    // 普通表格获取模型
    QAbstractItemModel* pModel = pTable->model();
    if (pModel == Q_NULLPTR)
    {
        return;
    }

    // 插入一行复制表头内容
    bool bSortEnable = pTable->isSortingEnabled();
    pTable->setSortingEnabled(false);
    int iRow = pModel->rowCount();
    pModel->insertRow(iRow);
    for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
    {
        if (!pModel->index(iRow, iCol).isValid())
        {
            continue;
        }

        pModel->setData(pModel->index(iRow, iCol), pModel->headerData(iCol, Qt::Horizontal, Qt::DisplayRole), Qt::DisplayRole);
    }

    // 根据内容列宽自适应
    pHorHeaderView->resizeSections(QHeaderView::ResizeToContents);

    // 删除插入行
    pModel->removeRow(iRow);
    pTable->setSortingEnabled(bSortEnable);

    // 如果表格列宽未铺满，则每列均分剩余空间
    int iVisibleCol = 0;
    int iSumColWidth = 0;
    int iLastVisibleCol = -1;
    for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
    {
        if (pTable->isColumnHidden(iCol))
        {
            continue;
        }

        iSumColWidth += pTable->columnWidth(iCol);
        iLastVisibleCol = iCol;
        ++iVisibleCol;
    }

    // 没有可见列则直接返回
    if (iVisibleCol <= 0)
    {
        return;
    }

    // 获取垂直滚动条宽度
    int iMaxStep = pTable->verticalScrollBar()->maximum();
    int iMinStep = pTable->verticalScrollBar()->minimum();
    int iScrollBarWidth = (iMaxStep != iMinStep) ? pTable->verticalScrollBar()->width() : 0;

    if (iSumColWidth + iVisibleCol < pTable->width())
    {
        int iColAddAvg = (pTable->width() - iSumColWidth - iScrollBarWidth) / iVisibleCol;
        //emit SigTblAddColWidth(pTable, iColAddAvg);
        for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
        {
            if (pTable->isColumnHidden(iCol))
            {
                continue;
            }

            if (iCol == iLastVisibleCol)
            {
                continue;
            }

            pTable->setColumnWidth(iCol, pTable->columnWidth(iCol) + iColAddAvg);
        }
    }

    // 如果没有水平滚动条，则最后一列可伸缩铺满表格
    //if (!pTable->horizontalScrollBar()->isVisible())
    {
        pHorHeaderView->setStretchLastSection(true);
    }
}

///
/// @brief
///     设置表格文本对齐方式
///
/// @param[in]  pTable  表格
/// @param[in]  eAlign  对齐方式
/// @param[in]  iRow    表格行号，-1代表全表格
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月12日，新建函数
///
void UiCommon::SetTableItemAlign(QTableWidget* pTable, int eAlign, int iSpecRow)
{
    // 设置表格文本对齐方式
    for (int iRow = 0; iRow < pTable->rowCount(); iRow++)
    {
        if ((iSpecRow >= 0) && (iSpecRow != iRow))
        {
            continue;
        }

        for (int iCol = 0; iCol < pTable->columnCount(); iCol++)
        {
            QTableWidgetItem* pItem = pTable->item(iRow, iCol);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            pItem->setTextAlignment(eAlign);
        }
    }
}

///
/// @brief
///     设置表格文本对齐方式
///
/// @param[in]  pModel  表格
/// @param[in]  eAlign  对齐方式
/// @param[in]  iRow    表格行号，-1代表全表格
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月12日，新建函数
///
void UiCommon::SetTableItemAlign(QStandardItemModel* pModel, int eAlign, int iSpecRow)
{
    // 设置表格文本对齐方式
    for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
    {
        if ((iSpecRow >= 0) && (iSpecRow != iRow))
        {
            continue;
        }

        for (int iCol = 0; iCol < pModel->columnCount(); iCol++)
        {
            QStandardItem* pItem = pModel->item(iRow, iCol);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            pItem->setTextAlignment(Qt::Alignment(eAlign));
        }
    }
}

///
/// @brief
///     设置起止日期编辑控件初始范围
///
/// @param[in]  pStartEdit      起始日期编辑框
/// @param[in]  pEndEdit        终止日期编辑框
/// @param[in]  iIntervalMonth  间隔月数
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月19日，新建函数
///
void UiCommon::BindStartAndEndDateEdit(QDateEdit* pStartEdit, QDateEdit* pEndEdit, int iIntervalMonth)
{
    pStartEdit->setDate(QDate::currentDate().addMonths(-iIntervalMonth));
    pEndEdit->setDate(QDate::currentDate());

    // 关联最小日期和最大日期
    pStartEdit->setMinimumDate(QDate(2000, 1, 1));
    pStartEdit->setMaximumDate(pEndEdit->date());
    pEndEdit->setMinimumDate(pStartEdit->date());

    // 连接信号槽
    connect(pStartEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnStartDateChanged()));
    connect(pEndEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnEndDateChanged()));
    connect(pStartEdit, SIGNAL(destroyed()), this, SLOT(OnDateEditDistroyed()));
    connect(pEndEdit, SIGNAL(destroyed()), this, SLOT(OnDateEditDistroyed()));

    // 加入容器
    m_dateEditMap.insert(pStartEdit, pEndEdit);
}

///
/// @brief
///     设置质控起止日期编辑控件初始范围
///
/// @param[in]  pStartEdit  起始日期编辑框
/// @param[in]  pEndEdit    终止日期编辑框
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月17日，新建函数
///
void UiCommon::BindQcStartAndEndDateEdit(QDateEdit* pStartEdit, QDateEdit* pEndEdit)
{
    pStartEdit->setDate(QDate::currentDate().addDays(-QC_QUERY_MAX_INTERVAL_DAYS));
    pEndEdit->setDate(QDate::currentDate());

    // 关联最小日期和最大日期
    pStartEdit->setMinimumDate(QDate(2000, 1, 1));
    pStartEdit->setMaximumDate(pEndEdit->date());
    pEndEdit->setMinimumDate(pStartEdit->date());
    pEndEdit->setMaximumDate(QDate::currentDate());

    // 连接信号槽
    connect(pStartEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnStartDateChanged()));
    connect(pEndEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(OnEndDateChanged()));
    connect(pStartEdit, SIGNAL(destroyed()), this, SLOT(OnDateEditDistroyed()));
    connect(pEndEdit, SIGNAL(destroyed()), this, SLOT(OnDateEditDistroyed()));

    // 加入容器
    m_qcDateEditMap.insert(pStartEdit, pEndEdit);
}

///
/// @brief
///     获取维护项目的名称映射表
///
/// @param[out]  nameList  具体的映射表内容
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月17日，新建函数
///
void UiCommon::GetMantenanceNameList(std::vector<QString>& nameList)
{
    nameList.clear();
    nameList.push_back(tr("reset"));
    nameList.push_back(tr("washcup"));
    nameList.push_back(tr("washReactionTank"));
    nameList.push_back(tr("ReactionCupTest"));
    nameList.push_back(tr("waterBlank"));
    nameList.push_back(tr("siphonoff"));
    nameList.push_back(tr("injectWater"));
}

bool UiCommon::CheckUserInputQualitativeData(QWidget* widget, QString type/*=""*/)
{
	bool emptyFalg = false;
	double tempData = 0;
	for (int i = 1; i < 7; i++)
	{
		auto lowEdit = widget->findChild<QLineEdit*>("lineEdit_judge_low_" + type + QString::number(i));
		auto upEdit = widget->findChild<QLineEdit*>("lineEdit_judge_up_" + type + QString::number(i));
		auto signEdit = widget->findChild<QLineEdit*>("lineEdit_sign_" + type + QString::number(i));

		if (signEdit->text().isEmpty() && lowEdit->text().isEmpty() && upEdit->text().isEmpty())
		{
			tempData = 0;
			continue;
		}

		if ((signEdit->text().isEmpty()
			|| (lowEdit->text().isEmpty() && lowEdit->objectName() != "lineEdit_judge_low_" + type + "1")
			|| (upEdit->text().isEmpty() && upEdit->objectName() != "lineEdit_judge_up_" + type + "6")))
		{
			emptyFalg = true;
		}

		if (!signEdit->text().isEmpty() || !lowEdit->text().isEmpty() || !upEdit->text().isEmpty())
		{
			if (emptyFalg)
			{
                TipDlg(tr("定性判断范围错误")).exec();
				return false;
			}
		}

		if ((lowEdit->text().toDouble() < tempData)
			|| ((!lowEdit->text().isEmpty() && !upEdit->text().isEmpty())
				&& (upEdit->text().toDouble() < lowEdit->text().toDouble())))
		{
            TipDlg(tr("定性判断范围错误")).exec();
			return false;
		}

		tempData = upEdit->text().toDouble();
	}

    for (int i=1; i<6; i++)
    {
        auto updata1 = widget->findChild<QLineEdit*>("lineEdit_judge_up_" + type + QString::number(i))->text();
        auto update2 = widget->findChild<QLineEdit*>("lineEdit_judge_up_" + type + QString::number(i + 1))->text();
        if ((updata1 == update2) && !updata1.isEmpty())
        {
            TipDlg(tr("定性判断范围错误")).exec();
            return false;
        }
    }

    // 检查定性判断标记
    QRegExp re("lineEdit_sign_");
    auto edits = widget->findChildren<QLineEdit*>(re);
    for (int i = 0; i < edits.size()-1; i++)
    {
        for (int j = i+1; j < edits.size(); j++)
        {
            if (edits[i]->text() == edits[j]->text() && !edits[i]->text().isEmpty())
            {
                TipDlg(tr("定性判断标记不能重复")).exec();
                return false;
            }
        }
    }

	return true;
}

// 重载"样本量/稀释"流输入运算
std::ostream& operator<<(std::ostream& out, const SUCK_VOL_OR_DILLUTE& val)
{
    switch (val)
    {
    case SUCK_VOL_STD:
        out << "SUCK_VOL_STD";
        break;
    case SUCK_VOL_INC:
        out << "SUCK_VOL_INC";
        break;
    case SUCK_VOL_DEC:
        out << "SUCK_VOL_DEC";
        break;
    case DILLUTE:
        out << "DILLUTE";
        break;
    case PRE_DILLUTE:
        out << "PRE_DILLUTE";
        break;
    default:
        out << static_cast<int>(val);
        break;
    }

    return out;
}

// 重载"稀释倍数"流输入运算
std::ostream& operator<<(std::ostream& out, const DILLUTE_FACTOR& val)
{
    switch (val)
    {
    case DILLUTE_FACTOR_3:
        out << "DILLUTE_FACTOR_3";
        break;
    case DILLUTE_FACTOR_5:
        out << "DILLUTE_FACTOR_5";
        break;
    case DILLUTE_FACTOR_10:
        out << "DILLUTE_FACTOR_10";
        break;
    case DILLUTE_FACTOR_20:
        out << "DILLUTE_FACTOR_20";
        break;
    case DILLUTE_FACTOR_50:
        out << "DILLUTE_FACTOR_50";
        break;
    default:
        out << static_cast<int>(val);
        break;
    }

    return out;
}

// 质控状态
std::ostream& operator<<(std::ostream& out, const QC_STATE& val)
{
    switch (val)
    {
    case QC_STATE_NONE:
        out << "QC_STATE_NONE";
        break;
    case QC_STATE_IN_CTRL:
        out << "QC_STATE_IN_CTRL";
        break;
    case QC_STATE_WARNING:
        out << "QC_STATE_WARNING";
        break;
    case QC_STATE_OUT_OF_CTRL:
        out << "QC_STATE_OUT_OF_CTRL";
        break;
        break;
    default:
        out << static_cast<int>(val);
        break;
    }

    return out;
}

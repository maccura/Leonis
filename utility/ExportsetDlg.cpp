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
/// @file     ExportSetDlg.cpp
/// @brief    导出设置对话框
///
/// @author   8580/GongZhiQiang
/// @date     2023年11月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ExportSetDlg.h"
#include "ui_ExportSetDlg.h"

#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/FileExporter.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "boost/optional.hpp"
#include "uidcsadapter/uidcsadapter.h"
#include "manager/UserInfoManager.h"

#include <QCheckBox>
#include <QLabel>

// 导出信息枚举字符串
#define PROPERTY_EXPORT_INFO_EN		("ExportInfoEn")

// 生化布局列数
#define LAYOUT_COLUM_MAX_NUM		(5)

ExportSetDlg::ExportSetDlg(QWidget *parent)
	: BaseDlg(parent),
	  m_bInit(false)
{
	ui = new Ui::ExportSetDlg();
	ui->setupUi(this);

	// 显示前初始化
	InitBeforeShow();
}

ExportSetDlg::~ExportSetDlg()
{
	delete ui;
}

///
/// @brief 显示前初始化
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::InitBeforeShow()
{
	// 设置标题
	SetTitleName(tr("导出设置"));

	// 对应界面索引
	m_btnGroup.addButton(ui->rabtn_iseSamAndQc, 0);
	m_btnGroup.addButton(ui->rabtn_iseCali,		1);
	m_btnGroup.addButton(ui->rabtn_normalAndQc, 2);
	m_btnGroup.addButton(ui->rbtn_cali,			3);


	// 默认选中第一个按钮
	if (!gUiAdapterPtr()->WhetherContainTargetDevClassify(tf::AssayClassify::ASSAY_CLASSIFY_ISE))
	{
		// 隐藏ISE选项
		ui->rabtn_iseSamAndQc->setVisible(false);
		ui->rabtn_iseCali->setVisible(false);
		ui->rabtn_normalAndQc->setChecked(true);
		ui->stackedWidget->setCurrentIndex(2);
	}
	else
	{
		ui->rabtn_iseSamAndQc->setChecked(true);
	}

	// 设置生化免疫切换按钮是否可见
	SOFTWARE_TYPE type = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (type == CHEMISTRY)
	{
		// 移除免疫标签
		ui->tabWidget->removeTab(1);
	}
	else if (type == IMMUNE)
	{
		// 移除生化标签，并隐藏子界面按钮
		ui->tabWidget->removeTab(0);
		ui->rabtn_widget->hide();
	}

	// 重新设置界面布局
	if (ui->tabWidget->count() < 2)
	{
		ui->tabWidget->tabBar()->hide();

		// 隐藏后，如果是生化，还需要调整界面
		if (type == CHEMISTRY)
		{
			ui->tabWidget->move(ui->tabWidget->x(), ui->tabWidget->y()+ ui->rabtn_widget->height());
			ui->tabWidget->setFixedHeight(ui->tabWidget->height() - ui->rabtn_widget->height());
		}
	}

	// 添加界面控件
	InitIseSamAndQcPage();
	InitIseCaliPage();
	InitNormalAndQcPage();
	InitCaliPage();
    InitImmunePage();
}

///
/// @brief 初始化校准界面
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::InitCaliPage()
{
	// 构建控件
	QVector<QPair<QString, ExportInfoEn>>	checkBoxList;
	checkBoxList.push_back(qMakePair(tr("校准次数"), ExportInfoEn::EIE_CALI_COUNT));
	checkBoxList.push_back(qMakePair(tr("校准方法"), ExportInfoEn::EIE_CALIBRATION_TYPE));
	checkBoxList.push_back(qMakePair(tr("校准点数"), ExportInfoEn::EIE_CALIBRATION_POINTS));
	checkBoxList.push_back(qMakePair(tr("样本条码"), ExportInfoEn::EIE_SAMPLE_BARCODE));
	checkBoxList.push_back(qMakePair(tr("样本号"), ExportInfoEn::EIE_SAMPLE_SEQ_NO));
	checkBoxList.push_back(qMakePair(tr("样本类别"), ExportInfoEn::EIE_SAMPLE_TYPESTAT));
	//checkBoxList.push_back(qMakePair(tr("架号"), ExportInfoEn::EIE_RACK));
	checkBoxList.push_back(qMakePair(tr("位置"), ExportInfoEn::EIE_POSITION));
	checkBoxList.push_back(qMakePair(tr("反应杯号"), ExportInfoEn::EIE_REACTION_CUP_NO));
	checkBoxList.push_back(qMakePair(tr("校准品名称"), ExportInfoEn::EIE_CALIBRATOR_NAME));
	checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_CALIBRATOR_LOT));
	checkBoxList.push_back(qMakePair(tr("项目名称"), ExportInfoEn::EIE_ASSAY_NAME));
	checkBoxList.push_back(qMakePair(tr("样本量"), ExportInfoEn::EIE_SUCK_VOL));
	//checkBoxList.push_back(qMakePair(tr("稀释样本量"), ExportInfoEn::EIE_DILUTION_SAMPLE_VOL));
	//checkBoxList.push_back(qMakePair(tr("稀释液量"), ExportInfoEn::EIE_DILUTION_VOL));
	checkBoxList.push_back(qMakePair(tr("计算吸光度"), ExportInfoEn::EIE_CALCULATE_ABS));
	checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_DATA_ALARM));
	//checkBoxList.push_back(qMakePair(tr("校准水平"), ExportInfoEn::EIE_CALI_LEVEL));
	checkBoxList.push_back(qMakePair(tr("浓度"), ExportInfoEn::EIE_CONC));
	checkBoxList.push_back(qMakePair(tr("单位"), ExportInfoEn::EIE_UNIT));
	checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_TEST_FINISHED_TIME));
	checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_MODULE));
	//checkBoxList.push_back(qMakePair(tr("试剂盘"), ExportInfoEn::EIE_REAGENT_DISK));
	checkBoxList.push_back(qMakePair(tr("试剂批号"), ExportInfoEn::EIE_REAGENT_LOT));
	checkBoxList.push_back(qMakePair(tr("试剂瓶号"), ExportInfoEn::EIE_REAGENT_SN));
	//checkBoxList.push_back(qMakePair(tr("酸性清洗液批号"), ExportInfoEn::EIE_ACIDITY_LOT));
	//checkBoxList.push_back(qMakePair(tr("碱性清洗液批号"), ExportInfoEn::EIE_ALKALINITY_LOT));
	checkBoxList.push_back(qMakePair(tr("主波长"), ExportInfoEn::EIE_PRIMARY_WAVE));
	checkBoxList.push_back(qMakePair(tr("次波长"), ExportInfoEn::EIE_DEPUTY_WAVE));
	checkBoxList.push_back(qMakePair(tr("反应时间"), ExportInfoEn::EIE_REACTION_TIME));
	checkBoxList.push_back(qMakePair(tr("测光点"), ExportInfoEn::EIE_DETECT_POINT));
	checkBoxList.push_back(qMakePair(tr("主波长吸光度"), ExportInfoEn::EIE_PRIMARY_WAVE_ABS));
	checkBoxList.push_back(qMakePair(tr("次波长吸光度"), ExportInfoEn::EIE_DEPUTY_WAVE_ABS));
	checkBoxList.push_back(qMakePair(tr("所有波长吸光度"), ExportInfoEn::EIE_ALL_WAVE_ABS));

	// 添加到界面
	AddCheckBoxsToWidget(ui->page_cali, checkBoxList);
}

///
/// @brief
///     初始化免疫页面
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月1日，新建函数
///
void ExportSetDlg::InitImmunePage()
{
    // 初始化成员变量（当前登录用户）
    bool bEng = false;
    std::shared_ptr<tf::UserInfo> spLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if ((spLoginUserInfo != Q_NULLPTR) && (spLoginUserInfo->type >= tf::UserType::USER_TYPE_ENGINEER))
    {
        bEng = true;
    }

    // 构建控件
    QVector<QPair<QString, ExportInfoEn>>	checkBoxList;
    checkBoxList.push_back(qMakePair(tr("样本号"), ExportInfoEn::EIE_IM_SAMPLE_NO));
    checkBoxList.push_back(qMakePair(tr("样本条码"), ExportInfoEn::EIE_IM_BARCODE));
    checkBoxList.push_back(qMakePair(tr("样本类型"), ExportInfoEn::EIE_IM_SAMPLE_SOURCE_TYPE));
    checkBoxList.push_back(qMakePair(tr("位置"), ExportInfoEn::EIE_IM_POS));
    checkBoxList.push_back(qMakePair(tr("项目名称"), ExportInfoEn::EIE_IM_ITEM_NAME));
    checkBoxList.push_back(qMakePair(tr("单位"), ExportInfoEn::EIE_IM_UNIT));

    // 首次结果
    checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_IM_TEST_MODULE));
    checkBoxList.push_back(qMakePair(tr("样本申请模式"), ExportInfoEn::EIE_IM_SAMPLE_SEND_MODLE));
    checkBoxList.push_back(qMakePair(tr("检测模式"), ExportInfoEn::EIE_IM_TEST_MODLE));
    checkBoxList.push_back(qMakePair(tr("样本管类型"), ExportInfoEn::EIE_IM_TUBE_TYPE));
    checkBoxList.push_back(qMakePair(tr("信号值"), ExportInfoEn::EIE_IM_RUL));
    if (bEng)
    {
        checkBoxList.push_back(qMakePair(tr("原始信号值"), ExportInfoEn::EIE_IM_ORIGNIAL_RUL));
    }
    checkBoxList.push_back(qMakePair(tr("检测结果"), ExportInfoEn::EIE_IM_TEST_RESULT));
    if (bEng)
    {
        checkBoxList.push_back(qMakePair(tr("原始结果"), ExportInfoEn::EIE_IM_ORIGNIAL_TEST_RESULT));
    }
    checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_IM_RESULT_STATUS));
    //checkBoxList.push_back(qMakePair(tr("视觉识别结果"), ExportInfoEn::EIE_IM_AI_RESULT));
    checkBoxList.push_back(qMakePair(tr("手工稀释倍数"), ExportInfoEn::EIE_IM_PRE_DILUTION_FACTOR));
    checkBoxList.push_back(qMakePair(tr("机内稀释倍数"), ExportInfoEn::EIE_IM_DILUTION_FACTOR));
    checkBoxList.push_back(qMakePair(tr("试剂批号"), ExportInfoEn::EIE_IM_REAGENT_BATCHNO));
    checkBoxList.push_back(qMakePair(tr("试剂瓶号"), ExportInfoEn::EIE_IM_REAGENT_SERIAL_NO));
    checkBoxList.push_back(qMakePair(tr("试剂上机时间"), ExportInfoEn::EIE_IM_RGT_REGISTER_T));
    checkBoxList.push_back(qMakePair(tr("试剂失效日期"), ExportInfoEn::EIE_IM_REAGENT_EXP_TIME));
    checkBoxList.push_back(qMakePair(tr("底物液批号"), ExportInfoEn::EIE_IM_SUBSTRATE_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("底物液瓶号"), ExportInfoEn::EIE_IM_SUBSTRATE_BOTTLE_NO));
    checkBoxList.push_back(qMakePair(tr("清洗缓冲液批号"), ExportInfoEn::EIE_IM_CLEAN_FLUID_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_IM_END_TEST_TIME));
    checkBoxList.push_back(qMakePair(tr("稀释液批号"), ExportInfoEn::EIE_IM_DILUENT_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("反应杯批号"), ExportInfoEn::EIE_IM_ASSAY_CUP_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_IM_CALI_LOT));
    checkBoxList.push_back(qMakePair(tr("当前工作曲线"), ExportInfoEn::EIE_IM_FIRST_CUR_CALI_TIME));
    checkBoxList.push_back(qMakePair(tr("项目重复次数"), ExportInfoEn::EIE_IM_FIRST_TEST_COUNTS));

    // 复查结果
    checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_IM_RE_TEST_MODULE));
    checkBoxList.push_back(qMakePair(tr("样本申请模式"), ExportInfoEn::EIE_IM_RE_SAMPLE_SEND_MODLE));
    checkBoxList.push_back(qMakePair(tr("检测模式"), ExportInfoEn::EIE_IM_RE_TEST_MODLE));
    checkBoxList.push_back(qMakePair(tr("样本管类型"), ExportInfoEn::EIE_IM_RE_TEST_TUBE_TYPE));
    checkBoxList.push_back(qMakePair(tr("信号值"), ExportInfoEn::EIE_IM_RE_TEST_RUL));
    if (bEng)
    {
        checkBoxList.push_back(qMakePair(tr("原始信号值"), ExportInfoEn::EIE_IM_RE_TEST_ORIG_LRUL));
    }
    checkBoxList.push_back(qMakePair(tr("检测结果"), ExportInfoEn::EIE_IM_RE_TESTRESULT));
    if (bEng)
    {
        checkBoxList.push_back(qMakePair(tr("原始结果"), ExportInfoEn::EIE_IM_RE_ORIGL_TEST_RESULT));
    }
    checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_IM_RE_RESULT_STATUS));
    //checkBoxList.push_back(qMakePair(tr("视觉识别结果"), ExportInfoEn::EIE_IM_RE_AI_RESULT));
    checkBoxList.push_back(qMakePair(tr("手工稀释倍数"), ExportInfoEn::EIE_IM_RE_PRE_DIL_FACTOR));
    checkBoxList.push_back(qMakePair(tr("机内稀释倍数"), ExportInfoEn::EIE_IM_RE_DIL_FACTOR));
    checkBoxList.push_back(qMakePair(tr("试剂批号"), ExportInfoEn::EIE_IM_RE_REAGENT_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("试剂瓶号"), ExportInfoEn::EIE_IM_RE_REAGENT_SERIAL_NO));
    checkBoxList.push_back(qMakePair(tr("试剂上机时间"), ExportInfoEn::EIE_IM_RE_RGT_REGISTER_T));
    checkBoxList.push_back(qMakePair(tr("试剂失效日期"), ExportInfoEn::EIE_IM_RE_REAGENT_EXP_TIME));
    checkBoxList.push_back(qMakePair(tr("底物液批号"), ExportInfoEn::EIE_IM_RE_SUB_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("底物液瓶号"), ExportInfoEn::EIE_IM_RE_SUB_BOTTLE_NO));
    checkBoxList.push_back(qMakePair(tr("清洗缓冲液批号"), ExportInfoEn::EIE_IM_RE_CLEAN_FLUID_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_IM_RE_END_TEST_TIME));
    checkBoxList.push_back(qMakePair(tr("稀释液批号"), ExportInfoEn::EIE_IM_RE_DILUENT_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("反应杯批号"), ExportInfoEn::EIE_IM_RE_ASSAY_CUP_BATCH_NO));
    checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_IM_RE_CALI_LOT));
    checkBoxList.push_back(qMakePair(tr("当前工作曲线"), ExportInfoEn::EIE_IM_RE_TEST_CUR_CALI_TIME));
    checkBoxList.push_back(qMakePair(tr("项目重复次数"), ExportInfoEn::EIE_IM_RE_TEST_COUNTS));

    // 构造标题
    QMap<int, GridTitleInfo> mapTitle;
    mapTitle.insert(0, GridTitleInfo(3, 6, QString("")));
    mapTitle.insert(2, GridTitleInfo(LAYOUT_COLUM_MAX_NUM, bEng ? 24 : 22, tr("首次检测信息")));
    mapTitle.insert(8, GridTitleInfo(LAYOUT_COLUM_MAX_NUM, bEng ? 24 : 22, tr("复查信息")));

    // 添加到界面
    AddCheckBoxsToWidget(ui->tab_im, checkBoxList, mapTitle);
}

///
/// @brief 初始化常规&质控界面
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::InitNormalAndQcPage()
{
	// 构建控件
	QVector<QPair<QString, ExportInfoEn>>	checkBoxList;
	checkBoxList.push_back(qMakePair(tr("样本申请模式"), ExportInfoEn::EIE_SAMPLE_APPLY_MODE));
	checkBoxList.push_back(qMakePair(tr("样本类型"), ExportInfoEn::EIE_SAMPLE_TYPE));
	checkBoxList.push_back(qMakePair(tr("样本管类型"), ExportInfoEn::EIE_TUBE_TYPE));
	checkBoxList.push_back(qMakePair(tr("样本条码"), ExportInfoEn::EIE_SAMPLE_BARCODE));
	checkBoxList.push_back(qMakePair(tr("样本号"), ExportInfoEn::EIE_SAMPLE_SEQ_NO));
	checkBoxList.push_back(qMakePair(tr("样本类别"), ExportInfoEn::EIE_SAMPLE_TYPESTAT));
	//checkBoxList.push_back(qMakePair(tr("架号"), ExportInfoEn::EIE_RACK));
	checkBoxList.push_back(qMakePair(tr("位置"), ExportInfoEn::EIE_POSITION));
	//checkBoxList.push_back(qMakePair(tr("加R1时间"), ExportInfoEn::EIE_ADD_REAGENT1_TIME));
	//checkBoxList.push_back(qMakePair(tr("加样本时间"), ExportInfoEn::EIE_ADD_SAMPLE_TIME));
	//checkBoxList.push_back(qMakePair(tr("混匀时间1"), ExportInfoEn::EIE_VORTEX_TIME1));
	//checkBoxList.push_back(qMakePair(tr("加R2时间"), ExportInfoEn::EIE_ADD_REAGENT2_TIME));
	//checkBoxList.push_back(qMakePair(tr("混匀时间2"), ExportInfoEn::EIE_VORTEX_TIME2));
	checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_TEST_FINISHED_TIME));
	checkBoxList.push_back(qMakePair(tr("项目名称"), ExportInfoEn::EIE_ASSAY_NAME));
	checkBoxList.push_back(qMakePair(tr("重复次数"), ExportInfoEn::EIE_REPEAT_TIMES));
	checkBoxList.push_back(qMakePair(tr("原始结果"), ExportInfoEn::EIE_ORIGINAL_RESULT));
	checkBoxList.push_back(qMakePair(tr("结果"), ExportInfoEn::EIE_RESULT));
	checkBoxList.push_back(qMakePair(tr("吸光度"), ExportInfoEn::EIE_ABS));
	checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_DATA_ALARM));
	checkBoxList.push_back(qMakePair(tr("单位"), ExportInfoEn::EIE_UNIT));
	checkBoxList.push_back(qMakePair(tr("样本量"), ExportInfoEn::EIE_SUCK_VOL));
	checkBoxList.push_back(qMakePair(tr("手工稀释倍数"), ExportInfoEn::EIE_PRE_DILUTION_FACTOR));
	checkBoxList.push_back(qMakePair(tr("机内稀释倍数"), ExportInfoEn::EIE_AUTO_DILUTION_FACTOR));
	//checkBoxList.push_back(qMakePair(tr("视觉识别结果"), ExportInfoEn::EIE_AI_RECOGNIZE_RESULT));
	checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_MODULE));
	//checkBoxList.push_back(qMakePair(tr("试剂盘"), ExportInfoEn::EIE_REAGENT_DISK));
	checkBoxList.push_back(qMakePair(tr("反应杯号"), ExportInfoEn::EIE_REACTION_CUP_NO));
	checkBoxList.push_back(qMakePair(tr("试剂批号"), ExportInfoEn::EIE_REAGENT_LOT));
	checkBoxList.push_back(qMakePair(tr("试剂瓶号"), ExportInfoEn::EIE_REAGENT_SN));
	checkBoxList.push_back(qMakePair(tr("试剂上机时间"), ExportInfoEn::EIE_REAGENT_REGISTER_TIME));
	checkBoxList.push_back(qMakePair(tr("试剂失效日期"), ExportInfoEn::EIE_REAGENT_EXPIRY_TIME));
	checkBoxList.push_back(qMakePair(tr("开瓶有效期"), ExportInfoEn::EIE_OPEN_BOTTLE_EXPIRY_TIME));
	//checkBoxList.push_back(qMakePair(tr("酸性清洗液批号"), ExportInfoEn::EIE_ACIDITY_LOT));
	//checkBoxList.push_back(qMakePair(tr("碱性清洗液批号"), ExportInfoEn::EIE_ALKALINITY_LOT));
	//checkBoxList.push_back(qMakePair(tr("针酸性清洗液批号"), ExportInfoEn::EIE_SAMPLE_PROBE_ACIDITY_LOT));
	//checkBoxList.push_back(qMakePair(tr("针碱性清洗液批号"), ExportInfoEn::EIE_SAMPLE_PROBE_ALKALINITY_LOT));
	checkBoxList.push_back(qMakePair(tr("校准品名称"), ExportInfoEn::EIE_CALIBRATOR_NAME));
	checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_CALIBRATOR_LOT));
	checkBoxList.push_back(qMakePair(tr("校准时间"), ExportInfoEn::EIE_CALI_TIME));
	checkBoxList.push_back(qMakePair(tr("分析方法"), ExportInfoEn::EIE_ANALYSIS_METHOD));
	checkBoxList.push_back(qMakePair(tr("主波长"), ExportInfoEn::EIE_PRIMARY_WAVE));
	checkBoxList.push_back(qMakePair(tr("次波长"), ExportInfoEn::EIE_DEPUTY_WAVE));
	checkBoxList.push_back(qMakePair(tr("反应时间"), ExportInfoEn::EIE_REACTION_TIME));
	checkBoxList.push_back(qMakePair(tr("测光点"), ExportInfoEn::EIE_DETECT_POINT));
	checkBoxList.push_back(qMakePair(tr("主波长吸光度"), ExportInfoEn::EIE_PRIMARY_WAVE_ABS));
	checkBoxList.push_back(qMakePair(tr("次波长吸光度"), ExportInfoEn::EIE_DEPUTY_WAVE_ABS));
	checkBoxList.push_back(qMakePair(tr("所有波长吸光度"), ExportInfoEn::EIE_ALL_WAVE_ABS));
	

	// 添加到界面
	AddCheckBoxsToWidget(ui->page_normalAndQc, checkBoxList);
}

///
/// @brief 初始化ISE样本&质控界面
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::InitIseSamAndQcPage()
{
	// 构建控件
	QVector<QPair<QString, ExportInfoEn>>	checkBoxList;
	checkBoxList.push_back(qMakePair(tr("样本申请模式"), ExportInfoEn::EIE_SAMPLE_APPLY_MODE));
	checkBoxList.push_back(qMakePair(tr("样本类型"), ExportInfoEn::EIE_SAMPLE_TYPE));
	checkBoxList.push_back(qMakePair(tr("样本管类型"), ExportInfoEn::EIE_TUBE_TYPE));
	checkBoxList.push_back(qMakePair(tr("稀释倍数"), ExportInfoEn::EIE_DILUTION_FACTOR));
	checkBoxList.push_back(qMakePair(tr("手工稀释倍数"), ExportInfoEn::EIE_PRE_DILUTION_FACTOR));
	checkBoxList.push_back(qMakePair(tr("样本条码"), ExportInfoEn::EIE_SAMPLE_BARCODE));
	checkBoxList.push_back(qMakePair(tr("样本号"), ExportInfoEn::EIE_SAMPLE_SEQ_NO));
	checkBoxList.push_back(qMakePair(tr("样本类别"), ExportInfoEn::EIE_SAMPLE_TYPESTAT));
	//checkBoxList.push_back(qMakePair(tr("架号"), ExportInfoEn::EIE_RACK));
	checkBoxList.push_back(qMakePair(tr("位置"), ExportInfoEn::EIE_POSITION));
	checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_TEST_FINISHED_TIME));
	checkBoxList.push_back(qMakePair(tr("项目名称"), ExportInfoEn::EIE_ASSAY_NAME));
	checkBoxList.push_back(qMakePair(tr("重复次数"), ExportInfoEn::EIE_REPEAT_TIMES));
	checkBoxList.push_back(qMakePair(tr("原始结果"), ExportInfoEn::EIE_ORIGINAL_RESULT));
	checkBoxList.push_back(qMakePair(tr("结果"), ExportInfoEn::EIE_RESULT));
	checkBoxList.push_back(qMakePair(tr("计算电动势"), ExportInfoEn::EIE_CALCULATE_EMF));
	checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_DATA_ALARM));
	//checkBoxList.push_back(qMakePair(tr("视觉识别结果"), ExportInfoEn::EIE_AI_RECOGNIZE_RESULT));
	checkBoxList.push_back(qMakePair(tr("单位"), ExportInfoEn::EIE_UNIT));
	checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_MODULE));
	checkBoxList.push_back(qMakePair(tr("内部标准液批号"), ExportInfoEn::EIE_IS_LOT));
	checkBoxList.push_back(qMakePair(tr("内部标准液瓶号"), ExportInfoEn::EIE_IS_SN));
	checkBoxList.push_back(qMakePair(tr("AT缓冲液批号"), ExportInfoEn::EIE_DILUENT_LOT));
	checkBoxList.push_back(qMakePair(tr("AT缓冲液瓶号"), ExportInfoEn::EIE_DILUENT_SN));
	//checkBoxList.push_back(qMakePair(tr("针酸性清洗液批号"), ExportInfoEn::EIE_SAMPLE_PROBE_ACIDITY_LOT));
	//checkBoxList.push_back(qMakePair(tr("针碱性清洗液批号"), ExportInfoEn::EIE_SAMPLE_PROBE_ALKALINITY_LOT));
	checkBoxList.push_back(qMakePair(tr("校准品名称"), ExportInfoEn::EIE_CALIBRATOR_NAME));
	checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_CALIBRATOR_LOT));
	checkBoxList.push_back(qMakePair(tr("校准时间"), ExportInfoEn::EIE_CALI_TIME));
	

	// 添加到界面
	AddCheckBoxsToWidget(ui->page_iseSamAndQc, checkBoxList);
}

///
/// @brief 初始化ISE校准界面
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::InitIseCaliPage()
{
	// 构建控件
	QVector<QPair<QString, ExportInfoEn>>	checkBoxList;
	checkBoxList.push_back(qMakePair(tr("校准次数"), ExportInfoEn::EIE_CALI_COUNT));
	checkBoxList.push_back(qMakePair(tr("样本条码"), ExportInfoEn::EIE_SAMPLE_BARCODE));
	checkBoxList.push_back(qMakePair(tr("样本号"), ExportInfoEn::EIE_SAMPLE_SEQ_NO));
	checkBoxList.push_back(qMakePair(tr("样本类别"), ExportInfoEn::EIE_SAMPLE_TYPESTAT));
	//checkBoxList.push_back(qMakePair(tr("架号"), ExportInfoEn::EIE_RACK));
	checkBoxList.push_back(qMakePair(tr("位置"), ExportInfoEn::EIE_POSITION));
	checkBoxList.push_back(qMakePair(tr("校准品名称"), ExportInfoEn::EIE_CALIBRATOR_NAME));
	checkBoxList.push_back(qMakePair(tr("校准品批号"), ExportInfoEn::EIE_CALIBRATOR_LOT));
	checkBoxList.push_back(qMakePair(tr("项目名称"), ExportInfoEn::EIE_ASSAY_NAME));
	checkBoxList.push_back(qMakePair(tr("稀释倍数"), ExportInfoEn::EIE_DILUTION_FACTOR));
	checkBoxList.push_back(qMakePair(tr("计算电动势"), ExportInfoEn::EIE_CALCULATE_EMF));
	checkBoxList.push_back(qMakePair(tr("数据报警"), ExportInfoEn::EIE_DATA_ALARM));
	checkBoxList.push_back(qMakePair(tr("模块"), ExportInfoEn::EIE_MODULE));
	//checkBoxList.push_back(qMakePair(tr("校准水平"), ExportInfoEn::EIE_CALI_LEVEL));
	checkBoxList.push_back(qMakePair(tr("浓度"), ExportInfoEn::EIE_CONC));
	checkBoxList.push_back(qMakePair(tr("单位"), ExportInfoEn::EIE_UNIT));
	checkBoxList.push_back(qMakePair(tr("检测完成时间"), ExportInfoEn::EIE_TEST_FINISHED_TIME));
	checkBoxList.push_back(qMakePair(tr("内部标准液批号"), ExportInfoEn::EIE_IS_LOT));
	checkBoxList.push_back(qMakePair(tr("内部标准液瓶号"), ExportInfoEn::EIE_IS_SN));
	checkBoxList.push_back(qMakePair(tr("AT缓冲液批号"), ExportInfoEn::EIE_DILUENT_LOT));
	checkBoxList.push_back(qMakePair(tr("AT缓冲液瓶号"), ExportInfoEn::EIE_DILUENT_SN));

	// 添加到界面
	AddCheckBoxsToWidget(ui->page_iseCali, checkBoxList);
}

///
/// @brief 显示后初始化
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::InitAfterShow()
{
	// 初始化控件
	InitChildCtrl();

	// 初始化信号槽连接
	InitConnect();
}

///
/// @brief 往目标界面添加checkBox
///
/// @param[in]  spWidget  目标界面
/// @param[in]  checkBoxList  需要添加的checkBox
/// @param[in]  mapTitle  标题映射（行号->标题）
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
/// @li 4170/TangChuXian，2023年12月18日，增加默认参数
///
void ExportSetDlg::AddCheckBoxsToWidget(QWidget* spWidget, const QVector<QPair<QString, ExportInfoEn>>& checkBoxList, 
                                        const QMap<int, GridTitleInfo>& mapTitle)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 参数检查
	if (spWidget == Q_NULLPTR || checkBoxList.isEmpty())
	{
		ULOG(LOG_ERROR, "AddCheckBoxsToWidget Failed");
		return;
	}

	// 获取权限
	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return;
	}

	// 添加栅格布局
	QGridLayout* pLayout = new QGridLayout(spWidget);
	pLayout->setVerticalSpacing(10);
	pLayout->setHorizontalSpacing(10);
    pLayout->setContentsMargins(QMargins(20, 20, 20, 20));
	
	// 添加控件
	int rowIndex = 0;
	int colIndex = 0;
    int colCnt   = LAYOUT_COLUM_MAX_NUM;
    int iTitleShowTime = -1;
	for (const auto&ckbInfo : checkBoxList)
	{
        // 查找当前行是否要显示标题
        auto it = mapTitle.find(rowIndex);
        if (it != mapTitle.end() && !it.value().strTitle.isEmpty() && it.value().iColumnCnt > 0 && it.value().iItemCnt > 0)
        {
            QLabel* pTitleLab = new QLabel(it.value().strTitle);
            pTitleLab->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
            pLayout->addWidget(pTitleLab, rowIndex, 0, 1, LAYOUT_COLUM_MAX_NUM);
            QFont labFont = pTitleLab->font();
            labFont.setBold(true);
            pTitleLab->setFont(labFont);
            pTitleLab->setMinimumHeight(35);
            colCnt = it.value().iColumnCnt;
            iTitleShowTime = it.value().iItemCnt;
            colIndex = 0;
            rowIndex++;
        }

        // 如果文本为空
        if (it != mapTitle.end() && it.value().strTitle.isEmpty() && it.value().iColumnCnt > 0 && it.value().iItemCnt > 0 && colIndex == 0)
        {
            colCnt = it.value().iColumnCnt;
            iTitleShowTime = it.value().iItemCnt;
        }

		QCheckBox* checkBox = new QCheckBox(ckbInfo.first, spWidget);
		checkBox->setProperty(PROPERTY_EXPORT_INFO_EN, ckbInfo.second);
		pLayout->addWidget(checkBox, rowIndex, colIndex);

		// 权限(不显示的隐藏)
		auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(ckbInfo.second);
		if (pSEInfo != nullptr && pLoginUserInfo->type < pSEInfo->m_UserType)
		{
			checkBox->setVisible(false);
			continue;
		}


        // 标题对应复选框个数计数完毕
        if (--iTitleShowTime == 0)
        {
            colIndex = 0;
            rowIndex++;
        }

		colIndex++;
		if (colIndex == colCnt)
		{
			colIndex = 0;
			rowIndex++;
		}
	}

	// 添加弹簧
	QSpacerItem* sPacerItem = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
	pLayout->addItem(sPacerItem, rowIndex+1, 0);
}

///
/// @brief 初始化信号槽
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::InitConnect()
{
	// 确定按钮
	connect(ui->save_btn, &QPushButton::clicked, this, &ExportSetDlg::OnSaveBtnClicked);
	// 取消按钮
	connect(ui->cancel_btn, &QPushButton::clicked, this, [&] {this->reject(); });

	// tab标签改变
	connect(ui->tabWidget, &QTabWidget::tabBarClicked, this, [&](int index) {
		// 生化
		if (index == 0) {
			ui->rabtn_widget->show();
		}
		// 免疫
		else{
			ui->rabtn_widget->hide();
		}
	});

	// 生化界面选择按钮改变
	connect(&m_btnGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
		[=](int id) { ui->stackedWidget->setCurrentIndex(id); });

}

///
/// @brief 初始化子控件
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::InitChildCtrl()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 初始化数据
	InitData();
}

///
/// @brief 初始化数据
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::InitData()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	std::map<ExportType, std::set<ExportInfoEn>> data;
    if (!DictionaryQueryManager::GetExportConfig(data))
    {
        ULOG(LOG_ERROR, "Failed to get exportinfoen config.");
        return;
    }

	// 更新导出设置信息相关数据
	for (auto& dataItem : data)
	{
		switch (dataItem.first)
		{
		// ISE样本&质控
		case ExportType::CH_ISE_SAM_AND_QC_TYPE:
		{
			UpdateWidgetCheckBoxStatus(ui->page_iseSamAndQc, dataItem.second);
		}break;

		// ISE校准
		case ExportType::CH_ISE_CALI_TYPE:
		{
			UpdateWidgetCheckBoxStatus(ui->page_iseCali, dataItem.second);
		}break;

		// 生化常规&质控
		case ExportType::CH_NORMAL_AND_QC_TYPE:
		{
			UpdateWidgetCheckBoxStatus(ui->page_normalAndQc, dataItem.second);
		}break;

		// 生化校准
		case ExportType::CH_CALI_TYPE:
		{
			UpdateWidgetCheckBoxStatus(ui->page_cali, dataItem.second);
		}break;

        // 免疫数据
        case ExportType::IM_ALL_SAMPLE:
        {
            UpdateWidgetCheckBoxStatus(ui->tab_im, dataItem.second);
        }break;

		default:
			break;
		}
	}

}

///
/// @brief 更新目标界面的数据
///
/// @param[in]  spWidget  目标界面
/// @param[in]  dataInfo  数据信息
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::UpdateWidgetCheckBoxStatus(const QWidget* spWidget, const std::set<ExportInfoEn>& dataInfo)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 参数检查
	if (spWidget == Q_NULLPTR )
	{
		ULOG(LOG_ERROR, "UpdateWidgetCheckBoxStatus Failed");
		return;
	}

	if (dataInfo.empty())
	{
		return;
	}

	// 获取子控件
	auto ckeckList = spWidget->findChildren<QCheckBox*>();
	if (ckeckList.isEmpty())
	{
		ULOG(LOG_ERROR, "FindChildren<QCheckBox*> Failed");
		return;
	}

	// 更新按钮
	bool retOK = false;
	int enumInt = -1;
	for (auto& spCheck : ckeckList)
	{
		if (spCheck == Q_NULLPTR)
		{
			continue;
		}
		retOK = false;
		enumInt = -1;
		enumInt = spCheck->property(PROPERTY_EXPORT_INFO_EN).toInt(&retOK);

		// 动态属性转换成功
		if (retOK && enumInt != -1)
		{
			ExportInfoEn target = static_cast<ExportInfoEn>(enumInt);
			if (dataInfo.count(target)>0)
			{
				spCheck->setChecked(true);
			}
			else
			{
				spCheck->setChecked(false);
			}
		}
	}
}

///
/// @brief 保存目标界面的信息
///
/// @param[in]  spWidget  目标界面
/// @param[in]  dataInfo  保存的信息
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
///
void ExportSetDlg::SaveWidgetCheckBoxStatus(const QWidget* spWidget, std::set<ExportInfoEn>& dataInfo)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 参数检查
	if (spWidget == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "SaveWidgetCheckBoxStatus Failed");
		return;
	}

	// 获取子控件
	auto ckeckList = spWidget->findChildren<QCheckBox*>();
	if (ckeckList.isEmpty())
	{
		ULOG(LOG_ERROR, "FindChildren<QCheckBox*> Failed");
		return;
	}

	// 清空数据
	dataInfo.clear();

	// 更新按钮
	bool retOK = false;
	int enumInt = -1;
	for (auto& spCheck : ckeckList)
	{
		// 如果为空或者没有勾选则跳过
		if (spCheck == Q_NULLPTR || !spCheck->isChecked())
		{
			continue;
		}
		retOK = false;
		enumInt = -1;
		enumInt = spCheck->property(PROPERTY_EXPORT_INFO_EN).toInt(&retOK);

		// 动态属性转换成功
		if (retOK && enumInt != -1)
		{
			ExportInfoEn target = static_cast<ExportInfoEn>(enumInt);
			dataInfo.insert(target);
		}
	}

}

///
/// @brief 保存按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
///
void ExportSetDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    std::map<ExportType, std::set<ExportInfoEn>> savedata;
    if (!DictionaryQueryManager::GetExportConfig(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get exportconfig.");
        return;
    }

	// 保存导出设置信息相关数据
	for (auto& dataItem : savedata)
	{
		switch (dataItem.first)
		{
		// ISE样本&质控
		case ExportType::CH_ISE_SAM_AND_QC_TYPE:
		{
			SaveWidgetCheckBoxStatus(ui->page_iseSamAndQc, dataItem.second);
		}break;

		// ISE校准
		case ExportType::CH_ISE_CALI_TYPE:
		{
			SaveWidgetCheckBoxStatus(ui->page_iseCali, dataItem.second);
		}break;

		// 生化常规&质控
		case ExportType::CH_NORMAL_AND_QC_TYPE:
		{
			SaveWidgetCheckBoxStatus(ui->page_normalAndQc, dataItem.second);
		}break;

		// 生化校准
		case ExportType::CH_CALI_TYPE:
		{
			SaveWidgetCheckBoxStatus(ui->page_cali, dataItem.second);
		}break;

        // 免疫
        case ExportType::IM_ALL_SAMPLE:
        {
            SaveWidgetCheckBoxStatus(ui->tab_im, dataItem.second);
        }break;

		default:
			break;
		}
	}

    if (!DictionaryQueryManager::SaveExportConfig(savedata))
    {
        ULOG(LOG_ERROR, "Failed to save export config.");
    }

	// 发送数据更新消息
	POST_MESSAGE(MSG_ID_EXPORT_SET_UPDATE, savedata);

	this->accept();
}

void ExportSetDlg::showEvent(QShowEvent *event)
{
	// 让基类处理事件
	BaseDlg::showEvent(event);

	// 第一次显示时初始化
	if (!m_bInit)
	{
		m_bInit = true;
		InitAfterShow();
	}
}



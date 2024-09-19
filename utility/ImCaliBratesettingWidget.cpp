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
/// @file     ImCaliBratesettingWidget.cpp
/// @brief    项目设置-校准
///
/// @author   1226/ZhangJing
/// @date     2023年2月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "ImCalibrateSettingWidget.h"
#include "ui_ImCalibrateSettingWidget.h"
#include <QComboBox>
#include <QtWidgets/QCheckBox>
#include <QIntValidator>
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "model/CaliVolsTableModel.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "manager/UserInfoManager.h"
#include "src/common/common.h"

#define  INVALID_ASSAY_CODE  (-1)

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

ImCalibrateSettingWidget::ImCalibrateSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImCalibrateSettingWidget)
{
    ui->setupUi(this);
    InitCtrls();
    ui->lot_valid_time_edit->setValidator(new QIntValidator(0, 999999));
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

ImCalibrateSettingWidget::~ImCalibrateSettingWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     加载指定项目的分析参数
///
/// @param[in]  item  数据
///
/// @par History:
/// @li 1226/zhangjing，2023年2月15日，新建函数
///
bool ImCalibrateSettingWidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 复位控件
    Reset();
    m_rowItem = item;

    // 判断待加载项有效
    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        return true;
    }

    // 查询通用项目信息
    auto &CIM = CommonInformationManager::GetInstance();
    auto GAI = CIM->GetAssayInfo(item.assayCode);
    if (GAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
        return false;
    }

    // 查询免疫通用项目信息
    std::shared_ptr<im::tf::GeneralAssayInfo> imGAI = CIM->GetImmuneAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, m_rowItem.version.toStdString());
    if (imGAI == nullptr)
    {
        ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
        return false;
    }

    // 校准结果管理
    // 批校准间隔时间
    ui->lot_valid_time_edit->setText(QString::number(imGAI->lotCaliHours));
    // 校准屏蔽
    ui->cali_failed_ckbox->setChecked(imGAI->autoCaliMask);

    // 自动校准设置
    // 换批
    ui->change_over_lot_ckbox->setChecked(imGAI->autoCaliSet.reagLotCali);
    // 换瓶
    ui->change_over_bottle_ckbox->setChecked(imGAI->autoCaliSet.reagBotCali);

    // 瓶曲线
    ui->bottl_timeout_ckbox->setChecked(imGAI->autoCaliSet.curveBotExp);
    //批曲线
    ui->lot_timeout_ckbox->setChecked(imGAI->autoCaliSet.curveLotExp);

    // 质控失控
    ui->single_qc_cali_mode_ckbox->setChecked(imGAI->autoCaliSet.ctrlFail);
    ui->union_qc_cali_mode_ckbox->setChecked(imGAI->autoCaliSet.uniteCtrlFail);

    // 校准判定参数
    if (imGAI->caliJudge.deltaLimit.size() >= 1)
    {
        ui->lower_delta1_edit->setText(QString::number(imGAI->caliJudge.deltaLimit[0].lower, 'f'));
        ui->upper_delta1_edit->setText(QString::number(imGAI->caliJudge.deltaLimit[0].upper, 'f'));
    }
    if (imGAI->caliJudge.deltaLimit.size() >= 2)
    {
        ui->lower_delta2_edit->setText(QString::number(imGAI->caliJudge.deltaLimit[1].lower, 'f'));
        ui->upper_delta2_edit->setText(QString::number(imGAI->caliJudge.deltaLimit[1].upper, 'f'));
    }
    if (imGAI->caliJudge.kLimit.size() >= 1)
    {
        ui->lower_k_edit->setText(QString::number(imGAI->caliJudge.kLimit[0].lower, 'f'));
        ui->upper_k_edit->setText(QString::number(imGAI->caliJudge.kLimit[0].upper, 'f'));
    }
    if (imGAI->caliJudge.cVBound.size() >= 1)
    {
        ui->CV1Edit->setText(QString::number(imGAI->caliJudge.cVBound[0], 'f'));
    }
    if (imGAI->caliJudge.cVBound.size() >= 2)
    {
        ui->CV2Edit->setText(QString::number(imGAI->caliJudge.cVBound[1], 'f'));
    }

    return true;
}

///
/// @brief
///     保存项目信息
///
///@param[in]   item 项目信息 
///
/// @return   成功返回true  
///
/// @par History:
/// @li 1226/zhangjing，2023年2月17日，新建函数
///
bool ImCalibrateSettingWidget::GetAnalysisParam(const AssayListModel::StAssayListRowItem& item, \
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查询通用项目信息
    auto &CIM = CommonInformationManager::GetInstance();

    bool bOk = false;

    // 校准判定参数	
    im::tf::CaliJudgeInfo ICP;
    ::tf::DoubleRange dr1, dr2, k;
    dr1.__set_lower((ui->lower_delta1_edit->text().isEmpty()) ? 0.0 : ui->lower_delta1_edit->text().toDouble(&bOk));
    OnErr(!bOk && !ui->lower_delta1_edit->text().isEmpty(), tr("值错误： 校准品A线性校准Δ低值（Δ1）"));
    dr1.__set_upper((ui->upper_delta1_edit->text().isEmpty()) ? 0.0 : ui->upper_delta1_edit->text().toDouble(&bOk));
    OnErr(!bOk && !ui->upper_delta1_edit->text().isEmpty(), tr("值错误： 校准品A线性校准Δ高值（Δ1）"));
    OnErr(dr1.lower > dr1.upper, tr("值错误： 校准品A线性校准Δ高值小于低值（Δ1）"));
    ICP.deltaLimit.push_back(dr1);
    dr2.__set_lower((ui->lower_delta2_edit->text().isEmpty()) ? 0.0 : ui->lower_delta2_edit->text().toDouble(&bOk));
    OnErr(!bOk && !ui->lower_delta2_edit->text().isEmpty(), tr("值错误： 校准品B线性校准Δ低值（Δ2）"));
    dr2.__set_upper((ui->upper_delta2_edit->text().isEmpty()) ? 0.0 : ui->upper_delta2_edit->text().toDouble(&bOk));
    OnErr(!bOk && !ui->upper_delta2_edit->text().isEmpty(), tr("值错误： 校准品B线性校准Δ高值（Δ2）"));
    OnErr(dr2.lower > dr2.upper, tr("值错误： 校准品B线性校准Δ高值小于低值（Δ2）"));
    ICP.deltaLimit.push_back(dr2);

    double detlow = (ui->lower_k_edit->text().isEmpty()) ? 0.0 : ui->lower_k_edit->text().toDouble(&bOk);
    OnErr(!bOk && !(ui->lower_k_edit->text().isEmpty()), tr("值错误： 校准品线性校准判定值范围K（=Δ1/Δ2）低值"));
    double dethig = (ui->upper_k_edit->text().isEmpty()) ? 0.0 : ui->upper_k_edit->text().toDouble(&bOk);
    OnErr(!bOk && !(ui->upper_k_edit->text().isEmpty()), tr("值错误： 校准品线性校准判定值范围K（=Δ1/Δ2）高值"));
    OnErr(detlow > dethig, tr("值错误： 校准品线性校准判定值范围K（=Δ1/Δ2）低值不能大于高值"));
    k.__set_lower(detlow);
    k.__set_upper(dethig);
    ICP.kLimit.push_back(k);

    double cv1, cv2;
    cv1 = (ui->CV1Edit->text().isEmpty()) ? 0.0 : ui->CV1Edit->text().toDouble(&bOk);
    OnErr(!bOk && !(ui->CV1Edit->text().isEmpty()), tr("值错误： 校准品A测定值方差范围（CV1）"));
    ICP.cVBound.push_back(cv1);
    cv2 = (ui->CV2Edit->text().isEmpty()) ? 0.0 : ui->CV2Edit->text().toDouble(&bOk);
    OnErr(!bOk && !(ui->CV2Edit->text().isEmpty()), tr("值错误： 校准品A测定值方差范围（CV2）"));
    ICP.cVBound.push_back(cv2);

    // 自动校准参数	
    im::tf::autoCali CS;
    CS.__set_reagLotCali(ui->change_over_lot_ckbox->isChecked());
    CS.__set_reagBotCali(ui->change_over_bottle_ckbox->isChecked());
    // 瓶曲线
    CS.__set_curveBotExp(ui->bottl_timeout_ckbox->isChecked());
    // 批曲线
    CS.__set_curveLotExp(ui->lot_timeout_ckbox->isChecked());
    // 质控失控
    CS.__set_ctrlFail(ui->single_qc_cali_mode_ckbox->isChecked());
    CS.__set_uniteCtrlFail(ui->union_qc_cali_mode_ckbox->isChecked());

    // 校准曲线管理
    int32_t hours = ui->lot_valid_time_edit->text().toInt(&bOk);
    OnErr(!bOk || hours <= 0 || hours > 999999, tr("值错误：成为批曲线时间限必须大于0且小于999999"));
    // 校准屏蔽
    bool caliMask = ui->cali_failed_ckbox->isChecked();

    for (auto info : vecImAssayInfo)
    {
        if (nullptr == info)
        {
            continue;
        }
        if (info->version == m_rowItem.version.toStdString())
        {
            info->__set_caliJudge(ICP);
        }
        info->__set_lotCaliHours(hours);
        info->__set_autoCaliMask(caliMask);
        info->__set_autoCaliSet(CS);
        info->__set_autoCaliActive(true);
    }

    return true;
}

///
/// @brief
///     复位界面
///
/// @par History:
/// @li 1226/zhangjing，2023年2月17日，新建函数
///
void ImCalibrateSettingWidget::Reset()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto children = this->children();

    for (auto child : children)
    {
        auto edit = dynamic_cast<QLineEdit *>(child);
        if (edit != nullptr)
        {
            edit->clear();
            continue;
        }

        auto combo = dynamic_cast<QComboBox *>(child);
        if (combo != nullptr)
        {
            combo->setCurrentIndex(-1);
            continue;
        }

        auto chkbox = dynamic_cast<QCheckBox *>(child);
        if (chkbox != nullptr)
        {
            chkbox->setChecked(false);
            continue;
        }
    }
}

///
/// @brief
///     初始化控件
///
/// @par History:
/// @li 1226/zhangjing，2023年2月17日，新建函数
///
void ImCalibrateSettingWidget::InitCtrls()
{
    // 校准类型下拉菜单
    for (auto box : QWidget::findChildren<QComboBox*>())
    {
        AddTfEnumItemToComBoBox(box, ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_TWO_POINTS);
    }

    SetCtrlsRegExp();

    //connect(ui->caliPointsEdit, &QLineEdit::textChanged, this, &ImCalibrateSettingWidget::OnsetTableViewRows);
}

///
/// @brief 为输入框设置正则表达式
///
/// @par History:
/// @li 1226/zhangjing，2023年2月17日，新建函数
///
void ImCalibrateSettingWidget::SetCtrlsRegExp()
{
    for (auto edit : findChildren<QLineEdit*>())
    {
        if (edit->objectName().endsWith("_valid_time_edit"))
        {
            edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
        }
        else if (edit->objectName().endsWith("_edit") || edit->objectName().endsWith("Edit"))
        {
            edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_FLOAT)));
        }
    }
}

///
/// @brief 获取更改操作细节信息
///
/// @param[in]  strOptLog  返回的日志记录信息
/// @param[in]  spAssayInfoUi  界面修改的项目信息
/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，新建函数
///
void ImCalibrateSettingWidget::GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi)
{
	auto MakeStr = [](QString& strOptLog, const QString strName, const QString strValue)
	{
		strOptLog += " ";
		strOptLog += strName;
		strOptLog += ":";
		strOptLog += strValue;
	};
	// 参数检查
	if (nullptr == spAssayInfoUi)
	{
		ULOG(LOG_WARN, "The input param is nullptr.");
		return;
	}
	// 从CommMgr中获取免疫项目信息
	std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
	std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();
	assayMgr->GetImmuneAssayInfos(vecImAssayInfo, spAssayInfoUi->assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
	auto spAssayInfo = assayMgr->GetAssayInfo(spAssayInfoUi->assayCode);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_WARN, "Get Assay parameter infor error.");
		return;
	}
	double dFactor = 1;
	for (auto& atUnit : spAssayInfoUi->units)
	{
		if (atUnit.isCurrent)
		{
			dFactor = atUnit.factor;
			break;
		}
	}
	QString strOpen(tr("打开")), strClose(tr("关闭")), strCheck(tr("勾选")), strUncheck(tr("取消勾选"));
	bool lotCaliHours = false, autoCaliMask = false, reagBotCali = false, reagLotCali = false, curveBotExp = false, curveLotExp = false,
		ctrlFail = false, uniteCtrlFail = false, deltaLimit1 = false, deltaLimit2 = false, kLimit = false, cVBound1 = false, cVBound2 = false;
	for (auto& atValUi : vecImAssayInfoUi)
	{
		for (auto& atVal : vecImAssayInfo)
		{
			if (atValUi->version != atVal->version)
			{
				continue;
			}
			// 如果修改了成为批曲线时间限制
			if (atValUi->lotCaliHours != atVal->lotCaliHours && !lotCaliHours)
			{
				MakeStr(strOptLog, ui->label_22->text(), QString::number(atValUi->lotCaliHours));
				lotCaliHours = true;
			}
			// 如果修改了校准屏蔽
			if (atValUi->autoCaliMask != atVal->autoCaliMask && !autoCaliMask)
			{
				MakeStr(strOptLog, ui->cali_failed_ckbox->text(), atValUi->autoCaliMask ? strOpen : strClose);
				autoCaliMask = true;
			}
			// 如果设置了试剂更换的换瓶校准
			if (atValUi->autoCaliSet.reagBotCali != atVal->autoCaliSet.reagBotCali && !reagBotCali)
			{
				MakeStr(strOptLog, ui->change_over_bottle_ckbox->text(), atValUi->autoCaliSet.reagBotCali ? strCheck : strUncheck);
				reagBotCali = true;
			}
			// 如果设置了试剂更换的换批校准
			if (atValUi->autoCaliSet.reagLotCali != atVal->autoCaliSet.reagLotCali && !reagLotCali)
			{
				MakeStr(strOptLog, ui->change_over_lot_ckbox->text(), atValUi->autoCaliSet.reagLotCali ? strCheck : strUncheck);
				reagLotCali = true;
			}
			// 如果设置了校准曲线过期的瓶曲线过期
			if (atValUi->autoCaliSet.curveBotExp != atVal->autoCaliSet.curveBotExp && !curveBotExp)
			{
				MakeStr(strOptLog, ui->bottl_timeout_ckbox->text(), atValUi->autoCaliSet.curveBotExp ? strCheck : strUncheck);
				curveBotExp = true;
			}
			// 如果设置了校准曲线过期的批曲线过期
			if (atValUi->autoCaliSet.curveLotExp != atVal->autoCaliSet.curveLotExp && !curveLotExp)
			{
				MakeStr(strOptLog, ui->lot_timeout_ckbox->text(), atValUi->autoCaliSet.curveLotExp ? strCheck : strUncheck);
				curveLotExp = true;
			}
			// 如果设置了质控失控的单质控失控
			if (atValUi->autoCaliSet.ctrlFail != atVal->autoCaliSet.ctrlFail && !ctrlFail)
			{
				MakeStr(strOptLog, ui->single_qc_cali_mode_ckbox->text(), atValUi->autoCaliSet.ctrlFail ? strCheck : strUncheck);
				ctrlFail = true;
			}
			// 如果设置了质控失控的联合质控失控
			if (atValUi->autoCaliSet.uniteCtrlFail != atVal->autoCaliSet.uniteCtrlFail && !uniteCtrlFail)
			{
				MakeStr(strOptLog, ui->union_qc_cali_mode_ckbox->text(), atValUi->autoCaliSet.uniteCtrlFail ? strCheck : strUncheck);
				uniteCtrlFail = true;
			}
			// 如果修改了校准品A线性校准Δ范围值（Δ1）
			if (atValUi->caliJudge.deltaLimit.size() >= 1 && atVal->caliJudge.deltaLimit.size() >= 1 && !deltaLimit1)
			{
				if (!IsEqual(atValUi->caliJudge.deltaLimit[0].lower, atVal->caliJudge.deltaLimit[0].lower) ||
					!IsEqual(atValUi->caliJudge.deltaLimit[0].upper, atVal->caliJudge.deltaLimit[0].upper) )
				{
					MakeStr(strOptLog, ui->label_11->text(), QString::number(atValUi->caliJudge.deltaLimit[0].lower, 'f')+"-"+ QString::number(atValUi->caliJudge.deltaLimit[0].upper, 'f'));
					deltaLimit1 = true;
				}
			}
			// 如果修改了校准品B线性校准Δ范围值（Δ2）
			if (atValUi->caliJudge.deltaLimit.size() >= 2 && atVal->caliJudge.deltaLimit.size() >= 2 && !deltaLimit2)
			{
				if (!IsEqual(atValUi->caliJudge.deltaLimit[1].lower, atVal->caliJudge.deltaLimit[1].lower) ||
					!IsEqual(atValUi->caliJudge.deltaLimit[1].upper, atVal->caliJudge.deltaLimit[1].upper))
				{
					MakeStr(strOptLog, ui->label_12->text(), QString::number(atValUi->caliJudge.deltaLimit[1].lower, 'f') + "-" + QString::number(atValUi->caliJudge.deltaLimit[1].upper, 'f'));
					deltaLimit2 = true;
				}
			}
			// 如果修改了校准品线性校准判定值范围K（=Δ1/Δ2）
			if (atValUi->caliJudge.kLimit.size() >= 1 && atVal->caliJudge.kLimit.size() >= 1 && !kLimit)
			{
				if (!IsEqual(atValUi->caliJudge.kLimit[0].lower, atVal->caliJudge.kLimit[0].lower) ||
					!IsEqual(atValUi->caliJudge.kLimit[0].upper, atVal->caliJudge.kLimit[0].upper))
				{
					MakeStr(strOptLog, ui->label_15->text(), QString::number(atValUi->caliJudge.kLimit[0].lower, 'f') + "-" + QString::number(atValUi->caliJudge.kLimit[0].upper, 'f'));
					kLimit = true;
				}
			}
			// 如果修改了校准品A测定值方差范围（CV1）
			if (atValUi->caliJudge.cVBound.size() >= 1 && atVal->caliJudge.cVBound.size() >= 1 && !cVBound1)
			{
				if (!IsEqual(atValUi->caliJudge.cVBound[0], atVal->caliJudge.cVBound[0]) )
				{
					MakeStr(strOptLog, ui->label_13->text(), QString::number(atValUi->caliJudge.cVBound[0], 'f'));
					cVBound1 = true;
				}
			}
			// 如果修改了校准品B测定值方差范围（CV2）
			if (atValUi->caliJudge.cVBound.size() >= 2 && atVal->caliJudge.cVBound.size() >= 2 && !cVBound2)
			{
				if (!IsEqual(atValUi->caliJudge.cVBound[1], atVal->caliJudge.cVBound[1]))
				{
					MakeStr(strOptLog, ui->label_14->text(), QString::number(atValUi->caliJudge.cVBound[1], 'f'));
					cVBound2 = true;
				}
			}
		}
	}	
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void ImCalibrateSettingWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 成为批曲线的权限
    userPms->IsPermissionShow(PMS_BATCH_CURVL_TIME_LIMIT) ? ui->groupBox_4->show() : ui->groupBox_4->hide();
    ui->groupBox_4->setEnabled(userPms->IsPermisson(PMS_BATCH_CURVL_TIME_LIMIT));

    // 校准参数权限
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_CALIPARAM) ? ui->CalSetting_range->show() : ui->CalSetting_range->hide();
    ui->CalSetting_range->setEnabled(userPms->IsPermisson(PSM_IM_ASSAYSET_CALIPARAM));

    // 校准屏蔽
    userPms->IsPermissionShow(PMS_CALI_SHEILD) ? ui->groupBox_5->show() : ui->groupBox_5->hide();
    ui->groupBox_5->setEnabled(userPms->IsPermisson(PMS_CALI_SHEILD));

    // 自动校准设置
    if (userPms->IsPermissionShow(PMS_SETTINGS_AUTO_CALI))
    {
        ui->CalSetting_change_over->show();
        ui->groupBox_2->show();
        ui->groupBox_3->show();
    }
    else
    {
        ui->CalSetting_change_over->hide();
        ui->groupBox_2->hide();
        ui->groupBox_3->hide();
    }
    bool enableAutoCali = userPms->IsPermisson(PMS_SETTINGS_AUTO_CALI);
    ui->CalSetting_change_over->setEnabled(enableAutoCali);
    ui->groupBox_2->setEnabled(enableAutoCali);
    ui->groupBox_3->setEnabled(enableAutoCali);
}

///
/// @brief
///     设置表格行数
///
/// @par History:
/// @li 1226/zhangjing，2023年2月17日，新建函数
///
void ImCalibrateSettingWidget::OnsetTableViewRows(const QString& text)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (text.toInt() > 0)
    {
        auto caliAspirateVols = CaliVolsTableModel::Instance().GetData();
        caliAspirateVols.resize(text.toInt());
        CaliVolsTableModel::Instance().SetData(caliAspirateVols);
    }
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 1226/zhangjing，2023年4月27日，新建函数
///
void ImCalibrateSettingWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);
}

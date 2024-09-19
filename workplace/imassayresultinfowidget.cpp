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
/// @file     imassayresultinfowidget.cpp
/// @brief    免疫项目结果详情信息界面
///
/// @author   4170/TangChuXian
/// @date     2022年9月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imassayresultinfowidget.h"
#include "ui_imassayresultinfowidget.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "shared/datetimefmttool.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"

#include "manager/UserInfoManager.h"
#include "manager/ResultStatusCodeManager.h"
#include "manager/OperationLogManager.h"

#include <QDate>

ImAssayResultInfoWidget::ImAssayResultInfoWidget(QWidget *parent)
	: QWidget(parent),
	m_bInit(false),
	m_bIsFirst(true),
	m_bIsLastReTest(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::ImAssayResultInfoWidget();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
	Clear();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

ImAssayResultInfoWidget::~ImAssayResultInfoWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     清空界面信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月21日，新建函数
///
void ImAssayResultInfoWidget::Clear()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->ResSpinbox->clear();
    ui->ResUnitLab->clear();
    ui->RawResValLab->clear();

    ui->SigValLab->clear();
	ui->OldSigValLab->clear();
    ui->ManualDiltValLab->clear();
    ui->DiltRatioValLab->clear();
    ui->ModuleValLab->clear();
    ui->CompleteTimeValLab->clear();

    ui->RgntLotValLab->clear();
    ui->RgntBnValLab->clear();
    ui->OpenValidDayValLab->clear();
    ui->RgntValidDayValLab->clear();
    ui->BaseLiqLotValLab->clear();
    ui->ReactionCupLotValLab->clear();
    ui->WashBufLotValLab->clear();
    ui->DiltLotValLab->clear();

    ui->CalLotValLab->clear();
    ui->CalValidDayValLab->clear();
    ui->CurCurveValidDayValLab->clear();
    ui->CurCurveValLab->clear();
    // 0020473: [工作] 结果详情界面未检测的样本结果状态仍显示上一次浏览过完成检测样本的结果状态码 add by chenjianlin 20230624
    ui->result_status->clear();
}


///
/// @brief 更新结果详情显示
///
/// @param[in]  data  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月3日，新建函数
///
void ImAssayResultInfoWidget::UpdateDetailData(const ::im::tf::AssayTestResult& data, bool bVolEditEnable, bool bIsFirst, bool bIsLastReTest)
{
	m_assayResult = data;
	m_bIsFirst = bIsFirst;
	m_bIsLastReTest = bIsLastReTest;

    // 文本红色提醒
    QString strTextRedColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_WARNFONT));
    QString strTextYellowColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_NOTEFONT));

    // 项目完成时间
    QDateTime dateTimeCplt = QDateTime::fromString(QString::fromStdString(data.endTime), UI_DATE_TIME_FORMAT);

    // 当前结果(可编辑)   
    double dConcEdit = 0;
    // 小数点后保留几位小数(默认2位)
    int iPrecision = 2;
    // 获取当前单位与主单位的转化倍率
    double factor = 1.0;
    // 单位
    std::string unitName = data.unit;
    // 项目类型
    tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

    if (CommonInformationManager::IsCalcAssay(data.assayCode))
    {
        auto spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(data.assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            iPrecision = spCalcInfo->decimalDigit;
			unitName = spCalcInfo->resultUnit;
        }
    }
    else
    {
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(data.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            iPrecision = spAssayInfo->decimalPlace;
            classi = spAssayInfo->assayClassify;
            for (const auto& unit : spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    factor = unit.factor;
                    unitName = unit.name;
                    break;
                }
            }
        }
    }

    // 不显示负数浓度
    if (data.__isset.conc)
    {
        ui->RawResValLab->setText(data.conc < 0 ? "" : QString::number(data.conc*factor, 'f', iPrecision));
        try
        {
            dConcEdit = data.concEdit*factor;
            // 结果详情不论小数位数多少都是先用2位默认值进行四舍五入，该处理有误，应先设小数位数。
            ui->ResSpinbox->setDecimals(iPrecision);
            ui->ResSpinbox->setValue(dConcEdit);            
            // 当前结果小于0时认为数值无效，不显示
            if (dConcEdit < -0.000001f)
            {
                ui->ResSpinbox->clear();
            }
        }
        catch (const std::exception&)
        {
            ui->ResSpinbox->clear();
        }
        // 根据使能状态设置结果编辑框是否可用
        ui->ResSpinbox->setEnabled(bVolEditEnable);
    }

	// 单位
	ui->ResUnitLab->setText(QString::fromStdString(unitName));

	// 结果状态码
    std::shared_ptr<ResultStatusCodeManager> rcodeMrg = ResultStatusCodeManager::GetInstance();
	std::string strCodes = "";
    if (data.__isset.resultStatusCodes)
    {
        auto result = CommonInformationManager::GetInstance()->GetImDisplyResultStr(data.assayCode, data);
        rcodeMrg->GetDisplayResCode(data.resultStatusCodes,
            strCodes, result.toStdString(), data.refRangeJudge, data.crisisRangeJudge, data.sampType);
        strCodes = rcodeMrg->GetResCodesDiscrbe(strCodes);       
    }
    ui->result_status->setText(QString::fromStdString(strCodes));

	///< 项目信息
    // 信号值
    if (data.__isset.RLUEdit)
    {        
        ui->SigValLab->setText(data.RLUEdit >= 0 ? QString::number(data.RLUEdit, 'f', 0) : "");
    }
	// 原始信号值
    if (data.__isset.RLU)
    {
        ui->OldSigValLab->setText(data.RLU >= 0 ? QString::number(data.RLU, 'f', 0) : "");
    }
	
	// 经过需求讨论，质控无稀释倍数，显示为空 add by wzx-231018 BUG 21797
	if (data.sampType == ::tf::SampleType::SAMPLE_TYPE_QC)
	{
		ui->ManualDiltValLab->clear();
		ui->DiltRatioValLab->clear();
	}
	else if(data.__isset.preDilutionFactor && data.__isset.dilutionFactor)
	{
		// 手动稀释 0021568: [工作] 未手工稀释的样本结果详情界面“手工稀释倍数”显示为“无” modify by chenjianlin 20230907
		ui->ManualDiltValLab->setText(QString::number(data.preDilutionFactor));
		// 稀释倍数
		ui->DiltRatioValLab->setText(QString::number(data.dilutionFactor));
	}
	// 模块
	auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(data.deviceSN);
	if (spDevice == nullptr)
	{
		ui->ModuleValLab->setText(QString::fromStdString(data.moduleNo));
	}
	else
	{
		ui->ModuleValLab->setText(QString::fromStdString(spDevice->name));
	}
	// 完成时间
	ui->CompleteTimeValLab->setText(ToCfgFmtDateTime(QString::fromStdString(data.endTime)));
	///< 稀释/耗材信息
	// 试剂批号
	ui->RgntLotValLab->setText(QString::fromStdString(data.reagentBatchNo));
	// 试剂瓶号
	ui->RgntBnValLab->setText(QString::fromStdString(data.reagentSerialNo));

	// 失效日期
    QDate dateRgntExpire = QDate::fromString(QString::fromStdString(data.supplyInfo.reagentExpTime), UI_DATE_FORMAT);
    QString strRgntExpireDate = ToCfgFmtDate(QString::fromStdString(data.supplyInfo.reagentExpTime));
    if (dateRgntExpire.isValid() && dateTimeCplt.isValid() && dateRgntExpire <= dateTimeCplt.date())
    {
        strRgntExpireDate = strTextRedColorFmt.arg(strRgntExpireDate);
    }
	ui->RgntValidDayValLab->setText(strRgntExpireDate);

	// 开瓶效期(天)
    if (data.__isset.openBottleExpiryDays)
    {
        QString strOpenBottleExpiryDays = QString::number(data.openBottleExpiryDays);
        if (data.openBottleExpiryDays <= 0)
        {
            strOpenBottleExpiryDays = strTextRedColorFmt.arg(strOpenBottleExpiryDays);
        }
        ui->OpenValidDayValLab->setText(strOpenBottleExpiryDays);
    }	
	// 底物液批号(只显示A)
	ui->BaseLiqLotValLab->setText(QString::fromStdString(data.supplyInfo.substrateABatchNo));
	// 反应杯批号
	ui->ReactionCupLotValLab->setText(QString::fromStdString(data.supplyInfo.assayCupBatchNo));
	// 清洗缓冲液批号
	ui->WashBufLotValLab->setText(QString::fromStdString(data.supplyInfo.cleanFluidBatchNo));
	// 稀释液批号
	ui->DiltLotValLab->setText(QString::fromStdString(data.supplyInfo.diluentBatchNo));
	///< 校准信息
	// 校准品批号
	ui->CalLotValLab->setText(QString::fromStdString(data.caliLot));
	// 校准品失效日期    
    if (!data.caliLot.empty())
    {
        // 失效日期
        QDateTime dateTimeCalExpire = QDateTime::fromString(QString::fromStdString(data.caliExpiryDate), UI_DATE_TIME_FORMAT);
        QString strCalExpireDate = ToCfgFmtDate(QString::fromStdString(data.caliExpiryDate));
        if (dateTimeCalExpire.isValid() && dateTimeCplt.isValid() && dateTimeCalExpire <= dateTimeCplt)
        {
            strCalExpireDate = strTextRedColorFmt.arg(strCalExpireDate);
        }
        ui->CalValidDayValLab->setText(strCalExpireDate);
    }
    else
    {
        ui->CalValidDayValLab->setText("");
    }

	if (data.__isset.caliCurveId && data.caliCurveId > 0)
	{
        // 获取当前曲线有效期
        QString strCaliCurveExpInfo = QString::fromStdString(data.caliCurveExpiryDays);
        QStringList strCalCurveExpLst = strCaliCurveExpInfo.split(":");
        if (strCalCurveExpLst.size() > 1 && strCalCurveExpLst.last().toInt() <= 0)
        {
            strCaliCurveExpInfo = strTextYellowColorFmt.arg(strCaliCurveExpInfo);
        }

		// 当前曲线有效期
		ui->CurCurveValidDayValLab->setText(strCaliCurveExpInfo);
		// 当前曲线
		ui->CurCurveValLab->setText(ToCfgFmtDateTime(QString::fromStdString(data.curCaliTime)));
	}

}

///
/// @brief 设置当前显示的testItem（数据浏览）
///
/// @param[in]  testItem  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月11日，新建函数
///
void ImAssayResultInfoWidget::SetCurrentTestItem(const tf::TestItem& testItem)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	m_currentTestItem = testItem;
}

///
/// @brief 获取当前显示的testItem（数据浏览）
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月14日，新建函数
///
const tf::TestItem& ImAssayResultInfoWidget::GetCurrentTestItem()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	return m_currentTestItem;
}

bool ImAssayResultInfoWidget::Save()
{
	// 和结果值（非原始结果值）比较，若本次未修改，则返回 modify by chenjianlin 20231215
	if (ui->ResSpinbox->value() == m_assayResult.concEdit)
	{
        ULOG(LOG_WARN, "No changed.");
		return false;
	}
	// 获取当前显示单位对于主单位的倍率，修改结果时需要除以倍率
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo;
	std::shared_ptr<::tf::CalcAssayInfo> spCalcInfo;
	// 获取当前单位与主单位的转化倍率
	double factor = 1.0;

	// 获取当前单位，按倍率转换并显示
	if (!CommonInformationManager::IsCalcAssay(m_assayResult.assayCode))
	{
		spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_assayResult.assayCode);
		if (spAssayInfo != Q_NULLPTR)
		{
			for (const auto& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
                    factor = unit.factor;
                    break;
				}
			}
		}
	}

	::tf::ResultLong _return;
	::tf::CurResultUpdate cr;
	cr.__set_resultId(m_assayResult.id);
	cr.__set_testItemId(m_assayResult.testItemId);
	cr.__set_result(ui->ResSpinbox->value() / factor); 
	// 保存前需检查结果状态是否符合规则 规则还不确定 TODO
	cr.__set_resultStatu(m_assayResult.resultStatusCodes);
	cr.__set_bFirstTest(m_bIsFirst);
	cr.__set_bLastReTest(m_bIsLastReTest);
    im::tf::AssayTestResult result;
    if (im::LogicControlProxy::UpdateCurResult(result, cr))
    {
        m_assayResult = result;
        UpdateDetailData(result, !ui->ResSpinbox->isEnabled(), m_bIsFirst, m_bIsLastReTest);
    }
    // 数据被修改，通知上层刷新数据
    emit dataChanged();

	// 添加操作日志 add by chenjianlin 20231215
	COperationLogManager::GetInstance()->AddTestItemOperationLog(tf::OperationType::type::MOD, m_assayResult.seqNo, m_assayResult.barcode, m_assayResult.assayCode);
	return true;
}

bool ImAssayResultInfoWidget::SetCurrentResult()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_assayResult.deviceSN);
	if (deviceInfo == nullptr)
	{
        ULOG(LOG_WARN, "Not exist deviceSN %s.", m_assayResult.deviceSN.c_str());
		return false;
	}

	tf::TestResultKeyInfo testKey;
	testKey.__set_deviceType(deviceInfo->deviceType);
	testKey.__set_assayTestResultId(m_assayResult.id);
	m_currentTestItem.__set_lastTestResultKey(testKey);
	m_currentTestItem.__set_retestConc(m_assayResult.conc);

	if (m_assayResult.__isset.concEdit)
	{
		m_currentTestItem.__set_retestConc(m_assayResult.concEdit);
		m_currentTestItem.__set_retestEditConc(true);
	}

	m_currentTestItem.__set_resultStatusCodes(m_assayResult.resultStatusCodes);
	m_currentTestItem.__set_retestEndTime(m_assayResult.endTime);

	if (!DcsControlProxy::GetInstance()->ModifyTestItemInfo(m_currentTestItem))
	{
        ULOG(LOG_WARN, "Failed to modify testiteminfo.");
		return false;
	}

	return true;
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImAssayResultInfoWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImAssayResultInfoWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void ImAssayResultInfoWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void ImAssayResultInfoWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImAssayResultInfoWidget::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        m_bInit = true;
    }
}

///
/// @bref
///		权限限制
///
/// @par History:
/// @li 8276/huchunli, 2023年8月11日，新建函数
///
void ImAssayResultInfoWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    // 原始值、信号量
    if (userPms->IsPermisson(PMS_RESULT_DETIAL_ORIGINVALUE))
    {
        ui->widget->show();
        ui->widget_oldSignal->show();
    }
    else
    {
        ui->widget->hide();
        ui->widget_oldSignal->hide();
    }
}

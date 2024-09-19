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
/// @file     imqcitemrltdetaildlg.cpp
/// @brief    免疫质控结果详情对话框
///
/// @author   4170/TangChuXian
/// @date     2023年4月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imqcitemrltdetaildlg.h"
#include "ui_imqcitemrltdetaildlg.h"
#include "shared/datetimefmttool.h"
#include "manager/ResultStatusCodeManager.h"
#include "src/leonis/shared/CommonInformationManager.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include <QDate>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#define             LINE_HEIGHT_OF_RLT_STATE_TEXT               (20)            // 结果状态文本行高

ImQcItemRltDetailDlg::ImQcItemRltDetailDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_bCalculated(true),
      m_lItenRltID(-1)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::ImQcItemRltDetailDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

ImQcItemRltDetailDlg::~ImQcItemRltDetailDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     获取单位
///
/// @param[in]  lItemRltId  结果ID
///
/// @return 结果单位
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月27日，新建函数
///
QString ImQcItemRltDetailDlg::GetUnit(long long lItemRltId)
{
    // 如果ID无效，则返回
    if (lItemRltId <= 0)
    {
        ULOG(LOG_INFO, "%s(), m_lItenRltID <= 0", __FUNCTION__);
        return "";
    }

    // 构造查询条件
    im::tf::AssayTestResultQueryCond queryAssaycond;
    im::tf::AssayTestResultQueryResp assayTestResult;
    queryAssaycond.__set_id(lItemRltId);

    if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond) ||
        (assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
        return "";
    }

    // 获取结果
    if (assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_INFO, "%s(), assayTestResult.lstAssayTestResult.empty()", __FUNCTION__);
        return "";
    }
    im::tf::AssayTestResult& stuItemResult = assayTestResult.lstAssayTestResult[0];

    // 定义变量
    return QString::fromStdString(stuItemResult.unit);
}

///
/// @brief
///     设置结果详情ID
///
/// @param[in]  lItemRltId  结果详情ID
/// @param[in]  bCalculate  是否计算
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
/// @li 4170/TangChuXian，2023年8月29日，增加参数bCalculate
///
void ImQcItemRltDetailDlg::SetItemRltDetailId(long long lItemRltId, bool bCalculate /*= true*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_lItenRltID = lItemRltId;
    m_bCalculated = bCalculate;
    UpdateItemRltDetailInfo();
}

///
/// @brief
///     获取结果详情ID
///
/// @return 结果详情ID
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
long long ImQcItemRltDetailDlg::GetItemRltDetailId()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    return m_lItenRltID;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 界面更新，临时隐藏稀释倍数显示，待确定后删除稀释倍数显示控件
    ui->verticalLayout_3->removeItem(ui->horizontalLayout_18);
    ui->verticalLayout_3->removeItem(ui->horizontalLayout_19);
    ui->verticalLayout_3->setContentsMargins(21, 50, 0, 212);

    // 更新结果详情信息
    UpdateItemRltDetailInfo();
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void ImQcItemRltDetailDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->CloseBtn->setText(tr("关闭"));
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("结果详情"));

    // 设置文本编辑框行高
    QTextDocument* pDoc = ui->result_status->document();
    for (auto it = pDoc->begin(); it != pDoc->end(); it = it.next())
    {
        QTextCursor lCursor(it);
        QTextBlockFormat lBlockFormat = lCursor.blockFormat();
        lBlockFormat.setTopMargin(0);
        lBlockFormat.setBottomMargin(LINE_HEIGHT_OF_RLT_STATE_TEXT);
        //lBlockFormat.setLineHeight(LINE_HEIGHT_OF_RLT_STATE_TEXT, QTextBlockFormat::FixedHeight);
        lCursor.setBlockFormat(lBlockFormat);
        ui->result_status->setTextCursor(lCursor);
    }
    ui->result_status->setAlignment(Qt::AlignTop);

    // 隐藏原始信号值
    //ui->RawSigFrame->setVisible(false);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::showEvent(QShowEvent *event)
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

///
/// @brief
///     清空内容
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::ClearContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->ResSpinbox->clear();
    ui->ResUnitLab->clear();

    ui->SigValLab->clear();
    ui->ManualDiltValLab->clear();
    ui->DiltRatioValLab->clear();
    //ui->RawSigValLab->clear();
    ui->ModuleValLab->clear();
    ui->CompleteTimeValLab->clear();

    ui->RgntLotValLab->clear();
    ui->RgntBnValLab->clear();
    ui->OpenValidDayValLab->clear();
    ui->RgntExpiredDateValLab->clear();
    ui->BaseLiqLotValLab->clear();
    ui->ReactionCupLotValLab->clear();
    ui->WashBufLotValLab->clear();
    ui->DiltLotValLab->clear();

    ui->CalLotValLab->clear();
    ui->CaliExpiredDateValLab->clear();
    ui->CurCurveValidDayValLab->clear();
    ui->CurCurveValLab->clear();
}

///
/// @brief
///     更新项目结果详情信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月1日，新建函数
///
void ImQcItemRltDetailDlg::UpdateItemRltDetailInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空内容
    ClearContent();

    // 如果ID无效，则返回
    if (m_lItenRltID <= 0)
    {
        ULOG(LOG_WARN, "Invalid, m_lItenRltID(%lld) <= 0.", m_lItenRltID);
        return;
    }

    // 构造查询条件
    im::tf::AssayTestResultQueryCond queryAssaycond;
    im::tf::AssayTestResultQueryResp assayTestResult;
    queryAssaycond.__set_id(m_lItenRltID);
    if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond) ||
        (assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Failed to query assaytestresult by rltId:%lld.", m_lItenRltID);
        return;
    }
    // 获取结果
    if (assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_INFO, "Empty result, assayTestResult.lstAssayTestResult.empty(), by id:%lld.", m_lItenRltID);
        return;
    }
    im::tf::AssayTestResult& stuItemResult = assayTestResult.lstAssayTestResult[0];

    // 定义变量
    double dConcEdit = 0;                                                           // 当前结果(可编辑)
    int iPrecision = 2;                                                             // 小数点后保留几位小数(默认2位)
    double factor = 1.0;                                                            // 获取当前单位与主单位的转化倍率
    std::string unitName = stuItemResult.unit;                                           // 单位

    // 文本红色提醒
    QString strTextRedColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_WARNFONT));
    QString strTextYellowColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_NOTEFONT));

    // 项目完成时间
    QDateTime dateTimeCplt = QDateTime::fromString(QString::fromStdString(stuItemResult.endTime), UI_DATE_TIME_FORMAT);

    // 获取项目信息
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuItemResult.assayCode);
    if (spAssayInfo != Q_NULLPTR)
    {
        iPrecision = spAssayInfo->decimalPlace;
        for (const tf::AssayUnit& unit : spAssayInfo->units)
        {
            if (unit.isCurrent)
            {
                factor = unit.factor;
                unitName = unit.name;
                break;
            }
        }
    }

    // 当前结果(可编辑)
    QString result = CommonInformationManager::GetInstance()->GetImDisplyResultStr(stuItemResult.assayCode, stuItemResult);
    try
    {
        double dConc = stuItemResult.conc;
        ui->ResSpinbox->setText(QString::number(dConc * factor, 'f', iPrecision));
        // 当前结果小于0时认为数值无效，不显示
        if (dConc <= DBL_EPSILON)
        {
            //ui->ResSpinbox->clear();  mod_tcx 0027895 要求显示结果
        }
    }
    catch (const std::exception&)
    {
        ui->ResSpinbox->clear();
    }

    // 单位
    ui->ResUnitLab->setText(QString::fromStdString(unitName));

    // 结果状态码
    std::string strCodes;
    ResultStatusCodeManager::GetInstance()->GetDisplayResCode(stuItemResult.resultStatusCodes, strCodes, result.toStdString(),
        stuItemResult.refRangeJudge, stuItemResult.crisisRangeJudge, tf::SampleType::type::SAMPLE_TYPE_QC);
    ui->result_status->setText(QString::fromStdString(ResultStatusCodeManager::GetInstance()->GetResCodesDiscrbe(strCodes)));

    // 先移除质控排除计算，再根据勾选状态决定是否添加
    QStringList strRltStateList = ResultStatusCodeManager::GetInstance()->EraseQcExcludeSymble(ui->result_status->toPlainText());

    // 如果要排除计算，则添加
    if (!m_bCalculated)
    {
        QString strQcExludeCodeDscrb = ResultStatusCodeManager::GetInstance()->GetQCExcludeCodeDiscrbe();
        strRltStateList.push_front(strQcExludeCodeDscrb);
    }
    ui->result_status->setText(strRltStateList.join("\n"));

    ///< 项目信息
    // 信号值
    ui->SigValLab->setText(QString::number(stuItemResult.RLU));

    // 原始结果
    //ui->RawSigValLab->setText(QString::number(stuItemResult.RLU));

    // 手动稀释 0021568: [工作] 未手工稀释的样本结果详情界面“手工稀释倍数”显示为“无” modify by chenjianlin 20230907
	ui->ManualDiltValLab->setText(QString::number(stuItemResult.preDilutionFactor));	

    // 稀释倍数
    ui->DiltRatioValLab->setText(QString::number(stuItemResult.dilutionFactor));

    // 架号位置号（删掉了）
    //ui->RackPosNumValLab->setText(QString::fromStdString(stuItemResult.pos));

    // 模块
    // 根据设备序列号获取设备名
    std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(stuItemResult.moduleNo);
    QString strDevName("");
    if (spDevInfo != Q_NULLPTR)
    {
        strDevName = QString::fromStdString(spDevInfo->name);
    }
    ui->ModuleValLab->setText(strDevName);

    // 完成时间
    ui->CompleteTimeValLab->setText(ToCfgFmtDateTime(QString::fromStdString(stuItemResult.endTime)));

    ///< 稀释/耗材信息
    // 试剂批号
    ui->RgntLotValLab->setText(QString::fromStdString(stuItemResult.reagentBatchNo));

    // 试剂瓶号
    ui->RgntBnValLab->setText(QString::fromStdString(stuItemResult.reagentSerialNo));

    // 失效日期
    QString strExpiredDate = QString::fromStdString(stuItemResult.supplyInfo.reagentExpTime).split(" ").front();
    QDate dateRgntExpire = QDate::fromString(strExpiredDate, UI_DATE_FORMAT);
    QString strRgntExpireDate = ToCfgFmtDateTime(strExpiredDate);
    if (dateRgntExpire.isValid() && dateTimeCplt.isValid() && dateRgntExpire <= dateTimeCplt.date())
    {
        strRgntExpireDate = strTextRedColorFmt.arg(strRgntExpireDate);
    }
    ui->RgntExpiredDateValLab->setText(strRgntExpireDate);

    // 开瓶效期(天)
    QString strOpenBottleExpiryDays = QString::number(stuItemResult.openBottleExpiryDays);
    if (stuItemResult.openBottleExpiryDays <= 0)
    {
        strOpenBottleExpiryDays = strTextRedColorFmt.arg(strOpenBottleExpiryDays);
    }
    ui->OpenValidDayValLab->setText(strOpenBottleExpiryDays);

    // 试剂有效期(天)
    //ui->RgntValidDayValLab->setText(QString::number(stuItemResult.reagentExpiryDays));

    // 底物液批号(只显示A)
    ui->BaseLiqLotValLab->setText(QString::fromStdString(stuItemResult.supplyInfo.substrateABatchNo));

    // 反应杯批号
    ui->ReactionCupLotValLab->setText(QString::fromStdString(stuItemResult.supplyInfo.assayCupBatchNo));

    // 清洗缓冲液批号
    ui->WashBufLotValLab->setText(QString::fromStdString(stuItemResult.supplyInfo.cleanFluidBatchNo));

    // 稀释液批号
    ui->DiltLotValLab->setText(QString::fromStdString(stuItemResult.supplyInfo.diluentBatchNo));

    ///< 校准信息
    // 校准品批号
    ui->CalLotValLab->setText(QString::fromStdString(stuItemResult.caliLot));

    // 校准品失效日期
    if (!stuItemResult.caliLot.empty())
    {
        // 失效日期
        QDateTime dateTimeCalExpire = QDateTime::fromString(QString::fromStdString(stuItemResult.caliExpiryDate), UI_DATE_TIME_FORMAT);
        QString strCalExpireDate = ToCfgFmtDate(QString::fromStdString(stuItemResult.caliExpiryDate));
        if (dateTimeCalExpire.isValid() && dateTimeCplt.isValid() && dateTimeCalExpire <= dateTimeCplt)
        {
            strCalExpireDate = strTextRedColorFmt.arg(strCalExpireDate);
        }
        ui->CaliExpiredDateValLab->setText(strCalExpireDate);
    }
    else
    {
        ui->CaliExpiredDateValLab->setText("");
    }

    if (stuItemResult.__isset.caliCurveId && stuItemResult.caliCurveId > 0)
    {
        // 获取当前曲线有效期
        QString strCaliCurveExpInfo = QString::fromStdString(stuItemResult.caliCurveExpiryDays);
        QStringList strCalCurveExpLst = strCaliCurveExpInfo.split(":");
        if (strCalCurveExpLst.size() > 1 && strCalCurveExpLst.last().toInt() <= 0)
        {
            strCaliCurveExpInfo = strTextYellowColorFmt.arg(strCaliCurveExpInfo);
        }

        // 当前曲线有效期
        ui->CurCurveValidDayValLab->setText(strCaliCurveExpInfo);
        // 当前曲线
        ui->CurCurveValLab->setText(ToCfgFmtDateTime(QString::fromStdString(stuItemResult.curCaliTime)));
    }
}

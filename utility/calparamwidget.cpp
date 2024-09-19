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
/// @file     calparamwidget.cpp
/// @brief    应用界面->项目设置界面->校准参数设置界面
///
/// @author   4170/TangChuXian
/// @date     2020年8月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "calparamwidget.h"
#include "ui_calparamwidget.h"
#include "mlog.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_types.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "thrift/DcsControlProxy.h"

CalParamWidget::CalParamWidget(QWidget *parent)
    : QWidget(parent),
    m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::CalParamWidget();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

CalParamWidget::~CalParamWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief 界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年9月8日，新建函数
///
void CalParamWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化校准类型下拉框
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_ONE_POINT);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_TWO_POINTS);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_MULTI_POINTS);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG3P);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG4P);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_LOGIT_LOG5P);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_EXPONENTIAL);
    AddTfEnumItemToComBoBox(ui->CalTypeCombo, ::ch::tf::CalibrationType::CALIBRATION_TYPE_SPLINE);
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化校准参数下拉框
    // 项目校准系数读取时间单位
    AddTfEnumItemToComBoBox(ui->AssayTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_HOUR);
    AddTfEnumItemToComBoBox(ui->AssayTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_DAY);
    AddTfEnumItemToComBoBox(ui->AssayTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_WEEK);

    // 批校准系数读取时间单位
    AddTfEnumItemToComBoBox(ui->LotTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_HOUR);
    AddTfEnumItemToComBoBox(ui->LotTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_DAY);
    AddTfEnumItemToComBoBox(ui->LotTimeUnitCombo, ::tf::TimeUnit::TIME_UNIT_WEEK);
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     清空参数内容
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::ClearText()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空校准检查参数
    ui->OffsetLmtEdit->clear();

    ui->DupLmtRateLineEdit->clear();
    ui->DupLmtAbsLineEdit->clear();

    ui->SensiRangeBlowLineEdit->clear();
    ui->SensiRangeUpLineEdit->clear();

    ui->Std1BlowLineEdit->clear();
    ui->Std1UpLineEdit->clear();

    // 清空校准参数
    ui->CalTypeCombo->setCurrentIndex(0);
    ui->AssayTimeUnitCombo->setCurrentIndex(0);
    ui->LotTimeUnitCombo->setCurrentIndex(0);
    ui->AssayTimeEdit->clear();
    ui->LotTimeEdit->clear();

    // 清空校准吸取量参数
    ui->Cal1OrglSampEdit->clear();
    ui->Cal2OrglSampEdit->clear();
    ui->Cal3OrglSampEdit->clear();
    ui->Cal4OrglSampEdit->clear();
    ui->Cal5OrglSampEdit->clear();
    ui->Cal6OrglSampEdit->clear();
    ui->Cal1MixLiquidEdit->clear();
    ui->Cal2MixLiquidEdit->clear();
    ui->Cal3MixLiquidEdit->clear();
    ui->Cal4MixLiquidEdit->clear();
    ui->Cal5MixLiquidEdit->clear();
    ui->Cal6MixLiquidEdit->clear();
    ui->Cal1DiluentEdit->clear();
    ui->Cal2DiluentEdit->clear();
    ui->Cal3DiluentEdit->clear();
    ui->Cal4DiluentEdit->clear();
    ui->Cal5DiluentEdit->clear();
    ui->Cal6DiluentEdit->clear();
}

///
/// @brief
///     加载指定项目的校准参数
///
/// @param[in]  db_no  指定项目的数据库主键
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::LoadCalParam(long long db_no)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空校准参数
    ClearText();

    // 如果项目数据库库主键无效，则返回
    if (db_no == UI_INVALID_DB_NO)
    {
        ULOG(LOG_INFO, "invalid assay config db no(%d)", db_no);
        return;
    }

    // 查询选中项目配置信息
    // 构造查询条件
    ::ch::tf::GeneralAssayInfoQueryCond qryCond;
    ::ch::tf::GeneralAssayInfoQueryResp qryResp;
    qryCond.__set_id(db_no);

    // 执行查询
    if (!ch::LogicControlProxy::QueryAssayConfigInfo(qryCond, qryResp) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryAssayConfigInfo() failed");
        return;
    }

    // 判断结果是否为空
    if (qryResp.totalCount <= 0)
    {
        ULOG(LOG_INFO, "QueryAssayConfigInfo(), the result empty");
        return;
    }
    /*
    // 更新校准检查参数
    if (qryResp.lstAssayConfigInfos[0].__isset.caliChkParam)
    {
        ui->OffsetLmtEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuOffsetLmtPar.sdLimit));
        ui->DupLmtRateLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuDispersedLmtPar.dupLmtRate));
        ui->DupLmtAbsLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuDispersedLmtPar.dupLmtAbs));
        ui->SensiRangeBlowLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuSensitiveLmtPar.sensiRangeBlow));
        ui->SensiRangeUpLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuSensitiveLmtPar.sensiRangeUp));
        ui->Std1BlowLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuFirstCalLmtPar.std1Blow));
        ui->Std1UpLineEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliChkParam.stuFirstCalLmtPar.std1Up));
    }

    // 更新校准方法
    if (qryResp.lstAssayConfigInfos[0].__isset.calibrationType)
    {
        ui->CalTypeCombo->setCurrentIndex(ui->CalTypeCombo->findData(qryResp.lstAssayConfigInfos[0].calibrationType));
    }

    // 更新校准品个数
    if (qryResp.lstAssayConfigInfos[0].__isset.caliQuantity)
    {
        ui->CalCntEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliQuantity));
    }

    // 更新校准系数读取时间
    if (qryResp.lstAssayConfigInfos[0].__isset.caliFactorRecordTime)
    {
        ui->AssayTimeEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliFactorRecordTime.iAssayValidTime));
        ui->LotTimeEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliFactorRecordTime.iLotValidTime));
        ui->AssayTimeUnitCombo->setCurrentIndex(ui->AssayTimeUnitCombo->findData(qryResp.lstAssayConfigInfos[0].caliFactorRecordTime.enAssayValidTimeUnit));
        ui->LotTimeUnitCombo->setCurrentIndex(ui->LotTimeUnitCombo->findData(qryResp.lstAssayConfigInfos[0].caliFactorRecordTime.enLotValidTimeUnit));
    }

    // 更新校准吸取量
    // 校准品1
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 0)
    {
        // 原始样本
        ui->Cal1OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[0].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[0].__isset.mixLiquid4Test)
        {
            ui->Cal1MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[0].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[0].__isset.diluent)
        {
            ui->Cal1DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[0].diluent / 10.0));
        }
    }

    // 校准品2
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 1)
    {
        // 原始样本
        ui->Cal2OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[1].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[1].__isset.mixLiquid4Test)
        {
            ui->Cal2MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[1].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[1].__isset.diluent)
        {
            ui->Cal2DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[1].diluent / 10.0));
        }
    }

    // 校准品3
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 2)
    {
        // 原始样本
        ui->Cal3OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[2].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[2].__isset.mixLiquid4Test)
        {
            ui->Cal3MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[2].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[2].__isset.diluent)
        {
            ui->Cal3DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[2].diluent / 10.0));
        }
    }

    // 校准品4
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 3)
    {
        // 原始样本
        ui->Cal4OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[3].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[3].__isset.mixLiquid4Test)
        {
            ui->Cal4MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[3].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[3].__isset.diluent)
        {
            ui->Cal4DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[3].diluent / 10.0));
        }
    }

    // 校准品5
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 4)
    {
        // 原始样本
        ui->Cal5OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[4].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[4].__isset.mixLiquid4Test)
        {
            ui->Cal5MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[4].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[4].__isset.diluent)
        {
            ui->Cal5DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[4].diluent / 10.0));
        }
    }

    // 校准品6
    if (qryResp.lstAssayConfigInfos[0].caliAspirateVols.size() > 5)
    {
        // 原始样本
        ui->Cal6OrglSampEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[5].originalSample / 10.0));

        // 混合液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[5].__isset.mixLiquid4Test)
        {
            ui->Cal6MixLiquidEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[5].mixLiquid4Test / 10.0));
        }

        // 稀释液
        if (qryResp.lstAssayConfigInfos[0].caliAspirateVols[5].__isset.diluent)
        {
            ui->Cal6DiluentEdit->setText(QString::number(qryResp.lstAssayConfigInfos[0].caliAspirateVols[5].diluent / 10.0));
        }
    }*/
}

///
/// @brief
///     保存指定项目的校准参数
///
/// @param[in]  db_no  指定项目的数据库主键
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建函数
///
void CalParamWidget::SaveCalParam(long long db_no)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 如果项目数据库库主键无效，则返回
    if (db_no == UI_INVALID_DB_NO)
    {
        ULOG(LOG_INFO, "invalid assay config db no(%d)", db_no);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("Select invalid item, save CAL param failed.")));
        pTipDlg->exec();
        return;
    }

    // 构造项目配置信息
    ::ch::tf::GeneralAssayInfo assayCfgInfo;
    assayCfgInfo.__set_id(db_no);

    // 获取校准检查参数
    ::ch::tf::AssayCaliChkParam stuParam;
    if (ui->OffsetLmtEdit->text().isEmpty() ||
        ui->DupLmtRateLineEdit->text().isEmpty() ||
        ui->DupLmtAbsLineEdit->text().isEmpty() ||
        ui->SensiRangeBlowLineEdit->text().isEmpty() ||
        ui->SensiRangeUpLineEdit->text().isEmpty() ||
        ui->Std1BlowLineEdit->text().isEmpty() ||
        ui->Std1UpLineEdit->text().isEmpty())
    {
        ULOG(LOG_WARN, "Therr is empty param in AssayCalChkParam!");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("The input of assay cal checking param is incomplete.")));
        pTipDlg->exec();
        return;
    }

    stuParam.stuOffsetLmtPar.sdLimit = ui->OffsetLmtEdit->text().toInt();
    stuParam.stuDispersedLmtPar.dupLmtRate = ui->DupLmtRateLineEdit->text().toInt();
    stuParam.stuDispersedLmtPar.dupLmtAbs = ui->DupLmtAbsLineEdit->text().toInt();
    stuParam.stuSensitiveLmtPar.sensiRangeBlow = ui->SensiRangeBlowLineEdit->text().toInt();
    stuParam.stuSensitiveLmtPar.sensiRangeUp = ui->SensiRangeUpLineEdit->text().toInt();
    stuParam.stuFirstCalLmtPar.std1Blow = ui->Std1BlowLineEdit->text().toInt();
    stuParam.stuFirstCalLmtPar.std1Up = ui->Std1UpLineEdit->text().toInt();
    assayCfgInfo.__set_caliChkParam(stuParam);

    // 获取校准方法
    ::ch::tf::CalibrationType::type enCalType = (::ch::tf::CalibrationType::type)ui->CalTypeCombo->currentData().value<int>();
    assayCfgInfo.__set_calibrationType(enCalType);

    // 获取校准品数量
    if (!ui->CalCntEdit->text().isEmpty())
    {
        assayCfgInfo.__set_caliQuantity(ui->CalCntEdit->text().toInt());
    }

    // 获取校准系数读取时间
    if (!ui->AssayTimeEdit->text().isEmpty() && !ui->LotTimeEdit->text().isEmpty())
    {
        ::ch::tf::CaliFactorRecordTime stuCalFactorTime;
        stuCalFactorTime.__set_iAssayValidTime(ui->AssayTimeEdit->text().toInt());
        stuCalFactorTime.__set_iLotValidTime(ui->LotTimeEdit->text().toInt());
        stuCalFactorTime.__set_enAssayValidTimeUnit((::tf::TimeUnit::type)ui->AssayTimeUnitCombo->currentData().toInt());
        stuCalFactorTime.__set_enLotValidTimeUnit((::tf::TimeUnit::type)ui->LotTimeUnitCombo->currentData().toInt());

        assayCfgInfo.__set_caliFactorRecordTime(stuCalFactorTime);
    }

    // 构造校准品吸取量
    std::vector<::ch::tf::SampleAspirateVol> caliAspirateVols;
    auto GetCalSuckVols = [this](std::vector<::ch::tf::SampleAspirateVol>& caliAspirateVols)
    {
        // 校准品1原始样本吸取量为空，则返回失败
        if (ui->Cal1OrglSampEdit->text().isEmpty())
        {
            return false;
        }

        // 更新校准品1吸取量
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal1OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal1MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal1MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal1DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal1DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 校准品2原始样本吸取量为空，则返回
        if (ui->Cal2OrglSampEdit->text().isEmpty())
        {
            return true;
        }

        // 更新校准品2吸取量
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal2OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal2MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal2MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal2DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal2DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 校准品3原始样本吸取量为空，则返回
        if (ui->Cal3OrglSampEdit->text().isEmpty())
        {
            return true;
        }

        // 更新校准品3吸取量
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal3OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal3MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal3MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal3DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal3DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 校准品4原始样本吸取量为空，则返回
        if (ui->Cal4OrglSampEdit->text().isEmpty())
        {
            return true;
        }

        // 更新校准品4吸取量
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal4OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal4MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal4MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal4DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal4DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 校准品5原始样本吸取量为空，则返回
        if (ui->Cal5OrglSampEdit->text().isEmpty())
        {
            return true;
        }

        // 更新校准品5吸取量
        if (!ui->Cal5OrglSampEdit->text().isEmpty())
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal5OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal5MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal5MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal5DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal5DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 校准品6原始样本吸取量为空，则返回
        if (ui->Cal6OrglSampEdit->text().isEmpty())
        {
            return true;
        }

        // 更新校准品6吸取量
        if (!ui->Cal6OrglSampEdit->text().isEmpty())
        {
            ::ch::tf::SampleAspirateVol stuSampleSuck;
            stuSampleSuck.__set_originalSample(ui->Cal6OrglSampEdit->text().toFloat() * 10);

            if (!ui->Cal6MixLiquidEdit->text().isEmpty())
            {
                stuSampleSuck.__set_mixLiquid4Test(ui->Cal6MixLiquidEdit->text().toFloat() * 10);
            }

            if (!ui->Cal6DiluentEdit->text().isEmpty())
            {
                stuSampleSuck.__set_diluent(ui->Cal6DiluentEdit->text().toFloat() * 10);
            }

            // 将元素压入数组
            caliAspirateVols.push_back(std::move(stuSampleSuck));
        }

        // 最终返回成功
        return true;
    };

    // 如果获取校准品吸取量失败，则弹框提示
    if (!GetCalSuckVols(caliAspirateVols))
    {
        ULOG(LOG_WARN, "Therr is empty param in CalSuckParam!");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("The input of CAL aspiration is incomplete.")));
        pTipDlg->exec();
        return;
    }

    // 更新校准品吸取量
    assayCfgInfo.__set_caliAspirateVols(caliAspirateVols);

    // 执行更新配置信息
    if (!ch::LogicControlProxy::ModifyAssayConfigInfo(assayCfgInfo))
    {
        ULOG(LOG_ERROR, "QueryAssayConfigInfo() failed");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("Save CAL param failed.")));
        pTipDlg->exec();
        return;
    }
}

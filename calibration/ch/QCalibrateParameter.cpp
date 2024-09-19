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
/// @file     QCalibrateParameter.h
/// @brief    显示校准参数的详细信息
///
/// @author   5774/WuHongTao
/// @date     2022年2月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCalibrateParameter.h"
#include "ui_QCalibrateParameter.h"
#include <QLabel>
#include "CaliBrateCommom.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

QCalibrateParameter::QCalibrateParameter(QWidget *parent)
    : QPushButton(parent)
{
    ui = new Ui::QCalibrateParameter();
    ui->setupUi(this);

    // 添加初始化参数
    m_labelParameter.push_back(ui->S1abs);
    m_labelParameter.push_back(ui->K);
    m_labelParameter.push_back(ui->A);
    m_labelParameter.push_back(ui->B);
    m_labelParameter.push_back(ui->C);
    m_labelParameter.push_back(ui->L);
    m_labelParameter.push_back(ui->H);
    m_labelParameter.push_back(ui->I);
}

QCalibrateParameter::~QCalibrateParameter()
{
}

///
/// @brief
///     显示校准曲线的参数信息
///
/// @param[out] caliCurve  校准曲线信息
/// @param[in]  group      试剂信息
/// @param[in]  curveType  曲线类型
///
/// @return true代表成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月28日，新建函数
///
bool QCalibrateParameter::ShowCurveParameter(ch::tf::CaliCurve& caliCurve, const ch::tf::ReagentGroup group, const TABLETYPE curveType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 曲线结果
    ::ch::tf::CaliCurveQueryResp ResultQueryCaliCurves;
    // 曲线查询条件
    ::ch::tf::CaliCurveQueryCond ConditionCurveQuery;
    // 设备sn
    ConditionCurveQuery.__set_deviceSN(std::vector<string>(1,group.deviceSN));
    // 项目编号
    ConditionCurveQuery.__set_assayCode(group.assayCode);

    PARAMETERTYPE parameterType = OTHER;
    switch (curveType)
    {
    case LOT:
        // 试剂盒关键信息不能为空
        if (group.reagentKeyInfos.empty())
        {
            ULOG(LOG_ERROR, "%s()", "reagentKeyInfos is empty");
            return false;
        }
        // 设置批号
        ConditionCurveQuery.__set_reagentLot(group.reagentKeyInfos[0].lot);
        ui->title->setText(tr("Lot_parameter"));
        break;
    case CURRENT:
        if (!group.__isset.caliCurveId)
        {
            ULOG(LOG_ERROR, "%s()", "caliCurveId is null");
            return false;
        }
        // 设置曲线Id
        ConditionCurveQuery.__set_id(group.caliCurveId);
        parameterType = USENOW;
        ui->title->setText(tr("Current_parameter"));
        break;
    case BOTTLE:
        if (!group.__isset.caliCurveId || group.caliCurveSource != tf::CurveSource::CURVE_SOURCE_BOTTLE)
        {
            ULOG(LOG_ERROR, "%s()", "no bottle");
            return false;
        }
        // 设置曲线Id
        ConditionCurveQuery.__set_id(group.caliCurveId);
        ui->title->setText(tr("Bottle_parameter"));
        break;
    default:
        // 项目曲线
        ui->title->setText(tr("Project_parameter"));
        break;
    }

    // 查询曲线数据
    ch::c1005::LogicControlProxy::QueryCaliCurve(ResultQueryCaliCurves, ConditionCurveQuery);
    if (ResultQueryCaliCurves.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || ResultQueryCaliCurves.lstCaliCurves.empty())
    {
        ULOG(LOG_INFO, "QueryCaliCurve failed");
        return false;
    }

    caliCurve = ResultQueryCaliCurves.lstCaliCurves[0];
    shared_ptr<ch::ICalibrateCurve> currentCurve = CaliBrateCommom::GetCurve(caliCurve);
    if (currentCurve == nullptr)
    {
        ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
        return false;
    }

    // 显示通用参数
    ShowCaliParameter(currentCurve);
    // 显示特别参数
    ShowCaliCaliGroupInfo(caliCurve, group, parameterType);
    return true;
}

///
/// @brief
///     显示校准参数
///
/// @param[in]  curve  曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
void QCalibrateParameter::ShowCaliParameter(shared_ptr<ch::ICalibrateCurve>& curve)
{
    if (curve == nullptr)
    {
        ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
        return;
    }

    int i = 0;
    for (const auto dataEach : curve->GetModelParameters())
    {
        if (i >= m_labelParameter.size())
        {
            return;
        }

        m_labelParameter[i++]->setText(QString::number(dataEach.second));
    }
}

///
/// @brief
///     显示校准详细信息
///
/// @param[in]  caliCurve  曲线数据
/// @param[in]  group      试剂组数据
/// @param[in]  type       类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
void QCalibrateParameter::ShowCaliCaliGroupInfo(ch::tf::CaliCurve& caliCurve, const ch::tf::ReagentGroup group, PARAMETERTYPE type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    shared_ptr<ch::ICalibrateCurve> currentCurve = CaliBrateCommom::GetCurve(caliCurve);
    if (currentCurve == nullptr)
    {
        ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
        return;
    }

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(caliCurve.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", caliCurve.assayCode);
        return;
    }

    // 当前正在使用的系数
    if (type == USENOW)
    {
        ui->stackedWidget->setCurrentIndex(1);
        // 项目名称
        ui->Assay_name->setText(QString::fromStdString(spAssayInfo->assayName));
        // 登记时间
        ui->Assay_register_time->setText(QString::fromStdString(group.registerTime));
        // 试剂批号
        ui->reagent_lot->setText(QString::fromStdString(caliCurve.reagentLot));
        // 校准方法
        ui->Calibrate_type->setText(CaliBrateCommom::GetCalibrateMethod(currentCurve));
        // 有效时间
        ui->Calibrate_expired->setText(QString::fromStdString(group.expiryTime));
        // 系数来源
        ui->Parameter_source->setText(CaliBrateCommom::GetCaliBrateType(group));
        // 试剂状态
        ui->Assay_status->setText(CaliBrateCommom::GetCaliBrateStatus(group));
        // 试剂类型
        ui->Assay_type->setText(CaliBrateCommom::GetCaliBrateReagentType(group));
        
    }
    // 其他类型
    else if (type == OTHER)
    {
        ui->stackedWidget->setCurrentIndex(0);
        // 校准方法
        ui->Calibrate_method->setText(CaliBrateCommom::GetCalibrateMethod(currentCurve));
        // 校准时间
        ui->Calibrate_method->setText(QString::fromStdString(caliCurve.caliTime));
        // 校准批号
        ui->Calibrate_lot->setText(QString::fromStdString(caliCurve.calibratorLot));
    }
}

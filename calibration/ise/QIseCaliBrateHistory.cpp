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
/// @file     QIseCaliBrateHistory.cpp
/// @brief    校准历史界面
///
/// @author   4170/TangChuXian
/// @date     2022年10月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QIseCaliBrateHistory.h"
#include "ui_QIseCaliBrateHistory.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QCheckBox>
#include <QLabel>
#include <QSpacerItem>
#include <QDateTime>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>

#define _MATH_DEFINES_DEFINED
#include "shared/calhiscurve.h"
#include "shared/QCustomScaleDraw.h"
#include "shared/mccustomqwtmarker.h"
#include "shared/QSerialModel.h"
#include "shared/ReagentCommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/datetimefmttool.h"
#include "shared/FileExporter.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "manager/UserInfoManager.h"

#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"

#include "src/common/Mlog/mlog.h"
#include "SortHeaderView.h"
#include "Serialize.h"
#include "printcom.hpp"
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

Q_DECLARE_METATYPE(::ise::tf::IseModuleInfo);

QIseCaliBrateHistory::QIseCaliBrateHistory(QWidget *parent)
    : QWidget(parent),
    m_CurveMode(nullptr),
    m_caliDataMode(new QSerialModel),
    m_pCalHisCurve(new CalHisCurve()),
    m_pScrollBar(nullptr),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::QIseCaliBrateHistory();
    ui->setupUi(this);
    m_currentIndex = -1;

    // 界面显示前初始化
    InitBeforeShow();
}

QIseCaliBrateHistory::~QIseCaliBrateHistory()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief刷新设备试剂列表
///     
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
QWidget*  QIseCaliBrateHistory::RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    bool bShowModuleSel = false;        // 是否显示选择模块

    m_strDevs.clear();
    // 根据模块依次显示
    for (auto device : devices)
    {
        if (device->moduleCount > 1)
            bShowModuleSel = true;

        m_strDevs.push_back(device->deviceSN);
    }

    ui->groupBox->setVisible(bShowModuleSel);

    // 设备列表为空则返回
    if (m_strDevs.empty())
    {
        ULOG(LOG_INFO, "deivce is empty");
        return this;
    }

    // 不用管几个模块，已经设置过是否显示了，直接设置名称
    ui->moduleA->setText(QString::fromStdString(devices[0]->name) + "A");
    ui->moduleB->setText(QString::fromStdString(devices[0]->name) + "B");
    ui->moduleA->setChecked(true);

    // 刷新列表
    OnRefreshHistory();

    return this;
}

///
/// @brief
///     重新刷新界面
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void QIseCaliBrateHistory::OnRefreshHistory()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设备列表为空则返回
    if (m_strDevs.empty() || m_strDevs.size() > 1)
    {
        ULOG(LOG_INFO, "deivce is error");
        return;
    }

    /*if (!(m_CurveMode->rowCount() > 0 && m_CurveMode->item(0) != nullptr
        && !m_CurveMode->item(0)->text().isEmpty()))
    {*/
        // 清空表格内容
        ClearCurveContent();

        // 查询生化信息表
        //const std::shared_ptr<tf::DeviceInfo>& dv = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
        UpdateCurveView();
    //}

    //if (m_CurveMode->rowCount() > 0 && m_CurveMode->item(0) != nullptr
    //&& !m_CurveMode->item(0)->text().isEmpty())
    //{
    //    ui->AssayNameTbl->setCurrentIndex(m_CurveMode->item(0)->index());
    //    emit ui->AssayNameTbl->clicked(m_CurveMode->item(0)->index());
    //}
    //else
    //{
        ui->AssayNameTbl->setCurrentIndex(QModelIndex());

        emit ui->AssayNameTbl->clicked(QModelIndex());
    //}

    SetPrintAndExportEnabled(false);
}

///
/// @brief 初始化校准量程表格
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月16日，新建函数
///
void QIseCaliBrateHistory::InitCaliDataModel()
{
    ui->CalRltTbl->setModel(m_caliDataMode);
    ui->CalRltTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    // 重新设置页面
    m_caliDataMode->clear();
    QStringList horHeaderTitle;
    horHeaderTitle << tr("校准品1") << tr("校准品2");

    QStringList verHeaderTitle;
    verHeaderTitle << tr("浓度") << tr("电动势") << tr("基准");
    m_caliDataMode->setHorizontalHeaderLabels(horHeaderTitle);
    m_caliDataMode->setVerticalHeaderLabels(verHeaderTitle);
}

///
/// @brief
///     设置显示曲线的详细信息
///
/// @param[in]  curveInfo  曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void QIseCaliBrateHistory::SetCurveDetail(const ise::tf::CaliCurve& curveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int assayCode = GetCurrentItemAssayCode();

    // 获取项目名称、单位、倍率
    if (curveInfo.__isset.id )
    {
        QString comboName = ui->AssayNameTbl->currentIndex().data(Qt::DisplayRole).toString();
        // 设置项目名称
        SetLableData(ui->AssayNameLab, comboName);
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);

        if (spAssay)
        {
            SetLableData(ui->ConcUnitLab, CommonInformationManager::GetInstance()->GetCurrentUnit(spAssay->assayCode));
        }
    }
    else
    {
        SetLableData(ui->AssayNameLab, "");
        SetLableData(ui->ConcUnitLab, "");
    }

    // 设置内部标准液批号
    if (curveInfo.__isset.IS_Lot)
    {
        SetLableData(ui->RgntLotLab, QString::fromStdString(curveInfo.IS_Lot));
    }
    else
    {
        SetLableData(ui->RgntLotLab, "");
    }

    // 设置内部标准液瓶号
    if (curveInfo.__isset.IS_Sn && !curveInfo.IS_Sn.empty())
    {
        SetLableData(ui->RgntBnLab, QString::fromStdString(curveInfo.IS_Sn));
    }
    else
    {
        SetLableData(ui->RgntBnLab, "");
    }

    // 设置校准品批号
    if (curveInfo.__isset.calibratorLot)
    {
        SetLableData(ui->CalLotLab, QString::fromStdString(curveInfo.calibratorLot));
    }
    else
    {
        SetLableData(ui->CalLotLab, "");
    }

    // 设置校准时间
    if (curveInfo.__isset.caliTime)
    {
        SetLableData(ui->CalTimeLab, ToCfgFmtDateTime(QString::fromStdString(curveInfo.caliTime)));
    }
    else
    {
        SetLableData(ui->CalTimeLab, "");
    }

    // 稀释液批号
    if (curveInfo.__isset.diluentLot)
    {
        SetLableData(ui->DiluentLotLab, QString::fromStdString(curveInfo.diluentLot));
    }
    else
    {
        SetLableData(ui->DiluentLotLab, "");
    }

    // 稀释液瓶号
    if (curveInfo.__isset.diluentSn)
    {
        SetLableData(ui->DiluentSnLab, QString::fromStdString(curveInfo.diluentSn));
    }
    else
    {
        SetLableData(ui->DiluentSnLab, "");
    }

    // 校准失败原因
    if (curveInfo.__isset.successed && !curveInfo.successed && curveInfo.__isset.failedReason)
    {
        SetLableData(ui->FailedReason, GetAlarmInfo(assayCode, curveInfo));
        ui->FailedReason->setVisible(true);
    }
    else
    {
        SetLableData(ui->FailedReason, "");
        ui->FailedReason->setVisible(false);
    }
}

///
/// @brief 设置控件数据
///
/// @param[in]  lb  需要设置的控件
/// @param[in]  text  数据内容
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月17日，新建函数
///
void QIseCaliBrateHistory::SetLableData(QLabel * lb, const QString & text)
{
    if (lb == nullptr)
    {
        return;
    }

    int pos = lb->text().indexOf(tr("："));

    if (pos == -1)
    {
        return;
    }

    lb->setText((lb->text().left(pos + 1) + text));
}

///
/// @brief 设置吸光度浓度表
///
/// @param[in]  curveInfo  曲线信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月17日，新建函数
///
void QIseCaliBrateHistory::SetCaliDataModel(const ise::tf::CaliCurve & curveInfo)
{
    InitCaliDataModel();
    bool isShowConn = UserInfoManager::GetInstance()->IsPermisson(PMS_CALI_HISTORY_SHOW_CONC);

    if(curveInfo.__isset.id && curveInfo.id >= 0)
    {
        const auto& coreData = GetCurveDataCore(curveInfo);

        // 汇总各水平的数据
        if (curveInfo.concs.size() == 6)
        {
            int assayCode = GetCurrentItemAssayCode();
            auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
            const auto& cons = GetAssayConc(assayCode, curveInfo.concs);

            if (isShowConn && std::get<0>(cons) != -1 && spAssay)
            {
                m_caliDataMode->SetItem(tr("浓度"), tr("校准品1"), CenterAligmentItem(QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<0>(cons)), 'f', spAssay->decimalPlace)));
                m_caliDataMode->SetItem(tr("浓度"), tr("校准品2"), CenterAligmentItem(QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<1>(cons)), 'f', spAssay->decimalPlace)));
            }
        }

        if (coreData)
        {
            m_caliDataMode->SetItem(tr("电动势"), tr("校准品1"), CenterAligmentItem(QString::number(coreData->lStdEMF.sampleEMF)));
            m_caliDataMode->SetItem(tr("电动势"), tr("校准品2"), CenterAligmentItem(QString::number(coreData->hStdEMF.sampleEMF)));
            m_caliDataMode->SetItem(tr("基准"), tr("校准品1"), CenterAligmentItem(QString::number(coreData->lStdEMF.baseEMF)));
            m_caliDataMode->SetItem(tr("基准"), tr("校准品2"), CenterAligmentItem(QString::number(coreData->hStdEMF.baseEMF)));
        }
    }
    //ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QString QIseCaliBrateHistory::SetCustomAxisInfo_X_Bottom(double value)
{
    if (m_curCaliCurves.size() < (int)value)
    {
        return QString(" \n ");
    }

    // 不管curveMark是否为空，都增加一个换行，可以保持视图不变
    return QString::number(value) + "\n";
}

void QIseCaliBrateHistory::SetPrintAndExportEnabled(bool enabled)
{
    ui->PrintBtn->setEnabled(enabled);
    ui->export_btn->setEnabled(enabled);
}

bool QIseCaliBrateHistory::ExportAssayInfo(QStringList & strExportTextList)
{
    strExportTextList.push_back((tr("项目名称") + "\t" + tr("单位") + "\t" + "模块"));
    auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
    int assayCode = GetCurrentItemAssayCode();
    QString rowData = ui->AssayNameTbl->currentIndex().data(Qt::DisplayRole).toString();
    rowData += "\t" + CommonInformationManager::GetInstance()->GetCurrentUnit(assayCode);

    QString moduleName = QString::fromStdString(spDevice->name);
    if (spDevice->moduleCount > 1)
    {
        if (ui->moduleB->isChecked())
            moduleName += "B";
        else
            moduleName += "A";
    }
    rowData += "\t" + moduleName;
    strExportTextList.push_back(rowData);
    strExportTextList.push_back("");

    return true;
}

bool QIseCaliBrateHistory::ExportCaliInfo(QStringList & strExportTextList)
{
    // 表头数据
    strExportTextList.push_back(tr("校准次序") + "\t" + tr("校准品批号") + "\t" + tr("IC批号") + "\t" + tr("IC瓶号") + "\t" + tr("BS批号") + "\t" +
     tr("BS瓶号") + "\t" + tr("校准时间") + "\t" + tr("校准结果") + "\t" + tr("数据报警") + "\t" + tr("斜率值") + "\t" + tr("校准品1浓度") + "\t" +
     tr("校准品1电动势") + "\t" + tr("基准1浓度") + "\t" + tr("基准1电动势") + "\t" + tr("校准品2浓度") + "\t" + tr("校准品2电动势") + "\t" + tr("基准2浓度") + "\t" + tr("基准2电动势"));

    int seqNo = 1;
    int assayCode = GetCurrentItemAssayCode();
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    
    for (const auto& curve :m_curCaliCurves)
    {
        QString rowData;
        // 获取校准品1和校准品2的浓度信息
        const auto& cons = GetAssayConc(assayCode, curve.concs);
        const auto& coreData = GetCurveDataCore(curve);

        //校准次序
        rowData += QString::number(seqNo);

        //校准品批号
        rowData += "\t" + QString::fromStdString(curve.calibratorLot);

        //内部标准液批号
        rowData += "\t" + QString::fromStdString(curve.IS_Lot);

        //内部标准液瓶号
        rowData += "\t" + QString::fromStdString(curve.IS_Sn);

        //缓冲液批号
        rowData += "\t" + QString::fromStdString(curve.diluentLot);

        //缓冲液瓶号
        rowData += "\t" + QString::fromStdString(curve.diluentSn);

        //校准时间
        rowData += "\t" + QString::fromStdString(curve.caliTime);

        //校准结果
        QString result = curve.successed ? tr("成功") : tr("失败");
        rowData += "\t" + result;

        //数据报警
        rowData += "\t" + GetAlarmInfo(assayCode, curve);

        //斜率值
        if (coreData)
            rowData += "\t" + QString::number(coreData->slope);
        else
            rowData += "\t";

        if (std::get<0>(cons) != -1)
            //校准品1浓度
            rowData += "\t" + QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<0>(cons)), 'f', spAssay->decimalPlace);
        else
            rowData += "\t";

        if (coreData)
        {
            //校准品1电动势
            rowData += "\t" + QString::number(coreData->lStdEMF.sampleEMF);
            //基准1浓度
            rowData += "\t";
            //基准1电动势
            rowData += "\t" + QString::number(coreData->lStdEMF.baseEMF);
        }
        else
        {
            rowData += "\t";
            rowData += "\t";
            rowData += "\t";
        }

        //校准品2浓度
        if (std::get<1>(cons) != -1)
            rowData += "\t" + QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<1>(cons)), 'f', spAssay->decimalPlace);
        else
            rowData += "\t";

        if (coreData)
        {
            //校准品2电动势
            rowData += "\t" + QString::number(coreData->hStdEMF.sampleEMF);
            //基准2浓度
            rowData += "\t";
            //基准2电动势
            rowData += "\t" + QString::number(coreData->hStdEMF.baseEMF);
        }
        else
        {
            rowData += "\t";
            rowData += "\t";
            rowData += "\t";
        }

        strExportTextList.push_back(rowData);
        seqNo++;
    }


    return true;
}

std::tuple<double, double> QIseCaliBrateHistory::GetAssayConc(int assayCode, const std::vector<double>& concs)
{
    double level_1_conc = -1, level_2_conc = -1;

    // 没有6个值，可能不正确
    if (concs.size() == 6)
    {
        if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA)
        {
            level_1_conc = concs[0];
            level_2_conc = concs[3];
        }
        else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K)
        {
            level_1_conc = concs[1];
            level_2_conc = concs[4];
        }
        else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            level_1_conc = concs[2];
            level_2_conc = concs[5];
        }
    }

    return std::tuple<double, double>(level_1_conc, level_2_conc);
}

bool QIseCaliBrateHistory::GetPrintExportInfo(CaliHistoryInfoIse& info)
{
    int seqNo = 1;
    for (const auto& curve : m_curCaliCurves)
    {
        int i = seqNo;
        auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
        int assayCode = GetCurrentItemAssayCode();
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
        if (spAssay == nullptr)
            return false;

        // 项目名称
        info.strName = ui->AssayNameTbl->currentIndex().data().toString().toStdString();

        // 单位
        info.strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(assayCode).toStdString();

        // 位置
        QString moduleName = QString::fromStdString(spDevice->name);
        if (spDevice->moduleCount > 1)
        {
            if (ui->moduleB->isChecked())
                moduleName += "B";
            else
                moduleName += "A";
        }
        info.strPos = moduleName.toStdString();

        // 获取校准品1和校准品2的浓度信息
        const auto& cons = GetAssayConc(assayCode, curve.concs);
        const auto& coreData = GetCurveDataCore(curve);
        IseCaliHisTimesData timesData;

        // 校准品次序
        timesData.strCaliSeqNo = std::to_string(i++);

        //校准品批号
        timesData.strCalibratorLot = curve.calibratorLot;

        //内部标准液批号
        timesData.IsLot = curve.IS_Lot;

        //内部标准液瓶号
        timesData.IsSn = curve.IS_Sn;

        //缓冲液批号
        timesData.strDiluLot = curve.diluentLot;

        //缓冲液瓶号
        timesData.strDiluSn = curve.diluentSn;

        //校准时间
        timesData.strCalibrateDate = curve.caliTime;

        //校准结果
        QString result = curve.successed ? tr("成功") : tr("失败");
        timesData.strCaliResult = result.toStdString();

        //数据报警
        timesData.strAlarm = GetAlarmInfo(assayCode, curve).toStdString();

        //斜率值
        if (coreData)
            timesData.strSlop = QString::number(coreData->slope).toStdString();


        IseCaliHisLvData caliLv1;
        caliLv1.strCalibrator = tr("校准品1").toStdString();
        if (std::get<0>(cons) != -1)
            //校准品1浓度
            caliLv1.strConc = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<0>(cons)), 'f', spAssay->decimalPlace).toStdString();

        if (coreData)
        {
            //校准品1电动势
            caliLv1.strEmf = QString::number(coreData->lStdEMF.sampleEMF).toStdString();
            //校准品1基准1
            caliLv1.strBase = QString::number(coreData->lStdEMF.baseEMF).toStdString();
        }

        IseCaliHisLvData caliLv2;
        caliLv2.strCalibrator = tr("校准品2").toStdString();
        //校准品2浓度
        if (std::get<1>(cons) != -1)
            caliLv2.strConc = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, std::get<1>(cons)), 'f', spAssay->decimalPlace).toStdString();

        if (coreData)
        {
            //校准品2电动势
            caliLv2.strEmf = QString::number(coreData->hStdEMF.sampleEMF).toStdString();
            //校准品2基准2
            caliLv2.strBase = QString::number(coreData->hStdEMF.baseEMF).toStdString();
        }
        timesData.vecCaliLvData.clear();
        timesData.vecCaliLvData.emplace_back(caliLv1);
        timesData.vecCaliLvData.emplace_back(caliLv2);

        info.vecCaliTimesData.emplace_back(timesData);

        ++seqNo;
    }

    return true;
}

QString QIseCaliBrateHistory::GetAlarmInfo(int assayCode, const ise::tf::CaliCurve & curve)
{
    QString alarmInfo;
    // 约定按照Na、K、Cl的顺序放在置报警，并且用 | 分割
    const auto& alarmList = QString::fromStdString(curve.failedReason).split("|");
    int alarmListSize = alarmList.size();

    if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA && alarmListSize > 0)
    {
        alarmInfo = alarmList[0];
    }
    else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K && alarmListSize > 1)
    {
        alarmInfo = alarmList[1];
    }
    else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL && alarmListSize > 2)
    {
        alarmInfo = alarmList[2];
    }
    else
    {
        alarmInfo = ""; // QString::number(core.itemStatus);
    }

    return alarmInfo;
}

///
/// @brief 显示当前图形选中索引的校准数据
///
/// @param[in]  index  图形上的索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void QIseCaliBrateHistory::OnCurrentSelCalHisInfoChanged(int index)
{
    // 图形上的索引需要减1
    --index;
    m_currentIndex = index;

    if (index >= 0 && index < m_curCaliCurves.size())
    {
        SetCaliDataModel(m_curCaliCurves[index]);
        SetCurveDetail(m_curCaliCurves[index]);
    }
    // 范围以外的，直接清空原有数据
    else
    {
        SetCaliDataModel(ise::tf::CaliCurve());
        SetCurveDetail(ise::tf::CaliCurve());

        // 设置显示最后一次曲线
        if (m_curCaliCurves.size() > 0)
        {
            int lastIndex;
            QDateTime lastTime;
            for (int i = 0; i < m_curCaliCurves.size(); i++)
            {
                if (i == 0)
                {
                    lastIndex = i;
                    lastTime = QDateTime::fromString(QString::fromStdString(m_curCaliCurves[i].caliTime), "yyyy-MM-dd hh:mm:ss");
                }
                else
                {
                    QDateTime currentTime = QDateTime::fromString(QString::fromStdString(m_curCaliCurves[i].caliTime), "yyyy-MM-dd hh:mm:ss");
                    if (currentTime > lastTime)
                    {
                        lastIndex = i;
                        lastTime = currentTime;
                    }
                }
            }

            SetCaliDataModel(m_curCaliCurves[lastIndex]);
            SetCurveDetail(m_curCaliCurves[lastIndex]);

            m_pCalHisCurve->SetSelIndex(lastIndex + 1);
        }
    }

    CreateScaleDraw_X(++index);
}

///
/// @brief
///     清空曲线内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void QIseCaliBrateHistory::ClearCurveContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CurveMode == nullptr)
    {
        m_CurveMode = new QToolTipModel(this);

        m_CurveModeProxyModel = new QIseCurveModeFilterDataModule(ui->AssayNameTbl);
        m_CurveModeProxyModel->setSourceModel(m_CurveMode);
        ui->AssayNameTbl->setModel(m_CurveModeProxyModel);
        // 升序 降序 原序
        SortHeaderView *pCurveModeHeader = new SortHeaderView(Qt::Horizontal, ui->AssayNameTbl);
        pCurveModeHeader->setStretchLastSection(true);
        ui->AssayNameTbl->setHorizontalHeader(pCurveModeHeader);
        connect(pCurveModeHeader, &SortHeaderView::SortOrderChanged, this, [this, pCurveModeHeader](int logicIndex, SortHeaderView::SortOrder order) {
            QTableView *view = ui->AssayNameTbl;
            //无效index或NoOrder就设置为默认未排序状态
            if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                SetAscSortByFirstColumn();
            }
            else
            {
                int iSortRole = Qt::DisplayRole;
                m_CurveMode->setSortRole(iSortRole);
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                view->sortByColumn(logicIndex, qOrderFlag);
            }
        });
        //去掉排序三角样式
        ui->AssayNameTbl->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
        //-1则还原model默认顺序
        m_CurveModeProxyModel->sort(-1, Qt::DescendingOrder);

        //ui->AssayNameTbl->setModel(m_CurveMode);
        ui->AssayNameTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->AssayNameTbl->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->AssayNameTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->AssayNameTbl->setSortingEnabled(true);
        ui->AssayNameTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		ui->AssayNameTbl->horizontalHeader()->setMinimumWidth(110);

		ResizeTblColToContent(ui->AssayNameTbl);
	}

    // 重新设置页面
    m_CurveMode->clear();
    QStringList supplyHeaderListString;
    supplyHeaderListString << tr("项目名称");
    m_CurveMode->setRowCount(0);
    m_CurveMode->setHorizontalHeaderLabels(supplyHeaderListString);
}

///
/// @brief
///     更新校准曲线表格数据
///
/// @param[in]  curveInfos  校准曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void QIseCaliBrateHistory::UpdateCurveView()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const IseAssayIndexCodeMaps& assayInfos = CommonInformationManager::GetInstance()->GetIseAssayIndexCodeMaps();

    // 获取当前选中的模块索引，默认为1，除非选择的是2
    int curModuleInx = (ui->moduleB->isChecked() && ui->moduleB->isVisible()) ? 2 : 1;

    ise::tf::IseModuleInfoQueryResp qryResp;
    ise::tf::IseModuleInfoQueryCond qryCond;
    qryCond.__set_deviceSn(m_strDevs);
    qryCond.__set_moduleIndex(curModuleInx);

    ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
    if (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || qryResp.lstIseModuleInfos.empty())
    {
        ULOG(LOG_INFO, "QueryIseModuleInfo failed!");
        return;
    }

    int seq = 0;
    for (const auto& mInfo : qryResp.lstIseModuleInfos)
    {
        if (mInfo.moduleIndex != curModuleInx)
            continue;

        // 依次显示校准曲线的历史的数据信息
        for (const auto& ass : assayInfos)
        {
            // 组合显示项目信息 例子Na(ISE-A-S)
            // 新的显示方式 NA-S（2023/05/31）
            QString assayName = QString::fromStdString(CommonInformationManager::GetInstance()->GetAssayInfo(ass.second->assayCode)->assayName);
            QString comboName = assayName + /*"(ISE-" + GetIseModuleName(mInfo.moduleIndex) +*/ '-' + GetIseSampleName(mInfo.sampleType)/* + ")"*/;
            QStandardItem* item = CenterAligmentItem(comboName);
            item->setData(QVariant::fromValue(mInfo));
            item->setData(QString::number(seq), Qt::UserRole + 2);
            item->setData(ass.second->assayCode, Qt::UserRole + 3);
            m_CurveMode->setItem(seq++, 0, item);
        }
    }
}

///
/// @brief
///     选中曲线--显示曲线
///
/// @param[in]  index  曲线Index
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void QIseCaliBrateHistory::OnSelectCurveShow(QModelIndex index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清理数据
    m_curCaliCurves.clear();
    InitCaliDataModel();
    // 隐藏显示点
    if (m_pCaliSelRing)
        m_pCaliSelRing->detach();
    if (m_pCaliSelTipMarker)
        m_pCaliSelTipMarker->detach();
    
    if (!index.isValid() || index.row() < 0 || index.column() < 0)
    {
        SetCurveDetail(ise::tf::CaliCurve());
        CreateQwtPlotCurveData(std::vector<ise::tf::CaliCurve>{});
        return;
    }

    // 获取当前项目的项目配置参数
    //const QString& str = m_CurveModeProxyModel->itemData(index)[Qt::DisplayRole].toString();
    //int lastPos = str.lastIndexOf('-');
    //if (lastPos == -1)
    //    return;
    //const QString& name = str.left(lastPos);
    //const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(name.toStdString());

    //// 判断初始条件是否满足
    //if (name.isEmpty() || assay == nullptr)
    //{
    //    return;
    //}

    // 获取历史曲线
    ::ise::tf::IseCaliCurveQueryResp qryResp;
    ::ise::tf::IseCaliCurveQueryCond qryCond;
    
    const auto& info = index.data(Qt::UserRole + 1).value<ise::tf::IseModuleInfo>();
    qryCond.__set_deviceSn(std::vector<std::string>{info.deviceSn});
    qryCond.__set_moduleIndex(info.moduleIndex);
    qryCond.__set_sampleType(info.sampleType);
    // 限制记录数
    tf::LimitCond limtidCon;
    limtidCon.__set_offset(0);
    limtidCon.__set_count(MAXHISTORY);

    // 排序条件
    ::tf::OrderByCond orderCnd;
    orderCnd.__set_asc(false);
    orderCnd.__set_fieldName("caliTime");
    qryCond.__set_limit(limtidCon);
    qryCond.orderByConds.push_back(orderCnd);
    ise::ise1005::LogicControlProxy::QueryIseCaliCurve(qryResp, qryCond);

    // 切换项目后确认是否能够点击打印、导出
    SetPrintAndExportEnabled(!qryResp.lstIseCaliCurves.empty());
    // 准备曲线数据
    CreateQwtPlotCurveData(qryResp.lstIseCaliCurves);
}

void QIseCaliBrateHistory::OnAssayUpdated()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设备列表为空则返回
    if (m_strDevs.empty() || m_strDevs.size() > 1)
    {
        ULOG(LOG_INFO, "deivce is error");
        return;
    }

    const auto& sourceMap = m_CurveModeProxyModel->mapToSource(ui->AssayNameTbl->currentIndex());
    auto item = m_CurveMode->item(sourceMap.row());
    QString text;
    if (item != nullptr)
    {
        text = item->text();
    }

    // 清空表格内容
    ClearCurveContent();

    // 查询生化信息表
    //const std::shared_ptr<tf::DeviceInfo>& dv = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
    UpdateCurveView();

    // 确定是否存在已选择的项目
    auto && items = m_CurveMode->findItems(text);
    if (!items.isEmpty())
    {
        const auto& fromSourceIndex = m_CurveModeProxyModel->mapFromSource(items[0]->index());
        ui->AssayNameTbl->setCurrentIndex(fromSourceIndex);
        OnSelectCurveShow(fromSourceIndex);
    }
    else
    {
        ui->AssayNameTbl->setCurrentIndex(QModelIndex());
        OnSelectCurveShow(QModelIndex());
    }
}

void QIseCaliBrateHistory::hideEvent(QHideEvent * event)
{
    SetAscSortByFirstColumn();
    ui->AssayNameTbl->setCurrentIndex(QModelIndex());
    OnSelectCurveShow(QModelIndex());

    QWidget::hideEvent(event);
}

void QIseCaliBrateHistory::SetAscSortByFirstColumn()
{
    // 第一列按位置升序排列
    m_CurveMode->setSortRole(Qt::DisplayRole + 2);
    ui->AssayNameTbl->sortByColumn(0, Qt::AscendingOrder);
    //去掉排序三角样式
    ui->AssayNameTbl->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    //-1则还原model默认顺序
    m_CurveModeProxyModel->sort(-1, Qt::DescendingOrder);
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void QIseCaliBrateHistory::InitBeforeShow()
{
    // 项目名称表
    ClearCurveContent();

    // 校准结果表
    InitCaliDataModel();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void QIseCaliBrateHistory::InitAfterShow()
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
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void QIseCaliBrateHistory::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("校准") + QString(" > ") + tr("校准历史"));

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
    }
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void QIseCaliBrateHistory::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_pScrollBar == nullptr || m_pCalHisCurve == nullptr)
    {
        QVBoxLayout* pHLayout = new QVBoxLayout();
        m_pScrollBar = new QScrollBar();
        m_pScrollBar->setRange(0, 25);
        m_pScrollBar->setSingleStep(1);
        m_pScrollBar->setOrientation(Qt::Horizontal);
        QObject::connect(m_pScrollBar, SIGNAL(valueChanged(int)), m_pCalHisCurve, SLOT(TestRangeChanged(int)));
        pHLayout->setSpacing(10);
        pHLayout->setMargin(0);
        pHLayout->addWidget(m_pCalHisCurve);
        pHLayout->addWidget(m_pScrollBar);
        ui->CalHisCurveWgt->setLayout(pHLayout);
    }
    CreateScaleDraw_X();
    m_pCalHisCurve->SetAxisTitle(QwtPlot::yLeft, "\n" + tr("斜率值") + "\n");
    m_pCalHisCurve->SetAxisColor(QwtPlot::yLeft, QColor(0, 0, 0));
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    // m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    // m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    // m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    // m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 选中效果圆环
    m_pCaliSelRing = new QwtPlotCurve();
    QwtSymbol* pQcSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
    pQcSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/img-qc-graphic-sel-ring.png"));
    m_pCaliSelRing->setSymbol(pQcSelSymbol);

    // 选中标记
    m_pCaliSelTipMarker = new QcGraphicSelTipMaker();

    // 默认不显示校准失败原因
    ui->FailedReason->setVisible(false);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void QIseCaliBrateHistory::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 选择每一行的槽函数
    //connect(ui->AssayNameTbl, SIGNAL(clicked(QModelIndex)), this, SLOT(OnSelectCurveShow(QModelIndex)));
    connect(ui->AssayNameTbl->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        const auto&  selections = selected.indexes();
        if (!selections.empty())
            OnSelectCurveShow(selections[0]);
    });
    connect(m_pCalHisCurve, &CalHisCurve::SigCurrentSelCalHisInfoChanged, this, &QIseCaliBrateHistory::OnCurrentSelCalHisInfoChanged);
    connect(m_pCalHisCurve, &CalHisCurve::SigOnSelectPoint, this, &QIseCaliBrateHistory::OnSelectPos);
    connect(ui->PrintBtn, &QPushButton::clicked, this, &QIseCaliBrateHistory::OnPrint);
    connect(ui->export_btn, &QPushButton::clicked, this, &QIseCaliBrateHistory::OnExport);
    connect(ui->moduleA, &QRadioButton::clicked, [this] {OnRefreshHistory(); });
    connect(ui->moduleB, &QRadioButton::clicked, [this] {OnRefreshHistory(); });
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void QIseCaliBrateHistory::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->export_btn->setVisible(userPms->IsPermisson(PSM_CALI_EXPORT_CURVE));
}

///
/// @brief 创建QwtCurve显示数据
///
/// @param[in]  caliCurves  校准数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void QIseCaliBrateHistory::CreateQwtPlotCurveData(const std::vector<ise::tf::CaliCurve>& caliCurves)
{
    static std::map<int, QVector<QPointF>>  caliHis;  //各水平的记录
    int hisCount = 0;   // 历史记录条数

    // 清理原有数据
    for (auto& his : caliHis)
    {
        his.second.clear();
    }

    // 汇总各水平的数据(成功失败都按照此逻辑)
    for (const auto& curve : caliCurves)
    {
        const auto& coreData = GetCurveDataCore(curve);

        if (coreData)
        {
            QPointF pointF;
            pointF.setX(hisCount + 1);
            pointF.setY(coreData->slope);
            caliHis[1].push_back(pointF);
        }

        ++hisCount;
        m_curCaliCurves.emplace_back(curve);
    }

    auto& legendAttrs = GetLegendAttrs();

    if (!legendAttrs.empty()) {
        for (size_t i = 0; i < legendAttrs.size(); ++i)
        {
            // 如果校准水平不足六个，缺省的部分使用初始值填充
            if (i >= caliHis.size() && legendAttrs[i].curve != nullptr)
                legendAttrs[i].curve->setSamples(QVector<QPointF>{});
            else
                legendAttrs[i].curve->setSamples(caliHis[i+1]);

            legendAttrs[i].curve->attach(m_pCalHisCurve->GetQwtPlot());
        }

        m_pCalHisCurve->SetHisCount(hisCount);
        m_pScrollBar->setValue(0);
        m_pCalHisCurve->TestRangeChanged(0);
    }

    // 默认选择第一个，高亮文字
    if (!caliCurves.empty())
        CreateScaleDraw_X(1);
}

///
/// @brief 创建线条基础数据
///
/// @param[in]  xAxis  x坐标轴位置
/// @param[in]  yAxis  y坐标轴位置
/// @param[in]  brushColor  线条颜色
///
/// @return 曲线对象
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
QwtPlotCurve * QIseCaliBrateHistory::CreateInitQwtPlotCurve(int xAxis, int yAxis, const QColor & brushColor)
{
    QwtPlotCurve* pCaliHisCurve = new QwtPlotCurve();

    pCaliHisCurve->setZ(30);

    // 设置坐标轴
    pCaliHisCurve->setAxes(xAxis, yAxis);

    // 设置曲线点符号
    QwtSymbol* pCircleSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pCircleSymbol->setBrush(QBrush(brushColor));
    pCircleSymbol->setPen(QPen(Qt::NoPen));
    pCircleSymbol->setSize(10);
    pCaliHisCurve->setSymbol(pCircleSymbol);

    // 设置曲线样式为点
    pCaliHisCurve->setStyle(QwtPlotCurve::NoCurve);

    return pCaliHisCurve;
}

///
/// @brief 获取图例数据
///
///
/// @return 图例对象集合
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
QVector<QIseCaliBrateHistory::QLegendAttr>& QIseCaliBrateHistory::GetLegendAttrs()
{
    static std::once_flag onceFlag;
    // 每个校准品水平的图例颜色以及数据点颜色
    static QMap<int, QColor> legendColor{
        { 1, QColor(31, 181, 88) },
        { 2, QColor(69, 159, 255) }/*,
        { 3, QColor(124, 63, 209) },
        { 4, QColor(209, 56, 204) },
        { 5, QColor(67, 74, 222) },
        { 6, QColor(250, 144, 22) }*/
    };

    static QVector<QIseCaliBrateHistory::QLegendAttr> legendAttrs(legendColor.size());

    // 初始化每个校准水平 点的属性
    std::call_once(onceFlag, [&] {
        // 创建显示的曲线（一次创建）
        for (size_t i = 1; i <= legendColor.size(); i++)
        {
            auto& legend = legendAttrs[i - 1];
            legend.curve = CreateInitQwtPlotCurve(QwtPlot::xBottom, i % 2 ? QwtPlot::yLeft : QwtPlot::yRight, legendColor[i]);
            legend.legendTitle = tr("校准品") + QString::number(i);
            legend.legendColor = legendColor[i];
        }
    });
 
    return legendAttrs;
}

///
/// @brief 返回校准曲线对应项目的核心数据
///
/// @param[in]  curve  校准曲线
///
/// @return 曲线核心数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月27日，新建函数
///
boost::optional<ise::tf::CaliDataCore> QIseCaliBrateHistory::GetCurveDataCore(const ise::tf::CaliCurve& curve)
{
    int assayCode= GetCurrentItemAssayCode();
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);

    // 判断初始条件是否满足
    if (assay == nullptr)
    {
        return boost::none;
    }

    // 检查项目是否合理
    if (!curve.__isset.id || !curve.__isset.caliData)
    {
        return boost::optional<ise::tf::CaliDataCore>();
    }
    
    if (assay->assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA)
        return boost::make_optional(curve.caliData.Na);

    if (assay->assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K)
        return boost::make_optional(curve.caliData.K);

    if (assay->assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL)
        return boost::make_optional(curve.caliData.Cl);

    return boost::none;
}

///
/// @brief 返回当前选择项目名称
///
///
/// @return 失败 返回空，否则返回正确名称
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月9日，新建函数
///
QString QIseCaliBrateHistory::GetCurrentItemAssayName()
{
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(GetCurrentItemAssayCode());

    // 检查项目是否合理
    if (assay)
    {
        return QString::fromStdString(assay->assayName);
    }

    return QString();
}

///
/// @brief 返回项目的编号
///
///
/// @return 失败返回-1，否则正确返回
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月9日，新建函数
///
int QIseCaliBrateHistory::GetCurrentItemAssayCode()
{
    const QVariant& userData =  ui->AssayNameTbl->currentIndex().data(Qt::UserRole+3);
    if (!userData.isValid() || userData.toInt() <= 0)
        return-1; 

    return userData.toInt();
}


void QIseCaliBrateHistory::CreateScaleDraw_X(int selIndex)
{
    QCustomScaleDraw* pScaleDrawX = new QCustomScaleDraw(std::bind(&QIseCaliBrateHistory::SetCustomAxisInfo_X_Bottom, this, std::placeholders::_1));
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
        pScaleDrawX->SetSelLabelColor(selIndex, QColor(0xfa, 0x0, 0x0));
        m_pCalHisCurve->GetQwtPlot()->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
    }
}

void QIseCaliBrateHistory::OnShowQwtCurveTip(bool isShow, int level, int curveIndex, const QPoint& pos)
{
    // 隐藏显示点
    m_pCaliSelRing->detach();
    m_pCaliSelTipMarker->detach();
    int assayCode = GetCurrentItemAssayCode();
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);

    // 不需要显示则直接退出
    if (!isShow || spAssay == nullptr)
        return;

    const auto& curve = m_curCaliCurves[curveIndex];
    const auto& coreData = GetCurveDataCore(curve);
    // 没有数据，就没有斜率值，就没有办法描绘点
    if (!coreData)
        return;

    QPointF pointF;
    QString context;
    double conc;
    
    pointF.setX(curveIndex + 1);
    pointF.setY(coreData->slope);
    // 汇总各水平的数据
    if (curve.concs.size() == 6)
    {
        if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA)
        {
            context = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, curve.concs[0 + (level - 1) * 3]), 'f', spAssay->decimalPlace);
        }
        else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K)
        {
            context = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, curve.concs[1 + (level - 1) * 3]), 'f', spAssay->decimalPlace);
        }
        else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            context = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, curve.concs[2 + (level - 1) * 3]), 'f', spAssay->decimalPlace);
        }
    }

    auto plot = m_pCalHisCurve->GetQwtPlot();
    // 显示高亮圆环
    m_pCaliSelRing->setSamples({ pointF });
    m_pCaliSelRing->attach(plot);

    // 显示提示框
    // 构造qwtText
    m_strSelPtTextList.clear();
    const auto& date = ToCfgFmtDate(QString::fromStdString(curve.caliTime));
    const auto& time = ToCfgFmtTime(QString::fromStdString(curve.caliTime));
    m_strSelPtTextList << context << time << date;
    m_pCaliSelTipMarker->SetQcTipContent(m_strSelPtTextList);

    // 根据坐标判断标签位置
    int iAlign = 0;
    if (pos.y() < 100)
    {
        iAlign |= Qt::AlignBottom;
    }
    else
    {
        iAlign |= Qt::AlignTop;
    }

    // 获取左右坐标轴的宽度
    /*int leftAxisWid = 0;
    int rightAxisWid = 0;

    auto yLeftAxisWdgt = plot->axisWidget(QwtPlot::yLeft);
    if (yLeftAxisWdgt)
    leftAxisWid = yLeftAxisWdgt->width();

    QwtScaleWidget *yRightAxisWdgt = plot->axisWidget(QwtPlot::yRight);
    if (yRightAxisWdgt)
    rightAxisWid = yRightAxisWdgt->width();*/

    if (pos.x()  < 120)
    {
        iAlign |= Qt::AlignRight;
    }
    else if (plot->canvas()->width() - pos.x() < 120)
    {
        iAlign |= Qt::AlignLeft;
    }
    else
    {
        iAlign |= Qt::AlignHCenter;
    }

    // 构造marker
    m_pCaliSelTipMarker->setValue(pointF);
    m_pCaliSelTipMarker->setLabelAlignment(Qt::Alignment(iAlign), Qt::AlignCenter);
    m_pCaliSelTipMarker->attach(plot);
}

void QIseCaliBrateHistory::OnSelectPos(const QPoint & pos)
{
    static;
    QVector<QLegendAttr>& legendArr = GetLegendAttrs();
    int level = 1;
    for (const auto& le : legendArr)
    {
        double d;
        if (le.curve)
        {
            // idx 是当前曲线下标索引对应的坐标轴的值
            // 如果曲线中间出现间隔例如(x轴 7后面是10，点击10时，返回的是7）
            int idx = le.curve->closestPoint(pos, &d);
            if (d < 4.5)
            {
                // 显示标记的点和弹框显示记录
                OnShowQwtCurveTip(true, level, m_currentIndex, pos);
                return;
            }
            else
            {
                OnShowQwtCurveTip(false, -1, -1);
            }
        }
        ++level;
    }
}

void QIseCaliBrateHistory::OnExport()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出保存文件对话框
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (fileName.isNull() || fileName.isEmpty())
    {
        //std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
        //pTipDlg->exec();
        return;
    }

    QFileInfo file(fileName);
    bool bRet = true;

    if (file.suffix() == "pdf")
    {
        CaliHistoryInfoIse Info;
        if (!GetPrintExportInfo(Info))
            return;
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        Info.strExportTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliHistoryInfoIse.lrxml";

        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        QStringList strExportTextList;
        // 导出项目信息
        if (!ExportAssayInfo(strExportTextList))
        {
            ULOG(LOG_ERROR, "ExportAssayInfo failed!");
            // 弹框提示导出失败
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
            pTipDlg->exec();
            return;
        }

        // 导出曲线信息
        if (!ExportCaliInfo(strExportTextList))
        {
            ULOG(LOG_ERROR, "ExportCaliInfo failed!");
            // 弹框提示导出失败
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
            pTipDlg->exec();
            return;
        }

        // 导出文件
        FileExporter fileExporter;
        bRet = fileExporter.ExportInfoToFile(strExportTextList, fileName);
    }

    // 弹框提示导出失败
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
    pTipDlg->exec();
}

void QIseCaliBrateHistory::OnPrint()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    CaliHistoryInfoIse Info;
    if (!GetPrintExportInfo(Info))
        return;

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

////////////////////////////////////////////////////////////////////////////
/// @file     QCaliBrateHistory.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QIseCurveModeFilterDataModule::QIseCurveModeFilterDataModule(QObject * parent)
{
}

QIseCurveModeFilterDataModule::~QIseCurveModeFilterDataModule()
{
}

bool QIseCurveModeFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
    // 获取源model
    QToolTipModel* pSouceModel = dynamic_cast<QToolTipModel*>(sourceModel());
    if (pSouceModel == nullptr)
    {
        return false;
    }

    QString leftData = pSouceModel->data(source_left).toString();
    QString rightData = pSouceModel->data(source_right).toString();
    return QString::localeAwareCompare(leftData, rightData) < 0;
}

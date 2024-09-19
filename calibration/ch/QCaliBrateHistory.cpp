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
/// @file     QCaliBrateHistory.cpp
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

#include "QCaliBrateHistory.h"
#include "ui_QCaliBrateHistory.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <QCheckBox>
#include <QLabel>
#include <QSpacerItem>
#include <QDateTime>
#include <QStandardItemModel>
#include <QTableView>
#include <QHBoxLayout>
#include <QFileDialog>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

#include "CaliBrateCommom.h"
#include "SortHeaderView.h"
#include "Serialize.h"
#include "printcom.hpp"

#include "shared/QSerialModel.h"
#include "shared/datetimefmttool.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/calhiscurve.h"
#include "shared/QCustomScaleDraw.h"
#include "shared/mccustomqwtmarker.h"
#include "shared/ReagentCommon.h"
#include "shared/datetimefmttool.h"
#include "shared/FileExporter.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"

#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"


QCaliBrateHistory::QCaliBrateHistory(QWidget *parent)
    : QWidget(parent),
    m_CurveMode(nullptr),
    m_caliDataMode(new QSerialModel),
    m_pCalHisCurve(new CalHisCurve()),
    m_pScrollBar(nullptr),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::QCaliBrateHistory();
    ui->setupUi(this);
    m_currentIndex = -1;

    // 界面显示前初始化
    InitBeforeShow();

    ui->legendLayout->setSpacing(10);
    // 先隐藏校准失败信息
    ui->FailedReason->setVisible(false);
}

QCaliBrateHistory::~QCaliBrateHistory()
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
QWidget*  QCaliBrateHistory::RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    bool bShowModuleSel = false;        // 是否显示选择模块
    m_strDevs.clear();
    // 根据模块依次显示
    for (const auto& device : devices)
    {
        if (device->__isset.groupName && !device->groupName.empty())
            bShowModuleSel = true;

        m_strDevs.push_back(device->deviceSN);
    }

    ui->moduleA->setChecked(true);
    ui->moduleB->setVisible(true);
    ui->groupBox->setVisible(bShowModuleSel);

    // 设备列表为空则返回
    if (m_strDevs.empty())
    {
        ULOG(LOG_INFO, "deivce is empty");
        return this;
     }

    // 排序一次，避免B显示在A前面
    std::sort(devices.begin(), devices.end());

    if (m_strDevs.size() > 1)
    {
        ui->moduleA->setText(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(m_strDevs[0])));
        ui->moduleB->setText(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(m_strDevs[1])));
    }
    else
    {
        ui->moduleA->setText(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(m_strDevs[0])));
        ui->moduleB->setVisible(false);
    }

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
void QCaliBrateHistory::OnRefreshHistory()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设备列表为空则返回
    if (m_strDevs.empty())
    {
        ULOG(LOG_INFO, "deivce is error");
        return;
    }

    int devIndex = 0;
    if (ui->moduleB->isVisible() && ui->moduleB->isChecked() && m_strDevs.size() > 1)
        devIndex = 1;


    if (!(m_CurveMode->rowCount() > 0 && m_CurveMode->item(0) != nullptr 
        && !m_CurveMode->item(0)->text().isEmpty()))
    {
        // 清空表格内容
        ClearCurveContent();

        // 显示项目表
        UpdateCurveView();
    }

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
void QCaliBrateHistory::InitCaliDataModel()
{
    m_caliDataMode->clear();
    ui->CalRltTbl->setModel(m_caliDataMode);
    ui->CalRltTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    // 重新设置页面
    m_caliDataMode->clear();
    QStringList horHeaderTitle;
    horHeaderTitle << tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4") << tr("校准品5") << tr("校准品6");

    QStringList verHeaderTitle;
    verHeaderTitle << tr("吸光度1") << tr("吸光度2");
    if (UserInfoManager::GetInstance()->IsPermisson(PMS_CALI_HISTORY_SHOW_CONC))
    {
        verHeaderTitle << tr("浓度");
    }
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
void QCaliBrateHistory::SetCurveDetail(const ch::tf::CaliCurve& curveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QStringList  infos;

    // 获取项目名称、单位、倍率
    if (curveInfo.__isset.assayCode)
    {
        // 项目名称
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curveInfo.assayCode);
        if (spAssay)
        {
            infos.push_back(std::move(QString::fromStdString(spAssay->assayName)));
            infos.push_back(std::move(CommonInformationManager::GetInstance()->GetCurrentUnit(curveInfo.assayCode)));
        }
    }
    else
    {
        infos.push_back("");
        infos.push_back("");
    }

    // 试剂批号
    if (curveInfo.__isset.reagentLot)
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.reagentLot)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准品批号
    if (curveInfo.__isset.calibratorLot)
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.calibratorLot)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准瓶号
    if (curveInfo.__isset.reagentKeyInfos && !curveInfo.reagentKeyInfos.empty())
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.reagentKeyInfos[0].sn)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准时间
    if (curveInfo.__isset.caliTime)
    {
        infos.push_back(std::move(ToCfgFmtDateTime(QString::fromStdString(curveInfo.caliTime))));
    }
    else
    {
        infos.push_back("");
    }

    // 执行方法
    infos.push_back(CaliBrateCommom::GetCaliBrateMode(curveInfo.caliMode));

    // 先隐藏校准失败信息
    ui->FailedReason->setVisible(false);

    // 校准信息
    {
        // 设置项目名称
        SetLableData(ui->AssayNameLab, infos[0]);
        // 浓度单位
        SetLableData(ui->ConcUnitLab, infos[1]);
        // 设置试剂批号
        SetLableData(ui->RgntLotLab, infos[2]);
        // 设置校准品批号
        SetLableData(ui->CalLotLab, infos[3]);
        // 设置校准瓶号
        SetLableData(ui->RgntBnLab, infos[4]);
        // 设置校准时间
        SetLableData(ui->CalTimeLab, infos[5]);
        // 设置执行方法
        SetLableData(ui->CaliMode, infos[6]);
    }

    // 校准失败信息
    if (curveInfo.__isset.successed && !curveInfo.successed)
    {
        // 失败信息
        QString failReason = "";
        if (curveInfo.__isset.failedReason && !curveInfo.failedReason.empty())
            failReason = QString::fromStdString(curveInfo.failedReason);
        SetLableData(ui->FailedReason, QString::fromStdString(curveInfo.failedReason));
        ui->FailedReason->setVisible(true);
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
void QCaliBrateHistory::SetLableData(QLabel * lb, const QString & text)
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
void QCaliBrateHistory::SetCaliDataModel(const ch::tf::CaliCurve & curveInfo)
{
    InitCaliDataModel();

    bool isShowConn = UserInfoManager::GetInstance()->IsPermisson(PMS_CALI_HISTORY_SHOW_CONC);
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curveInfo.assayCode);
    if (spAssay == nullptr)
        return;

    // 如果是校准成功的数据，则从曲线数据中获取值
    if (curveInfo.successed)
    {
        const shared_ptr<ch::ICalibrateCurve>& currentCurveParamter = CaliBrateCommom::GetCurve(curveInfo);

        if (currentCurveParamter == nullptr)
        {
            return;
        }

        //获取曲线的点
        const auto& info = currentCurveParamter->GetAlgorithmConfigs();
        auto points = info.origPoints;
        // 空白校准(水平1显示新数据，其它水平显示原数据)
        if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
            info.spanedPoints.size() > 1 && points.size() > 1)
        {
            points[0] = info.spanedPoints[0];
        }

        if (!points.empty())
        {
            int col = 1;
            for (auto& pt : points)
            {
                const auto& absorbance_1 = std::get<2>(pt);
                if (!absorbance_1.empty() && curveInfo.caliMode != tf::CaliMode::CALI_MODE_INTELLIGENT)
                {
                    m_caliDataMode->setItem(0, col - 1, CenterAligmentItem(QString::number(absorbance_1[0])));

                    if (absorbance_1.size() >= 2)
                        m_caliDataMode->setItem(1, col - 1, CenterAligmentItem(QString::number(absorbance_1[1])));
                }

                if (isShowConn)
                {
                    const QString& connText1 = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curveInfo.assayCode, std::get<0>(pt)), 'f', spAssay->decimalPlace);
                    m_caliDataMode->setItem(2, col - 1, CenterAligmentItem(connText1));
                }

                ++col;
            }

        }
    }
    // 校准失败的话，从m_assayTestResults中获取数据
    else
    {
        // 循环获取数据
        for (const auto& result : curveInfo.assayTestResults)
        {
            auto r = GetAssayTestResult(result.second);
            if (!r)
                continue;

            if (r->__isset.abs)
            {
                QStandardItem* item = m_caliDataMode->item(0, result.first - 1);

                // 校准品1是否已经填报
                if (item != nullptr && !item->text().isEmpty())
                    m_caliDataMode->setItem(1, result.first - 1, CenterAligmentItem(QString::number(r->abs)));
                else
                    m_caliDataMode->setItem(0, result.first - 1, CenterAligmentItem(QString::number(r->abs)));
            }

            if (isShowConn && r->__isset.conc)
            {
                const QString& connText1 = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curveInfo.assayCode, r->conc), 'f', spAssay->decimalPlace);
                m_caliDataMode->setItem(2, result.first - 1, CenterAligmentItem(connText1));
            }
        }
    }
    

    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QString QCaliBrateHistory::SetCustomAxisInfo_X_Bottom(double value)
{
    if (m_curCaliCurves.size() < (int)value)
    {
        return QString(" \n ");
    }

    // 默认成功的曲线都是瓶曲线
    QString curveMark = "B";

    // 1.如果校准失败了，curveMark为空
    // 2.如果索引大于曲线范围，也为空
    if (/*(failed) || */m_curCaliCurves.size() < (int)value || (int)value < 1)
    {
        curveMark = "";
    }
    else if (!m_curCaliCurves.empty() && m_curCaliCurves.size() >= (int)value)
    {
        auto& curve = m_curCaliCurves[(int)value - 1];
        
        if (!curve.successed)
            curveMark = "";
        else if (curve.caliMode == tf::CaliMode::CALI_MODE_INTELLIGENT)
        {
            curveMark = ("AC");
        }
        else if ((curve.lotCurve == ch::tf::CurveUsage::CURVE_USAGE_MAIN) ||
            (curve.lotCurve == ch::tf::CurveUsage::CURVE_USAGE_STANDBY))
            curveMark = ("L");
    }

    // 不管curveMark是否为空，都增加一个换行，可以保持视图不变
    return QString::number(value) + "\n" + curveMark;
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年10月30日，新建函数
///
void QCaliBrateHistory::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->export_btn->setVisible(userPms->IsPermisson(PSM_CALI_EXPORT_CURVE));
}

///
/// @bref
///		重新加载浓度表格
///
/// @par History:
/// @li 8276/huchunli, 2023年10月30日，新建函数
///
void QCaliBrateHistory::RefreshTableData(int index)
{
    if (index >= 0 && index < m_curCaliCurves.size())
    {
        SetCaliDataModel(m_curCaliCurves[index]);
        SetCurveDetail(m_curCaliCurves[index]);
    }
    // 范围以外的，直接清空原有数据
    else
    {
        SetCaliDataModel(ch::tf::CaliCurve());
        SetCurveDetail(ch::tf::CaliCurve());

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
}

void QCaliBrateHistory::SetPrintAndExportEnabled(bool enabled)
{
    ui->PrintBtn->setEnabled(enabled);
    ui->export_btn->setEnabled(enabled);
}

bool QCaliBrateHistory::ExportAssayInfo(QStringList & strExportTextList)
{
    // 获取当前项目的项目配置参数
    const auto& index = ui->AssayNameTbl->currentIndex();
    const QString& str = m_CurveModeProxyModel->itemData(index)[Qt::DisplayRole].toString();
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(str.toStdString());
    if (assay == nullptr)
    {
        ULOG(LOG_ERROR, "invalid assay, name is %s.", str.toStdString());
        return false;
    }
    strExportTextList << (tr("项目名称") + "\t" + tr("单位") + "\t" + tr("模块"));

    QString rowData;
    // 项目名称
    rowData += QString::fromStdString(assay->assayName) + "\t";

    // 单位
    rowData += CommonInformationManager::GetInstance()->GetCurrentUnit(assay->assayCode) + "\t";

    // 模块
    rowData += QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(m_strDevs[0]));

    strExportTextList.push_back(rowData);

    return true;
}

bool QCaliBrateHistory::ExportCaliInfo(QStringList& strExportTextList)
{
    if (m_curCaliCurves.empty())
        return false;

    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_curCaliCurves[0].assayCode);
    if (assayInfo == nullptr)
        return false;

    for (const auto& curve : m_curCaliCurves)
    {
        // 增加一个空行
        strExportTextList.push_back("");

        // 校准品信息
        {
            QString rowHeader = tr("校准品批号") + "\t" + tr("试剂批号") + "\t" + tr("试剂瓶号") +
            "\t" + tr("校准时间") + "\t" + tr("执行方法") + "\t" + tr("校准结果") + "\t" + tr("数据报警"); 
            strExportTextList.push_back(rowHeader);

            QString rowData;
            //校准品批号
            rowData += QString::fromStdString(curve.calibratorLot);

            //试剂批号
            rowData += "\t" + QString::fromStdString(curve.reagentLot);

            //试剂瓶号
            rowData += "\t" + QString::fromStdString(curve.reagentKeyInfos.empty() ? "" : curve.reagentKeyInfos.front().sn);

            //校准时间
            rowData += "\t" + QString::fromStdString(curve.caliTime);

            //执行方法
            rowData += "\t" + CaliBrateCommom::GetCaliBrateMode(curve.caliMode);

            //校准结果
            QString result = curve.successed ? tr("成功") : tr("失败");
            rowData += "\t" + result;

            //数据报警
            rowData += "\t" + QString::fromStdString(curve.failedReason);
            strExportTextList.push_back(rowData);
        }

        {
            QString levelHeader("\t" + tr("校准品1") + "\t" + tr("校准品2") + "\t" + tr("校准品3") + "\t" + tr("校准品4") + "\t" + tr("校准品5") + "\t" + tr("校准品6"));    // 校准水平
            QString pos(tr("校准品位置"));
            QString firstCupPos(tr("首次杯号"));
            QString firstSameCup(tr("同杯前反应"));
            QString secondCupPos(tr("第二次杯号"));
            QString secondSameCup(tr("同杯前反应"));
            QString abs_1(tr("吸光度1"));
            QString abs_2(tr("吸光度2"));
            QString abs(tr("吸光度均值"));
            QString conc(tr("浓度"));

            if (curve.successed)
            {
                // 校准数据
                shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(curve);
                if (currentCurveParamter == nullptr)
                {
                    ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
                    continue;
                }

                int ctiRow = 1;
                auto cupInfos = std::move(CaliBrateCommom::GetCurveCupInfos(curve));
                const auto& info = currentCurveParamter->GetAlgorithmConfigs();
                auto points = info.origPoints;
                if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
                    info.spanedPoints.size() > 0 && points.size() > 0)
                {
                    points[0] = info.spanedPoints[0];
                }

                for (const auto& curveAbsorb : points)
                {
                    ChCaliHisLvData caliResultData;
                    // 校准水平
                    //caliResultData.strCalibrator = tr("校准品%1").arg(ctiRow);
                    if (cupInfos.find(ctiRow) != cupInfos.end())
                    {
                        // 首次杯号
                        firstCupPos += QString::fromStdString("\t" + cupInfos[ctiRow][0]);
                        // 同杯前反应
                        firstSameCup += QString::fromStdString("\t" + cupInfos[ctiRow][1]);
                        // 第二次杯号
                        secondCupPos += QString::fromStdString("\t" + cupInfos[ctiRow][2]);
                        // 同杯前反应
                        secondSameCup += QString::fromStdString("\t" + cupInfos[ctiRow][3]);
                    }
                    //吸光度1
                    abs_1 += ("\t" + QString::number(std::get<2>(curveAbsorb)[0]));
                    //吸光度2
                    abs_2 += ("\t" + QString::number(std::get<2>(curveAbsorb)[1]));
                    //吸光度均值
                    abs += ("\t" + QString::number(std::get<1>(curveAbsorb)));
                    // 浓度
                    auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(curve.assayCode, std::get<0>(curveAbsorb));
                    conc += ("\t" + QString::number(curFacValue, 'f', assayInfo->decimalPlace));

                    ctiRow++;
                }
            }
            else
            {
                std::map<int64_t, int32_t> uniqueLevelAndAbs;

                for (const auto& result : curve.assayTestResults)
                {
                    auto chResult = GetAssayTestResult(result.second);

                    // 没有测试结果，跳过次数据
                    if (chResult == boost::none)
                    {
                        ULOG(LOG_ERROR, "GetAssayTestResult failed! db id is %lld", result.second);
                        continue;
                    }

                    // 如果校准水平已经处理过，则是第二次的数据
                    if (uniqueLevelAndAbs.find(result.first) != uniqueLevelAndAbs.end())
                    {
                        secondCupPos += "\t" + QString::number(chResult->cupSN);
                        secondSameCup += "\t" + GetPreAssayName(chResult->previousAssayCode);
                        abs_2 += "\t" + QString::number(chResult->abs);
                        uniqueLevelAndAbs[result.first] = std::round((chResult->abs + uniqueLevelAndAbs[result.first])*1.0 / 2);
                    }
                    // 处理首次校准数据
                    else
                    {
                        firstCupPos += "\t" + QString::number(chResult->cupSN);
                        firstSameCup += "\t" + GetPreAssayName(chResult->previousAssayCode);
                        abs_1 += "\t" + QString::number(chResult->abs);
                        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(chResult->assayCode, chResult->conc);
                        conc += "\t" + (chResult->__isset.conc ? QString::number(curFacValue, 'f', assayInfo->decimalPlace) : "");
                        uniqueLevelAndAbs[result.first] = chResult->abs;
                    }
                }

                for (auto& lvAndAbs : uniqueLevelAndAbs)
                {
                    const auto& strAbs = QString::number(lvAndAbs.second);
                    abs += ("\t" + strAbs);
                }
            }

            strExportTextList.push_back(levelHeader);
            strExportTextList.push_back(pos);
            strExportTextList.push_back(firstCupPos);
            strExportTextList.push_back(firstSameCup);
            strExportTextList.push_back(secondCupPos);
            strExportTextList.push_back(secondSameCup);
            strExportTextList.push_back(abs_1);
            strExportTextList.push_back(abs_2);
            strExportTextList.push_back(abs);
            strExportTextList.push_back(conc);
        }
    }

    return true;
}

bool QCaliBrateHistory::GetPrintExportInfo(CaliHistoryInfoCh& info)
{
    int seqNo = 1;
    for (const auto& curve : m_curCaliCurves)
    {
        int i = seqNo;

        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curve.assayCode);
        if (!spAssay)
            continue;

        int decimal = spAssay->decimalPlace;

        // 项目名称
        info.strName = spAssay->assayName;

        // 单位
        info.strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(curve.assayCode).toStdString();

        // 仪器编号
        //info.strDeviceNum = curve.deviceSN;

        // 软件版本
        //info.strSoftVersion = DictionaryQueryManager::GetInstance()->GetSoftwareVersion();
        ChCaliHisTimesData timesData;
        // 校准品次序
        timesData.strCaliSeqNo = to_string(i++);

        // 模块
        info.strModelName = CommonInformationManager::GetDeviceName(curve.deviceSN);

        // 校准品批号
        timesData.strCalibratorLot = curve.calibratorLot;

        // 试剂批号
        timesData.strReagentLot = curve.reagentLot;

        // 试剂瓶号
        timesData.strReagentSN = curve.reagentKeyInfos[0].sn;

        // 校准时间
        timesData.strCalibrateDate = curve.caliTime;

        // 校准结果
        timesData.strCaliResult = (curve.successed ? tr("成功") : tr("失败")).toStdString();

        // 校准方法
        // info.strCaliType = CaliBrateCommom::GetCalibrateType(CaliBrateCommom::GetCurve(*currentCurve)).toStdString();

        // 执行方法
        timesData.strCaliMode = CaliBrateCommom::GetCaliBrateMode(curve.caliMode).toStdString();

        // 数据报警
        timesData.strAlarm = curve.failedReason;

        timesData.vecCaliLvData.clear();
        if (curve.successed)
        {
            // 校准数据
            shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(curve);
            if (currentCurveParamter == nullptr)
            {
                ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
                continue;
            }

            int ctiRow = 1;
            auto cupInfos = std::move(CaliBrateCommom::GetCurveCupInfos(curve));
            const auto& info = currentCurveParamter->GetAlgorithmConfigs();
            auto points = info.origPoints;
            if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
                info.spanedPoints.size() > 0 && points.size() > 0)
            {
                points[0] = info.spanedPoints[0];
            }

            for (const auto& curveAbsorb : points)
            {
                ChCaliHisLvData caliResultData;
                // 校准水平
                caliResultData.strCalibrator = tr("校准品%1").arg(ctiRow).toStdString();
                if (cupInfos.find(ctiRow) != cupInfos.end())
                {
                    // 首次杯号
                    caliResultData.strFirstCup = cupInfos[ctiRow][0];
                    // 同杯前反应
                    caliResultData.strFirstCupPreAssay = cupInfos[ctiRow][1];
                    // 第二次杯号
                    caliResultData.strSecCup = cupInfos[ctiRow][2];
                    // 同杯前反应
                    caliResultData.strSecCupPreAssay = cupInfos[ctiRow][3];
                }
                //吸光度1
                caliResultData.strAbs1 = QString::number(std::get<2>(curveAbsorb)[0]).toStdString();
                //吸光度2
                caliResultData.strAbs2 = QString::number(std::get<2>(curveAbsorb)[1]).toStdString();
                //吸光度均值
                caliResultData.strAvgAbs = QString::number(std::get<1>(curveAbsorb)).toStdString();
                // 浓度
                auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(curve.assayCode, std::get<0>(curveAbsorb));
                caliResultData.strConc = QString::number(curFacValue, 'f', decimal).toStdString();

                timesData.vecCaliLvData.emplace_back(caliResultData);
                ctiRow++;
            }
        }
        // 失败的记录
        else
        {
            std::map<int64_t, int32_t> uniqueLevelAndAbs;
            std::map<int64_t, ChCaliHisLvData> caliResultDatas;

            auto FuncFillData = [&caliResultDatas, &timesData] {
                for (int i = 1; i <= 6; ++i)
                {
                    if (caliResultDatas.find(i) == caliResultDatas.end())
                    {
                        timesData.vecCaliLvData.emplace_back(ChCaliHisLvData());
                        continue;
                    }

                    timesData.vecCaliLvData.emplace_back(caliResultDatas[i]);
                }
            };

            if (curve.assayTestResults.empty())
            {
                FuncFillData();
                continue;
            }

            for (const auto& result : curve.assayTestResults)
            {
                auto chResult = GetAssayTestResult(result.second);

                // 没有测试结果，跳过次数据
                if (chResult == boost::none)
                {
                    ULOG(LOG_ERROR, "GetAssayTestResult failed! db id is %lld", result.second);
                    continue;
                }

                const auto& assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(chResult->assayCode);

                // 如果校准水平已经处理过，则是第二次的数据
                if (uniqueLevelAndAbs.find(result.first) != uniqueLevelAndAbs.end())
                {
                    // 第二次杯号
                    caliResultDatas[result.first].strSecCup = QString::number(chResult->cupSN).toStdString();
                    // 同杯前反应
                    caliResultDatas[result.first].strSecCupPreAssay = GetPreAssayName(chResult->previousAssayCode).toStdString();
                    //吸光度2
                    caliResultDatas[result.first].strAbs2 = QString::number(chResult->abs).toStdString();
                    uniqueLevelAndAbs[result.first] = std::round((chResult->abs + uniqueLevelAndAbs[result.first])*1.0 / 2);
                    // 浓度
                    if (chResult->__isset.conc)
                    {
                        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(chResult->assayCode, chResult->conc);
                        caliResultDatas[result.first].strConc = QString::number(curFacValue, 'f', decimal).toStdString();
                    }
                    //吸光度均值
                    caliResultDatas[result.first].strAvgAbs = QString::number(std::round(uniqueLevelAndAbs[result.first])).toStdString();
                }
                // 处理首次校准数据
                else
                {
                    // 首次杯号
                    caliResultDatas[result.first].strFirstCup = QString::number(chResult->cupSN).toStdString();
                    // 同杯前反应
                    caliResultDatas[result.first].strFirstCupPreAssay = GetPreAssayName(chResult->previousAssayCode).toStdString();
                    //吸光度1
                    caliResultDatas[result.first].strAbs1 = QString::number(chResult->abs).toStdString();
                    // 浓度
                    if (chResult->__isset.conc)
                    {
                        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(chResult->assayCode, chResult->conc);
                        caliResultDatas[result.first].strConc = QString::number(curFacValue, 'f', decimal).toStdString();
                    }
                    uniqueLevelAndAbs[result.first] = chResult->abs;
                    //吸光度均值(失败时，防止空值)
                    caliResultDatas[result.first].strAvgAbs = QString::number(std::round(uniqueLevelAndAbs[result.first])).toStdString();
                }
            }

            FuncFillData();
        }
        
        info.grpTimesData.emplace_back(timesData);

        ++seqNo;
    }
    
    return true;
}

///
/// @brief 显示当前图形选中索引的校准数据
///
/// @param[in]  index  图形上的索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void QCaliBrateHistory::OnCurrentSelCalHisInfoChanged(int index)
{
    // 图形上的索引需要减1
    --index;
    m_currentIndex = index;

    RefreshTableData(index);

    CreateScaleDraw_X(++index);
}

///
/// @brief
///     清空曲线内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void QCaliBrateHistory::ClearCurveContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CurveMode == nullptr)
    {
        m_CurveMode = new QToolTipModel(this);
        m_CurveModeProxyModel = new QCurveModeFilterDataModule(ui->AssayNameTbl);
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

		ui->AssayNameTbl->verticalHeader()->hide();
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
void QCaliBrateHistory::UpdateCurveView()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const auto& CIM = CommonInformationManager::GetInstance();

    // 查询生化已启用的普通项目
    std::vector<std::shared_ptr<ch::tf::GeneralAssayInfo>> chGAIs;
    auto chAssayIndexCodeMaps = CIM->GetChAssayIndexCodeMaps();
    for (const auto& each : chAssayIndexCodeMaps)
    {
        if (each.second->enable)
        {
            chGAIs.push_back(each.second);
        }
    }

    // 依次显示校准曲线的历史的数据信息
    int seq = 0;
    QSet<int> assayCodeSet;
    for (const auto& chGAI : chGAIs)
    {
        // 查询通用项目表
        auto GAI = CIM->GetAssayInfo(chGAI->assayCode);
        if (GAI == nullptr)
        {
            ULOG(LOG_ERROR, "Can not find assayInfo ,assaycode :%d", chGAI->assayCode);
            continue;
        }

        // 唯一项目编号确认
        if (assayCodeSet.contains(GAI->assayCode))
            continue;

        // 不显示血清指数(S.IND)相关信息
        if (GAI->assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
            continue;
        
        QStandardItem* item = CenterAligmentItem(QString::fromStdString(GAI->assayName));
        item->setData(QString::number(seq));
        m_CurveMode->setItem(seq++, 0, item);

        assayCodeSet.insert(GAI->assayCode);
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
void QCaliBrateHistory::OnSelectCurveShow(QModelIndex index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清理原有数据和图形
    m_curCaliCurves.clear();
    InitCaliDataModel();
    SetCurveDetail(ch::tf::CaliCurve());
    CreateQwtPlotCurveData(std::vector<ch::tf::CaliCurve>{});

    // 隐藏显示点
    if (m_pCaliSelRing)
        m_pCaliSelRing->detach();
    if (m_pCaliSelTipMarker)
        m_pCaliSelTipMarker->detach();
    
    if (!index.isValid() || index.row() < 0 || index.column() < 0)
    {
        return;
    }

    // 获取当前项目的项目配置参数
    const QString& str = m_CurveModeProxyModel->itemData(index)[Qt::DisplayRole].toString();
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(str.toStdString());

    // 判断初始条件是否满足
    if (str.isEmpty() || assay == nullptr)
    {
        return;
    }

    int devIndex = 0;
    if (ui->moduleB->isVisible() && ui->moduleB->isChecked() && m_strDevs.size() > 1)
        devIndex = 1;

    // 曲线结果
    ::ch::tf::CaliCurveQueryResp ResultQueryCaliCurves;
    tf::LimitCond limtidCon;
    limtidCon.__set_offset(0);
    limtidCon.__set_count(MAXHISTORY);
    // 曲线查询条件
    ::ch::tf::CaliCurveQueryCond ConditionCurveQuery;
    ::tf::OrderByCond orderCnd;
    orderCnd.__set_asc(false);
    orderCnd.__set_fieldName("caliTime");

    ConditionCurveQuery.__set_deviceSN( (m_strDevs.size() > devIndex) ? std::vector<std::string>({m_strDevs[devIndex]}) : m_strDevs);
    ConditionCurveQuery.__set_limit(limtidCon);
    ConditionCurveQuery.__set_assayCode(assay->assayCode);
    ConditionCurveQuery.__set_isEditCurve(false);
    ConditionCurveQuery.orderByConds.push_back(orderCnd);

    // 查询曲线数据
    ch::c1005::LogicControlProxy::QueryCaliCurve(ResultQueryCaliCurves, ConditionCurveQuery);

    if (ResultQueryCaliCurves.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)// || ResultQueryCaliCurves.lstCaliCurves.empty())
    {
        ULOG(LOG_INFO, "QueryCaliCurve failed");
        return;
    }

    // 切换项目后确认是否能够点击打印、导出
    SetPrintAndExportEnabled(!ResultQueryCaliCurves.lstCaliCurves.empty());

    // 准备曲线数据
    CreateQwtPlotCurveData(ResultQueryCaliCurves.lstCaliCurves);
}

void QCaliBrateHistory::OnAssayUpdated()
{
    if (m_strDevs.empty())
    {
        ULOG(LOG_INFO, "deivce is empty");
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

    // 显示项目表
    UpdateCurveView();

    // 确定是否存在已选择的项目
    auto && items = m_CurveMode->findItems(text);
    if (!items.isEmpty())
    {
        const auto& fromSourceIndex = m_CurveModeProxyModel->mapFromSource(items[0]->index());
        ui->AssayNameTbl->setCurrentIndex(sourceMap);
        OnSelectCurveShow(sourceMap);
    }
    else
    {
        ui->AssayNameTbl->setCurrentIndex(QModelIndex());
        OnSelectCurveShow(QModelIndex());
    }
}

void QCaliBrateHistory::hideEvent(QHideEvent * event)
{
    SetAscSortByFirstColumn();
    ui->AssayNameTbl->setCurrentIndex(QModelIndex());
    OnSelectCurveShow(QModelIndex());

    QWidget::hideEvent(event);
}

void QCaliBrateHistory::SetAscSortByFirstColumn()
{
    // 第一列按位置升序排列
    m_CurveMode->setSortRole(Qt::DisplayRole + 1);
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
void QCaliBrateHistory::InitBeforeShow()
{
    // 项目名称表
    ClearCurveContent();

    // 校准结果表
    InitCaliDataModel();

    // 初始化信号槽连接
    InitConnect();

    // 设置图例信息
    SetCurveLegend(GetLegendAttrs());

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
void QCaliBrateHistory::InitAfterShow()
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
void QCaliBrateHistory::showEvent(QShowEvent* event)
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
void QCaliBrateHistory::InitChildCtrl()
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
    m_pCalHisCurve->SetAxisTitle(QwtPlot::yLeft, "\n" + tr("吸光度") + "\n");
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
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void QCaliBrateHistory::InitConnect()
{
    // 选择每一行的槽函数
    //connect(ui->AssayNameTbl, SIGNAL(clicked(QModelIndex)), this, SLOT(OnSelectCurveShow(QModelIndex)));
    connect(ui->AssayNameTbl->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
        auto&&  selections = selected.indexes();
        if (!selections.empty())
            OnSelectCurveShow(selections[0]);
        else
        {
            OnSelectCurveShow(QModelIndex());
        }
    });
    connect(m_pCalHisCurve, &CalHisCurve::SigCurrentSelCalHisInfoChanged, this, &QCaliBrateHistory::OnCurrentSelCalHisInfoChanged);
    connect(m_pCalHisCurve, &CalHisCurve::SigOnSelectPoint, this, &QCaliBrateHistory::OnSelectPos);
    connect(ui->PrintBtn, &QPushButton::clicked, this, &QCaliBrateHistory::OnPrint);
    connect(ui->export_btn, &QPushButton::clicked, this, &QCaliBrateHistory::OnExport);
    connect(ui->moduleA, &QRadioButton::clicked, [this] {OnRefreshHistory(); });
    connect(ui->moduleB, &QRadioButton::clicked, [this] {OnRefreshHistory(); });
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
/// @brief 创建QwtCurve显示数据
///
/// @param[in]  caliCurves  校准数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void QCaliBrateHistory::CreateQwtPlotCurveData(const std::vector<ch::tf::CaliCurve>& caliCurves)
{
    static std::map<int, QVector<QPointF>>  caliHis;  //各水平的记录
    int hisCount = 0;   // 历史记录条数

    // 清理原有数据
    for (auto& his : caliHis)
    {
        his.second.clear();
    }

    // 汇总各水平的数据
    for (const auto& lst : caliCurves)
    {
        // 排除编辑曲线
        if (lst.isEditCurve)
            continue;

        // 校准成功的曲线
        if (lst.successed)
        {
            const shared_ptr<ch::ICalibrateCurve>& currentCurveParamter = CaliBrateCommom::GetCurve(lst);

            if (currentCurveParamter == nullptr)
            {
                return;
            }

            //获取曲线的点
            const auto& info = currentCurveParamter->GetAlgorithmConfigs();
            auto points = info.origPoints;
            // 空白校准
            if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
                info.spanedPoints.size() > 1 && points.size() > 1)
            {
                points[0] = info.spanedPoints[0];
            }

            for (size_t i = 1; i <= points.size(); i++)
            {
                QPointF pointF;
                
                pointF.setX(hisCount + 1);
                pointF.setY(std::get<1>(points[i - 1]));
                
                caliHis[i].push_back(pointF);
            }
        }
        // 校准失败的曲线
        else
        {
            // 如果有部分成功的点，则需要把部分成功的点显示出来
            if (!lst.assayTestResults.empty())
            {
                for (const auto& result : lst.assayTestResults)
                {
                    auto r = GetAssayTestResult(result.second);
                    
                    if (!r)
                        continue;

                    // 判断是否已经设置过一次点了
                    if (caliHis.find(result.first) != caliHis.end() &&
                        !caliHis[result.first].empty() &&
                        caliHis[result.first].back().x() == hisCount + 1)
                    {
                        QPointF pointF;
                        int vecSize = caliHis[result.first].size();
                        
                        pointF.setX(hisCount + 1);
                        // 获取吸光度数据（目前校准品是测两次，如果非两次的话，需要将此处逻辑移动外部循环）
                        pointF.setY(std::round((caliHis[result.first][vecSize - 1].y() + r->abs) / 2));
                        
                        caliHis[result.first][vecSize - 1] = pointF;
                    }
                    else
                    {
                        QPointF pointF;
                        
                        pointF.setX(hisCount + 1);
                        // 获取吸光度数据
                        pointF.setY(r->abs);
                        
                        caliHis[result.first].push_back(pointF);
                    }
                }
            }
        }
        // 曲线计数增加
        ++hisCount;
        m_curCaliCurves.emplace_back(lst);
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
        }

        m_pCalHisCurve->SetHisCount(hisCount);
        m_pScrollBar->setValue(0);
        m_pCalHisCurve->TestRangeChanged(0);
    }

    emit sigSetAllChecked();

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
QwtPlotCurve * QCaliBrateHistory::CreateInitQwtPlotCurve(int xAxis, int yAxis, const QColor & brushColor)
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
/// @brief 设置图例相关控件和处理函数
///
/// @param[in]  legendAttrs  图例数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void QCaliBrateHistory::SetCurveLegend(const QVector<QLegendAttr>& legendAttrs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清除原有数据
    QLayoutItem* child = nullptr;
    while ((child = ui->legendLayout->takeAt(0)) != nullptr)
    {
        if (child->widget())
            child->widget()->setParent(nullptr);

        delete child;
    }

    // 初始化图例数据
    QList<QCheckBox*> boxs;
    QList<QLabel*> labels;
    for (const auto& item : legendAttrs)
    {
        // 复选框
        QCheckBox* checkbox = new QCheckBox(item.legendTitle);
        checkbox->setMinimumWidth(110);
        checkbox->setChecked(true);
        //checkbox->setContentsMargins(80/*QMargins(checkbox->contentsMargins().left()*/, checkbox->contentsMargins().top(), 80, checkbox->contentsMargins().bottom());
        //checkbox->setStyleSheet("margin-right: 30; padding-right: 30");
        boxs.push_back(checkbox);

        // 图例图标
        const  QString& circleStyle = "min-width:12px; min-height:12px; max-width:12px; max-height:12px; border-radius:6px; background:" + item.legendColor.name();
        QLabel* imageLabel = new QLabel();
        imageLabel->setStyleSheet(circleStyle);
        labels.push_back(imageLabel);

        // 图例标题
        QLabel* legTitle = new QLabel(item.legendTitle);
        legTitle->setContentsMargins(QMargins(legTitle->contentsMargins().left(), legTitle->contentsMargins().top(), 10, legTitle->contentsMargins().bottom()));
        labels.push_back(legTitle);

        // 复选框点击处理
        connect(checkbox, &QCheckBox::stateChanged, this, [=](int state) {
            if (item.curve == nullptr)
                return;

            if (state == Qt::Checked)
                item.curve->attach(m_pCalHisCurve->GetQwtPlot());
            else
            {
                item.curve->detach();
            }

            for (auto& l : GetLegendAttrs())
            {
                if (item.curve == l.curve)
                {
                    l.isDetach = (state != Qt::Checked);
                }
            }
        });
    }

    // 添加复选框
    for (const auto& box : boxs)
    {
        ui->legendLayout->addWidget(box);
    }
    ui->legendLayout->setAlignment(Qt::AlignVCenter);
    ui->legendLayout->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Expanding));

    // 添加图例标签
    for (const auto& lb : labels)
    {
        ui->legendLayout->addWidget(lb);
    }

    // 处理checkbox全部选中信号
    connect(this, &QCaliBrateHistory::sigSetAllChecked, this, [=] {
        for (auto& box : boxs)
        {   
            auto state = box->checkState();
            
            if (state == Qt::Checked)
                emit box->stateChanged(Qt::Checked);
            else
                box->setChecked(true);
        }
    });
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
QVector<QCaliBrateHistory::QLegendAttr>& QCaliBrateHistory::GetLegendAttrs()
{
    static std::once_flag onceFlag;
    // 每个校准品水平的图例颜色以及数据点颜色
    static QMap<int, QColor> legendColor{
        { 1, QColor(31, 181, 88) },
        { 2, QColor(69, 159, 255) },
        { 3, QColor(124, 63, 209) },
        { 4, QColor(209, 56, 204) },
        { 5, QColor(67, 74, 222) },
        { 6, QColor(250, 144, 22) }
    };

    static QVector<QCaliBrateHistory::QLegendAttr> legendAttrs(legendColor.size());

    // 初始化每个校准水平 点的属性
    std::call_once(onceFlag, [&] {
        // 创建显示的曲线（一次创建）
        for (size_t i = 1; i <= legendColor.size(); i++)
        {
            auto& legend = legendAttrs[i - 1];
            legend.isDetach = false;
            legend.curve = CreateInitQwtPlotCurve(QwtPlot::xBottom, QwtPlot::yLeft, legendColor[i]);
            legend.legendTitle = tr("校准品") + QString::number(i);
            legend.legendColor = legendColor[i];
        }
    });
 
    return legendAttrs;
}

boost::optional<ch::tf::AssayTestResult> QCaliBrateHistory::GetAssayTestResult(int64_t dbId)
{
    ch::tf::AssayTestResultQueryCond queryAssaycond;
    ch::tf::AssayTestResultQueryResp assayTestResult;
    // 根据测试项目的ID来查询结果
    queryAssaycond.__set_id(dbId);

    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
        return boost::none;
    }

    return boost::make_optional(assayTestResult.lstAssayTestResult[0]); 
}

void QCaliBrateHistory::CreateScaleDraw_X(int selIndex)
{
    QCustomScaleDraw* pScaleDrawX = new QCustomScaleDraw(std::bind(&QCaliBrateHistory::SetCustomAxisInfo_X_Bottom, this, std::placeholders::_1));
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
        pScaleDrawX->SetSelLabelColor(selIndex, QColor(0xfa, 0x0, 0x0));
        m_pCalHisCurve->GetQwtPlot()->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
    }
}

void QCaliBrateHistory::OnShowQwtCurveTip(bool isShow, int level, int curveIndex, const QPoint& pos)
{
    // 隐藏显示点
    m_pCaliSelRing->detach();
    m_pCaliSelTipMarker->detach();

    // 不需要显示则直接退出
    if (!isShow || curveIndex< 0 || curveIndex >= m_curCaliCurves.size())
        return;

    const auto& index = ui->AssayNameTbl->currentIndex();
    const QString& str = m_CurveModeProxyModel->itemData(index)[Qt::DisplayRole].toString();
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(str.toStdString());
    if (assay == nullptr)
    {
        ULOG(LOG_ERROR, "invalid assay, name is %s.", str.toStdString());
        return;
    }

    QVector<qreal>  concs;  // 当前水平的浓度汇总
    QVector<qreal>  abses;  // 当前水平的吸光度汇总

    const auto& curve = m_curCaliCurves[curveIndex];

    if (curve.successed)
    {
        shared_ptr<ch::ICalibrateCurve>& currentCurveParamter = CaliBrateCommom::GetCurve(curve);

        if (currentCurveParamter == nullptr)
        {
            return;
        }

        //获取曲线的点
        const auto& info = currentCurveParamter->GetAlgorithmConfigs();
        auto points = info.origPoints;
        // 空白校准
        if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
            info.spanedPoints.size() > 1 && points.size() > 1)
        {
            points[0] = info.spanedPoints[0];
        }

        if (!points.empty())
        {
            int col = 1;
            for (auto& pt : points)
            {
                if (level != col)
                {
                    ++col;
                    continue;
                }

                //connText1 = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curve.assayCode, std::get<0>(pt)), 'f', 2);
                concs.append(std::get<0>(pt));
                abses.append(std::get<1>(pt));

                break;
            }

        }
    }
    // 校准失败的话，从m_assayTestResults中获取数据
    else
    {
        // 循环获取数据
        for (const auto& result : curve.assayTestResults)
        {
            if (result.first != level)
                continue;

            auto r = GetAssayTestResult(result.second);
            if (!r)
                continue;

            if (r->__isset.conc)
                concs.append(r->conc);

            if (r->__isset.abs)
                abses.append(r->abs);
            //break;
        }
    }

    if (abses.isEmpty())
        return;

    // 累加浓度和吸光度，用于平均计算，归一上面的处理
    qreal sum_conc = 0.0;
    for (const auto& p : concs)
    {
        sum_conc += p;
    }

    qreal sum_abs = 0.0;
    for (const auto& p : abses)
    {
        sum_abs += p;
    }

    auto plot = m_pCalHisCurve->GetQwtPlot();
    // 显示高亮圆环
    QPointF pointF;
    pointF.setX(curveIndex + 1);
    pointF.setY(std::round(sum_abs/ abses.size()));
    m_pCaliSelRing->setSamples({ pointF });
    m_pCaliSelRing->attach(plot);

    // 显示提示框
    // 构造qwtText
    m_strSelPtTextList.clear();
    const auto& date = ToCfgFmtDate(QString::fromStdString(curve.caliTime));
    const auto& time = ToCfgFmtTime(QString::fromStdString(curve.caliTime));
    QString strConc;
    if (!concs.isEmpty())
    {
        double curValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assay->assayCode, sum_conc / concs.size());
        strConc = QString::number(curValue, 'f', assay->decimalPlace);
    }
    m_strSelPtTextList << strConc << time << date;
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

void QCaliBrateHistory::OnSelectPos(const QPoint & pos)
{
    QVector<QLegendAttr>& legendArr = GetLegendAttrs();
    int level = 1;
    for (const auto& le : legendArr)
    {
        double d;
        // 有曲线，且显示了
        if (le.curve && !le.isDetach)
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

void QCaliBrateHistory::OnExport()
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
            CaliHistoryInfoCh Info;
            GetPrintExportInfo(Info);

            QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
            Info.strPrintTime = strPrintTime.toStdString();
            Info.strExportTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(Info);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliHistoryInfoCh.lrxml";
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

void QCaliBrateHistory::OnPrint()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    CaliHistoryInfoCh info;
    GetPrintExportInfo(info);

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    info.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(info);
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
QCurveModeFilterDataModule::QCurveModeFilterDataModule(QObject * parent)
{
}

QCurveModeFilterDataModule::~QCurveModeFilterDataModule()
{
}

bool QCurveModeFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
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

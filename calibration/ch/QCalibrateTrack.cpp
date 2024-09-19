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
/// @file     QCalibrateTrack.h
/// @brief    校准追踪
///
/// @author   5774/WuHongTao
/// @date     2022年3月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCalibrateTrack.h"
#include "ui_QCalibrateTrack.h"
#include <tuple>
#include <QStandardItemModel>

#include "CaliBrateCommom.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/common/Mlog/mlog.h"

QCalibrateTrack::QCalibrateTrack(QWidget *parent)
    : BaseDlg(parent),
    m_CalibrateTrackResultMode(nullptr)
{
    ui = new Ui::QCalibrateTrack();
    ui->setupUi(this);
    m_assayMap.clear();
    ui->Start_Edit->setDate(QDate::currentDate());
    ui->End_Edit->setDate(QDate::currentDate());
    InitCurveAttribute();
    InitProjectCombox();
    ResetCalibrateLevelResultTable();
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
    SetTitleName("CaliBrate Track");
}

QCalibrateTrack::~QCalibrateTrack()
{
}

///
/// @brief
///     初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月1日，新建函数
///
void QCalibrateTrack::InitCurveAttribute()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    //初始化控件最开始的大小
    CoordinateDataType tmpSetData = ui->TracckCurve->GetCurveAttrbute();
    // 改变类型
    tmpSetData.scaleType = 1;
    // 改变单位
    tmpSetData.unit = QString("");
    // 坐标系小数点余数
    tmpSetData.retainedNumber = 0;
    // 设置坐标属性
    ui->TracckCurve->setAttribute(tmpSetData);

    // 限制新增数目
    QRegExp regx("[-]?[0-4][0-9]{0,4}");
    QValidator *validator = new QRegExpValidator(regx, ui->maxy_Edit);
    ui->maxy_Edit->setValidator(validator);
    connect(ui->maxy_Edit, SIGNAL(textChanged(QString)), this, SLOT(OnChangeCurveMaxy(QString)));
    validator = new QRegExpValidator(regx, ui->miny_Edit);
    ui->miny_Edit->setValidator(validator);

    connect(ui->miny_Edit, SIGNAL(textChanged(QString)), this, SLOT(OnChangeCurveMiny(QString)));
    // 自动按钮
    ui->Auto_Button->setChecked(true);
    connect(ui->Auto_Button, SIGNAL(clicked()), this, SLOT(OnAutoAxisUpdate()));
    // 手动按钮
    connect(ui->Manual_Button, SIGNAL(clicked()), this, SLOT(OnManualAxisUpdate()));
    emit ui->Auto_Button->clicked();
    // 初始化曲线的样式
    m_AbsorbCurve.scatterMarkerShape = CIRCLE;
    m_AbsorbCurve.colorType = NINE;
}

///
/// @brief
///     初始化项目列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::InitProjectCombox()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui->project_comboBox->clear();
    ui->calibrate_comboBox->clear();
    if (!CommonInformationManager::GetInstance()->GetChAssayMaps(tf::DeviceType::DEVICE_TYPE_C1000, m_assayMap))
    {
        return;
    }

    // 项目信息
    for (const auto& AssayInfo : m_assayMap)
    {

		ui->project_comboBox->addItem(QString::fromStdString(AssayInfo.first), QString::fromStdString(AssayInfo.first));
    }

    // 项目选择的消息和槽函数
    connect(ui->project_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSelectAssay(int)));

    // 若不为空则发送信号
    if (!m_assayMap.empty())
    {
        emit ui->project_comboBox->currentIndexChanged(0);
    }
}

///
/// @brief
///     设置设备列表
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::SetDeivceList(std::vector<std::string>& devices)
{
    m_StrDevs = devices;
}

///
/// @brief
///     重置校准品结构跟踪表
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCalibrateTrack::ResetCalibrateLevelResultTable()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_CalibrateTrackResultMode == nullptr)
    {
        m_CalibrateTrackResultMode = new QStandardItemModel(this);
        ui->tableView->setModel(m_CalibrateTrackResultMode);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    m_CalibrateTrackResultMode->clear();
    QStringList CalibrateTrackResult;
    CalibrateTrackResult << tr("sequence") << tr("time") << tr("caliLot") << tr("ReagentLot")
        << tr("conc") << tr("absort");
    m_CalibrateTrackResultMode->setHorizontalHeaderLabels(CalibrateTrackResult);
    m_CalibrateTrackResultMode->setRowCount(30);
}

///
/// @brief
///     显示曲线的信息--以表格形式
///
/// @param[in]  curve  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::ShowCurveTable(int row, const ch::tf::CaliCurve& curve, curveSingleDataStore& ContinueCurve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CalibrateTrackResultMode == nullptr)
    {
        return;
    }

    int column = 0;
    // 序号
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::number(row)));
    // 校准时间
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::fromStdString(curve.caliTime)));
    // 校准品批号
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::fromStdString(curve.calibratorLot)));
    // 试剂批号
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::fromStdString(curve.reagentLot)));

    // 获取曲线句柄
    std::shared_ptr<ch::ICalibrateCurve> currentCurve = CaliBrateCommom::GetCurve(curve);
    if (currentCurve == nullptr)
    {
        return;
    }

    int index = ui->calibrate_comboBox->currentIndex();
    // 当前校准品的值异常
    if (index >= currentCurve->GetCalibratePoints().size())
    {
        return;
    }

    // 第一个值代表浓度
    double conc = std::get<0>(currentCurve->GetCalibratePoints()[index]);
    // 第二个值代表吸光度
    double absord = std::get<1>(currentCurve->GetCalibratePoints()[index]);
    // 浓度
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::number(conc, 'f', 2)));
    // 吸光度
    m_CalibrateTrackResultMode->setItem(row, column++, new QStandardItem(QString::number(absord, 'f', 2)));

    // 获取曲线信息
    RealDataStruct tempdata;
    tempdata.xData = row;
    tempdata.yData = absord;
    ContinueCurve.dataContent.append(tempdata);
}

///
/// @brief
///     显示曲线
///
/// @param[in]  curve  曲线的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::ShowCurveData(curveSingleDataStore& curve)
{

}

///
/// @brief
///     坐标系自动跟随
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建函数
///
void QCalibrateTrack::OnAutoAxisUpdate()
{
    // 屏蔽自动的输入框
    ui->miny_Edit->setEnabled(false);
    ui->maxy_Edit->setEnabled(false);
    // 设置坐标系为自动
    CoordinateDataType tmpSetData = ui->TracckCurve->GetCurveAttrbute();
    tmpSetData.needRefreshYScale = false;
    ui->TracckCurve->setAttribute(tmpSetData);
}

///
/// @brief
///     根据设置的手动数据更新坐标系
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建函数
///
void QCalibrateTrack::OnManualAxisUpdate()
{
    // 启用自动的输入框
    ui->miny_Edit->setEnabled(true);
    ui->maxy_Edit->setEnabled(true);

    // 如果没有数据，则返回
    if (ui->maxy_Edit->text().isEmpty() || ui->miny_Edit->text().isEmpty())
    {
        return;
    }

    int min = ui->miny_Edit->text().toInt();
    int max = ui->maxy_Edit->text().toInt();

    // 值得范围必须合理
    if (min >= max)
    {
        return;
    }

    CoordinateDataType tmpSetData = ui->TracckCurve->GetCurveAttrbute();
    tmpSetData.minYscale = min;
    tmpSetData.maxYsacle = max;
    tmpSetData.needRefreshYScale = true;
    ui->TracckCurve->setAttribute(tmpSetData);
}

///
/// @brief
///     改变曲线的Y轴的最大值
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCalibrateTrack::OnChangeCurveMaxy(QString text)
{
    if (ui->miny_Edit->text().isEmpty())
    {
        return;
    }

    int min = ui->miny_Edit->text().toInt();
    CoordinateDataType tmpSetData = ui->TracckCurve->GetCurveAttrbute();
    if (min > tmpSetData.maxYsacle)
    {
        return;
    }

    tmpSetData.needRefreshYScale = true;
    tmpSetData.minYscale = min;
    ui->TracckCurve->setAttribute(tmpSetData);
}

///
/// @brief
///     改变曲线的Y轴的最小值
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCalibrateTrack::OnChangeCurveMiny(QString text)
{
    if (ui->maxy_Edit->text().isEmpty())
    {
        return;
    }

    int max = ui->maxy_Edit->text().toInt();
    CoordinateDataType tmpSetData = ui->TracckCurve->GetCurveAttrbute();
    if (max < tmpSetData.minYscale)
    {
        return;
    }

    tmpSetData.needRefreshYScale = true;
    tmpSetData.maxYsacle = max;
    ui->TracckCurve->setAttribute(tmpSetData);
}

///
/// @brief
///     选择不同的水平的校准品
///
/// @param[in]  index  序号
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::OnSelectAssay(int index)
{
    ULOG(LOG_INFO, "%s() index is %d", __FUNCTION__, index);

    ui->calibrate_comboBox->clear();
    // 若查找不到
    QString assayName = ui->project_comboBox->itemData(index).toString();
    if (m_assayMap.find(assayName.toStdString()) == m_assayMap.end())
    {
        return;
    }

    // 水平选择的消息和槽函数
    disconnect(ui->calibrate_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSelectCaliLevel(int)));

    QString prefix = "CaliBrate";
	int caliQuantity = m_assayMap[assayName.toStdString()]->caliQuantity;
    for (int i = 0; i < caliQuantity; i++)
    {
        ui->calibrate_comboBox->addItem(prefix + QString::number(i));
    }
  
    // 水平选择的消息和槽函数
    connect(ui->calibrate_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSelectCaliLevel(int)));
}

///
/// @brief
///     选择不同水准的校准品
///
/// @param[in]  level  校准品水平
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月7日，新建函数
///
void QCalibrateTrack::OnSelectCaliLevel(int level)
{
    ULOG(LOG_INFO, "%s() level is %d", __FUNCTION__, level);
    // 重置表格
    ResetCalibrateLevelResultTable();
    //重新绘制曲线
    ui->TracckCurve->clearCurve();
    // 获取关键字
	QString assayName = ui->project_comboBox->currentData().toString();;
	if (m_assayMap.find(assayName.toStdString()) == m_assayMap.end())
	{
		return;
	}

    ::tf::TimeRangeCond timeCond;
    QString timeStart = ui->Start_Edit->date().toString("yyyy-MM-dd");
    timeStart = timeStart + " 00:00:00";
    QString endStart = ui->End_Edit->date().toString("yyyy-MM-dd");
    endStart = endStart + " 23:59:59";
    timeCond.__set_startTime(timeStart.toStdString());
    timeCond.__set_endTime(endStart.toStdString());

    // 曲线查询条件
    ch::tf::CaliCurveQueryCond ConditionCurveQuery;
    // 设置设备列表
    ConditionCurveQuery.__set_deviceSN(m_StrDevs);
    // 设置项目编号
    ConditionCurveQuery.__set_assayCode(m_assayMap[assayName.toStdString()]->assayCode);
    // 设置追踪时间范围
    ConditionCurveQuery.__set_caliTime(timeCond);
    // 查询曲线数据
    ch::tf::CaliCurveQueryResp ResultQueryCaliCurves;
    ch::c1005::LogicControlProxy::QueryCaliCurve(ResultQueryCaliCurves, ConditionCurveQuery);
    if (ResultQueryCaliCurves.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || ResultQueryCaliCurves.lstCaliCurves.empty())
    {
        ULOG(LOG_INFO, "QueryCaliCurve failed");
        return;
    }

    int row = 0;
    //获取算法计算的曲线
    for (const auto& curve : ResultQueryCaliCurves.lstCaliCurves)
    {
        ShowCurveTable(row, curve, m_AbsorbCurve);
        row++;
    }

    ui->TracckCurve->addCurveData(m_AbsorbCurve);
}

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
/// @file     QCurveDetial.cpp
/// @brief    校准曲线显示
///
/// @author   5774/WuHongTao
/// @date     2022年3月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCurveDetial.h"
#include "ui_QCurveDetial.h"
#include <QStandardItemModel>
#include "CaliBrateCommom.h"
#include "curveDraw/common.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"

QCurveDetial::QCurveDetial(QWidget *parent)
    : BaseDlg(parent),
    m_CurvePointMode(nullptr)
{
    ui = new Ui::QCurveDetial();
    ui->setupUi(this);
    Init();
    // 关闭按钮
    connect(ui->close_Button, SIGNAL(clicked()), this, SLOT(close()));
    SetTitleName(tr("校准曲线"));
}

QCurveDetial::~QCurveDetial()
{
}

///
/// @brief
///     曲线初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCurveDetial::Init()
{
    //初始化控件最开始的大小
    CoordinateDataType tmpSetData = ui->CaliCurve->GetCurveAttrbute();
    // 改变类型
    tmpSetData.scaleType = 1;
    // 改变单位
    tmpSetData.unit = QString("");
    // 坐标系小数点余数
    tmpSetData.retainedNumber = 0;
    // 设置坐标属性
    ui->CaliCurve->setAttribute(tmpSetData);
}

///
/// @brief
///     显示校准曲线信息
///
/// @param[in]  caliCurve  校准曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月28日，新建函数
///
void QCurveDetial::ShowReactionCurve(const ch::tf::CaliCurve& caliCurve)
{
    UpdateCurve(caliCurve);
    ResetCurveTable();
    std::shared_ptr<ch::ICalibrateCurve> curveCurrent = CaliBrateCommom::GetCurve(caliCurve);

    int column = 0;
    for (auto curveAbsorb : curveCurrent->GetCalibratePoints())
    {
        int row = 1;
        //浓度
        m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(std::get<0>(curveAbsorb), 'f', 2)));

        for (auto absorb : std::get<2>(curveAbsorb))
        {
            //吸光度
            m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(absorb, 'f', 2)));
        }
        column++;
    }
}

///
/// @brief
///     更新校准曲线的显示
///
/// @param[in]  curve  校准曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
shared_ptr<ch::ICalibrateCurve> QCurveDetial::UpdateCurve(const ch::tf::CaliCurve& curve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    //得到当前曲线的数据句柄
    if (curve.curveData.empty())
    {
        ULOG(LOG_ERROR, "can't get the curve json information %s()", __FUNCTION__);
        return nullptr;
    }

    //当有修改的时候，选择修改过的曲线数据（最新）
    shared_ptr<ch::ICalibrateCurve> currentCurve = CaliBrateCommom::GetCurve(curve);
    if (currentCurve == nullptr)
    {
        return nullptr;
    }

    static curveSingleDataStore ScatterCurve;
    ui->CaliCurve->removeCurve(ScatterCurve.curveId);
    ScatterCurve.dataContent.clear();

    // 获取离散曲线
    if (!CaliBrateCommom::CreatScatterCurve(ScatterCurve, currentCurve))
    {
        return currentCurve;
    }
    ScatterCurve.curveId = ui->CaliCurve->addCurveData(ScatterCurve);

    // 获取连续曲线--初始化
    static curveSingleDataStore ContinueCurve;
    ui->CaliCurve->removeCurve(ContinueCurve.curveId);
    ContinueCurve.dataContent.clear();

    // 获取连续曲线
    if (!CaliBrateCommom::CreatContinueCurve(ContinueCurve, ScatterCurve, currentCurve))
    {
        return currentCurve;
    }

    ContinueCurve.curveId = ui->CaliCurve->addCurveData(ContinueCurve);

    //返回曲线的句柄
    return currentCurve;
}

///
/// @brief
///     显示曲线的检测点
///
/// @param[in]  result  曲线检测点
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCurveDetial::ShowCurveDetectPoint(ch::tf::AssayTestResult& result)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CurvePointMode == nullptr)
    {
        m_CurvePointMode = new QStandardItemModel(this);
        ui->tableView->setModel(m_CurvePointMode);
    }

    m_CurvePointMode->clear();
    QStringList CurveTypeShowList;
    CurveTypeShowList << tr("calibrate") << tr("conc") << tr("calibrate") << tr("conc");
    m_CurvePointMode->setVerticalHeaderLabels(CurveTypeShowList);

    int primWave = -1;
    int subWave = -1;

    // 仅有主波长
    if (result.primarySubWaves.size() == 1)
    {
        primWave = result.primarySubWaves[0];
    }
    // 主次波长都有
    else if (result.primarySubWaves.size() >= 2)
    {
        primWave = result.primarySubWaves[0];
        subWave = result.primarySubWaves[1];
    }

    // 依次显示内容
    int column = 0;
    for (auto detectPoint : result.detectPoints)
    {
        int row = 0;
        // 测光点编号
        m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.pointSN)));
        // 主波长
        if (primWave == -1)
        {
            m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
        }
        else
        {
            m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[primWave])));
        }

        if (subWave == -1)
        {
            m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
            m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
        }
        else
        {
            // 次波长
            m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[subWave])));
            // 主-次
            m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[primWave] - detectPoint.ods[subWave])));
        }

        column++;
    }
}

///
/// @brief
///     重置曲线数据框
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月10日，新建函数
///
void QCurveDetial::ResetCurveTable()
{
    if (m_CurvePointMode == nullptr)
    {
        m_CurvePointMode = new QStandardItemModel(this);
        ui->tableView->setModel(m_CurvePointMode);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectColumns);
        ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->horizontalHeader()->hide();
    }

    m_CurvePointMode->clear();
    m_CurvePointMode->setVerticalHeaderLabels({ tr("pointer") , tr("pirmWave") , tr("secondWave") , tr("PrimSecond") });

    int column = 0;
	for (const auto& head : {tr("水"), tr("校准品1") , tr("校准品1") , tr("校准品2") , tr("校准品3") , tr("校准品4") })
    {
        m_CurvePointMode->setItem(0, column++, new QStandardItem(head));
    }
}

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
/// @file     QCalibrateDetail.h
/// @brief    校准品信息显示
///
/// @author   5774/WuHongTao
/// @date     2022年2月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCalibrateDetail.h"
#include "ui_QCalibrateDetail.h"
#include <QStandardItemModel>
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/common/Mlog/mlog.h"

QCalibrateDetail::QCalibrateDetail(QWidget *parent)
    : QWidget(parent),
    m_calibrateConcMode(nullptr),
    m_calibrateParameterMode(nullptr)
{
    ui = new Ui::QCalibrateDetail();
    ui->setupUi(this);
    // 初始化表格
    ClearTableContent();
}

QCalibrateDetail::~QCalibrateDetail()
{
}

///
/// @brief
///     更新曲线参数
///
/// @param[in]  curveParameter  参数曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建函数
///
void QCalibrateDetail::UpdateCurveParameter(std::map<std::string, double>& curveParameter)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ResetCValiBrateParameterTable();
    int column = 1;
    for (auto dataEach : curveParameter)
    {
        m_calibrateParameterMode->setItem(0, column++, new QStandardItem(QString::number(dataEach.second)));
    }
}

///
/// @brief
///     更新曲线的详细数据
///
/// @param[in]  curve  曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建函数
///
void QCalibrateDetail::UpdateCurveDetail(const std::vector<tuple<double, double, std::vector<double>>>& curveData)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ResetCaliBrateCurveTable();

    int column = 0;
    for (auto curveAbsorb : curveData)
    {
        int row = 1;
        //浓度
        m_calibrateConcMode->setItem(row++, column, new QStandardItem(QString::number(std::get<0>(curveAbsorb), 'f', 2)));

        for (auto absorb : std::get<2>(curveAbsorb))
        {
            //吸光度
            m_calibrateConcMode->setItem(row++, column, new QStandardItem(QString::number(absorb, 'f', 2)));
        }
        column++;
    }
}

///
/// @brief
///     清除表格内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建函数
///
void QCalibrateDetail::ClearTableContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ResetCaliBrateCurveTable();
    ResetCValiBrateParameterTable();
}

///
/// @brief
///     重置曲线数据框
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月10日，新建函数
///
void QCalibrateDetail::ResetCaliBrateCurveTable()
{
    if (m_calibrateConcMode == nullptr)
    {
        m_calibrateConcMode = new QStandardItemModel(this);
        ui->conc->setModel(m_calibrateConcMode);
        ui->conc->setSelectionBehavior(QAbstractItemView::SelectColumns);
        ui->conc->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->conc->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->conc->horizontalHeader()->hide();
    }

    m_calibrateConcMode->clear();
    QStringList CurveTypeShowList;
    CurveTypeShowList << tr("calibrate") << tr("conc") << tr("calibrate") << tr("conc");
    m_calibrateConcMode->setVerticalHeaderLabels(CurveTypeShowList);

    QStringList TitleShow;
    TitleShow << "Water" << "ComCalibrate1" << "ComCalibrate2" << "ComCalibrate3" << "ComCalibrate4" << "ComCalibrate5";
    int column = 0;
    for (auto title : TitleShow)
    {
        m_calibrateConcMode->setItem(0, column++, new QStandardItem(title));
    }
}

///
/// @brief
///     重置曲线的参数列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月10日，新建函数
///
void QCalibrateDetail::ResetCValiBrateParameterTable()
{
    if (!m_calibrateParameterMode)
    {
        m_calibrateParameterMode = new QStandardItemModel(this);
        ui->parameter->setModel(m_calibrateParameterMode);
        ui->parameter->setSelectionBehavior(QAbstractItemView::SelectColumns);
        ui->parameter->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->parameter->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    m_calibrateParameterMode->clear();
    QStringList calibrateParaMeterStringList;
    calibrateParaMeterStringList << tr("S1Abs") << tr("K") << tr("A") << tr("B")
        << tr("C") << tr("L") << tr("H") << tr("I");
    m_calibrateParameterMode->setHorizontalHeaderLabels(calibrateParaMeterStringList);
}

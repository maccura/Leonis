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
/// @file     CaliBrateCurveShow.h
/// @brief    校准曲线显示页面
///
/// @author   5774/WuHongTao
/// @date     2021年11月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CaliBrateCurveShow.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父指针
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
CaliBrateCurveShow::CaliBrateCurveShow(QDialog *parent)
    : QDialog(parent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui.setupUi(this);

    // 初始化吸光度表信息
    QStringList headerList;
    headerList << tr("name")<< tr("conc") << tr("absorbance")<< tr("lot");
    ui.tableWidget->setRowCount(5);
    ui.tableWidget->setColumnCount(headerList.size());
    ui.tableWidget->setHorizontalHeaderLabels(headerList);
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui.closeBtn, SIGNAL(clicked()), this, SLOT(OnCloseWindow()));
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
CaliBrateCurveShow::~CaliBrateCurveShow()
{
}

///
/// @brief
///     显示当前校准曲线
///
/// @param[in]  curveInfo  校准曲线数据
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
void CaliBrateCurveShow::StartShowCurrentCurve(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo)
{

    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空曲线
    ui.widget->clearCurve();

    // 防止崩溃，空指针
    if (curveInfo == nullptr)
    {
        return;
    }

    // 显示参数信息
    ShowCurveParameter(curveInfo);
    // 显示浓度吸光度信息
    ShowCurveData(curveInfo);

    //获取真实的对应点
    curveSingleDataStore dateCurve;
    dateCurve.scatterMarkerShape = CIRCLE;
    dateCurve.colorType = FIVE;
    dateCurve.IsPointerConnect = false;

    //计算点的个数，用于后面校准的计算
    int m = 0;
    dateCurve.dataContent.clear();
    //临时值
    RealDataStruct tempdata;
    //获取曲线的点
    for (auto pointer : curveInfo->GetCalibratePoints())
    {
        double x = std::get<0>(pointer);
        double y = std::get<1>(pointer);
        tempdata.xData = x;
        tempdata.yData = y;
        {
            tempdata.type = dateCurve.scatterMarkerShape;
            tempdata.color = dateCurve.colorType;
        }
        dateCurve.dataContent.append(tempdata);
        m++;
    }

    // 当曲线没有数据的时候，直接返回
    if (dateCurve.dataContent.empty())
    {
        ULOG(LOG_ERROR, "caliCurve data is empty %s()", __FUNCTION__);
        return;
    }

    // 添加曲线
    int curveId = ui.widget->addCurveData(dateCurve);

    double firstValue = dateCurve.dataContent[0].xData;
    double lastValue = 0;
    double step = 0;

    //保证不除以0,计算步进
    if (MAXPOINTER <= 0 || m<= 0)
    {
        return;
    }

    lastValue = dateCurve.dataContent[m - 1].xData;
    step = (lastValue - firstValue) / MAXPOINTER;

    //获取算法计算的曲线
    curveSingleDataStore dateCaculateCurve;
    dateCaculateCurve.dataContent.clear();
    dateCaculateCurve.scatterMarkerShape = SQUARE;
    dateCaculateCurve.IsPointerConnect = false;
    dateCaculateCurve.scatterMarkerShape = CALIBRATION;
    dateCaculateCurve.colorType = NINE;
    //每次取多少点来画曲线
    for (size_t i = 0; i <= MAXPOINTER; i++)
    {
        if (i == MAXPOINTER)
        {
            tempdata.xData = lastValue;
        }
        else
        {
            tempdata.xData = firstValue + i*step;
        }

        tempdata.yData = curveInfo->CalculateAbsorbance(tempdata.xData);
        dateCaculateCurve.dataContent.append(tempdata);
    }

    int continueCurveId = ui.widget->addCurveData(dateCaculateCurve);
}

///
/// @brief
///     显示曲线的参数信息
///
/// @param[in]  curveInfo  曲线句柄
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CaliBrateCurveShow::ShowCurveParameter(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 防止崩溃，空指针
    if (curveInfo == nullptr)
    {
        return;
    }

    // 显示参数的句柄
    std::vector<QLineEdit*> ShowParameters;
    ShowParameters.push_back(ui.s1abs);
    ShowParameters.push_back(ui.A);
    ShowParameters.push_back(ui.B);
    ShowParameters.push_back(ui.C);
    ShowParameters.push_back(ui.H);
    ShowParameters.push_back(ui.I);
    ShowParameters.push_back(ui.L);
    ShowParameters.push_back(ui.K);

    // 依次显示
    int index = 0;
    for (auto parameter : curveInfo->GetModelParameters())
    {
        // 防止越界
        if (index < ShowParameters.size())
        {
            ShowParameters[index++]->setText(QString::number(parameter.second));
        }
    }
}

///
/// @brief
///     显示曲线的数据
///
/// @param[in]  curveInfo  曲线句柄
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CaliBrateCurveShow::ShowCurveData(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 防止崩溃，空指针
    if (curveInfo == nullptr)
    {
        return;
    }

    int row = 0;
    for (auto pointer : curveInfo->GetCalibratePoints())
    {
        int column = 1;

        // 浓度数据
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(std::get<0>(pointer)));
        ui.tableWidget->setItem(row, column++, item);


        // 吸光度数据
        item = new QTableWidgetItem(QString::number(std::get<1>(pointer)));
        ui.tableWidget->setItem(row, column++, item);

        row++;
    }
}

///
/// @brief
///     关闭窗口
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
void CaliBrateCurveShow::OnCloseWindow()
{
    close();
}

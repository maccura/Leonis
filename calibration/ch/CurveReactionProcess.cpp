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
/// @file     CurveReactionProcess.cpp
/// @brief    反应过程的文件
///
/// @author   5774/WuHongTao
/// @date     2021年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CurveReactionProcess.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父指针
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
CurveReactionProcess::CurveReactionProcess(QWidget *parent)
    : BaseDlg(parent)
    , m_parent(qobject_cast<CaliBrationStatusPage*>(parent))
{
    ui.setupUi(this);
    // 更换校准水平的时候的处理函数
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLevelChanged(int)));
    // 关闭按钮的点击
    connect(ui.closeBtn, SIGNAL(clicked()), this, SLOT(OnCloseBtn()));
    Init();
}

///
/// @brief
///     析构函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
CurveReactionProcess::~CurveReactionProcess()
{
}

///
/// @brief
///     展示项目校准的反应过程
///
/// @param[in]  db  试剂组的主键
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CurveReactionProcess::StartShowAssayCaliBrateAction(int db)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ::ch::tf::ReagentGroupQueryResp qryResp;
    ::ch::tf::ReagentGroupQueryCond qryCond;

    // 设置查询条件，数据库主键
    qryCond.__set_id(db);

    // 查询成功
    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryResp, qryCond))
    {
        ULOG(LOG_ERROR, "Can not find the reagent group id is %d", db);
        return;
    }

    // 没有查找到试剂数据，直接返回
    if (qryResp.lstReagentGroup.size() < 1)
    {
        return;
    }

    // 需要显示的试剂组
    ch::tf::ReagentGroup showGroup = qryResp.lstReagentGroup[0];
    // 获取当前曲线
    ::ch::tf::CaliCurveQueryResp _return;
    ::ch::tf::CaliCurveQueryCond ccqc;

    // 如果没有设置曲线id，则返回
    if (!showGroup.__isset.caliCurveId)
    {
        ULOG(LOG_ERROR, "Can not find the curve id");
        return;
    }

    // 设置查询条件（曲线的主键）
    ccqc.__set_id(showGroup.caliCurveId);
    // 执行查询
    ch::c1005::LogicControlProxy::QueryCaliCurve(_return, ccqc);
    if ((_return.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS) || (_return.lstCaliCurves.size() <= 0))
    {
        ULOG(LOG_ERROR, "Can't find the curve data %d", showGroup.caliCurveId);
        return;
    }

    // 显示项目状态
    ShowAssayStatusInfo(showGroup, _return.lstCaliCurves[0]);
}

///
/// @brief
///     初始化表格
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月24日，新建函数
///
void CurveReactionProcess::Init()
{
    // 设置表头
    QStringList detectPointHeard;
    detectPointHeard << tr("samplePointer") << tr("primaryWave") << tr("subWave") << tr("minusWave")
        << tr("samplePointer") << tr("primaryWave") << tr("subWave") << tr("minusWave");
    ui.tableWidget->setRowCount(defaultRow);
    ui.tableWidget->setColumnCount(detectPointHeard.size());
    ui.tableWidget->setHorizontalHeaderLabels(detectPointHeard);
    ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 初始化测光点表格
    for (int index = 0; index < defaultRow; index++)
    {
        // 第一列采光点
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(index+1));
        ui.tableWidget->setItem(index, 0, item);

        // 第四列采光点
        item = new QTableWidgetItem(QString::number(index+1+ defaultRow));
        ui.tableWidget->setItem(index, 4, item);
    }
}

///
/// @brief
///     显示项目状态信息
///
/// @param[in]  group  试剂组信息
/// @param[in]  curve  校准曲线的信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CurveReactionProcess::ShowAssayStatusInfo(ch::tf::ReagentGroup& group, ch::tf::CaliCurve& curve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 显示项目名字
    ui.nameEdit->setText(m_parent->GetAssayNameFromCode(group.assayCode));
/*    ui.lotEdit->setText(QString::fromStdString(group.reagentLot));*/

    // 添加项目结果
    ui.comboBox->clear();
    // 结果和水平值
    std::map<int64_t, int> recordLevelResult;
    recordLevelResult.clear();
    for (auto level : curve.assayTestResults)
    {
        // 如果能找到对应水平值
        if (recordLevelResult.find(level.first) != recordLevelResult.end())
        {
            int times = recordLevelResult[level.first];
            // 显示字符串
            QString itemLevel = QString::number(level.first) + "(" + QString::number(++times) + ")";
            ui.comboBox->addItem(itemLevel, level.second);
            // 回写记录
            recordLevelResult[level.first] = times;
            continue;
        }

        ui.comboBox->addItem(QString::number(level.first), level.second);
        // 初次添加
        recordLevelResult.insert(pair<int64_t, int>(level.first, 0));
    }
}

///
/// @brief
///     水平变化
///
/// @param[in]  index  变化后对应的索引
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CurveReactionProcess::OnLevelChanged(int index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清除曲线
    ui.ReactionCurve->clearCurve();

    QComboBox* pSender = nullptr;
    pSender = qobject_cast<QComboBox*>(sender());
    // 获取对应的句柄
    if (pSender == nullptr)
    {
        ULOG(LOG_ERROR, "Can not get the handle");
        return;
    }

    // 获取曲线的校准结果的数据库主键
    int reactionDb = pSender->itemData(index).toInt();
    ch::tf::AssayTestResultQueryResp _return;
    ch::tf::AssayTestResultQueryCond trqc;
    trqc.__set_id(reactionDb);
    // 查询不成功，结果不等于成功，数据中没有值都返回
    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(_return, trqc) || (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        ||(_return.lstAssayTestResult.size() <= 0))
    {
        ULOG(LOG_ERROR, "Can not find the result of the curve");
        return;
    }

    //获取真实的对应点
    curveSingleDataStore PrimaryWaveCurve;
    curveSingleDataStore SubWaveCurve;
    curveSingleDataStore CompareWaveCurve;

    // 主波长对应的曲线形状
    {
        PrimaryWaveCurve.scatterMarkerShape = CIRCLE;
        PrimaryWaveCurve.colorType = FIVE;
        PrimaryWaveCurve.IsPointerConnect = false;
        PrimaryWaveCurve.dataContent.clear();
    }

    // 次波长对应的曲线形状
    {
        SubWaveCurve.scatterMarkerShape = SQUARE;
        SubWaveCurve.colorType = SIX;
        SubWaveCurve.IsPointerConnect = false;
        SubWaveCurve.dataContent.clear();
    }

    // 对比波长对应的曲线形状
    {
        CompareWaveCurve.scatterMarkerShape = TRIANGLE;
        CompareWaveCurve.colorType = SEVEN;
        CompareWaveCurve.IsPointerConnect = false;
        CompareWaveCurve.dataContent.clear();
    }

    int primaryWave = -1;
    int subWave = -1;
    // 出问题，没有主副波长的记录
    if (_return.lstAssayTestResult[0].primarySubWaves.size() <= 0)
    {
        ULOG(LOG_ERROR, "The primarySub is empty");
        return;
    }
    // 仅有主波长
    else if (_return.lstAssayTestResult[0].primarySubWaves.size() == 1)
    {
        primaryWave = _return.lstAssayTestResult[0].primarySubWaves[0];
    }
    // 主副波长都存在
    else
    {
        primaryWave = _return.lstAssayTestResult[0].primarySubWaves[0];
        subWave = _return.lstAssayTestResult[0].primarySubWaves[1];
    }

    int indexRow = 0;
    //获取曲线的点
    //临时值
    RealDataStruct tempdata;
    for (auto detect : _return.lstAssayTestResult[0].detectPoints)
    {
        double x = detect.pointSN;
        double y = 0;

        // 主波长--找不到下一个
        if (detect.ods.find(primaryWave) == detect.ods.end())
        {
            continue;
        }

        y = detect.ods[primaryWave];
        tempdata.type = PrimaryWaveCurve.scatterMarkerShape;
        tempdata.color = PrimaryWaveCurve.colorType;
        tempdata.xData = x;
        tempdata.yData = y;
        PrimaryWaveCurve.dataContent.append(tempdata);

        QTableWidgetItem* item = new QTableWidgetItem(QString::number(y));
        if (indexRow < defaultRow)
        {
            ui.tableWidget->setItem(indexRow, 1, item);
        }
        else
        {
            ui.tableWidget->setItem(indexRow - defaultRow, 5, item);
        }

        // 存在次波长--找不到下一个
        if (detect.ods.find(subWave) == detect.ods.end())
        {
            indexRow++;
            continue;
        }

        y = detect.ods[subWave];
        tempdata.type = SubWaveCurve.scatterMarkerShape;
        tempdata.color = SubWaveCurve.colorType;
        tempdata.xData = x;
        tempdata.yData = y;
        SubWaveCurve.dataContent.append(tempdata);

        item = new QTableWidgetItem(QString::number(y));
        if (indexRow < defaultRow)
        {
            ui.tableWidget->setItem(indexRow, 2, item);
        }
        else
        {
            ui.tableWidget->setItem(indexRow - defaultRow, 6, item);
        }

        // 对比曲线
        y = detect.ods[primaryWave] - detect.ods[subWave];
        tempdata.type = CompareWaveCurve.scatterMarkerShape;
        tempdata.color = CompareWaveCurve.colorType;
        tempdata.xData = x;
        tempdata.yData = y;
        CompareWaveCurve.dataContent.append(tempdata);

        item = new QTableWidgetItem(QString::number(y));
        if (indexRow < defaultRow)
        {
            ui.tableWidget->setItem(indexRow, 3, item);
        }
        else
        {
            ui.tableWidget->setItem(indexRow - defaultRow, 7, item);
        }
        indexRow++;
    }

    // 当曲线没有数据的时候，直接返回
    if (PrimaryWaveCurve.dataContent.empty())
    {
        ULOG(LOG_ERROR, "No curve to show");
        return;
    }

    // 添加曲线
    int primarycurveId = ui.ReactionCurve->addCurveData(PrimaryWaveCurve);
    int subWavecurveId = ui.ReactionCurve->addCurveData(SubWaveCurve);
    int minusCurveId = ui.ReactionCurve->addCurveData(CompareWaveCurve);
}

///
/// @brief
///     点击关闭按钮的槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建函数
///
void CurveReactionProcess::OnCloseBtn()
{
    close();
}

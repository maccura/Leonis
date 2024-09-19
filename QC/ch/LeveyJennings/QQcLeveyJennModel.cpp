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
/// @file     QQcLeveyJennModel.cpp
/// @brief    Level Jennings页面Model
///
/// @author   8090/YeHuaNing
/// @date     2022年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QQcLeveyJennModel.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/ReagentCommon.h"
#include "shared/ThriftEnumTransform.h"

#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"

#define  DEFAULT_ROW_CNT_OF_QC_INFO_TABLE                (10)          // 质控信息表默认行数
#define  DEFAULT_COL_CNT_OF_QC_INFO_TABLE                (13)          // 质控信息表默认列数

#define  DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE              (20)          // 质控结果表默认行数
#define  DEFAULT_COL_CNT_OF_QC_RESULT_TABLE              (11)          // 质控结果表默认列数

QQcProjectModel::QQcProjectModel()
{
}


QQcProjectModel::~QQcProjectModel()
{
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
QQcProjectModel& QQcProjectModel::Instance()
{
    static QQcProjectModel model;
    return model;
}

///
/// @brief 更新模式里面的数据
///
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
bool QQcProjectModel::Update(const std::string& device, int assayCode, const QString& startDate, const QString& endDate)
{
    ::tf::QcDocQueryResp _docReturn;
    if (!DcsControlProxy::GetInstance()->QueryQcDocByAssayCode(_docReturn, assayCode)
        || _docReturn.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    if (_docReturn.lstQcDocs.empty())
    {
        return true;
    }

    int startRow = 0;

    // 数据填充
    for (const auto& qcDoc : _docReturn.lstQcDocs)
    {
        setItem(startRow, (int)COLDOC::SELECT, new QStandardItem(""));                                              // 选择
        setItem(startRow, (int)COLDOC::DOCNO, new QStandardItem(QString::fromStdString(qcDoc.sn)));                 // 质控品编号
        setItem(startRow, (int)COLDOC::DOCNAME, new QStandardItem(QString::fromStdString(qcDoc.name)));             // 质控品名称
        setItem(startRow, (int)COLDOC::DOCTYPE, new QStandardItem(ThriftEnumTrans::GetSourceTypeName(qcDoc.sampleSourceType)));      // 质控品类型
        setItem(startRow, (int)COLDOC::DOCLEVEL, new QStandardItem(QString::number(qcDoc.level)));                  // 质控品水平
        setItem(startRow, (int)COLDOC::DOCLOT, new QStandardItem(QString::fromStdString(qcDoc.lot)));               // 质控品批号

        for (const auto& composition : qcDoc.compositions)
        {
            if (composition.assayCode == assayCode)
            {
                setItem(startRow, (int)COLDOC::TARGETVALUE, new QStandardItem(QString::number(composition.targetValue)));               // 靶值
                setItem(startRow, (int)COLDOC::SD, new QStandardItem(QString::number(composition.sd)));                                 // SD
                setItem(startRow, (int)COLDOC::CV, new QStandardItem(QString::number(composition.sd/composition.targetValue*100)));     // CV%
                break;
            }
        }

        GetQcResultDataList(qcDoc.id, device, assayCode, startDate.toStdString(), endDate.toStdString());
              
        ++startRow;
    }

    return true;
}
///
/// @brief 初始化质控
///
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcProjectModel::Init()
{
    clear();

    // 质控信息表
    // 设置表头
    QStringList header;
    header << tr("选择") << tr("质控品编号") << tr("质控品名称")
        << tr("质控品类型") << tr("质控品水平") << tr("质控品批号") << tr("靶值")
        << tr("SD") << tr("CV%") << tr("计算靶值") << tr("计算SD") << tr("计算CV%") << tr("数量");

    setHorizontalHeaderLabels(header);
    setRowCount(DEFAULT_ROW_CNT_OF_QC_INFO_TABLE);
    setColumnCount(DEFAULT_COL_CNT_OF_QC_INFO_TABLE);
}

///
/// @brief 获取计算靶值是否可用
///
/// @param[in]  index  选中索引
///
/// @return true 表示可用
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月16日，新建函数
///
bool QQcProjectModel::IsValidCalcTargetValue(const QModelIndex & index)
{
    QStandardItem* item = this->item(index.row(), (int)COLDOC::COMPUTEVALUE);

    // 计算靶值为0或者 为空的都不能更新靶值
    if (item == nullptr || item->text().isEmpty() || item->text().toDouble() <= 0.0)
    {
            return false;
    }

    return true;
}

///
/// @brief 获取质控文档对应的质控结果记录
///
/// @param[in]  docId  文档ID
/// @param[in]  device  设备序号
/// @param[in]  assayCode  项目编号
/// @param[in]  startDate  开始日期
/// @param[in]  endDate  结束日期
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月16日，新建函数
///
void QQcProjectModel::GetQcResultDataList(int64_t docId, const std::string & device, int assayCode, const std::string & startDate, const std::string & endDate)
{
    // 查询质控结果信息
    ch::tf::QcResultQueryResp _return;
    ch::tf::QcResultQueryCond cdqc;
    cdqc.__set_strDevSN(device);
    cdqc.__set_iAssayCode(assayCode);
    ::tf::TimeRangeCond timeRange;
    timeRange.__set_startTime(startDate + " 00:00:00");
    timeRange.__set_endTime(endDate + " 23:59:59");
    cdqc.__set_ptQcTime(timeRange);
    cdqc.__set_qcDocId(docId);

    ch::LogicControlProxy::QueryQcResult(_return, cdqc);

    if (!_return.lstQcResults.empty())
    {
        double calcValue = 0;
        double calcSd = 0;
        int cnt = 0;

        for (const auto& result : _return.lstQcResults)
        {
            if (result.bCalcSelect)
            {
                calcValue += result.dTargetValue;
                calcSd += result.dSTD;
                ++cnt;
            }
        }

        // 设置计算值
        //if (cnt > 0)
        //{
        //    setItem(startRow, (int)COLDOC::COMPUTEVALUE, new QStandardItem(QString::number(calcValue)));       // 计算靶值
        //    setItem(startRow, (int)COLDOC::COMPUTESD, new QStandardItem(QString::number(calcSd)));          // 计算SD
        //    setItem(startRow, (int)COLDOC::COMPUTECV, new QStandardItem(QString::number(calcSd / calcValue * 100)));          // 计算CV%
        //    setItem(startRow, (int)COLDOC::COUNT, new QStandardItem(QString::number(cnt)));              // 数量  
        //}
        //else
        //{
        //    setItem(startRow, (int)COLDOC::COMPUTEVALUE, new QStandardItem(""));       // 计算靶值
        //    setItem(startRow, (int)COLDOC::COMPUTESD, new QStandardItem(""));          // 计算SD
        //    setItem(startRow, (int)COLDOC::COMPUTECV, new QStandardItem(""));          // 计算CV%
        //    setItem(startRow, (int)COLDOC::COUNT, new QStandardItem(""));              // 数量  
        //}
    }
}

///
/// @brief 获取校准品的位置和杯类型
///
/// @param[in]  doc  校准品信息
///
/// @return 位置和类型字符串
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
QString QQcProjectModel::GetPosAndCupType(const ::tf::QcDoc& doc) const
{
    QString PostionAndtype = "";

    if (doc.rack.empty())
    {
        PostionAndtype = QObject::tr("位置");
    }
    else
    {
        PostionAndtype = QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
    }

    //连接字符&
    PostionAndtype += "&";
    switch (doc.tubeType)
    {
        case ::tf::TubeType::type::TUBE_TYPE_NORMAL:
            PostionAndtype += QObject::tr("标准");
            break;
        case ::tf::TubeType::type::TUBE_TYPE_MICRO:
            PostionAndtype += QObject::tr("微量");
            break;
        case ::tf::TubeType::type::TUBE_TYPE_INVALID:
            PostionAndtype += QObject::tr("无效");
            break;
        default:
            PostionAndtype += QObject::tr("类型");
            break;
    }

    return PostionAndtype;
}

///
/// @brief 初始化代码
///
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLJResultModel::Init()
{
    clear();

    QStringList header;
    header << tr("计算") << tr("序号") << tr("质控结果") << tr("结果") << tr("状态") << tr("失控规则")
        << tr("靶值") << tr("标准差") << tr("失控原因") << tr("处理措施") << tr("操作者");

    setHorizontalHeaderLabels(header);
    setRowCount(DEFAULT_ROW_CNT_OF_QC_RESULT_TABLE);
    setColumnCount(DEFAULT_COL_CNT_OF_QC_RESULT_TABLE);
}

QQcLJResultModel::QQcLJResultModel()
{
    
}

///
/// @brief 判断当前行是否有数据
///
/// @param[in]  index  索引
///
/// @return true 表示有效
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月16日，新建函数
///
bool QQcLJResultModel::IsValidLine(const QModelIndex & index)
{
    QStandardItem* item = this->item(index.row(), (int)COLRESULT::SN);

    // 计算靶值为0或者 为空的都不能更新靶值
    if (item == nullptr || item->text().isEmpty())
    {
        return false;
    }

    return true;
}

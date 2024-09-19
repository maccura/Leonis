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
/// @file     dbgparamcaliwidget.cpp
/// @brief    应用界面->工程师调试->参数（免疫）校准
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "dbgparamcaliwidget.h"
#include "ui_dbgparamcaliwidget.h"
#include <QStandardItemModel>

#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "UtilityCommonFunctions.h"
#include "src/common/Mlog/mlog.h"

#define DEFAULT_EMPTY_ROW_CNT 15 // 空表空行数

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

DbgParamCaliWidget::DbgParamCaliWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false)
{
    ui = new Ui::DbgParamCaliWidget();
    ui->setupUi(this);

    ui->CaliParamTbl->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->CaliParamTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->CaliParamTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->CaliParamTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_tabModel = new QStandardItemModel();
    QStringList headList{ tr("序号"), tr("校准起始点)"), tr("校准终止点"), tr("主标曲起始点"), tr("主标曲终止点"), tr("校准方法") };
    m_tabModel->setHorizontalHeaderLabels(headList);
    ui->CaliParamTbl->setModel(m_tabModel);
    // 列宽自适应
    ResizeTblColToContent(ui->CaliParamTbl);
}

DbgParamCaliWidget::~DbgParamCaliWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

void DbgParamCaliWidget::LoadAnalysisParam(const utilcomm::CaliCurvParam& curvParm)
{
    CleanUi();

    if (curvParm.spanFix2Segment != nullptr)
    {
        // 统计校准品个数(2个/3个)
        ui->CaliDocCntEdit->setText(QString::number(curvParm.spanFix2Segment->size()));
        // 主标曲点数
        ui->MajorCurvePtCntEdit->setText(QString::number(curvParm.spanFix2Segment->size()));
        // 校准段数
        ui->CaliSegCntEdit->setText(QString::number(curvParm.spanFix2Segment->size()));
    }

    AssignCurvTable(curvParm.spanFix2Segment);
}

bool DbgParamCaliWidget::TakeCaliInfo(utilcomm::CaliCurvParam& uiParam)
{
    auto funcTras = [](const QString& caliWay) ->im::SpanFixType {
        caliWay.trimmed();
        if (caliWay == tr("信号值变化率"))
        {
            return im::SpanFixType::IMM_SPAN_FIX_SIGNAL_RATE;
        }
        if (caliWay == tr("倍率法"))
        {
            return im::SpanFixType::IMM_SPAN_FIX_RATIO;
        }
        if (caliWay == tr("信号值法"))
        {
            return im::SpanFixType::IMM_SPAN_FIX_SIGNAL;
        }

        ULOG(LOG_ERROR, "Unkonwn cali way string.");
        return im::SpanFixType::IMM_SPAN_FIX_RATIO;
    };

    std::shared_ptr<im::SpanFix2Segment> spSeg(new im::SpanFix2Segment());

    int rowCnt = m_tabModel->rowCount();
    for (int i = 0; i < rowCnt; ++i)
    {
        QStandardItem* pItemConc = m_tabModel->item(i, 1);
        if (pItemConc == Q_NULLPTR || pItemConc->text().isEmpty())
        {
            break;
        }

        im::SegmentSpanElement curEk;
        curEk.pointIndex.push_back(pItemConc->text().toInt());
        QStandardItem* idx2 = m_tabModel->item(i, 2);
        curEk.pointIndex.push_back(idx2 == Q_NULLPTR ? 0 : idx2->text().toInt());

        QStandardItem* pS = m_tabModel->item(i, 3);
        curEk.startEnd.start = pS == Q_NULLPTR ? 0 : pS->text().toInt();
        QStandardItem* pE = m_tabModel->item(i, 4);
        curEk.startEnd.end = pE == Q_NULLPTR ? 0 : pE->text().toInt();

        QStandardItem* pWay = m_tabModel->item(i, 4);
        if (pWay != Q_NULLPTR)
        {
            curEk.spanFixType = funcTras(pWay->text());
        }

        spSeg->push_back(curEk);
    }

    uiParam.spanFix2Segment = spSeg;

    return true;
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void DbgParamCaliWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
    }
}

void DbgParamCaliWidget::AssignCurvTable(const std::shared_ptr<im::SpanFix2Segment>& spanFix2Segment)
{
    static std::map<im::SpanFixType, QString> dicCaliWay{
        { im::SpanFixType::IMM_SPAN_FIX_SIGNAL_RATE, tr("信号值变化率")},
        { im::SpanFixType::IMM_SPAN_FIX_RATIO, tr("倍率法") },
        { im::SpanFixType::IMM_SPAN_FIX_SIGNAL, tr("信号值法") }
    };

    m_tabModel->removeRows(0, m_tabModel->rowCount());

    // 点信息
    if (spanFix2Segment != nullptr)
    {
        int rowId = 0;
        for (const auto& pt : *spanFix2Segment)
        {
            QList<QStandardItem*> rowList;
            rowList.append(new QStandardItem(QString::number(rowId)));
            // 校准起始点
            if (pt.pointIndex.size() > 1)
            {
                rowList.append(new QStandardItem(QString::number(pt.pointIndex[0])));
                rowList.append(new QStandardItem(QString::number(pt.pointIndex[1])));
            }
            // 主标曲起始点
            rowList.append(new QStandardItem(QString::number(pt.startEnd.start)));
            rowList.append(new QStandardItem(QString::number(pt.startEnd.end)));

            // 校准方法
            std::map<im::SpanFixType, QString>::iterator it = dicCaliWay.find(pt.spanFixType);
            rowList.append(new QStandardItem(it == dicCaliWay.end() ? "" : it->second));

            m_tabModel->appendRow(rowList);

            rowId++;
        }
    }
    // 增加10行空行在末尾
    for (int i = 0; i < DEFAULT_EMPTY_ROW_CNT; ++i)
    {
        m_tabModel->appendRow({ new QStandardItem(""), new QStandardItem(""), new QStandardItem(""),
            new QStandardItem(""), new QStandardItem(""), new QStandardItem("") });
    }
}

void DbgParamCaliWidget::CleanUi()
{
    ui->CaliDocCntEdit->clear();
    ui->MajorCurvePtCntEdit->clear();
    ui->CaliSegCntEdit->clear();

    AssignCurvTable(nullptr);
}

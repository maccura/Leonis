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
/// @file     imcalhiswidget.cpp
/// @brief    免疫校准历史界面
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

#include "imcalhiswidget.h"
#include "ui_imcalhiswidget.h"
#include "src/common/Mlog/mlog.h"
#include "shared/calhiscurve.h"
#include <QVBoxLayout>
#include <QScrollBar>

#define  DEFAULT_ROW_CNT_OF_ASSAY_NAME_TABLE                   (44)          // 耗材信息表默认行数
#define  DEFAULT_COL_CNT_OF_ASSAY_NAME_TABLE                   (1)           // 耗材信息表默认列数

#define  DEFAULT_ROW_CNT_OF_CAL_RESULT_TABLE                   (3)           // 耗材信息表默认行数
#define  DEFAULT_COL_CNT_OF_CAL_RESULT_TABLE                   (7)           // 耗材信息表默认列数

ImCalHisWidget::ImCalHisWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::ImCalHisWidget();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

ImCalHisWidget::~ImCalHisWidget()
{
    delete ui;
}

///
/// @brief
///     切换设备刷新界面
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月08日，新建函数
///
void ImCalHisWidget::RefreshPage(std::vector<std::shared_ptr<tf::DeviceInfo>>& devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (devices.empty())
    {
        ULOG(LOG_WARN, "%s(), devices.empty()", __FUNCTION__);
        return;
    }

    // 记录当前设备
    m_strDevSn = QString::fromStdString(devices.front()->deviceSN);

    // 更新界面显示
    // 待完善
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void ImCalHisWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 项目名称表
    ui->AssayNameTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->AssayNameTbl->verticalHeader()->setVisible(false);

    // 校准结果表
    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setVisible(false);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void ImCalHisWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
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
void ImCalHisWidget::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

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
void ImCalHisWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 项目名称表设置默认行数，列数
    ui->AssayNameTbl->setRowCount(DEFAULT_ROW_CNT_OF_ASSAY_NAME_TABLE);
    ui->AssayNameTbl->setColumnCount(DEFAULT_COL_CNT_OF_ASSAY_NAME_TABLE);

    // 设置表头
    QStringList strAssayHeaderList;
    strAssayHeaderList << tr("模块") << tr("耗材名称") << tr("使用状态") << tr("余量") << tr("耗材批号") << tr("瓶号/序列号") << tr("失效日期") << tr("开瓶有效期(天)") << tr("上机时间");
    ui->AssayNameTbl->setHorizontalHeaderLabels(strAssayHeaderList);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->AssayNameTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->AssayNameTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AssayNameTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 校准结果表设置默认行数，列数
    ui->CalRltTbl->setRowCount(DEFAULT_ROW_CNT_OF_CAL_RESULT_TABLE);
    ui->CalRltTbl->setColumnCount(DEFAULT_COL_CNT_OF_CAL_RESULT_TABLE);

    // 设置表头
    QStringList strCalHeaderList;
    strCalHeaderList << tr("") << tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4") << tr("校准品5") << tr("校准品6");
    ui->CalRltTbl->setHorizontalHeaderLabels(strCalHeaderList);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->CalRltTbl->setSelectionBehavior(QAbstractItemView::SelectColumns);
    ui->CalRltTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->CalRltTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 初始化第一列内容
    ui->CalRltTbl->setItem(0, 0, new QTableWidgetItem(tr("信号值1")));
    ui->CalRltTbl->setItem(1, 0, new QTableWidgetItem(tr("信号值2")));
    ui->CalRltTbl->setItem(2, 0, new QTableWidgetItem(tr("浓度")));

    // 测试
    CalHisCurve* pCalHisCurve = new CalHisCurve();
    //pCalHisCurve->SetCurveName(tr("校准品1"), "校准品2");
   //pCalHisCurve->testData();
    QVBoxLayout* pHLayout = new QVBoxLayout();
    QScrollBar* pTestScrollBar = new QScrollBar();
    pTestScrollBar->setRange(0, 25);
    pTestScrollBar->setSingleStep(1);
    pTestScrollBar->setOrientation(Qt::Horizontal);
    QObject::connect(pTestScrollBar, SIGNAL(valueChanged(int)), pCalHisCurve, SLOT(TestRangeChanged(int)));
    pHLayout->setSpacing(10);
    pHLayout->setMargin(0);
    pHLayout->addWidget(pCalHisCurve);
    pHLayout->addWidget(pTestScrollBar);
    ui->CalHisCurveWgt->setLayout(pHLayout);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void ImCalHisWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

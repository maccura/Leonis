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
/// @file     QQcLeveyJennPage.cpp
/// @brief    L-J质控图界面
///
/// @author   8090/Yehuaning
/// @date     2022年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/Yehuaning，2022年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QQcLeveyJennPage.h"
#include "ui_QQcLeveyJennPage.h"

#include <memory>
#include <algorithm>
#include <QStandardItemModel>
#include <QVariant>
#include <QTimer>

#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include "QQcLeveyJennModel.h"
#include "LeveyJennPlot.h"

QQcLeveyJennPage::QQcLeveyJennPage(QWidget *parent)
    : QWidget(parent),
    m_bInit(false)
{
    // 初始化UI对象
    ui = new Ui::QQcLeveyJennPage();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

QQcLeveyJennPage::~QQcLeveyJennPage()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief 启动Levey Jennings页面的列表显示
///
/// @param[in]  devices  设备列表
///
/// @return 成功，返回true
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
bool QQcLeveyJennPage::StartLeveyJennAppPage(DeviceModules& devices)
{
    m_devInfos = devices;

    emit DeviceChanged();

    return true;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::InitBeforeShow()
{
    ui->QcInfoTable->setModel(&QQcProjectModel::Instance());

    // 质控结果表
    m_pTbl1Model = new QQcLJResultModel();
    m_pTbl2Model = new QQcLJResultModel();
    m_pTbl3Model = new QQcLJResultModel();
    ui->QcResultTbl1->setModel(m_pTbl1Model);
    ui->QcResultTbl2->setModel(m_pTbl2Model);
    ui->QcResultTbl3->setModel(m_pTbl3Model);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::InitAfterShow()
{
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::InitChildCtrl()
{
    InitTableView();

    // 初始化项目选择框信息
    ChAssayIndexUniqueCodeMaps      assayMap;
    CommonInformationManager::GetInstance()->GetChAssayMaps(::tf::DeviceType::DEVICE_TYPE_C1000, assayMap);

    for (const auto& assay : assayMap)
    {
        if (assay.second != nullptr && assay.second->enable)
        {
            std::shared_ptr<tf::GeneralAssayInfo> info = CommonInformationManager::GetInstance()->GetAssayInfo(assay.second->assayCode);
            if (info != nullptr)
                ui->AssayCombo->addItem(QString::fromStdString(info->assayName), assay.second->assayCode);
        }
    }

    // 初始化日期信息
    ui->QcEndDateEdit->setDate(QDate::currentDate());
}

///
/// @brief 初始化表格内容
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::InitTableView()
{
    // 设置列宽适应内容
    ui->QcResultTbl1->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->QcResultTbl2->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->QcResultTbl3->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    // 最小列宽
    // 根据内容调整列宽
    ResizeTblColToContent(ui->QcInfoTable);
    ResizeTblColToContent(ui->QcResultTbl1);
    ResizeTblColToContent(ui->QcResultTbl2);
    ResizeTblColToContent(ui->QcResultTbl3);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::InitConnect()
{
    // 质控查询条件改变
    // 查询项目发生变化
    connect(ui->AssayCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        Q_UNUSED(index);
        OnAssayOrDeviceChanged();
    });

    // 查询设备发生了变化
    connect(this, &QQcLeveyJennPage::DeviceChanged, this, &QQcLeveyJennPage::OnAssayOrDeviceChanged);

    // 起始日期发生变化
    connect(ui->QcStartDateEdit, &QDateTimeEdit::dateChanged, this, [=](const QDate& date) {
        if ((date > ui->QcEndDateEdit->date()) || date.daysTo(ui->QcEndDateEdit->date()) > 60)
            ui->QcEndDateEdit->setDate(date.addDays(60));
    });

    // 终止日期发生变化
    connect(ui->QcEndDateEdit, &QDateTimeEdit::dateChanged, this, [=](const QDate& date) {
        if ((date < ui->QcEndDateEdit->date()) || ui->QcStartDateEdit->date().daysTo(date) > 60)
            ui->QcStartDateEdit->setDate(date.addDays(-60));
    });

    connect(this, &QQcLeveyJennPage::AssayCodeChangde, &QQcProjectModel::Instance(), &QQcProjectModel::Update);

    // 质控结果表细节按钮被点击
    connect(ui->DetailBtn, &QPushButton::clicked, this, [=] {});

    // 失控处理按钮被点击
    connect(ui->OutCtrlBtn, &QPushButton::clicked, this, [=] {});

    // 更新靶值SD按钮被点击
    connect(ui->TargetValUpdateBtn, &QPushButton::clicked, [=] {});

    // 质控结果表单元格被点击
    connect(ui->QcResultTbl1, &QAbstractItemView::clicked, this, [=] {});
    connect(ui->QcResultTbl2, &QAbstractItemView::clicked, this, [=] {});
    connect(ui->QcResultTbl3, &QAbstractItemView::clicked, this, [=] {});

    // 连接初始化信号
    connect(this, &QQcLeveyJennPage::InitViewData, &QQcProjectModel::Instance(), &QQcProjectModel::Init);
    connect(this, &QQcLeveyJennPage::InitViewData, m_pTbl1Model, &QQcLJResultModel::Init);
    connect(this, &QQcLeveyJennPage::InitViewData, m_pTbl2Model, &QQcLJResultModel::Init);
    connect(this, &QQcLeveyJennPage::InitViewData, m_pTbl3Model, &QQcLJResultModel::Init);

    // 表格选择行状态变化
    connect(ui->QcInfoTable->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex &current, const QModelIndex &previous) {
        if (!current.isValid())
        {
            ui->TargetValUpdateBtn->setEnabled(false);
        }
        else
        {
            // 设置控件是否显示
            ui->TargetValUpdateBtn->setEnabled(QQcProjectModel::Instance().IsValidCalcTargetValue(current));
        }
    });

    // 质控结果TabWidget表格切换
    connect(ui->QcResultTabWidget, &QTabWidget::currentChanged, this, [=](int index) {
        QString tableViewName = QString("QcResultTbl") + QString::number(index);
        QTableView* table = ui->QcResultTabWidget->findChild<QTableView*>(tableViewName);

        if (table == nullptr)
        {
            ULOG(LOG_ERROR, "%s%s", "Can not find ", tableViewName.toStdString().c_str());
            ui->OutCtrlBtn->setEnabled(false);
            ui->DetailBtn->setEnabled(false);
        }
        else
        {
            QQcLJResultModel* resultModel = static_cast<QQcLJResultModel*>(table->model());

            bool valid = table->selectionModel()->currentIndex().isValid() ||
                resultModel == nullptr || resultModel->IsValidLine(table->selectionModel()->currentIndex());

            ui->OutCtrlBtn->setEnabled(valid);
            ui->DetailBtn->setEnabled(valid);
        }
    });

    // 初始化完成，开始请求数据
    emit ui->AssayCombo->currentIndexChanged(ui->AssayCombo->currentIndex());
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建函数
///
void QQcLeveyJennPage::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief 项目编号或者设备切换
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月17日，新建函数
///
void QQcLeveyJennPage::OnAssayOrDeviceChanged()
{
    if (ui->AssayCombo->currentIndex() < 0)
        return;

    // 项目发生变化清理所有数据
    emit InitViewData();
    emit  AssayCodeChangde(m_devInfos[0]->deviceSN, ui->AssayCombo->currentData().toInt(), ui->QcStartDateEdit->date().toString("yyyy-MM-dd"), ui->QcEndDateEdit->date().toString("yyyy-MM-dd"));
    // 触发一次质控品列表选区变化消息
    emit ui->QcInfoTable->selectionModel()->currentChanged(QModelIndex(), QModelIndex());
    // 触发一次质控结果切换页面消息
    emit ui->QcResultTabWidget->currentChanged(ui->QcResultTabWidget->currentIndex());
}

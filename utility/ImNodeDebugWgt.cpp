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
/// @file     ImNodeDebugWgt.cpp
/// @brief    生化节点调试界面
///
/// @author   4170/TangChuXian
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImNodeDebugWgt.h"
#include "ui_ImNodeDebugWgt.h" 
#include "ImStepModuleWgt.h"
#include "ImPumpModuleWgt.h"
#include "ImValveModuleWgt.h"
#include "ImDevFacadeModuleWgt.h"
#include "ImCustomNodeWgt.h"
#include <boost/filesystem.hpp>
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "shared/uicommon.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared\QComDelegate.h"
#include <QFileDialog>
#include <QWhatsThis>

// 声明元类型
Q_DECLARE_METATYPE(im::tf::DebugModule)

ImNodeDebugWgt::ImNodeDebugWgt(QWidget *parent)
    : QWidget(parent)
    , m_bInit(false)
    , ui(new Ui::ImNodeDebugWgt)
{
    ui->setupUi(this);

	InitUi();
	InitData();
	InitConnect();
}

ImNodeDebugWgt::~ImNodeDebugWgt()
{
}

void ImNodeDebugWgt::UpdateUi(const string& devSN)
{
	m_devSN = devSN;

    // 更新模块列表
    UpdateModuleList();
}

///
/// @brief
///     导出文件
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
void ImNodeDebugWgt::ExportFIle()
{
    // 获取导出文件位置
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("files (*.json);"));
    if (fileName.isNull() || fileName.isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
        pTipDlg->exec();
        return;
    }

    // 调用后台接口
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::ExportParamFile(retl, m_devSN, fileName.toStdString()) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), ExportParamFile failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("导出失败!")));
        pTipDlg->exec();
        return;
    }

    // 弹框提示导出成功
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("导出成功!")));
    pTipDlg->exec();
}

///
/// @brief
///     导入文件
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
void ImNodeDebugWgt::ImportFile()
{
    // 获取要导入的文件
    QString fileName = QFileDialog::getOpenFileName(this, tr("节点调试导入"), "", tr("节点调试文件(*.*)"));
    if (fileName.isEmpty())
    {
        ULOG(LOG_INFO, "%s(), not sel file", __FUNCTION__);
        return;
    }

    // 调用后台接口
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::ImportParamFile(retl, m_devSN, fileName.toStdString()) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), ImportParamFile failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("导入失败!")));
        pTipDlg->exec();
        return;
    }

    // 弹框提示导出成功
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("导入成功!")));
    pTipDlg->exec();

    // 刷新界面
    UpdateUi(m_devSN);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ImNodeDebugWgt::showEvent(QShowEvent *event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果不是第一次显示，则刷新界面
    if (m_bInit)
    {
        // 刷新表格
        UpdateUi(m_devSN);
    }
    else
    {
        m_bInit = true;
    }
}

void ImNodeDebugWgt::InitUi()
{
	// 只有一列
    ui->ModuleLstTbl->setColumnCount(1);

    // 表头
    QStringList strHeaderList;
    strHeaderList << tr("节点调试");
    ui->ModuleLstTbl->setHorizontalHeaderLabels(strHeaderList);
    ui->ModuleLstTbl->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->ModuleLstTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ModuleLstTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ModuleLstTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->ModuleLstTbl->horizontalHeader()->setStretchLastSection(true);

    auto headView = ui->ModuleLstTbl->horizontalHeader();
    m_pWhatsThisBtn = new CustomButton("", headView);
    m_pWhatsThisBtn->setObjectName(QStringLiteral("tooltip_btn"));
    // 设置按钮的位置
    m_pWhatsThisBtn->setGeometry(headView->sectionViewportPosition(0) + headView->sectionSize(0) - 35, 15, 50, headView->height());
}

void ImNodeDebugWgt::InitData()
{
	m_stepModuleWgt = new ImStepModuleWgt(this);
 	m_pumpModuleWgt = new ImPumpModuleWgt(this);
 	m_valveModuleWgt = new ImValveModuleWgt(this);
 	m_pLedModuleWgt = new ImDevFacadeModuleWgt(this);
    m_pImCustomNodeWgt = new ImCustomNodeWgt(this);
	ui->stackedWidget_center->insertWidget(STACK_PAGE_IDX_STEP, m_stepModuleWgt);
 	ui->stackedWidget_center->insertWidget(STACK_PAGE_IDX_PUMP, m_pumpModuleWgt);
 	ui->stackedWidget_center->insertWidget(STACK_PAGE_IDX_VALVE, m_valveModuleWgt);
 	ui->stackedWidget_center->insertWidget(STACK_PAGE_IDX_FACADE, m_pLedModuleWgt);
    ui->stackedWidget_center->insertWidget(STACK_PAGE_IDX_CUSTOM, m_pImCustomNodeWgt);
}

void ImNodeDebugWgt::InitConnect()
{
    // 问号按钮被点击
    connect(m_pWhatsThisBtn, SIGNAL(clicked()), this, SLOT(OnWhatsThisBtnClicked()));

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->ModuleLstTbl->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
    }

    REGISTER_HANDLER(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, this, OnDbgModuleUpdate);
}

///
/// @brief
///     选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月7日，新建函数
///
void ImNodeDebugWgt::OnSelectListItem()
{
    // 获取选中索引
    QModelIndex selIdx = ui->ModuleLstTbl->currentIndex();
    if (!selIdx.isValid() || ui->ModuleLstTbl->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        im::tf::DebugModule stuEmptyTfDbgMd;
        m_stepModuleWgt->UpdateUi(stuEmptyTfDbgMd);
        ui->stackedWidget_center->setCurrentIndex(STACK_PAGE_IDX_STEP);
        return;
    }

    // 获取模块信息
    im::tf::DebugModule stuTfDbgMd = ui->ModuleLstTbl->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugModule>();

    // 记录索引
    StackPageIdx enPageIdx = STACK_PAGE_IDX_STEP;

    // 模块类型
    if (stuTfDbgMd.moduleType == im::tf::ModuleType::E_NORMAL)
    {
        m_stepModuleWgt->UpdateDevSn(m_devSN);
        m_stepModuleWgt->UpdateUi(stuTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_STEP;
    }
    else if (QString::fromStdString(stuTfDbgMd.ModuleName) == tr("泵"))
    {
        m_pumpModuleWgt->UpdateDevSn(m_devSN);
        m_pumpModuleWgt->UpdateUi(stuTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_PUMP;
    }
    else if (QString::fromStdString(stuTfDbgMd.ModuleName) == tr("阀"))
    {
        m_valveModuleWgt->UpdateDevSn(m_devSN);
        m_valveModuleWgt->UpdateUi(stuTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_VALVE;
    }
    else if (QString::fromStdString(stuTfDbgMd.ModuleName) == tr("指示灯"))
    {
        m_pLedModuleWgt->UpdateDevSn(m_devSN);
        m_pLedModuleWgt->UpdateUi(stuTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_FACADE;
    }
    else if (QString::fromStdString(stuTfDbgMd.ModuleName) == tr("自定义参数"))
    {
        m_pImCustomNodeWgt->UpdateDevSn(m_devSN);
        m_pImCustomNodeWgt->UpdateUi(stuTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_CUSTOM;
    }
    else
    {
        im::tf::DebugModule stuEmptyTfDbgMd;
        m_stepModuleWgt->UpdateDevSn(m_devSN);
        m_stepModuleWgt->UpdateUi(stuEmptyTfDbgMd);
        enPageIdx = STACK_PAGE_IDX_STEP;
    }

    // 根据模块类型切换页面
    ui->stackedWidget_center->setCurrentIndex(enPageIdx);

    // 表格列宽自适应
    ResizeTblColToContent(ui->ModuleLstTbl);
}

///
/// @brief
///     更新模块列表
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月2日，新建函数
///
void ImNodeDebugWgt::UpdateModuleList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 记录之前的选中项
    QString strSelModule("");

    // 获取选中索引
    QModelIndex selIdx = ui->ModuleLstTbl->currentIndex();
    if (selIdx.isValid() && ui->ModuleLstTbl->item(selIdx.row(), 0) != Q_NULLPTR)
    {
        strSelModule = ui->ModuleLstTbl->item(selIdx.row(), 0)->text();
    }

    // 清空列表内容
    ui->ModuleLstTbl->setRowCount(0);

    // 获取模块
    im::tf::DebugModuleQueryResp dbgModuleResp;
    if (!im::LogicControlProxy::GetDebugModules(m_devSN, dbgModuleResp))
    {
        ULOG(LOG_ERROR, "%s(), GetDebugModules failed!", __FUNCTION__);
        return;
    }

    // 将模块加载到列表中
    int iRow = 0;
    for (const im::tf::DebugModule& stuTfDbgMd : dbgModuleResp.lstDebugModules)
    {
        // 行数不足则自增
        if (iRow >= ui->ModuleLstTbl->rowCount())
        {
            ui->ModuleLstTbl->setRowCount(iRow + 1);
        }

        // 插入数据
        auto *item = new QTableWidgetItem(QString::fromStdString(stuTfDbgMd.ModuleName));
        item->setData(Qt::UserRole, QVariant::fromValue<im::tf::DebugModule>(stuTfDbgMd));
        ui->ModuleLstTbl->setItem(iRow, 0, item);

        // 如果模块匹配则选中
        if (!strSelModule.isEmpty() && strSelModule == QString::fromStdString(stuTfDbgMd.ModuleName))
        {
            ui->ModuleLstTbl->selectRow(iRow);
        }

        ++iRow;
    }

    // 插入自定义参数
    im::tf::DebugModule stuTfCustomDbgMd;
    stuTfCustomDbgMd.__set_ModuleName(tr("自定义参数").toStdString());
    stuTfCustomDbgMd.__set_moduleType(im::tf::ModuleType::E_SWITCHOVER);
    auto *item = new QTableWidgetItem(tr("自定义参数"));
    item->setData(Qt::UserRole, QVariant::fromValue<im::tf::DebugModule>(stuTfCustomDbgMd));

    // 行数不足则自增
    if (iRow >= ui->ModuleLstTbl->rowCount())
    {
        ui->ModuleLstTbl->setRowCount(iRow + 1);
    }
    ui->ModuleLstTbl->setItem(iRow, 0, item);

    // 之前未选中则选中第一行
    if (strSelModule.isEmpty())
    {
        ui->ModuleLstTbl->selectRow(0);
    }
}

///
/// @brief
///     调试模块更新
///
/// @param[in]  stuTfDbgMd  调试模块
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月26日，新建函数
///
void ImNodeDebugWgt::OnDbgModuleUpdate(const im::tf::DebugModule& stuTfDbgMd)
{
    // 遍历模块表
    for (int iRow = 0; iRow < ui->ModuleLstTbl->rowCount(); iRow++)
    {
        // 获取缓存项
        QTableWidgetItem* pItem = ui->ModuleLstTbl->item(iRow, 0);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 判断是否是要更新项
        im::tf::DebugModule stuDbgMd = pItem->data(Qt::UserRole).value<im::tf::DebugModule>();
        if (stuDbgMd.ModuleName != stuTfDbgMd.ModuleName)
        {
            continue;
        }

        // 设置对应数据
        pItem->setData(Qt::UserRole, QVariant::fromValue<im::tf::DebugModule>(stuTfDbgMd));
    }
}

///
/// @brief
///     WhatsThis按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月8日，新建函数
///
void ImNodeDebugWgt::OnWhatsThisBtnClicked()
{
    QWhatsThis::enterWhatsThisMode();
}

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
/// @file     requirecalcwidget.cpp
/// @brief    需求计算信息界面
///
/// @author   4170/TangChuXian
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "requirecalcwidget.h"
#include "ui_requirecalcwidget.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/datetimefmttool.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/QComDelegate.h"
#include "uidcsadapter/uidcsadapter.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"
#include "Serialize.h"
#include "printcom.hpp"

Q_DECLARE_METATYPE(SPL_REQ_VOL_INFO)

// 声明静态成员变量
const int                       RequireCalcWidget::sm_ciReqCalcTblDefaultRowCnt = 30;           // 需求计算表默认行数
const int                       RequireCalcWidget::sm_ciReqCalcTblDefaultColCnt = 32;           // 需求计算表默认列数
QList<SPL_REQ_VOL_INFO>         RequireCalcWidget::sm_splReqInfoLst;                            // 耗材需求信息列表
QStandardItemModel*             RequireCalcWidget::sm_pStdModel = Q_NULLPTR;                    // 数据模型
bool                            RequireCalcWidget::sm_bHideNormaData = false;                   // 隐藏正常数据
QSet<RequireCalcWidget*>        RequireCalcWidget::sm_setAllInstance;                           // 管理所有实例

RequireCalcWidget::RequireCalcWidget(Qt::DayOfWeek enDayOfWeek, QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_cenDayOfWeek(enDayOfWeek)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化UI对象
    ui = new Ui::RequireCalcWidget();
    ui->setupUi(this);

    // 记录实例
    sm_setAllInstance.insert(this);

    // 界面显示前初始化
    InitBeforeShow();
}

RequireCalcWidget::~RequireCalcWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 移除实例
    sm_setAllInstance.remove(this);
    delete ui;
}

///
/// @brief
///     加载数据
///
/// @param[in]  iDevClassify  设备类别
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月28日，新建函数
///
void RequireCalcWidget::LoadData(int iDevClassify)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 读取耗材需求信息
    if (!gUiAdapterPtr()->GetSplReqInfo(sm_splReqInfoLst, iDevClassify))
    {
        ULOG(LOG_ERROR, "%s(), GetSplReqInfo() failed", __FUNCTION__);
        return;
    }

    // 暂停监听需求量和瓶缺口
    for (RequireCalcWidget* pInstance : sm_setAllInstance)
    {
        pInstance->ui->RequireCalcTbl->setModel(Q_NULLPTR);
        disconnect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), pInstance, SLOT(OnItemChanged(QStandardItem*)));
    }

    // 将数据加载到model中
    // 清空表格内容
    sm_pStdModel->setRowCount(0);
    sm_pStdModel->setRowCount(qMax(sm_ciReqCalcTblDefaultRowCnt, sm_splReqInfoLst.size()));

    // 所有单元格均设置为有Item，不可编辑
    int iRowCnt = sm_pStdModel->rowCount();
    int iColCnt = sm_pStdModel->columnCount();
    for (int iRow = 0; iRow < iRowCnt; iRow++)
    {
        // 设置不可编辑
        for (int iCol = 0; iCol < iColCnt; iCol++)
        {
            // 设置Item
            QStandardItem* pItem = new QStandardItem("");
            Qt::ItemFlags flagsItem = pItem->flags();
            pItem->setFlags(flagsItem & (~Qt::ItemIsEditable));
            sm_pStdModel->setItem(iRow, iCol, pItem);
        }
    }

    // 将数据加载到质控申请表中
    int iRow = 0;
    for (const auto& stuSplReqInfo : sm_splReqInfoLst)
    {
        // 如果行号大于等于行数，则行数自动增加（始终预留一行空白行）
        if (iRow >= sm_pStdModel->rowCount())
        {
            sm_pStdModel->setRowCount(iRow + 1);
        }

        // 动态计算值
        if (stuSplReqInfo.vecStrReqDynCalcVal.size() >= Qt::Sunday)
        {
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtMon, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Monday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtTue, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Tuesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtWed, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Wednesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtThur, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Thursday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtFri, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Friday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtSat, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Saturday - 1]));
            sm_pStdModel->setItem(iRow, Rch_DynamicCalcAtSun, new QStandardItem(stuSplReqInfo.vecStrReqDynCalcVal[Qt::Sunday - 1]));
        }

        // 需求量
        if (stuSplReqInfo.vecStrReqVal.size() >= Qt::Sunday)
        {
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtMon, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Monday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtTue, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Tuesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtWed, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Wednesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtThur, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Thursday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtFri, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Friday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtSat, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Saturday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqVolAtSun, new QStandardItem(stuSplReqInfo.vecStrReqVal[Qt::Sunday - 1]));
        }

        // 需求缺口
        if (stuSplReqInfo.vecStrLackingVal.size() >= Qt::Sunday)
        {
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtMon, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Monday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtTue, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Tuesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtWed, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Wednesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtThur, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Thursday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtFri, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Friday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtSat, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Saturday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqLackAtSun, new QStandardItem(stuSplReqInfo.vecStrLackingVal[Qt::Sunday - 1]));
        }

        // 需加载瓶数
        if (stuSplReqInfo.vecStrBottleLacking.size() >= Qt::Sunday)
        {
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtMon, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Monday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtTue, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Tuesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtWed, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Wednesday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtThur, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Thursday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtFri, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Friday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtSat, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Saturday - 1]));
            sm_pStdModel->setItem(iRow, Rch_ReqBottleCountAtSun, new QStandardItem(stuSplReqInfo.vecStrBottleLacking[Qt::Sunday - 1]));
        }

        // 添加数据
        sm_pStdModel->setItem(iRow, Rch_AssayName, new QStandardItem(stuSplReqInfo.strSplName));
        sm_pStdModel->setItem(iRow, Rch_ModuleRemain, new QStandardItem(stuSplReqInfo.strDevResidual));
        sm_pStdModel->setItem(iRow, Rch_RemainVol, new QStandardItem(stuSplReqInfo.strCurResidual));
        sm_pStdModel->setItem(iRow, Rch_BottleSpeciffic, new QStandardItem(stuSplReqInfo.strBottleSpec));

        // 设置需求量和规格可编辑
        for (int iCol = 0; iCol < sm_pStdModel->columnCount(); iCol++)
        {
            QStandardItem* pItem = sm_pStdModel->item(iRow, iCol);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }
            Qt::ItemFlags flagsItem = pItem->flags();
            if ((iCol < Rch_ReqVolAtMon || iCol > Rch_ReqVolAtSun) && iCol != Rch_BottleSpeciffic)
            {
                // 不可编辑
                pItem->setFlags(flagsItem & (~Qt::ItemIsEditable));
            }
            else
            {
                // 可编辑
                pItem->setFlags(flagsItem | Qt::ItemIsEditable);
            }
        }

        // 行号自增
        ++iRow;
    }

    // 单元格内容居中显示
    SetTblTextAlign(sm_pStdModel, Qt::AlignCenter);

    // 更新余量不足提示
    UpdateAllTblLackTipLab();

    // 更新当前表格余量文本颜色
    UpdateCurTblRemainValTextColor();

    // 监听需求量和瓶缺口
    for (RequireCalcWidget* pInstance : sm_setAllInstance)
    {
        pInstance->ui->RequireCalcTbl->setModel(sm_pStdModel);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), pInstance, SLOT(OnItemChanged(QStandardItem*)));
    }

    // 隐藏无关列
    UpdateAllTblColVisible();

    // 隐藏无关行
    UpdateAllTblRowVisible();
}

///
/// @brief
///     初始化数据模型
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月28日，新建函数
///
void RequireCalcWidget::InitModel()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果模型已经初始化，则忽略
    if (sm_pStdModel != Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 创建模型
    sm_pStdModel = new QStandardItemModel();

    // 加载模型
    sm_pStdModel->setRowCount(sm_ciReqCalcTblDefaultRowCnt);
    sm_pStdModel->setColumnCount(sm_ciReqCalcTblDefaultColCnt);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<ReqCalcHeader, QString>> recCalcHeader = {
        { Rch_AssayName, tr("名称") },
        { Rch_ModuleRemain, tr("模块余量分布") },
        { Rch_DynamicCalcAtMon, tr("动态计算") },
        { Rch_DynamicCalcAtTue, tr("动态计算") },
        { Rch_DynamicCalcAtWed, tr("动态计算") },
        { Rch_DynamicCalcAtThur, tr("动态计算") },
        { Rch_DynamicCalcAtFri, tr("动态计算") },
        { Rch_DynamicCalcAtSat, tr("动态计算") },
        { Rch_DynamicCalcAtSun, tr("动态计算") },
        { Rch_ReqVolAtMon, tr("需求量") },
        { Rch_ReqVolAtTue, tr("需求量") },
        { Rch_ReqVolAtWed, tr("需求量") },
        { Rch_ReqVolAtThur, tr("需求量") },
        { Rch_ReqVolAtFri, tr("需求量") },
        { Rch_ReqVolAtSat, tr("需求量") },
        { Rch_ReqVolAtSun, tr("需求量") },
        { Rch_RemainVol, tr("当前余量") },
        { Rch_ReqLackAtMon, tr("需求缺口") },
        { Rch_ReqLackAtTue, tr("需求缺口") },
        { Rch_ReqLackAtWed, tr("需求缺口") },
        { Rch_ReqLackAtThur, tr("需求缺口") },
        { Rch_ReqLackAtFri, tr("需求缺口") },
        { Rch_ReqLackAtSat, tr("需求缺口") },
        { Rch_ReqLackAtSun, tr("需求缺口") },
        { Rch_BottleSpeciffic, tr("瓶规格") },
        { Rch_ReqBottleCountAtMon, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtTue, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtWed, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtThur, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtFri, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtSat, tr("需载入瓶数") },
        { Rch_ReqBottleCountAtSun, tr("需载入瓶数") }
    };
    QStringList strHeaderList;
    for (auto it = recCalcHeader.begin(); it != recCalcHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    sm_pStdModel->setHorizontalHeaderLabels(strHeaderList);

    // 需求量和瓶规格显示蓝色
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtMon)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtTue)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtWed)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtThur)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtFri)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtSat)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_ReqVolAtSun)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
    sm_pStdModel->horizontalHeaderItem(Rch_BottleSpeciffic)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化模型
    InitModel();

    // 隐藏提示
    ui->TipLab->setVisible(false);

    // 获取日期
    QDate curDate = QDate::currentDate();
    QDate displayDate = curDate.addDays(int(m_cenDayOfWeek) - curDate.dayOfWeek());
    ui->DateLab->setText(ToCfgFmtDateTime(displayDate.toString(UI_DATE_FORMAT)));

    // 禁能排序
    ui->RequireCalcTbl->setSortingEnabled(false);

    // 创建模型
    ui->RequireCalcTbl->setModel(sm_pStdModel);

    // 设置代理
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtMon, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtTue, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtWed, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtThur, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtFri, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtSat, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_ReqVolAtSun, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_BottleSpeciffic, new InputLimitDelegate(UI_REG_REQ_VAL, this));
    ui->RequireCalcTbl->setItemDelegateForColumn(Rch_RemainVol, new CReadOnlyDelegate(this));
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void RequireCalcWidget::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->AllRBtn->setText(tr("全部") + QString(10, ' '));
    ui->RemainValRBtn->setText(tr("余量不足") + QString(10, ' '));
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 显示条件改变
    connect(ui->RemainValRBtn, SIGNAL(toggled(bool)), this, SLOT(SetJustShowResidualLackingData(bool)));

    // 监听需求量和瓶缺口
    connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表格选中模式为行选中，不可多选
    ui->RequireCalcTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RequireCalcTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->RequireCalcTbl->setEditTriggers(QAbstractItemView::DoubleClicked);

    // 表头不高亮
    ui->RequireCalcTbl->horizontalHeader()->setHighlightSections(false);

    // 初始化需求信息表，隐藏无关列
    UpdateColumnVisible();

    // 更新耗材需求信息表
    //UpdateSplReqTbl();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcWidget::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    // 显示时更新余量文本颜色
    UpdateRemainValTextColor();
}

///
/// @brief
///     更新耗材需求表
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
void RequireCalcWidget::UpdateSplReqTbl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 列宽自适应
    ResizeTblColToContent(ui->RequireCalcTbl);
}

///
/// @brief
///     设置是否只显示余量不足的数据
///
/// @param[in]  bJustShowLack  是否只显示余量不足的数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月28日，新建函数
///
void RequireCalcWidget::SetJustShowResidualLackingData(bool bJustShowLack)
{
    // 遍历表格
    for (int iRow = 0; iRow < sm_pStdModel->rowCount() && iRow < sm_splReqInfoLst.size(); iRow++)
    {
        // 默认显示
        ui->RequireCalcTbl->showRow(iRow);

        // 获取需求缺口列
        if (!bJustShowLack)
        {
            continue;
        }

        // 需求缺口为0则隐藏
        if (sm_splReqInfoLst[iRow].vecILackingVal.size() < Qt::Sunday || sm_splReqInfoLst[iRow].vecILackingVal[m_cenDayOfWeek- 1] <= 0)
        {
            ui->RequireCalcTbl->hideRow(iRow);
        }
    }
}

///
/// @brief
///     表格单元项改变
///
/// @param[in]  pChangedItem  改变的表格单元项
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月28日，新建函数
///
void RequireCalcWidget::OnItemChanged(QStandardItem* pChangedItem)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (pChangedItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pChangedItem == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 如果修改的是瓶规格，则更新需载入瓶数
    int iRow = pChangedItem->row();
    int iModCol = pChangedItem->column();
    QStandardItem* pOpItem = Q_NULLPTR;
    if (iModCol == Rch_BottleSpeciffic)
    {
        // 获取需求缺口和瓶规格
        pOpItem = sm_pStdModel->item(iRow, Rch_ReqLackAtMon + m_cenDayOfWeek - 1);
        if (pOpItem == Q_NULLPTR)
        {
            ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
            return;
        }
        int iLack = pOpItem->text().remove("ml").toInt();

        pOpItem = sm_pStdModel->item(iRow, Rch_BottleSpeciffic);
        if (pOpItem == Q_NULLPTR)
        {
            ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
            return;
        }
        int iSpeciffic = pOpItem->text().remove("ml").toInt();

        // 更新需载入瓶数
        pOpItem = sm_pStdModel->item(iRow, Rch_ReqBottleCountAtMon + m_cenDayOfWeek - 1);
        if (pOpItem == Q_NULLPTR || iSpeciffic == 0)
        {
            ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR || iSpeciffic == 0", __FUNCTION__);
            return;
        }
        int iLackBottleCnt = iLack <= 0 ? 0 : (iLack - 1) / iSpeciffic + 1;
        pOpItem->setText(iLackBottleCnt <= 0 ? "" : QString::number(iLackBottleCnt));

        // 重新连接信号槽
        return;
    }

    // 修改的不是需求量，则忽略
    if (iModCol < Rch_ReqVolAtMon || iModCol > Rch_ReqVolAtSun)
    {
        return;
    }

    // 如果界面未显示则忽略
    if (!isVisible())
    {
        return;
    }

    // 如果启用使用动态计算值，则忽略
    // 解码
    RgntReqCalcCfg stuReqCalcCfg;
    if (!DictionaryQueryManager::GetDynCalcConfig(stuReqCalcCfg))
    {
        return;
    }

    // 如果启用使用动态计算值则忽略
    if (stuReqCalcCfg.bEnable)
    {
        return;
    }

    // 断开信号槽连接
    disconnect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));

    // 更新需求缺口和需载入瓶数
    // 需求量
    pOpItem = sm_pStdModel->item(iRow, Rch_ReqVolAtMon + m_cenDayOfWeek - 1);
    if (pOpItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
        return;
    }
    int iReqVal = pOpItem->text().remove("ml").toInt();

    // 余量
    pOpItem = sm_pStdModel->item(iRow, Rch_RemainVol);
    if (pOpItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
        return;
    }
    int iRemainVal = pOpItem->text().remove("ml").toInt();
    QString strUnit = pOpItem->text().contains("ml") ? "ml" : "";

    // 设置文本颜色
    if (iRemainVal < iReqVal)
    {
        pOpItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
    }
    else
    {
        pOpItem->setData(QVariant(), Qt::TextColorRole);
    }

    // 更新需求缺口
    int iLackVal = (iRemainVal >= iReqVal) ? 0 : (iReqVal - iRemainVal);
    pOpItem = sm_pStdModel->item(iRow, Rch_ReqLackAtMon + m_cenDayOfWeek - 1);
    if (pOpItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
        return;
    }
    pOpItem->setText(iLackVal <= 0 ? "" : QString::number(iLackVal) + strUnit);

    // 获取瓶规格
    pOpItem = sm_pStdModel->item(iRow, Rch_BottleSpeciffic);
    if (pOpItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR", __FUNCTION__);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
        return;
    }
    int iSpeciffic = pOpItem->text().remove("ml").toInt();

    // 更新需载入瓶数
    pOpItem = sm_pStdModel->item(iRow, Rch_ReqBottleCountAtMon + m_cenDayOfWeek - 1);
    if (pOpItem == Q_NULLPTR || iSpeciffic == 0)
    {
        ULOG(LOG_INFO, "%s(), pOpItem == Q_NULLPTR || iSpeciffic == 0", __FUNCTION__);
        connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
        return;
    }
    int iLackBottleCnt = iLackVal <= 0 ? 0 : (iLackVal - 1) / iSpeciffic + 1;
    pOpItem->setText(iLackBottleCnt <= 0 ? "" : QString::number(iLackBottleCnt));

    // 重新连接信号槽
    connect(sm_pStdModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(OnItemChanged(QStandardItem*)));
}

///
/// @brief
///     更新列显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateColumnVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化需求信息表，隐藏无关列
    for (int iCol = 0; iCol < sm_pStdModel->columnCount(); iCol++)
    {
        // 周一到周天的列都要隐藏非本天的列——动态计算
        if (iCol >= Rch_DynamicCalcAtMon && iCol <= Rch_DynamicCalcAtSun)
        {
            if (iCol != (Rch_DynamicCalcAtMon + m_cenDayOfWeek - 1))
            {
                ui->RequireCalcTbl->hideColumn(iCol);
            }
            continue;
        }

        // 周一到周天的列都要隐藏非本天的列——需求量
        if (iCol >= Rch_ReqVolAtMon && iCol <= Rch_ReqVolAtSun)
        {
            if (iCol != (Rch_ReqVolAtMon + m_cenDayOfWeek - 1))
            {
                ui->RequireCalcTbl->hideColumn(iCol);
            }
            continue;
        }

        // 周一到周天的列都要隐藏非本天的列——需求缺口
        if (iCol >= Rch_ReqLackAtMon && iCol <= Rch_ReqLackAtSun)
        {
            if (iCol != (Rch_ReqLackAtMon + m_cenDayOfWeek - 1))
            {
                ui->RequireCalcTbl->hideColumn(iCol);
            }
            continue;
        }

        // 周一到周天的列都要隐藏非本天的列——需载入瓶数
        if (iCol >= Rch_ReqBottleCountAtMon && iCol <= Rch_ReqBottleCountAtSun)
        {
            if (iCol != (Rch_ReqBottleCountAtMon + m_cenDayOfWeek - 1))
            {
                ui->RequireCalcTbl->hideColumn(iCol);
            }
            continue;
        }
    }

    // 列宽自适应
    ui->RequireCalcTbl->verticalHeader()->setVisible(false);
    ResizeTblColToContent(ui->RequireCalcTbl);
}

///
/// @brief
///     更新行显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateRowVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 是否选中只显示余量不足
    SetJustShowResidualLackingData(ui->RemainValRBtn->isChecked());
}

///
/// @brief
///     更新余量不足提示标签
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateLackTipLab()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取余量不足项目个数
    int iLackCnt = 0;

    // 遍历表格
    for (int iRow = 0; iRow < sm_pStdModel->rowCount() && iRow < sm_splReqInfoLst.size(); iRow++)
    {
        // 需求缺口大于0则余量不足
        if (sm_splReqInfoLst[iRow].vecILackingVal.size() >= Qt::Sunday && sm_splReqInfoLst[iRow].vecILackingVal[m_cenDayOfWeek - 1] > 0)
        {
            ++iLackCnt;
        }
    }

    // 如果余量不足项目为0，则隐藏提示
    if (iLackCnt <= 0)
    {
        ULOG(LOG_INFO, "%s(), no lack reagnt", __FUNCTION__);
        ui->TipLab->setVisible(false);
        return;
    }

    // 否则显示提示
    ui->TipLab->setVisible(true);
    ui->TipLab->setText(QString::number(iLackCnt) + tr("个项目不满足测试需求!"));
}

///
/// @brief
///     更新余量文本颜色
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateRemainValTextColor()
{
    // 遍历表格
    QSet<int> setLackRow;
    for (int iRow = 0; iRow < sm_pStdModel->rowCount() && iRow < sm_splReqInfoLst.size(); iRow++)
    {
        // 需求缺口大于0则余量不足
        bool bLack = false;
        if (sm_splReqInfoLst[iRow].vecILackingVal.size() >= Qt::Sunday && sm_splReqInfoLst[iRow].vecILackingVal[m_cenDayOfWeek - 1] > 0)
        {
            bLack = true;
        }

        // 获取对应单元格
        QStandardItem* pItem = sm_pStdModel->item(iRow, Rch_RemainVol);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 设置文本颜色
        if (bLack)
        {
            pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
        }
        else
        {
            pItem->setData(QVariant(), Qt::TextColorRole);
        }
    }
}

///
/// @brief
///     保存数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
void RequireCalcWidget::SaveData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据模型构造要保存的数据
    for (int iRow = 0; iRow < sm_pStdModel->rowCount() && iRow < sm_splReqInfoLst.size(); iRow++)
    {
        // 取出需求量和规格
        QVector<QString> vecStrReqVal;
        for (int iCol = Rch_ReqVolAtMon; iCol <= Rch_ReqVolAtSun; iCol++)
        {
            QString strReqVal = sm_pStdModel->item(iRow, iCol)->text();
            strReqVal.remove("ml");
            vecStrReqVal.push_back(strReqVal);
        }
        QString strBottleSpec = sm_pStdModel->item(iRow, Rch_BottleSpeciffic)->text();
        strBottleSpec.remove("ml");
        bool bOk = false;
        for (int iDayOfWeek = 0; iDayOfWeek < Qt::Sunday; iDayOfWeek++)
        {
            bOk = false;
            int iReqVal = vecStrReqVal[iDayOfWeek].toInt(&bOk);
            if (!bOk && !vecStrReqVal[iDayOfWeek].isEmpty())
            {
                continue;
            }

            // 构建数组
            if (sm_splReqInfoLst.at(iRow).vecIReqVal.size() < Qt::Sunday)
            {
                sm_splReqInfoLst[iRow].vecIReqVal.fill(0, Qt::Sunday);
            }
            sm_splReqInfoLst[iRow].vecIReqVal[iDayOfWeek] = iReqVal;
        }

        bOk = false;
        int iBottleSpec = strBottleSpec.toInt(&bOk);
        if (bOk || strBottleSpec.isEmpty())
        {
            sm_splReqInfoLst[iRow].iBottleSpec = iBottleSpec;
        }
    }

    // 保存数据
    if (!gUiAdapterPtr()->SaveSplReqInfo(RequireCalcWidget::sm_splReqInfoLst))
    {
        ULOG(LOG_ERROR, "%s(), save failed", __FUNCTION__);
    }
}

///
/// @brief
///     更新所有表格列显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateAllTblColVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 所有实例更新列显示
    for (auto pWgt : sm_setAllInstance)
    {
        // 指针空判断
        if (pWgt == Q_NULLPTR)
        {
            continue;
        }

        pWgt->UpdateColumnVisible();
    }
}

///
/// @brief
///     更新所有表格行显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateAllTblRowVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 所有实例更新行显示
    for (auto pWgt : sm_setAllInstance)
    {
        // 指针空判断
        if (pWgt == Q_NULLPTR)
        {
            continue;
        }

        pWgt->UpdateRowVisible();
    }
}

///
/// @brief
///     更新所有表格余量不足提示
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateAllTblLackTipLab()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 所有实例更新余量不足提示
    for (auto pWgt : sm_setAllInstance)
    {
        // 指针空判断
        if (pWgt == Q_NULLPTR)
        {
            continue;
        }

        pWgt->UpdateLackTipLab();
    }
}

///
/// @brief
///     更新当前表格余量文本颜色
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月7日，新建函数
///
void RequireCalcWidget::UpdateCurTblRemainValTextColor()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前显示实例更新余量文本颜色
    for (auto pWgt : sm_setAllInstance)
    {
        // 指针空判断
        if (pWgt == Q_NULLPTR)
        {
            continue;
        }

        // 未显示则跳过
        if (!pWgt->isVisible())
        {
            continue;
        }

        // 更新当前余量文本颜色
        pWgt->UpdateRemainValTextColor();
    }
}

///
/// @brief
///     获取打印信息
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 6889/ChenWei，2024年3月25日，新建函数
///
void RequireCalcWidget::GetPrintInfo(RequireCalcInfo& Info)
{
    for (int iRow = 0; iRow < sm_pStdModel->rowCount(); iRow++)
    {
        if (ui->RequireCalcTbl->isRowHidden(iRow) || iRow >= sm_splReqInfoLst.size())
        {
            continue;
        }

        RequireCalcItem RCItem;
        QString strName = sm_pStdModel->item(iRow, Rch_AssayName)->data(Qt::DisplayRole).value<QString>();
        RCItem.strName = strName.toStdString();

        QString strModuleRemain = sm_pStdModel->item(iRow, Rch_ModuleRemain)->data(Qt::DisplayRole).value<QString>();
        RCItem.strModuleRemain = strModuleRemain.toStdString();

        QString strRemainVol = sm_pStdModel->item(iRow, Rch_RemainVol)->data(Qt::DisplayRole).value<QString>();
        RCItem.strRemainVol = strRemainVol.toStdString();

        QString strBottleSpeciffic = sm_pStdModel->item(iRow, Rch_BottleSpeciffic)->data(Qt::DisplayRole).value<QString>();
        RCItem.strBottleSpeciffic = strBottleSpeciffic.toStdString();

        int iDynamicCalc = Rch_DynamicCalcAtMon + (m_cenDayOfWeek - Qt::Monday);
        QString strDynamicCalc = sm_pStdModel->item(iRow, iDynamicCalc)->data(Qt::DisplayRole).value<QString>();
        RCItem.strDynamicCalc = strDynamicCalc.toStdString();

        // 需求量
        int iReqVolAt = Rch_ReqVolAtMon + (m_cenDayOfWeek - Qt::Monday);
        QString strReqVol = sm_pStdModel->item(iRow, iReqVolAt)->data(Qt::DisplayRole).value<QString>();
        RCItem.strReqVol = strReqVol.toStdString();

        // 需求缺口
        int iReqLack = Rch_ReqLackAtMon + (m_cenDayOfWeek - Qt::Monday);
        QString strReqLack = sm_pStdModel->item(iRow, iReqLack)->data(Qt::DisplayRole).value<QString>();
        RCItem.strReqLack = strReqLack.toStdString();

        // 需加载瓶数
        int iReqBottleCount = Rch_ReqBottleCountAtMon + (m_cenDayOfWeek - Qt::Monday);
        QString strReqBottleCount = sm_pStdModel->item(iRow, iReqBottleCount)->data(Qt::DisplayRole).value<QString>();
        RCItem.strReqBottleCount = strReqBottleCount.toStdString();

        Info.ItemVector.push_back(RCItem);
    }
}


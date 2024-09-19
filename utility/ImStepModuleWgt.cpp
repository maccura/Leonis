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
/// @file     ImStepModuleWgt.cpp
/// @brief    应用-节点调试-生化步进模块 
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
#include "ImStepModuleWgt.h"
#include "ui_ImStepModuleWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/DictionaryQueryManager.h"
#include <QWhatsThis>

#define          SERVO_MOTOR_INDEX_UI               49

// 声明元类型
Q_DECLARE_METATYPE(im::tf::DebugUnit)

ImStepModuleWgt::ImStepModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImStepModuleWgt)
{
    ui->setupUi(this);

	InitData();
	InitConnect();
    InitUi();
}

ImStepModuleWgt::~ImStepModuleWgt()
{
}

void ImStepModuleWgt::UpdateUi(const im::tf::DebugModule& stuTfDbgMd, bool reset)
{
    // 调试模块
    m_stiTfDbgModule = stuTfDbgMd;

    // 重置行数为0
    if (reset)
    {
        ui->tableWidget->setRowCount(0);
    }
    
    // 将数据加载到表格
    int iRow = 0;
    for (const auto& stuDebugUnit : stuTfDbgMd.DebugUnits)
    {
        // 行数不足则自增
        if (iRow >= ui->tableWidget->rowCount())
        {
            ui->tableWidget->setRowCount(iRow + 1);
        }

        // 设置单元名称
        int iUnitStartRow = iRow;
        auto unitNameItem = new QTableWidgetItem(QString::fromStdString(stuDebugUnit.UnitName));
        unitNameItem->setData(Qt::UserRole, QVariant::fromValue(stuDebugUnit));
        ui->tableWidget->setItem(iRow, 0, unitNameItem);
        for (const auto& stuDbgPart : stuDebugUnit.DebugParts)
        {
            // 行数不足则自增
            if (iRow >= ui->tableWidget->rowCount())
            {
                ui->tableWidget->setRowCount(iRow + 1);
            }

            // 非单元名称所在行
            if (iUnitStartRow != iRow)
            {
                auto unitNameHideItem = new QTableWidgetItem("");
                unitNameHideItem->setData(Qt::UserRole, QVariant::fromValue(stuDebugUnit));
                ui->tableWidget->setItem(iRow, 0, unitNameHideItem);
            }

            // 设置方向和目标位置
            auto dirItem = new QTableWidgetItem(ConvertTfEnumToQString(stuDbgPart.Direction));
            dirItem->setToolTip(QString::fromStdString(stuDbgPart.Comment));
            dirItem->setWhatsThis(QString::fromStdString(stuDbgPart.Comment));
            ui->tableWidget->setItem(iRow, 1, dirItem);
            if (stuDbgPart.FlashId >= 0)
            {
                ui->tableWidget->setItem(iRow, 2, new QTableWidgetItem(QString::number(stuDbgPart.Parameter)));
            }
            else
            {
                ui->tableWidget->setItem(iRow, 2, new QTableWidgetItem("/"));
            }

            // 行号自增
            ++iRow;
        }

        // 合并行
        ui->tableWidget->setSpan(iUnitStartRow, 0, iRow - iUnitStartRow, 1);
    }

    // 列宽自适应
    ResizeTblColToContent(ui->tableWidget);

    // 选中第一行
    if (reset)
    {
        ui->tableWidget->selectRow(0);
    }   
}

///
/// @brief
///     显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月7日，新建函数
///
void ImStepModuleWgt::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    //QWhatsThis::enterWhatsThisMode();
}

///
/// @brief
///     显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月7日，新建函数
///
void ImStepModuleWgt::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    //QWhatsThis::leaveWhatsThisMode();
}

void ImStepModuleWgt::InitUi()
{
	// 初始化tableWidget
	ui->tableWidget->setColumnCount(3);
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("方向") << tr("目标位置"));
    // 表头
    //ui->BackRgntQcTbl->horizontalHeaderItem(Dqh_Selected)->setTextColor(UI_TEXT_COLOR_HEADER_MARK);
    ui->tableWidget->verticalHeader()->setVisible(false);
    //ui->BackRgntQcTbl->setItemDelegateForColumn(Dqh_Selected, new McPixmapItemDelegate(ui->BackRgntQcTbl));

    // 设置表格选中模式为行选中，不可多选
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ResizeTblColToContent(ui->tableWidget);
// 	ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);// 根据内容自适应
// 	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);                   // 最后一列占满
// 	ui->tableWidget->setColumnWidth(0, 400);
//     ui->tableWidget->setColumnWidth(1, 200);
}

void ImStepModuleWgt::InitData()
{

}

void ImStepModuleWgt::InitConnect()
{
	connect(ui->btn_add, &QPushButton::clicked, this, &ImStepModuleWgt::OnAddClicked);
	connect(ui->btn_del, &QPushButton::clicked, this, &ImStepModuleWgt::OnDelClicked);
	connect(ui->btn_execute, &QPushButton::clicked, this, &ImStepModuleWgt::OnExecuteClicked);
	connect(ui->btn_save, &QPushButton::clicked, this, &ImStepModuleWgt::OnSaveClicked);

    connect(ui->btn_add_2, &QPushButton::clicked, this, &ImStepModuleWgt::OnAdd2Clicked);
    connect(ui->btn_del_2, &QPushButton::clicked, this, &ImStepModuleWgt::OnDel2Clicked);
    connect(ui->btn_save_2, &QPushButton::clicked, this, &ImStepModuleWgt::OnSave2Clicked);

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
    }
}

void ImStepModuleWgt::OnAddClicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    if (stuTfDbgUnit.DebugParts.empty() || stuTfDbgUnit.DebugParts.front().Direction == im::tf::DirectionType::E_EMPTY)
    {
        return;
    }

    // 获取第一个数据
    im::tf::DebugPart& stuPart1 = stuTfDbgUnit.DebugParts.front();

    // 获取步数
    int iStep = ui->spinBox_pos->value();
    if (iStep <= 0)
    {
        return;
    }

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::MoveMotor(retl, m_strDevSn, true, stuPart1, iStep) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), MoveMotor failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }

    if (stuPart1.MotorInd != SERVO_MOTOR_INDEX_UI)
    {
        stuPart1.__set_tempParam(stuPart1.tempParam + iStep);
        ui->spinBox_targetPos->setValue(stuPart1.tempParam);
        ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        // 看选中哪一行，只有最多两个方向
        if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
        else
        {
            if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row() + 1, 0)->text() == "")
            {
                ui->tableWidget->item(selIdx.row() + 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
            }
        }
        // 更新到模块
        for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
        {
            // 不是修改的则跳过
            if (stuDbgUnit.CmdCode > 0 && 
                stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode && 
                stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam && 
                stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
            {
                // 更新到缓存
                stuDbgUnit = stuTfDbgUnit;
                break;
            }
        }

        // 发送通知消息
        POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
    return;
}

void ImStepModuleWgt::OnDelClicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    if (stuTfDbgUnit.DebugParts.empty() || stuTfDbgUnit.DebugParts.front().Direction == im::tf::DirectionType::E_EMPTY)
    {
        return;
    }

    // 获取第一个数据
    im::tf::DebugPart& stuPart1 = stuTfDbgUnit.DebugParts.front();

    // 获取步数
    int iStep = ui->spinBox_pos->value();
    if (iStep <= 0)
    {
        return;
    }

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::MoveMotor(retl, m_strDevSn, false, stuPart1, iStep) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), MoveMotor failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }

    if (stuPart1.MotorInd != SERVO_MOTOR_INDEX_UI)
    {
        stuPart1.__set_tempParam(stuPart1.tempParam - iStep);
        ui->spinBox_targetPos->setValue(stuPart1.tempParam);
        ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        // 看选中哪一行，只有最多两个方向
        if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
        else
        {
            if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row() + 1, 0)->text() == "")
            {
                ui->tableWidget->item(selIdx.row() + 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
            }
        }
        // 更新到模块
        for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
        {
            // 不是修改的则跳过
            if (stuDbgUnit.CmdCode > 0 &&
                stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
                stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam &&
                stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
            {
                // 更新到缓存
                stuDbgUnit = stuTfDbgUnit;
                break;
            }
        }

        // 发送通知消息
        POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
    return;
}

void ImStepModuleWgt::OnExecuteClicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::ExcuteDebugUnit(retl, m_strDevSn, stuTfDbgUnit) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), ExcuteDebugUnit failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }
    bool bChanged = false;
    for (auto& debugPart : stuTfDbgUnit.DebugParts)
    {
        if (debugPart.Direction == im::tf::DirectionType::E_VERTICAL && stuTfDbgUnit.DebugParts.size() > 1)
        {
            debugPart.tempParam = 0;
            continue;
        }
        // 如果无flash参数，赋值0
        if (debugPart.Parameter <= 0)
        {
            debugPart.tempParam = 0;
            continue;
        }
        // 防止发生变化
        ::tf::ResultLong _return;
        if (im::LogicControlProxy::ReadDebugParameter(_return, m_strDevSn, debugPart) && 
            _return.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS && _return.value != debugPart.Parameter)
        {
            debugPart.__set_Parameter(_return.value);
            bChanged = true;
        }      
        debugPart.tempParam = debugPart.Parameter;
    }
    ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    // 看选中哪一行，只有最多两个方向
    if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
    {
        ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    }
    else
    {
        if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row() + 1, 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row() + 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
    }

    // 更新到模块
    for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
    {
        // 不是修改的则跳过
        if (stuDbgUnit.CmdCode > 0 &&
            stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
            stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam &&
            stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
        {
            // 更新到缓存
            stuDbgUnit = stuTfDbgUnit;
            break;
        }
    }

    // 发送通知消息
    POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);

    // 弹框提示
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();

    if (bChanged)
    {
        UpdateUi(m_stiTfDbgModule, false);
    }

    OnSelectListItem();
}

void ImStepModuleWgt::OnSaveClicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    // 获取第一个数据
    im::tf::DebugPart& stuPart1 = stuTfDbgUnit.DebugParts.front();

    if (stuTfDbgUnit.DebugParts.empty() ||
        (stuPart1.Direction == im::tf::DirectionType::E_EMPTY && stuPart1.FlashId < 0))
    {
        return;
    }

    // 获取步数
    int iCurPos = ui->spinBox_targetPos->value();


    stuPart1.__set_tempParam(iCurPos);

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::SaveDebugParameter(retl, m_strDevSn, stuPart1) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), SaveDebugParameter failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }
    stuPart1.__set_Parameter(stuPart1.tempParam);
    ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    // 看选中哪一行，只有最多两个方向
    if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
    {
        ui->tableWidget->item(selIdx.row()-1, 2)->setText(QString::number(stuPart1.Parameter));
        ui->tableWidget->item(selIdx.row()-1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    }
    else
    {
        ui->tableWidget->item(selIdx.row(), 2)->setText(QString::number(stuPart1.Parameter));
        if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row()+1, 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row()+1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
    }
    
    bool bOtherChanged = false;
    // 更新到模块
    for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
    {
        // 不是修改的则跳过
        if (stuDbgUnit.CmdCode > 0 && stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
            stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam && stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
        {
            // 更新到缓存
            stuDbgUnit = stuTfDbgUnit;
            continue;
        }

        for (auto& tpart : stuDbgUnit.DebugParts)
        {
            if (tpart.NodeInd == stuPart1.NodeInd && tpart.MotorInd == stuPart1.MotorInd && tpart.tempParam != stuPart1.tempParam)
            {
                tpart.__set_tempParam(stuPart1.tempParam);
                bOtherChanged = true;
            }
            if (tpart.NodeInd == stuPart1.NodeInd && tpart.FlashId == stuPart1.FlashId && 
                tpart.MotorInd == stuPart1.MotorInd && tpart.Parameter != stuPart1.Parameter)
            {
                tpart.__set_Parameter(stuPart1.Parameter);
                bOtherChanged = true;
            }
        }
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();

    // 发送通知消息
    POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);

    if (bOtherChanged)
    {
        UpdateUi(m_stiTfDbgModule, false);
    }

    return;
}

void ImStepModuleWgt::OnAdd2Clicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    if (stuTfDbgUnit.DebugParts.size() < 2 || stuTfDbgUnit.DebugParts.front().Direction == im::tf::DirectionType::E_EMPTY)
    {
        return;
    }

    // 获取第一个数据
    im::tf::DebugPart& stuPart2 = stuTfDbgUnit.DebugParts.at(1);

    // 获取步数
    int iStep = ui->spinBox_pos_2->value();
    if (iStep <= 0)
    {
        return;
    }

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::MoveMotor(retl, m_strDevSn, true, stuPart2, iStep) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), MoveMotor failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }
    
    if (stuPart2.MotorInd != SERVO_MOTOR_INDEX_UI)
    {
        stuPart2.__set_tempParam(stuPart2.tempParam + iStep);
        ui->spinBox_targetPos_2->setValue(stuPart2.tempParam);
        ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        // 看选中哪一行，只有最多两个方向
        if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
        else
        {
            if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row() + 1, 0)->text() == "")
            {
                ui->tableWidget->item(selIdx.row() + 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
            }
        }

        // 更新到模块
        for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
        {
            // 不是修改的则跳过
            if (stuDbgUnit.CmdCode > 0 &&
                stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
                stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam &&
                stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
            {
                // 更新到缓存
                stuDbgUnit = stuTfDbgUnit;
                break;
            }
        }

        // 发送通知消息
        POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
    return;
}

void ImStepModuleWgt::OnDel2Clicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    if (stuTfDbgUnit.DebugParts.size() < 2 || stuTfDbgUnit.DebugParts.front().Direction == im::tf::DirectionType::E_EMPTY)
    {
        return;
    }

    // 获取第一个数据
    im::tf::DebugPart& stuPart2 = stuTfDbgUnit.DebugParts.at(1);

    // 获取步数
    int iStep = ui->spinBox_pos_2->value();
    if (iStep <= 0)
    {
        return;
    }

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::MoveMotor(retl, m_strDevSn, false, stuPart2, iStep) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), MoveMotor failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }
    if (stuPart2.MotorInd != SERVO_MOTOR_INDEX_UI)
    {
        stuPart2.__set_tempParam(stuPart2.tempParam - iStep);
        ui->spinBox_targetPos_2->setValue(stuPart2.tempParam);
        ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        // 看选中哪一行，只有最多两个方向
        if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
        {
            ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
        }
        else
        {
            if (selIdx.row() < ui->tableWidget->rowCount() - 1 && ui->tableWidget->item(selIdx.row() + 1, 0)->text() == "")
            {
                ui->tableWidget->item(selIdx.row() + 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
            }
        }
        // 更新到模块
        for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
        {
            // 不是修改的则跳过
            if (stuDbgUnit.CmdCode > 0 &&
                stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
                stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam &&
                stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
            {
                // 更新到缓存
                stuDbgUnit = stuTfDbgUnit;
                break;
            }
        }

        // 发送通知消息
        POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
    return;
}

void ImStepModuleWgt::OnSave2Clicked()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();
    if (stuTfDbgUnit.DebugParts.size() < 2 || stuTfDbgUnit.DebugParts.front().Direction == im::tf::DirectionType::E_EMPTY)
    {
        return;
    }

    // 获取步数
    int iCurPos = ui->spinBox_targetPos_2->value();

    // 获取第一个数据
    im::tf::DebugPart& stuPart2 = stuTfDbgUnit.DebugParts.at(1);
    stuPart2.__set_tempParam(iCurPos);

    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::SaveDebugParameter(retl, m_strDevSn, stuPart2) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), SaveDebugParameter failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }
    stuPart2.__set_Parameter(stuPart2.tempParam);
    ui->tableWidget->item(selIdx.row(), 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    if (ui->tableWidget->item(selIdx.row(), 0)->text() == "")
    {
        ui->tableWidget->item(selIdx.row(), 2)->setText(QString::number(stuPart2.Parameter));
        ui->tableWidget->item(selIdx.row() - 1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    }
    else
    {
        ui->tableWidget->item(selIdx.row()+1, 2)->setText(QString::number(stuPart2.Parameter));
        ui->tableWidget->item(selIdx.row()+1, 0)->setData(Qt::UserRole, QVariant::fromValue(stuTfDbgUnit));
    }

    // 更新到模块
    bool bOtherChanged = false;
    for (auto& stuDbgUnit : m_stiTfDbgModule.DebugUnits)
    {
        // 不是修改的则跳过
        if (stuDbgUnit.CmdCode > 0 && stuDbgUnit.CmdCode == stuTfDbgUnit.CmdCode &&
            stuDbgUnit.CmdParam == stuTfDbgUnit.CmdParam && stuDbgUnit.UnitName == stuTfDbgUnit.UnitName)
        {
            // 更新到缓存
            stuDbgUnit = stuTfDbgUnit;
            continue;
        }

        for (auto& tpart : stuDbgUnit.DebugParts)
        {
            if (tpart.NodeInd == stuPart2.NodeInd && tpart.MotorInd == stuPart2.MotorInd && tpart.tempParam != stuPart2.tempParam)
            {
                tpart.__set_tempParam(stuPart2.tempParam);
                bOtherChanged = true;
            }
            if (tpart.NodeInd == stuPart2.NodeInd && tpart.FlashId == stuPart2.FlashId &&
                tpart.MotorInd == stuPart2.MotorInd && tpart.Parameter != stuPart2.Parameter)
            {
                tpart.__set_Parameter(stuPart2.Parameter);
                bOtherChanged = true;
            }
        }
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();

    // 发送通知消息
    POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);

    if (bOtherChanged)
    {
        UpdateUi(m_stiTfDbgModule, false);
    }

    return;
}

///
/// @brief
///     选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月7日，新建函数
///
void ImStepModuleWgt::OnSelectListItem()
{
    // 清空内容
    ui->frame->setVisible(false);
    ui->frame_2->setVisible(false);
    ui->btn_execute->setEnabled(false);

    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 选中单项
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        disconnect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
        ui->tableWidget->selectRow(selIdx.row());
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
    }

    // 获取数据
    im::tf::DebugUnit stuTfDbgUnit = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<im::tf::DebugUnit>();

    // 可执行,若为流水线模式则单机(考虑到多语言，固定指令参数)相关不可执行
    if (stuTfDbgUnit.CmdCode > 0 && 
        !(DictionaryQueryManager::GetInstance()->GetPipeLine() && stuTfDbgUnit.CmdCode == 241 &&
        (stuTfDbgUnit.CmdParam == "3" || stuTfDbgUnit.CmdParam == "4" || stuTfDbgUnit.CmdParam == "5" || stuTfDbgUnit.CmdParam == "6" || stuTfDbgUnit.CmdParam == "7") ))
    {
        ui->btn_execute->setEnabled(true);
    }
    const im::tf::DebugPart& stuPart1 = stuTfDbgUnit.DebugParts.front();
    if (stuTfDbgUnit.DebugParts.empty() || 
        (stuPart1.Direction == im::tf::DirectionType::E_EMPTY && stuPart1.FlashId < 0))
    {
        return;
    }

    // 显示第一个方向
    ui->frame->setVisible(true);   
    ui->FirstDirLab->setText(ConvertTfEnumToQString(stuPart1.Direction));
    ui->spinBox_targetPos->setValue(stuPart1.tempParam);
    if (stuPart1.FlashId >= 0)
    {
        ui->btn_save->setVisible(true);
    }
    else
    {
        ui->btn_save->setVisible(false);
    }

    if (stuPart1.MotorInd < 0 || stuPart1.Direction == im::tf::DirectionType::E_EMPTY)
    {
        ui->btn_add->setVisible(false);
        ui->btn_del->setVisible(false);
        ui->spinBox_pos->setVisible(false);
    }
    else
    {
        ui->btn_add->setVisible(true);
        ui->btn_del->setVisible(true);
        ui->spinBox_pos->setVisible(true);
    }

    // 没有第二个方向，则返回
    if (stuTfDbgUnit.DebugParts.size() < 2)
    {
        return;
    }

    // 获取第二个方向
    ui->frame_2->setVisible(true);
    const im::tf::DebugPart& stuPart2 = stuTfDbgUnit.DebugParts.at(1);
    ui->SecondDirLab->setText(ConvertTfEnumToQString(stuPart2.Direction));
    ui->spinBox_targetPos_2->setValue(stuPart2.tempParam);
    if (stuPart2.FlashId >= 0)
    {
        ui->btn_save_2->setVisible(true);
    }
    else
    {
        ui->btn_save_2->setVisible(false);
    }

    if (stuPart2.MotorInd < 0 || stuPart2.Direction == im::tf::DirectionType::E_EMPTY)
    {
        ui->btn_add_2->setVisible(false);
        ui->btn_del_2->setVisible(false);
        ui->spinBox_pos_2->setVisible(false);
    }
    else
    {
        ui->btn_add_2->setVisible(true);
        ui->btn_del_2->setVisible(true);
        ui->spinBox_pos_2->setVisible(true);
    }
}

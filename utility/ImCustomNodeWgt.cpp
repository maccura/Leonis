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
/// @file     ImCustomNodeWgt.cpp
/// @brief    应用-节点调试-免疫自定义参数 
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
#include "ImCustomNodeWgt.h"
#include "ui_ImCustomNodeWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/DictionaryQueryManager.h"

#define          SERVO_MOTOR_INDEX_UI               49

// 声明元类型
Q_DECLARE_METATYPE(im::tf::DebugUnit)

ImCustomNodeWgt::ImCustomNodeWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ImCustomNodeWgt)
{
    ui->setupUi(this);

	InitData();
	InitConnect();
    InitUi();
}

ImCustomNodeWgt::~ImCustomNodeWgt()
{
}

void ImCustomNodeWgt::UpdateUi(const im::tf::DebugModule& stuTfDbgMd)
{
    // 记录之前选中的单元名称
//     QString strSelUnitName("");
//     QModelIndex selIdx = ui->tableWidget->currentIndex();
//     if (selIdx.isValid() && ui->tableWidget->item(selIdx.row(), 0) != Q_NULLPTR)
//     {
//         strSelUnitName = ui->tableWidget->item(selIdx.row(), 0)->text();
//     }

    // 调试模块
    m_stiTfDbgModule = stuTfDbgMd;

    // 重置行数为0
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem(tr("自定义参数")));
    ui->tableWidget->setItem(0, 1, new QTableWidgetItem("/"));
    ui->tableWidget->setItem(0, 2, new QTableWidgetItem("/"));

    // 列宽自适应
    ResizeTblColToContent(ui->tableWidget);

    // 选中第一行
    ui->tableWidget->selectRow(0);
}

void ImCustomNodeWgt::InitUi()
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
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ResizeTblColToContent(ui->tableWidget);
// 	ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);// 根据内容自适应
// 	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);                   // 最后一列占满
// 	ui->tableWidget->setColumnWidth(0, 400);
//     ui->tableWidget->setColumnWidth(1, 200);
}

void ImCustomNodeWgt::InitData()
{

}

void ImCustomNodeWgt::InitConnect()
{
	connect(ui->btn_save, &QPushButton::clicked, this, &ImCustomNodeWgt::OnSaveClicked);
    connect(ui->ReadBtn, &QPushButton::clicked, this, &ImCustomNodeWgt::OnReadClicked);
}

///
/// @brief  点击执行按钮
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月25日，新建函数
///
void ImCustomNodeWgt::OnReadClicked()
{
    // 获取地址
    int iBoardID = ui->BoardIdEdit->value();
    int iMotorSeq = ui->MotorSeqEdit->value();
    int iFlashAddr = ui->FlashAddrEdit->value();

    // 构造对象
    im::tf::DebugPart stuTfDbgPart;
    stuTfDbgPart.__set_NodeInd(iBoardID);
    stuTfDbgPart.__set_MotorInd(iMotorSeq);
    stuTfDbgPart.__set_FlashId(iFlashAddr);

    // 调用后台接口
    ::tf::ResultLong ret;
    if (!im::LogicControlProxy::ReadDebugParameter(ret, m_strDevSn, stuTfDbgPart) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), ReadDebugParameter failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }

    // 将获取到的值显示在编辑框中
    ui->NumValEdit->setValue(ret.value);

    // 提示执行成功
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
}

void ImCustomNodeWgt::OnSaveClicked()
{
    // 获取地址
    int iBoardID = ui->BoardIdEdit->value();
    int iMotorSeq = ui->MotorSeqEdit->value();
    int iFlashAddr = ui->FlashAddrEdit->value();
    int iNumVal = ui->NumValEdit->value();

    // 构造对象
    im::tf::DebugPart stuTfDbgPart;
    stuTfDbgPart.__set_NodeInd(iBoardID);
    stuTfDbgPart.__set_MotorInd(iMotorSeq);
    stuTfDbgPart.__set_FlashId(iFlashAddr);
    stuTfDbgPart.__set_tempParam(iNumVal);

    // 调用后台接口
    ::tf::ResultLong ret;
    if (!im::LogicControlProxy::SaveDebugParameter(ret, m_strDevSn, stuTfDbgPart) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), SaveDebugParameter failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }

    // 提示执行成功
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行成功!")));
    pTipDlg->exec();
}

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
/// @file     CupHistoryWidget.cpp
/// @brief    应用-状态-反应杯历史
///
/// @author   7951/LuoXin
/// @date     2023年6月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年6月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CupHistoryWidget.h"
#include "PrintExportDefine.h"
#include "ui_CupHistoryWidget.h"
#include <QStandardItemModel>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"

#define VIEW_ROW_COUNT          (13)        // 表格的行数
#define VIEW_COLUNM_COUNT       (5)        // 表格的列数

CupHistoryWidget::CupHistoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CupHistoryWidget)
    , m_cupHistoryModel(nullptr)
{
    ui->setupUi(this);
    Init();
}

CupHistoryWidget::~CupHistoryWidget()
{

}

void CupHistoryWidget::SetDevName(QString text)
{
    ui->dev_name_label->setText(text);

    OnUpdateTableView();
    UpdateReactionCupWidget();
}

void CupHistoryWidget::GetCupHistoryInfo(CupHistoryInfo& info)
{
    info.strModule = ui->dev_name_label->text().toStdString();

    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto& cup : m_mapDevAndCupInfo[info.strModule])
    {
        CupHistoryItem item;
        item.strCupNum = std::to_string(cup.cupNo);
        item.strStatus = ConvertTfEnumToQString((ch::tf::ReactionCupStatus::type)cup.cupStatus).toStdString();
        item.strTestTimes = std::to_string(cup.testedCount);
        if (cup.cupStatus == ch::tf::ReactionCupStatus::CUP_STATUS_ADD_SAMPLE_OR_R1)
        {
            item.strTestingAssay = CIM->GetAssayNameByCode(cup.testedAssayCodes[cup.testedAssayCodes.size() - 1]);
        }

        item.strTestedAssay = GetTestedAssayNameByCodeList(cup.testedAssayCodes);

        info.vecRecord.push_back(std::move(item));
    }
}

void CupHistoryWidget::Init()
{
    // 初始化表格
    m_cupHistoryModel = new QStandardItemModel(ui->cup_info_view);
    m_cupHistoryModel->setHorizontalHeaderLabels(
        { tr("杯号")
        , tr("状态")
        , tr("总测试数")
        , tr("正在检测")
        , tr("前10次测试项目") });

    ui->cup_info_view->setModel(m_cupHistoryModel);

	ui->cup_info_view->setColumnWidth(0, 85);
	ui->cup_info_view->setColumnWidth(1, 130);
	ui->cup_info_view->setColumnWidth(2, 90);
	ui->cup_info_view->setColumnWidth(3, 165);
	ui->cup_info_view->horizontalHeader()->setMinimumSectionSize(85);
	
    REGISTER_HANDLER(MSG_ID_REACTION_CUP_INFO_UPDATE, this, OnChReactionCupInfoUpdate);

    // 按设备初始化反应杯信息
    auto spDevs = CommonInformationManager::GetInstance()->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 });
    m_mapDevAndCupInfo.clear();
    for (auto& spDev : spDevs)
    {
        std::vector<::ch::tf::ReactionCupHistoryInfo> _return;
        ch::LogicControlProxy::QueryReactionCupHistoryInfo(_return, spDev->deviceSN);
        std::string name = spDev->groupName.empty() ? spDev->name : spDev->groupName + spDev->name;
        m_mapDevAndCupInfo[name] = _return;
    }

    connect(ui->reactionDiskWidget, &ReactionDiskWidget::SelectItemChanged, this, &CupHistoryWidget::OnUpdateTableView);
    // 表格选中行改变
    connect(ui->cup_info_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CupHistoryWidget::OnCurrentRowChanged);

    // 初始化表格的行列
    for (int iRow=0; iRow<VIEW_ROW_COUNT; iRow++)
    {
        AddTextToTableView(m_cupHistoryModel, iRow, COLUNM_CUP_NUM, QString());
        AddTextToTableView(m_cupHistoryModel, iRow, COLUNM_STATUS, QString());
        AddTextToTableView(m_cupHistoryModel, iRow, COLUNM_TEST_NUM, QString());
        AddTextToTableView(m_cupHistoryModel, iRow, COLUNM_TESTING_ASSAY, QString());
        m_cupHistoryModel->setItem(iRow, COLUNM_TEN_ASSAY, new QStandardItem());
    }    
}

void CupHistoryWidget::UpdateReactionCupWidget()
{
    for (auto& item : m_mapDevAndCupInfo[ui->dev_name_label->text().toStdString()])
    {
        ui->reactionDiskWidget->SetCupStatus(item.cupNo, item.cupStatus);
    }  

    update();
}

string CupHistoryWidget::GetTestedAssayNameByCodeList(const vector<int32_t>& codeList)
{
    std::string tenAssayName;
    auto CIM = CommonInformationManager::GetInstance();
    for (int i = 0; i < codeList.size(); i++)
    {
        std::string name = CIM->GetAssayNameByCode(codeList[i]);
        tenAssayName += name.empty() ? to_string(codeList[i]) : name;
        if (i != codeList.size() - 1)
        {
            tenAssayName += "、";
        }
    }

    return tenAssayName;
}

void CupHistoryWidget::OnUpdateTableView()
{
    // 缓存当前选中行
    auto curViewIndex = ui->cup_info_view->currentIndex();

    // 显示当前选中的杯子信息
    int index = ui->reactionDiskWidget->GetCurrentItemIndex() * 13;
    auto& cupInfo = m_mapDevAndCupInfo[ui->dev_name_label->text().toStdString()];
    if (cupInfo.empty())
    {
        return;
    }

    for (int iRow = 0; iRow<VIEW_ROW_COUNT; iRow++)
    {
        auto& rowData = cupInfo[index+ iRow];             
        QString name;
        if (!rowData.testedAssayCodes.empty()
            && (rowData.cupStatus == ch::tf::ReactionCupStatus::CUP_STATUS_ADD_SAMPLE_OR_R1
            || rowData.cupStatus == ch::tf::ReactionCupStatus::CUP_STATUS_ADD_R2))
        {
            name = QString::fromStdString(CommonInformationManager::GetInstance()->
                GetAssayNameByCode(rowData.testedAssayCodes[rowData.testedAssayCodes.size() - 1]));
        }

        //杯号
        m_cupHistoryModel->item(iRow, COLUNM_CUP_NUM)->setText(QString::number(rowData.cupNo));
        //状态
        QString temp = ConvertTfEnumToQString((ch::tf::ReactionCupStatus::type)rowData.cupStatus);
        m_cupHistoryModel->item(iRow, COLUNM_STATUS)->setText(temp);
        //总测试数
        m_cupHistoryModel->item(iRow, COLUNM_TEST_NUM)->setText(QString::number(rowData.testedCount));
        //正在测试的项目
        m_cupHistoryModel->item(iRow, COLUNM_TESTING_ASSAY)->setText(name);
        //前十次测试的项目
        m_cupHistoryModel->item(iRow, COLUNM_TEN_ASSAY)->setText(
            QString::fromStdString(GetTestedAssayNameByCodeList(rowData.testedAssayCodes)));
    }

    int row = curViewIndex.isValid() ? curViewIndex.row() : 0;
    ui->cup_info_view->selectRow(row);
    ui->reactionDiskWidget->SetCupIsSelect(cupInfo[index].cupNo + row);
}

void CupHistoryWidget::OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int cupNo = m_cupHistoryModel->item(current.row(), COLUNM_CUP_NUM)->text().toInt();
    ui->reactionDiskWidget->SetCupIsSelect(cupNo); 
}

void CupHistoryWidget::OnChReactionCupInfoUpdate(QString deviceSN,
    std::vector<ch::tf::ReactionCupHistoryInfo, std::allocator<ch::tf::ReactionCupHistoryInfo>> rchis)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    // 设备名称
    auto name = CommonInformationManager::GetInstance()->GetDeviceName(deviceSN.toStdString());
    
    // 获得对应设备的杯子信息缓存
    auto& vecCups = m_mapDevAndCupInfo[name];

    // 更新所有上报的杯子信息
    for (auto& rchi : rchis)
    {
        auto iter = std::find_if(vecCups.begin(), vecCups.end(), [&rchi](auto& item) 
        {
            return item.cupNo == rchi.cupNo;
        });

        if (iter != vecCups.end())
        {
            iter->cupStatus = rchi.cupStatus;
            if (rchi.__isset.testedCount)
            {
                iter->testedCount = rchi.testedCount;
            }
            if (rchi.__isset.testedAssayCodes)
            {
                iter->testedAssayCodes = rchi.testedAssayCodes;
            }
        }
    }

    OnUpdateTableView();
    UpdateReactionCupWidget();
}

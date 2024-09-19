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
/// @file     AssayShiledDlg.cpp
/// @brief    项目遮蔽弹窗
///
/// @author   7951/LuoXin
/// @date     2023年2月3日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年2月3日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AssayShiledDlg.h"
#include "ui_AssayShiledDlg.h"

#include <QScrollBar>
#include <algorithm>
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/public/SerializeUtil.hpp"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"

#define TAB_HEIGHT  47      // tab页切换按钮的高度

// 表格列
enum TableViewColumn
{
	COLUMN_NAME,			// 名称
	COLUMN_CODE				// 编号
};

AssayShiledDlg::AssayShiledDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::AssayShiledDlg)
	, m_chTableViewMode(new QColorHeaderModel)
	, m_iseTableViewMode(new QColorHeaderModel)
	, m_imTableViewMode(new QColorHeaderModel)
{
	ui->setupUi(this);

	// 设置标题
	BaseDlg::SetTitleName(tr("项目遮蔽"));

	ui->tableView_ch->setModel(m_chTableViewMode);
	ui->tableView_ise->setModel(m_iseTableViewMode);
	ui->tableView_im->setModel(m_imTableViewMode);

	// 保存按钮被点击
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

	// 生化勾选框
	AddSelectToView(ui->tableView_ch, m_chTableViewMode);
	// ise勾选框
	AddSelectToView(ui->tableView_ise, m_iseTableViewMode);
	// 免疫勾选框
	AddSelectToView(ui->tableView_im, m_imTableViewMode);

    const auto& CIM = CommonInformationManager::GetInstance();
    auto type = CIM->GetSoftWareType();
    if (type == CHEMISTRY)
    {
        ui->tabWidget->removeTab(1);
    }
    else if (type == IMMUNE)
    {
        ui->tabWidget->removeTab(0);
        ui->tabWidget->removeTab(2);
    }

    bool hasIseDev = false;
    for (auto& dev : CIM->GetDeviceMaps())
    {
        if (dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            hasIseDev = true;
            break;
        }
    }

    if (!hasIseDev)
    {
        // 移除ISE页面
        ui->tabWidget->removeTab(ui->tabWidget->count() - 1);
    }

    if (ui->tabWidget->count() < 2)
    {
        ui->tabWidget->setFixedHeight(ui->tabWidget->height() - TAB_HEIGHT);
        ui->save_btn->move(ui->save_btn->x() - TAB_HEIGHT, ui->save_btn->y() - TAB_HEIGHT);
        ui->cancel_btn->move(ui->cancel_btn->x() - TAB_HEIGHT, ui->cancel_btn->y() - TAB_HEIGHT);
        setFixedHeight(height() - TAB_HEIGHT);
    }
}

AssayShiledDlg::~AssayShiledDlg()
{

}

void AssayShiledDlg::LoadDataToDlg()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_chTableViewMode->removeRows(0, m_chTableViewMode->rowCount());
    m_iseTableViewMode->removeRows(0, m_iseTableViewMode->rowCount());
    m_imTableViewMode->removeRows(0, m_imTableViewMode->rowCount());

	// 按项目添加行
	const auto& assayMap = CommonInformationManager::GetInstance()->GetAssayInfo();
	for (const auto& item : assayMap)
	{
		// 生化
		if (item.second->assayClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
		{
			// 排除L、H、I
			if (item.second->assayCode == ch::tf::g_ch_constants.ASSAY_CODE_L
                || item.second->assayCode == ch::tf::g_ch_constants.ASSAY_CODE_H
                || item.second->assayCode == ch::tf::g_ch_constants.ASSAY_CODE_I)
			{
				continue;
			}

			AddAssayToView(m_chTableViewMode, item.second, ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
		}
		// ise
		else if (item.second->assayClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
		{
			// 排除ISE
			if (item.second->assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
			{
                AddAssayToView(m_iseTableViewMode, item.second, ::tf::AssayClassify::ASSAY_CLASSIFY_ISE);
			}		
		}
		// im
		else if (item.second->assayClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
		{
			AddAssayToView(m_imTableViewMode, item.second, ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
		}
	}
}

void AssayShiledDlg::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 基类先处理
    BaseDlg::showEvent(event);

    ui->tabWidget->setCurrentIndex(0);

    LoadDataToDlg();

    // 设置表格的列宽
    for (QTableView* view : QWidget::findChildren<QTableView*>())
    {
        int column = view->model()->columnCount();
        if (column < 2)
        {
            return;
        }

        for (int i = 2; i < column; i++)
        {
            view->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
        }
    }

    // 还原默认顺序
    ui->tableView_ch->sortByColumn(-1, Qt::DescendingOrder);
    m_chSortModel->sort(-1, Qt::DescendingOrder);

    ui->tableView_im->sortByColumn(-1, Qt::DescendingOrder);
    m_imSortModel->sort(-1, Qt::DescendingOrder);
}

void AssayShiledDlg::AddSelectToView(QTableView* view, QColorHeaderModel* model)
{
    // 初始化表格
    InitTableView(view, model);

	connect(view, &QTableView::doubleClicked, this, [=](const QModelIndex& index)
	{
		if (index.column() > COLUMN_CODE)
		{
			model->item(index.row(), index.column())->setData(
				!model->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
		}
	});
}

void AssayShiledDlg::InitTableView(QTableView* view, QColorHeaderModel* model)
{
	// 清理表格
	model->clear();
   
	// 获取设备信息
	const auto& devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();

	// 设置表头
	QStringList headerData = { tr("项目名称"), tr("编号") };

	::tf::AssayClassify::type type;
    QUtilitySortFilterProxyModel* sortModel = nullptr;
	if (view->objectName().endsWith("ch"))
	{
        m_chSortModel = new QUtilitySortFilterProxyModel(this);
        sortModel = m_chSortModel;
		type = ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
	} 
	else if(view->objectName().endsWith("ise"))
	{
		type = ::tf::AssayClassify::ASSAY_CLASSIFY_ISE;
	}
	else if (view->objectName().endsWith("im"))
	{
        m_imSortModel = new QUtilitySortFilterProxyModel(this);
        sortModel = m_imSortModel;
		type = ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE;
	}

	for (const auto& item : devMap)
	{
		if (item.second->deviceClassify == type)
		{
            if (type == tf::AssayClassify::ASSAY_CLASSIFY_ISE && item.second->moduleCount > 1)
            {
                for (int i = 1; i <= item.second->moduleCount; i++)
                {
                    headerData.push_back(QString::fromStdString(item.second->groupName + item.second->name)
                        + "-" + QString('A'+ (i-1)));
                }
            } 
            else
            {
                headerData.push_back(QString::fromStdString(item.second->groupName + item.second->name));
            }
		}
	}

	model->setHorizontalHeaderLabels(headerData);

	// 设置代理
	for (int i = COLUMN_CODE + 1; i < model->columnCount(); i++)
	{
        model->SetColunmColor(i, UI_HEADER_FONT_COLOR);
        auto cbd = new CheckBoxDelegate(QString(":/Leonis/resource/image/icon-mask.png"), this);
        cbd->setSelectedBkColor(QColor(0xf1f3f5));
		view->setItemDelegateForColumn(i, cbd);
	}

    // ISE的列表中只有一行，不需要排序
    if (sortModel != nullptr)
    {
        // Sort Header
        sortModel->setSourceModel(model);
        view->setModel(sortModel);
        sortModel->SetTableViewSort(view, { COLUMN_NAME });

        // 还原默认顺序
        view->sortByColumn(-1, Qt::DescendingOrder);
        sortModel->sort(-1, Qt::DescendingOrder);
    }

	ui->tableView_ch->setColumnWidth(0, 200);
	ui->tableView_im->setColumnWidth(0, 200);

	// 隐藏列
    view->hideColumn(COLUMN_CODE);
}

bool AssayShiledDlg::SaveShiledByColumn(QColorHeaderModel* model, int column)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取设备信息
    const auto& CIM = CommonInformationManager::GetInstance();
    QString devName = model->headerData(column, Qt::Horizontal).toString();
    QStringList tempList = devName.split("-");
    auto devInfo = CIM->GetDeviceInfoByDeviceName(tempList[0].toStdString());

    // 获取屏蔽的项目,ISE多模块显示ISE-A,ISE-B,数据库中存储模块索引，从1开始
    int moduleIndex = 1;
    std::map<int, std::set<int>> assayCodes;
    if (tempList.size() == 2 && DecodeJson(assayCodes, devInfo->maskedAssayCode))
    {
        moduleIndex = tempList[1].toLatin1()[0];
        moduleIndex -= 64;
        assayCodes[moduleIndex].clear();
    }

    for (int row = 0; row < model->rowCount(); row++)
    {
        if (model->item(row, column)->data(Qt::UserRole + 1).toBool())
        {
            int code = model->data(model->index(row, COLUMN_CODE)).toString().toInt();
            assayCodes[moduleIndex].insert(code);
        }
    }

	std::set<int> maskCodeMap;
	std::map<int, std::set<int>> tmpAssayCodes;
	DecodeJson(tmpAssayCodes, devInfo->maskedAssayCode);
	maskCodeMap = CIM->GetDifferentMap(tmpAssayCodes, assayCodes);

    std::string xml;
    if (!Encode2Json(xml, assayCodes))
    {
        ULOG(LOG_ERROR, "Encode2Json maskedAssayCode Failed!");
        return false;
    }

	// 修改
	::tf::DeviceInfo dev;
	if (devInfo->maskedAssayCode != xml)
	{
		dev.__set_deviceSN(devInfo->deviceSN);
		dev.__set_maskedAssayCode(xml);

		if (!CIM->ModifyDeviceInfo(dev))
		{
			ULOG(LOG_ERROR, "ModifyDeviceInfo Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return false;
		}
	}

	// 更新遮蔽项目
	POST_MESSAGE(MSG_ID_ASSAY_MASK_UPDATE, maskCodeMap);
	return true;
}

void AssayShiledDlg::AddAssayToView(QColorHeaderModel* model,std::shared_ptr<tf::GeneralAssayInfo> gai, int deviceClassify)
{
	int row = model->rowCount();

	// 项目名称
    const auto& CIM = CommonInformationManager::GetInstance();
    std::string name = gai->assayName;
    if (gai->assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
    {
        name = CIM->GetAssayNameByCode(::ise::tf::g_ise_constants.ASSAY_CODE_NA) + "、";
        name += CIM->GetAssayNameByCode(::ise::tf::g_ise_constants.ASSAY_CODE_K) + "、";
        name += CIM->GetAssayNameByCode(::ise::tf::g_ise_constants.ASSAY_CODE_CL);
    }

    AddTextToTableView(model, row, COLUMN_NAME, QString::fromStdString(name));
	// 编号
	model->setItem(row, COLUMN_CODE, new QStandardItem(QString::number(gai->assayCode)));

	// 勾选框
    for (int i = COLUMN_CODE + 1; i < model->columnCount(); i++)
    {
        auto si = new QStandardItem("");
        model->setItem(row, i, si);

        // 获取设备信息
        QString devName = model->horizontalHeaderItem(i)->text();
        QStringList tempList = devName.split("-");
        const auto& devInfo = CIM->GetDeviceInfoByDeviceName(tempList[0].toStdString());

        // ISE多模块显示ISE-A,ISE-B,数据库中存储模块索引，从1开始
        int moduleIndex = tempList.size() < 2 ? 1 : tempList[1].toLatin1()[0] - 64;

        std::map<int, std::set<int>> assayCodes;
        if (!DecodeJson(assayCodes, devInfo->maskedAssayCode))
        {
            ULOG(LOG_ERROR, "DecodeJson maskedAssayCode Failed!");
            return;
        }

        si->setData(assayCodes[moduleIndex].count(gai->assayCode) != 0, Qt::UserRole + 1);
    }
}

void AssayShiledDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 生化
	for (int column=COLUMN_CODE+1; column < m_chTableViewMode->columnCount(); column++)
	{
		if (!SaveShiledByColumn(m_chTableViewMode, column))
		{
			return;
		}
	}

	// ise
	for (int column = COLUMN_CODE + 1; column < m_iseTableViewMode->columnCount(); column++)
	{
		if (!SaveShiledByColumn(m_iseTableViewMode, column))
		{
			return;
		}
	}

	// im
	for (int column = COLUMN_CODE + 1; column < m_imTableViewMode->columnCount(); column++)
	{
		if (!SaveShiledByColumn(m_imTableViewMode, column))
		{
			return;
		}
	}

    close();
	emit sigAssayShiledChanged();
}

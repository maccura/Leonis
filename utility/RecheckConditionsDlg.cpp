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
/// @file     RecheckConditionsDlg.cpp
/// @brief    自动复查条件列表
///
/// @author   7951/LuoXin
/// @date     2023年2月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年2月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "RecheckConditionsDlg.h"
#include "SortHeaderView.h"
#include "ui_RecheckConditionsDlg.h"
#include <QStandardItemModel>
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"

#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "manager/DictionaryQueryManager.h"

#define CH_IS_SHOW_ROLE     (Qt::UserRole + 10)         // 表格的item存储是否显示生化的role
#define ISE_IS_SHOW_ROLE    (Qt::UserRole + 11)         // 表格的item存储是否显示电解质的role

// 复查条件列表的列
enum RecheckTableViewColunm
{
	COLUNM_CODE,				// 数据报警码
	COLUNM_NAME,				// 名称
	COLUNM_SAMPLE_VOL,			// 样本量
	COLUNM_ENDABLE				// 启用
};

RecheckConditionsDlg::RecheckConditionsDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::RecheckConditionsDlg)
	, m_recheckConditionsMode(new QStandardItemModel)
{
	ui->setupUi(this);

	// 设置标题
	BaseDlg::SetTitleName(tr("数据报警复查"));

    // 表格表头
    m_recheckConditionsMode->setHorizontalHeaderLabels({ tr("数据报警"), tr("描述"), tr("复查样本量"), tr("启用") });

    // Sort Header
    // 升序 降序 原序
    m_sortModel = new QUtilitySortFilterProxyModel(this);
    m_sortModel->setSourceModel(m_recheckConditionsMode);
    ui->tableView->setModel(m_sortModel);
    m_sortModel->SetTableViewSort(ui->tableView, { COLUNM_CODE,	COLUNM_NAME, COLUNM_SAMPLE_VOL });

	ui->tableView->setItemDelegateForColumn(COLUNM_ENDABLE, new CheckBoxDelegate(this));
	ui->tableView->setItemDelegateForColumn(COLUNM_NAME, new CReadOnlyDelegate(this));
	ui->tableView->setItemDelegateForColumn(COLUNM_CODE, new CReadOnlyDelegate(this));
	ui->tableView->setItemDelegateForColumn(COLUNM_SAMPLE_VOL, new CReadOnlyDelegate(this));
    ui->tableView->horizontalHeader()->setSectionResizeMode(COLUNM_SAMPLE_VOL, QHeaderView::Stretch);

	LoadDataToDlg();

	// 设置表格列宽
	ui->tableView->setColumnWidth(COLUNM_CODE, 300);
	ui->tableView->setColumnWidth(COLUNM_NAME, 780);
	ui->tableView->setColumnWidth(COLUNM_SAMPLE_VOL, 150);
    ui->tableView->horizontalHeader()->setSectionResizeMode(COLUNM_ENDABLE, QHeaderView::Stretch);

	// 保存按钮被点击
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

	// 勾选框
	connect(ui->tableView, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.column() == COLUNM_ENDABLE)
		{
			m_recheckConditionsMode->item(index.row(), index.column())->setData(
				!m_recheckConditionsMode->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
		}
	});
}

RecheckConditionsDlg::~RecheckConditionsDlg()
{

}

void RecheckConditionsDlg::UpdateSelectRows(const std::vector<std::string>& shieldStatusCodes, bool isCh)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	for (int row=0; row<m_recheckConditionsMode->rowCount(); row++)
	{
        auto item = m_recheckConditionsMode->item(row, COLUNM_CODE);
        // 状态码
		std::string code = item->text().toStdString();
        // 是否显示
        bool isShow = item->data(isCh ? CH_IS_SHOW_ROLE : ISE_IS_SHOW_ROLE).toBool();
        ui->tableView->setRowHidden(row, !isShow);

        // 勾选
		bool check = std::find(shieldStatusCodes.begin(), shieldStatusCodes.end(), code) != shieldStatusCodes.end();
		m_recheckConditionsMode->item(row, COLUNM_ENDABLE)->setData(!check, Qt::UserRole + 1);
	}

    // 还原默认顺序
    ui->tableView->sortByColumn(-1, Qt::DescendingOrder);
    m_sortModel->sort(-1, Qt::DescendingOrder);
}

void RecheckConditionsDlg::LoadDataToDlg()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取数据报警信息
	std::vector<tf::DataAlarmItem> dataAlarm;
	DcsControlProxy::GetInstance()->QueryDataAlarm(dataAlarm, "ALL");

	if (dataAlarm.empty())
	{
		ULOG(LOG_INFO, " dataAlarm Infos Is Empty!");
		return;
	}

	// 添加同量的数据报警信息到表格
	AddDataToTableView(::tf::RecheckModel::RECHECK_MODEL_SAME, dataAlarm);
	// 添加增量的数据报警信息到表格
	AddDataToTableView(::tf::RecheckModel::RECHECK_MODEL_INCREASE, dataAlarm);
	// 添加减量的数据报警信息到表格
	AddDataToTableView(::tf::RecheckModel::RECHECK_MODEL_REDUCE, dataAlarm);
}

void RecheckConditionsDlg::AddDataToTableView(int type, const std::vector<tf::DataAlarmItem>& dataAlarm)
{
	int row = m_recheckConditionsMode->rowCount();
	int beginRow = row;

	for (auto item : dataAlarm)
	{
		if (item.recheckModel == type)
		{
            // 数据报警
            auto si = new QStandardItem(QString::fromStdString(item.code));
            si->setData(item.enableCh, CH_IS_SHOW_ROLE);
            si->setData(item.enableIse, ISE_IS_SHOW_ROLE);
            si->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
            m_recheckConditionsMode->setItem(row, COLUNM_CODE, si);

			// 报警名称
			m_recheckConditionsMode->setItem(row, COLUNM_NAME, new QStandardItem(QString::fromStdString(item.name)));
			// 勾选
            si = new QStandardItem("");
            si->setData(true, Qt::UserRole + 1);
			m_recheckConditionsMode->setItem(row, COLUNM_ENDABLE, si);
			// 样本量
			AddTextToTableView(m_recheckConditionsMode, row, COLUNM_SAMPLE_VOL, ConvertTfEnumToQString(item.recheckModel));
			
			row++;
		}	
	}
}

void RecheckConditionsDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::vector<std::string> statusCodes;
    std::vector<std::string> shieldStatusCodes;
	for (int row = 0; row < m_recheckConditionsMode->rowCount(); row++)
	{
        auto code = m_recheckConditionsMode->data(m_recheckConditionsMode->index(row, COLUNM_CODE)).toString().toStdString();
		
        if (!m_recheckConditionsMode->item(row, COLUNM_ENDABLE)->data().toBool())
		{
            shieldStatusCodes.push_back(code);
		}
        else
        {
            statusCodes.push_back(code);
        }
	}

    const auto& oldDataAlarm = DictionaryQueryManager::GetChResultStatusCodes();
    QString errText;
    for (const std::string& code : statusCodes)
    {
        QString  qstrCode = QString::fromStdString(code);
        if (oldDataAlarm.contains(qstrCode))
        {
            errText += errText.isEmpty() ? qstrCode : "、" + qstrCode;
        }
    }

    if (!errText.isEmpty() 
        && TipDlg(tr("在应用-提示设置-数据报警中未启用[%1]，样本复查后将不会在工作页面显示此数据报警").arg(errText),
                TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

	emit sigSaveBtnClicked(shieldStatusCodes);

    close();
}

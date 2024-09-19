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
/// @file     SampleRackAllocationDlg.cpp
/// @brief    样本架分配弹窗
///
/// @author   7951/LuoXin
/// @date     2022年9月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SampleRackAllocationDlg.h"
#include "ui_SampleRackAllocationDlg.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryDecode.h"
#include "src/public/DictionaryKeyName.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

#define SAMPLE_SOURCE_TYPES_NUM			(6)		// 样本源类型数量(包括生化+免疫所支持的全部类型)

// 表格各数据所在列
enum TABLE_VIEW_COLUNM
{
    SAMPLE_COLUNM,				// 样本源
	ROUTINE_COLUNM,				// 常规架号
	EMERGENCY_COLUNM			// 急诊架号
};

SampleRackAllocationDlg::SampleRackAllocationDlg(QWidget *parent)
	: BaseDlg(parent)
	, m_tabViewMode(nullptr)
	, ui(new Ui::SampleRackAllocationDlg)
{
	ui->setupUi(this);
	BaseDlg::SetTitleName(tr("样本架分配"));

	// 初始化样本源数据
	InitSampleSourceTypeVec();

	Initctrls();
}

SampleRackAllocationDlg::~SampleRackAllocationDlg()
{

}

void SampleRackAllocationDlg::LoadDataToDlg()
{
	// 清空表格
	for (int rowIndex=0; rowIndex<m_sampleSourceTypesVec.size(); rowIndex++)
	{
		m_tabViewMode->setItem(rowIndex, ROUTINE_COLUNM, new QStandardItem(""));
		m_tabViewMode->setItem(rowIndex, EMERGENCY_COLUNM, new QStandardItem(""));
	}

    SampleRackAllocation sra;
    if (!DictionaryQueryManager::GetSampleRackAllocationConfig(sra))
    {
        ULOG(LOG_ERROR, "Failed to get samplerack allocation config.");
        return;
    }

	// 加载常规架号
	for (auto item : sra.RoutineRack)
	{
		int rowIndex = m_sampleSourceTypesVec.indexOf(static_cast<::tf::SampleSourceType::type>(item.sampleSourceType));
		if (rowIndex == -1)
		{
			continue;
		}

		m_tabViewMode->item(rowIndex, ROUTINE_COLUNM)->setText(QString::number(item.lower) + "-" + QString::number(item.upper));
	}

	// 加载急诊架号
	for (auto item : sra.EmergencyRack)
	{
		int rowIndex = m_sampleSourceTypesVec.indexOf(static_cast<::tf::SampleSourceType::type>(item.sampleSourceType));
		if (rowIndex == -1)
		{
			continue;
		}
		m_tabViewMode->item(rowIndex, EMERGENCY_COLUNM)->setText(QString::number(item.lower) + "-" + QString::number(item.upper));
	}

    ui->tableView->selectionModel()->clear();
    QModelIndex topLeft = m_tabViewMode->index(0, SAMPLE_COLUNM);
    QModelIndex bottomRight = m_tabViewMode->index(0, EMERGENCY_COLUNM);
    ui->tableView->selectionModel()->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);
}

void SampleRackAllocationDlg::OnSaveBtnClicked()
{
	SampleRackAllocation sra;
	RackRange rackRange;

	// 处理架号
    if (!GetUserInputData(false, sra) || !GetUserInputData(true, sra))
    {
        return;
    }

    // 保存样本架分配
    if (!DictionaryQueryManager::SaveSampleRackAllocationConfig(sra))
    {
        ULOG(LOG_ERROR, "Failed to save samplerack allocation config.");
        TipDlg(tr("保存失败"), tr("保存失败"));
        return;
    }

	//LoadDataToDlg();
    close();
}

void SampleRackAllocationDlg::UpdateCtrlsEnabled()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto& uim = UserInfoManager::GetInstance();

    ui->tableView->setEnabled(uim->IsPermisson(PSM_IM_TESTSET_RACKDISPATCH));
}

void SampleRackAllocationDlg::Initctrls()
{
	// 最后一行不拉伸铺满
	ui->tableView->verticalHeader()->setStretchLastSection(false);

    m_tabViewMode = new NumericTableModel();
	ui->tableView->setModel(m_tabViewMode);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);

	// 设置列表横向表头
	QStringList assayHeaderListString;
	assayHeaderListString << tr("样本类型") << tr("常规") << tr("急诊");
	m_tabViewMode->setHorizontalHeaderLabels(assayHeaderListString);

    // 设置表头字体颜色
    m_tabViewMode->SetColunmColor(ROUTINE_COLUNM, UI_HEADER_FONT_COLOR);
    m_tabViewMode->SetColunmColor(EMERGENCY_COLUNM, UI_HEADER_FONT_COLOR);

    // 只读代理
    ui->tableView->setItemDelegateForColumn(SAMPLE_COLUNM, new CReadOnlyDelegate(this));

	// 设置列表纵向表头
	QStringList listString;
	for (int rowIndex = 0; rowIndex < m_sampleSourceTypesVec.size(); rowIndex++)
	{
        AddTextToTableView(m_tabViewMode, rowIndex, SAMPLE_COLUNM, ConvertTfEnumToQString(m_sampleSourceTypesVec[rowIndex]));
        AddTextToTableView(m_tabViewMode, rowIndex, ROUTINE_COLUNM, "");
        AddTextToTableView(m_tabViewMode, rowIndex, EMERGENCY_COLUNM, "");
	}

	// 调整UI显示
	AdjustUIDisplay();

	connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

bool SampleRackAllocationDlg::CheckRackRang(const std::vector<RackRange>& vecRackRange)
{
	for (int i=0; i<vecRackRange.size(); i++)
	{
		for (int j=0; j<vecRackRange.size(); j++)
		{
			if (i==j)
			{
				continue;
			}

			if ((vecRackRange[i].lower >= vecRackRange[j].lower && vecRackRange[i].lower <= vecRackRange[j].upper)
				|| (vecRackRange[i].upper >= vecRackRange[j].lower && vecRackRange[i].upper <= vecRackRange[j].upper))
			{
				return false;
			}
		}
		
	}

	return true;
}

///
/// @brief	初始化样本源数据，用于过滤免疫不支持的样本源
///     
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月11日，新建函数
///
void SampleRackAllocationDlg::InitSampleSourceTypeVec()
{
	// 清空
	m_sampleSourceTypesVec.clear();
	
	for (int typeIndex = 0; typeIndex < SAMPLE_SOURCE_TYPES_NUM; typeIndex++)
	{
		auto ssType = static_cast<::tf::SampleSourceType::type>(typeIndex);

		// 过滤掉"其他"类型,放到列表最后
		if(ssType == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)	continue;

		// 过滤免疫不支持的浆膜腔积液
		if (CommonInformationManager::GetInstance()->GetSoftWareType() == IMMUNE &&
			ssType == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY)
		{
			continue;
		}

		m_sampleSourceTypesVec.push_back(ssType);
	}

	// 最后放置"其他"类型
	m_sampleSourceTypesVec.push_back(::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);

}

///
/// @brief	调整UI显示
///     
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月11日，新建函数
///
void SampleRackAllocationDlg::AdjustUIDisplay()
{
	int cutDownRowNum = SAMPLE_SOURCE_TYPES_NUM - m_tabViewMode->rowCount();
	if (cutDownRowNum<=0 || m_tabViewMode->rowCount() <= 0)
	{
		return;
	}

	// 获取第一行的高度
	int cutDownHight = cutDownRowNum*ui->tableView->rowHeight(0);

	ui->tableView->resize(ui->tableView->width(), ui->tableView->height() - cutDownHight);
	ui->line_v_2->resize(ui->line_v_2->width(),ui->line_v_2->height()- cutDownHight);
	ui->line_v->resize(ui->line_v->width(), ui->line_v->height() - cutDownHight);
	this->resize(this->width(), this->height()- cutDownHight);

	ui->line_h_2->move(ui->line_h_2->x(), ui->line_h_2->y() - cutDownHight);
	ui->ok_btn->move(ui->ok_btn->x(), ui->ok_btn->y() - cutDownHight);
	ui->cancel_btn->move(ui->cancel_btn->x(), ui->cancel_btn->y() - cutDownHight);
}

bool SampleRackAllocationDlg::GetUserInputData(bool isStat, SampleRackAllocation& sra)
{
    RackRange rackRange;
    QString tipText(isStat ? tr("急诊") : tr("常规"));
    auto&vecRack = isStat ? sra.EmergencyRack : sra.RoutineRack;
    double lower;
    double upper;

    if (isStat)
    {
        lower = DEFAULT_RACK_NUM_RANGE_STAT_LOWER_LMT;
        upper = DEFAULT_RACK_NUM_RANGE_STAT_UPPER_LMT;
    }
    else
    {
        lower = DEFAULT_RACK_NUM_RANGE_NORMAL_LOWER_LMT;
        upper = DEFAULT_RACK_NUM_RANGE_NORMAL_UPPER_LMT;
    }

    for (int rowIndex = 0; rowIndex < m_sampleSourceTypesVec.size(); rowIndex++)
    {
        QString rack = m_tabViewMode->data(m_tabViewMode
            ->index(rowIndex, isStat ? EMERGENCY_COLUNM : ROUTINE_COLUNM)).toString();
        if (rack.isEmpty())
        {
            continue;
        }

        // 样本类型为其他，特殊处理
        rackRange.sampleSourceType = m_sampleSourceTypesVec[rowIndex];

        auto tempData = rack.split("-");
        if (tempData.size() != 2)
        {
            ReportErr(true, ConvertTfEnumToQString((::tf::SampleSourceType::type)rackRange.sampleSourceType)
                + tr("值错误：%1架号范围格式错误，正确格式：下限-上限").arg(tipText));
        }

        // 架号下限
        rackRange.lower = tempData[0].toInt();
        ReportErr(rackRange.lower < lower || rackRange.lower > upper,
            ConvertTfEnumToQString((::tf::SampleSourceType::type)rackRange.sampleSourceType)
            + tr("值错误：%1≤%2架号范围≤%3").arg(lower).arg(tipText).arg(upper));

        // 架号上限
        rackRange.upper = tempData[1].toInt();
        ReportErr(rackRange.upper < lower || rackRange.upper > upper,
            ConvertTfEnumToQString((::tf::SampleSourceType::type)rackRange.sampleSourceType)
            + tr("值错误：%1≤%2架号范围≤%3").arg(lower).arg(tipText).arg(upper));

        if (rackRange.lower > rackRange.upper)
        {
            ReportErr(true, ConvertTfEnumToQString((::tf::SampleSourceType::type)rackRange.sampleSourceType)
                + tr("值错误：架号的下限不能大于上限"));
        }

        vecRack.push_back(rackRange);
    }

    // 检查架号
    ReportErr(!CheckRackRang(vecRack), tr("%1架号范围设置重复").arg(tipText));

    return true;
}

bool NumericTableModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::EditRole 
        && index.column() != SAMPLE_COLUNM)
    {
        ReportErr(!validateInput(value.toString()), tr("架号范围格式错误，正确格式：起始架号-结束架号"));
    }

    return QStandardItemModel::setData(index, value, role);
}

QVariant NumericTableModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    int rCnt = rowCount();
    if ((role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        auto iter = m_mapRowAndColor.find(section);
        if (iter != m_mapRowAndColor.end())
        {
            return iter->second;
        }
    }

    return QStandardItemModel::headerData(section, orientation, role);
}

bool NumericTableModel::validateInput(const QString& input) const
{
    QRegExp numericPattern("\\d{1,4}");
    QRegExp numericRangPattern("\\d{1,4}-\\d{1,4}");
    return numericPattern.exactMatch(input) || numericRangPattern.exactMatch(input) || input.isEmpty();
}

void NumericTableModel::SetColunmColor(int row, QColor color)
{
    m_mapRowAndColor[row] = color;
}

QVariant NumericTableModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    return QStandardItemModel::data(index, role);
}

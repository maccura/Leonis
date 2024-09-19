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
/// @file     QReCalcDialog.cpp
/// @brief    
///
/// @author   1226/ZhangJing
/// @date     2023年1月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年1月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QReCalcDialog.h"
#include "ui_QReCalcDialog.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/QComDelegate.h"
#include "shared/QSerialModel.h"

#include <QRadioButton>
#include <QHBoxLayout>
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "manager/ResultStatusCodeManager.h"

#include "src/common/Mlog/mlog.h"


Q_DECLARE_METATYPE(im::tf::ReagentInfoTable);
Q_DECLARE_METATYPE(im::tf::AssayTestResult);

///
/// @brief 重新计算的样本列表列头序号
///
enum ReCalSampeCol
{
    RsChoose = 0,       // 选择
    RsSampleNo,         // 样本号
    RsSampleBarcode,    // 样本条码
    RsRLU,              // 发光值
    RsResult,           // 计算结果
    RsResultStat        // 结果状态
};

QReCalcDialog::QReCalcDialog(QWidget *parent)
    : BaseDlg(parent)
	, m_tableModelReag(new QSerialModel)
	, m_tableModelResults(new QSerialModel)
	, m_curDevSN("")
{
    ui = new Ui::QReCalcDialog();
	SetTitleName(tr("重新计算"));
    ui->setupUi(this);

	// 设置表格选中模式为行选中，不可多选
	ui->tableView_reag->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_reag->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_reag->setModel(m_tableModelReag);
	ui->tableView_reag->verticalHeader()->hide();
	ui->tableView_reag->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui->tableView_result->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_result->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_result->setModel(m_tableModelResults);
	ui->tableView_result->verticalHeader()->hide();
	ui->tableView_result->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView_result->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->tableView_result->setSelectionMode(QAbstractItemView::MultiSelection);

    InitTableTitle();
    InitMachines();

	connect(ui->cls_Button, &QPushButton::clicked, this, [this] { close(); });
	connect(ui->save_button, &QPushButton::clicked, this, &QReCalcDialog::SaveReCalcResults);
	connect(ui->reCalc_Button, &QPushButton::clicked, this, &QReCalcDialog::onReCalcSelResults);
	connect(ui->reCalc_All_Button, &QPushButton::clicked, this, &QReCalcDialog::onReCalcAllResults);
	connect(ui->pushButton_search, &QPushButton::clicked, this, &QReCalcDialog::onQueryReagInfos);
	//connect(ui->pushButton_reset, &QPushButton::clicked, this, &QReCalcDialog::onResetReagInfos);

	connect(ui->comboBox_projectName, SIGNAL(currentIndexChanged(int)), this, SLOT(OnItemComboxChange(int)));

	connect(ui->tableView_reag, &QTableView::clicked, this, &QReCalcDialog::onSelectRow);
    connect(ui->tableView_reag->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        if (selected.indexes().empty())
        {
			m_tableModelResults->removeRows(0, m_tableModelResults->rowCount());
        }
        else
            onSelectRow(selected.indexes()[0]);
    });
}

QReCalcDialog::~QReCalcDialog()
{
}

///
/// @brief 初始化表标题
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::InitTableTitle()
{
	QStringList hReagTitle;
	hReagTitle << tr("试剂批号") << tr("试剂瓶号") << tr("校准品批号")<< tr("校准时间") << tr("当前曲线有效期");
	m_tableModelReag->setHorizontalHeaderLabels(hReagTitle);

	QStringList hResultsTitle;
	hResultsTitle << tr("选择") << tr("样本号") << tr("样本条码")<< tr("发光值") << tr("计算结果") << tr("结果状态") ;
	m_tableModelResults->setHorizontalHeaderLabels(hResultsTitle);
    ui->tableView_result->setColumnWidth(0, 20);

    // 列宽自适应
    ResizeTblColToContent(ui->tableView_result);
    ResizeTblColToContent(ui->tableView_reag);

	// 获取所有项目信息
	auto assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();

	// 获取免疫项目信息
	auto imAssayMap = CommonInformationManager::GetInstance()->GetImAssayIndexCodeMaps();

    // 依次添加项目名称到容器中
    QStringList projectNames;
    projectNames.push_back("");
    std::set<QString> uniqStr; // 用于去重
	for (const auto& imAssay : imAssayMap)
	{
		if (nullptr == imAssay.second)
		{
			continue;
		}
		auto itrAssay = assayMap.find(imAssay.second->assayCode);
		if (itrAssay != assayMap.end() && itrAssay->second != nullptr)
		{
            QString strAssayName = QString::fromStdString(itrAssay->second->assayName);
            if (uniqStr.find(strAssayName) != uniqStr.end())
            {
                continue;
            }
            uniqStr.insert(strAssayName);
            projectNames.append(strAssayName);
		}		
	}
	ui->comboBox_projectName->clear();
	ui->comboBox_projectName->addItems(projectNames);
	ui->comboBox_projectName->setCurrentIndex(-1);
}

///
/// @brief 初始化设备
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::InitMachines()
{
    // 首先刷新设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_I6000 }));

    if (deviceList.size() == 1 && deviceList[0] != nullptr)
    {
        m_curDevSN = deviceList[0]->deviceSN;
    }
    // 设备列表为空或只有一个仪器，则直接返回 bug20225
    if (deviceList.size() < 2)
    {
        return;
    }

    // 开始插入仪器选项
    QHBoxLayout* layout = new QHBoxLayout(ui->radioButtons);
    if (layout == nullptr)
    {
        return;
    }

    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
    layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    bool isChecked = false;

    for (const auto& device : deviceList)
    {
        QRadioButton *pButton = new QRadioButton(QString::fromStdString(device->name));
        connect(pButton, &QPushButton::clicked, this, [this, device] { onSelectMachine(device->deviceClassify, device->deviceSN); });
        layout->addWidget(pButton);
        pButtonGroup->addButton(pButton);

        if (!isChecked)
        {
            pButton->setChecked(true);
            emit pButton->clicked(true);
        }
    }
}

///
/// @brief 执行保存
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::SaveReCalcResults()
{
    std::vector<int64_t> sItemId;
	for (int iRowIndx = 0; iRowIndx < m_tableModelResults->rowCount(); iRowIndx++)
	{
		auto result = m_tableModelResults->item(iRowIndx, ImResultHeader::rcr_SampleNo);
		if (result == nullptr)
		{
			continue;
		}
		// 获取结果信息
		im::tf::AssayTestResult rt = result->data().value<im::tf::AssayTestResult>();
		// 已经重新计算了
		if (rt.caliCurveId > 0 )
		{
			if (!im::i6000::LogicControlProxy::ModifyAssayTestResult(rt))
			{
				ULOG(LOG_ERROR, "save testresult err.");
				continue;
			}
            sItemId.push_back(rt.testItemId);
		}
	}

    if (sItemId.size() <= 0)
    {
        return;
    }
    // 删除重复itemid
    sItemId.erase(unique(sItemId.begin(), sItemId.end()), sItemId.end());
    ::tf::ResultLong rtl;
    if (!im::LogicControlProxy::UpdateResultToItemHist(rtl,sItemId) || rtl.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "save item change err.");
        return;
    }
}

///
/// @brief 当选择设备后，进行刷新
///
/// @param[in]  deviceName  设备名称
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::onSelectMachine(tf::AssayClassify::type  devClassify, const std::string& deviceName)
{
	if (!deviceName.empty())
		m_curDevSN = deviceName;

	if (m_curDevSN.empty())
	{
		return;
	}

	ui->comboBox_projectName->setCurrentIndex(0);
	ui->comboBox_reagLot->clear();

	SetImReagInfoToTable();	
}


///
/// @brief 将当前表格数据设置为免疫信息
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::SetImReagInfoToTable()
{
	ui->tableView_reag->setCurrentIndex(QModelIndex());   // 取消选择
	m_tableModelReag->removeRows(0, m_tableModelReag->rowCount());
	m_tableModelResults->removeRows(0, m_tableModelResults->rowCount());

	im::tf::ReagentInfoTableQueryCond rqc;
	rqc.__set_deviceSN(m_curDevSN);
	// 增加查询条件
	if (!ui->comboBox_projectName->currentText().isEmpty() && !ui->comboBox_reagLot->currentText().isEmpty())
	{
		//获取项目名称的code
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(ui->comboBox_projectName->currentText().toStdString());
		if (nullptr == spAssayInfo)  return;
		rqc.__set_assayCode(spAssayInfo->assayCode);
		rqc.__set_reagentLot(ui->comboBox_reagLot->currentText().toStdString());
	}
	im::tf::ReagentInfoTableQueryResp resp;
	if (!im::LogicControlProxy::QueryReagentInfoForReCalc(resp, rqc) || 
		resp.lstReagentInfos.size() == 0)
	{
		return;
	}

	int iRow = 0;
	for (const auto& reag : resp.lstReagentInfos)
	{
		QString strRgntName = QString::number(reag.assayCode);
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(reag.assayCode);
		if (Q_NULLPTR != spAssayInfo)
		{
			ULOG(LOG_INFO, "valid assay code,");
			strRgntName = QString::fromStdString(spAssayInfo->assayName);
		}
		// 内容list
		QList<QStandardItem*> itemList;

		// 试剂批号
        QStandardItem* itemLot = new QStandardItem(QString::fromStdString(reag.reagentLot));
		itemLot->setData(QVariant::fromValue<im::tf::ReagentInfoTable>(reag));
        itemList.push_back(itemLot);
		// 试剂瓶号
		itemList.push_back(new QStandardItem(QString::fromStdString(reag.reagentSN)));
		// 校准批号
		itemList.push_back(new QStandardItem(QString::fromStdString(reag.reagentLot)));
		// 校准时间
		itemList.push_back(new QStandardItem(QString::fromStdString(reag.caliTime)));
		// 当前曲线有效期
		itemList.push_back(new QStandardItem(QString::fromStdString(reag.caliCurveExpInfo)));
		// 插入内容
		m_tableModelReag->insertRow(iRow++, itemList);
	}
	// 选中首行
	ui->tableView_reag->setCurrentIndex(m_tableModelReag->item(0)->index());
	emit ui->tableView_reag->clicked(m_tableModelReag->item(0)->index());
	UpdateResultsInfoToTable();

    // 单元格内容居中显示
    SetTblTextAlign(m_tableModelResults, Qt::AlignCenter);
    SetTblTextAlign(m_tableModelReag, Qt::AlignCenter);
	return;
}


///
/// @brief 当某一行选择后
///
/// @param[in]  index  被点击的项目
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::onSelectRow(const QModelIndex &index)
{
	if (!index.isValid() || index.row() < 0 || index.column() < 0)
		return;
	UpdateResultsInfoToTable();
}

///
/// @brief  更新右侧未计算结果信息
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
void QReCalcDialog::UpdateResultsInfoToTable()
{
	// 获取当前选中行
	auto curModIdx = ui->tableView_reag->currentIndex();
	// 判断是否选中有效索引
	if (!curModIdx.isValid())
	{
		ULOG(LOG_ERROR, "Select invalid item.");
		return;
	}
	auto curItem = m_tableModelReag->item(curModIdx.row(), 0);
	if (curItem == nullptr)
	{
		ULOG(LOG_ERROR, "Select item is null.");
		return;
	}
	m_tableModelResults->removeRows(0, m_tableModelResults->rowCount());
	im::tf::ReagentInfoTable reag = curItem->data().value<im::tf::ReagentInfoTable>();

	im::tf::AssayTestResultQueryResp resp;
	if (!im::i6000::LogicControlProxy::QueryResultsUnCalc(resp, reag) ||
		resp.lstAssayTestResult.size() == 0)
	{
		return;
	}

	int iRow = 0;
	for (const auto& result : resp.lstAssayTestResult)
	{
		// 内容list
		QList<QStandardItem*> itemList;
        itemList.push_back(nullptr);
		// 样本号
		QStandardItem* itemNo = new QStandardItem(QString::fromStdString(result.seqNo));
		itemNo->setData(QVariant::fromValue<im::tf::AssayTestResult>(result));
		itemList.push_back(itemNo);
		// 样本条码
		itemList.push_back(new QStandardItem(QString::fromStdString(result.barcode)));
		// 发光值
		itemList.push_back(new QStandardItem(QString::number(result.RLU)));
		// 计算结果
        auto strResult = CommonInformationManager::GetInstance()->GetImDisplyResultStr(result.assayCode, result);
		itemList.push_back(new QStandardItem(strResult));

		// 结果状态
		std::string strCodes;
        ResultStatusCodeManager::GetInstance()->GetDisplayResCode(result.resultStatusCodes, strCodes, 
            strResult.toStdString(),result.refRangeJudge,result.crisisRangeJudge, result.sampType);
		itemList.push_back(new QStandardItem(QString::fromStdString(strCodes)));
	
		// 插入内容
		m_tableModelResults->insertRow(iRow, itemList);
        m_tableModelResults->setData(m_tableModelResults->index(iRow++, 0), false, Qt::UserRole + 1);
	}
}

///
/// @brief  重新计算未计算结果
///
/// @param[in]  bAll  是否全部
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::reCalcResults(bool bAll)
{
	// 获取当前选中行
	auto curModIdx = ui->tableView_reag->currentIndex();
	// 判断是否选中有效索引
	if (!curModIdx.isValid())
	{
		ULOG(LOG_ERROR, "Select invalid item.");
		return;
	}
	auto curItem = m_tableModelReag->item(curModIdx.row(), ImReagHeader::rcr_ReagLot);
	if (curItem == nullptr)
	{
		ULOG(LOG_ERROR, "Select item is null.");
		return;
	}
	// 获取试剂信息
	im::tf::ReagentInfoTable reag = curItem->data().value<im::tf::ReagentInfoTable>();

	std::vector< ::im::tf::AssayTestResult> lsResult;
	if (bAll)
	{
		for (int iRowIndx = 0; iRowIndx < m_tableModelResults->rowCount(); iRowIndx++)
		{
			auto result = m_tableModelResults->item(iRowIndx, ImResultHeader::rcr_SampleNo);
			if (result == nullptr)
			{
				continue;
			}
			// 获取结果信息
			im::tf::AssayTestResult rt = result->data().value<im::tf::AssayTestResult>();
			lsResult.push_back(rt);
		}
	}
	else
	{
        for (int iRowIndx = 0; iRowIndx < m_tableModelResults->rowCount(); iRowIndx++)
        {
            // 未选中
            if (!m_tableModelResults->index(iRowIndx, ImResultHeader::rcr_Select).data(Qt::UserRole + 1).toBool())
            {
                continue;
            }
            auto result = m_tableModelResults->item(iRowIndx, ImResultHeader::rcr_SampleNo);
            if (result == nullptr)
            {
                continue;
            }
            // 获取结果信息
            im::tf::AssayTestResult rt = result->data().value<im::tf::AssayTestResult>();
            lsResult.push_back(rt);
        }
	}

	im::tf::AssayTestResultQueryResp resp;
	if (!im::i6000::LogicControlProxy::ReCalcResult(resp, lsResult, reag) ||
		resp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "ReCalcResult not success.");
		return;
	}

	for (int iRowIndx = 0; iRowIndx < m_tableModelResults->rowCount(); iRowIndx++)
	{
		auto result = m_tableModelResults->item(iRowIndx, ImResultHeader::rcr_SampleNo);
		if (result == nullptr)
		{
			continue;
		}
		// 获取结果信息
		im::tf::AssayTestResult rt = result->data().value<im::tf::AssayTestResult>();
		
		for (const auto& r : resp.lstAssayTestResult)
		{
			// 更新数据
			if (r.id == rt.id)
			{
				result->setData(QVariant::fromValue<im::tf::AssayTestResult>(r));
                m_tableModelResults->setItem(iRowIndx, RsResult, new QStandardItem(QString::number(r.conc)));
                m_tableModelResults->setItem(iRowIndx, RsResultStat, new QStandardItem(QString::fromStdString(r.resultStatusCodes)));
				break;
			}
		}
	}
}

///
/// @brief  重新计算该试剂全部未计算结果
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::onReCalcAllResults()
{
	reCalcResults(true);
}


///
/// @brief  重新计算该试剂选中未计算结果
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::onReCalcSelResults()
{
	reCalcResults(false);
}

///
/// @bref
///		项目下拉框事件
///
/// @param[in] sel 选择的sel
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::OnItemComboxChange(int sel)
{
	// 清空试剂批号下拉菜单
	ui->comboBox_reagLot->clear();
    ui->unitEdit->setText("");
	//获取需要查找的项目信息
	std::string projectName = ui->comboBox_projectName->currentText().toStdString();
    
	// 如果在空的时候进行查找，直接返回
    if (projectName == "")    return;

	//获取项目名称的code
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(projectName);
	if (nullptr == spAssayInfo)  return;
    // 设置单位名称
    ui->unitEdit->setText(CommonInformationManager::GetInstance()->GetCurrentUnit(spAssayInfo->assayCode)); 
    
	im::tf::ReagentInfoTableQueryCond rqc;
	rqc.__set_deviceSN(m_curDevSN);
	rqc.__set_assayCode(spAssayInfo->assayCode);
	im::tf::ReagentInfoTableQueryResp resp;
	if (!im::LogicControlProxy::QueryReagentInfoForReCalc(resp, rqc) ||
		resp.lstReagentInfos.size() == 0)
	{
		return;
	}

	for (const auto& reag : resp.lstReagentInfos)
	{
		QString lot = QString::fromStdString(reag.reagentLot);
		if (ui->comboBox_reagLot->findText(lot) < 0)
		{
			ui->comboBox_reagLot->addItem(lot);
		}
	}
}

///
/// @brief  筛选试剂
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::onQueryReagInfos()
{
	SetImReagInfoToTable();
}

///
/// @brief  筛选试剂
///
/// @par History:
/// @li 1226/zhangjing，2023年2月1日，新建函数
///
void QReCalcDialog::onResetReagInfos()
{
	ui->comboBox_projectName->setCurrentIndex(0);
	ui->comboBox_reagLot->clear();
	SetImReagInfoToTable();
}

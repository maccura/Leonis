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
/// @file     QAssayParamDownloadDlg.cpp
/// @brief    应用-项目设置-参数下载的对话框
///
/// @author   8276/huchunli
/// @date     2023年8月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年8月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayParamDownloadDlg.h"
#include "ui_QAssayParamDownloadDlg.h"
#include <thread>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QUrl>
#include <QFileDialog>
#include <QTextStream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/EnCodeString.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "SortHeaderView.h"
#include "QAssayExportDlg.h"

#define PARAM_TABLE_INDEX_ROLE                   (Qt::UserRole + 1)                     // 信息查询索引

QAssayParamDownloadDlg::QAssayParamDownloadDlg(QParamDownloadModel::DownloadClassify downloadClassify, QWidget *parent)
    : BaseDlg(parent),
    m_bInit(false)
{
    ui = new Ui::QAssayParamDownloadDlg();
    ui->setupUi(this);

    // 初始化模型
    m_paramDownloadModel.InitModel(downloadClassify);

    // 显示前初始化
    InitBeforeShow();
}

QAssayParamDownloadDlg::~QAssayParamDownloadDlg()
{

}

///
/// @bref
///		初始化参数表格
///
/// @param[in] tbView 表格控件
/// @param[in] tbModel 表格模型
///
/// @par History:
/// @li 8276/huchunli, 2023年8月2日，新建函数
///
void QAssayParamDownloadDlg::InitTableView(QTableView* tbView, QStandardItemModel* tbModel)
{
    if (tbView == nullptr || tbModel == nullptr)
    {
        return;
    }

    // 排序初始化
    auto sortHeader = new SortHeaderView(Qt::Horizontal, tbView);

    tbView->setHorizontalHeader(sortHeader);
    tbView->setModel(tbModel);
    // 默认排序初始化
    sortHeader->setSortIndicator(-1, Qt::DescendingOrder);
    // 最后一格铺满
    sortHeader->setStretchLastSection(true);
    // 允许排序
    tbView->setSortingEnabled(true);

    // 点击表头触发排序
    connect(sortHeader, &SortHeaderView::SortOrderChanged, this, [tbView, tbModel, sortHeader](int logicIndex, SortHeaderView::SortOrder order)
    {
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            tbModel->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
            tbView->sortByColumn(logicIndex, Qt::AscendingOrder);
            sortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
        }
        else
        {
            int iSortRole = (logicIndex == COLUNM_ASSAY_CHECKBOX) ? UI_ITEM_ROLE_SEQ_CHECKBOX_SORT : Qt::DisplayRole;
            tbModel->setSortRole(iSortRole);
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            tbView->sortByColumn(logicIndex, qOrderFlag);
        }
    });

    // 设置表头
    QStringList headerTitle = { tr(""), tr("项目名称"), tr("样本类型"), tr("参数版本") };
    tbModel->setHorizontalHeaderLabels(headerTitle);

    tbView->verticalHeader()->setDefaultSectionSize(45);

    // 增加选择框
    QCheckBox* chkBox = new QCheckBox(tbView);
    chkBox->setMinimumSize(20, 20);
    UiCommon::Instance()->AddCheckboxToTableView(tbView, chkBox);

    // 设置列宽
    tbView->verticalHeader()->hide();
	tbView->horizontalHeader()->setMinimumSectionSize(30);
	tbView->setColumnWidth(COLUNM_ASSAY_CHECKBOX, 47);
	tbView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	tbView->setColumnWidth(COLUNM_ASSAY_NAME, 370);
	tbView->setColumnWidth(COLUNM_ASSAY_SAMPLETYPE, 370);
	//tbView->setColumnWidth(COLUNM_ASSAY_VERSION, 350);

    // 单选，不可编辑
    tbView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbView->setSelectionMode(QAbstractItemView::SingleSelection);
    tbView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::InitBeforeShow()
{
    SetTitleName(tr("参数下载"));

    // 切换页面
    OnTabPageChange(0);

    // 文件下载
    m_tbModelFileImport = new QStandardItemModel(ui->tableViewFileImport);
    InitTableView(ui->tableViewFileImport, m_tbModelFileImport);

    // 网络下载
    m_tbModelNetDownload = new QStandardItemModel(ui->tableViewDownload);
    InitTableView(ui->tableViewDownload, m_tbModelNetDownload);

}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空表格
    m_tbModelFileImport->removeRows(0, m_tbModelFileImport->rowCount());
    m_tbModelNetDownload->removeRows(0, m_tbModelNetDownload->rowCount());

    // 清空输入框
    ui->lineEditFileAddr->setEnabled(false);
    ui->lineEditFileAddr->clear();

    ui->comboBoxAssayName->clear();

	// 初始化提示框
	ui->label_FileStatus->clear();
	ui->label_FileStatus->setStyleSheet("");
	ui->label_NetStatus->clear();
	ui->label_NetStatus->setStyleSheet("");
}

///
/// @bref
///	对话框底部的按钮显示与隐藏的处理
///
/// @param[in] pageIdx 页索引
///
/// @par History:
/// @li 8276/huchunli, 2023年8月2日，新建函数
///
void QAssayParamDownloadDlg::OnTabPageChange(int pageIdx)
{
    if (ui->tabWidget->currentWidget() == ui->tabFileImport)
    {
        ui->pushButtonDownload->hide();
        ui->pushButtonClear->hide();
        ui->pushButtonImport->show();

        // 文件下载
        m_paramDownloadModel.SetDownloadWay(QParamDownloadModel::DownloadWay::FILE_WAY);
    }
    else
    {
        ui->pushButtonDownload->show();
        ui->pushButtonClear->show();
        ui->pushButtonImport->hide();

        // 网络下载
        m_paramDownloadModel.SetDownloadWay(QParamDownloadModel::DownloadWay::NET_WAY);
    }

}

///
/// @bref
///		初始化信号槽
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::InitConnect()
{
    // 关闭
    connect(ui->pushButtonClose, &QPushButton::clicked, this, [&] {this->close(); });
    // 文件选择
    connect(ui->pushButtonFileSelect, &QPushButton::clicked, this, &QAssayParamDownloadDlg::OnPushButtonFileSelectClicked);
    // 标签切换
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabPageChange(int)));
    // 参数导入
    connect(ui->pushButtonImport, &QPushButton::clicked, this, &QAssayParamDownloadDlg::OnPushButtonImportClicked);
    // 项目名称
    connect(ui->comboBoxAssayName, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QAssayParamDownloadDlg::OnComboBoxAssayNameChanged);

    // 查询
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &QAssayParamDownloadDlg::OnPushButtonSearchClicked);
    // 下载
    connect(ui->pushButtonDownload, &QPushButton::clicked, this, &QAssayParamDownloadDlg::OnPushButtonDownloadClicked);
    // 清空
    connect(ui->pushButtonClear, &QPushButton::clicked, this, &QAssayParamDownloadDlg::OnPushButtonClearClicked);
}

///
/// @bref
///	路径选择槽函数
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::OnPushButtonFileSelectClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QUrl assayFileUrl = QFileDialog::getOpenFileUrl(this, tr(u8"项目参数文件选择"), QUrl("./"), tr("*.maccura"));
	if (assayFileUrl.isEmpty())
	{
		return;
	}

    // 设置路径
    QString path = assayFileUrl.path().remove(0, 1);

    ui->lineEditFileAddr->setText(path);

    // 模型读取文件参数
    QStringList errInfo;
    if (!m_paramDownloadModel.ReadParamsFileData(path, errInfo))
    {
        ULOG(LOG_ERROR, "ReadParamsFileData() Failed!");
        // 提示信息
        UpdateNoticeInfo(false, errInfo.join(";"));
        return;
    }

    // 遍历模型数据，并显示在表格上
    if (!InitFileImportTableData())
    {
        ULOG(LOG_ERROR, "InitFileImportTableData() Failed!");
        // 提示信息
        UpdateNoticeInfo(false, tr("更新表格失败！"));
        return;
    }
}

///
/// @bref
///		文件导入项目参数
///
/// @param[in] index 改变的索引
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::OnComboBoxAssayNameChanged(int index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    Q_UNUSED(index);

    // 参数判断
    if (m_tbModelFileImport == nullptr)
    {
        return;
    }

    // 获取名称
    QString comBoxStr = ui->comboBoxAssayName->currentText();

    // 查找表格中的该项目名称
    int rowNum = m_tbModelFileImport->rowCount();
    for (int rowIndex = 0; rowIndex < rowNum; rowIndex++)
    {
        // 找到第一个即返回
        if (comBoxStr == m_tbModelFileImport->index(rowIndex, COLUNM_ASSAY_NAME).data().toString())
        {
            // 跳到该行
            ui->tableViewFileImport->scrollTo(m_tbModelFileImport->index(rowIndex, 0), QAbstractItemView::PositionAtTop);
            ui->tableViewFileImport->selectRow(rowIndex);
            return;
        }
    }

}

///
/// @bref
///		文件导入项目参数
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::OnPushButtonImportClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空提示
	UpdateNoticeInfo(true, "");
    if (m_tbModelFileImport == nullptr)
    {
        ULOG(LOG_WARN, "Null table model.");
        UpdateNoticeInfo(false, tr("导入失败！"));
        return;
    }

    // 获取勾选项
    QVector<int> indexVec;
    if (!GetTableViewSelected(ui->tableViewFileImport, m_tbModelFileImport, indexVec))
    {
        ULOG(LOG_ERROR, "GetTableViewSelected error!");
        UpdateNoticeInfo(false, tr("导入失败！"));
        return;
    }

    // 判空
    if (indexVec.size() == 0)
    {
        UpdateNoticeInfo(false, tr("导入失败！未选中任何导入项目。"));
        return;
    }

	// 防止多次创建
	if (m_paramDownloadModel.GetWorkStatus())
	{
		return;
	}

	// 创建一个新的线程来导入
	auto importFunc = [&](QVector<int> indexs) {
		POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("导入中..."), true);
		connect(this, &QAssayParamDownloadDlg::importResult, this, &QAssayParamDownloadDlg::OnImportHander);
		m_paramDownloadModel.SetWorkStatus(true);

		// 第一个项目
		QString strAssayName = m_tbModelFileImport->item(indexs[0], 1)->text();
		bool ret = m_paramDownloadModel.ImportFileParams(indexs);
		emit importResult(ret, strAssayName);
	};
	std::thread importThread(importFunc, indexVec);
	importThread.detach();
}

///
/// @brief 添加一行数据到表格
///
/// @param[in]  TableView   表格
/// @param[in]  TableModel  表格模型
/// @param[in]  index		信息查找索引
/// @param[in]  info		数据信息
///
/// @return true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
void QAssayParamDownloadDlg::AddRowToTableView(QTableView* TableView, QStandardItemModel* TableModel, const int& index, const ::adapter::tf::CloudAssayParam& info)
{
    if (TableModel == nullptr || TableView == nullptr)	return;

    int rowIndex = TableModel->rowCount();

    AddTextToTableView(TableModel, rowIndex, COLUNM_ASSAY_NAME, QString::fromStdString(info.assayName));
    AddTextToTableView(TableModel, rowIndex, COLUNM_ASSAY_SAMPLETYPE, ConvertTfEnumToQString(static_cast<::tf::SampleSourceType::type>(info.sampleSourceType)));
    AddTextToTableView(TableModel, rowIndex, COLUNM_ASSAY_VERSION, QString::fromStdString(info.version));
    // 勾选框
    QWidget* wid = new QWidget(TableView);
    wid->setMinimumSize(44, 44);
    QCheckBox* box = new QCheckBox(wid);
    box->setMinimumSize(20, 20);
    box->move(6, 2);
    TableView->setIndexWidget(TableModel->index(rowIndex, COLUNM_ASSAY_CHECKBOX), wid);
    QStandardItem *item = new QStandardItem();
    TableModel->setItem(rowIndex, COLUNM_ASSAY_CHECKBOX, item);

    // 设置映射索引
    item->setData(index, PARAM_TABLE_INDEX_ROLE);

    connect(box, &QCheckBox::stateChanged, this, [item, TableView, TableModel](int state) {

        // 参数检查
        if (item == nullptr || TableView == nullptr || TableModel == nullptr)
        {
            return;
        }
        item->setData((state == Qt::Checked ? true : false), UI_ITEM_ROLE_SEQ_CHECKBOX_SORT);

		// 是否全选
		bool IsAllChecked = true;
		for (int rowIndex = 0; rowIndex < TableModel->rowCount(); rowIndex++)
		{
			// 获取是否选中
			QWidget* wid = qobject_cast<QWidget*>(TableView->indexWidget(TableModel->index(rowIndex, COLUNM_ASSAY_CHECKBOX)));
			QCheckBox* box = wid->findChild<QCheckBox*>();
			if (!box->isChecked())
			{
				IsAllChecked = false;
				break;
			}
		}

		// 全选框状态修改
		auto allBox = TableView->horizontalHeader()->findChild<QCheckBox*>();
		allBox->setChecked(IsAllChecked);
		
    });


	ui->tableViewFileImport->horizontalHeader()->setMinimumSectionSize(40);
	ui->tableViewFileImport->setColumnWidth(0, 40);
}

///
/// @brief 获取表格中杯选中的信息索引
///
/// @param[in]  TableView	表格
/// @param[in]  TableModel  表格模型
/// @param[in]  indexVec	返回的信息索引
///
/// @return	true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QAssayParamDownloadDlg::GetTableViewSelected(QTableView* TableView, QStandardItemModel* TableModel, QVector<int>& indexVec)
{
    if (TableModel == nullptr || TableView == nullptr)	return false;

    // 清空
    indexVec.clear();
    for (int rowIndex = 0; rowIndex < TableModel->rowCount(); rowIndex++)
    {
        // 获取是否选中
        QWidget* wid = qobject_cast<QWidget*>(TableView->indexWidget(TableModel->index(rowIndex, COLUNM_ASSAY_CHECKBOX)));
        QCheckBox* box = wid->findChild<QCheckBox*>();
        if (!box->isChecked())
        {
            continue;
        }

        // 获取索引
        auto infoIndex = TableModel->item(rowIndex, COLUNM_ASSAY_CHECKBOX)->data(PARAM_TABLE_INDEX_ROLE);
        if (infoIndex.isValid())
        {
            indexVec.push_back(infoIndex.toInt());
        }
    }

    return true;
}

///
/// @bref
///		初始化文件下载参数表格数据
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
bool QAssayParamDownloadDlg::InitFileImportTableData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 表格模型判空
    if (m_tbModelFileImport == nullptr)
    {
        return false;
    }

    // 清空表格,初始化显示
    m_tbModelFileImport->removeRows(0, m_tbModelFileImport->rowCount());

    // 阻塞信号发出
    ui->comboBoxAssayName->blockSignals(true);
    ui->comboBoxAssayName->clear();
	UpdateNoticeInfo(true,"");

    // 遍历模型的数据进行添加
    auto fileInfo = m_paramDownloadModel.GetFileImportInfo();
    QMapIterator<int, ::adapter::tf::CloudAssayParam> iter(fileInfo);
    while (iter.hasNext()) {
        iter.next();

        // 添加进表格
        AddRowToTableView(ui->tableViewFileImport, m_tbModelFileImport, iter.key(), iter.value());

        // 添加进项目选择框(唯一性)
        QString addAssayName = QString::fromStdString(iter.value().assayName);
        if (ui->comboBoxAssayName->findText(addAssayName) == -1)
        {
            ui->comboBoxAssayName->addItem(addAssayName);
        }
    }

    // 设置默认排序值
    SetDefaultSort(m_tbModelFileImport);

    // 阻塞信号解除
    ui->comboBoxAssayName->setCurrentIndex(-1);
    ui->comboBoxAssayName->blockSignals(false);

    return true;
}

///
/// @bref
///		初始化网络下载参数表格数据
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
bool QAssayParamDownloadDlg::InitNetImportTableData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 表格模型判空
    if (m_tbModelNetDownload == nullptr)
    {
        return false;
    }

    // 清空表格,初始化显示
    m_tbModelNetDownload->removeRows(0, m_tbModelNetDownload->rowCount());

    // 遍历模型的数据进行添加
    auto netInfo = m_paramDownloadModel.GetNetDownloadInfo();
    QMapIterator<int, ::adapter::tf::CloudAssayParam> iter(netInfo);
    while (iter.hasNext()) {
        iter.next();
        iter.key(); iter.value();

        // 添加进表格
        AddRowToTableView(ui->tableViewDownload, m_tbModelNetDownload, iter.key(), iter.value());
    }

    // 设置默认排序值
    SetDefaultSort(m_tbModelNetDownload);

    return true;
}

///
/// @bref
///		网络下载查询按钮点击
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::OnPushButtonSearchClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判断输入框
    QString assayName = ui->lineEditAssayName->text();
    if (assayName.isEmpty())
    {
        // 提示信息
        UpdateNoticeInfo(false, tr("请输入项目名称！"));
        return;
    }

	// 防止多次创建
	if (m_paramDownloadModel.GetWorkStatus())
	{
		return;
	}

	// 创建一个新的线程来查询
	auto searchFunc = [&](QString aName) {
		POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("查询中..."), true);
		connect(this, &QAssayParamDownloadDlg::searchResult, this, &QAssayParamDownloadDlg::OnSearchHander);
		m_paramDownloadModel.SetWorkStatus(true);
		// 查询
		bool ret = m_paramDownloadModel.SearchNetParams(aName);
		emit searchResult(ret);
	};
	std::thread searchThread(searchFunc,assayName);
	searchThread.detach();

}

///
/// @brief 查询处理接口
///
/// @param[in]  isSuccess  是否成功
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
///
void QAssayParamDownloadDlg::OnSearchHander(bool isSuccess)
{
	disconnect(this, &QAssayParamDownloadDlg::searchResult, this, &QAssayParamDownloadDlg::OnSearchHander);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
	m_paramDownloadModel.SetWorkStatus(false);
	// 失败
	if (!isSuccess)
	{
		ULOG(LOG_ERROR, "SearchNetParams() Failed!");
		// 提示信息
		UpdateNoticeInfo(false, tr("参数查询失败！请确认网络是否正常!"));
		return;
	}

    // 未查询到任何可用结果-bug27703
    auto netInfo = m_paramDownloadModel.GetNetDownloadInfo();
    if (netInfo.size() <= 0)
    {
        ULOG(LOG_ERROR, "SearchNetParams() empty!");
        // 提示信息
        UpdateNoticeInfo(false, tr("未查询到符合条件的参数！"));
        return;
    }

	// 遍历模型数据，并显示在表格上
	if (!InitNetImportTableData())
	{
		ULOG(LOG_ERROR, "InitNetImportTableData() Failed!");
		// 提示信息
		UpdateNoticeInfo(false, tr("更新表格失败！"));
		return;
	}

	UpdateNoticeInfo(true, tr("查询成功！"));
}

///
/// @bref
///		网络下载按钮点击
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::OnPushButtonDownloadClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空提示
	UpdateNoticeInfo(true,"");
    if (m_tbModelNetDownload == nullptr)
    {
        ULOG(LOG_ERROR, "m_tbModelNetDownload is null!");
        UpdateNoticeInfo(false, tr("下载失败！"));
        return;
    }

    // 获取勾选项
    QVector<int> indexVec;
    if (!GetTableViewSelected(ui->tableViewDownload, m_tbModelNetDownload, indexVec))
    {
        ULOG(LOG_ERROR, "GetTableViewSelected error!");
        UpdateNoticeInfo(false, tr("下载失败！"));
        return;
    }

    // 判空
    if (indexVec.size() == 0)
    {
        UpdateNoticeInfo(false, tr("下载失败！未选中任何下载项目。"));
        return;
    }

	// 防止多次创建
	if (m_paramDownloadModel.GetWorkStatus())
	{
		return;
	}

	// 创建一个新的线程来下载
	auto downloadFunc = [&](QVector<int> indexs) {
		POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("下载中..."), true);
		connect(this, &QAssayParamDownloadDlg::downloadResult, this, &QAssayParamDownloadDlg::OnDownloadHander);
		m_paramDownloadModel.SetWorkStatus(true);

		bool ret = m_paramDownloadModel.DownloadNetParams(indexs);
		emit downloadResult(ret);
	};
	std::thread downloadThread(downloadFunc, indexVec);
	downloadThread.detach();

}

///
/// @bref
///		清空按钮点击
///
///
/// @par History:
/// @li 1226/ZhangJing，2024年5月15日，新建函数
///
void QAssayParamDownloadDlg::OnPushButtonClearClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空提示
	UpdateNoticeInfo(true, "");
    // 表格模型判空
    if (m_tbModelNetDownload == nullptr)
    {
        return;
    }
    // 清空表格,初始化显示
    m_tbModelNetDownload->removeRows(0, m_tbModelNetDownload->rowCount());
	ui->lineEditAssayName->clear();
}

///
/// @brief 下载处理接口
///
/// @param[in]  isSuccess  是否成功
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
///
void QAssayParamDownloadDlg::OnDownloadHander(bool isSuccess)
{
	disconnect(this, &QAssayParamDownloadDlg::downloadResult, this, &QAssayParamDownloadDlg::OnDownloadHander);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
	m_paramDownloadModel.SetWorkStatus(false);

	// 是否有跳过项目
	QString skipNoticeInfo = "";
	if (m_paramDownloadModel.GetSkipFlag())
	{
		skipNoticeInfo = tr("存在已有项目信息，已自动跳过。");
	}

	// 下载失败
	if (!isSuccess)
	{
		ULOG(LOG_ERROR, "DownloadParams Failed!");
		// 提示信息
		UpdateNoticeInfo(false, tr("下载失败！"));
		return;
	}

	// 通知项目更新,-1:更新全部项目
	CommonInformationManager::GetInstance()->UpdateAssayCfgInfo({ -1 });

	// 提示信息
	UpdateNoticeInfo(true, tr("下载成功！") + skipNoticeInfo);
}


///
/// @brief 导入处理接口
///
/// @param[in]  isSuccess  是否成功
/// @param[in]  firstAssayName  第一个项目名称
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年5月16日，新建函数
///
void QAssayParamDownloadDlg::OnImportHander(bool isSuccess, QString firstAssayName)
{
	disconnect(this, &QAssayParamDownloadDlg::importResult, this, &QAssayParamDownloadDlg::OnImportHander);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
	m_paramDownloadModel.SetWorkStatus(false);

	// 是否有跳过项目
	QString skipNoticeInfo = "";
	if (m_paramDownloadModel.GetSkipFlag())
	{
		skipNoticeInfo = tr("存在已有项目信息，已自动跳过。");
	}

	// 失败
	if (!isSuccess)
	{
		ULOG(LOG_ERROR, "ImportParams Failed!");
		// 提示信息
		UpdateNoticeInfo(false, tr("导入失败！"));
		return;
	}

	// 通知项目更新,-1:更新全部项目
	CommonInformationManager::GetInstance()->UpdateAssayCfgInfo({ -1 });

	// 提示信息
	UpdateNoticeInfo(true, tr("导入成功！") + skipNoticeInfo);

	// 通知项目列表，选中导入成功的第一个项目
	POST_MESSAGE(MSG_ID_ASSAY_SELECT, firstAssayName);
}

///
/// @brief
///     设置为默认排序
///
/// @param[in]  pTableModel  表格控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月15日，新建函数
///
void QAssayParamDownloadDlg::SetDefaultSort(QStandardItemModel* pTableModel)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (pTableModel == nullptr)
    {
        return;
    }

    // 将数据加载到质控申请表中
    for (int iRow = 0; iRow < pTableModel->rowCount(); iRow++)
    {
        // 设置默认排序值
        for (int iCol = 0; iCol < pTableModel->columnCount(); iCol++)
        {
            // 获取Item
            QStandardItem* pItem = pTableModel->item(iRow, iCol);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 设置默认排序值为行号
            pItem->setData(iRow, UI_ITEM_ROLE_DEFAULT_SORT);
        }
    }

}

///
/// @brief 设置下载提示信息
///
/// @param[in] isSuccess 是否下载成功
/// @param[in] noticeInfo 提示信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::UpdateNoticeInfo(bool isSuccess, QString noticeInfo)
{
	// 获取提示控件
	auto NoticeLable = ui->tabWidget->currentWidget() == ui->tabFileImport ? ui->label_FileStatus : ui->label_NetStatus;
	if (NoticeLable == nullptr)
	{
		return;
	}

	// 如果为空则清空
	if (noticeInfo.isEmpty())
	{
		NoticeLable->clear();
		NoticeLable->setStyleSheet("");
		return;
	}

	NoticeLable->setText(noticeInfo);

    if (isSuccess)
    {
		NoticeLable->setStyleSheet("color:#11b93e; background: url(:/Leonis/resource/image/bk-tip-success.png) left no-repeat; padding-left: 22px;");
    }
    else
    {
		NoticeLable->setStyleSheet("color:#ff565f; background: url(:/Leonis/resource/image/bk-tip-fail.png) left no-repeat; padding-left: 22px;");
    }
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayParamDownloadDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}


///
/// @brief	参数下载模型
///
QParamDownloadModel::QParamDownloadModel(QObject* parent)
    :m_downloadClassify(DownloadClassify::DEV_INVALID)
    , m_downloadWay(DownloadWay::OTHER_WAY)
	, m_isRequesting(false)
	, m_skipFlag(false)
{

}

///
/// @brief 初始化模型
///
/// @param[in]  downloadDevType  下载类型
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月21日，新建函数
///
void QParamDownloadModel::InitModel(const DownloadClassify& downloadClassify)
{
    m_downloadClassify = downloadClassify;
}

///
/// @brief 设置参数下载方式
///
/// @param[in]  downloadWay  参数下载方式
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月21日，新建函数
///
void QParamDownloadModel::SetDownloadWay(DownloadWay downloadWay)
{
    m_downloadWay = downloadWay;
}

///
/// @brief 读取文件数据
///
/// @param[in]  path  文件路径
///
/// @return true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::ReadParamsFileData(const QString& path, QStringList& errInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    errInfo.clear();
    // 清空文件下载缓存
    m_fileImportInfo.clear();

    // 读取数据
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ULOG(LOG_ERROR, "Open() path(%s) Failed!", path);
        errInfo.append(tr("打开文件错误！%1").arg(path));
        return false;
    }

    // 逐行读取
    QTextStream reader(&file);
	reader.setCodec("UTF-8");
    QString lineTemp;
    ::adapter::tf::CloudAssayParam paramInfoTemp;
    auto CommInfoManager = CommonInformationManager::GetInstance();
    int saveIndex = 0;
    while (!reader.atEnd())
    {
        lineTemp = reader.readLine();
        if (lineTemp.isEmpty())
        {
            continue;
        }

        // 解析JSON
        if (!DecodeJson2ParamStu(lineTemp, paramInfoTemp))
        {
            ULOG(LOG_ERROR, "Failed decode josn:%s", lineTemp.toStdString().c_str());
            errInfo.append(tr("解析参数错误！%1").arg(lineTemp));
            continue;
        }

        // 过滤不支持的机型
        if (!CommInfoManager->IsSupportAimDevType(static_cast<tf::DeviceType::type>(paramInfoTemp.modelId)))
        {
            continue;
        }

        // 填入缓存
        m_fileImportInfo.insert(saveIndex++, paramInfoTemp);
    }

    file.close();

    return errInfo.length() == 0;
}

///
/// @brief 获取全部文件导入数据信息
///
///
/// @return 文件导入数据信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
QMap<int, ::adapter::tf::CloudAssayParam>& QParamDownloadModel::GetFileImportInfo()
{
    return m_fileImportInfo;
}

///
/// @brief 获取全部网络下载数据信息
///
///
/// @return 网络下载数据信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
QMap<int, ::adapter::tf::CloudAssayParam>& QParamDownloadModel::GetNetDownloadInfo()
{
    return m_netDownloadInfo;
}

///
/// @brief 根据查找索引导入文件参数
///
/// @param[in]  indexVec  信息查找索引
///
/// @return true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::ImportFileParams(const QVector<int>& indexVec)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 遍历导入信息
    for (auto index : indexVec)
    {
        // 是否存在该索引
        if (!m_fileImportInfo.contains(index))
        {
            continue;
        }

        // 解析并存储项目信息
        if (!SaveAssayInfo(m_fileImportInfo[index]))
        {
            ULOG(LOG_ERROR, "SaveAssayInfo Failed, assayName:%s ", m_fileImportInfo[index].assayName);
            return false;
        }
    }

    return true;
}

///
/// @brief 通过项目名称从网络上查询
///
/// @param[in]  assayName  项目名称
///
/// @return true:查询成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::SearchNetParams(const QString& assayName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空
    m_netDownloadInfo.clear();

    // thrift查询
    ::adapter::tf::QueryCloudAssayParamCond qry;
    ::adapter::tf::CloudAssayParamResp resp;

    // 产品区域，默认国内
    qry.__set_productAreas(::adapter::tf::CloudProductArea::AREA_CHINA);
    // 产品体系
    switch (m_downloadClassify)
    {
    case DownloadClassify::DEV_IM:
        qry.__set_productSystem(::adapter::tf::CloudProductSystem::SYSTEM_IMMUNE);
        // 产品类型
        qry.__set_modelId(tf::DeviceType::DEVICE_TYPE_I6000);
        break;
    case DownloadClassify::DEV_CH:
        qry.__set_productSystem(::adapter::tf::CloudProductSystem::SYSTEM_CHEMISTRY);
        // 产品类型
        qry.__set_modelId(tf::DeviceType::DEVICE_TYPE_C1000);
        break;
    default:
        ULOG(LOG_ERROR, "err downloadClassify!");
        return false;
    }

    // 项目名称
    qry.__set_assayName(assayName.toStdString());


    // 查询
    if (!DcsControlProxy::GetInstance()->GetCloudAssayParamUrls(resp, qry) || !resp.__isset.result ||
        resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "GetCloudAssayParamUrls Failed!");
        return false;
    }

    auto CommInfoManager = CommonInformationManager::GetInstance();
    int saveIndex = 0;
    for (auto param : resp.lstParamInfos)
    {
        // 过滤当前不支持的设备
        if (!CommInfoManager->IsSupportAimDevType(static_cast<tf::DeviceType::type>(param.modelId)))
        {
            continue;
        }

        // 填入缓存
        m_netDownloadInfo.insert(saveIndex++, param);
    }

    return true;
}

///
/// @brief 通过信息查找索引从网络下载参数
///
/// @param[in]  indexVec  信息查找索引
///
/// @return true:成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::DownloadNetParams(const QVector<int>& indexVec)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::vector<::adapter::tf::CloudAssayParam>  lscp;

    // 遍历导入信息
    for (auto index : indexVec)
    {
        // 是否存在该索引
        if (!m_netDownloadInfo.contains(index))
        {
            continue;
        }

        // 转换到请求vec中
        lscp.push_back(m_netDownloadInfo[index]);
    }

    // 通过URL查询信息
    ::adapter::tf::CloudAssayParamResp resp;
    if (!DcsControlProxy::GetInstance()->GetCloudAssayParamInfos(resp, lscp) || !resp.__isset.result ||
        resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || resp.lstParamInfos.empty())
    {
        ULOG(LOG_ERROR, "GetCloudAssayParamInfos Failed!");
        return false;
    }

    // 遍历解析存储
    for (auto param : resp.lstParamInfos)
    {
        // 解析并存储项目信息
        if (!SaveAssayInfo(param))
        {
            ULOG(LOG_ERROR, "SaveAssayInfo Failed, assayName:%s ", param.assayName);
            return false;
        }
    }

    return true;
}

///
/// @brief 设置状态
///
/// @param[in]  isRequesting  是否正在请求
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
///
void QParamDownloadModel::SetWorkStatus(bool isRequesting) 
{ 
	m_isRequesting = isRequesting; 

	// 开始工作，跳过标识置位
	if (isRequesting)
	{
		m_skipFlag = false;
	}
	
}

///
/// @brief 保存项目参数
///
/// @param[in]  info  项目信息
///
/// @return true:保存成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::SaveAssayInfo(const ::adapter::tf::CloudAssayParam& info)
{
    ULOG(LOG_INFO, "%s(),AssayInfo :%s", __FUNCTION__, info.paramInfo.c_str());

    // 参数检查
    std::string assayInfo = info.paramInfo;
    if (assayInfo.empty())
    {
        ULOG(LOG_ERROR, "AssayInfo is Empty!");
        return false;
    }

    // 根据下载时候选择设备类型分发对应接口
    switch (m_downloadClassify)
    {

        // 生化
    case QParamDownloadModel::DEV_CH:
    {
        // 存储生化
        if (!SaveChAssayInfo(assayInfo))
        {
            return false;
        }

    }break;

    // 免疫
    case QParamDownloadModel::DEV_IM:
    {
        // 存储项目信息
        if (!SaveImAssayInfo(assayInfo))
        {
            return false;
        }

    }break;

    default:
        return false;
        break;
    }

    return true;
}

///
/// @brief 保存生化项目参数
///
/// @param[in]  assayInfo  项目信息
///
/// @return true:保存成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::SaveChAssayInfo(const std::string& assayInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 解析
    ch::tf::ChAssayInfoList tfChInfo;
    if (!ch::LogicControlProxy::DecodeChAssayInfo(assayInfo, tfChInfo) ||
        tfChInfo.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
        !tfChInfo.__isset.gai || tfChInfo.cgais.size() != 1 || tfChInfo.cgais[0].csais.size() != 1)
    {
        ULOG(LOG_ERROR, "CH DecodeChAssayInfo() Failed!");
        return false;
    }

    // 单条结构，不是复合结构
    auto tfGAssayInfo = tfChInfo.gai;
    auto tfChGAssayInfo = tfChInfo.cgais[0].cgai;
    auto tfChSAssayInfo = tfChInfo.cgais[0].csais[0];

	// 查询通用项目
	tf::GeneralAssayInfoQueryCond gQryCond;
	gQryCond.__set_assayCode(tfGAssayInfo.assayCode);
	tf::GeneralAssayInfoQueryResp gQryResp;
	if (DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(gQryResp, gQryCond)
		&& (gQryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		&& (gQryResp.lstAssayInfos.size() == 1))
	{
		tfGAssayInfo.__set_id(gQryResp.lstAssayInfos[0].id);
	}

	// 查询生化通用项目(查到了则设置ID，没查到则新增并启用此版本)
	ch::tf::GeneralAssayInfoQueryCond chQryCond;
	chQryCond.__set_assayCode(tfChGAssayInfo.assayCode);
	chQryCond.__set_deviceType(tfChGAssayInfo.deviceType);
	//chQryCond.__set_version(tfChGAssayInfo.version);
	ch::tf::GeneralAssayInfoQueryResp chQryResp;
	if (ch::LogicControlProxy::QueryAssayConfigInfo(chQryCond, chQryResp)
		&& (chQryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		&& (chQryResp.lstGeneralAssayInfos.size() != 0))
	{
		// 是否已经有了版本的参数
		for (auto oldGAI : chQryResp.lstGeneralAssayInfos)
		{
			if (oldGAI.version == tfChGAssayInfo.version)
			{
				tfChGAssayInfo.__set_id(oldGAI.id);
			}
		}

		// 设置为不启用
		tfChGAssayInfo.__set_enable(false);
	}
	else
	{
		tfChGAssayInfo.__set_enable(true);
	}

	// 查询生化特殊项目（查到了则设置ID，没查到则新增，并将此样本设置为优先）
	::ch::tf::SpecialAssayInfoQueryCond chSQryCond;
	chSQryCond.__set_assayCode(tfChSAssayInfo.assayCode);
	chSQryCond.__set_deviceType(tfChSAssayInfo.deviceType);
	//chSQryCond.__set_sampleSourceType(tfChSAssayInfo.sampleSourceType);
	chSQryCond.__set_version(tfChSAssayInfo.version);
	ch::tf::SpecialAssayInfoQueryResp chSQryResp;
	if (ch::LogicControlProxy::QuerySpecialAssayInfo(chSQryResp, chSQryCond)
		&& (chSQryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		&& (chSQryResp.lstSpecialAssayInfos.size() != 0))
	{
		// 是否已经有了该样本类型的参数
		for (auto oldSAI : chSQryResp.lstSpecialAssayInfos)
		{
			if (oldSAI.sampleSourceType == tfChSAssayInfo.sampleSourceType)
			{
				tfChSAssayInfo.__set_id(oldSAI.id);
			}
		}

		// 设置为低优先级
		tfChSAssayInfo.__set_priority(false);
	}
	else
	{
		tfChSAssayInfo.__set_priority(true);
	}

	// 如果三张表设置了id,则不进行设置，并提示跳过
	if (tfGAssayInfo.__isset.id && 
		tfChGAssayInfo.__isset.id && 
		tfChSAssayInfo.__isset.id)
	{
		// 第一次跳过
		if (!m_skipFlag)
		{
			m_skipFlag = true;
		}

		return true;
	}

    // 特殊处理样本量，样本量必须为三项，不足三项补齐三项
    int volNum = 3 - tfChSAssayInfo.sampleAspirateVols.size();
    for (int i = 0; i < volNum; i++)
    {
        ::ch::tf::SampleAspirateVol sv;
        sv.__set_originalSample(0);
        tfChSAssayInfo.sampleAspirateVols.emplace_back(std::move(sv));
    }

	// 新增
	if (!ch::LogicControlProxy::AddAssayInfo(tfGAssayInfo, tfChGAssayInfo, tfChSAssayInfo))
	{
		ULOG(LOG_ERROR, "save ch asaay params faield!");
		return false;
	}

	// 添加操作日志，暂时使用常量字符串
	if (!AddOptLog(::tf::OperationType::ADD, tr("新增项目") + QString::fromStdString(tfGAssayInfo.assayName)))
	{
		ULOG(LOG_ERROR, "Add add assay operate log failed !");
	}

	return true;
}

///
/// @brief 通用项目信息表导入
///		   项目通道号号相同则覆盖，没有则新增
///
/// @param[in]  gai  通用项目信息
///
/// @return true:导入成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::ImportGeneralAssayParams(const ::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查询数据库是否有目标项目
    bool isContainsAimAssay = true;
    tf::GeneralAssayInfoQueryCond gQryCond;
    gQryCond.__set_assayCode(gai.assayCode);
    tf::GeneralAssayInfoQueryResp gQryResp;
    if (!DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(gQryResp, gQryCond)
        || (gQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        || (gQryResp.lstAssayInfos.size() != 1))
    {
        isContainsAimAssay = false;
    }

    // 添加进数据库
    if (!isContainsAimAssay)
    {
        // 没有该项目，则新增
        tf::ResultLong ret;
        DcsControlProxy::GetInstance()->AddGeneralAssayInfo(ret, gai);
        if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "AddGeneralAssayInfo Failed");
            return false;
        }

        // 添加操作日志，暂时使用常量字符串
        if (gai.__isset.assayName && !AddOptLog(::tf::OperationType::ADD, tr("新增项目") + QString::fromStdString(gai.assayName)))
        {
            ULOG(LOG_ERROR, "Add add assay operate log failed !");
        }
    }
    else
    {
        // 有该项目，则覆盖
        ::tf::GeneralAssayInfo gaiTemp = gai;
        gaiTemp.__set_id(gQryResp.lstAssayInfos[0].id);
        if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(gaiTemp))
        {
            ULOG(LOG_ERROR, "ModifyGeneralAssayInfo Failed");
            return false;
        }
    }

    return true;
}

///
/// @brief 生化通用项目信息表导入
///		   项目通道号，仪器类型，版本信息相同则覆盖，没有则新增
///
/// @param[in]  cgai  生化通用项目信息
///
/// @return true:导入成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::ImportChGeneralAssayParams(const ch::tf::GeneralAssayInfo& cgai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查询是否有该项目信息
    bool isContainsAimAssay = true;
    ch::tf::GeneralAssayInfoQueryCond chQryCond;
    chQryCond.__set_assayCode(cgai.assayCode);
    chQryCond.__set_deviceType(cgai.deviceType);
    chQryCond.__set_version(cgai.version);
    ch::tf::GeneralAssayInfoQueryResp chQryResp;
    if (!ch::LogicControlProxy::QueryAssayConfigInfo(chQryCond, chQryResp)
        || (chQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        || (chQryResp.lstGeneralAssayInfos.size() != 1))
    {
        isContainsAimAssay = false;
    }

    // 添加进数据库
    if (!isContainsAimAssay)
    {
        // 没有则新增
        tf::ResultLong ret;
        ch::LogicControlProxy::AddGeneralAssayInfo(ret, cgai);
        if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ch::LogicControlProxy::AddGeneralAssayInfo Failed");
            return false;
        }
    }
    else
    {
        // 有该项目，则覆盖
        ch::tf::GeneralAssayInfo gaiTemp = cgai;
        gaiTemp.__set_id(chQryResp.lstGeneralAssayInfos[0].id);
        if (!ch::LogicControlProxy::ModifyAssayConfigInfo(gaiTemp))
        {
            ULOG(LOG_ERROR, "ModifyGeneralAssayInfo Failed");
            return false;
        }
    }

    // 导入的项目必有一个启用，如果当前项目中有启用的，则将原来的项目不启用
    if (cgai.enable == true)
    {
        auto comManager = CommonInformationManager::GetInstance();
        auto chGIS = comManager->GetAllChemistryAssayInfo(cgai.assayCode, cgai.deviceType);
        for (auto chgi : chGIS)
        {
            // 将原来启用设置为不启用
            if (chgi->enable == true)
            {
                ch::tf::GeneralAssayInfo chgaiT = *chgi;
                chgaiT.__set_enable(false);
                if (!ch::LogicControlProxy::ModifyAssayConfigInfo(chgaiT))
                {
                    ULOG(LOG_ERROR, "ModifyGeneralAssayInfo Failed");
                    return false;
                }
            }
        }
    }

    return true;
}

///
/// @brief 生化特殊项目信息表导入
///		   项目通道号,仪器类型,参数版本, 样本源类型相匹配则覆盖，没有则新增
///
/// @param[in]  csai  生化特殊项目信息
///
/// @return true:导入成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::ImportChSpecialAssayParams(const ch::tf::SpecialAssayInfo& csai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查询是否有该项目信息
    bool isContainsAimAssay = true;
    ::ch::tf::SpecialAssayInfoQueryCond chSQryCond;
    chSQryCond.__set_assayCode(csai.assayCode);
    chSQryCond.__set_deviceType(csai.deviceType);
    chSQryCond.__set_sampleSourceType(csai.sampleSourceType);
    chSQryCond.__set_version(csai.version);
    ch::tf::SpecialAssayInfoQueryResp chSQryResp;
    if (!ch::LogicControlProxy::QuerySpecialAssayInfo(chSQryResp, chSQryCond)
        || (chSQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        || (chSQryResp.lstSpecialAssayInfos.size() != 1))
    {
        isContainsAimAssay = false;
    }

    // 添加进数据库
    if (!isContainsAimAssay)
    {
        // 没有则新增
        tf::ResultLong ret;
        ch::LogicControlProxy::AddSpecialAssayInfo(ret, csai);
        if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ch::LogicControlProxy::AddSpecialAssayInfo Failed");
            return false;
        }
    }
    else
    {
        // 有该项目，则覆盖
        ch::tf::SpecialAssayInfo csaiTemp = csai;
        csaiTemp.__set_id(chSQryResp.lstSpecialAssayInfos[0].id);
        if (ch::LogicControlProxy::ModifySpecialAssayInfo(csaiTemp) != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ModifySpecialAssayInfo Failed");
            return false;
        }
    }

    // 导入的特殊项目必有一个启用，如果当前特殊项目中有启用的，则将原来的项目不启用
    if (csai.priority == true)
    {
        auto comManager = CommonInformationManager::GetInstance();
        auto chSIS = comManager->GetChemistrySpecialAssayInfo(csai.assayCode, csai.deviceType, -1, csai.version);
        for (auto chsi : chSIS)
        {
            // 将原来启用设置为不启用
            if (chsi->priority == true)
            {
                ch::tf::SpecialAssayInfo chsaiT = *chsi;
                chsaiT.__set_priority(false);
                if (!ch::LogicControlProxy::ModifySpecialAssayInfo(chsaiT) != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
                {
                    ULOG(LOG_ERROR, "ModifyGeneralAssayInfo Failed");
                    return false;
                }
            }
        }
    }

    return true;
}

///
/// @brief 保存免疫项目参数
///
/// @param[in]  assayInfo  项目信息
///
/// @return true:保存成功 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::SaveImAssayInfo(const std::string& assayInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 解密
    CEnCodeString imDec;
    std::string strDecoded;
    if (!imDec.DecodeString(assayInfo, strDecoded))
    {
        ULOG(LOG_ERROR, "IM Decode Error!");
        return false;
    }

    // 存储
    if (!im::LogicControlProxy::ParaseAssayBarCodeToDB(strDecoded))
    {
        ULOG(LOG_ERROR, "IM Save AssayInfo Failed!");
        return false;
    }

    return true;
}

///
/// @brief 将JSON信息转换为项目参数信息
///
/// @param[in]  json  源JSON信息
/// @param[in]  paramStu  参数信息
///
/// @return true:转换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
///
bool QParamDownloadModel::DecodeJson2ParamStu(const QString& json, ::adapter::tf::CloudAssayParam& paramStu)
{
    if (json.isEmpty())
    {
        return false;
    }

    try
    {
        Document jsonDoc;
        if (!RapidjsonUtil::ParseRapidjson(json.toStdString(), jsonDoc))
        {
            return false;
        }

        paramStu.__set_assayName(RapidjsonUtil::GetStringForKey(jsonDoc, "assayName"));
        paramStu.__set_assayCode(RapidjsonUtil::GetIntForKey(jsonDoc, "assayCode"));
        paramStu.__set_sampleSourceType(RapidjsonUtil::GetIntForKey(jsonDoc, "sampleType"));
        paramStu.__set_version(RapidjsonUtil::GetStringForKey(jsonDoc, "version"));
        paramStu.__set_modelId(RapidjsonUtil::GetIntForKey(jsonDoc, "deviceType"));
        paramStu.__set_paramInfo(RapidjsonUtil::GetStringForKey(jsonDoc, "assayInfo"));
    }
    catch (const std::exception& ex)
    {
        ULOG(LOG_WARN, "exception: %s", ex.what());
        return false;
    }

    return true;
}

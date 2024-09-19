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
/// @file     QQcDownloadRegDlg.cpp
/// @brief    质控下载登记
///
/// @author   8580/GongZhiQiang
/// @date     2023年4月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年4月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <thread>
#include <QVariant>
#include <QDateTime>
#include "QQcDownloadRegDlg.h"
#include "ui_QQcDownloadRegDlg.h"
#include <QStandardItemModel>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "SortHeaderView.h"
#include "thrift/DcsControlProxy.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"
#include "src/leonis/manager/OperationLogManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CReadOnlyDelegate.h"
#include "uidcsadapter/uidcsadapter.h"

Q_DECLARE_METATYPE(::adapter::tf::CloudQcData)
Q_DECLARE_METATYPE(::adapter::tf::CloudQcComp)
Q_DECLARE_METATYPE(::adapter::tf::CloudQcDoc)

#define  COL_CNT_OF_DOWNLOAD_QC_TABLE                   (10)          // 默认质控表默认列数
#define  COL_SELECT                                     (9)           // 选折列
#define  COL_CALI_LOT                                   (8)           // 校准品批号
#define  COL_REAG_LOT                                   (7)           // 试剂批号
#define  COL_ITEM_NAME                                  (6)           // 项目名称
#define  COL_EXP_DATE                                   (5)           // 失效日期

QQcDownloadRegDlg::QQcDownloadRegDlg(QWidget *parent)
	: QWidget(parent)
	, m_QcDownloadGroup(nullptr)
    , m_selCloudQcDoc(nullptr)
    , m_rowIndex(-1)
    , m_qcDoc(nullptr)
	, m_isRequesting(false)
{
    ui = new Ui::QQcDownloadRegDlg();
	ui->setupUi(this);

    m_fixLengthSn = DcsControlProxy::GetInstance()->GetQcConfigLengthSn();

    ui->lineEdit_QcLot->setValidator(new QIntValidator(0, 9999999));
    ui->lineEdit_QcLot->setMaxLength(7);

	// 初始化对话框
	InitDialg();

	// 初始化信号槽
	InitConnects();
}

QQcDownloadRegDlg::~QQcDownloadRegDlg()
{
}

///
/// @brief 初始化对话框
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年4月26日，新建函数
///
void QQcDownloadRegDlg::InitDialg()
{
	// 设置标题
	//SetTitleName(tr("下载登记"));

	if (m_QcDownloadGroup == nullptr)
	{
		m_QcDownloadGroup = new QStandardItemModel(this);
	}
	
	// 设置表头
	m_QcDownloadGroup->clear();
	QStringList horheadList;
	horheadList << tr("质控品编号") << tr("质控品名称") << tr("质控品类型") << tr("质控品水平") << tr("质控品批号") 
		        << tr("失效日期") << tr("项目名称") << tr("试剂批号")   << tr("校准品批号")   << tr("选择");
	m_QcDownloadGroup->setHorizontalHeaderLabels(horheadList);
	m_QcDownloadGroup->setColumnCount(COL_CNT_OF_DOWNLOAD_QC_TABLE);
    m_QcDownloadGroup->setHeaderData(horheadList.size() - 1, Qt::Horizontal, UI_TEXT_COLOR_HEADER_MARK, Qt::ForegroundRole);
	ui->tableView->setModel(m_QcDownloadGroup);
    ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
	ui->tableView->verticalHeader()->hide();
    ui->tableView->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(3, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(4, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(5, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(6, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(7, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(8, new CReadOnlyDelegate(this));
    ui->tableView->setItemDelegateForColumn(9, new CReadOnlyDelegate(this));

    ui->tableView->setColumnWidth(0, 150);
    ui->tableView->setColumnWidth(1, 150);
    ui->tableView->setColumnWidth(2, 150);
    ui->tableView->setColumnWidth(3, 120);
    ui->tableView->setColumnWidth(4, 150);
    ui->tableView->setColumnWidth(5, 170);
    ui->tableView->setColumnWidth(6, 170);
    ui->tableView->setColumnWidth(7, 170);
    ui->tableView->setColumnWidth(8, 170);
    ui->tableView->setColumnWidth(9, 50);

    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setAutoScroll(false);

    // 水平下拉，选中第一水平
    ui->QcLevelCombo->clearEditText();
    ui->QcLevelCombo->addItem("");
    // 水平最高为6-bug27428
    for (int iLev = 1; iLev <= 6; iLev++)
    {
        ui->QcLevelCombo->addItem(QString::number(iLev));
    }
    ui->QcLevelCombo->setCurrentIndex(0);
	
    // 有效期内默认
    ui->QcExpiryTimeCombo->clearEditText();
	ui->QcExpiryTimeCombo->addItem(tr("全部"));
    ui->QcExpiryTimeCombo->addItem(tr("有效期内"));
    ui->QcExpiryTimeCombo->setCurrentIndex(1);

	// 适用平台
	ui->QcSystemCombo->clear();
	ui->QcSystemCombo->addItem(tr("生化"));
	ui->QcSystemCombo->addItem(tr("免疫"));
	// 单机版隐藏
	if (gUiAdapterPtr()->WhetherOnlyChDev())
	{
		ui->QcSystemCombo->setVisible(false);
		ui->QcSystemLab->setVisible(false);
		ui->QcSystemCombo->setCurrentIndex(0);
	}

	if (gUiAdapterPtr()->WhetherOnlyImDev())
	{
		ui->QcSystemCombo->setVisible(false);
		ui->QcSystemLab->setVisible(false);
		ui->QcSystemCombo->setCurrentIndex(1);
	}
	
}

///
/// @brief 初始化信号槽
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年4月26日，新建函数
///
void QQcDownloadRegDlg::InitConnects()
{
	// 取消按钮
	connect(ui->pushButton_Cancel, &QPushButton::clicked, this, [&]() {emit SigClosed(); });
    // 查询按钮
    connect(ui->pushButton_Search, &QPushButton::clicked, this, &QQcDownloadRegDlg::OnClickSearchBtn);
    // 根据选择进行按钮使能设置
    connect(ui->tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTblItemClicked(const QModelIndex&)));
    // 下载按钮
    connect(ui->pushButton_Download, &QPushButton::clicked, this, &QQcDownloadRegDlg::OnClickDownload);
    // 重置按钮
    connect(ui->pushButton_Reset, &QPushButton::clicked, this, &QQcDownloadRegDlg::OnClickReset);
}

///
/// @brief 进入下载模式
///
/// @return true 成功
///
/// @par History:
/// @li 1226/zhangjing，2023年12月1日，新建函数
///
bool QQcDownloadRegDlg::StartDownload(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    m_rowIndex = rowIndex;
    m_qcDoc = curQcDoc;

    return true;
}

///
/// @brief  点击查询按钮
///
/// @par History:
/// @li 1226/zhangjing，2023年11月28日，新建函数
///
void QQcDownloadRegDlg::OnClickSearchBtn()
{
    // 验证空
    if (ui->lineEdit_QcName->text().isEmpty()
        && ui->lineEdit_QcLot->text().isEmpty())
    {
        TipDlg(tr("筛选质控品信息时名称和批号不能同时为空，请重新输入！")).exec();
        return;
    }

    // 验证长度
    if (ui->lineEdit_QcName->text().length() > 30 ||
        ui->lineEdit_QcLot->text().length() > 7)
    {
        TipDlg(tr("筛选质控品的名称长度不能超过30个字符，批号长度不能超过7个字符，请重新输入！")).exec();
        return;
    }
    ::adapter::tf::QueryCloudQcCond qry;
    if (!ui->lineEdit_QcName->text().isEmpty())
    {
        qry.__set_qcName((ui->lineEdit_QcName->text()).toStdString());
    }
    if (!ui->lineEdit_QcLot->text().isEmpty())
    {
        qry.__set_qcLot((ui->lineEdit_QcLot->text()).toStdString());
    }
    auto levelIndx = ui->QcLevelCombo->currentIndex();
    if (levelIndx > 0)
    {
        qry.__set_qcLevel(QString::number(ui->QcLevelCombo->currentIndex()).toStdString());
    }

	// 有效期(0:全部，1:有效期内)
	int expiry = ui->QcExpiryTimeCombo->currentIndex();
	qry.__set_hasExpiry(expiry);
    
    qry.__set_productAreas(::adapter::tf::CloudProductArea::AREA_CHINA);    // 默认国内
    qry.__set_productSystem((ui->QcSystemCombo->currentIndex()==1)? 
		::adapter::tf::CloudProductSystem::SYSTEM_IMMUNE : adapter::tf::CloudProductSystem::SYSTEM_CHEMISTRY); 
    qry.__set_devType((ui->QcSystemCombo->currentIndex() == 1) ? "i 6000":"C 1000");  // 老版本为字符串，所以不修改
    
	 // 防止多次进入
	if (m_isRequesting)
	{
		return;
	}

	// 清空当前结果
	m_QcDownloadGroup->setRowCount(0);
	m_downloadDataCache.clear();

	// 创建一个新的线程来查询
	auto searchFunc = [&](::adapter::tf::QueryCloudQcCond qcqc) {

		// 初始化状态
		POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("查询中..."), true);
		connect(this, &QQcDownloadRegDlg::searchResult, this, &QQcDownloadRegDlg::OnSearchHander);
		m_isRequesting = true;

		bool ret = true;
		::adapter::tf::CloudQcDocResp resp;
		if (!DcsControlProxy::GetInstance()->DownloadQcDocs(resp, qcqc) || !resp.__isset.result ||
			resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || resp.lstQcDocs.size() <= 0)
		{
			ret = false;
		}
		else
		{
			// 更新缓存
			m_downloadDataCache = resp.lstQcDocs;
		}

		emit searchResult(ret);
	};
	std::thread searchThread(searchFunc, qry);
	searchThread.detach();
}

///
/// @brief 查询处理
///
/// @param[in]  isSuccess  是否查询成功
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
///
void QQcDownloadRegDlg::OnSearchHander(bool isSuccess)
{
	// 复位条件
	m_isRequesting = false;
	disconnect(this, &QQcDownloadRegDlg::searchResult, this, &QQcDownloadRegDlg::OnSearchHander);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);

	// 查询失败
	if (!isSuccess)
	{
		TipDlg(tr("质控品查询失败！")).exec();
		return;
	}

	// 更新数据
	OnDisplayCloudQcDocs(m_downloadDataCache);
}

///
/// @brief  显示已筛选到的doc
///
/// @param[in]  vtDocs  查询结果
///
/// @par History:
/// @li 1226/zhangjing，2023年11月29日，新建函数
///
void QQcDownloadRegDlg::OnDisplayCloudQcDocs(std::vector <::adapter::tf::CloudQcDoc> vtDocs)
{
    int iRowCount = 0;
    for (auto cloudQc : vtDocs)
    {
        iRowCount += cloudQc.dataCount;
    }
    // 重新查询要清空，bug26092
    m_selCloudQcDoc = nullptr;
    m_QcDownloadGroup->setRowCount(iRowCount);
    if (iRowCount <= 0)
    {
        return;
    }
    int iLastRowIndx = 0;
    for (auto cloudQc : vtDocs)
    {
        if (cloudQc.compositions.size() < 1)
        {
            continue;
        }
        int colum = 0;
        int iBeginRow = iLastRowIndx;
        // 编号
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum, new QStandardItem(QString::fromStdString(cloudQc.sn)));
        m_QcDownloadGroup->item(iLastRowIndx, colum++)->setData(QVariant::fromValue<::adapter::tf::CloudQcDoc>(cloudQc), Qt::UserRole);
        // 名称
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum++, new QStandardItem(QString::fromStdString(cloudQc.name)));
        // 质控品样本类型
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum++, new QStandardItem(ThriftEnumTrans::GetSourceTypeName(cloudQc.sampleSourceType)));
        // 质控水平
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum++, new QStandardItem(QString::number(cloudQc.level)));
        // 质控品批号
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum++, new QStandardItem(QString::fromStdString(cloudQc.lot)));
        // 质控品失效日期
        ui->tableView->setSpan(iLastRowIndx, colum, cloudQc.dataCount, 1);
        m_QcDownloadGroup->setItem(iLastRowIndx, colum++, new QStandardItem(QString::fromStdString(cloudQc.expireTime)));
        // 项目信息
        for (auto comp: cloudQc.compositions)
        {
            int iCompBeginRow = iLastRowIndx;
            if (comp.qcData.size() > 1)
            {
                ui->tableView->setSpan(iLastRowIndx, colum, comp.qcData.size(), 1);               
            }
            auto genInfo = CommonInformationManager::GetInstance()->GetAssayInfo(comp.assayCode);
            if (nullptr == genInfo)
            {
                m_QcDownloadGroup->setItem(iLastRowIndx, COL_ITEM_NAME, new QStandardItem(QString::number(comp.assayCode)));
            }
            else
            {
                m_QcDownloadGroup->setItem(iLastRowIndx, COL_ITEM_NAME, new QStandardItem(QString::fromStdString(genInfo->printName)));
            }
            m_QcDownloadGroup->item(iLastRowIndx, COL_ITEM_NAME)->setData(QVariant::fromValue<::adapter::tf::CloudQcComp>(comp), Qt::UserRole);
            for (auto data: comp.qcData)
            {
                // 试剂批号
                m_QcDownloadGroup->setItem(iLastRowIndx, COL_REAG_LOT, new QStandardItem(QString::fromStdString(data.reagLot)));
                m_QcDownloadGroup->item(iLastRowIndx, COL_REAG_LOT)->setData(iBeginRow, Qt::UserRole);
                // 校准品批号
                m_QcDownloadGroup->setItem(iLastRowIndx, COL_CALI_LOT, new QStandardItem(QString::fromStdString(data.caliLot)));
                m_QcDownloadGroup->item(iLastRowIndx, COL_CALI_LOT)->setData(iCompBeginRow, Qt::UserRole);
                // 选择
                m_QcDownloadGroup->setItem(iLastRowIndx, COL_SELECT, new QStandardItem(""));
                m_QcDownloadGroup->item(iLastRowIndx, COL_SELECT)->setIcon(data.isSelect ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());
                m_QcDownloadGroup->item(iLastRowIndx, COL_SELECT)->setData(QVariant::fromValue<::adapter::tf::CloudQcData>(data),Qt::UserRole);
                iLastRowIndx++;
            }
        }
    }

}

///
/// @brief  质控下载表格被点击
///
/// @param[in]  modIdx 被点击表格信息 
///
/// @par History:
/// @li 1226/zhangjing，2022年11月29日，新建函数
///
void QQcDownloadRegDlg::OnTblItemClicked(const QModelIndex& modIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->tableView->clearSelection();
    // 检查参数
    if (!modIdx.isValid() || (m_QcDownloadGroup->item(modIdx.row(), COL_REAG_LOT) == Q_NULLPTR) ||
        (m_QcDownloadGroup->item(modIdx.row(), COL_CALI_LOT) == Q_NULLPTR) ||
        (m_QcDownloadGroup->item(modIdx.row(), COL_SELECT) == Q_NULLPTR))
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        return;
    } 

    // 获取缓存试剂信息
    int iBeginRow = m_QcDownloadGroup->item(modIdx.row(), COL_REAG_LOT)->data(Qt::UserRole).value<int>();    
    int iCompBeginRow = m_QcDownloadGroup->item(modIdx.row(), COL_CALI_LOT)->data(Qt::UserRole).value<int>();
    ::adapter::tf::CloudQcData data = m_QcDownloadGroup->item(modIdx.row(), COL_SELECT)->data(Qt::UserRole).value<::adapter::tf::CloudQcData>();
    if ((m_QcDownloadGroup->item(iCompBeginRow, COL_ITEM_NAME) == Q_NULLPTR) ||
        (m_QcDownloadGroup->item(iBeginRow, 0) == Q_NULLPTR))
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        return;
    }

    for (int i = iBeginRow; i < m_QcDownloadGroup->rowCount(); i++)
    {
        // 如果质控品comp.qcData为空则该值则直接退出
        if (m_QcDownloadGroup->item(i, COL_REAG_LOT) == nullptr)
        {
            break;
        }
        if (iBeginRow != m_QcDownloadGroup->item(i, COL_REAG_LOT)->data(Qt::UserRole).value<int>())
        {
            break;
        }
        ui->tableView->selectRow(i);
    }

    ::adapter::tf::CloudQcComp comp = m_QcDownloadGroup->item(iCompBeginRow, COL_ITEM_NAME)->data(Qt::UserRole).value<::adapter::tf::CloudQcComp>();
    ::adapter::tf::CloudQcDoc doc = m_QcDownloadGroup->item(iBeginRow, 0)->data(Qt::UserRole).value<::adapter::tf::CloudQcDoc>();

    // 检查参数
    if ((modIdx.column() != COL_SELECT) )
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        m_selCloudQcDoc = std::make_shared<::adapter::tf::CloudQcDoc>(doc);
        return;
    }

    int iCompRowCnt = comp.qcData.size();
    std::vector<adapter::tf::CloudQcData> vtDatas;

    // 选择状态反转
    data.__set_isSelect(!data.isSelect);
    vtDatas.push_back(data);
    // 成功则更新选择图标
    m_QcDownloadGroup->item(modIdx.row(), COL_SELECT)->setIcon(data.isSelect ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());
    m_QcDownloadGroup->item(modIdx.row(), COL_SELECT)->setData(QVariant::fromValue<::adapter::tf::CloudQcData>(data), Qt::UserRole);

    if (data.isSelect && iCompRowCnt > 1)
    {
        for (int rowInd = iCompRowCnt; rowInd < iCompRowCnt + iCompRowCnt; rowInd++)
        {
            if (rowInd == modIdx.row())
            {
                continue;
            }
            ::adapter::tf::CloudQcData dat = m_QcDownloadGroup->item(rowInd, COL_SELECT)->data(Qt::UserRole).value<::adapter::tf::CloudQcData>();
            if (dat.__isset.isSelect && dat.isSelect)
            {
                dat.__set_isSelect(false);
                m_QcDownloadGroup->item(rowInd, COL_SELECT)->setIcon(QIcon());
                m_QcDownloadGroup->item(rowInd, COL_SELECT)->setData(QVariant::fromValue<::adapter::tf::CloudQcData>(dat), Qt::UserRole);
            }
            vtDatas.push_back(dat);
        }
    }

    comp.__set_qcData(vtDatas);
    m_QcDownloadGroup->item(iCompBeginRow, COL_ITEM_NAME)->setData(QVariant::fromValue<::adapter::tf::CloudQcComp>(comp), Qt::UserRole);
    std::vector<::adapter::tf::CloudQcComp> vtComps;
    for (auto c : doc.compositions)
    {
        if (c.assayCode == comp.assayCode)
        {
            vtComps.push_back(comp);
        }
        else
        {
            vtComps.push_back(c);
        }
    }
    doc.__set_compositions(vtComps);
    m_QcDownloadGroup->item(iBeginRow, 0)->setData(QVariant::fromValue<::adapter::tf::CloudQcDoc>(doc), Qt::UserRole);
    m_selCloudQcDoc = std::make_shared<::adapter::tf::CloudQcDoc>(doc);

}

///
/// @bref 保存前ISE特殊检测
///		  ISE(包含Na, K, CL项目)样本类型只支持“血清血浆”或“尿液”
///
/// @par History:
/// @li 8580/GongZhiQiang, 2023年11月10日，新建函数
///
bool  QQcDownloadRegDlg::IseSpecialCheck(const ::tf::QcDoc& saveQcDoc)
{
    // 遍历检查
    bool isContansIse = false;
    for (const auto& qDC : saveQcDoc.compositions)
    {
        if (qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA ||
            qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K ||
            qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            isContansIse = true;
            break;
        }
    }

    // 检查样本类型
    if (isContansIse &&
        (saveQcDoc.sampleSourceType != ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ &&
            saveQcDoc.sampleSourceType != ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
    {
        return false;
    }

    return true;
}

///
/// @bref
///		检测是否在数据库中存在质控品编号批号水平重复的质控品
///
/// @param[in] saveDoc 编辑后的质控品
/// @return true 无重复可继续流程，false重复不可修改
///
/// @par History:
/// @li 8276/huchunli, 2023年8月17日，新建函数
///
bool QQcDownloadRegDlg::CheckRepeatQc(const ::tf::QcDoc& saveDoc)
{
    ULOG(LOG_INFO, __FUNCTION__);

    tf::QcDocQueryResp resp;
    tf::QcDocQueryCond cond;
    cond.__set_lot(saveDoc.lot);
    cond.__set_sn(saveDoc.sn);
    cond.__set_level(saveDoc.level);
    if (DcsControlProxy::GetInstance()->QueryQcDoc(resp, cond) && resp.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS && \
        resp.lstQcDocs.size() > 0)
    {
        // 当编辑行原来为空，或原来的id非编辑后的id，则认为是重复了
        if (m_qcDoc == nullptr || \
            (m_qcDoc->__isset.id && resp.lstQcDocs[0].__isset.id && m_qcDoc->id != resp.lstQcDocs[0].id))
        {
            TipDlg(tr("相同编号、批号和水平的质控品信息已存在，请重新输入！")).exec();
            return false;
        }
    }

    return true;
}

///
/// @brief  点击下载按钮
///
/// @par History:
/// @li 1226/zhangjing，2023年11月29日，新建函数
///
void QQcDownloadRegDlg::OnClickDownload(void)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    if (nullptr == m_selCloudQcDoc)
    {
        TipDlg(tr("未选中任何质控品，请选择后再下载！")).exec();
        return;
    }

    ::tf::QcDoc qcDoc;
    qcDoc.__set_name(m_selCloudQcDoc->name);
    qcDoc.__set_shortName(m_selCloudQcDoc->name);
    qcDoc.__set_level(m_selCloudQcDoc->level);
    auto strSn = m_selCloudQcDoc->sn;
    if (strSn.size() == m_fixLengthSn)
    {
        strSn += std::to_string(m_selCloudQcDoc->level);
    }
    qcDoc.__set_sn(strSn);
    qcDoc.__set_lot(m_selCloudQcDoc->lot);
    qcDoc.__set_sampleSourceType(m_selCloudQcDoc->sampleSourceType);
    QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(m_selCloudQcDoc->expireTime), UI_DATE_FORMAT);
    qcDoc.__set_expireTime(expDateTime.toString(UI_DATE_TIME_FORMAT).toStdString());
    qcDoc.__set_registType(tf::DocRegistType::REG_DOWNLOAD);
    qcDoc.__set_index(m_rowIndex);
    qcDoc.__set_oneDimensionalCode(m_selCloudQcDoc->oneDimensionalCode);
    qcDoc.__set_tubeType(tf::TubeType::TUBE_TYPE_STORE);

    std::vector<tf::QcComposition> vtComps;
    for (const auto& comp : m_selCloudQcDoc->compositions)
    {
        for (auto data : comp.qcData)
        {
            if (!data.__isset.isSelect || !data.isSelect)
            {
                continue;
            }
            tf::QcComposition qcp;
            qcp.__set_assayCode(comp.assayCode);
            qcp.__set_sd(data.sd);
            qcp.__set_targetValue(data.targetValue);
            // 设备类型
			qcp.__set_deviceType(GetDeciveTypeByAssaycode(comp.assayCode));
            vtComps.push_back(qcp);
        }
    }
    qcDoc.__set_compositions(vtComps);

    // ISE特殊检查，只要包含（Na,K,Cl）其中一个项目，样本类型就必须选择“血清/血浆”或“尿液”
    if (!IseSpecialCheck(qcDoc))
    {
        TipDlg(tr("ISE（Na,K,Cl）项目的样本类型只支持“血清/血浆”或“尿液”!")).exec();
        return;
    }

    // 判断是否重复
    if (!CheckRepeatQc(qcDoc))
    {
        ULOG(LOG_ERROR, "Repeat qc error.");
        // 重复报警bug24755
        //TipDlg(tr("重复添加同一质控品!")).exec();
        return;
    }

    // 如果是在原有质控品选中的情况下添加质控品
    int keyValue = -1;
    if (m_qcDoc)
    {
        keyValue = m_qcDoc->id;
        if (m_qcDoc->sn == qcDoc.sn)
        {
            tf::QcDoc tempDoc = qcDoc;
            tempDoc.__set_id(m_qcDoc->id);
            tf::ResultLong ret;
            if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, tempDoc) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg(tr("覆盖操作执行失败！")).exec();
                return ;
            }
        }
        else
        {
            TipDlg(tr("当前质控品与修改后的质控品的编号存在差异，不能执行覆盖操作！")).exec();
            return ;
        }
        // 添加操作日志
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::MOD, qcDoc, m_qcDoc);
    }
    else
    {
        tf::ResultLong ret;
        DcsControlProxy::GetInstance()->AddQcDoc(ret, qcDoc);
        if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            TipDlg(tr("新增质控品操作执行失败！")).exec();
            return;
        }
        keyValue = ret.value;

        // 添加操作日志
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::ADD, qcDoc);
    }

    emit SigClosed();
    emit Update();

    m_qcDoc = std::make_shared<tf::QcDoc>(qcDoc);
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    tf::QcDocUpdate qcDocUpdate;
    qcDocUpdate.__set_index(m_rowIndex);
    qcDocUpdate.__set_db(keyValue);
    qcDocUpdate.__set_updateType(tf::UpdateType::UPDATE_TYPE_ADD);
    vecQcDocUpdate.push_back(qcDocUpdate);
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);
}

///
/// @brief 根据项目编号获取机型
///
/// @param[in]  assayCode  项目编号
///
/// @return 机型，失败返回无效机型
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年5月17日，新建函数
///
tf::DeviceType::type QQcDownloadRegDlg::GetDeciveTypeByAssaycode(int assayCode)
{
	// 生化
	if (assayCode >= tf::AssayCodeRange::CH_RANGE_MIN && assayCode <= tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
	{
		return tf::DeviceType::DEVICE_TYPE_C1000;
	}
	// ISE
	else if(assayCode > tf::AssayCodeRange::CH_OPEN_RANGE_MAX && assayCode <= tf::AssayCodeRange::CH_RANGE_MAX)
	{
		return tf::DeviceType::DEVICE_TYPE_ISE1005;
	}
	// 免疫
	else if (assayCode >= tf::AssayCodeRange::IM_RANGE_MIN && assayCode <= tf::AssayCodeRange::IM_RANGE_MAX)
	{
		return tf::DeviceType::DEVICE_TYPE_I6000;
	}

	return tf::DeviceType::DEVICE_TYPE_INVALID;
}

///
/// @brief  点击重置按钮
///
/// @par History:
/// @li 1226/zhangjing，2023年12月1日，新建函数
///
void QQcDownloadRegDlg::OnClickReset(void)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui->lineEdit_QcName->setText("");
    ui->lineEdit_QcLot->setText("");
    ui->QcLevelCombo->setCurrentIndex(0);
    ui->QcExpiryTimeCombo->setCurrentIndex(1);
    m_QcDownloadGroup->setRowCount(0);
    m_selCloudQcDoc = nullptr;
}

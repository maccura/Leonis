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
/// @file     QSupplyList.h
/// @brief    耗材列表
///
/// @author   5774/WuHongTao
/// @date     2021年10月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QSupplyList.h"
#include "ui_QSupplyList.h"
#include <QStandardItemModel>
#include <QItemSelection>
#include <QFileDialog>
#include "src/leonis/thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/leonis/thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/DcsControlProxy.h"

#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/datetimefmttool.h"
#include "shared/FileExporter.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/defs.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ConfigSerialize.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigDefine.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "utility/maintaindatamng.h"
#include "Serialize.h"
#include "printcom.hpp"

#include "im/imaddreactcupdlg.h"
#include "ui_imaddreactcupdlg.h"
#include "rgntnoticedatamng.h"
#include "SortHeaderView.h"
#include "ch/QSampleDetectDlg.h"
#include "ch/QRecordISESupply.h"
#include "ch/QIsePerfusionDlg.h"

#define SUPPLYDEVICENAME(supplyInfo) (supplyInfo.deviceSN + "-" + to_string(supplyInfo.moduleIndex))
#define MAXROW          6
#define MAXROW_ISE      2
#define MAXROW_I6000    16

#define NAMEROW   1

#define UL01_TO_ML		10000.0		//	0.1ul转换为ml的倍率	

///
/// @brief
///     列索引号枚举
/// 
enum SupplyColumnIndex {
    ECI_Module = 0, // 模块
    ECI_Name,       // 名称
    ECI_Statu,      // 使用状态
    ECI_Residual,   // 余量
    ECI_Lot,        // 耗材批号
    ECI_Serial,     // 瓶号
    ECI_OpenEffectDays, // 开瓶有效期（条）
    ECI_BoardTime,      // 上机时间
    ECI_Expire          // 失效日期
};

Q_DECLARE_METATYPE(im::tf::SuppliesInfoTable)

QSupplyList::QSupplyList(QWidget *parent)
    : QWidget(parent),
    m_supplyMode(nullptr),
    m_regIseDialog(nullptr),
    m_needUpdate(false),
    m_bInit(false),
	m_sampleDetectDlg(Q_NULLPTR),
    m_isePerfusionDlg(new QIsePerfusionDlg(this)),
    ui(new Ui::SupplyList)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->setupUi(this);
    m_sortedHistory = { -1, Qt::AscendingOrder };
    Init();
}

QSupplyList::~QSupplyList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     更新按钮显示（根据当前选择设备）
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月29日，新建函数
///
void QSupplyList::UpdateBtnVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据当前选中设备，更新按钮显示
    // 分类设备类型
    std::vector<std::string> deviceSnCh;
    std::vector<std::string> deviceSnIse;
    std::vector<std::string> deviceSnIm;
    for (auto& dev : m_deivceVec)
    {
        switch (dev->deviceType)
        {
        case tf::DeviceType::DEVICE_TYPE_ISE1005:
            deviceSnIse.push_back(dev->deviceSN);
            break;
        case tf::DeviceType::DEVICE_TYPE_I6000:
            deviceSnIm.push_back(dev->deviceSN);
            break;
        case tf::DeviceType::DEVICE_TYPE_C1000:
        case tf::DeviceType::DEVICE_TYPE_C200:
            deviceSnCh.push_back(dev->deviceSN);;
            break;
        default:
            break;
        }
    }

    // 构造是否显示对应按钮变量
    bool bShowChBtn = !deviceSnCh.empty();
    bool bShowIseBtn = !deviceSnIse.empty();
    bool bShowImBtn = !deviceSnIm.empty();

    // 更新按钮显示
    ui->AddReactionBtn->setVisible(bShowImBtn);
    ui->supplyLoad_btn->setVisible(bShowIseBtn);
    ui->perfusion_btn->setVisible(bShowIseBtn);
	//ui->reagentAllowance->setVisible(bShowChBtn || bShowIseBtn);
	// 联机版才处理余量探测 并且现在ch不包含样本针酸碱清洗液，不需要进行余量探测
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		ui->reagentAllowance->setVisible(bShowIseBtn);
	}
}

///
/// @brief 通过模块描述获取的设备名称
///
/// @param[in]  device  模块描述
///
/// @return 设备名称
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月15日，新建函数
///
QString QSupplyList::GetDeviceNameByModuleDsc(const QString& deviceModuleDescrip)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QStringList devKeyinfos = deviceModuleDescrip.split("-");
    if (devKeyinfos.empty())
    {
        return QString();
    }

    // 修改为只要模块号
    std::string deviceName = CommonInformationManager::GetDeviceName(devKeyinfos[0].toStdString());
    QString moduleName = QString::fromStdString(deviceName);

    return devKeyinfos.size() >= 2 ? moduleName + "-" + devKeyinfos[1] : moduleName;
}

///
/// @brief
///     初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月16日，新建函数
///
void QSupplyList::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化提醒管理器
    RgntNoticeDataMng::GetInstance();

    // 优先使用
    connect(ui->supplyPriority, SIGNAL(clicked()), this, SLOT(OnPrioritySupply()));
    // 耗材加载
    connect(ui->supplyLoad_btn, SIGNAL(clicked()), this, SLOT(OnApplyIseSupply()));
    // 页面打印
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));

    // 添加反应杯按钮被点击
    connect(ui->AddReactionBtn, SIGNAL(clicked()), this, SLOT(OnAddReactBtnClicked()));

	// 液路灌注
	connect(ui->perfusion_btn, SIGNAL(clicked()), this, SLOT(OnISEPerfusion()));

    // 列排序
    //connect(ui->tableView->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnSortByColumn(int)));

    // 导出
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 监听项目编码管理器信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnUpdateAssayTable);

    // 监听生化仓外耗材更新信息
    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, OnUpdateSupplyInfo);

    // 监听耗材信息更新
    REGISTER_HANDLER(MSG_ID_IM_SUPPLY_INFO_UPDATE, this, UpdateImSplChanged);

	// ISE耗材更新消息
	REGISTER_HANDLER(MSG_ID_ISE_SUPPLY_INFO_UPDATE, this, OnUpdateSupplyForISE);

    // 监听耗材管理设置更新
    REGISTER_HANDLER(MSG_ID_CONSUMABLES_MANAGE_UPDATE, this, OnSplMngCfgChanged);

    // 当前日期改变
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, OnSplInfoUpdate);

    // 耗材报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, OnSplInfoUpdate);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 监听耗材管理设置更新(ISE)
	REGISTER_HANDLER(MSG_ID_DEVICE_OTHER_INFO_UPDATE, this, OnDeviceOtherInfoUpdate);

    ui->supplyPriority->setDisabled(true);

    // 初始化表格
    m_supplyMode = new QStandardItemModel();
    ui->tableView->setModel(m_supplyMode);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// 设置名称代理
	ui->tableView->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
    // 设置余量代理
    ui->tableView->setItemDelegateForColumn(3, new CReadOnlyDelegate(this));
    // 使用状态代理
    ui->tableView->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
    // 耗材失效日期
    ui->tableView->setItemDelegateForColumn(8, new CReadOnlyDelegate(this));
    // 耗材开瓶有效期
    ui->tableView->setItemDelegateForColumn(6, new CReadOnlyDelegate(this));
    // 耗材上机时间
    ui->tableView->setItemDelegateForColumn(7, new CReadOnlyDelegate(this));
    ui->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setAutoScroll(false);

    // 需要在setModel后可
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this, SLOT(OnTabelSelectChanged(QItemSelection, QItemSelection)));

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal);
    m_pSortHeader->setStretchLastSection(true);
    ui->tableView->setHorizontalHeader(m_pSortHeader);

    // 默认排序初始化
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);

    // 点击表头触发排序
    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order)
    {
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            m_supplyMode->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
            ui->tableView->sortByColumn(logicIndex, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
            m_sortedHistory.first = -1;
            m_sortedHistory.second = Qt::DescendingOrder;
        }
        else
        {
            m_supplyMode->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->tableView->sortByColumn(logicIndex, qOrderFlag);
            m_sortedHistory.first = logicIndex;
            m_sortedHistory.second = qOrderFlag;
        }
    });

    // 重新设置页面
    QStringList supplyHeaderListString;
    supplyHeaderListString << tr("模块") << tr("名称") << tr("使用状态") << tr("余量")
        << tr("耗材批号") << tr("瓶号") + "/" + tr("序列号") << tr("开瓶有效期(天)") << tr("上机时间") << tr("失效日期");
    m_supplyMode->setHorizontalHeaderLabels(supplyHeaderListString);
    
	ui->tableView->setColumnWidth(0, 150);
	ui->tableView->setColumnWidth(1, 360);
	ui->tableView->setColumnWidth(2, 170);
	ui->tableView->setColumnWidth(3, 170);
	ui->tableView->setColumnWidth(7, 230);
    ui->tableView->setColumnWidth(8, 230);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setTextElideMode(Qt::ElideLeft);

	// 点击余量探测按钮
	connect(ui->reagentAllowance, &QPushButton::clicked, this, &QSupplyList::OnRemainDetect);

	// 注册自定义类型
	qRegisterMetaType<std::vector<std::pair<std::string, int>>>("std::vector<std::pair<std::string, int>>");

	// 开始样本针碱性清洗液余量探测
	m_sampleDetectDlg = new QSampleDetectDlg(this);
	connect(m_sampleDetectDlg, &QSampleDetectDlg::signalRemainDetect, this, &QSupplyList::StartRemainDetect);

	// 单机版隐藏余量探测
	if (!DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		ui->reagentAllowance->hide();
	}
}

///
/// @brief刷新耗材列表
///     
///
/// @param[in]  spDevModule  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月21日，新建函数
///
void QSupplyList::RefreshSupplyList(Devices& spDevModule)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_deivceVec = spDevModule;

    // 根据当前选择设备，更新按钮显示
    UpdateBtnVisible();

    // 更新耗材表
    UpdateConsumeTable();
}

void QSupplyList::updatePerfusionBtnEnable()
{
	// 查找ise设备
	int iseCount = 0;
	bool hasStandby = false;
	for (const auto &dev : m_deivceVec)
	{
		if (dev->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
		{
			iseCount++;
			hasStandby = (dev->status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY);
		}
	}

	// 无ise设备
	if (iseCount == 0)
	{
		ui->perfusion_btn->setEnabled(false);
	}
	// 有ise设备且有待机态
	else
	{
		ui->perfusion_btn->setEnabled(hasStandby);
	}
}

void QSupplyList::UpdateTableListCh(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (deviceSn.size() == 0)
    {
        return;
    }

	// 获取所有CH设备的耗材信息
	std::vector<::ch::tf::SuppliesInfo> vecSplInfoes;
	// （设备序列号，反应杯酸碱清洗液是否启用）
	map<string, ChDeviceOtherInfo> mapChOtherInfos;
    for (const auto& strDevSn : deviceSn)
    {
		// 获取仓外耗材
        auto mapSplInfo = CommonInformationManager::GetInstance()->GetChCabinetSupplies(strDevSn);
		for (auto it = mapSplInfo.begin(); it != mapSplInfo.end(); it++)
		{
			vecSplInfoes.push_back(it->second);
		}

		// 查询反应杯酸碱清洗液是否启用
		auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
		if (spDevice == nullptr)
		{
			continue;
		}

		// 查找配置信息
		ChDeviceOtherInfo cdoi;
		if (!DecodeJson(cdoi, spDevice->otherInfo))
		{
			ULOG(LOG_WARN, "DecodeJson ChDeviceOtherInfo Failed");
			continue;
		}

		mapChOtherInfos[strDevSn] = cdoi;
    }

    // 排序耗材
    std::sort(vecSplInfoes.begin(), vecSplInfoes.end(), CompareSupplyPriority<decltype (vecSplInfoes[0])>);

    // 将查询结果更新到表格中
    for (const auto& supplyInfo : vecSplInfoes)
    {
		// 非反应杯酸碱清洗液不处理
		if (supplyInfo.pos < ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1 ||
			supplyInfo.pos > ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2)
		{
			continue;
		}

		// 设备序列号
		string sn = supplyInfo.deviceSN;
		if (mapChOtherInfos.count(sn))
		{
			// 如果应用界面-耗材管理 未启用反应杯酸性清洗液，不显示
			if ((supplyInfo.pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1
				|| supplyInfo.pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY2)
				&& !mapChOtherInfos[sn].acidity)
			{
				continue;
			}
			// 如果应用界面-耗材管理 未启用反应杯碱性清洗液，不显示
			else if ((supplyInfo.pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY1
				|| supplyInfo.pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2)
				&& !mapChOtherInfos[sn].alkalinity)
			{
				continue;
			}
		}

        ShowSupplyItemCh(supplyInfo, tableContent);
    }
}

void QSupplyList::UpdateTableListIse(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (deviceSn.size() == 0)
    {
        return;
    }

    // ISE设备耗材
	std::vector<::ise::tf::SuppliesInfo> vecSplInfoes;
	// （设备序列号，酸碱清洗液是否启用）
	map<string, IseDeviceOtherInfo> mapIseOtherInfos;
    for (const auto& strDevSn : deviceSn)
    {
		// 获取所有ISE设备的耗材
        auto mapSplInfo = CommonInformationManager::GetInstance()->GetIseSupplies(strDevSn);
		for (auto it = mapSplInfo.begin(); it != mapSplInfo.end(); it++)
		{
			vecSplInfoes.push_back(it->second);
		}

		// 查询酸碱清洗液是否启用
		auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
		if (spDevice == nullptr)
		{
			continue;
		}

		// 查找配置信息
		IseDeviceOtherInfo idoi;
		if (!DecodeJson(idoi, spDevice->otherInfo))
		{
			ULOG(LOG_WARN, "DecodeJson IseDeviceOtherInfo Failed");
			continue;
		}

		mapIseOtherInfos[strDevSn] = idoi;
    }

    // 排序耗材
    std::sort(vecSplInfoes.begin(), vecSplInfoes.end(), CompareSupplyPriority<decltype (vecSplInfoes[0])>);

	// 遍历耗材进行显示
    for (const auto& supplyInfo : vecSplInfoes)
    {
		// 设备序列号
		string sn = supplyInfo.deviceSN;
		if (mapIseOtherInfos.count(sn))
		{
			// 如果应用界面-耗材管理 未启用样本针酸清洗液，不显示
			if (supplyInfo.pos == ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ACIDITY
				&& supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY
				&& !mapIseOtherInfos[sn].acidity)
			{
				continue;
			}
			// 如果应用界面-耗材管理 未启用样本针碱清洗液，不显示
			else if (supplyInfo.pos == ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ALKALINITY
				&& supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY
				&& !mapIseOtherInfos[sn].alkalinity)
			{
				continue;
			}
		}

        ShowSupplyItemISE(supplyInfo, tableContent);
    }
}

void QSupplyList::UpdateTableListIm(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 判断是否包含反应杯
    auto AddDefaultReactionCup = [](const std::string& devSn, std::vector<im::tf::SuppliesInfoTable>& vSplInfo)
    {
        bool bContainReactionCup = false;
        for (const auto& stuSplInfo : vSplInfo)
        {
            if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
            {
                bContainReactionCup = true;
                break;
            }
        }

        // 如果未包含反应杯，则添加一个默认的
        if (!bContainReactionCup)
        {
            im::tf::SuppliesInfoTable splCup;
            splCup.__set_deviceSN(devSn);
            splCup.__set_supType(im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS);
            vSplInfo.push_back(splCup);
        }
    };

    for (const std::string& dev : deviceSn)
    {
        // 构造查询条件和查询结果
        ::im::tf::SuppliesInfoTableQueryResp qryResp;
        ::im::tf::SuppliesInfoTableQueryCond qryCond;
        qryCond.__set_deviceSN(dev);
        qryCond.__set_isLoad(true);

        // 查询所有耗材信息
        bool bRet = ::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "Query suppliesInfo failed.");
            continue;
        }

        // 添加默认反应杯信息
        AddDefaultReactionCup(dev, qryResp.lstSuppliesInfos);

        //遍历耗材信息， 更新界面
        ULOG(LOG_INFO, "Supplies item size: %d, (dev:%s).", qryResp.lstSuppliesInfos.size(), dev.c_str());
        std::vector<im::tf::SuppliesInfoTable> sortedSupplies;
        SortSuppliesIm(qryResp.lstSuppliesInfos, sortedSupplies);
        for (int i = 0; i < sortedSupplies.size(); ++i)
        {
            ShowSupplyItemIm(sortedSupplies[i], tableContent);
        }
    }
}

///
/// @brief
///     更新耗材信息
///
/// @param[in]  deviceSnList  设备名称列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月16日，新建函数
///
void QSupplyList::UpdateConsumeTable()
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 禁能排序
    ui->tableView->setSortingEnabled(false);

    // 记录刷新前选择行
    QModelIndex preRowIdx = ui->tableView->currentIndex();

    // 初始化设备试剂信息列表
    m_supplyMode->removeRows(0, m_supplyMode->rowCount());
    m_supplyItemCache.clear();

    // 分类设备类型
    std::vector<std::string> deviceSnCh;
    std::vector<std::string> deviceSnIse;
    std::vector<std::string> deviceSnIm;
    for (auto& dev : m_deivceVec)
    {
        switch (dev->deviceType)
        {
        case tf::DeviceType::DEVICE_TYPE_ISE1005:
            deviceSnIse.push_back(dev->deviceSN);
            break;
        case tf::DeviceType::DEVICE_TYPE_I6000:
            deviceSnIm.push_back(dev->deviceSN);
            break;
        default:
            deviceSnCh.push_back(dev->deviceSN);
            break;
        }
    }

    // 设备名排序
    std::sort(deviceSnCh.begin(), deviceSnCh.end());
    std::sort(deviceSnIse.begin(), deviceSnIse.end());
    std::sort(deviceSnIm.begin(), deviceSnIm.end());

    // 更新生化、ISE、免疫耗材界面
    std::vector<QList<QStandardItem*>> tableContent;
    UpdateTableListCh(deviceSnCh, tableContent);
    UpdateTableListIse(deviceSnIse, tableContent);
    UpdateTableListIm(deviceSnIm, tableContent);
    for (int i=0; i<tableContent.size(); ++i)
    {
        m_supplyMode->appendRow(tableContent[i]);
    }

    // 更新排序值
    UpdateSortValue();

    // 更新ISE液路灌注按钮的使能状态
	updatePerfusionBtnEnable();

    // 居中
    SetTblTextAlign(m_supplyMode, Qt::AlignCenter);

    // 使能排序
    ui->tableView->setSortingEnabled(true);

    // 恢复排序
    if (m_sortedHistory.first >= 0)
    {
        m_supplyMode->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
        ui->tableView->sortByColumn(m_sortedHistory.first, m_sortedHistory.second);
        m_pSortHeader->setSortIndicator(m_sortedHistory.first, m_sortedHistory.second);
    }
    else
    {
        m_supplyMode->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
        ui->tableView->sortByColumn(ECI_Module, Qt::AscendingOrder);
        m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    }

    // 选中之前的选中行
    if (preRowIdx.isValid())
    {
        ui->tableView->selectRow(preRowIdx.row());
    }

    // 更新添加反应杯按钮使能状态
    if (!CommonInformationManager::GetInstance()->GetImisSingle())
    {
        ui->AddReactionBtn->setEnabled(IsSelectReaction());
    }
}

///
/// @brief
///     设置显示空行
///
/// @param[out]  currentPos  当前的行数
/// @param[in]  lastPos      上次的位置
/// @param[in]  endPostion   本次的位置
/// @param[in]  devicesn     设备编号
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月24日，新建函数
///
void QSupplyList::ShowBlankLine(int& currentPos, int lastPos, int endPostion, const QString& devicesn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString moduleName = GetDeviceNameByModuleDsc(devicesn);
    if (moduleName.isEmpty())
    {
        return;
    }

    for (int pos = lastPos + 1; pos < endPostion + 1; pos++)
    {
        QStandardItem* item = new QStandardItem(moduleName);
        item->setData(moduleName + QString::number(pos), Qt::UserRole);
        m_supplyMode->setItem(currentPos, 0, item);
        SetTblTextAlign(m_supplyMode, Qt::AlignCenter, currentPos);
        currentPos++;
    }
}

// 开瓶有效期（天）
QStandardItem* QSupplyList::OpenBottleExpireItem(const std::string& openBottleExpiryTime)
{
    QStandardItem* itemOpenDay = new QStandardItem("0");
	int openDay = GetOpenRestdays(openBottleExpiryTime);
	// 若过期则显示红色
    if (openDay <= 0) 
    {
        SetTblItemState(itemOpenDay, STATE_WARNING);
    }
	else
	{
		itemOpenDay->setText(QString::number(openDay));
	}

    return itemOpenDay;
}

// 耗材有效期
QStandardItem* QSupplyList::ExpireItem(const std::string& expireTime)
{
    QString strDataTime = QString::fromStdString(ConverStdStringToDateString(expireTime));
    QStandardItem* itemExpired = new QStandardItem(strDataTime);

	// 若过期则显示红色
    if (ChSuppliesExpire(expireTime))
    {
        SetTblItemState(itemExpired, STATE_WARNING);
    }

    itemExpired->setToolTip(strDataTime);

    return itemExpired;
}

///
/// @brief
///     显示耗材信息
///
/// @param[in]  pos  位置信息
/// @param[in]  supplyInfo  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月16日，新建函数
///
void QSupplyList::ShowSupplyItemCh(const ch::tf::SuppliesInfo& supplyInfo, std::vector<QList<QStandardItem*>>& tableContent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int column = 0;
    QList<QStandardItem*> rowContent;
    std::shared_ptr<SuppyDataItem> sItem = std::make_shared<SuppyDataItem>(supplyInfo);

    // 模块
    QString strModelName = QString::fromStdString(CommonInformationManager::GetDeviceName(supplyInfo.deviceSN));
    QString keyName = strModelName + QString::number(supplyInfo.pos);
    QStandardItem* moduleNameItem = new QStandardItem(strModelName);
    moduleNameItem->setData(keyName, Qt::UserRole);
    m_supplyItemCache[keyName] = sItem; // 缓存当前数据
    rowContent.push_back(moduleNameItem);

	if (supplyInfo.pos < ::ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ACIDITY1
		|| supplyInfo.pos > ::ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ALKALINITY2)
	{
		ULOG(LOG_INFO, "supplyInfo.pos error, not to show.");
		return;
	}

	// 耗材名称
	QString strName = supplyInfo.suppliesCode <= 0 ? tr("扫描失败") :
		(ThriftEnumTrans::GetSupplyName((ch::tf::SuppliesType::type)supplyInfo.suppliesCode, false)
		+ QString("-%1").arg((supplyInfo.pos <= 2) ? 1 : 2));

    QStandardItem* nameItem = new QStandardItem(strName);
    nameItem->setData(true, Qt::UserRole + 10);
    nameItem->setToolTip(strName);
    rowContent.push_back(nameItem);

	//  若是扫描失败，只显示扫描失败后面的信息不显示
	if (supplyInfo.suppliesCode <= 0)
	{
		SetTblItemState(nameItem, STATE_WARNING);
		for (int i = 0; i < m_supplyMode->columnCount() - 2; i++)
		{
			rowContent.push_back(new QStandardItem(""));
		}
		tableContent.push_back(rowContent);
		return;
	}

	if (supplyInfo.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY
		&& supplyInfo.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY)
	{
		ULOG(LOG_INFO, "supplyInfo.type error, not to show.");
		return;
	}

    // 耗材状态, --备用特殊处理
    auto itemStatus = GetSupplyShowStatus(supplyInfo.placeStatus, supplyInfo.usageStatus, 
		supplyInfo.suppliesCode, supplyInfo.backupTotal, supplyInfo.backupNum);
    rowContent.push_back(itemStatus);

    // 耗材余量
    int residual = GetSupplyResidual(supplyInfo);
	QStandardItem * itemRemainCount = nullptr;
	// 探测失败或者余量小于0
	if (supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL || residual <= 0)
	{
		itemRemainCount = new QStandardItem("0ml");
		SetTblItemState(itemRemainCount, STATE_WARNING);
	}
	else
	{
		// 耗材余量 在列表显示时转换为毫升,向上取整
		itemRemainCount = new QStandardItem(QString("%1ml").arg(std::ceil(residual / UL01_TO_ML)));

		// 判断耗材是否是试剂针或者样本针的清洗液
		auto IsSupplyType = [](const ch::tf::SuppliesType::type& type)-> bool
		{
			return type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY
				|| type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY;
		};
		// 样本针的耗材属性信息
		int bottleAlarmThreshold = -1; // 耗材的瓶提醒
		auto supplyAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(supplyInfo.type, IsSupplyType(supplyInfo.type));
		if (supplyAttribute)
		{
			bottleAlarmThreshold = supplyAttribute.value()->bottleAlarmThreshold;
		}
		// 余量为0
		if (residual <= 0)
		{
			SetTblItemState(itemRemainCount, STATE_WARNING);
		}
		// 余量不足
		else if (bottleAlarmThreshold > 0 && residual <= bottleAlarmThreshold)
		{
			SetTblItemState(itemRemainCount, STATE_NOTICE);
		}
	}
    rowContent.push_back(itemRemainCount);

	// 试剂批号 瓶号(需根据lotSnMode进行判断，因为会自动生成瓶号，当未输入瓶号时需要隐藏)
	QString strLot, strSn;
	if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
	{
		strLot = supplyInfo.__isset.lot ? QString::fromStdString(supplyInfo.lot) : QStringLiteral("");
		strSn = supplyInfo.__isset.sn ? QString::fromStdString(supplyInfo.sn) : QStringLiteral("");
	}
	else if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT)
	{
		strLot = supplyInfo.__isset.lot ? QString::fromStdString(supplyInfo.lot) : QStringLiteral("");
	}
	else if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_SN)
	{
		strSn = supplyInfo.__isset.sn ? QString::fromStdString(supplyInfo.sn) : QStringLiteral("");
	}
    // 耗材批号
    rowContent.push_back(new QStandardItem(strLot));

    // 耗材瓶号
    rowContent.push_back(new QStandardItem(strSn));

    // 开瓶有效期
    QStandardItem* itemExpOpen = OpenBottleExpireItem(supplyInfo.openBottleExpiryTime);
    rowContent.push_back(itemExpOpen);

    // 上机时间
    QString strRegTime = QString::fromStdString(supplyInfo.registerTime);
    QStandardItem* pItemReg = new QStandardItem(strRegTime);
    pItemReg->setToolTip(strRegTime);
    rowContent.push_back(pItemReg);

    // 耗材有效期
    QStandardItem* itemExp = ExpireItem(supplyInfo.expiryTime);
    rowContent.push_back(itemExp);

    tableContent.push_back(rowContent);
}

///
/// @brief 显示ISE耗材信息
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  total   ise模块总数（当只有一个ise模块的时候，显示名称是ISE，当有两个的时候显示ISE-A,ISE-B）
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月7日，新建函数
///
void QSupplyList::ShowSupplyItemISE(const ise::tf::SuppliesInfo& supplyInfo, std::vector<QList<QStandardItem*>>& tableContent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int column = 0;
    std::shared_ptr<SuppyDataItem> sItem = std::make_shared<SuppyDataItem>(supplyInfo);
    QList<QStandardItem*> rowContent;

    // <模块
    QString moduleName = GetIseName(supplyInfo);
	// ise耗材特殊处理，使用设备名+模块id+位置作为key
    QString keyName =  QString("%1-%2-%3").arg(moduleName).arg(supplyInfo.moduleIndex).arg(supplyInfo.pos);
    m_supplyItemCache[keyName] = sItem;  // 缓存当前数据
    QStandardItem* moduleNameItem = new QStandardItem(moduleName);
    moduleNameItem->setData(keyName, Qt::UserRole);
    rowContent.push_back(moduleNameItem);

    // 耗材名称
    QString strName = ThriftEnumTrans::GetSupplyName((ise::tf::SuppliesType::type)supplyInfo.suppliesCode);
    QStandardItem* pItemName = new QStandardItem(strName);
    pItemName->setToolTip(strName);
    rowContent.push_back(pItemName);

	// 耗材余量 在列表显示时转换为毫升 向上取整
	QStandardItem * itemRemainCount = new QStandardItem(supplyInfo.residual <= 0 ? "0ml" 
		: QString("%1ml").arg(std::ceil(supplyInfo.residual / UL01_TO_ML)));

	// 耗材的瓶提醒值
	int bottleAlarmThreshold = -1;
	// 样本针的耗材属性信息
	auto supplyAttribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(supplyInfo.suppliesCode);
	if (supplyAttribute)
	{
		bottleAlarmThreshold = supplyAttribute.value()->bottleAlarmThreshold;
	}
	// 耗材状态
	bool mlResidualZero = false;
	// 余量小于0或者液位探测失败
	if (supplyInfo.residual <= 0 || supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		mlResidualZero = true;
		SetTblItemState(itemRemainCount, STATE_WARNING);
	}
	else if (bottleAlarmThreshold > 0 && supplyInfo.residual <= bottleAlarmThreshold)
	{
		SetTblItemState(itemRemainCount, STATE_NOTICE);
	}
	// 耗材状态
	QStandardItem* itemSuppStat = GetSupplyShowStatus(supplyInfo.placeStatus, supplyInfo.usageStatus, 
		supplyInfo.suppliesCode, -1, -1, mlResidualZero);
	rowContent.push_back(itemSuppStat);

	// 耗材余量
    rowContent.push_back(itemRemainCount);

	// 试剂批号 瓶号(需根据lotSnMode进行判断，因为会自动生成瓶号，当未输入瓶号时需要隐藏)
	QString strLot, strSn;
	if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
	{
		strLot = supplyInfo.__isset.lot ? QString::fromStdString(supplyInfo.lot) : QStringLiteral("");
		strSn = supplyInfo.__isset.sn ? QString::fromStdString(supplyInfo.sn) : QStringLiteral("");
	}
	else if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT)
	{
		strLot = supplyInfo.__isset.lot ? QString::fromStdString(supplyInfo.lot) : QStringLiteral("");
	}
	else if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_SN)
	{
		strSn = supplyInfo.__isset.sn ? QString::fromStdString(supplyInfo.sn) : QStringLiteral("");
	}
	// 耗材批号
    rowContent.push_back(new QStandardItem(strLot));

	// 耗材瓶号
    rowContent.push_back(new QStandardItem(strSn));

    // 开瓶有效期
	if (supplyInfo.openBottleExpiryTime.empty())
	{
		rowContent.push_back(new QStandardItem(""));
	}
	else
	{
		rowContent.push_back(OpenBottleExpireItem(supplyInfo.openBottleExpiryTime));
	}

    // 上机时间
	if (supplyInfo.registerTime.empty())
	{
		rowContent.push_back(new QStandardItem(""));
	}
	else
	{
		rowContent.push_back(new QStandardItem(QString::fromStdString(supplyInfo.registerTime)));
	}

    // 耗材有效期
	if (supplyInfo.expiryTime.empty())
	{
		rowContent.push_back(new QStandardItem(""));
	}
	else
	{
		QStandardItem* itemExp = ExpireItem(supplyInfo.expiryTime);
		rowContent.push_back(itemExp);
	}

    tableContent.push_back(rowContent);
}

bool QSupplyList::IsExceptSupplyTypeIm(im::tf::SuppliesType::type supplyType)
{
    return (supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID ||
        supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID ||
        supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_WASH_WATER ||
        supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_PURE_WATER ||
        supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_CONFECT_LIQUID);
}

std::vector<int> QSupplyList::GetSelectedRows()
{
    QModelIndexList mList = ui->tableView->selectionModel()->selectedRows();
    std::vector<int> rowIdx;
    for (const auto& mItem : mList)
    {
        rowIdx.push_back(mItem.row());
    }

    return rowIdx;
}

///
/// @bref
///		填充打印导出数据
///
/// @param[out] vecSupply 输出的填充好的数据
///
/// @par History:
/// @li 8276/huchunli, 2023年8月29日，新建函数
///
void QSupplyList::FillExportContainer(SupplyItemVector &vecSupply)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 获取一个单元格内容
    auto funcValue = [this](int ir, int ic) -> std::string {
        QStandardItem* qStand = m_supplyMode->item(ir, ic);
        if (qStand == Q_NULLPTR)
        {
            return "";
        }
        return qStand->text().toStdString();
    };

    vecSupply.clear();
    int rowNumber = m_supplyMode->rowCount();
    for (int i=0; i< rowNumber; ++i)
    {
        if (funcValue(i, ECI_Module).empty())
        {
            continue;
        }

        SupplyItem info;
        info.strModule = funcValue(i, ECI_Module);            // 模块
        info.strType = funcValue(i, ECI_Name);                // 类型
        info.strLotNumber = funcValue(i, ECI_Lot);            // 批号
        info.strSerialNumber = funcValue(i, ECI_Serial);      // 序列号
        info.strRemainQuantity = funcValue(i, ECI_Residual);  // 剩余量    
        info.strExpirationDate = funcValue(i, ECI_Expire);    // 失效日期
        info.strLoadDate = funcValue(i, ECI_BoardTime);       // 加载日期
        info.strUsageStatus = funcValue(i, ECI_Statu);        // 使用状态
        info.strOpenEffectDays = funcValue(i, ECI_OpenEffectDays);        // 开瓶有效期
        vecSupply.push_back(info);
    }
}

///
/// @bref
///		耗材指定顺序的优先级包装
///
struct SupplySortItemIm
{
    ///
    /// @bref
    ///		包装进行排序的3段组合
    ///
    struct ThreeSegItemPri
    {
        ///
        /// @bref
        ///		默认构造
        ///
        ThreeSegItemPri()
        {
            m_d1 = 0;
            m_d2 = 0;
            m_d3 = 0;
        }

        ///
        /// @bref
        ///		赋值
        ///
        void SetPri(int d1, int d2, int d3)
        {
            m_d1 = d1;
            m_d2 = d2;
            m_d3 = d3;
        }

        ///
        /// @bref
        ///		用于排序的小于重载
        ///
        bool operator<(const ThreeSegItemPri& src) const
        {
            if ((m_d1 < src.m_d1) ||
                (m_d1 == src.m_d1 && m_d2 < src.m_d2) ||
                (m_d1 == src.m_d1 && m_d2 == src.m_d2 && m_d3 < src.m_d3))
            {
                return true;
            }
            return false;
        }

    private:
        int m_d1;
        int m_d2;
        int m_d3;
    };

    ///
    /// @bref
    ///		排序优先级构造
    ///
    SupplySortItemIm(im::tf::SuppliesType::type st, short gid)
    {
        // 指定的显示顺序
        if (st == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
        {
            m_priority.SetPri(0, 0, 0); // 排在前面
        }
        else if (st == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
            st == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B)
        {
            m_priority.SetPri(1, gid, st); // 放置A的后面
        }
        else if (st == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID)
        {
            m_priority.SetPri(2, gid, 0);
        }
        else
        {
            m_priority.SetPri(3, 0, 0);
        }
    }

    ///
    /// @bref
    ///		用于map排序的小于重载
    ///
    bool operator<(const SupplySortItemIm& supplyItem) const
    {
        return m_priority < supplyItem.m_priority;
    }

private:
    ThreeSegItemPri m_priority; // 优先级,小的放前面
};

///
/// @bref
///		对免疫的耗材按照指定的顺序进行排序
///
/// @param[in/out] srcSupplies 输入的耗材列表
///
/// @par History:
/// @li 8276/huchunli, 2023年6月19日，新建函数
///
void QSupplyList::SortSuppliesIm(const std::vector<im::tf::SuppliesInfoTable>& srcSupplies, std::vector<im::tf::SuppliesInfoTable>& outSorted)
{
    // 记录原始顺序,使用multimap忠实于数据库
    std::multimap<SupplySortItemIm, int> origOrder;
    for (int i = 0; i < srcSupplies.size(); ++i)
    {
        origOrder.insert(std::pair<SupplySortItemIm, int>(SupplySortItemIm(srcSupplies[i].supType, srcSupplies[i].groupNum), i));
    }
    // 重新按照map顺序填数据
    outSorted.clear();
    for (std::multimap<SupplySortItemIm, int>::iterator it = origOrder.begin(); it != origOrder.end(); ++it)
    {
        outSorted.push_back(srcSupplies[it->second]);
    }
}

bool QSupplyList::ShowSupplyItemIm(const im::tf::SuppliesInfoTable& stuSplInfo, std::vector<QList<QStandardItem*>>& tableContent)
{
    //ULOG(LOG_INFO, "%s()", __FUNCTION__); // too frequency

    // 试剂耗材界面不显示
    if (IsExceptSupplyTypeIm(stuSplInfo.supType))
    {
        ULOG(LOG_WARN, "Type%d not to show.", stuSplInfo.supType);
        return false;
    }

    int columIdx = 0;
    std::shared_ptr<SuppyDataItem> sItem = std::make_shared<SuppyDataItem>(stuSplInfo);
    QList<QStandardItem*> rowContent;

    // 模块名称
    std::shared_ptr<const tf::DeviceInfo> pDeviceInfo = FindDeviceInfoBySn(m_deivceVec, stuSplInfo.deviceSN);
    std::string moduleName = pDeviceInfo == nullptr ? stuSplInfo.deviceSN : pDeviceInfo->name;
    QString keyName = QString::fromStdString(moduleName) + "-" + QString::number(stuSplInfo.iD);
    m_supplyItemCache[keyName] = sItem; // 缓存当前行对应的耗材对象

    QStandardItem* moduleNameItem = new QStandardItem(QString::fromStdString(moduleName));
    moduleNameItem->setData(keyName, Qt::UserRole);
    rowContent.push_back(moduleNameItem);

    // 耗材名称(待完善)
    QString strSplName = ConvertTfEnumToQString(stuSplInfo.supType);
    if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID ||
        stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
        stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B)
    {
        strSplName += QString::number(stuSplInfo.groupNum);
    }

    QStandardItem* supplyNameItem = new QStandardItem(strSplName);
    supplyNameItem->setData(true, Qt::UserRole + 10);
    rowContent.push_back(supplyNameItem);

    // 获取耗材停用信息
    ImSuppliesEnableInfo stuImSplEnableInfo = RgntNoticeDataMng::GetInstance()->GetSplEnableInfo(stuSplInfo.deviceSN);

    // 判断是否是停用
    QString strStatus("");
    if ((stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
        stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B) &&
        stuSplInfo.groupNum == 1 &&
        !stuImSplEnableInfo.bBaseLiquidGrp1)
    {
        // 底物组1停用
        strStatus = ConvertTfEnumToQString(im::tf::SuppliesUsingStatus::SUPPLIES_USING_STATUS_STOP_USE);
    }
    else if ((stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
        stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B) &&
        stuSplInfo.groupNum == 2 &&
        !stuImSplEnableInfo.bBaseLiquidGrp2)
    {
        // 底物组2停用
        strStatus = ConvertTfEnumToQString(im::tf::SuppliesUsingStatus::SUPPLIES_USING_STATUS_STOP_USE);
    }
    else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID &&
        stuSplInfo.groupNum == 1 &&
        !stuImSplEnableInfo.bCleanBuffer1)
    {
        // 清洗缓冲液1停用
        strStatus = ConvertTfEnumToQString(im::tf::SuppliesUsingStatus::SUPPLIES_USING_STATUS_STOP_USE);
    }
    else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID &&
        stuSplInfo.groupNum == 2 &&
        !stuImSplEnableInfo.bCleanBuffer2)
    {
        // 清洗缓冲液2停用
        strStatus = ConvertTfEnumToQString(im::tf::SuppliesUsingStatus::SUPPLIES_USING_STATUS_STOP_USE);
    }
    else
    {
        // 使用状态
        strStatus = ConvertTfEnumToQString(im::tf::SuppliesUsingStatus::type(stuSplInfo.usingStatus));
    }

    // 记录文本
    rowContent.push_back(new QStandardItem(strStatus));

    // 余量
    int iResidual = stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS ? int(stuSplInfo.remainVol) : int(stuSplInfo.remainVol + 1.0 - 0.0001);
    if (stuSplInfo.available == 0 && 
        (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A || 
         stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B ||
         stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID))
    {
        iResidual = 0;
    }

    // 余量,获取剩余量(反应杯为剩余使用次数，其他为ml)  // 0019069: [试剂] 耗材界面，底物余量显示小数点，应该为整数显示。 modify by chenjialin 20230423
    QString struResidual = stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS ? \
        QString::number(iResidual) + " " + tr("个") : \
        QString::number(iResidual) + " " + "ml";

    QStandardItem* pRemainTestItem = new QStandardItem(struResidual);// 获取剩余测试数item
    if (iResidual <= 0)
    {
        SetTblItemState(pRemainTestItem, STATE_WARNING);
    }
    else if (RgntNoticeDataMng::GetInstance()->IsSplReVolNotice(stuSplInfo.supType, stuSplInfo.remainVol))
    {
        SetTblItemState(pRemainTestItem, STATE_NOTICE);
    }
    rowContent.push_back(pRemainTestItem);

    // 耗材批号
    rowContent.push_back(new QStandardItem(QString::fromStdString(stuSplInfo.supLot)));
    // 瓶号/序列号
    rowContent.push_back(new QStandardItem(QString::fromStdString(stuSplInfo.supSerial)));
    // 开瓶有效期(天)(反应杯) // 0019125: [试剂] 新扫描添加反应杯，添加成功后，耗材信息界面反应杯开瓶有效期显示为0且红色填充 modify bey chenjianlin 20230423
    if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
    {
        rowContent.push_back(new QStandardItem(""));
    }
    else // 开瓶有效期(天)(其他)
    {
        QDateTime splRegisterDate = QDateTime::fromString(QString::fromStdString(stuSplInfo.recordDate), UI_DATE_TIME_FORMAT);
        int showStableRestday = stuSplInfo.stableDays - splRegisterDate.daysTo(QDateTime::currentDateTime());
        if (showStableRestday < 0)
        {
            showStableRestday = 0;
        }
        QStandardItem* pStableDayItem = new QStandardItem(QString::number(showStableRestday));
        rowContent.push_back(pStableDayItem);
        if (showStableRestday <= 0)
        {
            SetTblItemState(pStableDayItem, STATE_WARNING);
        }
    }

    // 上机时间
    QStandardItem* itemrecordDate = new QStandardItem(QString::fromStdString(stuSplInfo.recordDate));
    rowContent.push_back(itemrecordDate);

    // 上机日期
    QDateTime recordDateTime = QDateTime::fromString(QString::fromStdString(stuSplInfo.recordDate), UI_DATE_TIME_FORMAT);
    if (recordDateTime.isValid() && (recordDateTime <= QDateTime(QDate(2000, 1, 1))))
    {
        itemrecordDate->setText("");
    }

    // 失效日期
    QStringList strDateTimeList = QString::fromStdString(stuSplInfo.expiryDate).split(" ");
    QString strDate = strDateTimeList.size() >= 1 ? strDateTimeList.front() : (""); //构造过期日期
    QStandardItem* itemExp = new QStandardItem(strDate);

    QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(stuSplInfo.expiryDate), UI_DATE_TIME_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDateTime(QDate(2000, 1, 1))))
    {
        itemExp->setText("");
    }
    else if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
    {
        SetTblItemState(itemExp, STATE_WARNING);
    }
    rowContent.push_back(itemExp);

    tableContent.push_back(rowContent);

    return true;
}

///
/// @brief 根据位置和模块名称查找对应的位置
///
/// @param[in]  postion  位置
/// @param[in]  devicesn  模块号
///
/// @return list中的位置
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月20日，新建函数
///
int QSupplyList::GetRowByPostion(int postion, const std::string& devicesn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int ret = -1;
    QString moduleName = GetDeviceNameByModuleDsc(QString::fromStdString(devicesn));
    if (m_supplyMode == nullptr || moduleName.isEmpty())
    {
        return ret;
    }

    QString strTarget = moduleName + QString::number(postion);
    QList<QStandardItem*> items = m_supplyMode->findItems(moduleName);
    for (const auto& item : items)
    {
        if (item->data(Qt::UserRole).toString() == strTarget)
        {
            return item->row();
        }
    }

    return ret;
}

///
/// @bref
///		获取当前模块开始的行号
///
/// @param[in] devicesn 模块SN
///
/// @par History:
/// @li 8276/huchunli, 2023年1月5日，新建函数
///
bool QSupplyList::GetRowsByModuleName(const std::string& devicesn, std::vector<int>& rows)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    auto moduleName = GetDeviceNameByModuleDsc(QString::fromStdString(devicesn));
    if (m_supplyMode == nullptr || moduleName.isEmpty())
    {
        return false;
    }

    rows.clear();
    QList<QStandardItem*> items = m_supplyMode->findItems(moduleName);
    for (const auto& it : items)
    {
        rows.push_back(it->row());
    }

    return rows.size() != 0;
}

///
/// @brief 根据设备列表获取设备SN列表
///
/// @param[in]  deviceModules  设备模块列表
///
/// @return 设备号列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月13日，新建函数
///
std::vector<std::string> QSupplyList::GetDeviceSnVec(Devices& deviceModules)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    std::vector<std::string> deviceSnList;
    std::transform(deviceModules.begin(), deviceModules.end(), std::back_inserter(deviceSnList), [](const auto& device)->std::string
    {return device->deviceSN; });
    return std::move(deviceSnList);
}

///
/// @brief
///     设置表格单元格状态
///
/// @param[in]  pItem    表格单元项
/// @param[in]  enState  状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月28日，新建函数
///
void QSupplyList::SetTblItemState(QStandardItem* pItem, TblItemState enState)
{
    //ULOG(LOG_INFO, __FUNCTION__); // too frequency.
    // 参数检查
    if (pItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, __FUNCTION__);
        return;
    }

    // 判断状态
    switch (enState)
    {
    case STATE_NORMAL:
        pItem->setData(QVariant(), Qt::TextColorRole);
        pItem->setData(QVariant(), Qt::UserRole + 1);
        //SetItemColor(pItem, QVariant());
        break;
    case STATE_NOTICE:
        pItem->setData(QColor(UI_REAGENT_NOTEFONT), Qt::TextColorRole);
        pItem->setData(QColor(UI_REAGENT_NOTEFONT), Qt::UserRole + 1);
        //SetItemColor(pItem, UI_REAGENT_NOTECOLOR);
        break;
    case STATE_WARNING:
        pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
        pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::UserRole + 1);
        //SetItemColor(pItem, UI_REAGENT_WARNCOLOR);
        break;
    case STATE_SCAN_FAILED:
        pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
        pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::UserRole + 1);
        //SetItemColor(pItem, QVariant());
        break;
    default:
        pItem->setData(QVariant(), Qt::TextColorRole);
        pItem->setData(QVariant(), Qt::UserRole + 1);
        //SetItemColor(pItem, QVariant());
        break;
    }
}

///
/// @brief 页面显示时，处理事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月26日，新建函数
///
void QSupplyList::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_needUpdate)
    {
        RefreshSupplyList(m_deivceVec);
        m_needUpdate = false;
    }

    // 初始化时列宽自适应
    if (!m_bInit)
    {
        // 列宽自适应
        ResizeTblColToContent(ui->tableView);
        m_bInit = true;
    }

    // 使能排序
    ui->tableView->setSortingEnabled(true);

    // 恢复默认排序
    m_pSortHeader->ResetAllIndex();
    m_supplyMode->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
    ui->tableView->sortByColumn(ECI_Module, Qt::AscendingOrder);
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    m_sortedHistory.first = -1;
    m_sortedHistory.second = Qt::DescendingOrder;
}

///
/// @brief
///     更新排序值
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月9日，新建函数
///
void QSupplyList::UpdateSortValue()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (m_supplyMode == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), m_supplyMode == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 遍历表格中所有单元格
    for (int iRow = 0; iRow < m_supplyMode->rowCount(); iRow++)
    {
        for (int iCol = 0; iCol < m_supplyMode->columnCount(); iCol++)
        {
            // 获取对应项
            QStandardItem* pItem = m_supplyMode->item(iRow, iCol);
            if (pItem == Q_NULLPTR)
            {
                continue;
            }

            // 设置默认排序值
            pItem->setData(iRow, UI_ITEM_ROLE_DEFAULT_SORT);

            // 获取文本
            QString strItemText = pItem->text();

            // 设置非默认排序值
            if ((iCol != ECI_Residual) &&
                (iCol != ECI_Lot) &&
                (iCol != ECI_Serial) &&
                (iCol != ECI_OpenEffectDays))
            {
                // 非数字相关列直接用文本排序
                pItem->setData(strItemText, UI_ITEM_ROLE_SEQ_NO_SORT);
                continue;
            }

            // 数字相关列特殊处理
            pItem->setData(GetStringSortVal(strItemText), UI_ITEM_ROLE_SEQ_NO_SORT);
        }
    }
}

///
/// @brief
///     耗材信息更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月12日，新建函数
///
void QSupplyList::OnSplInfoUpdate()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    UpdateConsumeTable();
}

///
/// @brief 优先使用耗材
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
void QSupplyList::OnPrioritySupply()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹框提示是否确认优先使用
	if (!IsReagentPriortyTipDlg(false))
	{
		ULOG(LOG_INFO, "Cancel priority!");
		return;
	}

    // 若是空行，不要进行操作
    std::vector<int> rowsIdx = GetSelectedRows();
    for (int rIdx : rowsIdx)
    {
        auto itemPos = m_supplyMode->item(rIdx, 0);
        if (itemPos == nullptr)
        {
            continue;
        }

        QString keyName = itemPos->data(Qt::UserRole).toString();
        std::map<QString, std::shared_ptr<SuppyDataItem>>::iterator it = m_supplyItemCache.find(keyName);
        if (it == m_supplyItemCache.end())
        {
            continue;
        }

        if (it->second->m_supplyType == ST_IM)
        {
            // 免疫
            im::tf::SuppliesInfoTable pItem = boost::any_cast<im::tf::SuppliesInfoTable>(it->second->m_supplyItem);
            tf::ResultLong _return;
            bool bRet = ::im::i6000::LogicControlProxy::SetSupplyPreUse(_return, pItem);
            if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "SetSupplyPreUse failed.");
                return;
            }

            // 免疫只调用一次
            break;
        }
        else
        {
            // 生化
            if (it->second->m_supplyType == ST_CH)
            {
                ch::tf::SuppliesInfo chSup = boost::any_cast<ch::tf::SuppliesInfo>(it->second->m_supplyItem);
                UpdatePrioritySupplyCh(chSup.deviceSN, chSup.suppliesCode, chSup.pos);
            }
            else if (it->second->m_supplyType == ST_ISE)
            {
                ise::tf::SuppliesInfo chSup = boost::any_cast<ise::tf::SuppliesInfo>(it->second->m_supplyItem);
                UpdatePrioritySupplyCh(chSup.deviceSN, chSup.suppliesCode, chSup.pos);
            }
        }
    }
    UpdateConsumeTable();// TODO: update appointed one.
}

void QSupplyList::UpdatePrioritySupplyCh(const std::string& deviceSn, int supplyCode, int pos)
{
	ULOG(LOG_INFO, "%s(), devSn:%s, supplyCode:%d, pos:%d.", __FUNCTION__, deviceSn, supplyCode, pos);

    ch::tf::SuppliesInfoQueryCond qryCond;
    // 耗材位置
    qryCond.__set_pos(pos);
    // 设置耗材编码
    qryCond.__set_suppliesCode(supplyCode);
    // 设置仓外耗材
    qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_CABINET);
    // 设备编号
    qryCond.__set_deviceSN(std::vector<std::string>({ deviceSn }));
    // 必须是在用的耗材
    vector<::tf::PlaceStatus::type> vecPlaceStatus;
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_REGISTER);
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_LOAD);
    qryCond.__set_placeStatus(vecPlaceStatus);

    // 查询选中项目的耗材信息
    std::vector< ::ch::tf::SuppliesInfoQueryCond> siqc;
    siqc.push_back(qryCond);
    if (!ch::c1005::LogicControlProxy::PriorUsingSupplies(siqc))
    {
        ULOG(LOG_ERROR, "PriorUsingSupplies() failed");
        return;
    }
}

///
/// @brief 更新耗材信息
///   
/// @param[in]  supplyUpdates  需要更新的耗材的详细信息（位置等）
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
void QSupplyList::OnUpdateSupplyInfo(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    std::vector<std::string> deviceSn = GetDeviceSnVec(m_deivceVec);
    for (const auto& supplyUpdate : supplyUpdates)
    {
        // 是否属于刷新范围
        if (!Contains(deviceSn, supplyUpdate.deviceSN)
            || supplyUpdate.posInfo.area != ch::tf::SuppliesArea::type::SUPPLIES_AREA_CABINET)
        {
            continue;
        }
        break;
    }
    UpdateConsumeTable();// TODO: update appointed one.
}


///
/// @brief  弹窗提示 未选中ISE的AT缓冲液或者内部标准液
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年12月29日，新建函数
///
void notSelectIsBuffer()
{
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QObject::tr("未选中ISE的%1或者%2!").arg(ISE_IS_NAME).arg(ISE_BUF_NAME)));
	pTipDlg->exec();
}

void QSupplyList::OnApplyIseSupply()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_regIseDialog == nullptr)
    {
        m_regIseDialog = new QRecordISESupply();
    }

	// 获取当前选中耗材
	std::vector<int> rowsIdx = GetSelectedRows();
	// 未选中
	if (rowsIdx.size() != 1)
	{
		notSelectIsBuffer();
		return;
	}

	int rIdx = rowsIdx[0];
	auto itemPos = m_supplyMode->item(rIdx, 0);
	if (itemPos == Q_NULLPTR)
	{
		notSelectIsBuffer();
		return;
	}

	QString keyName = itemPos->data(Qt::UserRole).toString();
	std::map<QString, std::shared_ptr<SuppyDataItem>>::iterator it = m_supplyItemCache.find(keyName);
	if (it == m_supplyItemCache.end())
	{
		return;
	}

	// 必须是ISE缓冲液或者内部标准液
	if (it->second->m_supplyType != ST_ISE)
	{
		notSelectIsBuffer();
		return;
	}

	// 获取ise耗材信息
	ise::tf::SuppliesInfo iseSupply = boost::any_cast<ise::tf::SuppliesInfo>(it->second->m_supplyItem);

	// 必须是ISE缓冲液或者内部标准液
	if (iseSupply.suppliesCode != ise::tf::SuppliesType::SUPPLIES_TYPE_IS
		&& iseSupply.suppliesCode != ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
	{
		notSelectIsBuffer();
		return;
	}

	std::shared_ptr<const tf::DeviceInfo> iseDevice;
	for (const auto& device : m_deivceVec)
	{
		if (device->deviceSN == iseSupply.deviceSN)
		{
			iseDevice = device;
			break;
		}
	}

	// 弹出耗材加载对话框
	m_regIseDialog->StartDialog(iseDevice,iseSupply.moduleIndex,iseSupply.suppliesCode);
}

void QSupplyList::SelectRelatedSupplyIm(int rowIdx)
{
    ULOG(LOG_INFO, __FUNCTION__);

    QStandardItem* itemModul = m_supplyMode->item(rowIdx, 0);
    if (itemModul == Q_NULLPTR)
    {
        return;
    }

    QVariant cachKey = itemModul->data(Qt::UserRole);
    if (!cachKey.isValid())
    {
        return;
    }
    auto it = m_supplyItemCache.find(cachKey.toString());
    if (it == m_supplyItemCache.end())
    {
        return;
    }

    if (it->second->m_supplyType == ST_IM)
    {
        im::tf::SuppliesInfoTable curSupply = boost::any_cast<im::tf::SuppliesInfoTable>(it->second->m_supplyItem);
        if (!IsExistStandbyType(curSupply.supType))
        {
            return;
        }

        std::vector<int> sameSelectedRows = { rowIdx };
        QList<QStandardItem*> sameModulItems = m_supplyMode->findItems(itemModul->text());
        for (const auto& item : sameModulItems)
        {
            // 无效行或者当前行则跳过
            QVariant itemCachKey = item->data(Qt::UserRole);
            if (!itemCachKey.isValid() || rowIdx == item->row())
            {
                continue;
            }

            // 无缓存对象则跳过
            auto subIt = m_supplyItemCache.find(itemCachKey.toString());
            if (subIt == m_supplyItemCache.end())
            {
                continue;
            }

            // 如果是同组耗材则选中
            im::tf::SuppliesInfoTable itemSupply = boost::any_cast<im::tf::SuppliesInfoTable>(subIt->second->m_supplyItem);
            if (IsExistStandbyType(itemSupply.supType) && curSupply.groupNum == itemSupply.groupNum)
            {
                sameSelectedRows.push_back(item->row());
            }
        }

        // 执行选中
        for (int iRow : sameSelectedRows)
        {
            QItemSelection selectItem(m_supplyMode->index(iRow, 0), m_supplyMode->index(iRow, 1));
            ui->tableView->selectionModel()->select(selectItem, QItemSelectionModel::Rows | QItemSelectionModel::Select);
        }
        ui->tableView->viewport()->update();
    }
}
///
/// @brief
///     是否选中反应杯
///
/// @return true表示是
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月19日，新建函数
///
bool QSupplyList::IsSelectReaction()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QModelIndex qIndex = ui->tableView->currentIndex();
    if (!qIndex.isValid())
    {
        return false;
    }
    int rowIdx = qIndex.row();

    // 获取模块项
    QStandardItem* itemModul = m_supplyMode->item(rowIdx, 0);
    if (itemModul == Q_NULLPTR)
    {
        return false;
    }

    // 获取缓存
    QVariant cachKey = itemModul->data(Qt::UserRole);
    if (!cachKey.isValid())
    {
        return false;
    }
    auto it = m_supplyItemCache.find(cachKey.toString());
    if (it == m_supplyItemCache.end())
    {
        return false;
    }

    if (it->second->m_supplyType != ST_IM)
    {
        return false;
    }

    im::tf::SuppliesInfoTable curSupply = boost::any_cast<im::tf::SuppliesInfoTable>(it->second->m_supplyItem);
    if (curSupply.supType != im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     获取选中设备序列号
///
/// @return 选中项的设备序列号，没有时返回空
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月8日，新建函数
///
QString QSupplyList::GetSelectDevSn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QModelIndex qIndex = ui->tableView->currentIndex();
    if (!qIndex.isValid())
    {
        return "";
    }
    int rowIdx = qIndex.row();

    // 获取模块项
    QStandardItem* itemModul = m_supplyMode->item(rowIdx, 0);
    if (itemModul == Q_NULLPTR)
    {
        return "";
    }

    // 获取模块名
    QString strDevName = itemModul->text();
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(strDevName);

    return strDevSn;
}

///
/// @brief 刷新按钮状态
///
/// @param[in]  index  行
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月22日，新建函数
///
void QSupplyList::OnTabelSelectChanged(const QItemSelection& currIndex, const QItemSelection& prevIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 禁能添加反应杯
    if (!CommonInformationManager::GetInstance()->GetImisSingle())
    {
        ui->AddReactionBtn->setEnabled(false);
    }

    QModelIndex qIndex = ui->tableView->currentIndex();
    if (!qIndex.isValid())
    {
        return;
    }
    int rowIdx = qIndex.row();

    // 如果只有免疫设备，则禁用耗材加载
    ui->supplyLoad_btn->setDisabled(IsDevicesOnlyIm(m_deivceVec));

    // 优先使用按钮状态
    auto item = m_supplyMode->item(rowIdx, NAMEROW);
    auto itemStatus = m_supplyMode->item(rowIdx, NAMEROW + 1);
    bool isPriority = (item && itemStatus) && item->data(Qt::UserRole + 10).toBool() && itemStatus->text().contains(tr("备用"));
    ui->supplyPriority->setEnabled(isPriority);

    // 更新添加反应杯按钮使能状态
    if (!CommonInformationManager::GetInstance()->GetImisSingle())
    {
        ui->AddReactionBtn->setEnabled(IsSelectReaction());
    }

    // 免疫组关联选中
    SelectRelatedSupplyIm(rowIdx);
}

///
/// @brief
///     免疫耗材信息更新
///
/// @param[in]  stuSplInfo   耗材信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月5日，新建函数
///
void QSupplyList::UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType)
{
    //ULOG(LOG_INFO, "%s()", __FUNCTION__); // too frequnce...
    Q_UNUSED(stuSplInfo);
    Q_UNUSED(changeType);

    UpdateConsumeTable(); // TODO: update appointed one.
}

///
/// @brief
///     耗材管理设置改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  stuSplMng   耗材管理
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月27日，新建函数
///
void QSupplyList::OnSplMngCfgChanged(QString strDevSn, ImDeviceOtherInfo stuSplMng)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(strDevSn);
    Q_UNUSED(stuSplMng);

    UpdateConsumeTable();
}

void QSupplyList::OnSortByColumn(int columNm)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int sortedColum = ui->tableView->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder sortedOrder = ui->tableView->horizontalHeader()->sortIndicatorOrder();

    m_sortedHistory = { sortedColum, sortedOrder };
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QSupplyList::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    SupplyItemVector vecInfo;
    FillExportContainer(vecInfo);

    // 如果没有任何数据，弹框提示
    if (vecInfo.empty())
    {
        // 弹框提示导出完成
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以打印的数据!")));
        pTipDlg->exec();
        return;
    }

    SupplyInfo Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    Info.vecSupply = std::move(vecInfo);
    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
}

///
/// @brief 响应导出按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QSupplyList::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    SupplyInfo Info;
    FillExportContainer(Info.vecSupply);

    // 如果没有任何数据，弹框提示
    if (Info.vecSupply.empty())
    {
        // 弹框提示导出完成
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可以导出的数据!")));
        pTipDlg->exec();
        return;
    }

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportSupplyInformation.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportSupplyInfo(Info.vecSupply, strFilepath);
    }
    
    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

///
/// @brief
///     加载反应杯按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月3日，新建函数
///
void QSupplyList::OnAddReactBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QModelIndex qIndex = ui->tableView->currentIndex();
    if (!qIndex.isValid())
    {
        ULOG(LOG_WARN, "%s(), invalid index", __FUNCTION__);
        return;
    }
    int rowIdx = qIndex.row();

    // 获取模块项
    QStandardItem* itemModul = m_supplyMode->item(rowIdx, 0);
    if (itemModul == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), empty row", __FUNCTION__);
        return;
    }

    // 获取模块名
    QString strDevName = itemModul->text();
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(strDevName);
    if (strDevSn.isEmpty())
    {
        ULOG(LOG_WARN, "%s(), strDevSn.isEmpty()", __FUNCTION__);
        return;
    }

    // 弹出添加反应杯对话框
    std::shared_ptr<ImAddReactCupDlg> spImAddReactCupDlg(new ImAddReactCupDlg(this));

    // 连接信号槽
    connect(spImAddReactCupDlg.get(), SIGNAL(SigBarcodeScanFinished()), this, SLOT(OnReactCupScanFinished()));

    // 弹出对话框
    if (spImAddReactCupDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
        return;
    }

    // 添加反应杯
    im::tf::SuppliesInfoTable stuSplInfo = spImAddReactCupDlg->GetScanSplData().value<im::tf::SuppliesInfoTable>();
    stuSplInfo.__set_deviceSN(strDevSn.toStdString());

    // 执行添加
    tf::ResultLong _return;
    bool bRet = ::im::LogicControlProxy::AddSupplyCup(_return, stuSplInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "AddSupplyCup failed.");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("反应杯信息存储失败，请检查反应杯条码！")));
        pTipDlg->exec();
        return;
    }

    // 刷新耗材界面
    UpdateConsumeTable();
}

///
/// @brief
///     反应杯扫描结束
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void QSupplyList::OnReactCupScanFinished()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    ImAddReactCupDlg* pImAddReactCupDlg = qobject_cast<ImAddReactCupDlg*>(sender());
    if (pImAddReactCupDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), qobject_cast failed.", __FUNCTION__);
        return;
    }

    // 获取设备序列号
    QString strDevSn = GetSelectDevSn();
    if (strDevSn.isEmpty())
    {
        ULOG(LOG_WARN, "%s(), strDevSn.isEmpty().", __FUNCTION__);
        return;
    }

    // 默认将提示置为扫描失败
    pImAddReactCupDlg->ui->ScanRltTipLab->setText(tr("信息读取失败！"));
    pImAddReactCupDlg->ui->ScanRltTipLab->setStyleSheet("color:red;font-size:16;");

    // 禁能确认按钮，清空耗材信息
    pImAddReactCupDlg->ui->OkBtn->setEnabled(false);
    pImAddReactCupDlg->ui->SplNameValLab->clear();
    pImAddReactCupDlg->ui->ResidualValLab->clear();
    pImAddReactCupDlg->ui->SplLotValLab->clear();
    pImAddReactCupDlg->ui->SplSnValLab->clear();
    pImAddReactCupDlg->ui->ExpDateValLab->clear();
    pImAddReactCupDlg->ui->RegisterTimeValLab->clear();
    pImAddReactCupDlg->SetScanSplData(QVariant());

    // 获取条码
    QString strSplBarcode = pImAddReactCupDlg->ui->BarcodeInputEdit->text();

    // 解析条码
    ::im::tf::ParseSupplyeCupResp CupRlt;
    bool bRet = ::im::LogicControlProxy::ParseSupplyCupInfo(CupRlt, strSplBarcode.toStdString(), strDevSn.toStdString());
    if (!bRet || CupRlt.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "ParseSupplyCupInfo failed.");
        return;
    }

    // 获取返回结果
    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::HasAdded)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("反应杯信息重复登记，请更换反应杯条码！")));
        pTipDlg->exec();
        return;
    }

    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::IsNotCup)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("不是反应杯条码。")));
        pTipDlg->exec();
        return;
    }

    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::ParseBarcodeErr)
    {
        //         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("解析反应杯条码失败。")));
        //         pTipDlg->exec();
        //         return;
    }

    if (CupRlt.scanResult != im::tf::ScanSupplyCupType::CanAdd)
    {
        ULOG(LOG_WARN, "can not add.");
        return;
    }

    // 允许添加
    // 更新提示
    pImAddReactCupDlg->ui->ScanRltTipLab->setText(tr("扫描结果成功！"));
    pImAddReactCupDlg->ui->ScanRltTipLab->setStyleSheet("color:green;font-size:16;");

    // 使能确认按钮，加载耗材信息
    pImAddReactCupDlg->ui->OkBtn->setEnabled(true);
    pImAddReactCupDlg->ui->SplNameValLab->setText(tr("反应杯"));
    pImAddReactCupDlg->ui->ResidualValLab->setText(QString::number(CupRlt.cup.remainVol) + tr("个"));
    pImAddReactCupDlg->ui->SplLotValLab->setText(QString::fromStdString(CupRlt.cup.supLot));
    pImAddReactCupDlg->ui->SplSnValLab->setText(QString::fromStdString(CupRlt.cup.supSerial));
    pImAddReactCupDlg->ui->ExpDateValLab->setText(ToCfgFmtDateTime(QString::fromStdString(CupRlt.cup.expiryDate)));
    pImAddReactCupDlg->ui->RegisterTimeValLab->setText(ToCfgFmtDateTime(QString::fromStdString(CupRlt.cup.recordDate)));

    // 缓存数据
    pImAddReactCupDlg->SetScanSplData(QVariant::fromValue<im::tf::SuppliesInfoTable>(CupRlt.cup));
}

void QSupplyList::OnRemainDetect()
{
	// 传入ise和ch设备进行样本针酸碱余量探测
	std::vector<std::shared_ptr<const tf::DeviceInfo>> devs;

	/*
	// 如果是单机 ise样本针酸碱清洗液使用的是CH的
	if (!CommonInformationManager::GetInstance()->GetPipeLine())
	{
		devs = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>(\
		{ tf::DeviceType::DEVICE_TYPE_C1000 }));
	}
	// 联机时ise样本针酸碱清洗液才是独立的
	else
	{
		devs = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>(\
		{ tf::DeviceType::DEVICE_TYPE_C1000 ,tf::DeviceType::DEVICE_TYPE_ISE1005 }));
	*/

	// 最新需求只有联机ise需要样本针余量探测
	devs = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_ISE1005 }));

	m_sampleDetectDlg->LoadDataToDlg(devs);
	m_sampleDetectDlg->show();
}

void QSupplyList::StartRemainDetect(std::vector<std::pair<std::string, int>> devs)
{
	ULOG(LOG_INFO, "%s()",__FUNCTION__);

	// 需要余量探测的设备
	std::vector<::tf::DevicekeyInfo> maintainDevs;
	for (const auto& dev : devs)
	{
		::tf::DevicekeyInfo devKeyInfo;
		devKeyInfo.__set_sn(dev.first);
		devKeyInfo.__set_modelIndex(0);
		maintainDevs.push_back(std::move(devKeyInfo));
	}

	// 获取余量探测维护组
	auto maintainGroup = MaintainDataMng::GetInstance()->GetSingleMaintainGrpByType(tf::MaintainItemType::MAINTAIN_ITEM_ISE_SURPLUS_DETECTION);

	// 执行维护
	tf::MaintainExeParams exeParams;
	exeParams.__set_groupId(maintainGroup.id);
	exeParams.__set_lstDev(maintainDevs);
	if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
	{
		ULOG(LOG_ERROR, "Failed to execute ISE_SURPLUS_DETECTION.");
	}

}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年10月31日，新建函数
///
void QSupplyList::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 导出按钮权限限制
    if (userPms->IsPermisson(PSM_IM_EXPORT_SUPPLY))
    {
        ui->export_btn->show();
    }
    else
    {
        ui->export_btn->hide();
    }
}

void QSupplyList::OnISEPerfusion()
{
	m_isePerfusionDlg->LoadDataToDlg(m_deivceVec);
	m_isePerfusionDlg->show();
}

void QSupplyList::OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	UpdateConsumeTable();
}

void QSupplyList::OnDeviceOtherInfoUpdate(const QString& devSn)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devSn.toStdString());

	auto iter = find_if(m_deivceVec.begin(), m_deivceVec.end(), [=](std::shared_ptr<const tf::DeviceInfo> spDevice) {
		return spDevice->deviceSN == devSn.toStdString();
	});

	// 如果设备列表无相关设备无需更新
	if (iter == m_deivceVec.end())
	{
		return;
	}

	UpdateConsumeTable();
}

SuppyDataItem::SuppyDataItem(const ch::tf::SuppliesInfo& supplyItem)
{
    m_supplyType = ST_CH;
    m_supplyItem = supplyItem;
}

SuppyDataItem::SuppyDataItem(const ise::tf::SuppliesInfo& supplyItem)
{
    m_supplyType = ST_ISE;
    m_supplyItem = supplyItem;
}

SuppyDataItem::SuppyDataItem(const im::tf::SuppliesInfoTable& supplyItem)
{
    m_supplyType = ST_IM;
    m_supplyItem = supplyItem;
}

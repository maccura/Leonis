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
/// @file     QCalibrateOverView.h
/// @brief    校准总览
///
/// @author   5774/WuHongTao
/// @date     2022年2月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCalibrateOverView.h"
#include "ui_QCalibrateOverView.h"
#include <QStandardItemModel>

#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/QComDelegate.h"
#include "shared/QMachineFactor.h"
#include "shared/QCaliRackOverview.h"
#include "shared/ReagentCommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/CReadOnlyDelegate.h"

#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_constants.h"

#include "CaliBrateCommom.h"
#include "SortHeaderView.h"
#include "QCaliBrateMethodApp.h"
#include "QReactionCurve.h"
#include "QCaliBrateResult.h"
#include "QCalibrationFactorEdit.h"

#include "../im/ImQCaliBrateResult.h"
#include "../ise/QIseCaliBrateResult.h"
#include "../ise/QIseCaliAppManager.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#define SelectedColumn 9
#define CaliMethodColumn 8
#define CaliReasonColumn 7
#define ExpirateDays 5

Q_DECLARE_METATYPE(QCalibrateOverView::UserDataRoles);
Q_DECLARE_METATYPE(::ise::tf::IseModuleInfo);

QCalibrateOverView::QCalibrateOverView(QWidget *parent)
    : QWidget(parent),
    m_caliBrateMode(nullptr),
    m_CaliAppDialog(nullptr),
    m_reactionDialog(nullptr),
    m_calibrateResultDialog(nullptr),
    m_imCaliRestDlg(nullptr),
    m_iseCaliRestDlg(nullptr),
    m_machineFactorDlg(nullptr),
    m_caliRackOverviewDlg(new QCaliRackOverview(this)),
    m_iseCaliAppManager(new QIseCaliAppManager),
	m_caliFactorEditDlg(nullptr),
	m_needUpdate(false),
    m_blankMethodEnable(false),
    m_bIsSearching(false)
{
    ui = new Ui::QCalibrateOverView();
    ui->setupUi(this);
    Init();

    // 限制搜索框最多50个字符
    ui->comboBox_projectName->lineEdit()->setMaxLength(50);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

QCalibrateOverView::~QCalibrateOverView()
{
}

///
/// @brief刷新设备试剂列表
///     
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
void QCalibrateOverView::RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    // 1.如果页面不可见，但是有数据，说明页面已经初始过一次数据了，不用重复初始化了
    /*if (!isVisible() && m_caliBrateMode->rowCount() > 1)
        return;*/

    // 重置表格和搜索框的内容
    ClearContent();
    bool firstEntry = m_devs.empty();
    m_devs.clear();
    m_loadeAssays.clear();
    m_filterAssayName.clear();

    // 根据模块依次显示
    for (const std::shared_ptr<const tf::DeviceInfo>& device : devices)
    {
        //if(!device->masked)   是否屏蔽
        m_devs[device->deviceClassify].push_back(device->deviceSN);
    }

    // 全选，第一次进入页面，获取所有数据进行缓存
    //if (firstEntry)
    //{
        // 刷新页面
        UpdateCalibrateStatusTable(m_devs);
    //}

    // 按第一列升序排序
    SetAscSortByFirstColumn();
}

void QCalibrateOverView::hideEvent(QHideEvent * event)
{
    SetAscSortByFirstColumn();

    QWidget::hideEvent(event);
}

void QCalibrateOverView::SetAscSortByFirstColumn()
{
    // 第一列按位置升序排列
    m_caliBrateMode->setSortRole(Qt::UserRole + DefaultSort);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);
    //去掉排序三角样式
    ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    //-1则还原model默认顺序
    m_caliBrateModeProxyModel->sort(-1, Qt::DescendingOrder);
}

///
/// @brief
///     初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月16日，新建函数
///
void QCalibrateOverView::Init()
{
    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, UpdateCaliSatus);
    // 监听免疫试剂更新信息
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateSingleCaliReqInfo);
    // 监听电解质校准申请更新信息
    REGISTER_HANDLER(MSG_ID_ISE_CALI_APP_UPDATE, this, UpdateCaliAppSatus);
	// 项目信息变更
	REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
    // 过零点刷新过期时间
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, OnRefreshCaliInfoList);
    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);
    // 监听校准提示是否变化
    REGISTER_HANDLER(MSG_ID_CALI_LINE_EXPIRE_CONFIG, this, OnCaliExpireNoticeChanged);
    // 选择
    connect(ui->calibrate_sel_btn, &QPushButton::clicked, this, &QCalibrateOverView::OnSelectBtn);
    // 执行方法
    connect(ui->caliBrateApp_Button, SIGNAL(clicked()), this, SLOT(OnShowAppDialog()));
    // 校准屏蔽解除
    connect(ui->unMaskButton, &QPushButton::clicked, this, &QCalibrateOverView::UnMasckCali);
    // 校准结果
    connect(ui->calibrate_result_btn, SIGNAL(clicked()), this, SLOT(OnShowCalibrateResult()));
    // 反应曲线
    connect(ui->Reaction_Button, SIGNAL(clicked()), this, SLOT(OnShowReactCurve()));
	// 校准系数编辑
	connect(ui->caliFactorEdit_btn, &QPushButton::clicked, this, &QCalibrateOverView::OnShowCaliFactorEditDialog);
    // 校准架概况
    connect(ui->caliRackOverview_btn, &QPushButton::clicked, this, &QCalibrateOverView::OnShowCaliRackOverviewDlg);
    // 校准文档的选择
    connect(ui->tableView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(OnCaliDocSelected(const QModelIndex&)));
	// 执行方法更改
	connect(ui->tableView, &QTableView::doubleClicked, this, &QCalibrateOverView::OnCaliModeChanged);
	// 项目名称查询
	connect(ui->pushButton_search, &QPushButton::clicked, this, &QCalibrateOverView::OnCaliProjectNameSearch);
    connect(ui->comboBox_projectName, &QFilterComboBox::ResetText, this, &QCalibrateOverView::OnCaliProjectNameSearch);
    //connect(ui->comboBox_projectName, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
    //    this, &QCalibrateOverView::OnCaliProjectNameSearch);

    connect(ui->machineFactor, &QPushButton::clicked, this, [this] {
        if (m_machineFactorDlg == nullptr)
        {
            m_machineFactorDlg = new QMachineFactor(this);
        }

        m_machineFactorDlg->show();
    });
    connect(this, &QCalibrateOverView::sigSelBtnStateChanged, this, [this](bool sel) {
        if (sel)
            ui->calibrate_sel_btn->setText(tr("选择"));
        else
            ui->calibrate_sel_btn->setText(tr("取消选择"));
    });

    // 初始状态筛选按钮不可用，有文字的时候才可以用
    ui->pushButton_search->setEnabled(false);
    connect(ui->comboBox_projectName, &QComboBox::editTextChanged, this, [this] {
        if (ui->comboBox_projectName->lineEdit()->text().isEmpty())
            ui->pushButton_search->setEnabled(false);
        else
            ui->pushButton_search->setEnabled(true);
    });
	
    // 清除内容
    ClearContent();
}

///
/// @brief
///     清空内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
void QCalibrateOverView::ClearContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_caliBrateMode == nullptr)
    {
        m_caliBrateMode = new QToolTipModel(this);
		m_caliBrateModeProxyModel = new QCalibrateOverFilterDataModule(ui->tableView);
		m_caliBrateModeProxyModel->setSourceModel(m_caliBrateMode);
        ui->tableView->setModel(m_caliBrateModeProxyModel);
		// 升序 降序 原序
		SortHeaderView *pCaliBrateHeader = new SortHeaderView(Qt::Horizontal, ui->tableView);
		//pCaliBrateHeader->setStretchLastSection(true);
		ui->tableView->setHorizontalHeader(pCaliBrateHeader);
		connect(pCaliBrateHeader, &SortHeaderView::SortOrderChanged, this, [this, pCaliBrateHeader](int logicIndex, SortHeaderView::SortOrder order) {
			QTableView *view = ui->tableView;
			//无效index或NoOrder就设置为默认未排序状态
			if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                SetAscSortByFirstColumn();
            }
            else
            {
                int iSortRole = Qt::DisplayRole;
                switch (logicIndex)
                {
                    case 0:
                        iSortRole = Qt::UserRole + DefaultSort;
                        break;
                    case SelectedColumn:
                        iSortRole = Qt::UserRole + 1;
                        break;
                    case  ExpirateDays:
                        iSortRole = Qt::UserRole + 3;
                        break;
                }
                m_caliBrateMode->setSortRole(iSortRole);
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                view->sortByColumn(logicIndex, qOrderFlag);
			}
		});
		//去掉排序三角样式
		ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
		//-1则还原model默认顺序
		m_caliBrateModeProxyModel->sort(-1, Qt::DescendingOrder);

        ui->tableView->setModel(m_caliBrateMode);
        ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        //ui->tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
        ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView->verticalHeader()->hide();
        ui->tableView->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(3, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(4, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(5, new AutoExpriteDelegate(this));
        ui->tableView->setItemDelegateForColumn(6, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(CaliReasonColumn, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(CaliMethodColumn, new CReadOnlyDelegate(this));
        ui->tableView->setItemDelegateForColumn(SelectedColumn, new CheckBoxDelegate(this));
        ui->tableView->setSortingEnabled(true);
        m_caliBrateMode->setRowCount(30);

        connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QCalibrateOverView::OnSelectionChanged);

    }

    // 清除页面
    m_caliBrateMode->clear();
    m_iseCaliAppManager->OnClearTableModel();
    // 设置校准屏蔽解除按钮是否可用
    OnSelectionChanged(QItemSelection(), QItemSelection());
    // 重新设置页面
    QStringList supplyHeaderListString;
    supplyHeaderListString << tr("试剂位") << tr("项目名称") << tr("试剂批号") << tr("使用状态") << tr("校准状态")
        << tr("当前曲线有效期") << tr("上机时间") << tr("校准原因") << tr("执行方法") << tr("选择");
    m_caliBrateMode->setHorizontalHeaderLabels(supplyHeaderListString);
    m_caliBrateMode->setHeaderData(CaliMethodColumn, Qt::Horizontal, QColor::fromRgb(2,91,199), Qt::ForegroundRole);
    m_caliBrateMode->setHeaderData(SelectedColumn, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);
    
    SetTableColumnFixedWidth();
}

///
/// @brief
///     更新试剂的校准状态
///
/// @param[in]  devices  设备信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
void QCalibrateOverView::UpdateCalibrateStatusTable(DeviceMap&  devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清除表格内容
    ClearContent();

    // 存储最后一个设备的最大位置号
    int maxPos = -1;
    // 当前已经写入数据的设备
    QVector<std::string> devList;

    // 不是执行筛选的话，需要清理一次搜索框
    if (!m_bIsSearching)
        m_loadeAssays.clear();
    
    //如果有生化设备
    if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) != devices.end())
    {
        // 显示生化按钮
        SetChBtnVisible(true);
        const auto& lstReagentGroup = CommonInformationManager::GetInstance()->GetAllChReagentGroupsByDevice(devices[tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY]);
        for (const ch::tf::ReagentGroup& reagentGroup : lstReagentGroup)
        {
            auto spSupply = GetSupplyInfoByReagent(reagentGroup);
            if (!spSupply)// || spSupply->placeStatus != tf::PlaceStatus::PLACE_STATUS_LOAD)
            {
                ULOG(LOG_ERROR, "spSupply is nullptr, reagentGroup id:%lld.", reagentGroup.id);
                continue;
            }
            AddAssayToSearchList(reagentGroup.assayCode);
            bool action = GetInsertMode(reagentGroup.posInfo.pos, reagentGroup.deviceSN, maxPos, devList);

            ShowReagentStatusItem(action, m_caliBrateMode->rowCount(), reagentGroup);
        }
    }
    else
    {
        // 隐藏生化按钮
        SetChBtnVisible(false);
    }

    // 检查是否选中了免疫设备
    if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE) != devices.end())
    {
        // 清除所有已配对信息
        m_imPairedReagent.clear();

        // 构造查询条件和查询结果
        ::im::tf::ReagentInfoTableQueryResp qryResp;
        ::im::tf::ReagTableUIQueryCond qryCond;

        // 设置设备列表
        qryCond.__set_deviceSNs(devices[tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE]);

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        }
        // 如果结果为空则返回
        else if (qryResp.lstReagentInfos.empty())
        {
            ULOG(LOG_INFO, "qryResp.lstReagentInfos.empty().");
        }
        else 
        {
            // 将试剂信息加载到试剂盘中
            int iRow = 0;

            // 预处理试剂列表
            std::vector<im::tf::ReagentInfoTable> preDealReagents;

            for (const im::tf::ReagentInfoTable& stuRgntInfo : qryResp.lstReagentInfos)
            {
                // 保存试剂配对信息，如果不存在配对信息，会直接返回
                StorePairedReagent(stuRgntInfo);
                AddAssayToSearchList(stuRgntInfo.assayCode);

				// 预处理试剂的瓶1显示
				if (stuRgntInfo.bottleIdx == 1)
				{
                    preDealReagents.push_back(stuRgntInfo);
					continue;					
				}
                bool action = GetInsertMode(stuRgntInfo.reagentPos, stuRgntInfo.deviceSN, maxPos, devList);

                ShowImReagentStatusItem(action, m_caliBrateMode->rowCount(), stuRgntInfo, true);
            }
            
            //处理bottleIdx为为1的配对信息
            for (const auto& reagent : preDealReagents)
            {
                bool action = GetInsertMode(reagent.reagentPos, reagent.deviceSN, maxPos, devList);

                ShowImReagentStatusItem(action, m_caliBrateMode->rowCount(), reagent, true);
            }
        }
    }

    // 检查是否选中了ISE设备
    if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_ISE) != devices.end())
    {
        const auto& items = m_iseCaliAppManager->GetIseCaliItems(devices[tf::AssayClassify::ASSAY_CLASSIFY_ISE]);

        m_loadeAssays.insert("ISE-U");
        m_loadeAssays.insert("ISE-S");

        for (const auto& line : items)
        {
            if (IsShowLineByFilter(line))
                m_caliBrateMode->appendRow(line);
        }
            
    }
    
    SetSearchList();
	
    SetTableColumnFixedWidth();
}

///
/// @brief
///     显示生化试剂校准状态信息
///
/// @param[in]  reagentInfo  试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
void QCalibrateOverView::ShowReagentStatusItem(bool addAction, int pos, const ch::tf::ReagentGroup& reagentInfo)
{
    ULOG(LOG_INFO, "%s(). id:%lld,deviceSn:%s, beDeleted:%d, pos:%d, caliStatus:%d, caliReason:%d, caliMode:%d, caliSelect:%d", 
        __FUNCTION__, reagentInfo.id, reagentInfo.deviceSN, reagentInfo.beDeleted, reagentInfo.posInfo.pos,
        reagentInfo.caliStatus, reagentInfo.caliReason, reagentInfo.caliMode, reagentInfo.caliSelect);

    // 获取耗材信息
    auto supplyInfo = GetSupplyInfoByReagent(reagentInfo);
    if (supplyInfo == nullptr)
    {
        ULOG(LOG_ERROR, "supplyInfo is nullptr!");
        return;
    }

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(reagentInfo.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", reagentInfo.assayCode);
        return;
    }

    // 根据项目名称过滤,过滤不需要的项目信息
    QList<int> shiledList;
    shiledList << ch::tf::g_ch_constants.ASSAY_CODE_SIND;
    if (shiledList.contains(spAssayInfo->assayCode))
    {
        ULOG(LOG_INFO, "SIND is ");
        return;
    }

    // 内容list
    QList<QStandardItem*> itemList;

    // 如果是覆盖的话(按照位置覆盖)
    if (!addAction)
    {
        int col = 0;
        auto GetColumnItem = [&](int row, int column) -> QStandardItem* {
            QStandardItem* item = m_caliBrateMode->item(row, column);
            if (item == nullptr)
            {
                item = new QStandardItem;
            }

            // 初始化文字颜色
            //item->setForeground(QColor("#565656"));
            // 初始化颜色为空
            //SetItemColor(item, QVariant());
            //item->setData(QColor("#565656"), Qt::TextColorRole);
            item->setData(QColor("#565656"), Qt::UserRole + 1);

            return item;
        };

        // 项目位置
        QStandardItem* itemPostion = SetItemTextAligCenter(GetItemFromPostionAndDevice(reagentInfo.deviceSN, reagentInfo.posInfo.pos, GetColumnItem(pos, col++)));
        itemPostion->setData(QVariant::fromValue<ch::tf::ReagentGroup>(reagentInfo), Qt::UserRole + QCalibrateOverView::ReagentInfo);
        itemPostion->setData(static_cast<qlonglong>(reagentInfo.id), Qt::UserRole + QCalibrateOverView::DatabaseId);
        itemPostion->setData(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, Qt::UserRole + QCalibrateOverView::AssayClassify);
        // 生化有双向同侧，再增加一个项目编号，用来同位置的排序
        QString sotrPost = itemPostion->text().split('-')[0] + "-" + QString::number(reagentInfo.posInfo.pos).rightJustified(3, '0') + "-" + QString::number(reagentInfo.assayCode).rightJustified(5, '0');
        itemPostion->setData(sotrPost, Qt::UserRole + QCalibrateOverView::DefaultSort);
        /*    m_caliBrateMode->setItem(pos, column++, itemPostion);*/
        itemList.push_back(itemPostion);

        // 项目名称
        /*    m_caliBrateMode->setItem(pos, column++, new QStandardItem(QString::fromStdString(spAssayInfo->assayName)));*/

        // 如果是开放试剂，则加*
        QStandardItem* item = GetColumnItem(pos, col++);
        if (reagentInfo.openReagent)
            item->setText(QString::fromStdString(spAssayInfo->assayName + "*"));
        else
            item->setText(QString::fromStdString(spAssayInfo->assayName));
        itemList.push_back(SetItemTextAligCenter(item));

        // 试剂批号
        /*	m_caliBrateMode->setItem(pos, column++, new QStandardItem(QString::fromStdString((supplyInfo->lot))));*/
        item = GetColumnItem(pos, col++);
        item->setText(QString::fromStdString(supplyInfo->lot));
        itemList.push_back(SetItemTextAligCenter(item));

        // 使用状态
        /*	m_caliBrateMode->setItem(pos, column++, GetReagentShowStatus(*supplyInfo, reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetReagentShowStatus(*supplyInfo, reagentInfo, GetColumnItem(pos, col++))));

        // 校准状态
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliBrateShowStatus(reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetCalibrateShowStatus(reagentInfo.caliStatus, IsCaliCurveIdValiable(reagentInfo), GetColumnItem(pos, col++))));

        // 校准类型
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliBrateType(reagentInfo));*/
        //itemList.push_back(GetCaliBrateType(reagentInfo));

        // 当前曲线有效期
        /*    column++;*/
        itemList.push_back(SetItemTextAligCenter(GetCaliCurveExpirateDate(reagentInfo, GetColumnItem(pos, col++))));

        // 上机时间
        item = GetColumnItem(pos, col++);
        item->setText(QString::fromStdString(reagentInfo.registerTime));
        itemList.push_back(SetItemTextAligCenter(item));

        // 试剂有效期
        /*    m_caliBrateMode->setItem(pos, column++, GetReagenExpiredStatus(reagentInfo));*/
        //itemList.push_back(GetReagenExpiredStatus(reagentInfo));

        // 校准时间
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliTime(reagentInfo));*/
        //itemList.push_back(GetCaliTime(reagentInfo));

        // 校准原因
        /*    m_caliBrateMode->setItem(pos, column++, GetCaliBrateReason(reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetCaliBrateReason(reagentInfo.caliReason, GetColumnItem(pos, col++))));

        // 执行方法
        /*    m_caliBrateMode->setItem(pos, column++, GetCaliBrateMethod(reagentInfo.caliMode));*/
        itemList.push_back(SetItemTextAligCenter(GetCaliBrateMethod(reagentInfo.caliMode, GetColumnItem(pos, col++))));

        // 设置选择与否
        QStandardItem* IsSelectedItem = GetColumnItem(pos, col++);
        // 是否选择校准
        if (reagentInfo.caliSelect)
        {
            IsSelectedItem->setData(true);
        }
        else
        {
            IsSelectedItem->setData(false);
        }
        // 选择与否
        /*	m_caliBrateMode->setItem(pos, column++, IsSelectedItem);*/
        itemList.push_back(IsSelectedItem);
    }
    else
    {
        // 项目位置
        QStandardItem* itemPostion = SetItemTextAligCenter(GetItemFromPostionAndDevice(reagentInfo.deviceSN, reagentInfo.posInfo.pos));
        itemPostion->setData(QVariant::fromValue<ch::tf::ReagentGroup>(reagentInfo), Qt::UserRole + QCalibrateOverView::ReagentInfo);
        itemPostion->setData(static_cast<qlonglong>(reagentInfo.id), Qt::UserRole + QCalibrateOverView::DatabaseId);
        itemPostion->setData(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, Qt::UserRole + QCalibrateOverView::AssayClassify);
        QString sotrPost = itemPostion->text().split('-')[0] + "-" + QString::number(reagentInfo.posInfo.pos).rightJustified(3, '0');
        itemPostion->setData(sotrPost, Qt::UserRole + QCalibrateOverView::DefaultSort);
        /*    m_caliBrateMode->setItem(pos, column++, itemPostion);*/
        itemList.push_back(itemPostion);

        // 项目名称
        /*    m_caliBrateMode->setItem(pos, column++, new QStandardItem(QString::fromStdString(spAssayInfo->assayName)));*/

        // 如果是开放试剂，则加*
        if (reagentInfo.openReagent)
            itemList.push_back(CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName + "*")));
        else
            itemList.push_back(CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName)));

        // 试剂批号
        /*	m_caliBrateMode->setItem(pos, column++, new QStandardItem(QString::fromStdString((supplyInfo->lot))));*/
        itemList.push_back(CenterAligmentItem(QString::fromStdString(supplyInfo->lot)));

        // 使用状态
        /*	m_caliBrateMode->setItem(pos, column++, GetReagentShowStatus(*supplyInfo, reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetReagentShowStatus(*supplyInfo, reagentInfo)));

        // 校准状态
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliBrateShowStatus(reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetCalibrateShowStatus(reagentInfo.caliStatus, IsCaliCurveIdValiable(reagentInfo))));

        // 校准类型
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliBrateType(reagentInfo));*/
        //itemList.push_back(GetCaliBrateType(reagentInfo));

        // 当前曲线有效期
        /*    column++;*/
        int iSortData = -1;
        if (itemList.back()->text().indexOf(":") != -1)
        {
            iSortData = (itemList.back()->text().mid(itemList.back()->text().indexOf(":") + 1)).toInt();
        }
        itemList.push_back(SetItemTextAligCenter(GetCaliCurveExpirateDate(reagentInfo)));

        // 上机时间
        itemList.push_back(CenterAligmentItem(QString::fromStdString(reagentInfo.registerTime)));

        // 试剂有效期
        /*    m_caliBrateMode->setItem(pos, column++, GetReagenExpiredStatus(reagentInfo));*/
        //itemList.push_back(GetReagenExpiredStatus(reagentInfo));

        // 校准时间
        /*	m_caliBrateMode->setItem(pos, column++, GetCaliTime(reagentInfo));*/
        //itemList.push_back(GetCaliTime(reagentInfo));

        // 校准原因
        /*    m_caliBrateMode->setItem(pos, column++, GetCaliBrateReason(reagentInfo));*/
        itemList.push_back(SetItemTextAligCenter(GetCaliBrateReason(reagentInfo.caliReason)));

        // 执行方法
        /*    m_caliBrateMode->setItem(pos, column++, GetCaliBrateMethod(reagentInfo.caliMode));*/
        itemList.push_back(SetItemTextAligCenter(GetCaliBrateMethod(reagentInfo.caliMode)));

        // 设置选择与否
        QStandardItem* IsSelectedItem = CenterAligmentItem("");
        // 是否选择校准
        if (reagentInfo.caliSelect)
        {
            IsSelectedItem->setData(true);
        }
        else
        {
            IsSelectedItem->setData(false);
        }
        // 选择与否
        /*	m_caliBrateMode->setItem(pos, column++, IsSelectedItem);*/
        itemList.push_back(IsSelectedItem);
    }

    if ((reagentInfo.reagentMask) || (reagentInfo.caliMask))
    {
        for (int i = 0; i < SelectedColumn; ++i)
        {
            //itemList[i]->setForeground(Qt::gray);
            itemList[i]->setData(QColor(Qt::gray), Qt::UserRole + 1);
        }
    }

    SetRowItemData(reagentInfo.deviceSN, itemList, pos, reagentInfo.posInfo.pos, addAction);
}

///
/// @brief 免疫试剂校准状态信息
///
/// @param[in]  addAction  true表示插入数据,false表示覆盖
/// @param[in]  pos  数据写入行数（插入时，重新计算位置）
/// @param[in]  reagentInfo  试剂信息
/// @param[in]  findDB  是否重新查数据库
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月24日，新建函数
///
void QCalibrateOverView::ShowImReagentStatusItem(bool addAction, int pos, const im::tf::ReagentInfoTable & reagentInfo, bool findDB)
{
	// 不可用状态直接 或超过位置的 不刷新
	// 此处使用固定数字作为位置，可能会出现维护问题
	if (reagentInfo.reagentPos < 1 || reagentInfo.reagentPos > ::im::i6000::tf::g_i6000_constants.REAGENT_DISK_SLOT)
		return;

	im::tf::ReagentInfoTable reagent = reagentInfo;
	if (findDB)
	{
		::im::tf::ReagentInfoTableQueryResp qryResp;
		::im::tf::ReagTableUIQueryCond qryCond;
		qryCond.__set_reagentId(reagentInfo.id);
		// 查询所有试剂信息
		bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
		if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || qryResp.lstReagentInfos.size() != 1)
		{
			ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
            return;
		}
		reagent = qryResp.lstReagentInfos.at(0);
	}

    if (reagent.bottleIdx == 1 && DealImPairedReagentOfBottleIdx_1(reagent))
        return;

    // 存储位置与试剂组的对应关系(增加|作为分隔符，避免因为纯数字造成的设备号加id造成的偶然重复)
    QString keyGroupInfo = QString::fromStdString(reagentInfo.deviceSN) + "|" + QString::number(reagentInfo.id);

    // 内容list
    QList<QStandardItem*> itemList;

    // 项目位置
    QStandardItem* itemPostion = SetItemTextAligCenter(GetItemFromPostionAndDevice(reagentInfo.deviceSN, reagentInfo.reagentPos));
    itemPostion->setData(QVariant::fromValue<im::tf::ReagentInfoTable>(reagentInfo), Qt::UserRole + QCalibrateOverView::ReagentInfo);
    itemPostion->setData(static_cast<qlonglong>(reagentInfo.id), Qt::UserRole + QCalibrateOverView::DatabaseId);
    itemPostion->setData(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE, Qt::UserRole + QCalibrateOverView::AssayClassify);
    QString sotrPost = itemPostion->text().split('-')[0] + "-" + QString::number(reagentInfo.reagentPos).rightJustified(3, '0');
    itemPostion->setData(sotrPost, Qt::UserRole + QCalibrateOverView::DefaultSort);
    itemList.push_back(itemPostion);

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(reagentInfo.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", reagentInfo.assayCode);
        return;
    }

    // 项目名称
    itemList.push_back(CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName)));

    // 试剂批号
    itemList.push_back(CenterAligmentItem(QString::fromStdString(reagentInfo.reagentLot)));

    // 使用状态（标红：未放置、无效试剂、不可用）
    QString strUseStatus = ConvertTfEnumToQString(reagent.reagStatusUI.status);
    if (reagent.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
    {
        strUseStatus += QString::number(reagent.reagStatusUI.backupCnt) + "-" + QString::number(reagent.reagStatusUI.backupIndx);
    }
    itemList.push_back(CenterAligmentItem(strUseStatus));
    /*if (::im::tf::StatusForUI::REAGENT_STATUS_CALI_MASK == reagent.reagStatusUI.status)
        SetItemColor(itemList.back(), UI_REAGENT_WARNCOLOR);*/

    // 校准状态（标红：校准失败、未校准）
    //itemList.push_back(GetCalibrateShowStatus(reagentInfo.caliStatus, IsCaliCurveIdValiable(reagentInfo)));
	//itemList.push_back(new QStandardItem(ConvertTfEnumToQString(reagent.caliStatus)));
	// 校准状态(未校准有曲线为空)
    QString strCaliStatus = "";
    if (reagent.caliStatus != -1 && (reagent.caliStatus != ::tf::CaliStatus::CALI_STATUS_NOT || reagent.caliCurveExpInfo.empty()))
    {
        strCaliStatus = ConvertTfEnumToQString(reagent.caliStatus);
    }
    QStandardItem *itemCaliStat = CenterAligmentItem(strCaliStatus);
    if ((reagent.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL) ||
        ((reagent.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT && reagent.caliCurveExpInfo.empty())))
    {
        SetTblItemState(itemCaliStat, STATE_WARNING);
    }
    itemList.push_back(itemCaliStat);

    // 当前曲线有效期（标红：过期）-BUG25815应该标黄
    QString caliCurveExpInfo = QString::fromStdString(reagent.caliCurveExpInfo);
    auto item = CenterAligmentItem(caliCurveExpInfo);
    const auto& splitList = caliCurveExpInfo.split(":");
    if (splitList.size() > 1)
    {
        int days = caliCurveExpInfo.split(":")[1].toInt();
        auto s = boost::posix_time::second_clock::local_time() - TimeStringToPosixTime(reagent.curveExcuteDate);
        int onDays = int32_t(s.hours() / 24);
        if (days == 0)
        {
            days = -1;
        }
        const auto& endExpTime = TimeStringToPosixTime(reagent.curveExcuteDate) + boost::posix_time::hours((onDays+days)*24);
        const auto& strEndExpTime = QString::fromStdString(PosixTimeToTimeString(endExpTime));
        item->setData(strEndExpTime, Qt::UserRole + 10);
    }
    /*if (!DictionaryQueryManager::GetInstance()->IsCaliLineExpire() && caliCurveExpInfo.contains(":"))
        caliCurveExpInfo = caliCurveExpInfo.mid(0, caliCurveExpInfo.indexOf(":"));*/
    itemList.push_back(item);
    

    
    // 备注:2024/06/27 根据免疫Mantis29845,和周晓锋确认，只是不显示红色颜色，不是不显示过期天数
    if (DictionaryQueryManager::GetInstance()->IsCaliLineExpire() &&
        !itemList.back()->text().isEmpty() &&
        itemList.back()->text().split(':').size() > 1 &&
        itemList.back()->text().split(':')[1].toInt() <= 0)
    {
        SetTblItemState(itemList.back(), STATE_NOTICE);
    }
    // 设置一个用户数据，用于排序
    auto&& list = itemList.back()->text().split(":");
    if (list.size() == 2)
    {
        QString sortData;
        // 先补充至10位数值，不足的部分补0
        sortData = list[0] + list[1].rightJustified(10, '0');
        itemList.back()->setData(sortData, Qt::UserRole + 3);
    }
    else
        itemList.back()->setData(itemList.back()->text(), Qt::UserRole + 3);

    // 上机时间
    itemList.push_back(CenterAligmentItem(QString::fromStdString(reagent.registerTime)));
    // 根据上机时间判断是否校准过期
    if (IsCaliExpired(reagent))
    {
        SetTblItemState(itemList.back(), STATE_NOTICE);
    }

    // 校准原因
    itemList.push_back(SetItemTextAligCenter(GetCaliBrateReason(reagent.caliReason)));

    // 执行方法
    itemList.push_back(CenterAligmentItem(tr("两点")));

    // 设置选择与否
    QStandardItem* IsSelectedItem = CenterAligmentItem("");

    // 是否选择校准
    if (reagentInfo.selCali)
    {
        IsSelectedItem->setData(true);
    }
    else
    {
        IsSelectedItem->setData(false);
    }

    if ((reagent.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_CALI_MASK) 
        || (reagent.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_MASK))
    {
        for (int i = 0; i < SelectedColumn; ++i)
        {
            //itemList[i]->setForeground(Qt::gray);
            itemList[i]->setData(QColor(Qt::gray), Qt::UserRole + 1);
        }
    }

    itemList.push_back(IsSelectedItem);

    SetRowItemData(reagentInfo.deviceSN, itemList, pos, reagentInfo.reagentPos, addAction);
}

///
/// @brief 
///
/// @param[in]  devSn  试剂的设备序列号
/// @param[in]  itemList  数据列表
/// @param[in]  rowIndex  需要写入表中的行（插入模式此数据无效需重新计算
/// @param[in]  regeantPos  试剂盘中的位置
/// @param[in]  action  true表示插入数据,false表示覆盖
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月24日，新建函数
///
void QCalibrateOverView::SetRowItemData(const std::string& devSn, const QList<QStandardItem*>& itemList, int rowIndex, int reagentPos, bool action)
{
    // 判断筛选条件
    if (!IsShowLineByFilter(itemList) || itemList.isEmpty())
    {
        return;
    }
    // 插入模式（插入的时候不能直接插入，按照位置顺序插入）
    if (action)
    {
        bool insert = false;	//是否插入成功
        int devMaxPos = -1;		//记录当前设备的最大位置号
        int rowCnt = m_caliBrateMode->rowCount();

        for (int i = 0; i < rowCnt && !insert; ++i)
        {
            tf::AssayClassify::type classify = GetItemClassify(i);
			
            switch (classify)
            {
            case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
            {
                // 获取试剂组详细信息
                boost::optional<im::tf::ReagentInfoTable> group = GetItemUserDataInfo<im::tf::ReagentInfoTable>(i, ReagentInfo);

                if (group && group->deviceSN == devSn)
                {
                    // 因为需求是按顺序显示信息，则一直向后
                    devMaxPos = i;
                    if (group->reagentPos > reagentPos)
                    {
                        m_caliBrateMode->insertRow(i, itemList);
                        insert = true;
                    }
                }
            }
            break;
            case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
            {
                // 获取试剂组详细信息
                boost::optional<ch::tf::ReagentGroup> group = GetItemUserDataInfo<ch::tf::ReagentGroup>(i, ReagentInfo);

                if (group && group->deviceSN == devSn)
                {
                    // 因为需求是按顺序显示信息，则一直向后
                    devMaxPos = i;
                    if (group->posInfo.pos > reagentPos)
                    {
                        m_caliBrateMode->insertRow(i, itemList);
                        insert = true;
                    }
                }
            }
            break;
            case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
                break;
            default:
                break;
            }
        }

        // 处理未成功插入的情况
        if (!insert)
        {
            // 如果比当前设备最大已知的最大位置还要大，则插入到该记录后方
            if (devMaxPos >= 0)
                m_caliBrateMode->insertRow(devMaxPos + 1, itemList);
            // 如果当前设备没有任何试剂信息，则直接放到列表最后
            else
            {
                int column = 0;
                for (const auto& item : itemList)
                {
                   m_caliBrateMode->setItem(rowIndex, column++, item);
                }
            }
        }
    }
    // 覆盖模式
    else
    {
        int column = 0;
        for (const auto& item : itemList)
        {
            m_caliBrateMode->setItem(rowIndex, column++, item);
        }
    }
}

///
/// @brief 确认当前试剂信息在表格中的写入方式
///
/// @param[in]  pos  试剂位置
/// @param[in]  deviceSn  设备序列号
/// @param[in]  maxPos  保存的最大位置
/// @param[in]  devList  已经出现过的设备信息
///
/// @return true表示需要插入数据，false表示覆盖数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月24日，新建函数
///
bool QCalibrateOverView::GetInsertMode(int pos, const std::string & deviceSn, int & maxPos, QVector<std::string>& devList)
{
    int devIndex = devList.indexOf(deviceSn);

    // 如果没有此设备的数据，直接插入数据到表格结尾
    if (devIndex == -1)
    {
        devList.push_back(deviceSn);

        // 最后一个设备的当前试剂组的最大位置号
        maxPos = pos;

        return false;
    }
    // 如果是按照设备名称 且 按位置号升序连续展示数据，则数据直接放入到末尾
    else if (devList.back() == deviceSn && maxPos <= pos)
    {
        // 最后一个设备的当前试剂组的最大位置号
        maxPos = pos;

        return false;
    }

    return true;
}

///
/// @brief
///     校准申请对话框显示
///
/// @param[in]  index  索引
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCalibrateOverView::OnShowAppDialog()
{
    QModelIndex selectedIndex = ui->tableView->currentIndex();
    if (!selectedIndex.isValid() || m_caliBrateMode->item(selectedIndex.row(), 0)->text().isEmpty())
    {
        return;
    }

    if (m_CaliAppDialog == nullptr)
    {
        m_CaliAppDialog = new QCaliBrateMethodApp(this);
        connect(m_CaliAppDialog, SIGNAL(CaliBrateMethod(int, bool)), this, SLOT(OnChangeCaliMethod(int, bool)));
    }

    // 设置校准方法默认值
    m_CaliAppDialog->SetMethod(tf::CaliMode::CALI_MODE_FULLDOT, m_blankMethodEnable);
    m_CaliAppDialog->show();

}

///
/// @brief 校准屏蔽解除
///
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月6日，新建函数
///
void QCalibrateOverView::UnMasckCali()
{
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("解除校准屏蔽"), tr("确定解除校准屏蔽?"), TipDlgType::TWO_BUTTON));
    if (pTipDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel unmask calibrate!");
        return;
    }

    const QModelIndexList& selList = ui->tableView->selectionModel()->selectedRows();
    // 查询选中项目的耗材信息
    std::vector< ::ch::tf::ReagentGroupQueryCond> siqc;
    std:vector< ::ise::tf::IseModuleInfo> iseModuleInfos;
    // 
    for (const auto& li : selList)
    {
        tf::AssayClassify::type classify = GetItemClassify(li.row());

        switch (classify)
        {
        case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
        {
            // 获取缓存试剂信息
            boost::optional<im::tf::ReagentInfoTable> stuRgntInfo = GetItemUserDataInfo<im::tf::ReagentInfoTable>(li.row(), ReagentInfo);

            // 更新数据库
            ::tf::ResultLong _return;
            if (stuRgntInfo && im::i6000::LogicControlProxy::ClearReagentCaliMask(_return, *stuRgntInfo) && _return.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                // 如果解除校准屏蔽后，不会通知客户端，则此处可能需要先客户端手动获取更新一次（测试后再补充逻辑）
                ShowImReagentStatusItem(false, li.row(), *stuRgntInfo);
                SetPairedInfo(*stuRgntInfo);
            }
        }
        break;
        case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
        {
            // 如果校准被屏蔽了
            boost::optional<::ch::tf::ReagentGroup> reagentInfo = GetItemUserDataInfo<::ch::tf::ReagentGroup>(li.row(), ReagentInfo);

            if (reagentInfo && reagentInfo->caliMask)
            {
                ch::tf::ReagentGroupQueryCond qryCond;
                // 位置信息
                qryCond.__set_posInfo(reagentInfo->posInfo);
                // 设备编号
                qryCond.__set_deviceSN(std::vector<string>{reagentInfo->deviceSN});
                // 项目编号
                qryCond.__set_assayCode(reagentInfo->assayCode);
                // 耗材编号
                qryCond.__set_suppliesCode(reagentInfo->suppliesCode);

                siqc.push_back(qryCond);
            }
        }
        break;
        case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        {
            // 如果校准被屏蔽了
            boost::optional<::ise::tf::IseModuleInfo> reagentInfo = GetItemUserDataInfo<::ise::tf::IseModuleInfo>(li.row(), ReagentInfo);

            if (reagentInfo && reagentInfo->caliMasked)
            {
                ise::tf::IseModuleInfo newModuleInfo;
                newModuleInfo.__set_id(reagentInfo->id);
                // 设置为不屏蔽
                newModuleInfo.__set_caliMasked(false);
                if (ise::LogicControlProxy::ModifyIseModuleInfo(newModuleInfo))
                {
                    iseModuleInfos.push_back(*reagentInfo);
                }

            }
        }
            break;
        default:
            break;
        }
    }

    if (!siqc.empty())
    {
        if (!ch::c1005::LogicControlProxy::UnMaskCalibrate(siqc))
        {
            ULOG(LOG_ERROR, "UnMaskReagentGroups() failed");
            return;
        }
    }

    if (!iseModuleInfos.empty())
        m_iseCaliAppManager->UpdateCaliStatus(iseModuleInfos);

    // 更新按钮状态
    OnSelectionChanged(QItemSelection(), QItemSelection());
}

///
/// @brief 执行选择功能
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月25日，新建函数
///
void QCalibrateOverView::OnSelectBtn()
{
    const QModelIndexList& selList = ui->tableView->selectionModel()->selectedRows();
    auto selsec = ui->tableView->selectionModel()->selection();

    // 如果没有选择任何行，直接返回
    if (selList.empty())
    {
        return;
    }
    else
    {
        std::list<int32_t> lstRows;
        // 依次修改对应的行的校准方法
        for (const auto& selectIndex : selList)
        {
            // 设置选择状态
            m_caliBrateMode->item(selectIndex.row(), SelectedColumn)->setData(m_selItem, Qt::UserRole + 1);
            
            // 更新数据
            UpdateCaliSelect(selectIndex.row()); 
            lstRows.push_front(selectIndex.row());
        }
        
        // 选区不变，操作方式取反
        m_selItem = !m_selItem;

        emit sigSelBtnStateChanged(m_selItem);

        // bug18501,17954
        for (auto row:lstRows)
        {
            QItemSelection selctionItem(m_caliBrateMode->index(row, 0), m_caliBrateMode->index(row, 1));
            ui->tableView->selectionModel()->select(selctionItem, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        SetTableColumnFixedWidth();
    }
}

///
/// @brief 显示反应曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月19日，新建函数
///
void QCalibrateOverView::OnShowReactCurve()
{
    int firstIndex = GetFirstIndex();
    tf::AssayClassify::type classify = GetItemClassify(firstIndex);

    switch (classify)
    {
    // 免疫没有反应曲线
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
        break;
    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
    {
        boost::optional<ch::tf::ReagentGroup> reagent = GetItemUserDataInfo<ch::tf::ReagentGroup>(firstIndex, ReagentInfo);

        if (reagent)
        {
            ch::tf::CaliCurveQueryCond curveCond;
            curveCond.__set_id(reagent->caliCurveId);
            const auto& curve = CaliBrateCommom::GetCurvesByCond(std::vector<ch::tf::CaliCurveQueryCond>({ curveCond }));

            if (m_reactionDialog == nullptr)
            {
                m_reactionDialog = new QReactionCurve(this);
            }

            // 如果没有数据，则显示空白界面
            if (curve.empty())
            {
                m_reactionDialog->ShowCurveList(nullptr);
            }
            else
            {
                m_reactionDialog->ShowCurveList(curve[0]);
            }
        }
    }
    break;
    case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        return;
        break;
    default:
        return;
        break;
    }

    m_reactionDialog->show();
}

///
/// @brief 显示校准结果
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCalibrateOverView::OnShowCalibrateResult()
{
    int firstRow = GetFirstIndex();

    switch (GetItemClassify(firstRow))
    {
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
    {
        if (m_imCaliRestDlg == nullptr)
            m_imCaliRestDlg = new ImQCaliBrateResult(this);

        boost::optional<im::tf::ReagentInfoTable> reagent = GetItemUserDataInfo<im::tf::ReagentInfoTable>(firstRow, ReagentInfo);

        if (reagent)
        {
            m_imCaliRestDlg->StartDialog(*reagent);
            m_imCaliRestDlg->show();
        }
    }
    break;
    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
    {
        if (m_calibrateResultDialog == nullptr)
        {
            m_calibrateResultDialog = new QCaliBrateResult(this);
        }

        boost::optional<ch::tf::ReagentGroup> group = GetItemUserDataInfo<ch::tf::ReagentGroup>(firstRow, ReagentInfo);

        if (group)
        {
            m_calibrateResultDialog->StartDialog(*group);
            m_calibrateResultDialog->show();
        }
    }
    break;
    case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
    {
        if (m_iseCaliRestDlg == nullptr)
        {
            m_iseCaliRestDlg = new QIseCaliBrateResult(this);
        }

        boost::optional<ise::tf::IseModuleInfo> module = GetItemUserDataInfo<ise::tf::IseModuleInfo>(firstRow, ReagentInfo);

        if (module)
        {
            m_iseCaliRestDlg->StartDialog(*module);
            m_iseCaliRestDlg->show();
        }
    }
        break;
    default:
        break;
    }
}

///
/// @brief 显示校准系数编辑对话框
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
void QCalibrateOverView::OnShowCaliFactorEditDialog()
{
	int firstRow = GetFirstIndex();

	switch (GetItemClassify(firstRow))
	{
	case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
	{
		/*********免疫待补充***********/
	}
	break;
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
	{
		
		if (m_caliFactorEditDlg == nullptr)
		{
			m_caliFactorEditDlg = new QCalibrationFactorEdit(this);
		}
		boost::optional<ch::tf::ReagentGroup> group = GetItemUserDataInfo<ch::tf::ReagentGroup>(firstRow, ReagentInfo);
		if (group)
		{
			if (m_caliFactorEditDlg->initDialog(firstRow, *group)) {
				m_caliFactorEditDlg->show();
			}
		}
	}
	break;
	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
		break;
	default:
		break;
	}

}

void QCalibrateOverView::OnShowCaliRackOverviewDlg()
{
    if (m_caliRackOverviewDlg == nullptr)
    {
        return;
    }

    m_caliRackOverviewDlg->show();
    //m_caliRackOverviewDlg->RefreshData();
}

///
/// @brief
///     改变方法
///
/// @param[in] method	    方法 
/// @param[in] isSelected   是否选择 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QCalibrateOverView::OnChangeCaliMethod(int method, bool isSelected)
{
    Q_UNUSED(isSelected);
    bool caliDoing = true;  // 修改项目是否全在校准中 true:全在校准中  false:非全在校准中
    bool allIse = true;     // 是否全是ise，全ISE时，不用修改执行方法，所以不需要提示正在校准

    // 依次修改对应的行的校准方法
    for (const auto& selectIndex : ui->tableView->selectionModel()->selectedRows())
    {
        tf::AssayClassify::type classify = GetItemClassify(selectIndex.row());

        switch (classify)
        {
        case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
            // 免疫只有全点，所以不修改
        {
            // 获取缓存试剂信息
            boost::optional<im::tf::ReagentInfoTable> stuRgntInfo = GetItemUserDataInfo<im::tf::ReagentInfoTable>(selectIndex.row(), ReagentInfo);
            allIse = false;
            
            // 如果正在校准，则不能修改执行方法
            if (stuRgntInfo && stuRgntInfo->caliStatus == tf::CaliStatus::CALI_STATUS_DOING)
                continue;

            caliDoing = false;

            // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑
                // 2023/8/23 最新需求，执行方法不变更校准原因
            /*if (stuRgntInfo && (stuRgntInfo->caliReason == tf::CaliReason::CALI_REASON_NONE))
                stuRgntInfo->__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);*/
            // 更新数据库
            ::tf::ResultLong _return;
            im::i6000::LogicControlProxy::SetReagentCaliSel(_return, *stuRgntInfo);
            ShowImReagentStatusItem(false, selectIndex.row(), *stuRgntInfo);
            SetPairedInfo(*stuRgntInfo);
        }
        break;
        case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
        {
            boost::optional<ch::tf::ReagentGroup> reagent = GetItemUserDataInfo<ch::tf::ReagentGroup>(selectIndex.row(), ReagentInfo);
            allIse = false;

            // 如果正在校准，则不能修改执行方法
            if (reagent && reagent->caliStatus == tf::CaliStatus::CALI_STATUS_DOING)
                continue;

            caliDoing = false;

            if (reagent)
            {
				// 当没有校准曲线，并且需要更新为空白时,不更新
				switch (method)
				{
				case tf::CaliMode::CALI_MODE_BLANK:{

					if (!isContansCaliCurve(reagent))	continue;

				}	break;
				default:
					break;
				}
                ch::tf::ReagentGroup newGroup;
                newGroup.__set_id(reagent->id);
                newGroup.__set_caliMode(static_cast<tf::CaliMode::type>(method));
    
                // 修改了校准方法以后，一律设置为手动校准
                // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑，上一行注释逻辑不生效
                // 2023/8/23 最新需求，执行方法不变更校准原因
                /*if(reagent->caliReason == tf::CaliReason::CALI_REASON_NONE)
                    reagent->__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);*/

                // 修改数据
                ch::c1005::LogicControlProxy::ModifyReagentGroup(newGroup);
            }
        }
        break;
        case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        {
            // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑
            // 2023/8/23 最新需求，执行方法不变更校准原因
            /*if (reagent && (reagent->caliReason == tf::CaliReason::CALI_REASON_NONE))
                reagent->__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);*/
            // 更新数据库
            //ise::LogicControlProxy::ModifyIseModuleInfo(*reagent);
        }
            break;
        default:
            continue;
            break;
        }
    }

    if (caliDoing && !allIse)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("正在进行校准，不能修改执行方法。"), TipDlgType::SINGLE_BUTTON));
        pTipDlg->exec();
    } 
}

///
/// @brief 校准文档杯选择与否
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月15日，新建函数
///
void QCalibrateOverView::OnCaliDocSelected(const QModelIndex& index)
{
    // 1: 有效 2： 不能范围外 3： 必须是选择项目对应的列
    if (!index.isValid() || !m_caliBrateMode->item(index.row()) || m_caliBrateMode->item(index.row())->text().isEmpty())
    {
        return;
    }
    int row = index.row();
    if (index.column() == SelectedColumn)
    {
        UpdateCaliSelect(index.row());
    }

    SetTableColumnFixedWidth();
    // 选中行-BUG21570(注释此行代码，因此行代码会造成不能Ctrl多选)
    // ui->tableView->selectRow(row);
}

///
/// @brief 表格选择变化消息处理
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月31日，新建函数
///
void QCalibrateOverView::OnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    FUNC_ENTRY_LOG();
    Q_UNUSED(selected); Q_UNUSED(deselected);
   
    const QModelIndexList& selList = ui->tableView->selectionModel()->selectedRows();
    if (selList.empty())
    {
        ULOG(LOG_INFO, "Selection is empty!");
        UpdateButtonStatus(false, false, false, false, false, false);
        return;
    }
    
    bool caliMask = true;           // 解除校准屏蔽按钮是否可用
    bool reactionCurve = true;      // 反应曲线是否可用
    bool modifyMethod = false;      // 是否可以修改执行方法
	bool caliCurveEdit = true;      // 校准系数编辑是否可用
    m_selItem = false;              // 显示为选择/取消选择
	m_blankMethodEnable = false;    // 校准方法中，空白校准是否可用，默认不可用

    for (const auto& li : selList)
    {
        if (li.isValid() && li.row() >= 0)
        {
            tf::AssayClassify::type classify = GetItemClassify(li.row());

            switch (classify)
            {
            case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
            {
                boost::optional<im::tf::ReagentInfoTable> reagent = GetItemUserDataInfo<im::tf::ReagentInfoTable>(li.row(), ReagentInfo);
                if (!reagent || reagent->caliSheld != "1")
                {
                    // 设置校准屏蔽解除按钮是否可用
                    caliMask = false;
                }
                // 执行方法可用
                if (reagent && reagent->caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
                {
                    modifyMethod = true;
                }
            }
            break;
            case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
            {
                boost::optional<ch::tf::ReagentGroup> reagent = GetItemUserDataInfo<ch::tf::ReagentGroup>(li.row(), ReagentInfo);
                // 禅道缺陷2108，试剂屏蔽和校准屏蔽同时存在时，不能取消校准屏蔽
                if (!reagent || reagent->reagentMask || !reagent->caliMask)
                {
                    // 设置校准屏蔽解除按钮是否可用
                    caliMask = false;
                }

				// 因为免疫和ISE只有全点，当生化有校准曲线时，就可选择空白
				if (reagent->caliCurveId >= 0)
					m_blankMethodEnable = true;

                // 执行方法可用
                if (reagent && reagent->caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
                {
                    modifyMethod = true;
                }
            }
            break;
            case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
            {
                boost::optional<ise::tf::IseModuleInfo> reagent = GetItemUserDataInfo<ise::tf::IseModuleInfo>(li.row(), ReagentInfo);
                if (!reagent || !reagent->caliMasked)
                {
                    // 设置校准屏蔽解除按钮是否可用
                    caliMask = false;
                }

                // 执行方法可用
                if (reagent && reagent->caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
                {
                    modifyMethod = true;
                }
            }
                break;
            default:
                break;
            }

            // 如果有一项没有选择，则记录执行选择时，先执行选择操作
            if (m_caliBrateMode->item(li.row(), SelectedColumn) && !m_caliBrateMode->item(li.row(), SelectedColumn)->data(Qt::UserRole + 1).toBool())
            {
                m_selItem = true;
            }
        }
    }

    // 因为前面出现过非校准屏蔽项，已经设置控件并返回，此处可以直接
    tf::AssayClassify::type type = GetItemClassify(GetFirstIndex());
    if (type == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE || type == tf::AssayClassify::ASSAY_CLASSIFY_ISE) {
        reactionCurve = false;
        caliCurveEdit = false;
    }

    emit sigSelBtnStateChanged(m_selItem);

    UpdateButtonStatus(true, modifyMethod, true, reactionCurve, caliMask, caliCurveEdit);
}

///
/// @brief 更新试剂的校准状态信息
///  
/// @param[in]  supplyUpdates  位置信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月20日，新建函数
///
void QCalibrateOverView::UpdateCaliSatus(class std::vector<class ch::tf::SupplyUpdate, class std::allocator<class ch::tf::SupplyUpdate>> supplyUpdates)
{
    ULOG(LOG_INFO, "%s(size:%d)", __FUNCTION__, supplyUpdates.size());

    // 对应的模块为空
    if (m_caliBrateMode == nullptr || !isVisible())
    {
        m_needUpdate = true;
        ULOG(LOG_INFO, "%s needUpdate.", __FUNCTION__);
        return;
    }
    bool resetSearchList = false;
    for (const auto& supplyUpdate : supplyUpdates)
    {
        // 打印的信息
        ULOG(LOG_INFO, "supplyUpdate devSn:%s, pos:%d.", supplyUpdate.deviceSN.c_str(), supplyUpdate.posInfo.pos);

        // 是否属于刷新范围(只需要查询生化的仪器),当前设备未选择显示，则不需要更新
        if (m_devs.find(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) == m_devs.end() ||
            m_devs[tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY].end() == std::find(m_devs[tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY].begin(), m_devs[tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY].end(), supplyUpdate.deviceSN))
        {
            // 打印的信息
            ULOG(LOG_INFO, "There is not chemistry device or device is not selected, deviceSn is %s.", supplyUpdate.deviceSN);
            continue;
        }

        const auto& rgtSupplies = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(supplyUpdate.deviceSN, { supplyUpdate.posInfo.pos });

        // 若找不到，则删除之前的行
        if (rgtSupplies.empty())
        {
            std::vector<int> rows = GetRowByPostion(supplyUpdate.posInfo.pos, supplyUpdate.deviceSN);
            // 从大到小来排序
            std::sort(rows.begin(), rows.end(), std::greater<int>());
            for (auto row : rows)
            {
                if (row == -1)
                {
                    ULOG(LOG_ERROR, "invaid row!");
                    continue;
                }

                const auto& info = GetItemUserDataInfo<ch::tf::ReagentGroup>(row, ReagentInfo);
                if (info)
                {
                    if (RmAssayFromSearchList(info->assayCode))
                    {
                        resetSearchList = true;
                    }
                }

                // 删除对应的行
                m_caliBrateMode->removeRow(row);
            }
        }

        for (const auto& supply : rgtSupplies)
        {
            // 跳过未放置的
            if (supply.second.supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_UNLOAD)
                continue;

            if (supply.second.reagentInfos.empty())
            {
                ULOG(LOG_ERROR, "Query ReagentGroup() failed");
                continue;
            }

            std::vector<int> rows = GetRowByPostion(supplyUpdate.posInfo.pos, supplyUpdate.deviceSN);
            // 没有找到对应的行，添加新的行
            if (rows.empty())
            {
                for (const ch::tf::ReagentGroup& reagent : supply.second.reagentInfos)
                {
                    ULOG(LOG_INFO, "reagent assayCode:%d, id:%lld.", reagent.assayCode, reagent.id);
                    if (AddAssayToSearchList(reagent.assayCode))
                        resetSearchList = true;

                    ShowReagentStatusItem(true, m_caliBrateMode->rowCount(), reagent);
                }
                continue;
            }

            // 存在的时候，表示刷新状态
            if (supply.second.reagentInfos.size() != rows.size())
            {
                DoChangeAssays(supply.second.reagentInfos, rows);
                continue;
            }

            // 依次更新每一行
            int i = 0;
            for (const ch::tf::ReagentGroup& reagent : supply.second.reagentInfos)
            {
                for(auto row : rows)
                {
                    if (row == -1)
                    {
                        continue;
                    }

                    boost::optional<ch::tf::ReagentGroup> r = GetItemUserDataInfo<ch::tf::ReagentGroup>(row, ReagentInfo);
                    if (r && r->assayCode == reagent.assayCode)
                    {
                        ShowReagentStatusItem(false, row, reagent);
                    }
                }
            }
        }
    }

    if (resetSearchList)
        SetSearchList();

    //刷新按钮可用状态(此处选中行会丢失，所以不用计算多选清空）
    OnSelectionChanged(QItemSelection(), QItemSelection());

    SetTableColumnFixedWidth();
}

///
/// @brief 更新电解质的校准状态信息
///  
/// @param[in]  iseAppUpdates  更新信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月30日，新建函数
///
void QCalibrateOverView::UpdateCaliAppSatus(std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates)
{
    m_iseCaliAppManager->UpdateCaliStatus(iseAppUpdates); 
    OnSelectionChanged(QItemSelection(), QItemSelection());
}

///
/// @brief
///     更新单条校准申请信息
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void QCalibrateOverView::UpdateSingleCaliReqInfo(const im::tf::ReagentInfoTable & stuRgntInfo, const im::tf::ChangeType::type type)
{
    // 如果未显示，则标志更新后返回
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    // 是否属于刷新范围(只需要查询免疫的仪器),当前设备未选择显示，则不需要更新
    if (m_devs.find(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE) != m_devs.end() &&
        m_devs[tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE].end() == std::find(m_devs[tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE].begin(), m_devs[tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE].end(), stuRgntInfo.deviceSN))
    {
        return;
    }
	// 旧逻辑：如果批号序列号为空，则表明扫描失败
    // 新逻辑：如果项目编号小于等于0，则表明扫描失败（因bug17830变更）
	if (stuRgntInfo.assayCode <= 0)
	{
		return;
	}

	::im::tf::ReagentInfoTableQueryResp _returnRsp;
	::im::tf::ReagTableUIQueryCond diq;
    // 特殊处理已经被卸载或者别的原因造成的位置不正确的试剂
    //if (stuRgntInfo.reagentPos < 1 || stuRgntInfo.reagentPos > ::im::i6000::tf::g_i6000_constants.REAGENT_DISK_SLOT)
    {
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntInfo.assayCode);
        if (!spAssay)
            return;

        // 获取该设备的所有该通道号的试剂，全部清除，重新设置主备
        QString strRegExp = QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(stuRgntInfo.deviceSN)) + "-";
        
        QList<QStandardItem*> itemsByDev = m_caliBrateMode->findItems(strRegExp, Qt::MatchStartsWith);
        QList<QStandardItem*> itemsByName = m_caliBrateMode->findItems(QString::fromStdString(spAssay->assayName), Qt::MatchExactly, 1);

        std::vector<int> stdItemsByDev;
        std::vector<int> stdItemsByName;
        std::transform(itemsByDev.begin(), itemsByDev.end(), std::back_inserter(stdItemsByDev), [](auto& item)->int {return item->row(); });
        std::transform(itemsByName.begin(), itemsByName.end(), std::back_inserter(stdItemsByName), [](auto& item)->int {return item->row(); });


        std::vector<int> uniqueRows;
        uniqueRows.resize(min(stdItemsByDev.size(), stdItemsByName.size()));
        auto & end = std::set_intersection(stdItemsByDev.begin(), stdItemsByDev.end(), stdItemsByName.begin(), stdItemsByName.end(), uniqueRows.begin());

        // 从大到小来排序
        std::sort(uniqueRows.begin(), end, std::greater<int>());
        auto it = uniqueRows.begin();
        while (it != end)
        {
            if (*it >= 0)
            {
                RemovePairedReagentsByRow(*it);
                // 删除对应的行
                m_caliBrateMode->removeRow(*it);
            }
            ++it;
        }
    }

        diq.__set_deviceSNs(std::vector<std::string>({ stuRgntInfo.deviceSN }));
		diq.__set_assayCode(stuRgntInfo.assayCode);
        if (!im::LogicControlProxy::QueryReagentInfoForUI(_returnRsp, diq) &&
            _returnRsp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            return;

        if (_returnRsp.lstReagentInfos.empty())
        {
            if(RmAssayFromSearchList(stuRgntInfo.assayCode))
                SetSearchList();

            SetTableColumnFixedWidth();
            return;
        }

    // 预处理试剂列表
    std::vector<im::tf::ReagentInfoTable> preDealReagents;
    bool resetSearchList = false;

    // 更新信息
    for (const im::tf::ReagentInfoTable& reagent : _returnRsp.lstReagentInfos)
    {
        // 保存试剂配对信息，如果不存在配对信息，会直接返回
        StorePairedReagent(reagent);
        if (AddAssayToSearchList(reagent.assayCode))
            resetSearchList = true;

		if (reagent.bottleIdx == 1)
		{
            preDealReagents.push_back(reagent);
			continue;
		}

            ShowImReagentStatusItem(true, m_caliBrateMode->rowCount(), reagent,false);
    }

    //处理bottleIdx为1的配对信息
    for (const auto& reagent : preDealReagents)
    {
        if (AddAssayToSearchList(reagent.assayCode))
            resetSearchList = true;

        std::vector<int> rows = GetRowByPostion(reagent.reagentPos, reagent.deviceSN);
        // 没有找到对应的行，添加新的行
        if (rows.empty())
        {
            ShowImReagentStatusItem(true, m_caliBrateMode->rowCount(), reagent, false);
            continue;
        }

        // 循环覆盖，原则上同一位置不会有多个（不会出现类似双向同侧的情况）
        for (const auto& i : rows)
        {
            ShowImReagentStatusItem(false, i, reagent, false);
        }
    }

    if (resetSearchList)
    {
        SetSearchList();
    }

    SetTableColumnFixedWidth();
    OnSelectionChanged(QItemSelection(), QItemSelection());
}

///
/// @brief
///     更新单条校准申请信息
///
/// @param[in]  reagentInfo  试剂信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月1日，新建函数
///
void QCalibrateOverView::UpdateSingleReagentGroup(const ch::tf::ReagentGroup & reagentInfo)
{
    ch::tf::SupplyUpdate supplyUpdate;
    supplyUpdate.posInfo = reagentInfo.posInfo;
    supplyUpdate.deviceSN = reagentInfo.deviceSN;

    UpdateCaliSatus(std::vector<ch::tf::SupplyUpdate>{supplyUpdate});
}

///
/// @brief 更新校准选择状态
///
/// @param[in]  row  触发行的索引
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月1日，新建函数
///
void QCalibrateOverView::UpdateCaliSelect(int row)
{
    // 获取当前选择状态
    bool IsSelected =  m_caliBrateMode->item(row, SelectedColumn)->data().toBool();
    tf::AssayClassify::type classify = GetItemClassify(row);

    switch (classify)
    {
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
    {
        boost::optional<im::tf::ReagentInfoTable> reagent = GetItemUserDataInfo<im::tf::ReagentInfoTable>(row, ReagentInfo);

        if (reagent)
        {
            // 没变化不需要修改
            if ((reagent->selCali > 0) == IsSelected)
            {
                break;
            }
            // 设置选择状态
            reagent->__set_selCali(IsSelected);
            // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑，上一行注释逻辑不生效
            if (IsSelected && reagent->caliReason == tf::CaliReason::CALI_REASON_NONE)
                reagent->__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);
            // 新确认逻辑，只要取消勾选，校准原因都清空，不需要判断原有校准原因 2023/1/5
            else if (!IsSelected /*&& reagent->caliReason == tf::CaliReason::CALI_REASON_MANUAL*/)
                reagent->__set_caliReason(tf::CaliReason::CALI_REASON_NONE);

            // 更新数据库
            ::tf::ResultLong _return;
            if (!im::i6000::LogicControlProxy::SetReagentCaliSel(_return, *reagent) || _return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "failed change reagentInfo cali select.");
                // 获取缓存试剂信息
                m_caliBrateMode->item(row, SelectedColumn)->setData(!IsSelected, Qt::UserRole + 1);
            }
            else
            {
                ShowImReagentStatusItem(false, row, *reagent);
                SetPairedInfo(*reagent);
            }
        }
    }
    break;
    // 生化项目
    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
    {
        boost::optional<ch::tf::ReagentGroup> reagent = GetItemUserDataInfo<ch::tf::ReagentGroup>(row, ReagentInfo);

        if (reagent)
        {
            if (reagent->caliSelect  == IsSelected)
            {
                break;
            }
            ch::tf::ReagentGroup newReagent;
            newReagent.__set_id(reagent->id);
            // 设置选择状态
            newReagent.__set_caliSelect(IsSelected);
            // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑，上一行注释逻辑不生效
            if (IsSelected && reagent->caliReason == tf::CaliReason::CALI_REASON_NONE)
                newReagent.__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);
            // 新确认逻辑，只要取消勾选，校准原因都清空，不需要判断原有校准原因 2023/1/5
            else if (!IsSelected /*&& reagent->caliReason == tf::CaliReason::CALI_REASON_MANUAL*/)
                newReagent.__set_caliReason(tf::CaliReason::CALI_REASON_NONE);

            if (!ch::c1005::LogicControlProxy::ModifyReagentGroup(newReagent))
            {
                m_caliBrateMode->item(row, SelectedColumn)->setData(!IsSelected, Qt::UserRole + 1);
            }
        }
    }
    break;
    case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
    {
        boost::optional<ise::tf::IseModuleInfo> reagent = GetItemUserDataInfo<ise::tf::IseModuleInfo>(row, ReagentInfo);

        if (reagent)
        {
            if (reagent->caliSelect == IsSelected)
            {
                break;
            }
            ise::tf::IseModuleInfo newModuleInfo;

            newModuleInfo.__set_id(reagent->id);
            // 设置选择状态
            newModuleInfo.__set_caliSelect(IsSelected);
            // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑，上一行注释逻辑不生效
            if (IsSelected && reagent->caliReason == tf::CaliReason::CALI_REASON_NONE)
                newModuleInfo.__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);
            // 新确认逻辑，只要取消勾选，校准原因都清空，不需要判断原有校准原因 2023/1/5
            else if (!IsSelected /*&& reagent->caliReason == tf::CaliReason::CALI_REASON_MANUAL*/)
                newModuleInfo.__set_caliReason(tf::CaliReason::CALI_REASON_NONE);

            if (!ise::LogicControlProxy::ModifyIseModuleInfo(newModuleInfo))
            {
                m_caliBrateMode->item(row, SelectedColumn)->setData(!IsSelected, Qt::UserRole + 1);
            }
        }
    }
        break;
    default:
        break;
    }
}

///
/// @brief 执行方法双击切换
///
/// @param[in]  index  索引
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月9日，新建函数
///
void QCalibrateOverView::OnCaliModeChanged(const QModelIndex& index)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 1: 有效 2：执行方法列 
	if (!index.isValid() || index.column() != CaliMethodColumn)		return;
	
	// 开始切换
	tf::AssayClassify::type classify = GetItemClassify(index.row());
	switch (classify)
	{

	// 免疫-执行方法切换
	case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:{
		/**********免疫待补充***********/
	}	break;
	
	// 生化-执行方法切换
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:{
		// 获取当前双击的缓存试剂信息
		boost::optional<ch::tf::ReagentGroup> reagent = GetItemUserDataInfo<ch::tf::ReagentGroup>(index.row(), ReagentInfo);
		
		if (reagent){
            // 如果正在进行校准，则不能修改执行方法（关联bug2535，需求中未明确正在校准时，不能修改）
            if (reagent->caliStatus == tf::CaliStatus::CALI_STATUS_DOING)
                return;

			bool modifySucess = false;
			tf::CaliMode::type newMethod = tf::CaliMode::CALI_MODE_NONE;
			switch (reagent->caliMode)
			{
			// 当前为空白，切换为全点 
			case tf::CaliMode::CALI_MODE_BLANK: {
				newMethod = tf::CaliMode::CALI_MODE_FULLDOT;
				modifySucess = true;
			}	break;

			// 当前为全点，切换为空白
			case tf::CaliMode::CALI_MODE_FULLDOT: {

				// 检查试剂校准曲线来源，如果没有，则不能切换为空白
				if (isContansCaliCurve(reagent) && reagent->caliStatus != tf::CaliStatus::CALI_STATUS_DOING) {
					newMethod = tf::CaliMode::CALI_MODE_BLANK;
					modifySucess = true;
				}
				else {
					modifySucess = false;
				}
			}	break;

			// 默认为全点
			default: {
				newMethod = tf::CaliMode::CALI_MODE_FULLDOT;
				modifySucess = true;
			}	break;
				
			}

			// 如果修改成功，则修改数据
			if (modifySucess) {
                ::ch::tf::ReagentGroup newReagent;
                newReagent.__set_id(reagent->id);
                newReagent.__set_caliMode(newMethod);

                // 修改了校准方法以后，一律设置为手动校准
                // 只有在校准原因为none时，才修改校准原因为手动校准2023/1/5新确认逻辑，上一行注释逻辑不生效
                // 2023/8/23 最新需求，执行方法不变更校准原因
                /*if (reagent->caliReason == tf::CaliReason::CALI_REASON_NONE)
                    reagent->__set_caliReason(tf::CaliReason::CALI_REASON_MANUAL);*/

				// 修改数据
				ch::c1005::LogicControlProxy::ModifyReagentGroup(newReagent);
			}
			
		}
	
	}	break;

	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
		break;
	default:
		break;
	} 
}

///
/// @brief 刷新按钮可用状态
///
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月31日，新建函数
///
void QCalibrateOverView::UpdateButtonStatus(bool bSel, bool bApp, bool bResult, bool bReaction, bool bUnMaskButton, bool bCaliFactorEdit)
{
    // 选择按钮
    ui->calibrate_sel_btn->setEnabled(bSel);
    // 执行方法
    ui->caliBrateApp_Button->setEnabled(bApp);
    // 校准结果
    ui->calibrate_result_btn->setEnabled(bResult);
    // 反应曲线
    ui->Reaction_Button->setEnabled(bReaction);
    // 校准屏蔽
    ui->unMaskButton->setEnabled(bUnMaskButton);
	// 校准系数编辑
	ui->caliFactorEdit_btn->setEnabled(bCaliFactorEdit);
}

///
/// @brief 获取制定行的设备分类
///
/// @param[in]  row  行号
///
/// @return 设备类型
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月25日，新建函数
///
tf::AssayClassify::type QCalibrateOverView::GetItemClassify(int row)
{
    if (row < 0 || row > m_caliBrateMode->rowCount())
        return tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

    QStandardItem* item = m_caliBrateMode->item(row, 0);

    if (item == nullptr)
        return tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

    return item->data(Qt::UserRole + AssayClassify).value<tf::AssayClassify::type>();
}

///
/// @brief 获取选择行执行选择按钮时的动作（需要自行判断是否有选择项）
///（选择行中有任意一个没有选择，则执行选择）
///
/// @return true：表示执行选择 false:表示取消选择
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月26日，新建函数
///
bool QCalibrateOverView::GetSelectAction(const QModelIndexList& selList)
{
    for (const auto& li : selList)
    {
        // 如果有一项没有选择，则记录执行选择时，先执行选择操作
        if (!m_caliBrateMode->item(li.row(), SelectedColumn)->data(Qt::UserRole + 1).toBool())
        {
            return true;
        }
    }

    return false;
}

///
/// @brief 处理双项同测的试剂的切换（从双项同测切换到单试剂，或者从单试剂切换到双项目）
///
/// @param[in]  reagentGroups  试剂组
/// @param[in]  rows           行列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月23日，新建函数
///
void QCalibrateOverView::DoChangeAssays(const std::vector<ch::tf::ReagentGroup>& reagentGroups, const std::vector<int>& rows)
{
    // 参数检查
    if (reagentGroups.size() == 0 || reagentGroups.size() == rows.size())
    {
        return;
    }

    // 行数大于试剂数，说明从双项切换到单数
    if (reagentGroups.size() < rows.size())
    {
        ShowReagentStatusItem(false, rows[0], reagentGroups[0]);
        m_caliBrateMode->removeRow(rows[1]);
    }
    // 行数小于试剂数，说明从单项变到双项同测，或者可能处于筛选状态
    else
    {
        bool  doFirst = false;
        for (const ch::tf::ReagentGroup& reagent : reagentGroups)
        {
            auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(reagent.assayCode);
            if (spAssay == nullptr || (!m_filterAssayName.isEmpty() && QString::fromStdString(spAssay->assayName) != m_filterAssayName))
                continue;

            if (!doFirst)
            {
                ShowReagentStatusItem(false, rows[0], reagent);
                doFirst = true;
            }
            else
            {
                ShowReagentStatusItem(true, rows[0], reagent);
            }
        }
    }
}

///
/// @brief 根据位置获取行号
///
/// @param[in]  postion  位置
/// @param[in]  devicesn 设备编号
///
/// @return 行号
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
std::vector<int> QCalibrateOverView::GetRowByPostion(int postion, const std::string& devicesn)
{
    std::vector<int> rets;
    if (m_caliBrateMode == nullptr)
    {
        return rets;
    }

    QStandardItem* item = GetItemFromPostionAndDevice(devicesn, postion);
    if (item == nullptr)
    {
        return rets;
    }

    QList<QStandardItem*> items = m_caliBrateMode->findItems(item->text());
    if (items.empty())
    {
        return rets;
    }

    std::transform(items.begin(), items.end(), std::back_inserter(rets), [](auto& item)->int {return item->row(); });
    return rets;
}

///
/// @brief 页面显示时，处理事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月26日，新建函数
///
void QCalibrateOverView::showEvent(QShowEvent *event)
{
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("校准") + QString(" > ") + tr("校准申请"));
    if (m_needUpdate)
    {
        ULOG(LOG_INFO, "%s, need update.", __FUNCTION__ );
        UpdateCalibrateStatusTable(m_devs);
        m_needUpdate = false;
    }

    SetTableColumnFixedWidth();
}

///
/// @brief 获取表格中被选中的第一行
///
///
/// @return 第一行的行索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月11日，新建函数
///
int QCalibrateOverView::GetFirstIndex()
{
    int minRow = 2147483647;
    bool setted = false;
    const QModelIndexList& ls = ui->tableView->selectionModel()->selectedIndexes();

    for (const auto& l : ls)
    {
        if (l.isValid() && l.row() < minRow)
        {
            minRow = l.row();
            setted = true;
        }
    }

    if (!setted)
        minRow = -1;

    return minRow;
}

///
/// @brief
///     判断该试剂组是否拥有校准曲线
///
/// @param[in]  reagent  待判断试剂组
/// 
/// 
/// @return true:表示有校准曲线  false:表示没有校准曲线
///
/// @par History:
/// @li 8580/GongzhiQiang，2022年12月9日，新建函数
///
bool QCalibrateOverView::isContansCaliCurve(const boost::optional<ch::tf::ReagentGroup>& reagent)
{
	// 通过试剂曲线ID进行判断
	return (reagent->caliCurveId < 0) ? false : true;
}


///
/// @brief 校准项目查询
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
///
void QCalibrateOverView::OnCaliProjectNameSearch()
{
	//获取需要查找的项目信息
	auto&& projectName = ui->comboBox_projectName->currentText();
	
	// 如果在空的时候进行查找，直接返回
    if (projectName != m_filterAssayName)
    {
        m_filterAssayName = projectName;
        OnCaliProjectNameReset();
        SetAscSortByFirstColumn();
    }
}


///
/// @brief 校准项目查询重置
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
///
void QCalibrateOverView::OnCaliProjectNameReset()
{
    m_bIsSearching = true;
	//刷新
	UpdateCalibrateStatusTable(m_devs);
    m_bIsSearching = false;
}

bool QCalibrateOverView::IsShowLineByFilter(const QList<QStandardItem*>& data)
{
    if (!m_filterAssayName.isEmpty() && data.size() > 1 && !data[1]->text().contains(m_filterAssayName, Qt::CaseInsensitive))
        return false;

    return true;
}

///
/// @brief
///     初始化校准申请界面中的查询名称
/// 
/// 
///
/// @par History:
/// @li 8580/GongzhiQiang，2022年12月19日，新建函数
///
void QCalibrateOverView::initCaliProjectNameBox(DeviceMap&  devices)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 用一个有序容器进行存储项目名称
	QStringList projectNames;
	projectNames.clear();

	// 如果有生化设备
	if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) != devices.end())
	{
		// 获取所有项目信息
		auto assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();

		// 获取生化项目信息
		auto chAssayMap = CommonInformationManager::GetInstance()->GetChAssayIndexCodeMaps();
		// 依次添加项目名称到容器中
		for (const auto& chAssay : chAssayMap)
        {
            if (chAssay.second) 
            {
                std::map<int, std::shared_ptr<tf::GeneralAssayInfo>>::iterator gIter = assayMap.find(chAssay.second->assayCode);
                if (gIter != assayMap.end())
                {
                    projectNames.append(QString::fromStdString(gIter->second->assayName));
                }
            }
		}
	}

	// 检查是否选中了免疫设备
	if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE) != devices.end())
	{
		// 获取所有项目信息
		auto assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();

		// 获取免疫项目信息
		auto imAssayMap = CommonInformationManager::GetInstance()->GetImAssayIndexCodeMaps();

		// 依次添加项目名称到容器中
		for (const auto& imAssay : imAssayMap)
		{
            if(imAssay.second)
            {
                std::map<int, std::shared_ptr<tf::GeneralAssayInfo>>::iterator iIter = assayMap.find(imAssay.second->assayCode);
                if (iIter != assayMap.end())
                {
                    QString strName = QString::fromStdString(iIter->second->assayName);
                    if (!projectNames.contains(strName))
                    {
                        projectNames.append(strName);
                    }
                }
            }
		}
	}

	// 检查是否选中了ISE设备
	if (devices.find(tf::AssayClassify::ASSAY_CLASSIFY_ISE) != devices.end())
	{
		projectNames.append("ISE-U");
		projectNames.append("ISE-S");
	}

	// 将名称进行排序
	projectNames.sort();

	// 重新装载
	ui->comboBox_projectName->clear();
	ui->comboBox_projectName->addItems(projectNames);
	ui->comboBox_projectName->setCurrentIndex(-1);
	
}

void QCalibrateOverView::StorePairedReagent(const im::tf::ReagentInfoTable & reagent)
{
    // 不存在配对信息，则直接返回
    if (reagent.subStep.empty() || !reagent.__isset.assayCode || reagent.assayCode <= 0)
        return;

    // 生成并验证主键
    QString&& key = GetImPairReagentMapKey(reagent);
    if (key.isEmpty())
        return;

    // 没有找到已存在的配对信息，则直接新建一个
    if (m_imPairedReagent.find(key) == m_imPairedReagent.end())
    {
        m_imPairedReagent[key] = std::move(QVector<im::tf::ReagentInfoTable>{ im::tf::ReagentInfoTable() , im::tf::ReagentInfoTable() });
    }
    
    m_imPairedReagent[key][reagent.bottleIdx] = reagent;
}

QString QCalibrateOverView::GetImPairReagentMapKey(const im::tf::ReagentInfoTable & reagent)
{
    if (!reagent.__isset.assayCode || reagent.assayCode <= 0 || reagent.subStep.empty())
        return QString();

    QString key;

    // 始终以0号的试剂信息来作为主键
    if (reagent.bottleIdx == 0)
        key = QString::number(reagent.assayCode) + "|" + QString::fromStdString(reagent.reagentLot) + "|" + QString::fromStdString(reagent.reagentSN);
    else if (reagent.bottleIdx == 1)
        key = QString::number(reagent.assayCode) + "|" + QString::fromStdString(reagent.reagentLot) + "|" + QString::fromStdString(reagent.subStep);

    return key;
}

void QCalibrateOverView::SetTableColumnFixedWidth()
{
    // zhang.changjiang ??? 此处设置列宽是因为，初次显示没有设置默认列宽
    // 设置表格列宽比
    //QVector<double> vAssayScale;
    //vAssayScale << 0.7 << 1.0 << 1.0 << 1.0 << 1.0 << 1.5 << 1.4 << 1.5 << 0.8 << 0.4;
    //SetTblColWidthScale(ui->tableView, vAssayScale);
    ResizeTblColToContent(ui->tableView);
}

bool QCalibrateOverView::DealImPairedReagentOfBottleIdx_1(const im::tf::ReagentInfoTable& reagent)
{
    //处理bottleIdx为为1的配对信息
    QString key = GetImPairReagentMapKey(reagent);
    if (key.isEmpty() || !reagent.__isset.assayCode || reagent.assayCode <= 0 ||
        m_imPairedReagent.find(key) == m_imPairedReagent.end() || reagent.bottleIdx != 1)
        return false;

    auto& reagentInx_0 = m_imPairedReagent[key][0];

    if (reagentInx_0.reagentSN == reagent.subStep)
    {
        auto&& rows = GetRowByPostion(reagentInx_0.reagentPos, reagentInx_0.deviceSN);

        for (const auto& r : rows)
        {
            SetPairedReagentPos(reagent, r);
        }

        return true;
    }

    return false;
}

void QCalibrateOverView::SetPairedInfo(const im::tf::ReagentInfoTable & reagent)
{
    // 单条更新时，只会更新bottleIdx为0的信息
    // 此处查询一次是否存在配对试剂，更新一次配对试剂的信息

    //处理bottleIdx为为1的配对信息
    QString key = GetImPairReagentMapKey(reagent);
    if (key.isEmpty() || !reagent.__isset.assayCode || reagent.assayCode <= 0 ||
        m_imPairedReagent.find(key) == m_imPairedReagent.end()
        || reagent.bottleIdx == 1)
        return;

    if (m_imPairedReagent[key][1].reagentSN == reagent.subStep)
    {
        auto&& rows = GetRowByPostion(reagent.reagentPos, reagent.deviceSN);

        for (const auto& r : rows)
        {
            SetPairedReagentPos(m_imPairedReagent[key][1], r);
        }
    }
}

void QCalibrateOverView::RemovePairedReagentsByRow(int row)
{
    if (row < 0)
        return;

    tf::AssayClassify::type classify = GetItemClassify(row);
    
    if (tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE == classify)
    {
        // 获取缓存试剂信息
        boost::optional<im::tf::ReagentInfoTable> stuRgntInfo = GetItemUserDataInfo<im::tf::ReagentInfoTable>(row, ReagentInfo);

        if (stuRgntInfo)
            RemovePairedReagent(*stuRgntInfo);
    }
}

void QCalibrateOverView::RemovePairedReagent(const im::tf::ReagentInfoTable & reagent)
{
    QString&& key = GetImPairReagentMapKey(reagent);

    if (key.isEmpty() || m_imPairedReagent.find(key) == m_imPairedReagent.end())
        return;

    m_imPairedReagent.remove(key);
}

void QCalibrateOverView::SetPairedReagentPos(const im::tf::ReagentInfoTable& reagent, int r)
{
    auto item = m_caliBrateMode->item(r);
    boost::optional<im::tf::ReagentInfoTable> stuRgntInfo = GetItemUserDataInfo<im::tf::ReagentInfoTable>(r, ReagentInfo);

    if (item == nullptr || !stuRgntInfo)
        return;

    if (stuRgntInfo->reagentPos < reagent.reagentPos)
    {
        // 设置新的位置信息
        QString newText = item->text() + "," + QString::number(reagent.reagentPos);
        item->setText(newText);
    }
    else
    {
        auto pItem = GetItemFromPostionAndDevice(reagent.deviceSN, reagent.reagentPos);
        if (pItem)
        {
            // 设置新的位置信息
            QString newText = pItem->text() + "," + QString::number(stuRgntInfo->reagentPos);
            item->setText(newText);
            delete pItem;
        }
    }
}

bool QCalibrateOverView::AddAssayToSearchList(int assayCode)
{
    if (assayCode < 1 || assayCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND || m_bIsSearching)
        return false;

    const auto& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssay && !spAssay->assayName.empty())
    {
        

        const auto&& name = QString::fromStdString(spAssay->assayName);

        if (m_loadeAssays.find(name) == m_loadeAssays.end())
        {
            m_loadeAssays.insert(name);
            return true;
        }
    }
        
    return false;
}

bool QCalibrateOverView::RmAssayFromSearchList(int assayCode)
{
    if (assayCode < 1 || m_bIsSearching)
        return false;

    const auto& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    
    if (spAssay && !spAssay->assayName.empty())
    {
        auto&& name = QString::fromStdString(spAssay->assayName);

        if (m_caliBrateMode->findItems(name).isEmpty() && m_loadeAssays.find(name) != m_loadeAssays.end())
        {
            m_loadeAssays.remove(name);
            return true;
        }
    }

    return false;
}

void QCalibrateOverView::SetSearchList()
{
    if (m_bIsSearching)
        return;

    auto&& namelist = m_loadeAssays.toList();
    namelist.sort();

    //disconnect(ui->comboBox_projectName, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &QCalibrateOverView::OnCaliProjectNameSearch);
    //QString curText = ui->comboBox_projectName->currentText();
    // 重新装载
    if (ui->comboBox_projectName->model()->rowCount() > 0)
        ui->comboBox_projectName->clear();
    ui->comboBox_projectName->addItems(namelist);
    if (!m_filterAssayName.isEmpty() && m_loadeAssays.find(m_filterAssayName) != m_loadeAssays.end())
        ui->comboBox_projectName->setCurrentText(m_filterAssayName);
    else
        ui->comboBox_projectName->setCurrentIndex(-1);

    //connect(ui->comboBox_projectName, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
    //    this, &QCalibrateOverView::OnCaliProjectNameSearch);
}

void QCalibrateOverView::SetChBtnVisible(bool visible)
{
    ui->caliBrateApp_Button->setVisible(visible);
    ui->Reaction_Button->setVisible(visible);
    ui->caliFactorEdit_btn->setVisible(visible);
}

void QCalibrateOverView::SetTblItemState(QStandardItem * pItem, TblItemState enState)
{
    ULOG(LOG_INFO, __FUNCTION__);
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
            //SetItemColor(pItem, QVariant());
            break;
        case STATE_NOTICE:
            pItem->setData(QColor(UI_REAGENT_NOTEFONT), Qt::TextColorRole);
            //SetItemColor(pItem, UI_REAGENT_NOTECOLOR);
            break;
        case STATE_WARNING:
            pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
            //SetItemColor(pItem, UI_REAGENT_WARNCOLOR);
            break;
        case STATE_SCAN_FAILED:
            pItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
            //SetItemColor(pItem, QVariant());
            break;
        default:
            pItem->setData(QVariant(), Qt::TextColorRole);
            //SetItemColor(pItem, QVariant());
            break;
    }
}

///
/// @brief 项目配置发生变化
///
/// @par History:
/// @li 1226/zhangjing，2023年6月28日，新建函数
///
void QCalibrateOverView::OnAssayUpdated()
{
    /* 项目发生变化后的影响：
        1.可能名称变化 （项目名称和筛选下拉框）
        2.可能有效期发生变化（）
    */

    m_loadeAssays.clear();
    m_filterAssayName.clear();

    // 重置搜索框
    //initCaliProjectNameBox(m_devs);

    // 刷新页面
    UpdateCalibrateStatusTable(m_devs);

    // 按第一列升序排序
    SetAscSortByFirstColumn();
}

///
/// @bref
///		权限变更响应
///
/// @par History:
/// @li 8276/huchunli, 2023年10月18日，新建函数
///
void QCalibrateOverView::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 解除校准屏蔽
    userPms->IsPermisson(PSM_IM_CALI_APPLY_UNSHILD) ? ui->unMaskButton->show() : ui->unMaskButton->hide();
    // 编辑校准结果
    bool bEditPer = userPms->IsPermisson(PSM_CALI_EDIT_RESULT);
    bool hasCh = m_devs.find(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) != m_devs.end();
    (bEditPer && hasCh) ? ui->caliFactorEdit_btn->show() : ui->caliFactorEdit_btn->hide();
    // 仪器系数
    userPms->IsPermisson(PMS_SET_INSTRUMENT_PARAM) ? ui->machineFactor->show() : ui->machineFactor->hide();
}


///
/// @brief  新的一天刷新界面  bug21887
///
/// @par History:
/// @li 1226/zhangjing，2023年8月29日，新建函数
///
void QCalibrateOverView::OnRefreshCaliInfoList()
{
    // 刷新页面
    UpdateCalibrateStatusTable(m_devs);
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 1226/zhangjing，2023年11月8日，新建函数
///
void QCalibrateOverView::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    // 刷新页面
    UpdateCalibrateStatusTable(m_devs);
}

void QCalibrateOverView::OnCaliExpireNoticeChanged(bool bOn)
{
    m_needUpdate = true;
    // 打开校准提示的话，需要重新获取每一行的校准过期提醒
//     if (bOn)
//     {
//         for (int i = 0; i < m_caliBrateMode->rowCount(); ++i)
//         {
// 
//         }
//     }
//     else
//     {
//         auto list = std::move(m_caliBrateMode->findItems(":", Qt::MatchExactly, 5/*当前曲线有效期列*/));
// 
//         for (int i = 0; i < list.size(); ++i)
//         {
// 
//             //m_caliBrateMode->setData();
//         }
//     }
}

///////////////////////////////////////////////////////////////////////////
/// @file     QCalibrateOverView.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QCalibrateOverFilterDataModule::QCalibrateOverFilterDataModule(QObject * parent)
	: QSortFilterProxyModel(parent)
{
}

QCalibrateOverFilterDataModule::~QCalibrateOverFilterDataModule()
{
}

bool QCalibrateOverFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
	// 获取源model
    QToolTipModel* pSouceModel = dynamic_cast<QToolTipModel*>(sourceModel());
	if (pSouceModel == nullptr)
	{
		return false;
	}

	QString leftData = pSouceModel->data(source_left).toString();
	QString rightData = pSouceModel->data(source_right).toString();
	return QString::localeAwareCompare(leftData, rightData) < 0;
}


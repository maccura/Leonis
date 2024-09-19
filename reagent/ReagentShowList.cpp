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
/// @file     ReagentShowList.h
/// @brief    试剂界面
///
/// @author   4170/TangChuXian
/// @date     2020年5月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ReagentShowList.h"
#include "ui_ReagentShowList.h"
#include <vector>
#include <algorithm>
#include <QStandardItem>
#include <QAbstractItemView>
#include <QItemSelection>
#include <QFileDialog>
#include <QScrollBar>

#include "src/thrift/im/i6000/gen-cpp/i6000_constants.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_constants.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"

#include "shared/ReagentCommon.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/FileExporter.h"

#include "reagent/ch/QReagentScanDialog.h"
#include "reagent/ch/QSelectConditionDialog.h"
#include "reagent/ch/QRegisterSupplyInfo.h"
#include "reagent/im/imbeadmixdlg.h"
#include "reagent/im/imloaderresetdlg.h"
#include "reagent/rgntnoticedatamng.h"
#include "requirecalcdlg.h"
#include "SortHeaderView.h"

#include "Serialize.h"
#include "printcom.hpp"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "src/public/ConfigSerialize.h"

///
/// @brief
///     试剂列索引
/// 
enum ReagentColumnIndex {
    RCI_ReagentPos = 0, // 试剂位
    RCI_Name,           // 名称
    RCI_ValidTestCnt,   // 可用测试数
    RCI_Residual,       // 剩余测试数
    RCI_Statu,          // 使用状态
    RCI_CaliStatu,      // 校准状态
    RCI_ReagentLot,     // 试剂批号
    RCI_ReagentSerial,  // 瓶号
    RCI_CaliExpire,     // 当前曲线有效期
    RCI_OpenEffectDays, // 开瓶有效期(天) 记录pos、sn、试剂类型data的行，在（开瓶有效期(天)）
    RCI_OnBoardTime,    // 上机时间
    RCI_Expire,         // 失效日期
    RCI_ColumnAmount    // 总列数
};

#define     USER_SORT_ROLE                  (Qt::UserRole + 21)      // 排序值
#define     USER_EMPTY_ROW_FLAG_ROLE        (Qt::UserRole + 22)      // 空行标志

#define     SetItemSortData(item, sortData) {item->setData(sortData, USER_SORT_ROLE);}

///
/// @brief
///     表格排序自旋锁
/// 
class TblSortLockGuard
{
public:
    TblSortLockGuard(QTableView* pTable) : m_pTable(pTable)
    {
        if (m_pTable == Q_NULLPTR)
        {
            return;
        }

        // 暂时禁能排序
        m_bEbableSort = m_pTable->isSortingEnabled();
        m_pTable->setSortingEnabled(false);
    }

    ~TblSortLockGuard()
    {
        if (m_pTable == Q_NULLPTR)
        {
            return;
        }

        // 还原排序使能
        m_pTable->setSortingEnabled(m_bEbableSort);
    }

private:
    bool            m_bEbableSort;      // 是否排序使能
    QTableView*     m_pTable;            //表格
};

ReagentShowList::ReagentShowList(QWidget *parent /*= Q_NULLPTR*/)
    : QWidget(parent),
    ui(new Ui::ReagentListShow),
    m_selectDialog(nullptr),
    m_scanDialog(nullptr),
    m_registerDialog(nullptr),
    m_needUpdate(true),
    m_bInit(false),
    m_iSortOrder(SortHeaderView::NoOrder),
    m_iSortColIdx(-1)
{
    ui->setupUi(this);
    m_selectCondition = nullptr;
    m_commAssayMgr = CommonInformationManager::GetInstance();

    // 以支持大小写不敏感排序
    m_reagentModel = new QStandardItemModel();
    ui->tableView->setModel(m_reagentModel);

    m_reagentModel->setSortRole(USER_SORT_ROLE);
    m_reagentModel->setColumnCount(RCI_ColumnAmount);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableView->setItemDelegate(new CReadOnlyDelegate(this));
    ui->tableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableView->setSortingEnabled(true);

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal);
    m_pSortHeader->setStretchLastSection(true);
    ui->tableView->setHorizontalHeader(m_pSortHeader);

    // 默认排序初始化
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);

    // 点击表头触发排序
    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order){
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            ui->tableView->sortByColumn(RCI_ReagentPos, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
            m_iSortOrder = SortHeaderView::NoOrder;
            m_iSortColIdx = -1;
        }
        else
        {
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->tableView->sortByColumn(logicIndex, qOrderFlag);
            m_iSortOrder = order;
            m_iSortColIdx = logicIndex;
        }
    });

    // 重新设置页面
    QStringList reagentHeaderListString = { tr("试剂位"), tr("名称"), tr("可用测试数"), tr("剩余测试数"), tr("使用状态"),
        tr("校准状态"), tr("试剂批号"), tr("瓶号"), tr("当前曲线有效期"), tr("开瓶有效期(天)"), tr("上机时间"), tr("失效日期") };
    m_reagentModel->setHorizontalHeaderLabels(reagentHeaderListString);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 100);
    ui->tableView->setColumnWidth(1, 190);
    ui->tableView->setColumnWidth(2, 150);
    ui->tableView->setColumnWidth(3, 150);
    ui->tableView->setColumnWidth(4, 150);
    ui->tableView->setColumnWidth(5, 150);
    ui->tableView->setColumnWidth(6, 150);
    ui->tableView->setColumnWidth(7, 150);
    ui->tableView->setColumnWidth(8, 150);
    ui->tableView->setColumnWidth(9, 193);
    ui->tableView->setColumnWidth(10, 200);
    ui->tableView->setColumnWidth(11, 112);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->horizontalHeader()->setTextElideMode(Qt::ElideLeft);

    // 显示前初始化
    InitBeforeShow();
}

ReagentShowList::~ReagentShowList()
{
}

///
/// @brief 界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月5日，新建函数
///
void ReagentShowList::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化余量提醒管理器
    RgntNoticeDataMng::GetInstance();

    // 筛选对话框
    connect(ui->selectCheck, SIGNAL(clicked()), this, SLOT(OnSelectConditionDialog()));
    // 屏蔽试剂
    connect(ui->reagentshield, SIGNAL(clicked()), this, SLOT(OnShieldReagent()));
    // 优先使用
    connect(ui->reagentPriority, SIGNAL(clicked()), this, SLOT(OnPriorityReagent()));
    // 试剂扫描
    connect(ui->reagentScan, SIGNAL(clicked()), this, SLOT(OnScanReagent()));
    // 试剂卸载
    connect(ui->reagentUnload, SIGNAL(clicked()), this, SLOT(OnReagentUnload()));
    // 试剂加载
    connect(ui->reagentUploadBtn, SIGNAL(clicked()), this, SLOT(OnReagentUpload()));
    // 信息录入
    connect(ui->reagentRegister, SIGNAL(clicked()), this, SLOT(onRegisterClicked()));

    // 根据选择进行按钮使能设置
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this, SLOT(OnTabelSelectChanged()));
    connect(ui->tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTableViewClicked(const QModelIndex&)));

    // 磁珠混匀
    connect(ui->mixMagneticBtn, SIGNAL(clicked()), this, SLOT(OnMixBtnClicked()));

    // 装载装置复位按钮被点击
    connect(ui->LoadPartResetBtn, SIGNAL(clicked()), this, SLOT(OnLoadPartResetBtnClicked()));

    // 打印
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));

    // 导出
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 需求计算
    connect(ui->RequireCalcBtn, SIGNAL(clicked()), this, SLOT(OnRequireCalcBtnClicked()));

    // 监听列宽变化
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(OnTblColResized()));

    // 监听项目编码管理器信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnRefreshReagentList);
    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, UpdateReagentInformation);
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateImReagentChanged);
    REGISTER_HANDLER(MSG_ID_IM_DILUENT_INFO_UPDATE, this, UpdateImDltChanged);
    // 监听试剂扫描开始消息
    //REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnClearReagentInfo);
    // 当前日期改变
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, OnRefreshReagentList);

    // 试剂报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_REAGENT, this, OnRefreshReagentList);

	// 耗材报警值更新（有些耗材放在试剂的提示中，需要关注该消息）
	REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, OnRefreshReagentList);

    // 指定位置试剂扫描
    REGISTER_HANDLER(MSG_ID_REAGENT_SCAN_POS_START, this, OnRgntScanPosStart);

    // 显示设置更新
    REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, OnDisplayCfgUpdate);

    // 试剂加卸载状态更新
    REGISTER_HANDLER(MSG_ID_REAGENT_LOAD_STATUS_UPDATE, this, OnRgntLoadStatusChanged);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);

    // 监听设备温度异常状态改变
    //REGISTER_HANDLER(MSG_ID_TEMPERATURE_ERR, this, OnDevTemperatureErrChanged);

    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);

    ui->tableView->setAutoScroll(true); // 自动滚动

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 设备otherinfo更新（为了实现当前选中试剂为生化设备时，而应用-功能屏蔽中取消勾选了自动装卸载，当前界面试剂卸载按钮应置灰）
	REGISTER_HANDLER(MSG_ID_DEVICE_OTHER_INFO_UPDATE, this, OnUpdateChUnloadReagentBtn);

	// 试剂加载器的更新
	REGISTER_HANDLER(MSG_ID_MANAGER_REAGENT_LOADER_UPDATE, this, OnLoaderStatusUpdate);

	// 未分配项目列表更新
	REGISTER_HANDLER(MSG_ID_MANAGER_UNALLOCATE_ASSAYS_UPDATE, this, OnRefreshReagentList);

	// 申请更换试剂结果消息
	REGISTER_HANDLER(MSG_ID_MANUAL_HANDL_REAG, this, OnManualHandleReagResult);

    // 监听校准过期提示开关改变
    REGISTER_HANDLER(MSG_ID_CALI_LINE_EXPIRE_CONFIG, this, OnCaliExpireNoticeOnOrOff);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DCS_REPORT_STATUS_CHANGED, this, UpdateDeviceStatus);

    //设置按钮多语言显示
    QString tmpStr = ui->reagentUploadBtn->fontMetrics().elidedText(ui->reagentUploadBtn->text(), Qt::ElideRight, 80);
    ui->reagentUploadBtn->setToolTip(ui->reagentUploadBtn->text());
    ui->reagentUploadBtn->setText(tmpStr);

    tmpStr = ui->reagentUnload->fontMetrics().elidedText(ui->reagentUnload->text(), Qt::ElideRight, 80);
    ui->reagentUnload->setToolTip(ui->reagentUnload->text());
    ui->reagentUnload->setText(tmpStr);

    tmpStr = ui->mixMagneticBtn->fontMetrics().elidedText(ui->mixMagneticBtn->text(), Qt::ElideRight, 80);
    ui->mixMagneticBtn->setToolTip(ui->mixMagneticBtn->text());
    ui->mixMagneticBtn->setText(tmpStr);

    tmpStr = ui->RequireCalcBtn->fontMetrics().elidedText(ui->RequireCalcBtn->text(), Qt::ElideRight, 80);
    ui->RequireCalcBtn->setToolTip(ui->RequireCalcBtn->text());
    ui->RequireCalcBtn->setText(tmpStr);
}

void ReagentShowList::GetClassfiedDeviceSn(std::vector<std::string>& chDevicesSn, std::vector<std::string>& imDevicesSn)
{
    for (const auto& device : m_deivces)
    {
        // ISE 不显示
        if (device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            continue;
        }

        // 分类设备SN
        if (device->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            imDevicesSn.push_back(device->deviceSN);
        }
        else
        {
            chDevicesSn.push_back(device->deviceSN);
        }
    }
}

void ReagentShowList::FillExportContainer(ReagentItemVector &vecReagent, bool bPrint)
{
    ULOG(LOG_INFO, "%s, reagent size:%d, isPrint:%d.", __FUNCTION__, vecReagent.size(), bPrint);

    // 获取一个单元格内容
    auto funcValue = [this](int ir, int ic) -> std::string {
        QStandardItem* qStand = m_reagentModel->item(ir, ic);
        if (qStand == Q_NULLPTR)
        {
            return "";
        }
        return qStand->text().toStdString();
    };

    vecReagent.clear();
    int rowNumber = m_reagentModel->rowCount();
    for (int i = 0; i < rowNumber; ++i)
    {
        ReagentItem info;
        info.strReagentName = funcValue(i, RCI_Name); // 项目名称

        QString strPos = QString::fromStdString(funcValue(i, RCI_ReagentPos));
        std::shared_ptr<BaseReagData> reagData = m_reagCacheMgr.GetReagData(strPos, QString::fromStdString(info.strReagentName));
        if (reagData == nullptr)
        {
            continue;
        }
        // DB id/ Pos/ AssayCode
        reagData->AssignPrintInfor(info);

        // 赋予打印名称 生化项目不做此要求
        if (bPrint && (reagData->m_myType == RT_IM || reagData->m_myType == RT_IMDIT) && info.uAssayCode != 0)
        {
            auto pGeneralAssay = CommonInformationManager::GetInstance()->GetAssayInfo(info.uAssayCode);
            if (pGeneralAssay != nullptr)
            {
                info.strReagentName = pGeneralAssay->printName;
            }
        }

        info.strReagentPos = funcValue(i, RCI_ReagentPos);                  // 位置
        info.strValidityTestNum = funcValue(i, RCI_ValidTestCnt);           // 可用测试数
        info.strResidualTestNum = funcValue(i, RCI_Residual);               // 剩余测试数
        info.strUseStatus = funcValue(i, RCI_Statu);                        // 使用状态
        info.strCaliStatus = funcValue(i, RCI_CaliStatu);                   // 校准状态
        info.strReagentLot = funcValue(i, RCI_ReagentLot);                  // 试剂批号
        info.strReagentSN = funcValue(i, RCI_ReagentSerial);                // 试剂序号
        info.strCaliValidityDays = funcValue(i, RCI_CaliExpire);            // 校准曲线有效期
        info.strBottleValidityDays = funcValue(i, RCI_OpenEffectDays);      // 开瓶有效天数（strReagentValidityDate未被显示，采用strReagentValidityDays显示）
        info.strReagentValidityDays = funcValue(i, RCI_Expire);             // 试剂有效天数（试剂有效期 ）
        info.strRegisterDate = funcValue(i, RCI_OnBoardTime);               // 注册日期
        info.strReagentValidityDate = funcValue(i, RCI_Expire);               // 失效日期

        vecReagent.push_back(info);
    }
}

void ReagentShowList::CleanTableview()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 清空表格
    m_reagentModel->removeRows(0, m_reagentModel->rowCount());
    m_reagCacheMgr.m_reagentInfoCache.clear();

    // 计算行数
    int iRowCnt = 0;
    for (const auto& device : m_deivces)
    {
        // ISE 不显示
        if (device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            continue;
        }

        // 初始化设备试剂信息列表
        int reagentSlotNumber = gUiAdapterPtr(QString::fromStdString(device->name), QString::fromStdString(device->groupName))->GetReagentSlotNumber();
        if (reagentSlotNumber == -1)
        {
            ULOG(LOG_ERROR, "Unknown reagent slot, by deviceName:%s.", device->name.c_str());
            continue;
        }
        iRowCnt += reagentSlotNumber;
    }
    m_reagentModel->setRowCount(iRowCnt);

    // 初始化设备试剂信息列表
    int currentIndex = 0;
    for (const auto& device : m_deivces)
    {
        // ISE 不显示
        if (device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            continue;
        }

        // 初始化设备试剂信息列表
        int reagentSlotNumber = gUiAdapterPtr(QString::fromStdString(device->name), QString::fromStdString(device->groupName))->GetReagentSlotNumber();
        if (reagentSlotNumber == -1)
        {
            ULOG(LOG_ERROR, "Unknown reagent slot, deiveName:%s.", device->name.c_str());
            continue;
        }
        ShowBlankLine(currentIndex, 1, reagentSlotNumber, device->deviceSN);
    }
}

void ReagentShowList::UpdateModeDevices(const std::vector<std::shared_ptr<const tf::DeviceInfo>>& spDevModule)
{
    ULOG(LOG_INFO, "%s, devCnt:%d.", __FUNCTION__, spDevModule.size());

    // 更新device.
    m_deivces = spDevModule;

    ui->tableView->setCurrentIndex(QModelIndex());

    // 更新按钮显示
    UpdateBtnVisible();

    // 刷新界面
    if (m_selectCondition != nullptr && m_selectCondition->CountTrue() != 0)
    {
        OnConditionSearchUpdate(m_selectCondition);
    }
    else
    {
        RefreshModeReagentList();
    }

    // 更新按钮状态
    UpdateBtnEnabled();
}

///
/// @brief 设备状态变更
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
void ReagentShowList::DevStateChange()
{
    // 设置试剂扫描按钮是否允许状态，只针对免疫单机
    SetEnableOfReagSacnBtn();
}

///
/// @brief刷新界面
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月21日，新建函数
///
void ReagentShowList::RefreshModeReagentList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 记录前选中状态
    QModelIndexList curSelectedRows = ui->tableView->selectionModel()->selectedRows();

    std::vector<std::string> deviceSnListCh;
    std::vector<std::string> deviceSnListIm;
    GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);
    CleanTableview();

    // 更新试剂列表
	UpdateConsumeTableCh(deviceSnListCh);
    UpdateConsumeTableImm(deviceSnListIm);

    // 恢复前选中状态
    SelectByModelIndex(curSelectedRows);

    // 重置排序
    ui->tableView->sortByColumn(RCI_ReagentPos, Qt::AscendingOrder);
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    m_iSortOrder = SortHeaderView::NoOrder;
    m_iSortColIdx = -1;
}

///
/// @brief
///     重新刷新试剂列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月22日，新建函数
///
void ReagentShowList::OnRefreshReagentList()
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, __FUNCTION__);
    if (m_selectCondition != nullptr && m_selectCondition->CountTrue() != 0)
    {
        OnConditionSearchUpdate(m_selectCondition);
    }
    else
    {
        RefreshModeReagentList();
    }
}

///
/// @brief 设备状态更新
///
/// @param[in]  deviceInfo  设备信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月11日，新建函数
///

void ReagentShowList::OnFilterLableClosed()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (m_selectCondition != nullptr)
    {
        m_selectCondition->CleanValues();
    }
    RefreshModeReagentList();

    // 恢复排序
    //无效index或NoOrder就设置为默认未排序状态
    if (m_iSortColIdx < 0 || m_iSortOrder == SortHeaderView::NoOrder) {
        ui->tableView->sortByColumn(RCI_ReagentPos, Qt::AscendingOrder);
        m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    }
    else
    {
        Qt::SortOrder qOrderFlag = m_iSortOrder == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->tableView->sortByColumn(m_iSortColIdx, qOrderFlag);
    }
}

///
/// @brief
///     更新按钮显示（根据当前选择设备）
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月29日，新建函数
///
void ReagentShowList::UpdateBtnVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据当前选中设备，更新按钮显示
    std::vector<std::string> deviceSnListCh;
    std::vector<std::string> deviceSnListIm;
    GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);

    // 构造是否显示对应按钮变量
    bool bShowChBtn = !deviceSnListCh.empty();
    bool bShowImBtn = !deviceSnListIm.empty();

    // 更新按钮显示
    ui->reagentRegister->setVisible(bShowChBtn);
    ui->reagentUploadBtn->setVisible(bShowImBtn);
    ui->mixMagneticBtn->setVisible(bShowImBtn);
    ui->LoadPartResetBtn->setVisible(bShowImBtn);
}

///
/// @brief 显示试剂的信息到页面上
///     
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  reagentInfo  试剂信息
/// 
/// @par History:
/// @li 5774/WuHongTao，2022年1月21日，新建函数
///
void ReagentShowList::ShowReagnetItem(UiRowUpdateType updateType, const ch::tf::SuppliesInfo& supplyInfo, \
    const ch::tf::ReagentGroup& reagentInfo)
{
    ULOG(LOG_INFO, "%s(type:%d, supPos:%d, reagId:%lld)", __FUNCTION__, updateType, supplyInfo.pos, reagentInfo.id);
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = m_commAssayMgr->GetAssayInfo(reagentInfo.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can't find the assay item from commAssayMgr, by assayCode:%d.", reagentInfo.assayCode);
        return;
    }

    // 获取内容行
    QList<QStandardItem*> itemList;
    std::shared_ptr<BaseReagData> rItem = std::shared_ptr<BaseReagData>(new ReagDataCh(reagentInfo, supplyInfo));

    std::shared_ptr<BaseReagData::RowContentInfo> rdt = rItem->GetRowContent();
    rdt->GetStandardItemList(itemList);
    m_reagCacheMgr.m_reagentInfoCache[rdt->GetUniqKey()] = rItem;

    // 获取当前行
    if (itemList.size() == 0)
    {
        ULOG(LOG_WARN, "Invalid itemlist size.");
        return;
    }
    QString qstrItem = itemList[0]->text();
    QList<QStandardItem*> targItem = m_reagentModel->findItems(qstrItem);
    if (targItem.size() == 0)
    {
        ULOG(LOG_WARN, "Not find model item by %s.", qstrItem.toStdString().c_str());
        return;
    }

    int rowPosition = targItem[0]->row();
    // 通过项目来进一步确定行号
    if (targItem.size() > 1)
    {
        for (QStandardItem* pIt : targItem)
        {
            if (pIt == Q_NULLPTR)
            {
                continue;
            }
            QStandardItem* pNameIt = m_reagentModel->item(pIt->row(), 1);
            if (pNameIt != Q_NULLPTR)
            {
                QString strText = pNameIt->text();
                if (strText == rdt->m_assayName)
                {
                    rowPosition = pIt->row();
                }
            }
        }
    }

    // 设置行样式
    SetStanterdRowStyle(itemList, reagentInfo.reagentMask, reagentInfo.caliMask);
    
    // 插入模式
    if (updateType == Ur_Insert)
    {
        m_reagentModel->insertRow(rowPosition, itemList);
    }
    // 覆盖模式
    else
    {
        for (int column = 0; column < itemList.size(); column++)
        {
            // newItem一定要被setItem()或删除，否则内存泄漏
            m_reagentModel->setItem(rowPosition, column, itemList[column]);
        }
    }

    // 设置排序值
    SetSortValue(itemList, rowPosition);
}

void ReagentShowList::CoverChSupplies(const int pos, const ch::tf::SuppliesInfo& supplyInfo)
{
	ULOG(LOG_INFO, "%s(pos:%d, supplyInfo:%s)", __FUNCTION__, pos, ToString(supplyInfo));

	// 更新内容时锁定排序
	TblSortLockGuard tblSortLock(ui->tableView);

	// 获取内容行
	QList<QStandardItem*> itemList;
	std::shared_ptr<BaseReagData> rItem = std::shared_ptr<BaseReagData>(new ReagDataCh(ch::tf::ReagentGroup(), supplyInfo));

	std::shared_ptr<BaseReagData::RowContentInfo> rdt = rItem->GetRowContent();
	rdt->GetStandardItemList(itemList);
	m_reagCacheMgr.m_reagentInfoCache[rdt->GetUniqKey()] = rItem;

	// 覆盖模式
	for (int column = 0; column < itemList.size(); column++)
	{
		m_reagentModel->setItem(pos, column, itemList[column]);
	}

	// 设置排序值
	SetSortValue(itemList, pos);
}

void ReagentShowList::ShowReagnetItemImm(const ::im::tf::ReagentInfoTable& stuRgntInfo)
{
    ULOG(LOG_INFO, "%s, name:%s, pos:%d.", __FUNCTION__, stuRgntInfo.name.c_str(), stuRgntInfo.reagentPos);

    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 批号序列号同时为空判定扫描失败
    if (stuRgntInfo.reagentLot.empty() && stuRgntInfo.reagentSN.empty())
    {
        QString strKey = ShowScanFailedLine(stuRgntInfo.reagentPos, QString::fromStdString(stuRgntInfo.deviceSN));
        m_reagCacheMgr.m_reagentInfoCache[strKey] = std::make_shared<ReagDataIm>(stuRgntInfo);
        return;
    }

    QList<QStandardItem*> itemList;
    std::shared_ptr<BaseReagData> rItem = std::make_shared<ReagDataIm>(stuRgntInfo);

    std::shared_ptr<BaseReagData::RowContentInfo> rdt = rItem->GetRowContent();
    rdt->GetStandardItemList(itemList);
    m_reagCacheMgr.m_reagentInfoCache[rdt->GetUniqKey()] = rItem; // 缓存试剂信息

    // 设置行样式
    bool isReagMask = (stuRgntInfo.reagStatusUI.status == im::tf::StatusForUI::type::REAGENT_STATUS_MASK);
    bool isCaliMask = (stuRgntInfo.reagStatusUI.status == im::tf::StatusForUI::type::REAGENT_STATUS_CALI_MASK);
    SetStanterdRowStyle(itemList, isReagMask, isCaliMask);

    // 获取当前行
    if (itemList.size() == 0)
    {
        return;
    }
    QList<QStandardItem*> targItem = m_reagentModel->findItems(itemList[0]->text());
    if (targItem.size() == 0)
    {
        return;
    }
    int rowPosition = targItem[0]->row();

    // 覆盖刷新界面对应试剂行
    for (int column = 0; column < itemList.size(); column++)
    {
        m_reagentModel->setItem(rowPosition, column, itemList[column]);
    }

    // 设置排序值
    SetSortValue(itemList, rowPosition);
}

///
/// @brief
///     显示耗材项目
///
/// @param[in]  pos  耗材位置
/// @param[in]  supplyInfo  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月14日，新建函数
///
void ReagentShowList::ShowSupplyItem(const ch::tf::SuppliesInfo& supplyInfo)
{
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    QList<QStandardItem*> itemList;
    std::shared_ptr<BaseReagData> rItem = std::make_shared<ReagDataChSup>(supplyInfo);

    std::shared_ptr<BaseReagData::RowContentInfo> rdt = rItem->GetRowContent();
    rdt->GetStandardItemList(itemList);
    m_reagCacheMgr.m_reagentInfoCache[rdt->GetUniqKey()] = rItem;

    // 获取当前行
    if (itemList.size() == 0)
    {
        return;
    }
    QList<QStandardItem*> targItem = m_reagentModel->findItems(itemList[0]->text());
    if (targItem.size() == 0)
    {
        return;
    }
    int rowPosition = targItem[0]->row();

    // 覆盖刷新界面对应试剂行
    for (int column = 0; column < itemList.size(); column++)
    {
        m_reagentModel->setItem(rowPosition, column, itemList[column]);
    }

    // 设置排序值
    SetSortValue(itemList, rowPosition);
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
void ReagentShowList::ShowBlankLine(int& currentPos, int lastPos, int endPostion, const std::string& devicesn)
{
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    std::shared_ptr<const tf::DeviceInfo> currDevice = nullptr;
    for (const auto& device : m_deivces)
    {
        if (device->deviceSN == devicesn)
        {
            currDevice = device;
            break;
        }
    }

    int totalColumn = m_reagentModel->columnCount();
    for (int pos = lastPos; pos <= endPostion; pos++)
    {
        // 位置
        QStandardItem* item = GetItemFromPostionAndDevice(devicesn, pos);
        item->setData(true, USER_EMPTY_ROW_FLAG_ROLE); // 是否空行
        m_reagentModel->setItem(currentPos, 0, item);

        // 设置排序值
        QString spliterSymb = pos < 10 ? QStringLiteral("-0") : QStringLiteral("-");
        QString strSortPos = QString::fromStdString(CommonInformationManager::GetDeviceName(devicesn)) + spliterSymb + QString::number(pos);
        SetItemSortData(item, strSortPos);
        QString strItemText = item->text();
        m_reagCacheMgr.EraseByPositionString(strItemText);

        // 缓存设备
        m_rowInforMap[strItemText] = ReagRowInfor(currDevice, pos);

        // 余下的全是设置为空
        int i = 1;
        while (i < totalColumn)
        {
            m_reagentModel->setItem(currentPos, i++, Q_NULLPTR);
        }
        SetTblTextAlign(m_reagentModel, Qt::AlignCenter, currentPos);
        currentPos++;
    }
}

///
/// @brief
///     设置显示空行
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月7日，新建函数
///
void ReagentShowList::ShowBlankLine(const im::tf::ReagentInfoTable& stuRgntInfo)
{
    ULOG(LOG_INFO, "%s(pos:%d)", __FUNCTION__, stuRgntInfo.reagentPos);
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(stuRgntInfo.deviceSN));
    if (strDevNamePair.first.isEmpty())
    {
        return;
    }
    QString strDevName = strDevNamePair.second + strDevNamePair.first;

    // 获取项目信息
    QString strAssayName;
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = m_commAssayMgr->GetAssayInfo(stuRgntInfo.assayCode);
    if (Q_NULLPTR != spAssayInfo)
    {
        strAssayName = QString::fromStdString(spAssayInfo->assayName);
    }
    else if (stuRgntInfo.assayCode > 0)
    {
        strAssayName = QString::fromStdString(stuRgntInfo.name);
    }
    else
    {
        strAssayName = tr("扫描失败");
    }

    // 遍历表格，查找对应行
    int iMatchRow = -1;
    for (int iRow = 0; iRow < m_reagentModel->rowCount(); iRow++)
    {
        // 匹配设备名
        QStandardItem* pItem = Q_NULLPTR;
        pItem = m_reagentModel->item(iRow, RCI_ReagentPos);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 解析设备
        QString strDevAndPos = pItem->text();
        QStringList strDevPosLst = strDevAndPos.split("-");
        if (strDevPosLst.isEmpty() || (strDevPosLst.front() != strDevName))
        {
            continue;
        }

        // 获取位置
        int iPos = strDevPosLst.back().toInt();
        if ((stuRgntInfo.reagentPos > 0) && (stuRgntInfo.reagentPos == iPos))
        {
            // 匹配成功则记录行号
            iMatchRow = iRow;
            m_reagCacheMgr.EraseByPositionString(strDevAndPos);
            break;
        }

        // 位置不匹配则跳过
        if (stuRgntInfo.reagentPos > 0)
        {
            continue;
        }

        // 匹配项目名
        pItem = m_reagentModel->item(iRow, RCI_Name);
        if (pItem == Q_NULLPTR || (strAssayName != pItem->text()))
        {
            continue;
        }

        // 最后匹配ID
        std::shared_ptr<BaseReagData> reagData = m_reagCacheMgr.GetReagData(strDevAndPos, strAssayName);
        if (reagData == nullptr)
        {
            continue;
        }

        // 类型不为免疫试剂则跳过
        if (reagData->m_myType != RT_IM)
        {
            continue;
        }

        // 转换数据
        im::tf::ReagentInfoTable stuBufRgntInfo = boost::any_cast<im::tf::ReagentInfoTable>(reagData->m_reagent);
        if (stuRgntInfo.id != stuBufRgntInfo.id)
        {
            continue;
        }

        // 匹配成功则记录行号
        m_reagCacheMgr.EraseByPositionString(strDevAndPos);
        iMatchRow = iRow;
        break;
    }

    // 如果没有找到匹配的行号，则返回
    if (iMatchRow < 0)
    {
        return;
    }

    // 设置空行标志
    QStandardItem* pPosItem = m_reagentModel->item(iMatchRow);
    if (pPosItem != Q_NULLPTR)
    {
        pPosItem->setData(true, USER_EMPTY_ROW_FLAG_ROLE);
        pPosItem->setData(QVariant(), Qt::TextColorRole);
    }

    // 匹配行除了第一列全部置空
    for (int iCol = 1; iCol < m_reagentModel->columnCount(); iCol++)
    {
        m_reagentModel->setItem(iMatchRow, iCol, Q_NULLPTR);
    }

    // 居中
    SetTblTextAlign(m_reagentModel, Qt::AlignCenter, iMatchRow);
}

///
/// @brief
///     设置显示空行
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月7日，新建函数
///
void ReagentShowList::ShowBlankLine(const im::tf::DiluentInfoTable& stuDltInfo)
{
    ULOG(LOG_INFO, "%s(name:%s, pos:%d)", __FUNCTION__, stuDltInfo.diluentName.c_str(), stuDltInfo.pos);
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(stuDltInfo.deviceSN));
    if (strDevNamePair.first.isEmpty())
    {
        return;
    }
    QString strDevName = strDevNamePair.second + strDevNamePair.first;

    // 获取项目信息
    QString strAssayName = QString::fromStdString(stuDltInfo.diluentName);

    // 遍历表格，查找对应行
    int iMatchRow = -1;
    for (int iRow = 0; iRow < m_reagentModel->rowCount(); iRow++)
    {
        // 匹配设备名
        QStandardItem* pItem = Q_NULLPTR;
        pItem = m_reagentModel->item(iRow, RCI_ReagentPos);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 解析设备
        QString strDevAndPos = pItem->text();
        QStringList strDevPosLst = strDevAndPos.split("-");
        if (strDevPosLst.isEmpty() || (strDevPosLst.front() != strDevName))
        {
            continue;
        }

        // 获取位置
        int iPos = strDevPosLst.back().toInt();
        if ((stuDltInfo.pos > 0) && (stuDltInfo.pos == iPos))
        {
            // 匹配成功则记录行号s
            iMatchRow = iRow;
            break;
        }

        // 位置不匹配则跳过
        if (stuDltInfo.pos > 0)
        {
            continue;
        }

        // 匹配项目名
        pItem = m_reagentModel->item(iRow, RCI_Name);
        if (pItem == Q_NULLPTR || (strAssayName != pItem->text()))
        {
            continue;
        }

        // 最后匹配ID
        std::shared_ptr<BaseReagData> reagData = m_reagCacheMgr.GetReagData(strDevAndPos, strAssayName);
        if (reagData == nullptr)
        {
            continue;
        }

        // 类型不为免疫试剂则跳过
        if (reagData->m_myType != RT_IMDIT)
        {
            continue;
        }

        // 转换数据
        im::tf::DiluentInfoTable stuBufDltInfo = boost::any_cast<im::tf::DiluentInfoTable>(reagData->m_reagent);
        if (stuDltInfo.id != stuBufDltInfo.id)
        {
            continue;
        }

        // 匹配成功则记录行号
        m_reagCacheMgr.EraseByPositionString(strDevAndPos);
        iMatchRow = iRow;
        break;
    }

    // 如果没有找到匹配的行号，则返回
    if (iMatchRow < 0)
    {
        return;
    }

    // 设置空行标志
    QStandardItem* pPosItem = m_reagentModel->item(iMatchRow);
    if (pPosItem != Q_NULLPTR)
    {
        pPosItem->setData(true, USER_EMPTY_ROW_FLAG_ROLE);
        pPosItem->setData(QVariant(), Qt::TextColorRole);
    }

    // 匹配行除了第一列全部置空
    for (int iCol = 1; iCol < m_reagentModel->columnCount(); iCol++)
    {
        m_reagentModel->setItem(iMatchRow, iCol, Q_NULLPTR);
    }

    // 居中
    SetTblTextAlign(m_reagentModel, Qt::AlignCenter, iMatchRow);
}

///
/// @brief 显示扫描失败的项目
///
/// @param[in]  currentPos  当前位置
/// @param[in]  devicesn    设备序列号
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月10日，新建函数
///
QString ReagentShowList::ShowScanFailedLine(int showPostion, const QString& devicesn)
{
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 位置
    QStandardItem* itemPosition = GetItemFromPostionAndDevice(devicesn.toStdString(), showPostion);
    QList<QStandardItem*> targItem = m_reagentModel->findItems(itemPosition->text());
    if (targItem.size() == 0)
    {
        return "";
    }
    int currentPos = targItem[0]->row();
    targItem[0]->setData(false, USER_EMPTY_ROW_FLAG_ROLE);

    // 设置排序值
    QString spliterSymb = showPostion < 10 ? QStringLiteral("-0") : QStringLiteral("-");
    QString strSortPos = QString::fromStdString(CommonInformationManager::GetDeviceName(devicesn.toStdString())) + spliterSymb + QString::number(showPostion);
    SetItemSortData(targItem[0], strSortPos);

    // 扫描失败
    QStandardItem* itemStat = new QStandardItem(tr("扫描失败"));
    SetTblItemState(itemStat, STATE_SCAN_FAILED);
    m_reagentModel->setItem(currentPos, 1, itemStat);
    SetItemSortData(itemStat, itemStat->text());

    // 余下的全是设置为空
    int i = 2;
    auto column = m_reagentModel->columnCount();
    while (i < column)
    {
        m_reagentModel->setItem(currentPos, i++, new QStandardItem(""));
    }

    // 居中
    SetTblTextAlign(m_reagentModel, Qt::AlignCenter, currentPos);

    return itemPosition->text() + itemStat->text();
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
std::vector<int> ReagentShowList::GetRowByPostion(int postion, const std::string& devicesn)
{
    std::vector<int> rets;

    QStandardItem* item = GetItemFromPostionAndDevice(devicesn, postion);
    if (item == nullptr)
    {
        return rets;
    }

    QList<QStandardItem*> items = m_reagentModel->findItems(item->text());
    if (items.empty())
    {
        return rets;
    }

    std::transform(items.begin(), items.end(), std::back_inserter(rets), [](auto& item)->int {return item->row(); });
    return rets;
}

///
/// @brief 根据设备号和项目名称获取对应的行号
///
/// @param[in]  assayName  项目名称
/// @param[in]  device     设备号
///
/// @return 包括实际名称和相同设备的行
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月29日，新建函数
///
std::vector<int> ReagentShowList::GetRowsByAssayInfo(const QString& assayName, const std::string& device)
{
    std::vector<int> rets;

    QList<QStandardItem*> items = m_reagentModel->findItems(assayName, Qt::MatchExactly, 1);
    if (items.empty())
    {
        return rets;
    }

    QString devices = QString::fromStdString(CommonInformationManager::GetDeviceName(device));
    for (const auto& itemData : items)
    {
        // 获取位置行
        auto itemPos = m_reagentModel->item(itemData->row(), 0);
        auto text = itemPos->text();
        if (text.contains(devices))
        {
            rets.push_back(itemData->row());
        }
    }

    return rets;
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
void ReagentShowList::DoChangeAssays(const ch::tf::SuppliesInfo& supplyInfo, \
    const std::vector<ch::tf::ReagentGroup>& reagentGroups, const std::vector<int>& rows)
{
    ULOG(LOG_INFO, "%s, name:%s, pos:%d.", __FUNCTION__, supplyInfo.name.c_str(), supplyInfo.pos);

    // 参数检查
    if (reagentGroups.size() == 0 || reagentGroups.size() == rows.size())
    {
        ULOG(LOG_WARN, "Invalid reagent groups, size is:%d", reagentGroups.size());
        return;
    }

    // 行数大于试剂数，说明从双项切换到单数
    if (reagentGroups.size() < rows.size() && rows.size() > 1)
    {
        ShowReagnetItem(Ur_Cover, supplyInfo, reagentGroups[0]);
        m_reagentModel->removeRow(rows[1]);
    }
    // 行数小于试剂数，说明从单项变到双项同测
    else
    {
        if (rows.size() > 0)
        {
            UiRowUpdateType updateType = Ur_Cover;
            for (const ch::tf::ReagentGroup& reagent : reagentGroups)
            {
                ShowReagnetItem(updateType, supplyInfo, reagent);
                updateType = Ur_Insert; // 第二次开始使用Insert模式
            }
        }
    }
}

///
/// @brief
///     是否是数字列
///
/// @param[in]  iColumn  列号
///
/// @return true表示是数字列
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月28日，新建函数
///
bool ReagentShowList::IsNumberColumn(int iColumn)
{
    //ULOG(LOG_INFO, __FUNCTION__); // too frequency...
    if ((iColumn == RCI_ReagentPos) ||
        (iColumn == RCI_ValidTestCnt) ||
        (iColumn == RCI_Residual) ||
        (iColumn == RCI_ReagentLot) ||
        (iColumn == RCI_ReagentSerial) ||
        (iColumn == RCI_OpenEffectDays) ||
        (iColumn == RCI_CaliExpire))
    {
        return true;
    }

    return false;
}

///
/// @brief
///     从数据库中加载显示设置
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月13日，新建函数
///
void ReagentShowList::LoadDisplayCfg()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 解析xml
    std::vector<DisplaySet> data;
    if (!DictionaryQueryManager::GetUiDispalySet(data))
    {
        ULOG(LOG_ERROR, "Failed to get DisplaySet.");
        return;
    }

    // 获取试剂信息界面的显示设置
    for (auto& dataItem : data)
    {
        // 获取试剂信息显示设置
        if (dataItem.type == REAGENTTYPE)
        {
            m_stuDisplayCfg = dataItem;
            break;
        }
    }
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
void ReagentShowList::SetTblItemState(QStandardItem* pItem, TblItemState enState)
{
    // ULOG(LOG_INFO, __FUNCTION__); // too frequency
    // 参数检查
    if (pItem == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s, tbitemsstate:%d.", __FUNCTION__, enState);
        return;
    }
    // 判断状态
    std::shared_ptr<RowCellInfo> cellInfo = std::make_shared<RowCellInfo>();
    GetTblItemColor(cellInfo, enState);

    pItem->setData(cellInfo->m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo->m_fontColor), Qt::TextColorRole);
    //SetItemColor(pItem, cellInfo->m_backgroudColor.isEmpty() ? QVariant() : cellInfo->m_backgroudColor);
}

///
/// @brief 页面显示时，处理事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月26日，新建函数
///
void ReagentShowList::showEvent(QShowEvent *event)
{
    if (m_needUpdate)
    {
        OnRefreshReagentList();
        //QTimer::singleShot(0, this, SLOT(UpdateTblDisplayByCfg()));
        m_needUpdate = false;
    }

    // 第一次显示
    if (!m_bInit)
    {
        // 按显示设置更新
        UpdateTblDisplayByCfg();
    }

    // 重置为默认排序
    ui->tableView->sortByColumn(RCI_ReagentPos, Qt::AscendingOrder);
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    m_pSortHeader->ResetAllIndex();
    m_iSortOrder = SortHeaderView::NoOrder;
    m_iSortColIdx = -1;

    // 列宽自适应； 注释掉的原因：当数据大于500条时，每次切换页面都有大于1秒的明显卡顿
    //ResizeTblColToContent(ui->tableView);

    // 初始化标志置为true
    m_bInit = true;

    // 更新按钮使能状态
    UpdateBtnEnabled();
}

///
/// @brief 设置试剂扫描按钮是否允许状态，只针对免疫单机
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
void ReagentShowList::SetEnableOfReagSacnBtn()
{
    // 如果不是免疫单机
    std::shared_ptr<const tf::DeviceInfo> spDev = CommonInformationManager::GetInstance()->GetImSingleDevice();
    if (nullptr == spDev)
    {
        return;
    }
    // 试剂扫描按钮是否允许
    ui->reagentScan->setEnabled(IsExistDeviceCanScan());
}

///
/// @brief
///     初始化后表格列宽自适应
///
/// @param[in]  pTblView  表格
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月6日，新建函数
///
void ReagentShowList::ResizeTblColToContentAfterInit(QTableView* pTblView)
{
    // 如果已经初始化，则执行列宽自适应
    if (m_bInit)
    {
        ResizeTblColToContent(pTblView);
    }
}

///
/// @brief
///     无位置选择卸载试剂
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月10日，新建函数
///
void ReagentShowList::UnloadRgntInNoSelPos()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果设备序列号为空，则置为当前设备序列号
    std::string deviceSn;
    int reagentPos = -1;

    // 获取当前选中项索引(以获取选中设备和位置)    
    QModelIndex qIndex = ui->tableView->currentIndex();
    if (qIndex.isValid())
    {
        // 获取选中行对应的设备和位置
        int row = qIndex.row();
        if (!GetPositionInfo(row, deviceSn, reagentPos))
        {
            ULOG(LOG_INFO, "Failed to get deviceSN.");
        }
    }

    // 如果没有指定设备，弹框提示
    // 根据当前选中设备，更新按钮显示
    std::vector<std::string> deviceSnListCh;
    std::vector<std::string> deviceSnListIm;
    GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);

    // 如果存在生化设备直接返回
    if (!deviceSnListCh.empty())
    {
        ULOG(LOG_INFO, "!deviceSnListCh.empty().");
        return;
    }

    if (deviceSn.empty() && deviceSnListIm.size() != 1)
    {
        TipDlg(tr("请指定要进行试剂卸载的设备。")).exec();
        return;
    }

    // 获取设备序列号
    if (deviceSn.empty())
    {
        deviceSn = deviceSnListIm.front();
    }

    // 获取选中设备信息
    tf::DeviceInfo stuTfDevInfo = *CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);

    // 是否禁止卸载
    bool bForbidden = PopUnloadRgntForbiddenTip(stuTfDevInfo);

    // 禁止卸载则返回
    if (bForbidden)
    {
        return;
    }

    // 卸载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
    bool bExist = false;
    if (!ReagentLoadExistReag(bExist, deviceSn))
    {
        ULOG(LOG_ERROR, "ReagentLoadExistReag failed, devSn:%s.", deviceSn.c_str());
        return;
    }
    // 存在试剂盒，调用取消加载试剂指令（包含检测是否有试剂盒以及复位回收流程）
    if (bExist)
    {
		// 设置已启动卸载
		RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(deviceSn), true);

        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, deviceSn);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return;
        }
        return;
    }

    // 检查是否屏蔽试剂在线加载功能，如果屏蔽，则卸载按钮功能为试剂申请 TODO
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, deviceSn))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed, devSn:%s.", deviceSn.c_str());
        return;
    }

    // 屏蔽后进入申请更换试剂流程
    if (bMask)
    {
        // 判断当前申请更换试剂状态
        auto RgntApplyStatus = GetApplyChangeRgntStatus(deviceSn);

        // 如果已申请，则取消
        if (RgntApplyStatus == RGNT_CHANGE_APPLY_STATUS_APPLYING)
        {
            TipDlg tipDlg(tr("取消申请更换试剂"), tr("是否取消申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, deviceSn, false);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("取消申请更换试剂"), tr("取消申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(deviceSn, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }
        else
        {
            TipDlg tipDlg(tr("申请更换试剂"), tr("是否申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, deviceSn);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("申请更换试剂"), tr("申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(deviceSn, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }

        return;
    }

    // 判断试剂盘是否存在空瓶
    if (!RgntNoticeDataMng::GetInstance()->IsExistEmptyRgntOrDlt())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前试剂盘不存在可卸载的空瓶！")));
        pTipDlg->exec();
        return;
    }
	
    // 弹框提示是否确认卸载试剂
    TipDlg confirmTipDlg(QObject::tr("试剂卸载"), QObject::tr("确定卸载试剂瓶？"), TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel unload!");
        return;
    }

    // 执行卸载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::UnLoadReagent(_return, deviceSn, -1);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "UnLoadReagent failed, devSn:%s, -1.", deviceSn.c_str());
    }

    // 如果结果返回1，表示找不到要卸载的试剂
    if (_return.value == 1)
    {
        TipDlg(tr("未选择需要卸载的试剂，请重新选择！")).exec();
        return;
    }

    // 设置已启动卸载
    RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(deviceSn), true);
}

///
/// @brief
///     免疫合并加卸载按钮为试剂申请按钮
///
/// @param[in]  bCombine  是否合并
/// @param[in]  strDevSn  免疫设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2024年6月28日，新建函数
///
void ReagentShowList::CombineLoadUnloadBtn(bool bCombine, const std::string& strDevSn /*= ""*/)
{
    // 尝试合并加卸载按钮为【申请更换试剂】
    if (bCombine)
    {
        // 查询加载功能是否屏蔽
        bool bMask = false;
        if (!ReagentLoadIsMask(bMask, strDevSn))
        {
            ULOG(LOG_WARN, "ReagentLoadIsMask failed, devSn:%s.", strDevSn.c_str());
        }

        // 如果在线加载功能屏蔽，则合并加载和卸载按钮
        if (bMask)
        {
            ui->reagentUnload->setVisible(false);
            ui->reagentUploadBtn->setText(GetApplyChangeRgntStatus(strDevSn) == RGNT_CHANGE_APPLY_STATUS_APPLYING ? tr("取消申请") : tr("申请更换"));
        }
    }
    else
    {
        ui->reagentUnload->setVisible(true);
        ui->reagentUploadBtn->setText(tr("试剂装载"));
    }
}

///
/// @brief
///     免疫合并加卸载按钮为试剂申请按钮
///
/// @param[in]  vDevSn  免疫设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2024年6月28日，新建函数
///
void ReagentShowList::CombineLoadUnloadBtn(const std::vector<std::string>& vDevSn)
{
    if (vDevSn.empty())
    {
        return;
    }

    // 遍历设备
    bool bMask = true;
    for (const auto& strDevSn : vDevSn)
    {
        if (!ReagentLoadIsMask(bMask, strDevSn))
        {
            ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        }

        // 如果在线加载功能未屏蔽，则不合并加载和卸载按钮
        if (!bMask)
        {
            break;
        }
    }

    // 判断按钮是否屏蔽
    if (bMask)
    {
        ui->reagentUnload->setVisible(false);
        ui->reagentUploadBtn->setText(tr("申请更换"));
    }
    else
    {
        ui->reagentUnload->setVisible(true);
        ui->reagentUploadBtn->setText(tr("试剂装载"));
    }
}

///
/// @brief
///     获取剩余量的状态
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  reagent     试剂信息
///
/// @return 耗材状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
QStandardItem* ReagentShowList::GetBlanceStatus(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagent)
{
    QStandardItem* blanceItem = new QStandardItem();

    // 红色警告
    int blanceTimes = reagent.remainCount;
    if (blanceTimes <= 0)
    {
        SetTblItemState(blanceItem, STATE_WARNING);
    }
    else if (blanceTimes <= 9)
    {
        SetTblItemState(blanceItem, STATE_NOTICE);
    }

    blanceItem->setText(QString::number(blanceTimes));
    return blanceItem;
}

void ReagentShowList::UpdateChSuppliesToUI(const std::map<std::string, std::set<int>> &mapUpdateDevPos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 更新内容时锁定排序
	TblSortLockGuard tblSortLock(ui->tableView);

	// 将查询结果更新到表格中
	for (auto it = mapUpdateDevPos.begin(); it != mapUpdateDevPos.end(); it++)
	{
		// 全盘更新
		if (it->second.empty())
		{
			auto mapChSi = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(it->first);
			for (auto &si : mapChSi)
			{
				UpdateOneRowChSupplyInfo(it->first, si.second.supplyInfo, si.second.reagentInfos);
			}
		}
		// 更新指定位置
		else
		{
			auto mapChSi = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(it->first, it->second);
			for (auto pos : it->second)
			{
				// 已卸载
				if (!mapChSi.count(pos))
				{
					// 获取当前位置对应的行数
					std::vector<int> vRows = GetRowByPostion(pos, it->first);
					int rowCount = vRows.size();
					// 已卸载的显示为空白
					for (int i = 0; i < rowCount; i++)
					{
						int iRow = vRows[i];
						if (i == 0)
						{
							ShowBlankLine(iRow, pos, pos, it->first);
						}
						else
						{
							// 如果得到的行数≥2，需删除多余的行
							m_reagentModel->removeRow(iRow);
						}
					}
				}
				else
				{
					UpdateOneRowChSupplyInfo(it->first, mapChSi[pos].supplyInfo, mapChSi[pos].reagentInfos);
				}
			}
		}
    }
}

void ReagentShowList::UpdateOneRowChSupplyInfo(const std::string& devSN, const ch::tf::SuppliesInfo& supplyInfo,
	const std::vector<::ch::tf::ReagentGroup>& reagentInfos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	int pos = supplyInfo.pos;
	if (pos > ::ch::c1005::tf::g_c1005_constants.REAGENT_DISK_SLOT)
	{
		return;
	}

	// 获取待更新的行号
	std::vector<int> suppliesRows = GetRowByPostion(pos, devSN);
	if (suppliesRows.empty())
	{
		return;
	}

	// 无效耗材的情况
	if (supplyInfo.suppliesCode == -1)
	{
		for (int row : suppliesRows)
		{
			QString strKey = ShowScanFailedLine(pos, QString::fromStdString(devSN));
			m_reagCacheMgr.m_reagentInfoCache[strKey] = std::make_shared<ReagDataChSup>(supplyInfo);
		}
	}
	// 试剂
	else if (supplyInfo.type == ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
	{
		// 根据耗材获取试剂组信息
		if (reagentInfos.empty())
		{
			for (int row : suppliesRows)
			{
				// 测试要求没有试剂信息的时候要求显示耗材名称
				CoverChSupplies(row, supplyInfo);
			}

			ULOG(LOG_WARN, "Empty ReagentGroup, just add supplies name");
			return;
		}

		// 双向同侧的切换
		if (reagentInfos.size() != suppliesRows.size())
		{
			DoChangeAssays(supplyInfo, reagentInfos, suppliesRows);
			return;
		}

        // 依次更新每一行
        int i = 0;
		for (const auto& reagent : reagentInfos)
		{
			if (suppliesRows[i] != -1)
			{
				ShowReagnetItem(Ur_Cover, supplyInfo, reagent);
			}
			i++;
		}
	}
	// 非试剂的处理
	else
	{
		for (int row : suppliesRows)
		{
			ShowSupplyItem(supplyInfo);
		}
	}
}

void ReagentShowList::UpdateConsumeTableCh(const std::vector<std::string>& deviceSnList)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 参数检查
    if (deviceSnList.empty())
    {
        ULOG(LOG_INFO, "%s(), deviceSnList.empty()", __FUNCTION__);
        return;
    }

    // 刷新所有设备的试剂
	std::map<std::string, std::set<int>> mapDevPos;
    for (const std::string& strDevSn : deviceSnList)
    {
		mapDevPos[strDevSn] = std::set<int>();
    }

	UpdateChSuppliesToUI(mapDevPos);

    // 更新耗材信息
    ResizeTblColToContentAfterInit(ui->tableView);
}

///
/// @brief
///     更新单条试剂信息
///
/// @param[in]  reagentInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月11日，新建函数
///
void ReagentShowList::UpdateSingleRegntInfo(const ::im::tf::ReagentInfoTable& stuRgntInfo)
{
    ULOG(LOG_INFO, "%s(devSN:%s, pos:%d, name:%s)", __FUNCTION__, stuRgntInfo.deviceSN.c_str(), stuRgntInfo.reagentPos, stuRgntInfo.name.c_str());

    // 查找对应试剂位置所在行
    std::vector<int> rowsOfPosition = GetRowByPostion(stuRgntInfo.reagentPos, stuRgntInfo.deviceSN);
    if (rowsOfPosition.size() == 0)
    {
        ULOG(LOG_ERROR, "Invalid reagent position.");
        return;
    }

    // 批号序列号同时为空判定扫描失败
    if (stuRgntInfo.reagentLot.empty() && stuRgntInfo.reagentSN.empty())
    {
        for (int row : rowsOfPosition)
        {
            QString strKey = ShowScanFailedLine(stuRgntInfo.reagentPos, QString::fromStdString(stuRgntInfo.deviceSN));
            m_reagCacheMgr.m_reagentInfoCache[strKey] = std::make_shared<ReagDataIm>(stuRgntInfo);
        }
        return;
    }

    // 添加数据
    ShowReagnetItemImm(stuRgntInfo);
}

///
/// @brief
///     更新单条稀释液信息
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月28日，新建函数
///
void ReagentShowList::ShowSingleDltInfoImm(const ::im::tf::DiluentInfoTable& stuDltInfo)
{
    ULOG(LOG_INFO, "%s(name:%s, pos:%d.)", __FUNCTION__, stuDltInfo.diluentName.c_str(), stuDltInfo.pos);
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 查找对应试剂位置所在行
    std::vector<int> rowsOfPosition = GetRowByPostion(stuDltInfo.pos, stuDltInfo.deviceSN);
    if (rowsOfPosition.size() == 0)
    {
        ULOG(LOG_ERROR, "Invalid diluent position.");
        return;
    }

    // 批号序列号同时为空判定扫描失败
    if (stuDltInfo.dluentLot.empty() && stuDltInfo.diluentSN.empty())
    {
        QString strKey = ShowScanFailedLine(stuDltInfo.pos, QString::fromStdString(stuDltInfo.deviceSN));
        m_reagCacheMgr.m_reagentInfoCache[strKey] = std::make_shared<ReagDataImDilu>(stuDltInfo);
        return;
    }

    QList<QStandardItem*> itemList;
    std::shared_ptr<BaseReagData> rItem = std::make_shared<ReagDataImDilu>(stuDltInfo);

    std::shared_ptr<BaseReagData::RowContentInfo> rdt = rItem->GetRowContent();
    rdt->GetStandardItemList(itemList);
    m_reagCacheMgr.m_reagentInfoCache[rdt->GetUniqKey()] = rItem;

    // 设置行样式
    bool isReagMask = (stuDltInfo.statusForUI.status == im::tf::StatusForUI::type::REAGENT_STATUS_MASK);
    bool isCaliMask = (stuDltInfo.statusForUI.status == im::tf::StatusForUI::type::REAGENT_STATUS_CALI_MASK);
    SetStanterdRowStyle(itemList, isReagMask, isCaliMask);

    // 覆盖刷新界面对应试剂行
    for (int column = 0; column < itemList.size(); column++)
    {
        m_reagentModel->setItem(rowsOfPosition[0], column, itemList[column]);
    }

    // 设置排序值
    SetSortValue(itemList, rowsOfPosition[0]);
}

///
/// @brief
///     更新免疫稀释液信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月28日，新建函数
///
void ReagentShowList::UpdateImDltInfo(const std::vector<std::string>& deviceSn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 构造查询条件和查询结果
    ::im::tf::DiluentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 筛选条件
    qryCond.__set_deviceSNs(deviceSn);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryDiluentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDiluentInfoForUI failed!");
        return;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::DiluentInfoTable& stuDltInfo : qryResp.lstDiluentInfos)
    {
        // 更新单条质控信息
        ShowSingleDltInfoImm(stuDltInfo);
    }
}

void ReagentShowList::UpdateConsumeTableImm(const std::vector<std::string>& deviceSnList)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (deviceSnList.empty())
    {
        ULOG(LOG_INFO, "%s(), deviceSnList.empty()", __FUNCTION__);
        return;
    }

    // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    qryCond.__set_deviceSNs(deviceSnList);
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::ReagentInfoTable& stuRgntInfo : qryResp.lstReagentInfos)
    {
        UpdateSingleRegntInfo(stuRgntInfo);
    }

    // 更新稀释液信息
    UpdateImDltInfo(deviceSnList);
    ResizeTblColToContentAfterInit(ui->tableView);
}

void ReagentShowList::UpdateRowsToEmpty(const std::string& deviceSn, int reagentPos)
{
    bool Isfirst = true;
    std::vector<int> rows = GetRowByPostion(reagentPos, deviceSn);

    // 从大到小来排序
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (auto row : rows)
    {
        if (row == -1)
        {
            continue;
        }

        // 设置为空白行--第一行
        if (Isfirst)
        {
            ShowBlankLine(row, reagentPos, reagentPos, deviceSn);
            Isfirst = false;
            continue;
        }

        // 删除对应的行
        m_reagentModel->removeRow(row);
    }
}

///
/// @brief
///     更新试剂信息
///
/// @param[in]  deviceSN  设备编号
/// @param[in]  positions  位置信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月20日，新建函数
///
void ReagentShowList::UpdateReagentInformation(const std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>>& supplyUpdates)
{
	ULOG(LOG_INFO, u8"试剂列表收到刷新消息 %s(%s)", __FUNCTION__, ToString(supplyUpdates));

    // 对应的模块为空
    if (m_reagentModel == nullptr || !isVisible())
    {
        m_needUpdate = true;
        return;
    }

    // 记录更新内容
    std::map<std::string, std::set<int>> mapDevSnUpdatePos;
    for (const auto& supplyUpdate : supplyUpdates)
    {
        // 仓外耗材不再此处更新，或是否属于刷新范围
        if (supplyUpdate.posInfo.area == ch::tf::SuppliesArea::type::SUPPLIES_AREA_CABINET)
        {
            continue;
        }

		// 设备不在刷新范围内
		auto iter = std::find_if(m_deivces.begin(), m_deivces.end(), [=](std::shared_ptr<const tf::DeviceInfo>& spDev){
			return spDev->deviceSN == supplyUpdate.deviceSN;
		});

		if (iter == m_deivces.end())
		{
			continue;
		}

        // 记录要更新的耗材
        auto it = mapDevSnUpdatePos.find(supplyUpdate.deviceSN);
        if (it == mapDevSnUpdatePos.end())
        {
            // 没有则插入
            mapDevSnUpdatePos[supplyUpdate.deviceSN] = std::set<int>();
        }
		mapDevSnUpdatePos[supplyUpdate.deviceSN].insert(supplyUpdate.posInfo.pos);

		// 获取当前位置上的耗材信息
		auto currSupplyInfo = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(
			supplyUpdate.deviceSN, { supplyUpdate.posInfo.pos });
		if (currSupplyInfo.empty())
		{
			continue;
		}

		// 获取同组的位置，为了刷新可用测试数
		for (const auto &si : currSupplyInfo)
		{
			// 仓内试剂
			if (si.second.supplyInfo.type == ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
			{
				auto vecReagents = CommonInformationManager::GetInstance()->GetChGroupReagents(
					supplyUpdate.deviceSN, si.second.supplyInfo.suppliesCode);

				for (const auto &reagent : vecReagents)
				{
					mapDevSnUpdatePos[supplyUpdate.deviceSN].insert(reagent.posInfo.pos);
				}
			}
			// 仓内耗材
			else
			{
				auto vecSupplies = CommonInformationManager::GetInstance()->GetChGroupSupplies(
					supplyUpdate.deviceSN, si.second.supplyInfo.suppliesCode);

				for (const auto &supply : vecSupplies)
				{
					mapDevSnUpdatePos[supplyUpdate.deviceSN].insert(supply.pos);
				}
			}
		}
    }

	// 刷新生化耗材
	UpdateChSuppliesToUI(mapDevSnUpdatePos);

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);

    // 更新按钮状态
    UpdateBtnEnabled();
}

///
/// @brief
///     筛选按钮点击事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月9日，新建函数
///
void ReagentShowList::OnSelectConditionDialog()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_selectDialog == nullptr)
    {
        m_selectDialog = new QSelectConditionDialog(this);
        connect(m_selectDialog, SIGNAL(SendSearchCondition(std::shared_ptr<SelectConditions>)), \
            this, SLOT(OnConditionSearchUpdate(std::shared_ptr<SelectConditions>)));
    }

    m_selectDialog->InitConditionCheck(m_selectCondition, m_deivces);
    m_selectDialog->show();
}

void ReagentShowList::UpdateBySelectedCondidionsCh(const std::shared_ptr<SelectConditions>& cond, \
    std::vector<std::string>& deviceSn)
{
    ULOG(LOG_INFO, __FUNCTION__);

	/* 不能使用数据库查，"在用、备用、试剂屏蔽、校准屏蔽、未放置、空"为或的关系，数据库sql查询无法实现
	数据库的未放置和在用备用、试剂屏蔽、校准屏蔽已经是多个字段了，变成了与的关系，需要自己手写过滤  */

	// 遍历设备
	for (const auto &devSn : deviceSn)
	{
		// 获取指定设备的耗材试剂信息
		auto mapChSi = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(devSn);
		for (const auto &pair : mapChSi)
		{
			auto supply = pair.second.supplyInfo;
			auto rgtInfos = pair.second.reagentInfos;

			// 扫描失败特殊处理
			if (supply.suppliesCode <= 0)
			{
				// 判断其他“与”条件是否选中
				if (cond->m_scanFailed && !cond->SelCaliStatusIf() && !cond->SelUseStatusIf())
				{
					QString strKey = ShowScanFailedLine(supply.pos, QString::fromStdString(supply.deviceSN));
					m_reagCacheMgr.m_reagentInfoCache[strKey] = std::make_shared<ReagDataChSup>(supply);
				}

				continue;
			}

			// 未放置
			bool notPlace = cond->m_notPlace && supply.placeStatus == ::tf::PlaceStatus::type::PLACE_STATUS_REGISTER;

			// 耗材
			if (supply.type != ::ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
			{
				// 扫描失败提前处理了，筛选校准状态时不显示耗材，开放封闭也不显示
				if (cond->m_scanFailed || cond->m_caliSuccess || cond->m_caliing || cond->m_caliFailed || cond->m_notyetCali
					|| cond->m_caliEdit || cond->m_caliStatusEmpty || cond->m_openType || cond->m_closeType)
				{
					continue;
				}

				// 或条件判断
				bool inUse = cond->m_inUse && supply.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_CURRENT;
				bool backup = cond->m_backup && supply.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_BACKUP;
				bool empty = cond->m_usageStatusEmpty && supply.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_CAN_NOT_USE;
				if (inUse || backup || notPlace || empty)
				{
					ShowSupplyItem(supply);
				}
			}
			// 试剂
			else
			{
				// deviceSn+Pos
				std::set<std::string> existSamePos; 
				for (const auto &rgt : rgtInfos)
				{
					// 使用状态相关
					bool inUse = cond->m_inUse && rgt.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_CURRENT;
					bool backup = cond->m_backup && rgt.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_BACKUP;
					bool empty = cond->m_usageStatusEmpty && rgt.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_CAN_NOT_USE
						&& !rgt.reagentMask && !rgt.caliMask && supply.placeStatus != ::tf::PlaceStatus::type::PLACE_STATUS_REGISTER;
					bool reagentMask = cond->m_reagentShield && rgt.reagentMask;
					bool caliMask = cond->m_caliShield && rgt.caliMask;

					// 校准状态相关
					bool caliSuccess = cond->m_caliSuccess && rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_SUCCESS;
					bool caliFailed = cond->m_caliFailed && rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_FAIL;
					bool caliEdit = cond->m_caliEdit && rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_EDIT;
					bool caliing = cond->m_caliing && rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_DOING;
					bool caliNot = cond->m_notyetCali && rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT;
					bool caliEmpty = cond->m_caliStatusEmpty && 
						(rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_EMPTY || 
							(rgt.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT && rgt.caliCurveId > 0));

					// sind校准状态永远为空
					if (rgt.suppliesCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
					{
						caliSuccess = false;
						caliFailed = false;
						caliEdit = false;
						caliing = false;
						caliNot = false;
						caliEmpty = cond->m_caliStatusEmpty;
					}

					// 试剂类型相关
					bool openReagent = cond->m_openType && rgt.openReagent;
					bool closeReagent = cond->m_closeType && !rgt.openReagent;

					// 如果不满足筛选条件
					if (cond->SelUseStatusIf() && !inUse && !backup && !empty && !reagentMask && !caliMask && !notPlace)
					{
						continue;
					}
					if (cond->SelCaliStatusIf() && !caliSuccess && !caliFailed && !caliEdit && !caliing && !caliNot && !caliEmpty)
					{
						continue;
					}
					if (cond->SelReagentTypeIf() && !openReagent && !closeReagent)
					{
						continue;
					}

					// 如果筛选存在多个相同位置，则insert（双项同测项目
					std::string uniqKey = supply.deviceSN + "+" + std::to_string(supply.pos);
					if (existSamePos.find(uniqKey) != existSamePos.end())
					{
						ShowReagnetItem(Ur_Insert, supply, rgt);
					}
					else
					{
						existSamePos.insert(uniqKey);
						ShowReagnetItem(Ur_Cover, supply, rgt);
					}
				}
			}
		}
	}
}

void ReagentShowList::UpdateBySelectedCondidionsIm(const std::shared_ptr<SelectConditions>& cond, std::vector<std::string>& deviceSn)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (cond == nullptr)
    {
        ULOG(LOG_WARN, "Null select conditions param.");
        return;
    }

    ::im::tf::ReagTableUIQueryCond qryCond;
    cond->CovertTo(qryCond);
    qryCond.__set_deviceSNs(deviceSn);

    // 执行查询
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return;
    }

    // 将筛选结果添加到表格中
    for (const im::tf::ReagentInfoTable& stuRgntInfo : qryResp.lstReagentInfos)
    {
        // 更新单条试剂信息
        UpdateSingleRegntInfo(stuRgntInfo);
    }

    // 查询所有耗材信息
    im::tf::DiluentInfoTableQueryResp qryDiluResp;
    bRet = ::im::LogicControlProxy::QueryDiluentInfoForUI(qryDiluResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDiluentInfoForUI failed.");
        return;
    }

    // 将耗材信息加载到试剂盘中
    for (const im::tf::DiluentInfoTable& stuDltInfo : qryDiluResp.lstDiluentInfos)
    {
        // 更新单条耗材信息
        ShowSingleDltInfoImm(stuDltInfo);
    }
}

///
/// @brief
///     根据条件更新界面的试剂列表数据
///
/// @param[in]  searchCondtion  查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月10日，新建函数
///
void ReagentShowList::OnConditionSearchUpdate(std::shared_ptr<SelectConditions> searchCondtion)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (searchCondtion == nullptr)
    {
        ULOG(LOG_WARN, "Null search condition param.");
        return;
    }

    m_selectCondition = searchCondtion; // 缓存筛选条件

    // 如果筛选条件为0，则关闭筛选Label
    int conditionCnt = searchCondtion->CountTrue();
    if (conditionCnt == 0)
    {
        emit SentHideFilterLable();
        OnRefreshReagentList();
        return;
    }

    std::vector<std::string> deviceSnListCh;
    std::vector<std::string> deviceSnListIm;
    GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);
    CleanTableview();

    // 发送筛选条件个数的信号
    QString strTip = QString(tr("筛选：%1个筛选条件|")).arg(conditionCnt) + searchCondtion->toString();
    emit SendCondMessage(strTip);

    // 更加查询结果更新界面
    if (deviceSnListCh.size() > 0)
    {
        UpdateBySelectedCondidionsCh(searchCondtion, deviceSnListCh);
    }
    if (deviceSnListIm.size() > 0)
    {
        UpdateBySelectedCondidionsIm(searchCondtion, deviceSnListIm);
    }

    //无效index或NoOrder就设置为默认未排序状态
    if (m_iSortColIdx < 0 || m_iSortOrder == SortHeaderView::NoOrder) {
        ui->tableView->sortByColumn(RCI_ReagentPos, Qt::AscendingOrder);
        m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    }
    else
    {
        Qt::SortOrder qOrderFlag = m_iSortOrder == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->tableView->sortByColumn(m_iSortColIdx, qOrderFlag);
    }
}

///
/// @brief
///     试剂屏蔽
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月21日，新建函数
///
void ReagentShowList::OnShieldReagent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 构造试剂屏蔽文本
    bool bRgntMasked = false;

    int rowIdx = GetCurrentNoEmptyRowIndex();
    if (rowIdx == -1)
    {
        ULOG(LOG_ERROR, "Invalid row index, at OnShieldReagent.");
        return;
    }

    std::shared_ptr<BaseReagData> reagData = GetReagCache(rowIdx);
    if (reagData == nullptr)
    {
        return;
    }
    // 执行试剂屏蔽
    reagData->ProcShieldReagent();

    // 更新试剂屏蔽文本
    ui->reagentshield->setText(reagData->IsMaskReagent() ? tr("解除屏蔽") : tr("试剂屏蔽"));
}

int ReagentShowList::GetCurrentNoEmptyRowIndex()
{
    QModelIndexList curSelectedRows = ui->tableView->selectionModel()->selectedRows();
    if (curSelectedRows.size() == 0)
    {
        return -1;
    }

    // 免疫 预处理试剂多选行时,使用点击的行。
    if (curSelectedRows.size() > 1 && !m_curSelectedKeyString.isEmpty())
    {
        // 判断是否存在排序后的变动错开了选中序号，如果错开了就使用多选的第一个进行处理
        for (const QModelIndex& mi : curSelectedRows)
        {
            if (m_curSelectedKeyString == GetUniqIdentify(mi.row()))
            {
                ULOG(LOG_INFO, "%s, chocie row %d by %s.", __FUNCTION__, mi.row(), m_curSelectedKeyString.toStdString().c_str());
                return mi.row();
            }
        }
    }

    // 超出范围了
    int rowIndex = curSelectedRows[0].row();
    if (rowIndex > m_reagentModel->rowCount())
    {
        return -1;
    }

    // 若是空行，不要进行操作
    QStandardItem* item0 = m_reagentModel->item(rowIndex);
    if (item0 == nullptr || item0->data(USER_EMPTY_ROW_FLAG_ROLE).toBool())
    {
        return -1;
    }

    return rowIndex;
}

///
/// @brief
///     获取当前选中行
///
/// @return 当前选中行
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月6日，新建函数
///
int ReagentShowList::GetCurrentRowIndex()
{
    QModelIndexList curSelectedRows = ui->tableView->selectionModel()->selectedRows();
    if (curSelectedRows.size() == 0)
    {
        return -1;
    }
    int rowIndex = curSelectedRows[0].row();

    // 免疫 预处理试剂多选行时,使用点击的行。
    if (curSelectedRows.size() > 1 && !m_curSelectedKeyString.isEmpty())
    {
        // 判断是否存在排序后的变动错开了选中序号，如果错开了就使用多选的第一个进行处理
        for (const QModelIndex& mi : curSelectedRows)
        {
            if (m_curSelectedKeyString == GetUniqIdentify(mi.row()))
            {
                return mi.row();
            }
        }
    }

    // 超出范围了
    if (rowIndex > m_reagentModel->rowCount())
    {
        return -1;
    }

    return rowIndex;
}

bool ReagentShowList::GetPositionInfo(int rowIndex, std::string& deviceSn, int& reagentPos)
{
    QString posStr = m_reagentModel->item(rowIndex, 0)->text();
    auto itDev = m_rowInforMap.find(posStr);
    if (itDev == m_rowInforMap.end() || itDev->second.m_dev == nullptr)
    {
        return false;
    }

    deviceSn = itDev->second.m_dev->deviceSN;
    reagentPos = itDev->second.m_rowPos;

    return true;
}

///
/// @brief
///     优先使用
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void ReagentShowList::OnPriorityReagent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取当前行号
    int rowIdx = GetCurrentNoEmptyRowIndex();
    if (rowIdx == -1)
    {
        ULOG(LOG_ERROR, "Invalid row index.");
        return;
    }

    // 弹框提示是否确认试剂屏蔽
    QStandardItem* itemName = m_reagentModel->item(rowIdx, RCI_Name);
	if (itemName != nullptr && !IsReagentPriortyTipDlg(true))
    {
        ULOG(LOG_INFO, "Call isreagentprioritytipdlg return negetive!");
        return;
    }

    // 获取试剂对象
    std::shared_ptr<BaseReagData> reagData = GetReagCache(rowIdx);
    if (reagData == nullptr)
    {
        return;
    }
    reagData->ProcPriorityReagent();

    // 更新按钮使用状态
    UpdateBtnEnabled();
}

void ReagentShowList::UnloadReagentIm(const std::shared_ptr<const tf::DeviceInfo>& stuTfDevInfo, int rowIdx, const QString& strReg)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (stuTfDevInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null devinfo param.");
        return;
    }

    // 检查是否屏蔽试剂在线加载功能，如果屏蔽，则卸载按钮功能为试剂申请 TODO
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo->deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        return;
    }

    // 屏蔽后进入申请更换试剂流程
    if (bMask)
    {
        // 判断当前申请更换试剂状态
        auto RgntApplyStatus = GetApplyChangeRgntStatus(stuTfDevInfo->deviceSN);

        // 如果已申请，则取消
        if (RgntApplyStatus == RGNT_CHANGE_APPLY_STATUS_APPLYING)
        {
            TipDlg tipDlg(tr("取消申请更换试剂"), tr("是否取消申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, stuTfDevInfo->deviceSN, false);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("取消申请更换试剂"), tr("取消申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(stuTfDevInfo->deviceSN, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }
        else
        {
            TipDlg tipDlg(tr("申请更换试剂"), tr("是否申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, stuTfDevInfo->deviceSN);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("申请更换试剂"), tr("申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(stuTfDevInfo->deviceSN, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }

        return;
    }

    std::shared_ptr<BaseReagData> reagData = GetReagCache(rowIdx);
    if (reagData == nullptr)
    {
        ULOG(LOG_WARN, "Not find reagent data in cache.");
        return;
    }

    // 获取试剂信息或耗材信息
    int64_t position = -1;
    im::tf::StatusForUI::type enUseState = im::tf::StatusForUI::REAGENT_STATUS_INVALID;
    ::tf::CaliStatus::type enCaliState = ::tf::CaliStatus::CALI_STATUS_NOT;
    if (reagData->m_myType == RT_IM)
    {
        // 获取试剂信息
        ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(reagData->m_reagent);

        // 判断是否可卸载
        if (PopUnloadRgntForbiddenTip(*stuTfDevInfo, reagentInfo))
        {
            ULOG(LOG_INFO, "Popup not readly to unload reagent.");
            return;
        }

        position = reagentInfo.reagentPos;
        enUseState = reagentInfo.reagStatusUI.status;
        enCaliState = reagentInfo.caliStatus;
    }
    else if (reagData->m_myType == RT_IMDIT)
    {
        // 获取稀释液信息
        ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(reagData->m_reagent);

        // 判断是否可卸载
        if (PopUnloadDltForbiddenTip(*stuTfDevInfo, reagentInfo))
        {
            ULOG(LOG_INFO, "Popup not readly to unload diluent.");
            return;
        }

        position = reagentInfo.pos;
        enUseState = reagentInfo.statusForUI.status;
    }
    // 卸载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
    ::tf::ResultBool _return_existReag;
    im::LogicControlProxy::QueryReagentLoadUnloadExistReagent(_return_existReag, stuTfDevInfo->deviceSN);
    if (_return_existReag.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return;
    }
    // 存在试剂盒，调用取消加载试剂指令（包含检测是否有试剂盒以及复位回收流程）
    if (_return_existReag.value)
    {
		// 设置已启动卸载
		RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(stuTfDevInfo->deviceSN), true);

        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, stuTfDevInfo->deviceSN);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return;
        }
        return;
    }

    // 弹框提示是否确认试剂卸载
    if (!IsReagentUnloadTipDlg())
    {
        return;
    }

    // 执行卸载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::UnLoadReagent(_return, stuTfDevInfo->deviceSN, position);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Unload immune reagent error!");
        TipDlg msgTpi(tr("试剂卸载失败！"));
        msgTpi.exec();
        return;
    }

    // 设置已启动卸载
    RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(stuTfDevInfo->deviceSN), true);
}

void ReagentShowList::UnloadReagentCh(const std::shared_ptr<const tf::DeviceInfo>& stuTfDevInfo, int rowIdx, const QString& strReg)
{
    ULOG(LOG_INFO, __FUNCTION__);

    std::shared_ptr<BaseReagData> reagData = GetReagCache(rowIdx);
    if (reagData == nullptr)
    {
        ULOG(LOG_WARN, "Not exist reagent in cache.");
        return;
    }

    // 卸载前确认询问
    if (!IsReagentUnloadTipDlg())
    {
        return;
    }

    ch::tf::SuppliesInfo reagentSupply = boost::any_cast<ch::tf::SuppliesInfo>(reagData->m_reagentSupply);
    if (!ch::c1005::LogicControlProxy::UnloadSupplies(stuTfDevInfo->deviceSN, std::vector<int>(1, reagentSupply.pos)))
    {
        ULOG(LOG_ERROR, "Unload chemical reagent error!");
        TipDlg msgTpi(tr("试剂卸载失败！"));
        msgTpi.exec();
    }
}

///
/// @brief
///     试剂卸载
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void ReagentShowList::OnReagentUnload()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 首先判断是否有效 或超出范围了
    int row = GetCurrentNoEmptyRowIndex();
    if (row == -1)
    {
        ULOG(LOG_ERROR, "Invlaid row index.");
        UnloadRgntInNoSelPos();
        return;
    }

    QString rowPosition = m_reagentModel->item(row, RCI_ReagentPos)->text();
    // 获取设备信息
    auto spDevice = GetDeviceByPositionName(rowPosition);
    if (spDevice == nullptr)
    {
        ULOG(LOG_WARN, "Null device ptr.");
        UnloadRgntInNoSelPos();
        return;
    }

    // 试剂名
    QStandardItem* itemName = m_reagentModel->item(row, RCI_Name);
    if (itemName == nullptr)
    {
        ULOG(LOG_ERROR, "Invalid item.");
        UnloadRgntInNoSelPos();
        return;
    }
    QString strReagetName = itemName->text();

    // 卸载免疫试剂
    if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        UnloadReagentIm(spDevice, row, strReagetName);
    }
    // 卸载生化试剂
    else
    {
        UnloadReagentCh(spDevice, row, strReagetName);
    }
}

void ReagentShowList::OnReagentUpload()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果设备序列号为空，则置为当前设备序列号
    std::string deviceSn;
    int reagentPos = -1;

    // 获取当前选中项索引(以获取选中设备和位置)    
    QModelIndex qIndex = ui->tableView->currentIndex();
    // 获取选中行对应的设备和位置
    int row = GetCurrentRowIndex();
    if (row >= 0 && !GetPositionInfo(row, deviceSn, reagentPos))
    {
        ULOG(LOG_INFO, "Failed to get deviceSN.");
    }

    // 如果没有指定设备，弹框提示
    // 根据当前选中设备，更新按钮显示
    std::vector<std::string> deviceSnListCh;
    std::vector<std::string> deviceSnListIm;
    GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);
    if (deviceSn.empty() && deviceSnListIm.size() != 1)
    {
        TipDlg(tr("请指定要进行试剂装载的设备。")).exec();
        return;
    }

    // 获取设备序列号
    if (deviceSn.empty())
    {
        deviceSn = deviceSnListIm.front();
    }

    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;
    devInfoCond.__set_deviceSN(deviceSn);

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed by devSn:%s!", deviceSn.c_str());
        return;
    }

    // 获取选中设备信息
    tf::DeviceInfo& stuTfDevInfo = devInfoResp.lstDeviceInfos[0];

	// 检查是否屏蔽试剂在线加载功能，如果屏蔽，则卸载按钮功能为试剂申请 TODO
	bool bMask = false;
	if (!ReagentLoadIsMask(bMask, deviceSn))
	{
		ULOG(LOG_WARN, "ReagentLoadIsMask failed, deviceSn:%s!", deviceSn.c_str());
		return;
	}

	// 屏蔽后进入申请更换试剂流程
	if (bMask)
	{
        // 判断当前申请更换试剂状态
        auto RgntApplyStatus = GetApplyChangeRgntStatus(deviceSn);

        // 如果已申请，则取消
        if (RgntApplyStatus == RGNT_CHANGE_APPLY_STATUS_APPLYING)
        {
            TipDlg tipDlg(tr("取消申请更换试剂"), tr("是否取消申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, deviceSn, false);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("取消申请更换试剂"), tr("取消申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(deviceSn, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }
        else
        {
            TipDlg tipDlg(tr("申请更换试剂"), tr("是否申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, deviceSn);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("申请更换试剂"), tr("申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(deviceSn, RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnEnabled();
        }

		return;
	}

    // 弹出禁止装载提示框
    if (PopLoadRgntForbiddenTip(stuTfDevInfo, reagentPos))
    {
        return;
    }

    // 检查是否存在试剂盒，如果不存在，则提醒用户——陈建林老师、郭静老师、甘鹏飞老师讨论后决定加载前检查
    bool bExist = false;
    do 
    {
        // 弹框提示是否确认加载试剂
        TipDlg tipDlg(tr("试剂装载"), tr("请将试剂盒放入自动加载处，完成后点击“确定”。"), TipDlgType::TWO_BUTTON);
        if (tipDlg.exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel PriorUse!");
			// 设置已启动卸载
			RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(deviceSn), true);

            // 取消加载试剂
            ::tf::ResultLong _return;
            bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, deviceSn);
            if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "CancelLoadReagent failed.");
            }
            return;
        }

        // 检查槽位是否存在试剂盒
        if (!ReagentLoadExistReag(bExist, deviceSn))
        {
            ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
            return;
        }
    } while (!bExist);

    // 加载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::LoadReagent(_return, deviceSn, reagentPos);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "LoadReagent failed.");
        return;
    }

	// 设置已启动卸载
	RgntNoticeDataMng::GetInstance()->SetStartUnload(QString::fromStdString(deviceSn), true);
}

///
/// @brief
///     试剂扫描
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void ReagentShowList::OnScanReagent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_scanDialog == nullptr)
    {
        m_scanDialog = new QReagentScanDialog(this);
    }
    m_scanDialog->show();
}

void ReagentShowList::SelectRelatedReagentIm(int rowIndex)
{
    ULOG(LOG_INFO, "%s, idx:%d.", __FUNCTION__, rowIndex);

    // 获取界面选择的免疫试剂项目
    std::shared_ptr<BaseReagData> reagData = GetReagCache(rowIndex);
    if (reagData == nullptr)
    {
        ULOG(LOG_WARN, "Not exist reaget cache data.");
        return;
    }
    if (reagData->m_myType != RT_IM)
    {
        return;
    }
    ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(reagData->m_reagent);
    QString matchedItemPos = m_reagCacheMgr.GetImRelativeReagent(reagentInfo);
    if (!matchedItemPos.isEmpty())
    {
        QList<QStandardItem*> relateItem = m_reagentModel->findItems(matchedItemPos);
        if (relateItem.size() == 0)
        {
            return;
        }
        int relateRow = relateItem[0]->row();

        // 选中关联行
        QItemSelection selctionItem2(m_reagentModel->index(relateRow, 0), m_reagentModel->index(relateRow, 1));
        ui->tableView->selectionModel()->select(selctionItem2, QItemSelectionModel::Rows | QItemSelectionModel::Select);

        // 选中当前行
        QItemSelection selctionItem(m_reagentModel->index(rowIndex, 0), m_reagentModel->index(rowIndex, 1));
        ui->tableView->selectionModel()->select(selctionItem, QItemSelectionModel::Rows | QItemSelectionModel::Select);

        ui->tableView->viewport()->update();
        ULOG(LOG_INFO, "selected,%d %d", rowIndex, relateRow);
    }
}

void ReagentShowList::OnTabelSelectChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取当前选择的行， (此处不能用currIndex.indexes()来获取当前选中的行，因为当第二次点击相同行时selectedList是0)
    QModelIndexList selectedList = ui->tableView->selectionModel()->selectedRows(); //currIndex.indexes();
    if (selectedList.size() == 0)
    {
        ULOG(LOG_INFO, "There have no selected rows.");
        UpdateBtnEnabled();
        return;
    }

    // 获取当前行
    int rowIdx = selectedList[0].row();
    if (selectedList.size() > 1)
    {
        int iTemp = GetCurrentRowIndex();
        rowIdx = iTemp >= 0 ? iTemp : rowIdx;
    }
    else
    {
        m_curSelectedKeyString = GetUniqIdentify(rowIdx);
    }

    // 获取当前设备信息
    QString positionText = m_reagentModel->item(rowIdx, RCI_ReagentPos)->text();
    std::shared_ptr<const tf::DeviceInfo> spDevice = GetDeviceByPositionName(positionText);
    if (!spDevice)
    {
        ULOG(LOG_ERROR, "Null device ptr.");
        return;
    }

    // 试剂加载按钮状态(生化没有关联行？)
    if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        // 免疫关联试剂选择
        SelectRelatedReagentIm(rowIdx);
    }

    // 更新按钮使能状态
    UpdateBtnEnabled();
}

void ReagentShowList::OnTableViewClicked(const QModelIndex& qIndex)
{
    if (qIndex.isValid())
    {
        m_curSelectedKeyString = GetUniqIdentify(qIndex.row());
    }
}

///
/// @brief
///     试剂指定位置扫描开始
///
/// @param[in]  strDevName  设备名
/// @param[in]  vecPos      指定位置
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月22日，新建函数
///
void ReagentShowList::OnRgntScanPosStart(QString strDevName, std::vector<int, std::allocator<int>> vecPos)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, "%s, dev:%s", __FUNCTION__, strDevName.toStdString().c_str());

    // 获取设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(strDevName);

    // 将设备对应的所有行清空（保留位置信息）
    for (int iPos : vecPos)
    {
        std::vector<int> vRows = GetRowByPostion(iPos, strDevSn.toStdString());
        for (int iRow : vRows)
        {
            ShowBlankLine(iRow, iPos, iPos, strDevSn.toStdString());
        }
    }

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);
}

///
/// @brief
///     通过显示设置更新试剂信息表
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月13日，新建函数
///
void ReagentShowList::UpdateRgntTblByDisplayCfg()
{
    ULOG(LOG_INFO, __FUNCTION__);
    auto funcPreCmp = [](const DisplaySetItem& stuItem1, const DisplaySetItem& stuItem2)
    {
        // 显示的排前面
        if (stuItem1.enSelect != stuItem2.enSelect)
        {
            return !stuItem1.enSelect;
        }

        // 列数小的排前面
        return (stuItem1.postion < stuItem2.postion);
    };

    // 优先排序
    std::sort(m_stuDisplayCfg.displayItems.begin(), m_stuDisplayCfg.displayItems.end(), funcPreCmp);

    // 根据列名获取列索引
    auto funcGetColIdx = [this](const QString& strHeaderText)
    {
        for (int iCol = 0; iCol < m_reagentModel->columnCount(); iCol++)
        {
            QStandardItem* pHeaderItem = m_reagentModel->horizontalHeaderItem(iCol);
            if (pHeaderItem == Q_NULLPTR)
            {
                continue;
            }

            QString strItemText = pHeaderItem->text();
            if (pHeaderItem->text() != strHeaderText)
            {
                continue;
            }

            return iCol;
        }

        return -1;
    };

    // 排序完成后先设置位置
    for (const DisplaySetItem& stuColItem : m_stuDisplayCfg.displayItems)
    {
        // 获取对应列索引
        int iCol = stuColItem.type;
        if (iCol < 0)
        {
            continue;
        }

        // 设置列显示和隐藏
        if (!stuColItem.enSelect)
        {
            if (!ui->tableView->isColumnHidden(iCol))
            {
                ui->tableView->hideColumn(iCol);
            }
            continue;
        }

        // 显示列
        if (ui->tableView->isColumnHidden(iCol))
        {
            ui->tableView->showColumn(iCol);
        }

        // 设置位置
        ui->tableView->horizontalHeader()->moveSection(ui->tableView->horizontalHeader()->visualIndex(iCol), stuColItem.postion);
    }
}

///
/// @brief
///     显示设置更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月13日，新建函数
///
void ReagentShowList::OnDisplayCfgUpdate()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 根据显示设置更新界面
    LoadDisplayCfg();
    UpdateRgntTblByDisplayCfg();

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);
}

///
/// @brief
///     试剂仓加卸载是否正在运行
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bRunning  是否正在运行
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月30日，新建函数
///
void ReagentShowList::OnRgntLoadStatusChanged(QString strDevSn, bool bRunning)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, "%s(%s,%d)", __FUNCTION__, strDevSn.toStdString().c_str(), int(bRunning));
    // 更新加卸载试剂的按钮状态
    UpdateBtnEnabled();
}

///
/// @brief
///     设备状态改变
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
void ReagentShowList::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, __FUNCTION__);
    Q_UNUSED(deviceInfo);
    UpdateBtnEnabled();
}

///
/// @brief
///     设备温度异常状态改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  bErr        是否温度异常
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月30日，新建函数
///
void ReagentShowList::OnDevTemperatureErrChanged(QString strDevSn, bool bErr)
{
    ULOG(LOG_INFO, __FUNCTION__);
    Q_UNUSED(strDevSn);
    Q_UNUSED(bErr);
    UpdateBtnEnabled();
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月4日，新建函数
///
void ReagentShowList::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, __FUNCTION__);
    Q_UNUSED(mapUpdateInfo);
    RefreshModeReagentList();
}

///
/// @brief
///     更新加卸载按钮使能状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月31日，新建函数
///
void ReagentShowList::UpdateBtnEnabled()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 如果界面未显示则忽略
    if (!isVisible())
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 默认装载卸载按钮是分开的
    CombineLoadUnloadBtn(false);

    // 禁能按钮（必须选中才使能）
    ui->reagentshield->setEnabled(false);       // 屏蔽
    ui->reagentPriority->setEnabled(false);     // 优先
    ui->reagentUnload->setEnabled(false);       // 卸载 
    ui->reagentUploadBtn->setEnabled(false);    // 加载 
    ui->reagentRegister->setEnabled(false);     // 信息录入

    // 默认显示试剂屏蔽
    ui->reagentshield->setText(tr("试剂屏蔽"));

    // 试剂扫描按钮状态
    ui->reagentScan->setEnabled(IsExistDeviceCanScan());

    // 试剂混匀按钮状态
    ui->mixMagneticBtn->setEnabled(IsExistDeviceCanMix());

    // 装载装置复位按钮
    ui->LoadPartResetBtn->setEnabled(IsExistDeviceCanResetLoader());

    // 获取当前选择的行， (此处不能用currIndex.indexes()来获取当前选中的行，因为当第二次点击相同行时selectedList是0)
    QModelIndexList selectedList = ui->tableView->selectionModel()->selectedRows(); //currIndex.indexes();
    if (selectedList.size() == 0)
    {
        ULOG(LOG_INFO, "There have no selected rows.");
        // 没有选中项，如果存在唯一的免疫设备，依然判断加载按钮是否能使能
        // 根据当前选中设备，更新按钮显示
        std::vector<std::string> deviceSnListCh;
        std::vector<std::string> deviceSnListIm;
        GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);
        if (deviceSnListIm.size() != 1)
        {
            ULOG(LOG_INFO, "%s(), deviceSnListIm.size() != 1", __FUNCTION__);
            CombineLoadUnloadBtn(deviceSnListIm);
            return;
        }

        // 获取对应设备信息
        std::shared_ptr<const tf::DeviceInfo> spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSnListIm[0]);
        if (!spDevice)
        {
            ULOG(LOG_ERROR, "Not exist device:%s.", deviceSnListIm[0].c_str());
            return;
        }

        // 合并装载卸载按钮为【申请更换试剂】
        CombineLoadUnloadBtn(true, spDevice->deviceSN);

        // 更新加载按钮使能状态
        ui->reagentUploadBtn->setEnabled(IsEnableLoadRgnt(*spDevice));
        ui->reagentUnload->setEnabled(IsEnableUnloadRgnt(*spDevice));
        return;
    }

    // 获取选中行
    int rowIdx = selectedList[0].row();
    m_curSelectedKeyString = GetUniqIdentify(rowIdx);

    // 获取当前设备信息
    QString positionText = m_reagentModel->item(rowIdx, RCI_ReagentPos)->text();
    QString rgntNameText = (m_reagentModel->item(rowIdx, RCI_Name) == Q_NULLPTR) ? "" : m_reagentModel->item(rowIdx, RCI_Name)->text();
    QString rgntResidualText = (m_reagentModel->item(rowIdx, RCI_Residual) == Q_NULLPTR) ? "" : m_reagentModel->item(rowIdx, RCI_Residual)->text();

	// 获取设备序列号
	std::string devSN, devName;
	int pos = -1;
	GetPositionInfo(rowIdx, devSN, pos);
	if (devSN.empty() || pos == -1)
	{
		ULOG(LOG_ERROR, "GetPositionInfo by row: %d failed.", rowIdx);
		return;
	}

	// 使用GetDeviceByPositionName获取的设备指针里面的信息未更新
	// std::shared_ptr<tf::DeviceInfo> spDevice = GetDeviceByPositionName(positionText);
	auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(devSN);
    if (!spDevice)
    {
        ULOG(LOG_ERROR, "Null device ptr.");
        return;
    }

    // 试剂加载按钮状态
    if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        QStandardItem* item0 = m_reagentModel->item(rowIdx);
        if (item0 == nullptr || item0->data(USER_EMPTY_ROW_FLAG_ROLE).toBool()) // 空行
        {
            ui->reagentUploadBtn->setEnabled(IsEnableLoadRgnt(*spDevice));
            ui->reagentUnload->setEnabled(IsEnableUnloadRgnt(*spDevice));
        }

        // 合并装载卸载按钮为【申请更换试剂】
        CombineLoadUnloadBtn(true, spDevice->deviceSN);
    }

    // 获取界面选择的试剂项目
    std::shared_ptr<BaseReagData> reagData = m_reagCacheMgr.GetReagData(positionText, rgntNameText);
    if (reagData == nullptr)
    {
		// 空行允许信息录入
		if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
		{
			ui->reagentRegister->setEnabled(true);
		}

        return;
    }

    // 屏蔽、优先级、信息录入
    bool bRgntMasked = false;
    bool bPriorty = false;
	bool bRegister = false;

	// 生化试剂允许屏蔽
	bool bChRgtCanMask = true;

    // 试剂卸载按钮状态
    tf::UsageStatus::type useStat;
    tf::PlaceStatus::type placeStat;

	// 当前选中生化耗材或试剂、试剂组
	ch::tf::SuppliesInfo supplyInfo;
	ch::tf::ReagentGroup reagentGroupInfo;

    bool bImAndRgntEmpty(false);
    switch (reagData->m_myType)
    {
    case RT_IM:
    case RT_IMDIT:
    {
        // 查询加载功能是否屏蔽
        bool bMask = false;
        if (!ReagentLoadIsMask(bMask, spDevice->deviceSN))
        {
            ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        }

        // 屏蔽在线加载状态选中有试剂位置需要再判定
        if (bMask)
        {
            ui->reagentUploadBtn->setEnabled(IsEnableLoadRgnt(*spDevice));
        }

        bImAndRgntEmpty = (rgntResidualText.isEmpty() || (rgntResidualText == "0") || (rgntResidualText == "0ml"));
        ::im::tf::StatusForUI::type uiStat;
        if (reagData->m_myType == RT_IM)
        {
            ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(reagData->m_reagent);
            uiStat = reagentInfo.reagStatusUI.status;

            // 备用并且有工作曲线，使能优先使用按钮
            bPriorty = reagentInfo.reagStatusUI.status == im::tf::StatusForUI::REAGENT_STATUS_BACKUP && !reagentInfo.caliCurveExpInfo.empty();

            // 更新卸载按钮使能状态
            ui->reagentUnload->setEnabled(IsEnableUnloadRgnt(*spDevice, reagentInfo));
        }
        else
        {
            ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(reagData->m_reagent);
            uiStat = reagentInfo.statusForUI.status;

            // 备用状态则使能
            bPriorty = reagentInfo.statusForUI.status == im::tf::StatusForUI::REAGENT_STATUS_BACKUP;

            // 更新卸载按钮使能状态
            ui->reagentUnload->setEnabled(IsEnableUnloadDlt(*spDevice, reagentInfo));
        }
        bRgntMasked = (uiStat == im::tf::StatusForUI::REAGENT_STATUS_MASK);
        ui->reagentshield->setText(bRgntMasked ? tr("解除屏蔽") : tr("试剂屏蔽"));

        // 把::im::tf::StatusForUI转换为 tf::UsageStatus
        switch (uiStat)
        {
        case ::im::tf::StatusForUI::REAGENT_STATUS_ON_USE:
            useStat = tf::UsageStatus::USAGE_STATUS_CURRENT;
            break;
        case ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP:
            useStat = tf::UsageStatus::USAGE_STATUS_BACKUP;
            break;
        default:
            useStat = tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE;
            break;
        }

        // 免疫未加载的数据库中不存储，所以查出来的都是已加载的
        placeStat = tf::PlaceStatus::PLACE_STATUS_LOAD;
        break;
    }
    case RT_CH:
    {
		reagentGroupInfo = boost::any_cast<ch::tf::ReagentGroup>(reagData->m_reagent);
		supplyInfo = boost::any_cast<ch::tf::SuppliesInfo>(reagData->m_reagentSupply);
        useStat = reagentGroupInfo.usageStatus;
        placeStat = supplyInfo.placeStatus;
        bPriorty = useStat == tf::UsageStatus::USAGE_STATUS_BACKUP;
		bChRgtCanMask = (reagentGroupInfo.assayCode > 0 && reagentGroupInfo.modelUsed);

		// 如果选中 空行、扫描失败、开放试剂、正在校准的试剂不能信息录入
		if ((placeStat == tf::PlaceStatus::PLACE_STATUS_UNLOAD || supplyInfo.suppliesCode <= 0
			|| supplyInfo.suppliesCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE)
			&& reagentGroupInfo.caliStatus != tf::CaliStatus::CALI_STATUS_DOING)
		{
			bRegister = true;
		}
        break;
    }
    case RT_CHSUP:
    {
		supplyInfo = boost::any_cast<ch::tf::SuppliesInfo>(reagData->m_reagentSupply);
        useStat = supplyInfo.usageStatus;
        placeStat = supplyInfo.placeStatus;
        bPriorty = useStat == tf::UsageStatus::USAGE_STATUS_BACKUP;

		// 如果选中空行 或者扫描失败
		if (placeStat == tf::PlaceStatus::PLACE_STATUS_UNLOAD
			|| supplyInfo.suppliesCode <= 0)
		{
			bRegister = true;
		}
        break;
    }
    default:
        break;
    }

    // 生化的卸载按钮使能状态单独处理
    if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
    {
        ui->reagentUnload->setEnabled(UnloadReagentStat(*spDevice, supplyInfo, reagentGroupInfo));
    }

    // 试剂优先级按钮状态
    ui->reagentPriority->setEnabled(bPriorty);

    // 在用试剂不能屏蔽--运行状态下, 1）试剂才有屏蔽和解除 2）必须有放置
    // 试剂屏蔽--允许(耗材状态必须处于Load状态)
    if (ShieldReagentStat(*spDevice, placeStat) && (rgntNameText != tr("扫描失败")) && !bImAndRgntEmpty)
    {
        ui->reagentshield->setEnabled(true);

        // 试剂卸载按钮状态
        auto itemStatus = m_reagentModel->item(rowIdx, RCI_Statu);
        if (itemStatus == nullptr)
        {
            return;
        }
        QString strStatCell = itemStatus->text();
        QString shieldStr = strStatCell.contains(tr("试剂屏蔽")) ? tr("解除屏蔽") : tr("试剂屏蔽");
        ui->reagentshield->setText(shieldStr);
    }

    // 非耗材未放置
    if (reagData->m_myType == RT_CHSUP || !bChRgtCanMask
		|| placeStat == tf::PlaceStatus::PLACE_STATUS_UNLOAD 
		|| placeStat == tf::PlaceStatus::PLACE_STATUS_REGISTER)
    {
        ui->reagentshield->setText(tr("试剂屏蔽"));
        ui->reagentshield->setEnabled(false);
    }

	// 生化信息录入
	if (spDevice->deviceType == tf::DeviceType::DEVICE_TYPE_C1000 && bRegister)
	{
		ui->reagentRegister->setEnabled(true);
	}
}

///
/// @brief
///     根据配置更新表格显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月6日，新建函数
///
void ReagentShowList::UpdateTblDisplayByCfg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    LoadDisplayCfg();
    UpdateRgntTblByDisplayCfg();
    ResizeTblColToContentAfterInit(ui->tableView);
}

///
/// @brief	设备信息更新
///     
/// @par History:
/// @li 7951/LuoXin，2023年6月5日，新建函数
/// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
///
void ReagentShowList::UpdateDeviceStatus(tf::DeviceInfo deviceInfo)
{
    Q_UNUSED(deviceInfo);
    UpdateBtnEnabled();
}

///
/// @brief 信息录入槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月13日，新建函数
///
void ReagentShowList::onRegisterClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_registerDialog == nullptr)
	{
		m_registerDialog = new QRegisterSupplyInfo(this);
	}

	// 都是仓内耗材和试剂,超出范围了
	int row = ui->tableView->currentIndex().row();
	if (row > m_reagentModel->rowCount() || m_reagentModel->item(row) == nullptr)
	{
		ULOG(LOG_WARN, "Invalid reagent current index.");
		return;
	}

	// 获取设备序列号，设备名称，位置信息
	std::string devSN,devName;
	int pos = -1;
	GetPositionInfo(row, devSN, pos);
	if (devSN.empty() || pos == -1)
	{
		ULOG(LOG_ERROR, "GetPositionInfo by row: %d failed.", row);
		return;
	}

	// 获取设备名+“-”+位置
	auto pItem = m_reagentModel->item(row, RCI_ReagentPos);
	if (pItem == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "m_reagentModel->item(%d, RCI_ReagentPos) is nullptr.", row);
		return;
	}

	// 获取界面选择的试剂项目
	std::shared_ptr<BaseReagData> reagData = GetReagCache(row);
	// 空位或者扫描失败
	if (reagData == nullptr)
	{
		ch::tf::SuppliesInfo si;
		si.pos = pos;
		m_registerDialog->SetSupplyPostion(si, devSN, pItem->text().toStdString());
		m_registerDialog->show();
		return;
	}

	// 生化试剂或耗材
	if (reagData->m_myType == RT_CH || reagData->m_myType == RT_CHSUP)
	{
		ch::tf::SuppliesInfo reagentSupply = boost::any_cast<ch::tf::SuppliesInfo>(reagData->m_reagentSupply);
		m_registerDialog->SetSupplyPostion(reagentSupply, devSN, pItem->text().toStdString());
		m_registerDialog->show();
	}
}

void ReagentShowList::UpdateImReagentChanged(const im::tf::ReagentInfoTable& stuRgntInfo, im::tf::ChangeType::type changeType)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, int(changeType));
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 记录当前选中行
    QModelIndexList curSelectedRows = ui->tableView->selectionModel()->selectedRows();

    // 后台设计缺陷（卸载试剂时只是将数据库中试剂位置改为0，导致此处无法获得试剂原位置）
    // 获取项目信息(先移除对应试剂)
    ShowBlankLine(stuRgntInfo);

    // 如果是清空记录
    //if (changeType != im::tf::ChangeType::Delete)
    {
        // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
        ::im::tf::ReagentInfoTableQueryResp qryResp;
        ::im::tf::ReagTableUIQueryCond qryCond;

        // 构造查询条件
        if (stuRgntInfo.__isset.assayCode && (stuRgntInfo.assayCode > 0))
        {
            std::vector<std::string> vecDevSns;
            vecDevSns.push_back(stuRgntInfo.deviceSN);
            qryCond.__set_deviceSNs(vecDevSns);
            qryCond.__set_assayCode(stuRgntInfo.assayCode);
        }
        else
        {
            qryCond.__set_reagentId(stuRgntInfo.id);
        }

        // 执行查询
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
            UpdateBtnEnabled();
            return;
        }

        // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
        for (const auto& stuRgntIf : qryResp.lstReagentInfos)
        {
            UpdateSingleRegntInfo(stuRgntIf);
        }
    }

    // 如果之前有选中行
    SelectByModelIndex(curSelectedRows);

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);

    // 更新按钮状态
    UpdateBtnEnabled();
}

///
/// @brief
///     免疫稀释液信息更新
///
/// @param[in]  stuDltInfo   试剂信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月5日，新建函数
///
void ReagentShowList::UpdateImDltChanged(const im::tf::DiluentInfoTable& stuDltInfo, im::tf::ChangeType::type changeType)
{
    // 未显示则不更新
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    ULOG(LOG_INFO, __FUNCTION__);
    // 更新内容时锁定排序
    TblSortLockGuard tblSortLock(ui->tableView);

    // 记录当前选中行
    QModelIndexList curSelectedRows = ui->tableView->selectionModel()->selectedRows();

    // 后台设计问题
    // 需要将之前相同的试剂移除
    ShowBlankLine(stuDltInfo);

    // 如果是清空记录
    //if (changeType != im::tf::ChangeType::Delete)
    {
        // 后台设计缺陷，无法监听使用状态的改变，故由界面特殊处理
        if (stuDltInfo.__isset.deviceSN)
        {
            UpdateImDltInfo({ stuDltInfo.deviceSN });
        }
        else
        {
            std::vector<std::string> deviceSnListCh;
            std::vector<std::string> deviceSnListIm;
            GetClassfiedDeviceSn(deviceSnListCh, deviceSnListIm);
            UpdateImDltInfo(deviceSnListIm);
        }
    }

    // 如果之前有选中行
    SelectByModelIndex(curSelectedRows);

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);

    // 更新按钮状态
    UpdateBtnEnabled();
}

///
/// @brief
///     清空试剂信息
///
/// @param[in]  strDevSn		设备序列号
/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
/// @param[in]  lGrpId			维护组ID
/// @param[in]  mit				维护项目类型
/// @param[in]  enPhase			阶段
///
/// @par History:
/// @li 4170/TangChuXian，2023年02月24日，新建函数
///
void ReagentShowList::OnClearReagentInfo(QString strDevSn, int32_t devModuleIndex, int64_t lGrpId, tf::MaintainItemType::type mit, tf::MaintainResult::type enPhase)
{
    ULOG(LOG_INFO, "%s(, %d, %d)", __FUNCTION__, int(mit), int(enPhase));
    Q_UNUSED(devModuleIndex);
    Q_UNUSED(lGrpId);
    if ((mit != ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN) || (enPhase != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING))
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);
    if (strDevNamePair.first.isEmpty())
    {
        return;
    }

    // 获取设备试剂盘槽位数
    int reagentSlotNumber = gUiAdapterPtr(strDevNamePair.first, strDevNamePair.second)->GetReagentSlotNumber();

    // 将设备对应的所有行清空（保留位置信息）
    for (int iPos = 1; iPos <= reagentSlotNumber; iPos++)
    {
        std::vector<int> vRows = GetRowByPostion(iPos, strDevSn.toStdString());
        for (int iRow : vRows)
        {
            ShowBlankLine(iRow, iPos, iPos, strDevSn.toStdString());
        }
    }

    // 列宽自适应
    ResizeTblColToContentAfterInit(ui->tableView);
}

///
/// @brief
///     磁珠混匀按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月17日，新建函数
///
void ReagentShowList::OnMixBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单机模式直接询问是否执行
//     if (gUiAdapterPtr()->WhetherSingleDevMode() && (m_deivces.size() == 1))
//     {
//         // 执行单机磁珠混匀
//         ExecuteSingleDevBeadMix(m_deivces[0]->deviceSN);
//         return;
//     }

    // 弹出磁珠混匀对话框
    std::shared_ptr<ImBeadMixDlg> spImBeadMixDlg(new ImBeadMixDlg(this));
    if (spImBeadMixDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Mix Canceled.", __FUNCTION__);
        return;
    }

    // 获取选择的设备SN
    std::vector<std::string> deviceSns;
    spImBeadMixDlg->GetSelectDeviceSn(deviceSns);
    int iMixCnt = spImBeadMixDlg->GetMixTimes();

    // 获取选中设备信息
    for (const auto& strDevSn : deviceSns)
    {
        // 查询当前选中项设备状态
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
        if (spDevInfo == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "GetDeviceInfo(%s) failed", strDevSn.c_str());
            continue;
        }

        // 判断是否允许执行
        if (PopBeadMixForbiddenTip(*spDevInfo))
        {
            return;
        }
    }

    // 根据选中设备执行磁珠混匀
    for (const std::string& strSn : deviceSns)
    {
        // 执行磁珠混匀
        ::tf::ResultLong _return;
        std::vector<int> vecPos;
        vecPos.push_back(iMixCnt);
        bool bRet = im::LogicControlProxy::SingleMaintance(::tf::MaintainItemType::type::MAINTAIN_ITEM_REA_VORTEX, strSn, vecPos);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "MAINTAIN_ITEM_REA_VORTEX failed.");
        }
    }
}

///
/// @brief
///     装载装置复位按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
void ReagentShowList::OnLoadPartResetBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单机模式直接询问是否执行
    if (gUiAdapterPtr()->WhetherSingleDevMode() && !m_deivces.empty())
    {
        // 弹框提示是否确认优先使用
        std::shared_ptr<TipDlg> spTipIsPrio = std::make_shared<TipDlg>(tr("装载装置复位"), tr("确定执行装载装置复位？"), TipDlgType::TWO_BUTTON);
        if (spTipIsPrio->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel priority!");
            return;
        }

        // 执行单机装载装置复位
        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::ResetReagentLoader(_return, m_deivces[0]->deviceSN);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ResetReagentLoader(%s) failed", m_deivces[0]->deviceSN.c_str());
            return;
        }
        return;
    }

    // 弹出装载装置复位对话框
    std::shared_ptr<ImLoaderResetDlg> spImLoaderResetDlg(new ImLoaderResetDlg(this));
    if (spImLoaderResetDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), reset Canceled.", __FUNCTION__);
        return;
    }

    // 获取选择的设备SN
    std::vector<std::string> deviceSns;
    spImLoaderResetDlg->GetSelectDeviceSn(deviceSns);

    // 获取选中设备信息
    QVector<std::string> maskDevSns;
    for (const auto& strDevSn : deviceSns)
    {
        // 查询当前选中项设备状态
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
        if (spDevInfo == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "GetDeviceInfo(%s) failed", strDevSn.c_str());
            continue;
        }

        // 判断是否允许执行
        if (PopRgntLoaderResetForbiddenTip(*spDevInfo))
        {
            maskDevSns.push_back(strDevSn);
            continue;
        }
    }

    // 根据选中设备执行磁珠混匀
    for (auto strSn : deviceSns)
    {
        // 如果不能执行则跳过
        if (maskDevSns.contains(strSn))
        {
            continue;
        }

        // 执行装载装置复位
        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::ResetReagentLoader(_return, strSn);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ResetReagentLoader(%s) failed", strSn.c_str());
        }
    }
}

void ReagentShowList::SetStanterdRowStyle(QList<QStandardItem*>& rowItems, bool isReagMask, bool isCaliMask)
{
    // 试剂屏蔽、校准屏蔽则此行灰色显示
    if (isCaliMask || isReagMask)
    {
        for (auto& item : rowItems)
        {
            item->setForeground(QBrush(Qt::gray));
        }
    }
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void ReagentShowList::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);
    ReagentItemVector vecInfo;
    FillExportContainer(vecInfo, true);

    // 如果没有任何数据，弹框提示
    if (vecInfo.empty())
    {
        // 弹框提示导出完成
        TipDlg(tr("没有可以打印的数据!")).exec();
        return;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ReagentInfo Info;
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    Info.vecReagent = std::move(vecInfo);
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
void ReagentShowList::OnExportBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    ReagentInfo Info;
    FillExportContainer(Info.vecReagent, false);

    // 如果没有任何数据，弹框提示
    if (Info.vecReagent.empty())
    {
        // 弹框提示导出完成
        TipDlg(tr("没有可以导出的数据!")).exec();
        return;
    }

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        //ALOGI << "Export canceled!";
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
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportReagentInformation.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportReagentInfo(Info.vecReagent, strFilepath);
    }

    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();

}

///
/// @brief
///     需求计算按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月26日，新建函数
///
void ReagentShowList::OnRequireCalcBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 弹出需求计算对话框
    RequireCalcDlg imBeadMixDlg(this);
    if (imBeadMixDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), RequireCalcDlg Canceled.", __FUNCTION__);
        return;
    }
}

///
/// @brief
///     表格列宽改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月20日，新建函数
///
void ReagentShowList::OnTblColResized()
{
    // 获取水平滚动条
    QScrollBar* pHScrollBar = ui->tableView->horizontalScrollBar();
    if (pHScrollBar == Q_NULLPTR)
    {
        return;
    }

    // 未显示则忽略
    if (!pHScrollBar->isVisible())
    {
        return;
    }

    // 记录之前的水平滚动条位置
    //int oldVal = pHScrollBar->value();

    // 滚动条滚动
    //pHScrollBar->setValue(pHScrollBar->minimum());
//     pHScrollBar->setValue(oldVal - 1);
//     pHScrollBar->setValue(oldVal);
    //pHScrollBar->setValue(pHScrollBar->maximum());
    ui->tableView->update();
}

void ReagentShowList::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    
    // 导出按钮权限限制
    if (userPms->IsPermisson(PSM_IM_EXPORT_REAGENT))
    {
        ui->export_btn->show();
    }
    else
    {
        ui->export_btn->hide();
    }
}

void ReagentShowList::OnUpdateChUnloadReagentBtn(const QString &deviceSN)
{
	ULOG(LOG_INFO, "%s(%s)",__FUNCTION__, deviceSN.toStdString());

	// 获取当前选中行的设备
	int row = ui->tableView->currentIndex().row();
	if (row < 0 || row > m_reagentModel->rowCount() || m_reagentModel->item(row) == nullptr)
	{
		ULOG(LOG_INFO, "Invalid reagent current index, no need update.");
		return;
	}

	// 获取设备序列号
	std::string devSN, devName;
	int pos = -1;
	GetPositionInfo(row, devSN, pos);
	if (devSN.empty() || pos == -1)
	{
		ULOG(LOG_ERROR, "GetPositionInfo by row: %d failed.", row);
		return;
	}

	if (devSN != deviceSN.toStdString())
	{
		ULOG(LOG_INFO, "No need update.");
		return;
	}

	auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(devSN);
	if (spDev == nullptr)
	{
		ULOG(LOG_ERROR, "spDev is nullptr");
		return;
	}

	// 只针对生化设备
	if (spDev->deviceType != ::tf::DeviceType::DEVICE_TYPE_C1000)
	{
		return;
	}

	// 获取界面选择的试剂项目
	std::shared_ptr<BaseReagData> reagData = GetReagCache(row);
	if (reagData == nullptr)
	{
		return;
	}

	// 当前行的信息
	auto supplyInfo = boost::any_cast<ch::tf::SuppliesInfo>(reagData->m_reagentSupply);

	// 生化试剂
	if (reagData->m_myType == RT_CH)
	{
		// 如果屏蔽自动装卸载试剂需禁止点击试剂卸载按钮
		auto regGroup = boost::any_cast<ch::tf::ReagentGroup>(reagData->m_reagent);
		ui->reagentUnload->setEnabled(UnloadReagentStat(*spDev, supplyInfo, regGroup));
	}
	// 生化耗材 or 扫描失败
	else if (reagData->m_myType == RT_CHSUP)
	{
		ui->reagentUnload->setEnabled(UnloadReagentStat(*spDev, supplyInfo));
	}
}

void ReagentShowList::OnLoaderStatusUpdate(QString deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	OnUpdateChUnloadReagentBtn(deviceSN);
}

///
/// @brief 响应申请试剂结果消息，0-取消，1-接受，2-结束
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月11日，新建函数
///
void ReagentShowList::OnManualHandleReagResult(const QString& deviceSN, const int result)
{
    // 显示则刷新按钮使能
    if (this->isVisible() && (result != RGNT_CHANGE_RLT_ACCEPT))
    {
        UpdateBtnEnabled();
    }
}

///
/// @brief
///     校准过期提醒打开或关闭
///
/// @param[in]  bOn  是否打开
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
void ReagentShowList::OnCaliExpireNoticeOnOrOff(bool bOn)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, int(bOn));
    Q_UNUSED(bOn);
    m_needUpdate = true;
    DictionaryQueryManager::GetInstance()->SetCaliLineExpire(bOn);
}

void ReagentShowList::SelectByModelIndex(const QModelIndexList& mList)
{
    if (mList.size() > 0)
    {
        for (auto& rItem : mList)
        {
            QItemSelection selctionItem2(m_reagentModel->index(rItem.row(), 0), m_reagentModel->index(rItem.row(), 1));
            ui->tableView->selectionModel()->select(selctionItem2, QItemSelectionModel::Rows | QItemSelectionModel::Select);
        }
        ui->tableView->viewport()->update();
    }
}

///
/// @brief 试剂加载机构试剂槽是否存在试剂盒
///
/// @param[oyut]	bExist  true-存在，false-不存在
/// @param[in]		deviceSN  设备编号
///
/// @return true-获取成功，false-获取失败
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月4日，新建函数
///
bool ReagentShowList::ReagentLoadExistReag(bool& bExist, std::string& deviceSN)
{
    // 加载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
    ::tf::ResultBool _return_existReag;
    im::LogicControlProxy::QueryReagentLoadUnloadExistReagent(_return_existReag, deviceSN);
    if (_return_existReag.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "ReagentLoadExistReag failed, %s!", deviceSN.c_str());
        return false;
    }
    bExist = _return_existReag.value;

    return true;
}

void ReagentShowList::SetSortValue(QList<QStandardItem*>& itemList, int pos)
{
    // 设置排序值
    for (int column = 0; column < itemList.size(); column++)
    {
        QStandardItem* curItem = itemList[column];

        // 有效则跳过
        if (curItem->data(USER_SORT_ROLE).isValid())
        {
            continue;
        }

        // 是否为数字列
        QString strItemText = curItem->text();
        if (!IsNumberColumn(column))
        {
            strItemText = strItemText.toLower();
            SetItemSortData(curItem, strItemText);
            continue;
        }

        // 数字相关列特殊处理
        if (column == RCI_ReagentPos)
        {
            // 设置排序值
            QStringList strPosLst = strItemText.split("-");
            int iPos = strPosLst.back().toInt();
            QString spliterSymb = iPos < 10 ? QStringLiteral("-0") : QStringLiteral("-");
            QString strSortPos = strPosLst.join(spliterSymb);
            SetItemSortData(curItem, strSortPos);
        }
        else
        {
            SetItemSortData(curItem, GetStringSortVal(strItemText));
        }
    }

    SetTblTextAlign(m_reagentModel, Qt::AlignCenter, pos);
}

std::shared_ptr<BaseReagData> ReagentShowList::GetReagCache(int rowIdx)
{
    QStandardItem* qPos = m_reagentModel->item(rowIdx, RCI_ReagentPos);
    if (qPos == nullptr)
    {
        return nullptr;
    }
    QStandardItem* qName = m_reagentModel->item(rowIdx, RCI_Name);
    if (qName == nullptr)
    {
        return nullptr;
    }

    return m_reagCacheMgr.GetReagData(qPos->text(), qName->text());
}

QString ReagentShowList::GetUniqIdentify(int iRow)
{
    QString strUniq;
    QStandardItem* qStandNo = m_reagentModel->item(iRow, 0);
    if (qStandNo != Q_NULLPTR)
    {
        strUniq.append(qStandNo->text());
    }
    QStandardItem* qStandName = m_reagentModel->item(iRow, 1);
    if (qStandName != Q_NULLPTR)
    {
        strUniq.append(qStandName->text());
    }

    return strUniq;
}

// Class ReagDataCh start.
//////////////////////////////////////////////////////////////

ReagDataCh::ReagDataCh(const ch::tf::ReagentGroup& reagentInfo, const ch::tf::SuppliesInfo& supplyInfo)
{
    m_myType = RT_CH;
    m_rowContent = nullptr;
    m_reagent = reagentInfo;
    m_reagentSupply = supplyInfo;
}

void ReagDataCh::AssignPrintInfor(ReagentItem& info)
{
    ch::tf::ReagentGroup reagentInfo = boost::any_cast<ch::tf::ReagentGroup>(m_reagent);
    info.lRecordId = reagentInfo.id;
    info.uAssayCode = reagentInfo.assayCode;
}

void ReagDataCh::ProcShieldReagent()
{
    ch::tf::ReagentGroup reagentInfo = boost::any_cast<ch::tf::ReagentGroup>(m_reagent);
    ch::tf::SuppliesInfo reagentSupply = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);

    QString noticeMsg = !reagentInfo.reagentMask ? QObject::tr("确定屏蔽选中试剂瓶？") : QObject::tr("确定对选中试剂瓶解除屏蔽？");
    TipDlg confirmTipDlg(QObject::tr("试剂屏蔽"), noticeMsg, TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "IsReagentShieldTipDlg canceled.");
        return;
    }

    ch::tf::SuppliesInfoQueryCond qryCond;
    qryCond.__set_pos(reagentSupply.pos); // 试剂位置
    qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1); // 试剂盘号
    qryCond.__set_deviceSN(std::vector<std::string>({ reagentInfo.deviceSN })); // 设备编号

    // 必须是在用的耗材
    std::vector<::tf::PlaceStatus::type> vecPlaceStatus;
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_REGISTER);
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_LOAD);
    qryCond.__set_placeStatus(vecPlaceStatus);

    // 查询选中项目的耗材信息
    std::vector< ::ch::tf::SuppliesInfoQueryCond> siqc = { qryCond };
    if (reagentInfo.reagentMask)
    {
        // 当前试剂被屏蔽
        if (!ch::c1005::LogicControlProxy::UnMaskReagentGroups(siqc))
        {
            ULOG(LOG_ERROR, "UnMaskReagentGroups() failed, pos:%d, devSN:%s.", reagentSupply.pos, reagentInfo.deviceSN.c_str());
            return;
        }
    }
    else
    {
        // 若当前试剂没有被屏蔽
        if (!ch::c1005::LogicControlProxy::MaskReagentGroups(siqc))
        {
            ULOG(LOG_ERROR, "MaskReagentGroups() failed,pos:%d, devSN:%s.", reagentSupply.pos, reagentInfo.deviceSN.c_str());
            return;
        }
    }
}

bool ReagDataCh::IsMaskReagent()
{
    ch::tf::ReagentGroup reagentInfo = boost::any_cast<ch::tf::ReagentGroup>(m_reagent);
    return reagentInfo.reagentMask;
}

void ReagDataCh::ProcPriorityReagent()
{
    ch::tf::SuppliesInfo reagentInfo = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);

    ch::tf::SuppliesInfoQueryCond qryCond;
    qryCond.__set_pos(reagentInfo.pos); // 试剂位置
    qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1); // 试剂盘号

    // 必须是在用的耗材
    std::vector<::tf::PlaceStatus::type> vecPlaceStatus;
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_REGISTER);
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_LOAD);
    qryCond.__set_placeStatus(vecPlaceStatus);
    qryCond.__set_deviceSN(std::vector<std::string>({ reagentInfo.deviceSN })); // 设备编号

    if (!ch::c1005::LogicControlProxy::PriorUsingReagents({ qryCond }))
    {
        ULOG(LOG_ERROR, "PriorUsingReagents() failed, pos:%d, devSN:%s.", reagentInfo.pos, reagentInfo.deviceSN.c_str());
        return;
    }
}

std::shared_ptr<BaseReagData::RowContentInfo> ReagDataCh::GetRowContent(const std::vector<std::string>& devSNs)
{
    ch::tf::ReagentGroup reagentInfo = boost::any_cast<ch::tf::ReagentGroup>(m_reagent);
    ch::tf::SuppliesInfo reagentSupply = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);
    std::shared_ptr<RowContentInfo> rowContent = std::make_shared<RowContentInfo>();

    // 位置
    rowContent->m_pos = MakePositionString(reagentSupply.deviceSN, reagentSupply.pos);

	// 项目名称,优先显示项目名，不存在再显示耗材名
	auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(reagentInfo.assayCode);
	rowContent->m_assayName = (spAssayInfo == nullptr ? QString::fromStdString(reagentSupply.name) : QString::fromStdString(spAssayInfo->assayName));

    // 如果是开放试剂
	if(reagentSupply.suppliesCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE)
    {
        rowContent->m_assayName += "*";
    }

	// 不适用于机型或者试剂组信息不存在 只显示位置+名称
	if (!reagentSupply.modelUsed || reagentInfo.assayCode <= 0)
	{
		m_rowContent = rowContent;
		return rowContent;
	}

	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't find the assayCode=%d item from commAssayMgr.", reagentInfo.assayCode);
		return nullptr;
	}

    // 可用测试数 项目报警
    std::shared_ptr<RowCellInfo> pCell = std::make_shared<RowCellInfo>("?");
	// 如果设置了项目未分配那么显示“？”
	bool bUnAllocate = CommonInformationManager::GetInstance()->IsUnAllocatedAssay(reagentInfo.deviceSN, reagentInfo.assayCode);
	if (bUnAllocate)
	{
		GetTblItemColor(pCell, STATE_WARNING);
	}
	else
	{
		int totalTestTimes = GetAvailableReagentTimes(reagentInfo.assayCode, reagentInfo.deviceSN);
		pCell->m_text = totalTestTimes < 0 ? QString("0") : QString::number(totalTestTimes);
		if (totalTestTimes <= 0)
		{
			GetTblItemColor(pCell, STATE_WARNING);
		}
		else if (spAssayInfo->assayAlarmThreshold > 0 && totalTestTimes <= spAssayInfo->assayAlarmThreshold)
		{
			GetTblItemColor(pCell, STATE_NOTICE);
		}
	}
    rowContent->m_avaliableTimes = pCell;

    // 剩余测试数 瓶报警
    std::shared_ptr<RowCellInfo> pCellRem = std::make_shared<RowCellInfo>("?");
	// 如果设置了项目未分配那么显示“？”
	if (bUnAllocate)
	{
		GetTblItemColor(pCellRem, STATE_WARNING);
	}
	// 如果余量探测失败，那么显示“0”
	else if (reagentInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		pCellRem->m_text = "0";
		GetTblItemColor(pCellRem, STATE_WARNING);
	}
	else
	{
		pCellRem->m_text = reagentInfo.remainCount < 0 ? QString("0") : QString::number(reagentInfo.remainCount);
		if (reagentInfo.remainCount <= 0)
		{
			GetTblItemColor(pCellRem, STATE_WARNING);
		}
		else if (spAssayInfo->bottleAlarmThreshold > 0 && reagentInfo.remainCount <= spAssayInfo->bottleAlarmThreshold)
		{
			GetTblItemColor(pCellRem, STATE_NOTICE);
		}
	}
    rowContent->m_remainCount = pCellRem;
    
    // 试剂状态
    rowContent->m_useStatus = std::make_shared<RowCellInfo>();
    MakeReagentStatusInfo(reagentSupply, reagentInfo, *(rowContent->m_useStatus));
	// 项目未分配特殊处理，因为后台未将项目未分配的置为不可用，bug14082需要将未分配的状态置为不可用
	if (bUnAllocate)
	{
		rowContent->m_useStatus->m_text.clear();
	}

    // 校准状态
    rowContent->m_caliStatus = std::make_shared<RowCellInfo>();
	// SIND校准状态为空
	if (reagentSupply.suppliesCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		MakeCaliShowStatus(reagentInfo.caliStatus, IsCaliCurveIdValiable(reagentInfo), *(rowContent->m_caliStatus));
	}

    // 试剂批号 瓶号(需根据lotSnMode进行判断，因为会自动生成瓶号，当未输入瓶号时需要隐藏)
    QString strLot, strSn;
    if (reagentSupply.lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
    {
        rowContent->m_lot = reagentSupply.__isset.lot ? QString::fromStdString(reagentSupply.lot) : QStringLiteral("");
        rowContent->m_sn = reagentSupply.__isset.sn ? QString::fromStdString(reagentSupply.sn) : QStringLiteral("");
    }
    else if (reagentSupply.lotSnMode == tf::EnumLotSnMode::ELSM_LOT)
    {
        rowContent->m_lot = reagentSupply.__isset.lot ? QString::fromStdString(reagentSupply.lot) : QStringLiteral("");
    }
    else if (reagentSupply.lotSnMode == tf::EnumLotSnMode::ELSM_SN)
    {
        rowContent->m_sn = reagentSupply.__isset.sn ? QString::fromStdString(reagentSupply.sn) : QStringLiteral("");
    }

    // 校准曲线有效期
    rowContent->m_caliCurveExp = std::make_shared<RowCellInfo>();
	// SIND校准曲线有效期为空
	if (reagentSupply.suppliesCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		MakeCaliCurveExpirateDate(reagentInfo, *(rowContent->m_caliCurveExp));
	}

    QStringList strCalCurveExpLst = rowContent->m_caliCurveExp->m_text.split(":");
    if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0)
		&& DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
    {
		// 最新需求，校准过期显示黄色
        GetTblItemColor(rowContent->m_caliCurveExp, STATE_NOTICE);
    }

    // 试剂开瓶有效期（剩余天数）
    std::shared_ptr<RowCellInfo> pCellBep = std::make_shared<RowCellInfo>();
    int dayRest = GetOpenRestdays(reagentInfo.openBottleExpiryTime);
    pCellBep->m_text = QString::number(dayRest);
    if (dayRest <= 0)
    {
        GetTblItemColor(pCellBep, STATE_WARNING);
    }
    rowContent->m_openBottleExp = pCellBep;

    // 上机时间
    rowContent->m_registerTime = std::make_shared<RowCellInfo>(QString::fromStdString(reagentInfo.registerTime));

    // 试剂有效期
    rowContent->m_reagentExp = std::make_shared<RowCellInfo>();
    MakeReagenExpiredStatus(reagentInfo, *(rowContent->m_reagentExp));

    m_rowContent = rowContent;

    return rowContent;
}


// Class ReagDataChSup start.
//////////////////////////////////////////////////////////////


ReagDataChSup::ReagDataChSup(const ch::tf::SuppliesInfo& supplyInfo)
{
    m_myType = RT_CHSUP;
    m_rowContent = nullptr;
    m_reagentSupply = supplyInfo;
}

void ReagDataChSup::AssignPrintInfor(ReagentItem& info)
{
    ch::tf::SuppliesInfo reagentInfo = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);
    info.lRecordId = reagentInfo.id;
}

void ReagDataChSup::ProcShieldReagent()
{

}

void ReagDataChSup::ProcPriorityReagent()
{
    ch::tf::SuppliesInfo reagentInfo = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);

    ch::tf::SuppliesInfoQueryCond qryCond;
    qryCond.__set_pos(reagentInfo.pos); // 试剂位置
    qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1); // 试剂盘号

                                                                             // 必须是在用的耗材
    std::vector<::tf::PlaceStatus::type> vecPlaceStatus;
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_REGISTER);
    vecPlaceStatus.push_back(::tf::PlaceStatus::PLACE_STATUS_LOAD);
    qryCond.__set_placeStatus(vecPlaceStatus);
    qryCond.__set_deviceSN(std::vector<std::string>({ reagentInfo.deviceSN })); // 设备编号

    qryCond.__set_suppliesCode(reagentInfo.suppliesCode);
    if (!ch::c1005::LogicControlProxy::PriorUsingSupplies({ qryCond }))
    {
        ULOG(LOG_ERROR, "PriorUsingSupplies() failed, pos:%d, devSN:%s.", reagentInfo.pos, reagentInfo.deviceSN.c_str());
        return;
    }
}

std::shared_ptr<BaseReagData::RowContentInfo> ReagDataChSup::GetRowContent(const std::vector<std::string>& devSNs)
{
    std::shared_ptr<RowContentInfo> rowInfo = std::make_shared<RowContentInfo>();

    ch::tf::SuppliesInfo supplyInfo = boost::any_cast<ch::tf::SuppliesInfo>(m_reagentSupply);
    // 位置
    rowInfo->m_pos = MakePositionString(supplyInfo.deviceSN, supplyInfo.pos);

    // 耗材名称
    rowInfo->m_assayName = ThriftEnumTrans::GetSupplyName(supplyInfo.type, true);

	// 不适用于机型
	if (!supplyInfo.modelUsed)
	{
		m_rowContent = rowInfo;
		return rowInfo;
	}

    auto supplyAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(supplyInfo.type);
    // 耗材可用测试数
	{
		std::shared_ptr<RowCellInfo> cellInfo = std::make_shared<RowCellInfo>();
		// 传进来的devSNs为空，不能取该值进行查询
		double blanceTotal = GetAvailableSupplyTimes(supplyInfo.suppliesCode, supplyInfo.deviceSN);

        // 显示为ml
		cellInfo->m_text = QString::number(QString::number(blanceTotal / 10000.0, 'f', 1).toDouble()) + "ml";

        // 获取耗材的提醒值(项目报警)
        int assayAlarmThreshold = supplyAttribute ? supplyAttribute.value()->assayAlarmThreshold : 0;
        if (blanceTotal <= 0)
        {
			cellInfo->m_text = "0.0ml";
            GetTblItemColor(cellInfo, STATE_WARNING);
        }
        else if (assayAlarmThreshold > 0 && blanceTotal <= assayAlarmThreshold)
        {
            GetTblItemColor(cellInfo, STATE_NOTICE);
        }
        rowInfo->m_avaliableTimes = cellInfo;
    }

    // 剩余测试数 显示ml 瓶报警
    {
        std::shared_ptr<RowCellInfo> cellInfo = std::make_shared<RowCellInfo>("0.0ml");
		// 如果余量探测失败，那么显示“0”
		if (supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
		{
			GetTblItemColor(cellInfo, STATE_WARNING);
		}
		else
		{
			int blance = GetSupplyResidual(supplyInfo);
			int bottleAlarmThreshold = supplyAttribute ? supplyAttribute.value()->bottleAlarmThreshold : 0;
			// 显示为ml
			cellInfo->m_text = blance <= 0 ? "0.0ml" : QString::asprintf("%.1fml", blance / 10000.0);
			if (blance <= 0)
			{
				GetTblItemColor(cellInfo, STATE_WARNING);
			}
			else if (bottleAlarmThreshold > 0 && blance <= bottleAlarmThreshold)
			{
				GetTblItemColor(cellInfo, STATE_NOTICE);
			}
		}
        rowInfo->m_remainCount = cellInfo;
    }

    // 耗材状态
    rowInfo->m_useStatus = std::make_shared<RowCellInfo>();
    MakeSupplyShowStatus(*(rowInfo->m_useStatus), supplyInfo.placeStatus, supplyInfo.usageStatus, supplyInfo.id, supplyInfo.backupTotal, supplyInfo.backupNum);

    // 耗材没有校准状态
    rowInfo->m_caliStatus = std::make_shared<RowCellInfo>();

    // 耗材批号
    rowInfo->m_lot = QString::fromStdString(supplyInfo.lot);

    // 耗材瓶号
    rowInfo->m_sn = QString::fromStdString(supplyInfo.sn);

    // 校准曲线有效期
    rowInfo->m_caliCurveExp = std::make_shared<RowCellInfo>();

    // 试剂开瓶有效期（剩余天数）,开瓶有效期加上稳定时间减去当前时间，得到剩余时间
    int dayRest = GetOpenRestdays(supplyInfo.openBottleExpiryTime);
    rowInfo->m_openBottleExp = std::make_shared<RowCellInfo>();
    rowInfo->m_openBottleExp->m_text = QString::number(dayRest);
    if (dayRest <= 0)
    {
        GetTblItemColor(rowInfo->m_openBottleExp, STATE_WARNING);
    }

    // 上机时间
    rowInfo->m_registerTime = std::make_shared<RowCellInfo>(QString::fromStdString(supplyInfo.registerTime));

    // 耗材有效期
    rowInfo->m_reagentExp = std::make_shared<RowCellInfo>();
    rowInfo->m_reagentExp->m_text = QString::fromStdString(ConverStdStringToDateString(supplyInfo.expiryTime));

    if (ChSuppliesExpire(supplyInfo.expiryTime))
    {
        GetTblItemColor(rowInfo->m_reagentExp, STATE_WARNING); // 若过期则显示红色
    }

    m_rowContent = rowInfo;

    return rowInfo;
}

// Class ReagDataIm start.
//////////////////////////////////////////////////////////////

ReagDataIm::ReagDataIm(const ::im::tf::ReagentInfoTable& reagentInfo)
{
    m_myType = RT_IM;
    m_rowContent = nullptr;
    m_reagent = reagentInfo;
}

void ReagDataIm::AssignPrintInfor(ReagentItem& info)
{
    ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(m_reagent);
    info.lRecordId = reagentInfo.id;
    info.uAssayCode = reagentInfo.assayCode;
}

void ReagDataIm::ProcShieldReagent()
{
    ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(m_reagent);

    // 从缓存中获取项目信息
//     auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(reagentInfo.assayCode);
//     if (spAssayInfo == Q_NULLPTR)
//     {
//         ULOG(LOG_INFO, "GetAssayInfo failed.");
//         return;
//     }

    // 弹框提示是否确认试剂屏蔽
    if (!IsReagentShieldTipDlg(QString::fromStdString(reagentInfo.name), !reagentInfo.mask))
    {
        return;
    }

    // 执行试剂屏蔽
    im::i6000::tf::ResultReagent _return;
    bool bRet = ::im::i6000::LogicControlProxy::SetReagentMask(_return, reagentInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "SetReagentMask failed, name:%s.", reagentInfo.name.c_str());
        return;
    }
}

bool ReagDataIm::IsMaskReagent()
{
    ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(m_reagent);
    return reagentInfo.mask;
}

void ReagDataIm::ProcPriorityReagent()
{
    ::im::tf::ReagentInfoTable reagentInfo = boost::any_cast<::im::tf::ReagentInfoTable>(m_reagent);

    // 执行优先使用
    tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::SetReagentPreUse(_return, reagentInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "SetReagentPreUse failed, reagNmae:%s.", reagentInfo.name.c_str());
        return;
    }
}

std::shared_ptr<BaseReagData::RowContentInfo> ReagDataIm::GetRowContent(const std::vector<std::string>& devSNs)
{
    ::im::tf::ReagentInfoTable stuRgntInfo = boost::any_cast<::im::tf::ReagentInfoTable>(m_reagent);
    std::shared_ptr<RowContentInfo> rowInfo = std::make_shared<RowContentInfo>();

    // 位置
    rowInfo->m_pos = MakePositionString(stuRgntInfo.deviceSN, stuRgntInfo.reagentPos);

    // 试剂名称,转换项目编号为项目名
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntInfo.assayCode);
    rowInfo->m_assayName = (spAssayInfo == nullptr ? QString::fromStdString(stuRgntInfo.name) : QString::fromStdString(spAssayInfo->assayName));

    // 可用测试数
    rowInfo->m_avaliableTimes = std::make_shared<RowCellInfo>();
    rowInfo->m_avaliableTimes->m_text = QString::number(stuRgntInfo.avaTestNum);
    if (stuRgntInfo.avaTestNum <= 0)
    {
        GetTblItemColor(rowInfo->m_avaliableTimes, STATE_WARNING);
    }
    else if (RgntNoticeDataMng::GetInstance()->IsRgntAvaTestNotice(stuRgntInfo.assayCode, stuRgntInfo.avaTestNum))
    {
        GetTblItemColor(rowInfo->m_avaliableTimes, STATE_NOTICE);
    }

    // 剩余测试数
    {
        rowInfo->m_remainCount = std::make_shared<RowCellInfo>();
        rowInfo->m_remainCount->m_text = QString::number(stuRgntInfo.residualTestNum);

        // 液位探测失败，则文本显示为空，单元格红色填充
        // 配对失败显示空
        if ((stuRgntInfo.lLDError > 0) || stuRgntInfo.residualTestNum <= 0)
        {
            rowInfo->m_remainCount->m_text = "0";
            GetTblItemColor(rowInfo->m_remainCount, STATE_WARNING);
        }
        else if (RgntNoticeDataMng::GetInstance()->IsRgntResidualTestNotice(stuRgntInfo.assayCode, stuRgntInfo.residualTestNum))
        {
            GetTblItemColor(rowInfo->m_remainCount, STATE_NOTICE);
        }
    }

    // 试剂状态
    {
        rowInfo->m_useStatus = std::make_shared<RowCellInfo>();
        QString strSt = ConvertTfEnumToQString(stuRgntInfo.reagStatusUI.status);
        if (stuRgntInfo.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
        {
            strSt += QString::number(stuRgntInfo.reagStatusUI.backupCnt) + "-" + QString::number(stuRgntInfo.reagStatusUI.backupIndx);
        }
        if (stuRgntInfo.residualTestNum < 0)
        {
            // 配对失败显示空,且报警色
            strSt = "";
            GetTblItemColor(rowInfo->m_useStatus, STATE_WARNING);
        }
        rowInfo->m_useStatus->m_text = strSt;
    }

    // 校准状态(未校准有曲线为空)
    {
        rowInfo->m_caliStatus = std::make_shared<RowCellInfo>();
        QString strCaliStatus = "";
        if (stuRgntInfo.caliStatus != -1 && (stuRgntInfo.caliStatus != ::tf::CaliStatus::CALI_STATUS_NOT || stuRgntInfo.caliCurveExpInfo.empty()))
        {
            strCaliStatus = ConvertTfEnumToQString(stuRgntInfo.caliStatus);
        }
        if ((stuRgntInfo.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL) ||
            ((stuRgntInfo.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT && stuRgntInfo.caliCurveExpInfo.empty())))
        {
            GetTblItemColor(rowInfo->m_caliStatus, STATE_WARNING);
        }
        rowInfo->m_caliStatus->m_text = strCaliStatus;
    }

    // 试剂批号
    rowInfo->m_lot = stuRgntInfo.__isset.reagentLot ? QString::fromStdString(stuRgntInfo.reagentLot) : "";

    // 试剂瓶号
    rowInfo->m_sn = stuRgntInfo.__isset.reagentSN ? QString::fromStdString(stuRgntInfo.reagentSN) : "";

    // 校准曲线有效期
    {
        rowInfo->m_caliCurveExp = std::make_shared<RowCellInfo>();
        rowInfo->m_caliCurveExp->m_text = QString::fromStdString(stuRgntInfo.caliCurveExpInfo);
        QStringList strCalCurveExpLst = rowInfo->m_caliCurveExp->m_text.split(":");
        if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0) && DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
        {
            GetTblItemColor(rowInfo->m_caliCurveExp, STATE_NOTICE);
        }
    }

    // 开瓶有效期(天)
    {
        rowInfo->m_openBottleExp = std::make_shared<RowCellInfo>();
        int showStableRestday = GetOpenBottleValidDays(stuRgntInfo);
        if (showStableRestday < 0)
        {
            showStableRestday = 0;
        }
        rowInfo->m_openBottleExp->m_text = QString::number(showStableRestday);
        if (showStableRestday <= 0)
        {
            GetTblItemColor(rowInfo->m_openBottleExp, STATE_WARNING);
        }
    }

    // 上机时间
    {
        rowInfo->m_registerTime = std::make_shared<RowCellInfo>();
        rowInfo->m_registerTime->m_text = QString::fromStdString(stuRgntInfo.registerTime);
        // 如果校准过期
        if (IsCaliExpired(stuRgntInfo))
        {
            GetTblItemColor(rowInfo->m_registerTime, STATE_NOTICE);
        }
    }

    // 试剂有效期
    rowInfo->m_reagentExp = std::make_shared<RowCellInfo>();
    QString qstrReagentExpriy = QString::fromStdString(stuRgntInfo.reagentExpiry);
    QStringList strDateTimeList = qstrReagentExpriy.split(" ");
    if (strDateTimeList.size() > 0)
    {
        rowInfo->m_reagentExp->m_text = strDateTimeList.front();
    }
    QDate expDateTime = QDate::fromString(rowInfo->m_reagentExp->m_text, UI_DATE_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDate::currentDate()))
    {
        GetTblItemColor(rowInfo->m_reagentExp, STATE_WARNING);
    }

    m_rowContent = rowInfo;

    return rowInfo;
}

// Class ReagDataImDilu start.
//////////////////////////////////////////////////////////////

ReagDataImDilu::ReagDataImDilu(const ::im::tf::DiluentInfoTable& reagentInfo)
{
    m_myType = RT_IMDIT;
    m_rowContent = nullptr;
    m_reagent = reagentInfo;
}

void ReagDataImDilu::AssignPrintInfor(ReagentItem& info)
{
    ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(m_reagent);
    info.lRecordId = reagentInfo.id;
}

void ReagDataImDilu::ProcShieldReagent()
{
    // 获取稀释液信息
    ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(m_reagent);

    QString noticeMsg = !reagentInfo.mask ? QObject::tr("确定屏蔽选中的稀释液？") : QObject::tr("确定对选中的稀释液解除屏蔽？");
    TipDlg confirmTipDlg(QObject::tr("试剂屏蔽"), noticeMsg, TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "IsReagentShieldTipDlg canceled.");
        return;
    }

    // 执行稀释液屏蔽
    tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::SetDiluentMask(_return, reagentInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "SetDiluentMask failed, %s.", reagentInfo.diluentName.c_str());
        return;
    }
}

bool ReagDataImDilu::IsMaskReagent()
{
    ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(m_reagent);
    return reagentInfo.mask;
}

void ReagDataImDilu::ProcPriorityReagent()
{
    ::im::tf::DiluentInfoTable reagentInfo = boost::any_cast<::im::tf::DiluentInfoTable>(m_reagent);

    // 执行优先使用
    tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::SetDiluentPreUse(_return, reagentInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "SetDiluentPreUse failed, name:%s.", reagentInfo.diluentName.c_str());
        return;
    }
}

std::shared_ptr<BaseReagData::RowContentInfo> ReagDataImDilu::GetRowContent(const std::vector<std::string>& devSNs /* = */)
{
    ::im::tf::DiluentInfoTable stuDltInfo = boost::any_cast<::im::tf::DiluentInfoTable>(m_reagent);
    std::shared_ptr<RowContentInfo> rowInfo = std::make_shared<RowContentInfo>();

    // 位置
    rowInfo->m_pos = MakePositionString(stuDltInfo.deviceSN, stuDltInfo.pos);

    // 获取稀释液名称
    rowInfo->m_assayName = QString::fromStdString(stuDltInfo.diluentName);

    // 可用测试数
    {
        rowInfo->m_avaliableTimes = std::make_shared<RowCellInfo>();
        rowInfo->m_avaliableTimes->m_text = QString::number(QString::number(stuDltInfo.avaVol / 1000.0, 'f', 1).toDouble()) + "ml";
        if (rowInfo->m_avaliableTimes->m_text == "0ml")
        {
            // 可用测试数为0，红色填充
            GetTblItemColor(rowInfo->m_avaliableTimes, STATE_WARNING);
        }
        else if (RgntNoticeDataMng::GetInstance()->IsDltAvaVolNotice(stuDltInfo.diluentNumber, stuDltInfo.avaVol / 1000.0))
        {
            GetTblItemColor(rowInfo->m_avaliableTimes, STATE_NOTICE);
        }
    }

    // 剩余测试数(后台稀释液单位为ul，耗材为ml，不统一，难修改，故暂由界面特殊处理)
    {
        rowInfo->m_remainCount = std::make_shared<RowCellInfo>();
        QString strRm = QString::number(QString::number(stuDltInfo.curVol / 1000.0, 'f', 1).toDouble()) + "ml";
        if (bool(stuDltInfo.LLDErr))
        {
            // 液位探测失败，则文本显示为0，单元格红色填充
            strRm = "0ml";
            GetTblItemColor(rowInfo->m_remainCount, STATE_WARNING);
        }
        else if (stuDltInfo.curVol <= DBL_EPSILON)
        {
            GetTblItemColor(rowInfo->m_remainCount, STATE_WARNING);
        }
        else if (RgntNoticeDataMng::GetInstance()->IsDltCurVolNotice(stuDltInfo.diluentNumber, stuDltInfo.curVol / 1000.0))
        {
            GetTblItemColor(rowInfo->m_remainCount, STATE_NOTICE);
        }
        rowInfo->m_remainCount->m_text = strRm;
    }

    // 使用状态
    QString strUseStatus = ConvertTfEnumToQString(stuDltInfo.statusForUI.status);
    if (stuDltInfo.statusForUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
    {
        strUseStatus += QString::number(stuDltInfo.statusForUI.backupCnt) + "-" + QString::number(stuDltInfo.statusForUI.backupIndx);
    }
    rowInfo->m_useStatus = std::make_shared<RowCellInfo>();
    rowInfo->m_useStatus->m_text = strUseStatus;

    // 校准状态
    rowInfo->m_caliStatus = std::make_shared<RowCellInfo>();
    rowInfo->m_caliStatus->m_text = "-";

    // 试剂批号
    rowInfo->m_lot = QString::fromStdString(stuDltInfo.dluentLot);

    // 瓶号
    rowInfo->m_sn = QString::fromStdString(stuDltInfo.diluentSN);

    // 当前曲线有效期
    rowInfo->m_caliCurveExp = std::make_shared<RowCellInfo>();
    rowInfo->m_caliCurveExp->m_text = "-";

    // 开瓶有效期(天)
    int showStableRestday = GetOpenBottleValidDays(stuDltInfo);
    if (showStableRestday < 0)
    {
        showStableRestday = 0;
    }
    rowInfo->m_openBottleExp = std::make_shared<RowCellInfo>();
    rowInfo->m_openBottleExp->m_text = QString::number(showStableRestday);
    if (showStableRestday <= 0)
    {
        GetTblItemColor(rowInfo->m_openBottleExp, STATE_WARNING);
    }

    // 上机时间
    rowInfo->m_registerTime = std::make_shared<RowCellInfo>(QString::fromStdString(stuDltInfo.registerTime));

    // 失效日期
    rowInfo->m_reagentExp = std::make_shared<RowCellInfo>();
    QStringList strDateTimeList = QString::fromStdString(stuDltInfo.diluentExpiry).split(" ");
    rowInfo->m_reagentExp->m_text = strDateTimeList.size() >= 1 ? strDateTimeList.front() : ("");

    QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(stuDltInfo.diluentExpiry), UI_DATE_TIME_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
    {
        GetTblItemColor(rowInfo->m_reagentExp, STATE_WARNING);
    }

    m_rowContent = rowInfo;

    return rowInfo;
}


// Class BaseReagData start.
//////////////////////////////////////////////////////////////

BaseReagData::RowContentInfo::RowContentInfo()
{
    m_avaliableTimes = nullptr;
    m_remainCount = nullptr;
    m_useStatus = nullptr;
    m_caliStatus = nullptr;
    m_caliCurveExp = nullptr;
    m_openBottleExp = nullptr;
    m_reagentExp = nullptr;
}

void BaseReagData::RowContentInfo::GetStandardItemList(QList<QStandardItem*>& itemList)
{
    QStandardItem* itemPos = new QStandardItem(m_pos);
    itemPos->setData(false, USER_EMPTY_ROW_FLAG_ROLE); // 设置非空行
    itemList.append(itemPos);

    QStandardItem* itemName = new QStandardItem(m_assayName);
    itemName->setToolTip(m_assayName);
    itemList.append(itemName);

    itemList.append(MakeStandardItem(m_avaliableTimes));
    itemList.append(MakeStandardItem(m_remainCount));
    itemList.append(MakeStandardItem(m_useStatus));
    itemList.append(MakeStandardItem(m_caliStatus));
    itemList.append(new QStandardItem(m_lot));
    itemList.append(new QStandardItem(m_sn));
    itemList.append(MakeStandardItem(m_caliCurveExp));
    itemList.append(MakeStandardItem(m_openBottleExp));
    itemList.append(MakeStandardItem(m_registerTime));
    itemList.append(MakeStandardItem(m_reagentExp));
}

QStandardItem* BaseReagData::RowContentInfo::MakeStandardItem(const std::shared_ptr<RowCellInfo>& cellInfo)
{
    if (cellInfo == nullptr)
    {
        return new QStandardItem("");
    }

    QStandardItem* item = new QStandardItem(cellInfo->m_text);
    //SetItemColor(item, cellInfo->m_backgroudColor.isEmpty() ? QVariant() : cellInfo->m_backgroudColor);
    item->setData(cellInfo->m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo->m_fontColor), Qt::TextColorRole);

	// 设置了m_dataExt的导致当前曲线有效期列粗体、红色选中变黑等bug
    //if (!cellInfo->m_dataExt.isEmpty())
    //{
    //    item->setData(cellInfo->m_dataExt);
    //}
    if (cellInfo->m_text.length() > 0)
    {
        item->setToolTip(cellInfo->m_text);
    }

    return item;
}


// Class ReagentCacheMgr start.
//////////////////////////////////////////////////////////////

std::shared_ptr<BaseReagData> ReagentCacheMgr::GetReagData(const QString& strPos, const QString& strName)
{
    std::map<QString, std::shared_ptr<BaseReagData>>::iterator it = m_reagentInfoCache.find(strPos + strName);
    if (it != m_reagentInfoCache.end())
    {
        return it->second;
    }

    return nullptr;
}

void ReagentCacheMgr::EraseByPositionString(const QString& strPos)
{
    std::map<QString, std::shared_ptr<BaseReagData>>::iterator it = m_reagentInfoCache.begin();
    for (; it != m_reagentInfoCache.end(); ++it)
    {
        if (it->second->m_rowContent != nullptr && it->second->m_rowContent->m_pos == strPos)
        {
            it = m_reagentInfoCache.erase(it);
            if (it == m_reagentInfoCache.end())
            {
                break;
            }
        }
    }
}

QString ReagentCacheMgr::GetImRelativeReagent(const ::im::tf::ReagentInfoTable& targetReag)
{
    std::map<QString, std::shared_ptr<BaseReagData>>::iterator it = m_reagentInfoCache.begin();
    for (; it != m_reagentInfoCache.end(); ++it)
    {
        if (it->second->m_myType != RT_IM)
        {
            continue;
        }
        ::im::tf::ReagentInfoTable subReagent = boost::any_cast<::im::tf::ReagentInfoTable>(it->second->m_reagent);
        // 预处理试剂
        if (IsRelativeReagentIm(subReagent, targetReag))
        {
            if (it->second->m_rowContent != nullptr)
            {
                return it->second->m_rowContent->m_pos;
            }
            else
            {
                return MakePositionString(subReagent.deviceSN, subReagent.reagentPos);
            }
        }
    }

    return nullptr;
}

bool ReagentGroupCacheCh::InitCache(const std::vector<std::string>& deviceSn)
{
    ::ch::tf::ReagentGroupQueryCond qryReagentCond;
    qryReagentCond.__set_deviceSN(deviceSn);  
    qryReagentCond.__set_beDeleted(false); // 查找未卸载的

    ::ch::tf::ReagentGroupQueryResp qryReagentResp;
    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResp, qryReagentCond)
        || qryReagentResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_WARN, "Failed to query ch reagent group.");
        return false;
    }

    m_vecReagentGroups = qryReagentResp.lstReagentGroup;
    return true;
}

void ReagentGroupCacheCh::GetReagetnGroup(std::vector<ch::tf::ReagentGroup>& reagents, const std::string& devSn, int pos, int supplyCode)
{
    reagents.clear();

    for (const ch::tf::ReagentGroup& item : m_vecReagentGroups)
    {
        if (item.posInfo.pos == pos && item.suppliesCode == supplyCode && item.deviceSN == devSn)
        {
            reagents.push_back(item);
        }
    }
}

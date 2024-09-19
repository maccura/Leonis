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
/// @file     assayparamwidget.cpp
/// @brief    应用界面->项目参数
///
/// @author   4170/TangChuXian
/// @date     2020年5月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QMouseEvent>
#include "assayconfigwidget.h"
#include "ui_assayconfigwidget.h"

#include "BackUnitDlg.h"
#include "AddAssayDlg.h"
#include "AssayCodeManagerDlg.h"
#include "QAssayParamDownloadDlg.h"
#include "QAssayExportDlg.h"
#include "assayparamtabwidget.h"
#include "analysisparamwidget.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_constants.h"

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CReadOnlyDelegate.h"
#include "manager/UserInfoManager.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

AssayConfigWidget::AssayConfigWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::AssayConfigWidget),
    m_addAssayDlg(new AddAssayDlg(this)),
    m_backUnitDlg(new BackUnitDlg(this)),
    m_bInit(false),
    m_bNeedUpdateWidget(true),
    m_addAssayCode(INVALID_ASSAY_CODE)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->setupUi(this);

    m_deletedRowIdx = 0;

    // 生化的表格
    m_chSortModel = new QUtilitySortFilterProxyModel(this);
    m_chDataModel = new AssayListModel(ui->AssayTable);
    m_chDataModel->SetSortModel(m_chSortModel);
    m_chSortModel->setSourceModel(m_chDataModel);
    ui->AssayTable->setModel(m_chSortModel);
    m_chSortModel->SetTableViewSort(ui->AssayTable, { (int)AssayListModel::ASSAY_LIST_COL::NAME });
    // 还原默认顺序
    m_chSortModel->sort(-1, Qt::DescendingOrder);
    ui->AssayTable->sortByColumn(-1, Qt::DescendingOrder);
    ui->AssayTable->setColumnWidth(0, 75);

    // 设置数据未保存提示的代理
    auto dg = new SelectRowChangedDelegate(ui->AssayTable);
    dg->setView(ui->AssayTable);
    dg->setWidget(this);
    ui->AssayTable->setItemDelegate(dg);

    // 免疫的表格
    m_imSortModel = new QUtilitySortFilterProxyModel(this);
    m_imDataModel = new AssayListModel(ui->AssayTable_2);
    m_imDataModel->SetSortModel(m_imSortModel);
    m_imSortModel->setSourceModel(m_imDataModel);
    ui->AssayTable_2->setModel(m_imSortModel);
    m_imSortModel->SetTableViewSort(ui->AssayTable_2, { (int)AssayListModel::ASSAY_LIST_COL::NAME });
    // 还原默认顺序
    m_imSortModel->sort(-1, Qt::DescendingOrder);
    ui->AssayTable_2->sortByColumn(-1, Qt::DescendingOrder);
    ui->AssayTable_2->setColumnWidth(0, 75);
    ui->AssayTable_2->horizontalHeader()->setStretchLastSection(true);

    // 注册项目选中消息处理
    REGISTER_HANDLER(MSG_ID_ASSAY_SELECT, this, OnSelectAssayByName);

    // 注册项目的单位更新消息处理
    REGISTER_HANDLER(MSG_ID_ASSAY_UNIT_UPDATE, this, OnAssayUnitUpdate);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &AssayConfigWidget::OnChangePage);
    connect(m_addAssayDlg->GetSaveBtnPtr(), &QPushButton::clicked, this, &AssayConfigWidget::OnAddAssayDlgSaveClicked);

    // 初始化Tab页面
    InitTabWidget();

    // 设置标签鼠标事件穿透
    ui->show_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 表格安装事件过滤器
    ui->AssayTable->installEventFilter(this);
}

AssayConfigWidget::~AssayConfigWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月25日，新建函数
///
void AssayConfigWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化信号槽连接
    InitConnect();

    // 初始化测试项目表
    InitAssayTable();
}

///
/// @brief
///     初始化测试项目表
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月26日，新建函数
///
void AssayConfigWidget::InitAssayTable()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui->AssayTable->horizontalHeader()->setStretchLastSection(true);

    // 设置表头
    ui->AssayTable_2->verticalHeader()->setVisible(false);
    ui->AssayTable_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->AssayTable_2->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AssayTable_2->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 监听项目编码管理器信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, UpdateAssayTableData);

    // 查询数据并填充到表格中
    UpdateAssayTableData();

    // 样本源类型改变
    REGISTER_HANDLER(MSG_ID_ASSAY_SAMPLE_UPDATE, this, OnSelectRowSampleTypeChanged);
    // 参数版本改变
    REGISTER_HANDLER(MSG_ID_UTILITY_VERSION_UPDATE, this, OnSelectRowVersionChanged);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月26日，新建函数
///
void AssayConfigWidget::InitConnect()
{
    // 测试项目表选中项改变
    connect(ui->AssayTable->selectionModel(), &QItemSelectionModel::currentRowChanged,
        this, &AssayConfigWidget::OnSelectedAssayChanged);
    connect(ui->AssayTable_2->selectionModel(), &QItemSelectionModel::currentRowChanged,
        this, &AssayConfigWidget::OnSelectedAssayChanged);

    // 保存按钮被点击
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 追加按钮被点击
    connect(ui->flat_new_add, SIGNAL(clicked()), this, SLOT(OnflatAddClicked()));

    // 追加按钮被点击
    connect(ui->btn_assayCodeMag, SIGNAL(clicked()), this, SLOT(OnAssayCodeMagBtnClicked()));

    // 备选单位按钮被点击
    connect(ui->flat_alternative_unit, SIGNAL(clicked()), this, SLOT(OnBackUnitClicked()));

    // 删除按钮被点击
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));

    // 下载按钮
    connect(ui->flat_parameter_download, SIGNAL(clicked()), this, SLOT(OnParamDownload()));

    // 导出按钮
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnParamExport()));

    // 开始删除
    connect(this, &AssayConfigWidget::startDelete, this, [&] {
        // 弹出提示框
        if (TipDlg(tr("删除"), tr("是否删除选中项目"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
        {
            return;
        }

        // 启动线程开始删除
        std::thread deleteThread(std::bind(&AssayConfigWidget::deleteAssay, this));
        deleteThread.detach();
    });
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月25日，新建函数
///
void AssayConfigWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    ui->tabWidget->setCurrentIndex(0);
    OnChangePage(ui->tabWidget->currentIndex());
    m_bNeedUpdateWidget = false;
}

void AssayConfigWidget::hideEvent(QHideEvent *event)
{
    // 让基类处理事件
    QWidget::hideEvent(event);

    m_bNeedUpdateWidget = true;
}

///
/// @brief
///     查询生化需要显示的项目
///
/// @return key:assaycode+deviceType val:specialAssayInfos
///
/// @par History:
/// @li 6950/ChenFei，2022年05月13日，新建函数
///
ChSAIMaps AssayConfigWidget::QueryChShowAssays()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 查找该项目的所有样本源项目，并排序
    const auto& CIM = CommonInformationManager::GetInstance();
    // key:assaycode+deviceType val:assayInfos
    ChSAIMaps mapAssays;

    // 查询生化已启用的普通项目
    std::vector<std::shared_ptr<ch::tf::GeneralAssayInfo>> chGAIs;
    auto chAssayIndexCodeMaps = CIM->GetChAssayIndexCodeMaps();
    for (const auto& each : chAssayIndexCodeMaps)
    {
        if (each.second->enable)
        {
            chGAIs.push_back(each.second);
        }
    }

    for (const auto& chGAI : chGAIs)
    {
        // 查询通用项目表
        auto GAI = CIM->GetAssayInfo(chGAI->assayCode);
        if (GAI == nullptr)
        {
            ULOG(LOG_ERROR, "Can not find assayInfo ,assaycode :%d", chGAI->assayCode);
            continue;
        }

        // 查询生化特殊项目
        auto chSAIs = CIM->GetChemistrySpecialAssayInfo(chGAI->assayCode, chGAI->deviceType, -1, chGAI->version);
        // 保存映射
        for (const auto& each : chSAIs)
        {
            // 保存项目映射
            auto pair = std::make_pair(chGAI->assayCode, chGAI->deviceType);
            mapAssays[pair].emplace_back(each);
        }
    }

    return mapAssays;
}


///
/// @brief
///     查询ISE需要显示的项目
///
/// @return key:assaycode+deviceType val:specialAssayInfos
///
/// @par History:
/// @li 6950/ChenFei，2022年05月13日，新建函数
///
IseSAIMaps AssayConfigWidget::QueryIseShowAssays()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 查找该项目的所有样本源项目
    const auto& CIM = CommonInformationManager::GetInstance();
    // key:assaycode+modelType val:assayInfos
    IseSAIMaps mapAssays;

    // 获取已启用的项目
    auto& iseAssayIndexCodeMaps = CIM->GetIseAssayIndexCodeMaps();
    std::vector<std::shared_ptr<ise::tf::GeneralAssayInfo>> iseGAIs;
    for (const auto& each : iseAssayIndexCodeMaps)
    {
        if (each.second->enable)
        {
            iseGAIs.push_back(each.second);
        }
    }

    // 查询特殊项目
    for (const auto& iseGAI : iseGAIs)
    {
        auto GAI = CIM->GetAssayInfo(iseGAI->assayCode);
        if (nullptr == GAI)
        {
            continue;
        }

        // 保存映射关系
        auto iseSAIs = CIM->GetIseSpecialAssayInfo(iseGAI->assayCode, iseGAI->deviceType);
        for (const auto& iseSAI : iseSAIs)
        {	// 保存项目映射
            auto pair = std::make_pair(iseGAI->assayCode, iseGAI->deviceType);
            mapAssays[pair].emplace_back(iseSAI);
        }
    }

    return mapAssays;
}

///
/// @brief
///     查询免疫需要显示的项目
///
/// @return key:assaycode+deviceType val:specialAssayInfos
///
/// @par History:
/// @li 1226/zhangjing，2023年2月13日，新建函数
///
void AssayConfigWidget::QueryImShowAssays(ImGenMaps& mapImGeneralAssays)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 查找该项目的所有样本源项目
    auto& CIM = CommonInformationManager::GetInstance();

    // 获取已启用的项目
    std::set<int> uniqAssay;
    auto& ImAssayIndexCodeMaps = CIM->GetImAssayIndexCodeMaps();
    for (const auto& each : ImAssayIndexCodeMaps)
    {
        if (each.second == nullptr)
        {
            continue;
        }

        std::shared_ptr<im::tf::GeneralAssayInfo> imGAI = each.second;
        auto GAI = CIM->GetAssayInfo(imGAI->assayCode);
        if (nullptr == GAI)
        {
            continue;
        }
        // 处理多版本参数
        if (uniqAssay.find(imGAI->assayCode) != uniqAssay.end())
        {
            continue;
        }
        uniqAssay.insert(imGAI->assayCode);

        // 保存映射关系
        std::shared_ptr<im::tf::GeneralAssayInfo> usingImAssay = CIM->GetImmuneAssayInfo(imGAI->assayCode, imGAI->deviceType);
        if (usingImAssay != nullptr)
        {
            // 保存项目映射
            auto pair = std::make_pair(GAI->assayCode, usingImAssay->deviceType);
            mapImGeneralAssays[pair].emplace_back(usingImAssay);
        }
    }
}

void AssayConfigWidget::AddShowAssay(std::vector<AssayListModel::StAssayListRowItem>& items,
    int32_t assayCode, int32_t deviceType, int32_t sampleSourceType, const QString& version)
{
    ULOG(LOG_INFO, "%s(assayCode:%d, devType:%d, sourceType:%d)", __FUNCTION__, assayCode, deviceType, sampleSourceType);

    // 查询项目名称
    if (assayCode > ::ch::tf::g_ch_constants.ASSAY_CODE_SIND && assayCode <= ::ch::tf::g_ch_constants.ASSAY_CODE_I)
    {
        assayCode = ::ch::tf::g_ch_constants.ASSAY_CODE_SIND;
    }

    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed! assayCode = %d", assayCode);
        return;
    }

    auto iter = std::find_if(items.begin(), items.end(), [&](AssayListModel::StAssayListRowItem& it){ 
        return it.assayCode == assayCode; });

    if (iter == items.end())
    {
        // 填充显示数据
        AssayListModel::StAssayListRowItem item;
        item.assayCode = assayCode;
        item.model = deviceType;
        item.version = version;
        item.sampleType = sampleSourceType;
        item.assayClassify = spAssayInfo->assayClassify;
        item.name = QString::fromStdString(spAssayInfo->assayName.empty() ? spAssayInfo->assayFullName : spAssayInfo->assayName);

        items.emplace_back(std::move(item));
    }
}

///
/// @brief
///     获取选中行数据
///
/// @return     选中行数据，如果assayCode == -1，数据无效
///
/// @par History:
/// @li 6950/ChenFei，2022年05月16日，新建函数
///
AssayListModel::StAssayListRowItem AssayConfigWidget::GetSelRowItem()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    QTableView* view = nullptr;
    AssayListModel* md = nullptr;
    QUtilitySortFilterProxyModel* sortMd = nullptr;
    if (m_chemistry)
    {
        view = ui->AssayTable;
        md = m_chDataModel;
        sortMd = m_chSortModel;
    }
    else
    {
        view = ui->AssayTable_2;
        md = m_imDataModel;
        sortMd = m_imSortModel;
    }

    AssayListModel::StAssayListRowItem selItem;
    if (view->selectionModel()->hasSelection())
    {
        auto& data = md->GetData();
        selItem = data.at(sortMd->mapToSource(view->currentIndex()).row());
    }

    return selItem;
}

///
/// @brief
///     更新项目表数据
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月26日，新建函数
///
void AssayConfigWidget::UpdateAssayTableData()
{
    ULOG(LOG_INFO, "%s(), needUpdate:%d, isChemi:%d.", __FUNCTION__, m_bNeedUpdateWidget, m_chemistry);

    if (!m_bNeedUpdateWidget && m_chemistry)
    {
        return;
    }

    QTableView* tbView = ui->tabWidget->currentWidget() == ui->tab ? ui->AssayTable : ui->AssayTable_2;
    QModelIndex oriSeleced = tbView->currentIndex();
    int newAssayRow = -1;

    // 生化项目
    if (m_chemistry)
    {
        std::vector<AssayListModel::StAssayListRowItem> data;

        // 查询生化显示项目
        auto chAssays = QueryChShowAssays();
        for (const auto& assay : chAssays)
        {
            int lastShowSeq = 6;
            for (const auto& chSAI : assay.second)
            {
                AddShowAssay(data, chSAI->assayCode, (int)chSAI->deviceType, chSAI->sampleSourceType, QString::fromStdString(chSAI->version));
            }
        }

        // 存在ISE设备才显示ISE项目
        if (CommonInformationManager::GetInstance()->HasIseDevice())
        {
            // 查询ISE项目
            auto iseAssays = QueryIseShowAssays();
            for (const auto& assay : iseAssays)
            {
                for (const auto& iseSAI : assay.second)
                {
                    AddShowAssay(data, iseSAI->assayCode, (int)iseSAI->deviceType, iseSAI->sampleSourceType, QString::fromStdString(iseSAI->version));
                }
            }
        }

        // 增加空行(仅生化)
        AssayListModel::StAssayListRowItem temp;
        temp.assayCode = -1;
        data.push_back(temp);

        m_chDataModel->SetData(data);

        // 选中新增行
        if (m_addAssayCode != INVALID_ASSAY_CODE)
        {
            for (int i = 0; i < data.size(); i++)
            {
                if (data[i].assayCode == m_addAssayCode)
                {
                    newAssayRow = i;
                    break;
                }
            }
        }

        m_addAssayCode = INVALID_ASSAY_CODE;
    }
    else
    {
        // 查询免疫项目
        ImGenMaps imAssays;
        std::vector<AssayListModel::StAssayListRowItem> data;
        QueryImShowAssays(imAssays);
        for (const auto& assay : imAssays)
        {
            int imGAIIdx = 0;
            for (const auto& imGAI : assay.second)
            {
                AddShowAssay(data, imGAI->assayCode, (int)imGAI->deviceType, imGAI->sampleSource, QString::fromStdString(imGAI->version));
                imGAIIdx++;
            }
        }

        m_imDataModel->SetData(data);
    }

    // 控件显示多因素改变
    ControlDisplayFactorsChanged();

    ui->horizontalLayout->invalidate();

    if (newAssayRow >= 0 && m_chemistry)
    {
        tbView->selectRow(newAssayRow);
        m_addAssayCode = INVALID_ASSAY_CODE;
    }
    else if (oriSeleced.isValid())
    {
        int iNewRowCnt = tbView->model()->rowCount();
        if (oriSeleced.row() < iNewRowCnt)
        {
            tbView->selectRow(oriSeleced.row());
        }
        else
        {
            if (iNewRowCnt == 0)
            {
                QModelIndexList qls = tbView->selectionModel()->selectedIndexes();
                if (qls.size() > 0)
                {
                    OnSelectedAssayChanged(qls[0], qls[0]);
                }
            }
            else
            {
                tbView->selectRow(iNewRowCnt - 1);
            }
        }
    }
}

///
/// @brief
///     测试项目选中项发生改变
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月26日，新建函数
///
void AssayConfigWidget::OnSelectedAssayChanged(const QModelIndex &current, const QModelIndex &previous)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_bNeedUpdateWidget = true;
    if (!current.isValid())
    {
        ULOG(LOG_INFO, "Input params is invalid");
        return;
    }

    const auto& CIM = CommonInformationManager::GetInstance();
    AssayListModel::StAssayListRowItem item;
    if (m_chemistry)
    {
        if (!m_curAssaySampleSources.empty())
        {
            m_curAssaySampleSources.clear();

            // 增加空行(仅生化)
            AssayListModel::StAssayListRowItem temp;
            temp.assayCode = -1;

            m_chDataModel->SetRowData(m_chDataModel->rowCount() - 1, temp);
        }

        auto& data = m_chDataModel->GetData();
        auto idnex = m_chSortModel->mapToSource(current);
        if (!idnex.isValid() || idnex.row() >= data.size())
        {
            ULOG(LOG_WARN, "Invalid index, sort model index failed.");
            return;
        }
        item = data.at(idnex.row());

        // 设置当前选中的项目样本类型为最高优先级
        for (auto& spSai : CIM->GetChemistrySpecialAssayInfo(item.assayCode, (tf::DeviceType::type)item.model))
        {
            if (spSai->priority)
            {
                item.sampleType = spSai->sampleSourceType;
                item.version = QString::fromStdString(spSai->version);
            }
        }

        m_chDataModel->SetRowData(m_chSortModel->mapToSource(ui->AssayTable->currentIndex()).row(), item);
    }
    else
    {
        auto& data = m_imDataModel->GetData();
        item = data.at(m_imSortModel->mapToSource(current).row());
    }

    if (item.assayCode < 0)
    {
        ui->del_Button->setEnabled(false);
        ui->flat_save->setEnabled(false);
        ui->AssayParamTabWgt->hide(); // 生化的项目存在三种类型的项目对应三种界面，选中空行时显示哪种界面都不合适，故隐藏右侧界面，显示空白
        ui->show_label->show();
    }
    else
    {
        ui->del_Button->setEnabled(true);
        ui->flat_save->setEnabled(true);
        ui->AssayParamTabWgt->show();
        ui->show_label->hide();
    }
    ui->flat_alternative_unit->setEnabled(item.assayCode != ::ch::tf::g_ch_constants.ASSAY_CODE_SIND);
    ui->AssayParamTabWgt->LoadAssayParam(item);

    auto spAi = CIM->GetAssayInfo(item.assayCode);
    if (spAi != nullptr)
    {
        ui->flat_new_add->setEnabled(spAi->openAssay && spAi->assayCode != ::ch::tf::g_ch_constants.ASSAY_CODE_SIND);
    }
    else
    {
        // 生化开放项目编号用完后禁用新增按钮，bug2777
        int count = ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX - ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN + 1;
        for (auto& iter : CIM->GetAssayInfo())
        {
            if (iter.second->openAssay && iter.second->assayClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
            {
                count--;
            }
        }

        ui->flat_new_add->setEnabled(count);
    }

    // 开发者可新增项目封闭项目的参数版本
    if (UserInfoManager::GetInstance()->GetLoginUserInfo()->type == ::tf::UserType::USER_TYPE_TOP
        && item.assayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN
        && item.assayCode < ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN)
    {
        ui->flat_new_add->setEnabled(true);
    }
}

void AssayConfigWidget::OnSelectRowSampleTypeChanged(int sampleType)
{
    // 样本源切换是生化特有的功能
    if (m_chemistry)
    {
        DealSelectRowDataChanged(sampleType, false);
    }
}

void AssayConfigWidget::OnSelectRowVersionChanged(int version)
{
    // 生化参数版本切换
    if (m_chemistry)
    {
        DealSelectRowDataChanged(version, true);
    }
    else
    {
        // TODO 免疫参数版本切换
    }
}

void AssayConfigWidget::OnflatAddClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    auto item = GetSelRowItem();
    AddAssagData data;
    if (item.assayCode > 0)
    {
        // 编辑项目
        const auto& CIM = CommonInformationManager::GetInstance();
        auto spAssayInfo = CIM->GetAssayInfo(item.assayCode);
        if (spAssayInfo == nullptr)
        {
            ULOG(LOG_ERROR, "GetAssayInfo() Failed! assayCode = %d", item.assayCode);
            return;
        }

        auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();

        // 开发者以下的权限不能编辑封闭项目
        if (!spAssayInfo->openAssay && userInfo->type < ::tf::UserType::USER_TYPE_TOP)
        {
            // 弹出提示框
            TipDlg(tr("提示"), tr("封闭项目禁止编辑"), TipDlgType::TWO_BUTTON).exec();
            return;
        }

        // 参数版本
        data.version = item.version.toStdString();

        // 查询生化特殊项目
        auto SpecialAssayInfos = CIM->GetChemistrySpecialAssayInfo(item.assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000, -1, data.version);
        if (SpecialAssayInfos.empty())
        {
            ULOG(LOG_INFO, "GetChemistrySpecialAssayInfo() Failed! assayCode = %d", item.assayCode);
        }

        // 组装新增界面的显示信息
        for (const auto& assay : SpecialAssayInfos)
        {
            data.sampleSources.push_back(assay->sampleSourceType);
        }

        // 查找主单位
        for (const tf::AssayUnit& unit : spAssayInfo->units)
        {
            if (unit.isMain)
            {
                data.strUnit = unit.name;
                break;
            }
        }

        // 项目编号
        data.iAssayCode = item.assayCode;
        // 项目名称
        data.strAssayName = item.name.toStdString();
    }

    m_addAssayDlg->LoadDataToDlg(data);
    m_addAssayDlg->show();
}

void AssayConfigWidget::OnAssayCodeMagBtnClicked()
{
    // 创建窗口
    AssayCodeManagerDlg assayCodeManagerDlg;
    assayCodeManagerDlg.exec();
}

void AssayConfigWidget::OnBackUnitClicked()
{
    auto data = GetSelRowItem();
    if (data.assayCode == -1)
    {
        ULOG(LOG_ERROR, "%s() AssayCode:%d Error ", __FUNCTION__, data.assayCode);
        return;
    }

    m_bNeedUpdateWidget = true;

    m_backUnitDlg->LoadDataToCtrls(data.assayCode);
    m_backUnitDlg->exec();

    m_bNeedUpdateWidget = false;
}

///
/// @brief
///     保存按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2020年7月3日，新建函数
///
void AssayConfigWidget::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_bNeedUpdateWidget = false;

    const auto& CIM = CommonInformationManager::GetInstance();
    if (CIM->IsExistDeviceRuning())
    {
        TipDlg(tr("提示"), tr("项目测试中不可保存")).exec();
        // 恢复未保存的修改
        ui->AssayParamTabWgt->ReloadAssayParam();
        return;
    }

    // 保存项目参数
    QString strOptLog;
    auto data = GetSelRowItem();
    if (!ui->AssayParamTabWgt->SaveAssayParam(data, strOptLog))
    {
        ULOG(LOG_WARN, "Failed to save assayparam, assayCode:%d.", data.assayCode);
        return;
    }

    TipDlg(tr("提示"), tr("项目保存成功！")).exec();

    // 添加操作日志
    if (!strOptLog.isEmpty() && !AddOptLog(::tf::OperationType::MOD, strOptLog))
    {
        ULOG(LOG_ERROR, "Add modify assay sample source operate log failed !");
    }

    // 处理新增比色项目
    if (data.assayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN
        && data.assayCode <= ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
    {
        auto ssVec = CIM->GetChemistrySpecialAssayInfo(
            data.assayCode, (tf::DeviceType::type)data.model, -1, data.version.toStdString());

        int addAssayCount = 1;
        for (int ss : m_curAssaySampleSources)
        {
            if (std::find_if(ssVec.begin(), ssVec.end(), [&ss](const auto& data)
            {return data->sampleSourceType == ss; }) != ssVec.end())
            {
                addAssayCount++;
            }
        }

        // 新增项目样本类型的参数都编辑完，刷新列表
        if (m_curAssaySampleSources.size() == addAssayCount && !m_curAssaySampleSources.empty())
        {
            m_bNeedUpdateWidget = true;
        }
    }

    // 发送项目参数更新的消息
    if (data.assayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN
        && data.assayCode <= ::tf::AssayCodeRange::CH_RANGE_MAX)
    {
        CIM->UpdateAssayCfgInfo({ data.assayCode });

        // 修改参数版本或者名字后刷新
        QString assayName = QString::fromStdString(CIM->GetAssayNameByCode(data.assayCode));
        if (assayName != data.name)
        {
            data.name = assayName;
            m_chDataModel->SetRowData(m_chSortModel->mapToSource(ui->AssayTable->currentIndex()).row(), data);
        }
    }

    // 更新项目表
    UpdateAssayTableData();
}

void AssayConfigWidget::CheckAssayCanDelete()
{
    QItemSelectionModel * slcMd;

    const auto& CIM = CommonInformationManager::GetInstance();
    m_chemistry ? slcMd = ui->AssayTable->selectionModel() : slcMd = ui->AssayTable_2->selectionModel();
    const auto& data = m_chemistry ? m_chDataModel->GetData() : m_imDataModel->GetData();
    auto sortMd = m_chemistry ? m_chSortModel : m_imSortModel;
    auto rowIndexs = slcMd->selectedRows();

    // 显示等待弹窗
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在检查项目参数..."), true);
    for (const QModelIndex& rowIndex : rowIndexs)
    {
        if (!rowIndex.isValid() || rowIndex.row() >= data.size())
        {
            continue;
        }
        int iMapIdx = sortMd->mapToSource(rowIndex).row();
        if (iMapIdx > data.size())
        {
            continue;
        }
        const auto& rowData = data[iMapIdx];

        auto verList = CIM->GetAssayVersionList(rowData.assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000);
        auto enableGai = CIM->GetChemistryAssayInfo(rowData.assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000);
        if (verList.size() != 1 && (enableGai != nullptr && enableGai->version == rowData.version.toStdString()))
        {
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("不能删除启用的参数版本，请切换启用的参数版本后再删除"));
            return;
        }

        if (rowData.assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND
            || (rowData.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE
                && rowData.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL))
        {
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("当前选中行中存在S.IND或者电解质项目，不能删除"));
            return;
        }

        // 删除条件检查（查看是否存在试剂在机、是否在计算项目中、是否在样本中）
        if (!IsFitDeleteCondition(rowData.assayCode, rowData.name))
        {
            return;
        }
    }

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);

    // 通知主线程提示用户是否删除
    emit startDelete();
}

bool AssayConfigWidget::IsFitDeleteCondition(int assayCode, const QString& name)
{
    ULOG(LOG_INFO, "%s, assayCode:%d", __FUNCTION__, assayCode);

    // 免疫项目
    QString errStr(tr("项目[") + name + "]");
    switch (CommonInformationManager::GetAssayClassify(assayCode))
    {
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
    {
        // 查看是否存在试剂在机
        ::im::tf::ReagentInfoTableQueryResp qryResp;
        ::im::tf::ReagentInfoTableQueryCond qryCond;
        qryCond.__set_assayCode(assayCode);
        qryCond.__set_reagentPosMin(1);
        qryCond.__set_reagentPosMax(::im::i6000::tf::g_i6000_constants.REAGENT_DISK_SLOT);

        bool bRet = ::im::LogicControlProxy::QueryReagentInfo(qryResp, qryCond);
        if (!qryResp.lstReagentInfos.empty())
        {
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, errStr + tr("存在试剂在机，不能进行删除！"));
            return false;
        }
    }
    break;

    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
    {
        // 查看是否存在试剂在机
        if (CommonInformationManager::GetInstance()->IsExistOnUsedReagentByCode(assayCode))
        {
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, errStr + tr("存在试剂在机，不能进行删除！"));
            return false;
        }
    }
    break;

    default:
        break;
    }

    // 查看是否在计算项目中
    std::vector<std::string> existNames;
    if (CommonInformationManager::GetInstance()->IsExistAssayCodeInCalcAssayInfo(assayCode, existNames))
    {
        QString strCalNames = QString::fromStdString(GetContainerJoinString(existNames, "、"));
        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, errStr + tr("在计算项目%1中存在，不能进行删除！").arg(strCalNames));
        return false;
    }


    // 经会议讨论，存在两种限定方式，一种是采用i800的针对待删除项目无样本则可删除的限制，
    // 一种是采用i3000 只要样本界面存在任何样本记录均不可删除的更严格限制。
    // 讨论采用更i3000 的更严格的处理方式，所以屏蔽此段流程
#if 0
    // 查看是否在样本中存在
    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    sampQryCond.__set_containTestItems(true);
    sampQryCond.__set_assayCodes({ assayCode });
    sampQryCond.__set_qcDocId(-1);
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp))
    {
        ULOG(LOG_ERROR, "Failed to query sampleinfo.");
        return false;
    }
    std::vector<std::string> seqNum;
    //bug 0021545 add by ldx 修改为样本过多时最多显示10个样本，后面以省略号显示
    if (sampQryResp.lstSampleInfos.size() > 10)
    {
        for (int i = 0; i < 10; i++)
        {
            seqNum.push_back(sampQryResp.lstSampleInfos[i].seqNo);
        }
        //末尾以省略号结尾
        seqNum.push_back("...");
    }
    else
    {
        for (const auto& sItem : sampQryResp.lstSampleInfos)
        {
            seqNum.push_back(sItem.seqNo);
        }
    }

    if (seqNum.size() > 0)
    {
        ULOG(LOG_INFO, "Can't delete assay, due to exist in %s.", GetContainerJoinString(seqNum, "、"));
        TipDlg(errStr + tr("在数据浏览界面存在样本记录，不能进行删除！")).exec();
        return false;
    }

#else
    // i3000的只要存在任何样本记录，均不可删除项目
    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    tf::LimitCond limitCond;
    limitCond.__set_offset(0);
    limitCond.__set_count(1);
    sampQryCond.__set_limit(limitCond);
    sampQryCond.__set_sampleTypes({ tf::SampleType::SAMPLE_TYPE_PATIENT }); // 校准与质控不算样本
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
        || sampQryResp.lstSampleInfos.size() > 0)
    {
        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("请先清空数据浏览界面的所有样本，再删除项目！"));
        return false;
    }

#endif

    return true;
}

void AssayConfigWidget::OnDelBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 非待机/停机状态下不能删除项目，mantis的Bug11001 // Bug0026775(增加维护可删
    const auto &CIM = CommonInformationManager::GetInstance();
    for (auto& devPrt : CIM->GetDeviceMaps())
    {
        auto devStatus = (::tf::DeviceWorkState::type)devPrt.second->status;
        if (devPrt.second->deviceType != ::tf::DeviceType::DEVICE_TYPE_TRACK    // 删除项目时不判断轨道的状态，bug：13267
            && devStatus != ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT
            && devStatus != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY
            && devStatus != ::tf::DeviceWorkState::DEVICE_STATUS_HALT
            && devStatus != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
        {
            QString name = QString::fromStdString(devPrt.second->groupName + devPrt.second->name);
            TipDlg(tr("仪器[%1]的状态是：%2，不能删除选中项目！").arg(name).arg(ConvertTfEnumToQString(devStatus))).exec();
            return;
        }
    }

    // 启动线程检查能否删除
    std::thread checkThread(std::bind(&AssayConfigWidget::CheckAssayCanDelete, this));
    checkThread.detach();
}

///
/// @brief
///     生化按钮选中
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月9日，新建函数
///
void AssayConfigWidget::OnChemistryClicked()
{
    m_chemistry = true;
    UpdateAssayTableData();
}

///
/// @brief
///     免疫模式
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月9日，新建函数
///
void AssayConfigWidget::OnImmune()
{
    m_chemistry = false;
    UpdateAssayTableData();
}

void AssayConfigWidget::OnChangePage(int index)
{
    OnPermisionChanged();

    m_bNeedUpdateWidget = true;

    if (ui->tabWidget->tabText(index) == tr("生化"))
    {
        // 切换到生化页面
        ui->AssayParamTabWgt->OnchemistryPage();
        OnChemistryClicked();

        ui->AssayTable->sortByColumn(-1, Qt::DescendingOrder);
        m_chSortModel->sort(-1, Qt::DescendingOrder);

        if (m_chSortModel->rowCount())
        {
            ui->AssayTable->selectRow(0);
        }

        // 处理新增按钮的显示隐藏
        OnPermisionChanged();
    }
    else
    {
        // 切换到免疫页面
        ui->AssayParamTabWgt->OnimmunePage();
        OnImmune();

        // 隐藏分析参数配置页面和按钮
        ui->AssayParamTabWgt->show();
        ui->flat_new_add->hide();
        ui->btn_assayCodeMag->hide();

        ui->AssayTable_2->sortByColumn(-1, Qt::DescendingOrder);
        m_imSortModel->sort(-1, Qt::DescendingOrder);

        if (m_imSortModel->rowCount() > 0)
        {
            ui->AssayTable_2->selectRow(0);
        }
    }

    // 控件显示多因素改变
    ControlDisplayFactorsChanged();
}

void AssayConfigWidget::OnParamDownload()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 2024年8月14日，周晓峰与免疫的需求沟通确认，需求对可下载的状态不限制
    // 开发自行判断哪些状态可参数下载，运行、维护（因为试剂扫描要用到项目参数）不可下载
    for (const auto& iter : CommonInformationManager::GetInstance()->GetDeviceMaps())
    {
        if (iter.second->deviceType != ::tf::DeviceType::DEVICE_TYPE_TRACK
            && (devIsRun(*(iter.second))
                || iter.second->status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
        {
            TipDlg(tr("提示"), tr("有设备正在运行或者维护，不可进行参数下载！")).exec();
            return;
        }
    }

    // 设置表格更新标志
    m_bNeedUpdateWidget = true;

    auto currentDevModel = (m_chemistry ? QParamDownloadModel::DEV_CH : QParamDownloadModel::DEV_IM);
    auto sp = QSharedPointer<QAssayParamDownloadDlg>(new QAssayParamDownloadDlg(currentDevModel, this));
    sp->exec();

    // 设置刷新标识
    m_bNeedUpdateWidget = false;
}

///
/// @bref
///		参数导出按钮
///
/// @par History:
/// @li 8580/GongZhiQiang, 2023年8月10日，新建函数
///
void AssayConfigWidget::OnParamExport()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 只支持生化
    if (!m_chemistry)
    {
        TipDlg(tr("提示"), tr("项目参数导出暂时只支持生化！")).exec();
        return;
    }

    std::shared_ptr<QAssayExportDlg> sp(new QAssayExportDlg());
    // 加载表格里面的全部数据  
    auto& data = m_chDataModel->GetData();
    sp->LoadAssayData(data);
    sp->exec();
}

///
/// @brief
///     初始化Tab页
///
/// @par History:
/// @li 7951/LuoXin，2022年6月23日，新建函数
///
void AssayConfigWidget::InitTabWidget()
{
    ui->show_label->hide();

    // 刷新页面
    if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == tr("生化"))
    {
        ui->AssayParamTabWgt->OnchemistryPage();
        OnChemistryClicked();
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
    }
    // 获取设备分类
    const auto& CIM = CommonInformationManager::GetInstance();
    std::vector<::tf::AssayClassify::type> deviceClassifys = CIM->GetAllDeviceClassify();

    if (0 == std::count(deviceClassifys.begin(), deviceClassifys.end(), ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
    {
        // 禁用TabWidget生化页面
        ui->tabWidget->removeTab(0);
        ui->AssayTable_2->setFixedHeight(ui->AssayParamTabWgt->height());
        ui->line_v_7->setFixedHeight(ui->AssayParamTabWgt->height());
    }

    if (0 == std::count(deviceClassifys.begin(), deviceClassifys.end(), ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
    {
        // 禁用TabWidget免疫页面
        ui->tabWidget->removeTab(1);
        ui->AssayTable->setFixedHeight(ui->AssayParamTabWgt->height());
        ui->line_v_5->setFixedHeight(ui->AssayParamTabWgt->height());
    }
}

///
/// @bref	权限变化响应
///		
///
/// @par History:
/// @li 8580/GongZhiQiang, 2023年9月7日，新建函数
///
void AssayConfigWidget::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 参数下载
    ui->flat_parameter_download->setVisible(userPms->IsPermisson(PMS_PARAM_DOWNLOAD));

    // 删除项目参数
    ui->del_Button->setVisible(userPms->IsPermisson(PSM_IM_ASSAYSET_DEL_ASSAYPARAM));

    // 备选单位
    ui->flat_alternative_unit->setVisible(userPms->IsPermisson(PMS_MODIFY_BACKUP_UNIT));

    // 保存
    ui->flat_save->setVisible(userPms->IsPermisson(PSM_IM_ASSAYSET_SAVE_CHANGEGS));

    // 新增
    if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == tr("生化"))
    {
        ui->flat_new_add->setVisible(userPms->IsPermisson(PSM_ADD_ASSAY));
        ui->btn_assayCodeMag->setVisible(userPms->IsPermisson(PSM_ASSAYSET_CODE_MANAGER));
    }

    // 控件显示多因素改变
    ControlDisplayFactorsChanged();
}

void AssayConfigWidget::OnAssayUnitUpdate()
{
    ULOG(LOG_INFO, __FUNCTION__);
    m_bNeedUpdateWidget = true;
}

// 此函数仅生化使用，不考虑免疫的逻辑
void AssayConfigWidget::OnAddAssayDlgSaveClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    AddAssagData data;
    if (!m_addAssayDlg->GetModifyData(data))
    {
        return;
    }

    QSet<int> newsampleSources;
    for (auto& iter : data.sampleSources)
    {
        newsampleSources.insert(iter);
    }

    // 判断是否修改
    auto item = GetSelRowItem();
    if (item.assayCode == data.iAssayCode)
    {
        auto spSaiVec = CommonInformationManager::GetInstance()->GetChemistrySpecialAssayInfo(
            data.iAssayCode, (tf::DeviceType::type)item.model, -1, data.version);

        QSet<int> oldsampleSources;
        for (auto& spSai : spSaiVec)
        {
            oldsampleSources.insert(spSai->sampleSourceType);
        }

        // 未修改直接返回
        if (oldsampleSources == newsampleSources)
        {
            m_addAssayDlg->close();
            return;
        }
    }
    // 新增项目
    else
    {
        ui->show_label->hide();
        ui->AssayParamTabWgt->show();
    }

    item.assayCode = data.iAssayCode;
    item.version = QString::fromStdString(data.version);
    item.model = ::tf::DeviceType::DEVICE_TYPE_C1000;
    item.name = QString::fromStdString(data.strAssayName);
    item.assayClassify = ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
    m_addAssayCode = item.assayCode;
    m_curAssaySampleSources = newsampleSources;

    m_chDataModel->SetRowData(m_chSortModel->mapToSource(ui->AssayTable->currentIndex()).row(), item);

    POST_MESSAGE(MSG_ID_UTILITY_ADD_ASSAY, item.name, QString::fromStdString(data.strUnit),
        item.assayCode, item.version, newsampleSources);

    ui->flat_new_add->setEnabled(false);
    ui->flat_save->setEnabled(true);
    m_addAssayDlg->close();
}

void AssayConfigWidget::OnSelectAssayByName(QString strAssayName)
{
    ULOG(LOG_INFO, __FUNCTION__);

    QWidget* qWid = ui->tabWidget->currentWidget();
    if (qWid == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null current widget.");
        return;
    }
    QTableView* curTab = (qWid == ui->tab) ? ui->AssayTable : ui->AssayTable_2;
    QAbstractItemModel* qModel = curTab->model();
    if (qModel == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null tableview model.");
        return;
    }
    for (int i = 0; i < qModel->rowCount(); ++i)
    {
        QModelIndex curIdx = qModel->index(i, 1);
        if (curIdx.isValid() && curIdx.data().toString() == strAssayName)
        {
            curTab->selectRow(i);
            break;
        }
    }
}

void AssayConfigWidget::DealSelectRowDataChanged(int versionOrSampleType, bool isVerSionChanged)
{
    auto data = GetSelRowItem();

    // 缓存原来选中的行号
    std::vector<int> vecSelectRows;
    auto slcMd = ui->AssayTable->selectionModel();
    for (auto rowIndex : slcMd->selectedRows())
    {
        vecSelectRows.push_back(m_chSortModel->mapToSource(rowIndex).row());
    }

    if (vecSelectRows.empty())
    {
        ULOG(LOG_INFO, "Empty selected rows.");
        return;
    }

    // 修改选中行的数据
    const auto& CIM = CommonInformationManager::GetInstance();
    if (isVerSionChanged)
    {
        for (auto ptrChsai : CIM->GetChemistrySpecialAssayInfo(
            data.assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000, -1, std::to_string(versionOrSampleType)))
        {
            if (ptrChsai->priority)
            {
                data.sampleType = ptrChsai->sampleSourceType;
                break;
            }
        }

        data.version = QString::number(versionOrSampleType);
        m_chDataModel->SetRowData(vecSelectRows[vecSelectRows.size() - 1], data);
    }
    else
    {
        data.sampleType = versionOrSampleType;
        m_chDataModel->SetRowData(vecSelectRows[vecSelectRows.size() - 1], data);
    }

    // 选中原来的行
    QModelIndex topLeft = m_chSortModel->mapFromSource(m_chDataModel->index(vecSelectRows[vecSelectRows.size() - 1], (int)AssayListModel::ASSAY_LIST_COL::INDEX));
    ui->AssayTable->selectRow(topLeft.row());

    ui->AssayParamTabWgt->LoadAssayParam(data, isVerSionChanged);
}

void AssayConfigWidget::deleteAssay()
{
    // 显示等待弹窗
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在删除项目..."), true);

    QItemSelectionModel * slcMd;
    m_chemistry ? slcMd = ui->AssayTable->selectionModel() : slcMd = ui->AssayTable_2->selectionModel();
    const auto& data = m_chemistry ? m_chDataModel->GetData() : m_imDataModel->GetData();
    auto sortMd = m_chemistry ? m_chSortModel : m_imSortModel;
    auto rowIndexs = slcMd->selectedRows();

    QVector<int32_t> delCodes;
    auto CIM = CommonInformationManager::GetInstance();
    for (auto rowIndex : rowIndexs)
    {
        if (!rowIndex.isValid() || rowIndex.row() >= data.size())
        {
            continue;
        }
        int iMapIdx = sortMd->mapToSource(rowIndex).row();
        if (iMapIdx > data.size())
        {
            continue;
        }
        const auto& rowData = data[iMapIdx];

        // 免疫的删除
        if (rowData.assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            im::tf::GeneralAssayInfoQueryCond imCond;
            imCond.__set_assayCode(rowData.assayCode);
            if (!im::LogicControlProxy::DeleteGeneralAssayInfo(imCond))
            {
                POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除失败"));
                return;
            }
        }
        // 生化的删除
        else
        {
            auto spGSAIs = CIM->GetChemistrySpecialAssayInfo(rowData.assayCode, tf::DeviceType::DEVICE_TYPE_C1000);

            ::ch::tf::SpecialAssayInfoQueryCond SaiQc;
            SaiQc.__set_assayCode(rowData.assayCode);
            SaiQc.__set_version(rowData.version.toStdString());

            // 删除生化特殊项目
            if (!ch::LogicControlProxy::DeleteSpecialAssayInfo(SaiQc))
            {
                POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除失败"));
                return;
            }

            // 删除生化通用项目
            ::ch::tf::GeneralAssayInfoQueryCond chGaiQc;
            chGaiQc.__set_assayCode(rowData.assayCode);
            chGaiQc.__set_version(rowData.version.toStdString());

            if (!ch::LogicControlProxy::DeleteGeneralAssayInfo(chGaiQc))
            {
                POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除失败"));
                return;
            }
        }

        // 删除相关联的通用项目(免疫直接删除，生化参数版本只有一个时再删除)
        if (rowData.assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE
            || (CIM->GetAssayVersionList(rowData.assayCode, tf::DeviceType::DEVICE_TYPE_C1000).size() == 1))
        {
            ::tf::GeneralAssayInfoQueryCond GaiQc;
            GaiQc.__set_assayCode(rowData.assayCode);

            if (!DcsControlProxy::GetInstance()->DeleteGeneralAssayInfo(GaiQc))
            {
                POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除失败"));
                return;
            }
        }

        // 添加操作日志
        if (!rowData.name.isEmpty() && !AddOptLog(::tf::OperationType::DEL,
            tr("删除项目") + rowData.name + tr("，参数版本：") + rowData.version))
        {
            ULOG(LOG_ERROR, "Add delete assay operate log failed !");
        }

        m_deletedRowIdx = rowIndex.row();
        delCodes.push_back(rowData.assayCode);
    }

    // 更新项目参数
    m_bNeedUpdateWidget = true;
    CIM->UpdateAssayCfgInfo(delCodes);

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);
}

bool AssayConfigWidget::HasUnSaveChangedData(bool& isExistUnSaveSample)
{
    isExistUnSaveSample = !m_curAssaySampleSources.empty();
    return isExistUnSaveSample || ui->AssayParamTabWgt->HasUnSaveChangedData(GetSelRowItem());
}

///
/// @bref 控件显示影响因素改变
///		
///
/// @par History:
/// @li 8580/GongZhiQiang, 2023年11月22日，新建函数
///
void AssayConfigWidget::ControlDisplayFactorsChanged()
{
    auto curUser = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (curUser == nullptr)
    {
        ui->export_btn->hide();
        return;
    }

    // 最高权限+生化模式+非免疫机型
    if (curUser->type == tf::UserType::USER_TYPE_TOP &&
        m_chemistry && CommonInformationManager::GetInstance()->GetSoftWareType() != SOFTWARE_TYPE::IMMUNE)
    {
        // 导出
        ui->export_btn->show();
    }
    else
    {
        // 导出
        ui->export_btn->hide();
    }
}

bool AssayConfigWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        bool isExistUnSaveSample = false;
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);

        if ((ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down)
            && HasUnSaveChangedData(isExistUnSaveSample))
        {
            QString tipText(isExistUnSaveSample ? tr("当前项目存在未保存的样本类型") : tr("当前项目存在未保存的数据"));

            if (TipDlg(tipText + tr("，切换页面后将丢失这些信息，是否切换项目"),
                TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
            {
                event->ignore();
                return true;
            }
        }
    }

    return QWidget::eventFilter(object, event);
}

bool SelectRowChangedDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
    const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            QModelIndex index = m_pView->indexAt(me->pos());
            QModelIndex curIndex = m_pView->currentIndex();
            bool isExistUnSaveSample = false;

            if (index.isValid() && curIndex.isValid() && index.row() != curIndex.row()
                && m_pAssayConfigWidget->HasUnSaveChangedData(isExistUnSaveSample))
            {
                QString tipText(isExistUnSaveSample ? tr("当前项目存在未保存的样本类型") : tr("当前项目存在未保存的数据"));

                if (TipDlg(tipText + tr("，切换页面后将丢失这些信息，是否切换项目"),
                    TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
                {
                    event->ignore();
                    return true;
                }

                // 存在未保存的样本类型，但是用户选择忽略未保存的信息，设置界面需要刷新
                if (isExistUnSaveSample)
                {
                    m_pAssayConfigWidget->setNeedUpdateWidget();
                }
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

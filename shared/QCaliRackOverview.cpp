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
/// @file     QCaliRackOverview.cpp
/// @brief    校准架概况（只需要显示校准设置页面登记了位置的校准品信息，和试剂是否在机无关，和试剂是否被选择有关）
///
/// @author   8090/YeHuaNing
/// @date     2022年9月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliRackOverview.h"
#include "ui_QCaliRackOverview.h"
#include <QDateTime>
#include <QPainter>
#include <QLineEdit>
#include <QFileDialog>

#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "shared/QComDelegate.h"
#include "shared/FileExporter.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/ReagentCommon.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "manager/UserInfoManager.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "Serialize.h"
#include "printcom.hpp"

//获取项目名称
QString GetAssayName(int assayCode)
{
    const auto& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);

    if (spAssay)
        return QString::fromStdString(spAssay->assayName);

    return QString();
}

QCaliRackOverview::QCaliRackOverview(QWidget *parent)
    : BaseDlg(parent)
	, m_tableModel(new QCaliRackModel)
    , m_filterModel(new QCaliRackFilter)
{
    ui = new Ui::QCaliRackOverview();
    ui->setupUi(this);

	SetTitleName(tr("校准架概况"));

    m_filterModel->setSourceModel(m_tableModel);
	ui->tableView->setModel(m_filterModel);
    // 给日期列设置代理
    //ui->tableView->setItemDelegate(new BgColorDelegate(this));
    ui->tableView->setItemDelegate(new CReadOnlyDelegate(this));
    //ui->tableView->setItemDelegateForColumn(3, new BgColorDelegate(this));
    m_filterModel->setParent(ui->tableView);
    // 限制最多50个字符
    ui->comboBox->lineEdit()->setMaxLength(50);

    InitTableTitle();
    m_tableModel->RefreshData();
    SetAssayFilterList();
    SetTableViewSpan();


    // 根据权限显示按钮
    OnPermisionChanged();

    InitConnect();
}

QCaliRackOverview::~QCaliRackOverview()
{
}

void QCaliRackOverview::OnSearchIndexChanged(const QString & text)
{
    m_filterModel->SetFilterName(ui->comboBox->currentText()); 
    m_tableModel->UpdateModelData();
    SetTableViewSpan();
}


//void  QCaliRackOverview::RefreshData()
//{
    //m_tableModel->RefreshData();
//}

///
/// @brief 初始化表标题
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QCaliRackOverview::InitTableTitle()
{
	ui->tableView->setColumnWidth(0, 140);
	ui->tableView->setColumnWidth(1, 194);
	ui->tableView->setColumnWidth(2, 110);
	ui->tableView->setColumnWidth(3, 120);
	ui->tableView->setColumnWidth(4, 115);
	ui->tableView->setColumnWidth(5, 82);
	ui->tableView->setColumnWidth(6, 115);
	ui->tableView->setColumnWidth(7, 82);
	ui->tableView->setColumnWidth(8, 115);
	ui->tableView->setColumnWidth(9, 82);
	ui->tableView->setColumnWidth(10, 115);
	ui->tableView->setColumnWidth(11, 82);
	ui->tableView->setColumnWidth(12, 115);
	ui->tableView->setColumnWidth(13, 82);
	ui->tableView->setColumnWidth(14, 115);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

    /*for(int i = 0; i < m_tableModel->columnCount(); ++i)
        ui->tableView->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));*/
}

void QCaliRackOverview::InitConnect()
{
    // 全部按钮
    connect(ui->all_rBtn, &QRadioButton::clicked, this, [this] {m_filterModel->SetFilterApp(false); m_tableModel->UpdateModelData(); SetTableViewSpan(); });
    // 已选择按钮
    connect(ui->app_rBtn, &QRadioButton::clicked, this, [this] {m_filterModel->SetFilterApp(true); m_tableModel->UpdateModelData(); SetTableViewSpan(); });
    // 查找按钮
    connect(ui->search_btn, &QPushButton::clicked, this, [this] {m_filterModel->SetFilterName(ui->comboBox->currentText()); m_tableModel->UpdateModelData(); SetTableViewSpan(); });
    connect(ui->comboBox, &QFilterComboBox::ResetText, this, [this] {m_filterModel->SetFilterName(ui->comboBox->currentText()); m_tableModel->UpdateModelData(); SetTableViewSpan(); });
	// 打印按钮被点击
	connect(ui->print_none_flat_btn, &QPushButton::clicked, this, &QCaliRackOverview::OnPrintBtnClicked);
	// 导出按钮被点击
	connect(ui->export_none_flat_btn, &QPushButton::clicked, this, &QCaliRackOverview::OnExportBtnClicked);
    // 重置按钮
    //connect(ui->reset_btn, &QPushButton::clicked, this, [this] {ui->comboBox->setCurrentText(""); m_filterModel->SetFilterName(""); m_tableModel->UpdateModelData(); });
    // 关闭按钮
    connect(ui->cls_btn, &QPushButton::clicked, this, [this] { close(); });

    connect(m_tableModel, &QCaliRackModel::sigModelUpdated, this, [this] {
        ULOG(LOG_INFO, "signal QCaliRackModel::sigModelUpdated");
        SetAssayFilterList();
        SetTableViewSpan();
    });

    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, m_tableModel, UpdateCaliSatus);
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, m_tableModel, UpdateImCaliStatus);
    // 监听电解质校准申请更新信息
    REGISTER_HANDLER(MSG_ID_ISE_CALI_APP_UPDATE, m_tableModel, UpdateIseCaliSatus);
    REGISTER_HANDLER(MSG_ID_IM_CALI_SET_UPDATE, m_tableModel, OnImCaliSetUpdated);
    REGISTER_HANDLER(MSG_ID_CH_CALI_SET_UPDATE, m_tableModel, OnChCaliSetUpdated);
    // 项目信息变更
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, m_tableModel, OnAssayUpdated);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void QCaliRackOverview::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->export_none_flat_btn->setVisible(userPms->IsPermisson(PSM_CALI_RACK_VIEW_EXPORT));
}

void QCaliRackOverview::SetAssayFilterList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    //disconnect(ui->comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &QCaliRackOverview::OnSearchIndexChanged);
    // 取消编辑框的消息，否则会造成焦点框丢失时，会重置一次index为已有index。
    // 导致后面某一次下拉框选择后，查询不会被触发
    //disconnect(ui->comboBox->lineEdit(), 0, ui->comboBox, 0);
    const auto& names = m_tableModel->GetAssayNames();
    const QString& curText = ui->comboBox->currentText();
    ui->comboBox->clear();
    ui->comboBox->addItems(names);
    // 设置一次无效index，否则导致编辑框为空时，第一行选择项不会被触发查找
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->setCurrentText(curText);
    //connect(ui->comboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &QCaliRackOverview::OnSearchIndexChanged);
}

void QCaliRackOverview::SetTableViewSpan()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清除原有表格合并信息
    ui->tableView->clearSpans();

    auto && spanList = m_tableModel->UpdateTableSpan(m_filterModel->GetFilterApp(), m_filterModel->GetFilterName());
    int rowIndex = 0;
    for (const auto& li : spanList)
    {
        ui->tableView->setSpan(rowIndex, 0, li, 1);
        rowIndex += li;
    }
        
    SetPrintAndExportEnabled(m_tableModel->HasRackDatas());
}

QCaliRackModel::QCaliRackModel(QObject* parent /*= nullptr*/) : QAbstractTableModel(parent), m_rowCnt(0), m_bExImDev(false), m_bExChDev(false), m_bExIseDev(false){
    // 初始化标题
    m_hTitle << tr("项目名称") << tr("校准品名称") << tr("校准品批号") << tr("失效日期")
        << tr("校准品1") << tr("位置1") << tr("校准品2") << tr("位置2") << tr("校准品3") << tr("位置3")
        << tr("校准品4") << tr("位置4") << tr("校准品5") << tr("位置5") << tr("校准品6") << tr("位置6");

    // 是否存在免疫设备
    if (m_bExImDev = !CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 }).empty())
    {
        UpdateImSelectedAssay(); 
    }
    // 是否存在生化设备
    if (m_bExChDev = !CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_C1000 }).empty())
    {
        UpdateChSelectedAssay(); 
    }
    // 是否存在电解质设备
    if (m_bExIseDev = !CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_ISE1005 }).empty())
    {
        UpdateIseSelectedAssay();
    }
}


int QCaliRackModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_rowCnt;
}

int QCaliRackModel::columnCount(const QModelIndex & parent) const
{
    return m_hTitle.size();
}

QVariant QCaliRackModel::data(const QModelIndex & index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (!index.isValid() || row < 0 || col < 0)
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
    {
        // 取出信息
        const auto& info = GetCaliRackDetailByRow(row);
        if (info)
        {
            // 显示信息
            switch (col)
            {
                case 0:
                    return GetAssayName(info->m_assayCode);
                    break;
                case 1:
                    return info->m_grpName;
                    break;
                case 2:
                    return info->m_lot;
                    break;
                case 3:
                    return ToCfgFmtDateTime(info->m_exprite);
                    break;
                case 4:
                    return info->m_concL1 >= 0.0 ? QString::number(info->m_concL1) : QVariant();
                    break;
                case 5:
                    return info->m_rackL1;
                    break;
                case 6:
                    return info->m_concL2 >= 0.0 ? QString::number(info->m_concL2) : QVariant();
                    break;
                case 7:
                    return info->m_rackL2;
                    break;
                case 8:
                    return info->m_concL3 >= 0.0 ? QString::number(info->m_concL3) : QVariant();
                    break;
                case 9:
                    return info->m_rackL3;
                    break;
                case 10:
                    return info->m_concL4 >= 0.0 ? QString::number(info->m_concL4) : QVariant();
                    break;
                case 11:
                    return info->m_rackL4;
                    break;
                case 12:
                    return info->m_concL5 >= 0.0 ? QString::number(info->m_concL5) : QVariant();
                    break;
                case 13:
                    return info->m_rackL5;
                    break;
                case 14:
                    return info->m_concL6 >= 0.0 ? QString::number(info->m_concL6) : QVariant();
                    break;
                case 15:
                    return info->m_rackL6;
                    break;
                default:
                    break;
            }
        }
    }
    // 失效日期单元格颜色判断
    else if (role == Qt::UserRole + DELEGATE_COLOR_OFFSET_POS&& col == 3)
    {
        // 取出信息
        const auto& info = GetCaliRackDetailByRow(row);
        //if (info && QDate::fromString(info->m_exprite, Qt::ISODate) <= QDate::currentDate())
        //{
        //    QColor color;
        //    color.setNamedColor(UI_REAGENT_WARNCOLOR);
        //    return color; // QBrush(color);
        //}
    }
    // 失效日期单元格颜色判断（字体）
    //else if (role == Qt::ForegroundRole && col > 0)
    else if (role == Qt::UserRole + 1 && col > 0)
    {
        // 取出信息
        const auto& info = GetCaliRackDetailByRow(row);
        
        if (info && info->m_bShied)
        {
            return QColor(Qt::gray);
        }
        else if (col == 3 && info && QDate::fromString(info->m_exprite, Qt::ISODate) <= QDate::currentDate())
        {
            QColor color;
            color.setNamedColor(UI_REAGENT_WARNFONT);
            return color; // QBrush(color);
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter; // QVariant::fromValue(Qt::AlignCenter);
    }

    return QVariant();
}

QVariant QCaliRackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((role == Qt::DisplayRole) && (section >= 0 && section < m_hTitle.size()))
    {
        return m_hTitle.at(section);
    }

    return QVariant();
}

void QCaliRackModel::UpdateCaliSatus(class std::vector<class ch::tf::SupplyUpdate, class std::allocator<class ch::tf::SupplyUpdate>> supplyUpdates)
{
    ULOG(LOG_INFO, "%s().", __FUNCTION__);

    // 需要确认此处的试剂是否还存在，是否有一个试剂是选择的
    for (const auto& chSupply : supplyUpdates)
    {
        ULOG(LOG_INFO, "chSupply= %s", ToString(chSupply));
        ch::tf::SuppliesInfoQueryResp qrySuppliesResp;
        ch::tf::SuppliesInfoQueryCond qrySuppliesCond;
        qrySuppliesCond.__set_pos(chSupply.posInfo.pos);
        // 必须是试剂类型才有校准状态的概念
        qrySuppliesCond.__set_type(ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT);
        qrySuppliesCond.__set_deviceSN({ chSupply.deviceSN });

        // 清除原位置的信息
        const QString& posIndex = QString("%1|%2").arg(QString::fromStdString(chSupply.deviceSN)).arg(chSupply.posInfo.pos);
        for (const auto& assay : m_posIndex[posIndex])
        {
            m_appAssays[assay].remove(posIndex);
        }
        m_posIndex[posIndex].clear();

        // 查找对应设备的耗材信息
        if (!ch::c1005::LogicControlProxy::QuerySuppliesInfo(qrySuppliesResp, qrySuppliesCond) || qrySuppliesResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QuerySuppliesInfo() failed");
            continue;
        }

        for (const auto& supply : qrySuppliesResp.lstSuppliesInfo)
        {
            std::vector<ch::tf::ReagentGroup> reagentGroups;
            
            if (!GetReagentGroupsBySupply(supply, reagentGroups) || reagentGroups.empty())
            {    
                ULOG(LOG_ERROR, "Query ReagentGroup() failed");
                continue;
            }

            for (const auto& r : reagentGroups)
            {
                ULOG(LOG_INFO, "reagent id:%lld, selected:%d", r.id, r.caliSelect);
                if (r.caliSelect)
                {
                    m_appAssays[r.assayCode].insert(posIndex);
                    m_posIndex[posIndex].insert(r.assayCode);
                }
            }
        }
    }

    UpdateModelData();
    emit sigModelUpdated();
}

///
/// @brief  
///
/// @param[in]  ise::tf::IseModuleInfo  
/// @param[in]  iseAppUpdates  
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年12月15日，新建函数
///
void QCaliRackModel::UpdateIseCaliSatus(std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 需要确认此处的试剂是否还存在，是否有一个试剂是选择的
    for (const auto& iseModule : iseAppUpdates)
    {
        const QString& posIndex = QString("%1|%2|%3").arg(QString::fromStdString(iseModule.deviceSn)).arg(iseModule.moduleIndex).arg(iseModule.sampleType);
        ULOG(LOG_INFO, "posIndex: ", posIndex.toStdString());

        if (iseModule.caliSelect)
        {
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_NA].insert(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_K].insert(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_CL].insert(posIndex);

            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_NA);
            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_K);
            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_CL);
        }
        else
        {
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_NA].remove(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_K].remove(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_CL].remove(posIndex);

            m_posIndex[posIndex].remove(ise::tf::g_ise_constants.ASSAY_CODE_NA);
            m_posIndex[posIndex].remove(ise::tf::g_ise_constants.ASSAY_CODE_K);
            m_posIndex[posIndex].remove(ise::tf::g_ise_constants.ASSAY_CODE_CL);
        }
    }

    UpdateModelData();
    emit sigModelUpdated();
}

void QCaliRackModel::UpdateImCaliStatus(const im::tf::ReagentInfoTable & stuRgntInfo, const im::tf::ChangeType::type type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 旧逻辑：如果批号序列号为空，则表明扫描失败
    // 新逻辑：如果项目编号小于等于0，则表明扫描失败（因bug17830变更）
    if (stuRgntInfo.assayCode <= 0)
    {
        return;
    }

    // 如果没有这个设备了，页面就没有显示，则不需要统计
    if (CommonInformationManager::GetInstance()->GetDeviceInfo(stuRgntInfo.deviceSN) == nullptr)
        return;

    // 卸载需要重新获取,否则m_appAssays中数据有误-bug27351
    if (stuRgntInfo.reagentPos <= 0)
    {
        m_appAssays[stuRgntInfo.assayCode].clear();
        UpdateImSelectedAssay();
        return;
    }
	
    const QString& posIndex = QString("%1|%2").arg(QString::fromStdString(stuRgntInfo.deviceSN)).arg(stuRgntInfo.reagentPos);
    ULOG(LOG_INFO, "posIndex: ", posIndex.toStdString());

    if (stuRgntInfo.selCali)
    {
        m_appAssays[stuRgntInfo.assayCode].insert(posIndex);
        m_posIndex[posIndex].insert(stuRgntInfo.assayCode);
    }
    else
    {
        m_appAssays[stuRgntInfo.assayCode].remove(posIndex);
        m_posIndex[posIndex].remove(stuRgntInfo.assayCode);
    }

    UpdateModelData();
    emit sigModelUpdated();
}

void QCaliRackModel::RefreshData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空原有信息
    m_rackDetails.clear();

    // 获取所有的生化校准品信息
    UpdateChCaliRackInfos();

    // 获取所有的免疫校准品
    UpdateImCaliRackInfos();
}

boost::optional<std::tuple<QString, bool>> QCaliRackModel::GetNameAndIsApp(int sourceRow)
{
    int row = sourceRow;
    int tmpRowCnt = 0;

    if (row < 0)
        return boost::none;

    auto i = m_rackDetails.constBegin();
    while (i != m_rackDetails.constEnd()) {
        tmpRowCnt += i.value().size();
        if (tmpRowCnt > row)
        {
            int ind = row - (tmpRowCnt - i.value().size());
            if (ind < 0)
            {
                return boost::none;
            }
            auto& info = i.value()[ind];
            return boost::make_optional<std::tuple<QString, bool>>(std::tuple<QString, bool>(GetAssayName(info.m_assayCode), !m_appAssays[info.m_assayCode].isEmpty()));
        }
        ++i;
    }

    return boost::none;
}

QList<QPoint> QCaliRackModel::GetTableSpanInfos()
{
    QList<QPoint> spanList;

    int startRow = 0;
    for (const auto& rack : m_rackDetails)
    {
        int capSize = rack.size();
        if (capSize > 1)
        {
            spanList.append(QPoint(startRow, capSize));
        }
        startRow += capSize;
    }

    return std::move(spanList);
}

void QCaliRackModel::UpdateModelData()
{
    beginResetModel();
    endResetModel();
    emit dataChanged(index(0, 0), index(rowCount()-1, columnCount()-1));
}

const QList<QString> QCaliRackModel::GetAssayNames()
{
    QList<QString> assayNames;
    QMapIterator<int, QList<CaliRackDetails>> i(m_rackDetails);

    // 使用此方式可以按照通道号排序显示项目名称
    while (i.hasNext()) {
        i.next();
        const QString& name = GetAssayName(i.key());
        if (!name.isEmpty())
            assayNames.append(name);
    }

    return assayNames;
}

QVector<int> QCaliRackModel::UpdateTableSpan(bool isApp, const QString& filterName)
{
    QVector<int> spanVec;
    int row = 0;
    auto i = m_rackDetails.constBegin();
    while (i != m_rackDetails.constEnd()) {
        auto& data = i.value()[0];
        // 如果不满足筛选名称，跳过此记录
        if (!filterName.isEmpty() && !(GetAssayName(data.m_assayCode).contains(filterName, Qt::CaseInsensitive)))
        {
            ++i; 
            continue;
        }
        // 如果和选择条件不符，跳过此记录
        if (isApp && !m_appAssays[data.m_assayCode].isEmpty() != isApp)
        {
            ++i; 
            continue;
        }

        int count = i.value().size();

        spanVec.append(count);
        row += count;

        ++i;
    }

    return spanVec;
}

void QCaliRackModel::OnImCaliSetUpdated(tf::UpdateType::type upType, im::tf::CaliDocGroup group)
{
    ULOG(LOG_INFO, "%s(), upType:%d, groupId:%lld", __FUNCTION__, upType, group.id);

    switch (upType)
    {
        case tf::UpdateType::UPDATE_TYPE_DELETE:
        {
            if (!group.__isset.id || group.id <= 0 || m_mapImGroupAssays.find(group.id) == m_mapImGroupAssays.end())
                return;
            
            DeleteRackDetails(m_mapImGroupAssays, group.id);
        }
        break;
        case tf::UpdateType::UPDATE_TYPE_ADD:
        {
            // 如果水平信息为空，则下一个
            if (group.caliDocs.empty())
            {
                return;
            }

            // 循环取出每一个校准品组详细信息
            QMap<int, CaliRackDetails> detailFromCaliGroup;

            // 如果没有设置位置，就不用展示
            if (!UpdateImRackDetails(group, detailFromCaliGroup))
                return;

            // 将信息汇总
            AddRackDetails(m_mapImGroupAssays, detailFromCaliGroup, group.id);
        }
        break;
        case tf::UpdateType::UPDATE_TYPE_MODIFY:
        {
            // 如果水平信息为空，则下一个
            if (group.caliDocs.empty())
            {
                return;
            }

            // 循环取出每一个校准品组详细信息
            QMap<int, CaliRackDetails> detailFromCaliGroup;

            do
            {
                // 如果没有设置位置，就不用展示
                if (!UpdateImRackDetails(group, detailFromCaliGroup))
                {
                    // 删除原有数据
                    if (!group.__isset.id || group.id <= 0 || m_mapChGroupAssays.find(group.id) == m_mapChGroupAssays.end())
                        return;

                    DeleteRackDetails(m_mapImGroupAssays, group.id);
                    break;
                };

                // 刷新缓存
                ModifyRackDetails(m_mapImGroupAssays, detailFromCaliGroup, group.id);
            } while (false);
        }
        break;
    }

    UpdateModelData();
}

void QCaliRackModel::OnChCaliSetUpdated(tf::UpdateType::type upType, ch::tf::CaliDocGroup group, QList<ch::tf::CaliDoc> docs)
{
    ULOG(LOG_INFO, "%s(), upType:%d, groupId:%lld", __FUNCTION__, upType, group.id);

    switch (upType)
    {
        case tf::UpdateType::UPDATE_TYPE_DELETE:
        {
            if (!group.__isset.id || group.id <= 0 || m_mapChGroupAssays.find(group.id) == m_mapChGroupAssays.end())
                return;

            DeleteRackDetails(m_mapChGroupAssays, group.id);
        }
        break;
        case tf::UpdateType::UPDATE_TYPE_ADD:
        {
            bool isPosSetted = false;
            // 循环取出每一个校准品组详细信息
            QMap<int, CaliRackDetails> detailFromCaliGroup;

            // 当前校准品组内每个文档的浓度信息
            for (auto& doc : docs)
            {
                if (!doc.rack.empty())
                    isPosSetted = true;
                UpdateChRackDetails(doc, group, detailFromCaliGroup);
            }

            // 没有设置架号位置的，不用显示出来
            if (!isPosSetted)
                return;

            // 将信息汇总
            AddRackDetails(m_mapChGroupAssays, detailFromCaliGroup, group.id);
        }
        break;
        case tf::UpdateType::UPDATE_TYPE_MODIFY:
        {
            bool isPosSetted = false;
            // 循环取出每一个校准品组详细信息
            QMap<int, CaliRackDetails> detailFromCaliGroup;

            // 当前校准品组内每个文档的浓度信息
            for (auto& doc : docs)
            {
                if (!doc.rack.empty())
                    isPosSetted = true;
                UpdateChRackDetails(doc, group, detailFromCaliGroup);
            }

            do 
            {
                // 没有设置架号位置的，不用显示出来
                if (!isPosSetted)
                {
                    // 删除原有数据
                    if (!group.__isset.id || group.id <= 0 || m_mapChGroupAssays.find(group.id) == m_mapChGroupAssays.end())
                        return;

                    DeleteRackDetails(m_mapChGroupAssays, group.id);
                    break;
                }

                // 刷新缓存
                ModifyRackDetails(m_mapChGroupAssays, detailFromCaliGroup, group.id);
            } while (false);
        }
        break;
    }

    UpdateModelData();
    emit sigModelUpdated();
}

void QCaliRackModel::OnAssayUpdated()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    UpdateModelData();
    emit sigModelUpdated();
}

bool QCaliRackModel::UpdateChSelectedAssay()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取在机或者注册的项目
    ::ch::tf::ReagentGroupQueryResp qryResp;
    ::ch::tf::ReagentGroupQueryCond qryCond;
    qryCond.__set_beDeleted(false);

    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryResp, qryCond)
        || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentGroup failed");
    }

    for (auto& r : qryResp.lstReagentGroup)
    {
        // 排除异常项目
        if (r.assayCode < 0 || r.posInfo.pos <= 0)
            continue;

        // 如果没有这个设备了，页面就没有显示，则不需要统计
        if (CommonInformationManager::GetInstance()->GetDeviceInfo(r.deviceSN) == nullptr)
            continue;
        
        if (r.caliSelect)
        {
            const QString& posIndex = QString("%1|%2").arg(QString::fromStdString(r.deviceSN)).arg(r.posInfo.pos);
            m_appAssays[r.assayCode].insert(posIndex);
            m_posIndex[posIndex].insert(r.assayCode);
        }
    }

    return true;
}

bool QCaliRackModel::UpdateImSelectedAssay()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 首先刷新设备信息
    auto deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(
        std::vector<tf::DeviceType::type>({tf::DeviceType::DEVICE_TYPE_I6000 }));

    // 没有免疫设备，直接返回
    if (deviceList.empty())
        return false;
    std::vector<std::string> imDevices;
    for (const auto& d : deviceList)
    {
        if (d)
            imDevices.push_back(d->deviceSN);
    }

    // 构造查询条件和查询结果
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 设置设备列表(不设置的话，会导致免疫接口中参数检查不到设备，查询不到数据)
    qryCond.__set_deviceSNs(imDevices);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || qryResp.lstReagentInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
    }

    QSet<int> appAssays;

    for (auto& r : qryResp.lstReagentInfos)
    {
        // 排除异常项目
        if (r.assayCode < 0 || r.reagentPos <= 0)
            continue;

        // 如果没有这个设备了，页面就没有显示，则不需要统计
        if (CommonInformationManager::GetInstance()->GetDeviceInfo(r.deviceSN) == nullptr)
            continue;

        if (r.selCali)
        {
            const QString& posIndex = QString("%1|%2").arg(QString::fromStdString(r.deviceSN)).arg(r.reagentPos);
            m_appAssays[r.assayCode].insert(posIndex);
            m_posIndex[posIndex].insert(r.assayCode);
        }
    }

    return true;
}

bool QCaliRackModel::UpdateIseSelectedAssay()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取在机或者注册的项目
    ::ise::tf::IseModuleInfoQueryResp qryResp;
    ::ise::tf::IseModuleInfoQueryCond qryCond;

    ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
    if (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS )
    {
        ULOG(LOG_ERROR, "QueryReagentGroup failed");
    }

    QSet<int> appAssays;
    QSet<int> unAppAssays;

    for (auto& r : qryResp.lstIseModuleInfos)
    {
        // 如果没有这个设备了，页面就没有显示，则不需要统计
        if (CommonInformationManager::GetInstance()->GetDeviceInfo(r.deviceSn) == nullptr)
            continue;

        if (r.caliSelect)
        {
            const QString& posIndex = QString("%1|%2|%3").arg(QString::fromStdString(r.deviceSn)).arg(r.moduleIndex).arg(r.sampleType);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_NA].insert(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_K].insert(posIndex);
            m_appAssays[ise::tf::g_ise_constants.ASSAY_CODE_CL].insert(posIndex);

            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_NA);
            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_K);
            m_posIndex[posIndex].insert(ise::tf::g_ise_constants.ASSAY_CODE_CL);
        }
    }
    
    return true;
}

void QCaliRackModel::UpdateChCaliRackInfos()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_mapChGroupAssays.clear();
    beginResetModel();

    ::ch::tf::CaliDocGroupQueryResp ResultDocGroup;
    ::ch::tf::CaliDocGroupQueryCond queryDocGroupCondition;

    if (!ch::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
        || ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ResultDocGroup.lstCaliDocGroup.empty())
    {
        ULOG(LOG_INFO, "QueryCaliDocGroup failed! or lstCaliDocGroup is empty.");
        endResetModel();
        emit sigModelUpdated();
        return;
    }

    for (auto& grp : ResultDocGroup.lstCaliDocGroup)
    {
        // 此校准品组是否设置了位置（有一个都算设置了位置的）
        bool isPosSetted = false;
        // 循环取出每一个校准品组详细信息
        QMap<int, CaliRackDetails> detailFromCaliGroup;
        // 取出每一个文档信息
        for (auto& docKeyInfo : grp.caliDocKeyInfos)
        {
            ::ch::tf::CaliDocQueryResp ResultDocs;
            ::ch::tf::CaliDocQueryCond QueryCondition;
            QueryCondition.__set_level(docKeyInfo.level);
            QueryCondition.__set_lot(docKeyInfo.lot);
            QueryCondition.__set_sn(docKeyInfo.sn);
        
            // 查询文档详细信息
            if (!ch::LogicControlProxy::QueryCaliDoc(ResultDocs, QueryCondition)
                || ResultDocs.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || ResultDocs.lstCaliDocs.empty())
            {
                ULOG(LOG_ERROR, "fail to query QueryCaliDoc, level is %d, lot is %s, sn is %s", docKeyInfo.level, docKeyInfo.lot, docKeyInfo.sn);
                m_rackDetails.clear();
                endResetModel();
                emit sigModelUpdated();
                return;
            }
        
            // 去除当前校准品组内每个文档的浓度信息
            for (auto& doc : ResultDocs.lstCaliDocs)
            {
                if (!doc.rack.empty())
                    isPosSetted = true;
                UpdateChRackDetails(doc, grp, detailFromCaliGroup);
            }
        }

        // 没有设置架号位置的，不用显示出来
        if (!isPosSetted)
            continue;

        // 将信息汇总
        AddRackDetails(m_mapChGroupAssays, detailFromCaliGroup, grp.id);
    }

    endResetModel();

    emit sigModelUpdated();
}

void QCaliRackModel::UpdateImCaliRackInfos()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_mapImGroupAssays.clear();
    beginResetModel();

    // 查询校准组合信息
    ::im::tf::CaliDocGroupQueryResp ResultDocGroup;
    ::im::tf::CaliDocGroupQueryCond queryDocGroupCondition;

    if (!im::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
        || ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ResultDocGroup.lstCaliDocGroups.empty())
    {
        ULOG(LOG_INFO, "QueryCaliDocGroup failed! or lstCaliDocGroup is empty.");
        endResetModel();
        emit sigModelUpdated();
        return;
    }

    for (auto docGroup : ResultDocGroup.lstCaliDocGroups)
    {
        // 如果水平信息为空，则下一个
        if (docGroup.caliDocs.empty())
        {
            continue;
        }

        // 循环取出每一个校准品组详细信息
        QMap<int, CaliRackDetails> detailFromCaliGroup;

        // 如果没有设置位置，就不用展示
        if (!UpdateImRackDetails(docGroup, detailFromCaliGroup))
            continue;

        // 将信息汇总
        AddRackDetails(m_mapImGroupAssays, detailFromCaliGroup, docGroup.id);
    }

    endResetModel();
    emit sigModelUpdated();
}

void QCaliRackModel::UpdateChRackDetails(const ch::tf::CaliDoc & doc, const ch::tf::CaliDocGroup & grp, QMap<int, CaliRackDetails>& details)
{
    ULOG(LOG_INFO, "%s(), CaliGrpId is %lld", __FUNCTION__, grp.id);

    for (auto& c : doc.compositions)
    {
        auto& detail = details[c.assayCode];
        GetChCaliRackDetail(doc, c, grp, details[c.assayCode]);
    }
}

bool QCaliRackModel::GetChCaliRackDetail(const ch::tf::CaliDoc & doc, const ch::tf::CaliComposition & c, const ch::tf::CaliDocGroup & grp, CaliRackDetails& detail)
{
    // 填充基础信息
    detail.m_assayCode = c.assayCode;
    detail.m_groupId = grp.id;
    //detail.m_bApp = !(m_appAssays[c.assayCode].isEmpty());
    detail.m_bShied = (grp.disableAssayCodes.find(c.assayCode) != grp.disableAssayCodes.end());
    detail.m_grpName = QString::fromStdString(grp.name);
    detail.m_lot = QString::fromStdString(grp.lot);
    detail.m_exprite = QDateTime::fromString(QString::fromStdString(grp.expiryTime), Qt::ISODate).date().toString(Qt::ISODate);

    switch (doc.level)
    {
        case 1:
        {
            QString rack;
            if (grp.systemWater)
                rack = tr("系统水");
            else
                rack = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            detail.m_concL1 = c.conc;
            detail.m_rackL1 = rack;
        }
        break;
        case 2:
            detail.m_concL2 = c.conc;
            detail.m_rackL2 = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            break;
        case 3:
            detail.m_concL3 = c.conc;
            detail.m_rackL3 = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            break;
        case 4:
            detail.m_concL4 = c.conc;
            detail.m_rackL4 = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            break;
        case 5:
            detail.m_concL5 = c.conc;
            detail.m_rackL5 = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            break;
        case 6:
            detail.m_concL6 = c.conc;
            detail.m_rackL6 = doc.rack.empty() ? "" : QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
            break;
        default:
            break;
    }

    return true;
}

bool QCaliRackModel::UpdateImRackDetails(const im::tf::CaliDocGroup & grp, QMap<int, CaliRackDetails>& details)
{
    ULOG(LOG_INFO, "%s(), grp id is %lld", __FUNCTION__, grp.id);

    bool isRackSetted = false;
    for (auto& c : grp.caliDocs)
    {
        if (!c.rack.empty())
            isRackSetted = true;

        auto& detail = details[grp.assayCode];
        m_mapImGroupAssays[grp.id] << grp.assayCode;
        GetImCaliRackDetail(c, grp, details[grp.assayCode]);
    }

    return isRackSetted;
}

bool QCaliRackModel::GetImCaliRackDetail(const im::tf::CaliDoc & c, const im::tf::CaliDocGroup & grp, CaliRackDetails& detail)
{
    // 填充基础信息
    detail.m_assayCode = grp.assayCode;
    detail.m_groupId = grp.id;
    //detail.m_bApp = !(m_appAssays[grp.assayCode].isEmpty());
    detail.m_grpName = QString::fromStdString(grp.name);
    detail.m_lot = QString::fromStdString(grp.lot);
    detail.m_exprite = QDateTime::fromString(QString::fromStdString(grp.expiryTime), Qt::ISODate).date().toString(Qt::ISODate);

    switch (c.level)
    {
        case 1:
            detail.m_concL1 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL1 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        case 2:
            detail.m_concL2 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL2 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        case 3:
            detail.m_concL3 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL3 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        case 4:
            detail.m_concL4 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL4 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        case 5:
            detail.m_concL5 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL5 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        case 6:
            detail.m_concL6 = QString::fromStdString(c.conc).toDouble();
            detail.m_rackL6 = c.rack.empty() ? "" : QString::fromStdString(c.rack) + "-" + QString::number(c.pos);
            break;
        default:
            break;
    }

    return true;
}

void QCaliRackModel::AddRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, const QMap<int, CaliRackDetails>& detailFromCaliGroup, int64_t groupId)
{
    // 将信息汇总
    QMap<int, CaliRackDetails>::const_iterator itr = detailFromCaliGroup.constBegin();
    while (itr != detailFromCaliGroup.constEnd()) {
        auto assay = itr.key();
        const auto& detail = itr.value();

        m_rackDetails[assay].push_back(detail);
        ++itr;
        ++m_rowCnt;
        mapGroupAssayCache[groupId] << assay;
    }
}

void QCaliRackModel::DeleteRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, int64_t groupId)
{
    FUNC_ENTRY_LOG();

    if (mapGroupAssayCache.find(groupId) == mapGroupAssayCache.end())
        return;

    for (const auto& assay : mapGroupAssayCache[groupId])
    {
        if (m_rackDetails.find(assay) == m_rackDetails.end())
            continue;

        int i = 0;
        for (const auto& detail : m_rackDetails[assay])
        {
            if (detail.m_groupId == groupId)
            {
                m_rackDetails[assay].removeAt(i);
                if (m_rackDetails[assay].isEmpty())
                    m_rackDetails.remove(assay);
                --m_rowCnt;
                break;
            }
            ++i;
        }
    }

    mapGroupAssayCache.remove(groupId);
}

void QCaliRackModel::ModifyRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, const QMap<int, CaliRackDetails>& detailFromCaliGroup, int64_t groupId)
{
    // 删除缓存中已经不存在的项目数据
    std::set<int> rmAssayCodes;
    if (mapGroupAssayCache.find(groupId) != mapGroupAssayCache.end())
    {
        for (const auto& assay : mapGroupAssayCache[groupId])
        {
            // 已经没有这个项目了，那么就删除
            if (detailFromCaliGroup.find(assay) != detailFromCaliGroup.end())
                continue;

            if (m_rackDetails.find(assay) == m_rackDetails.end())
                continue;

            int i = 0;
            for (const auto& detail : m_rackDetails[assay])
            {
                if (detail.m_groupId == groupId)
                {
                    m_rackDetails[assay].removeAt(i);
                    rmAssayCodes.emplace(assay);
                    if (m_rackDetails[assay].isEmpty())
                        m_rackDetails.remove(assay);
                    --m_rowCnt;
                    break;
                }

                ++i;
            }
        }

        for (const auto& rmCode : rmAssayCodes)
        {
            // 清除原有的缓存
            mapGroupAssayCache[groupId].remove(rmCode);
        }

        if (mapGroupAssayCache[groupId].isEmpty())
            mapGroupAssayCache.remove(groupId);
    }

    // 将信息汇总
    QMap<int, CaliRackDetails>::const_iterator itr = detailFromCaliGroup.constBegin();
    while (itr != detailFromCaliGroup.constEnd()) {
        auto assay = itr.key();
        const auto& detail = itr.value();

        // 找不到的话，就是新增的项目
        if (m_mapChGroupAssays.find(groupId) == m_mapChGroupAssays.end() ||
            m_mapChGroupAssays[groupId].find(assay) == m_mapChGroupAssays[groupId].end())
        {
            m_rackDetails[assay].push_back(detail);
            ++m_rowCnt;
        }
        else
        {
            for (auto& refData : m_rackDetails[assay])
            {
                if (refData.m_groupId == groupId)
                {
                    refData = detail;
                    break;
                }
            }
        }

        mapGroupAssayCache[groupId] << assay;
        ++itr;
    }
}

boost::optional<CaliRackDetails> QCaliRackModel::GetCaliRackDetailByRow(int row) const
{
    int tmpRowCnt = 0;

    // 偏移获取对应行的CaliRackDetails
    auto i = m_rackDetails.constBegin();
    while (i != m_rackDetails.constEnd()) {
        tmpRowCnt += i.value().size();
        if (tmpRowCnt > row)
        {
            int ind = row - (tmpRowCnt - i.value().size());
            if (ind < 0)
            {
                ULOG(LOG_ERROR, "ind is less than 0.");
                return boost::none;
            }
            // 取出信息
            auto& info = i.value()[ind];
            return boost::make_optional(info);
        }

        ++i;
    }

    return boost::none;
}

bool QCaliRackFilter::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    QCaliRackModel* model = static_cast<QCaliRackModel*>(this->sourceModel());
    if (model)
    {
        auto data = model->GetNameAndIsApp(sourceRow);
        auto rackDetails = model->GetCaliRackDetailByRow(sourceRow);
        if (data)
        {
            if (!m_filterName.isEmpty() && !std::get<0>(*data).contains(m_filterName, Qt::CaseInsensitive))
                return false;

            if (m_bFilterApp && std::get<1>(*data) != m_bFilterApp)
                return false;
        }

        if (rackDetails == boost::none || nullptr == CommonInformationManager::GetInstance()->GetAssayInfo((*rackDetails).m_assayCode))
            return false;
    }

    return true;
}

///
/// @brief 打印按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
/// @li 6889/ChenWei，2023年12月14日，添加免疫校准架概况打印
///
void QCaliRackOverview::OnPrintBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 免疫打印
    CaliRackInfo Info;
    if (!GetCaliRackPrintData(Info, true))
    {
        ULOG(LOG_ERROR, "IM_GetCaliRackPrintData Failed!");
        return;
    }

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机！")));
    pTipDlg->exec();

}

///
/// @brief 导出按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
void QCaliRackOverview::OnExportBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出保存文件对话框
	QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
	if (fileName.isNull() || fileName.isEmpty())
	{
		//std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
		//pTipDlg->exec();
        ULOG(LOG_INFO, "%s()", "Cancel");
		return;
	}

    CaliRackInfo Info;
    if (!GetCaliRackPrintData(Info, false))
    {
        // 弹框提示导出失败
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
        pTipDlg->exec();
        return;
    }

    // 导出pdf需要有模板
    bool bRect = false;
    QFileInfo FileInfo(fileName);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliRackInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else 
    {
        // 导出文件
        FileExporter fileExporter;
        bRect = fileExporter.ExportCaliRackInfo(Info, fileName);
    }

	// 弹框提示导出失败
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出成功！") : tr("导出失败！")));
	pTipDlg->exec();

}

///
/// @brief 设置校准架信息（导出使用）
///
/// @param[in] strExportTextList   导出的数据
///
/// @return true:设置成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
bool QCaliRackOverview::SetCaliRackInfo(QStringList& strExportTextList)
{
	// 创建表头
	QString strHeader("");
	strHeader += QObject::tr("项目名称") + "\t";
	strHeader += QObject::tr("校准品名称") + "\t";
	strHeader += QObject::tr("校准品批号") + "\t";
	strHeader += QObject::tr("失效日期") + "\t";
	for (int level = 1; level <= 6; level++)
	{
		strHeader += QObject::tr("校准品") + QString::number(level) + "\t";
		strHeader += QObject::tr("位置") + QString::number(level) + "\t";
	}
	strExportTextList.push_back(std::move(strHeader));

	// 判空
	if (m_tableModel == nullptr)
	{
		ULOG(LOG_ERROR, "QCaliRackModel is nullptr.");
		return false;
	}

	auto addRackInfo = [](QString& row, const double& conc, QString& position)
	{
		row += (conc < 0) ? "" : QString::number(conc);
		row += "\t";
		row += (conc < 0) ? "" : position;
		row += "\t";
	};

	// 填充数据
	for (int rowIndex = 0; rowIndex < m_tableModel->rowCount(); rowIndex++)
	{
		auto&& crdTemp =  m_tableModel->GetCaliRackDetailByRow(rowIndex);
		if (!crdTemp)
		{
			continue;
		}

		QString rowData = "";

		// 1:项目名称
		auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(crdTemp->m_assayCode);
		rowData += QString::fromStdString((spAssay == nullptr) ? "" : spAssay->assayName);
		rowData += "\t";
		// 2:校准品名称
		rowData += crdTemp->m_grpName;
		rowData += "\t";
		// 3:校准品批号
		rowData += crdTemp->m_lot;
		rowData += "\t";
		// 4:失效日期
		rowData += crdTemp->m_exprite;
		rowData += "\t";
		// 5:校准品1
		addRackInfo(rowData, crdTemp->m_concL1, crdTemp->m_rackL1);
		// 7:校准品2
		addRackInfo(rowData, crdTemp->m_concL2, crdTemp->m_rackL2);
		// 8:校准品3
		addRackInfo(rowData, crdTemp->m_concL3, crdTemp->m_rackL3);
		// 9:校准品4
		addRackInfo(rowData, crdTemp->m_concL4, crdTemp->m_rackL4);
		// 10:校准品5
		addRackInfo(rowData, crdTemp->m_concL5, crdTemp->m_rackL5);
		// 11:校准品6
		addRackInfo(rowData, crdTemp->m_concL6, crdTemp->m_rackL6);
		
		strExportTextList.push_back(std::move(rowData));
	}
	
	return true;
}

void QCaliRackOverview::SetPrintAndExportEnabled(bool enabled)
{
    // 暂定为没有数据就不能导出打印
    ui->print_none_flat_btn->setEnabled(enabled);
    ui->export_none_flat_btn->setEnabled(enabled);
}

///
/// @brief 获取校准架概况打印数据（打印使用）
///
/// @param[in] caliRackInfo   校准架概况打印数据
///
/// @return true:获取成功
///
/// @par History:
/// @li 6889/ChenWei，2023年12月13日，新建函数
///
bool QCaliRackOverview::GetCaliRackPrintData(CaliRackInfo& caliRackInfo, bool isPrint)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判空
    if (m_tableModel == nullptr || m_filterModel == nullptr)
    {
        ULOG(LOG_ERROR, "QCaliRackModel is nullptr.");
        return false;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    caliRackInfo.strPrintTime = strPrintTime.toStdString();
    caliRackInfo.strInstrumentModel = std::string(tr("i 6000").toStdString());						// 仪器型号

    for (int i = 0; i < m_filterModel->rowCount(); i++)
    {
        QModelIndex FIndex = m_filterModel->index(i, 0);
        QModelIndex TIndex = m_filterModel->mapToSource(FIndex);

        auto&& crdTemp = m_tableModel->GetCaliRackDetailByRow(TIndex.row());
        if (!crdTemp)
        {
            continue;
        }

        CaliRack CR;

        // 1:项目名称
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(crdTemp->m_assayCode);
        std::string strName;
        if (isPrint && spAssay != nullptr)
        {
            strName = spAssay->printName;
        }
        else
        {
            strName = (spAssay->assayName.empty()) ? spAssay->assayFullName : spAssay->assayName;
        }

        CR.strItemName = strName;
        // 2:校准品名称
        CR.strCaliName = crdTemp->m_grpName.toStdString();
        // 3:校准品批号
        CR.strCaliLot = crdTemp->m_lot.toStdString();
        // 4:失效日期
        CR.strExpirationDate = crdTemp->m_exprite.toStdString();
        // 5:校准品1
        CR.strCalibrator1 = crdTemp->m_concL1 >= 0 ? QString::number(crdTemp->m_concL1).toStdString() : "";
        CR.strPos1 = crdTemp->m_rackL1.toStdString();
        // 7:校准品2
        CR.strCalibrator2 = crdTemp->m_concL2 >= 0 ? QString::number(crdTemp->m_concL2).toStdString() : "";
        CR.strPos2 = crdTemp->m_rackL2.toStdString();
        // 8:校准品3
        CR.strCalibrator3 = crdTemp->m_concL3 >= 0 ? QString::number(crdTemp->m_concL3).toStdString() : "";
        CR.strPos3 = crdTemp->m_rackL3.toStdString();
        // 9:校准品4
        CR.strCalibrator4 = crdTemp->m_concL4 >= 0 ? QString::number(crdTemp->m_concL4).toStdString() : "";
        CR.strPos4 = crdTemp->m_rackL4.toStdString();
        // 10:校准品5
        CR.strCalibrator5 = crdTemp->m_concL5 >= 0 ? QString::number(crdTemp->m_concL5).toStdString() : "";
        CR.strPos5 = crdTemp->m_rackL5.toStdString();
        // 11:校准品6
        CR.strCalibrator6 = crdTemp->m_concL6 >= 0 ? QString::number(crdTemp->m_concL6).toStdString() : "";
        CR.strPos6 = crdTemp->m_rackL6.toStdString();

        caliRackInfo.vecCaliRack.push_back(std::move(CR));

    }

    return true;
}

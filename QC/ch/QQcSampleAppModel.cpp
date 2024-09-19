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

#include "QQcSampleAppModel.h"
#include "thrift/ch/ChLogicControlProxy.h" 
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"

#define  COLUMNSHARE 7
#define  COLUMNSELECT 8

QQcSampleAppModel::QQcSampleAppModel(MODE mode)
    : QAbstractTableModel()
    , m_mode(mode)
{
    if (m_mode == MODE::DEFAULT)
    {
        m_heads << tr("模块") << tr("项目名称") << tr("质控品名称") << tr("质控品水平") << tr("质控品编号")
            << tr("质控品批号") << tr("质控品类型") << tr("失效时间") << tr("选择");
    }
    else
    {
        m_heads << tr("模块") << tr("项目名称") << tr("质控品名称") << tr("质控品水平") << tr("质控品编号")
            << tr("质控品批号") << tr("质控品类型") << tr("质控品原因") << tr("选择");
    }

}

QQcSampleAppModel::~QQcSampleAppModel()
{
}

///
/// @brief
///     设置单元格数据 供视图调用
///
/// @param[in]    index      当前单元格索引
/// @param[in]    value      单元格的值
/// @param[in]    role       被修改的角色类型
///
/// @return       bool       数据处理成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月1日，新建函数
///
bool QQcSampleAppModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != (Qt::UserRole + 1))
    {
        return false;
    }

    // 获取编辑的行和列
    const int col = index.column();
    const int row = index.row();

    // 总列数是9
    if (row >= m_data.size()
        || col > m_heads.size() || m_heads.size() != 9)
    {
        return false;
    }

    // 只有选择列可以被改动
    if (col != COLUMNSELECT)
    {
        return false;
    }

    // 自动更新
    autoCleaner<QQcSampleAppModel> clear(this);

    const auto& data = ShareClass::GetQcApplyInfo(int(m_mode), m_data[row]);
    if (!data)
    {
        ch::tf::QcApply qcApp;
        qcApp.__set_qcDocId(m_data[row].second->id);
        qcApp.__set_deviceSN(m_data[row].first->deviceSN);
        qcApp.__set_assayCode(m_data[row].first->assayCode);
        qcApp.__set_qcReason(::tf::QcReason::QC_REASON_MANNUAL);
        // 在用
        if (m_mode == MODE::ONUSE)
        {
            qcApp.__set_qcType(ch::tf::QcType::type::QC_TYPE_ONUSE);
        }
        // 默认
        else
        {
            qcApp.__set_qcType(ch::tf::QcType::type::QC_TYPE_DEFAULT);
        }

        qcApp.__set_select(true);
        ::tf::ResultLong _return;
        ::ch::LogicControlProxy::AddQcApply(_return, qcApp);
    }
    else
    {
        // 超时质控，只需要修改其中的选中的值即可
        if (data.value().qcReason == ::tf::QcReason::QC_REASON_OVERTIME)
        {
            ch::tf::QcApply qcApp;
            qcApp.__set_id(data.value().id);
            // 取反
            qcApp.__set_select(!data.value().select);
            if (!::ch::LogicControlProxy::ModifyQcApply(qcApp))
            {
                return false;
            }
        }
        else
        {
            ch::tf::QcApplyQueryCond queryCond;
            queryCond.__set_id(data.value().id);
            if (!::ch::LogicControlProxy::DeleteQcApply(queryCond))
            {
                return false;
            }
        }
    }

    return true;
}

///
/// @brief 设置页面显示关键信息
///
/// @param[in]  devices  设备信息
/// @param[in]  qcDoc    复合质控品
///
/// @return true表示显示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月31日，新建函数
///
bool QQcSampleAppModel::SetData(MODE mode, std::vector<std::shared_ptr<const tf::DeviceInfo>>& spdevices, ::tf::QcDoc& qcDoc)
{
    m_mode = mode;
    // 清除旧数据
    m_data.clear();
    // 自动更新
    autoCleaner<QQcSampleAppModel> clear(this);
    // 查找试剂组
    ::ch::tf::ReagentGroupQueryResp qryReagentResult;
    ::ch::tf::ReagentGroupQueryCond qryReagentCond;

    {
        // 设备名称
        std::vector<std::string> devices;
        for (const auto& device : spdevices)
        {
            devices.push_back(device->deviceSN);
        }
        qryReagentCond.__set_deviceSN(devices);
        // 试剂使用状态
        qryReagentCond.__set_usageStatus({ ::tf::UsageStatus::type::USAGE_STATUS_CURRENT });
    }


    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResult, qryReagentCond)
        || qryReagentResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    if (qryReagentResult.lstReagentGroup.empty())
    {
        return true;
    }

    // 生成显示数据列表
    for (const auto& reagent : qryReagentResult.lstReagentGroup)
    {
        auto iter  = std::find_if(qcDoc.compositions.begin(), qcDoc.compositions.end(), [&reagent](auto& compostion) {return (reagent.assayCode == compostion.assayCode); });
        if (iter == qcDoc.compositions.end())
        {
            continue;
        }

        m_data.push_back(std::make_pair<std::shared_ptr<ch::tf::ReagentGroup>, std::shared_ptr<::tf::QcDoc>>
            (std::make_shared<ch::tf::ReagentGroup>(reagent), std::make_shared<::tf::QcDoc>(qcDoc)));
    }

    return true;
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月31日，新建函数
///
// QQcSampleAppModel& QQcSampleAppModel::Instance()
// {
//     static QQcSampleAppModel module;
//     return module;
// }

void QQcSampleAppModel::Init()
{
    ::ch::tf::QcApplyQueryResp _return;
    ::ch::tf::QcApplyQueryCond qcapcd;
    ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
}

Qt::ItemFlags QQcSampleAppModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QQcSampleAppModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int QQcSampleAppModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

QVariant QQcSampleAppModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    const int row = index.row();
    const int column = index.column();
    if (row >= m_data.size() || column >= m_heads.size())
    {
        return QVariant();
    }

    auto data = m_data[row];
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(data.first->assayCode);
    if (!spAssay)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        switch (COLSAMPLE(column))
        {
            case QQcSampleAppModel::COLSAMPLE::MODULE:
                return QString::fromStdString(CommonInformationManager::GetDeviceName(data.first->deviceSN));
                break;
            case QQcSampleAppModel::COLSAMPLE::NAME:
                return QString::fromStdString(spAssay->assayName);
                break;
            case QQcSampleAppModel::COLSAMPLE::SAMPLENAME:
                return QString::fromStdString(data.second->name);
                break;
            case QQcSampleAppModel::COLSAMPLE::LEVEL:
                return QString::number(data.second->level);
                break;
            case QQcSampleAppModel::COLSAMPLE::SN:
                return QString::fromStdString(data.second->sn);
                break;
            case QQcSampleAppModel::COLSAMPLE::LOT:
                return QString::fromStdString(data.second->lot);
                break;
            case QQcSampleAppModel::COLSAMPLE::TYPE:
                return ThriftEnumTrans::GetSourceTypeName(data.second->sampleSourceType);
                break;
            case QQcSampleAppModel::COLSAMPLE::REASON:
            {
                // 显示过期时间，默认质控模式
                if(m_mode == MODE::DEFAULT)
                {
                    return QString::fromStdString(data.second->expireTime);
                }
                else
                {
                    auto qcApp = ShareClass::GetQcApplyInfo(int(m_mode), data);
                    if (!qcApp)
                    {
                        auto qcDefaultApp = ShareClass::GetQcApplyInfo(int(MODE::DEFAULT), data);
                        if (qcDefaultApp)
                        {
                            return tr("默认质控");
                        }

                        return QVariant();
                    }

                    return ShareClass::GetQcReason(qcApp.value().qcReason);
                }
            }
                break;
             // 需特殊处理在后面
            case QQcSampleAppModel::COLSAMPLE::SELECTED:
                return QVariant();
                break;
            default:
                return QVariant();
                break;
        }
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    // 特殊处理，选择项
    else if (role == (Qt::UserRole + 1))
    {
        auto qcApp = ShareClass::GetQcApplyInfo(int(m_mode), data);
        if (!qcApp)
        {
            return false;
        }

		// 如果因为超时而质控，不勾选
        if (qcApp.value().qcReason == ::tf::QcReason::type::QC_REASON_OVERTIME)
        {
            return false;
        }
		// 如果因为非超时而质控，返回勾选状态
        return qcApp.value().select;
    }
    else
    {
        return QVariant();
    }
}

QVariant QQcSampleAppModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

///
/// @brief 获取质控原因
///
/// @param[in]  reason  质控原因
///
/// @return 原因字符
///
/// @par History:
/// @li 5774/WuHongTao，2022年09月05日，新建函数
///
QString ShareClass::GetQcReason(const int32_t reason)
{
    switch (::tf::QcReason::type(reason))
    {
		case ::tf::QcReason::QC_REASON_MANNUAL:
        {
            return tr("手动选择");
        }
        break;

        case ::tf::QcReason::QC_REASON_DEFAULT:
        {
            return tr("默认质控");
        }
        break;
        
        case ::tf::QcReason::QC_REASON_OVERTIME:
        {
            return tr("超时质控");
        }
        break;

		case ::tf::QcReason::QC_REASON_ON_TIMER:
		{
			return tr("定时质控");
		}
		break;

		case ::tf::QcReason::QC_REASON_OUT_SAMPLE_COUNT:
		{
			return tr("超样本数");
		}
		break;

		case ::tf::QcReason::QC_REASON_AFTER_CALI:
		{
			return tr("校准后质控");
		}
		break;

        default:
            break;
    }

    return tr("");
}

///
/// @brief 获取质控申请信息
///
/// @param[in]  data  质控文档和复合质控品
///
/// @return 质控申请信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年09月05日，新建函数
///
boost::optional<ch::tf::QcApply> ShareClass::GetQcApplyInfo(int mode, const DataPair& data)
{
    ::ch::tf::QcApplyQueryResp _return;
    ::ch::tf::QcApplyQueryCond qcapcd;
    qcapcd.__set_qcDocId(data.second->id);
    qcapcd.__set_deviceSN(data.first->deviceSN);
    qcapcd.__set_assayCode(data.first->assayCode);

    // 在用
    if (mode == 0)
    {
        qcapcd.__set_qcType(ch::tf::QcType::type::QC_TYPE_ONUSE);
    }
    // 默认质控
    else if(mode == 1)
    {
        qcapcd.__set_qcType(ch::tf::QcType::type::QC_TYPE_DEFAULT);
    }
    // 备用试剂瓶的质控信息
    else
    {
        // 试剂盒的关键信息列表(从中获取批号瓶号)
        if (data.first->reagentKeyInfos.empty())
        {
            return boost::none;
        }

        auto keyInfo = data.first->reagentKeyInfos[0];
        // 必须加上试剂品的批号瓶号等详细信息
        qcapcd.__set_lot(keyInfo.lot);
        qcapcd.__set_sn(keyInfo.sn);
        qcapcd.__set_qcType(ch::tf::QcType::type::QC_TYPE_BACKUP);
    }

    ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || _return.lstQcApplys.empty())
    {
        return boost::none;
    }

    return _return.lstQcApplys[0];
}

QQcSpareAppModel::QQcSpareAppModel()
    : QAbstractTableModel()
{
    m_heads << tr("模块") << tr("项目名称")<< tr("试剂瓶号") << tr("试剂批号") << tr("质控品名称") << tr("质控品水平") << tr("质控品编号")
        << tr("质控品批号") << tr("质控品类型") << tr("失效时间") << tr("选择");
}

QQcSpareAppModel::~QQcSpareAppModel()
{

}

///
/// @brief
///     设置单元格数据 供视图调用
///
/// @param[in]    index      当前单元格索引
/// @param[in]    value      单元格的值
/// @param[in]    role       被修改的角色类型
///
/// @return       bool       数据处理成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月6日，新建函数
///
bool QQcSpareAppModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != (Qt::UserRole + 1))
    {
        return false;
    }

    // 获取编辑的行和列
    const int col = index.column();
    const int row = index.row();

    // 总列数是9
    if (row >= m_data.size()
        || col > m_heads.size() || m_heads.size() != 11)
    {
        return false;
    }

    // 只有选择列可以被改动
    if (col != (COLUMNSELECT+2))
    {
        return false;
    }

    // 自动更新
    autoCleaner<QQcSpareAppModel> clear(this);

    const auto& data = ShareClass::GetQcApplyInfo(-1, m_data[row]);
    if (!data)
    {
        // 试剂盒的关键信息列表(从中获取批号瓶号)
        if (m_data[row].first->reagentKeyInfos.empty())
        {
            return false;
        }

        auto keyInfo = m_data[row].first->reagentKeyInfos[0];
        ch::tf::QcApply qcApp;
        qcApp.__set_qcDocId(m_data[row].second->id);
        qcApp.__set_deviceSN(m_data[row].first->deviceSN);
        qcApp.__set_assayCode(m_data[row].first->assayCode);
        qcApp.__set_qcReason(::tf::QcReason::QC_REASON_MANNUAL);
        // 必须加上试剂品的批号瓶号等详细信息
        qcApp.__set_lot(keyInfo.lot);
        qcApp.__set_sn(keyInfo.sn);
        qcApp.__set_qcType(ch::tf::QcType::type::QC_TYPE_BACKUP);

        qcApp.__set_select(true);
        ::tf::ResultLong _return;
        ::ch::LogicControlProxy::AddQcApply(_return, qcApp);
    }
    else
    {
        // 超时质控，只需要修改其中的选中的值即可
        if (data.value().qcReason == ::tf::QcReason::QC_REASON_OVERTIME)
        {
            ch::tf::QcApply qcApp;
            qcApp.__set_id(data.value().id);
            // 取反
            qcApp.__set_select(!data.value().select);
            if (!::ch::LogicControlProxy::ModifyQcApply(qcApp))
            {
                return false;
            }
        }
        else
        {
            ch::tf::QcApplyQueryCond queryCond;
            queryCond.__set_id(data.value().id);
            if (!::ch::LogicControlProxy::DeleteQcApply(queryCond))
            {
                return false;
            }
        }
    }

    return true;
}

///
/// @brief 设置页面显示关键信息
///
/// @param[in]  spdevices设备信息
/// @param[in]  qcDoc    复合质控品
///
/// @return true表示显示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月6日，新建函数
///
bool QQcSpareAppModel::SetData(std::vector<std::shared_ptr<const tf::DeviceInfo>>& spdevices, ::tf::QcDoc& qcDoc)
{
    // 清除旧数据
    m_data.clear();
    // 自动更新
    autoCleaner<QQcSpareAppModel> clear(this);
    // 查找试剂组
    ::ch::tf::ReagentGroupQueryResp qryReagentResult;
    ::ch::tf::ReagentGroupQueryCond qryReagentCond;

    {
        // 设备名称
        std::vector<std::string> devices;
        for (const auto& device : spdevices)
        {
            devices.push_back(device->deviceSN);
        }
        qryReagentCond.__set_deviceSN(devices);
        // 试剂使用状态--备用试剂
        qryReagentCond.__set_usageStatus({ ::tf::UsageStatus::type::USAGE_STATUS_BACKUP });
    }


    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResult, qryReagentCond)
        || qryReagentResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    if (qryReagentResult.lstReagentGroup.empty())
    {
        return true;
    }

    // 生成显示数据列表
    for (const auto& reagent : qryReagentResult.lstReagentGroup)
    {
        auto iter = std::find_if(qcDoc.compositions.begin(), qcDoc.compositions.end(), [&reagent](auto& compostion) {return (reagent.assayCode == compostion.assayCode); });
        if (iter == qcDoc.compositions.end())
        {
            continue;
        }

        m_data.push_back(std::make_pair<std::shared_ptr<ch::tf::ReagentGroup>, std::shared_ptr<::tf::QcDoc>>
            (std::make_shared<ch::tf::ReagentGroup>(reagent), std::make_shared<::tf::QcDoc>(qcDoc)));
    }

    return true;
}

QVariant QQcSpareAppModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    const int row = index.row();
    const int column = index.column();
    if (row >= m_data.size() || column >= m_heads.size())
    {
        return QVariant();
    }

    auto data = m_data[row];
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(data.first->assayCode);
    if (!spAssay)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        switch (COLSAMPLE(column))
        {
            case QQcSpareAppModel::COLSAMPLE::MODULE:
                return QString::fromStdString(CommonInformationManager::GetDeviceName(data.first->deviceSN));
                break;
            case QQcSpareAppModel::COLSAMPLE::NAME:
                return QString::fromStdString(spAssay->assayName);
                break;
            case QQcSpareAppModel::COLSAMPLE::SAMPLENAME:
                return QString::fromStdString(data.second->name);
                break;
            case QQcSpareAppModel::COLSAMPLE::LEVEL:
                return QString::number(data.second->level);
                break;
            case QQcSpareAppModel::COLSAMPLE::SN:
                return QString::fromStdString(data.second->sn);
                break;
            case QQcSpareAppModel::COLSAMPLE::LOT:
                return QString::fromStdString(data.second->lot);
                break;
            case QQcSpareAppModel::COLSAMPLE::TYPE:
                return ThriftEnumTrans::GetSourceTypeName(data.second->sampleSourceType);
                break;
            case QQcSpareAppModel::COLSAMPLE::REASON:
                return QString::fromStdString(data.second->expireTime);
                break;
                // 试剂的瓶号
            case QQcSpareAppModel::COLSAMPLE::REAGENTSN:
            {
                if (data.first->reagentKeyInfos.empty())
                {
                    return QVariant();
                }

                auto keyinfo = data.first->reagentKeyInfos[0];
                return QString::fromStdString(keyinfo.sn);
            }
                break;
                // 试剂的批号
            case QQcSpareAppModel::COLSAMPLE::REAGENTLOT:
            {
                if (data.first->reagentKeyInfos.empty())
                {
                    return QVariant();
                }

                auto keyinfo = data.first->reagentKeyInfos[0];
                return QString::fromStdString(keyinfo.lot);
            }
                break;
            // 需特殊处理在后面
            case QQcSpareAppModel::COLSAMPLE::SELECTED:
                return QVariant();
                break;
            default:
                return QVariant();
                break;
        }
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    // 特殊处理，选择项
    else if (role == (Qt::UserRole + 1))
    {
        // 除开0,1和都是查询备用试剂瓶的质控申请信息
        auto qcApp = ShareClass::GetQcApplyInfo(-1, data);
        if (!qcApp)
        {
            return false;
        }

        if (qcApp.value().qcReason == ::tf::QcReason::type::QC_REASON_OVERTIME
            && !qcApp.value().select)
        {
            return false;
        }

        return true;
    }
    else
    {
        return QVariant();
    }
}

QVariant QQcSpareAppModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags QQcSpareAppModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QQcSpareAppModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int QQcSpareAppModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

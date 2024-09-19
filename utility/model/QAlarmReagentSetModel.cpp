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
/// @file     QAlarmReagentSetModel.h
/// @brief    试剂告警设置模式
///
/// @author   5774/WuHongTao
/// @date     2022年6月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QIntValidator>
#include "QAlarmReagentSetModel.h"
#include "thrift/DcsControlProxy.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "manager/DictionaryQueryManager.h"

#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"


QAlarmReagentSetModel::QAlarmReagentSetModel()
{
	//m_headerNames << tr("No") << tr("模块") << tr("项目名称") << tr("瓶注意级别（测试数") << tr("项目注意级别（测试数)");
    m_headerNames << tr("系统") << tr("项目名称") << tr("瓶报警值") << tr("项目报警值") << tr("单位");
	Update();
}

QAlarmReagentSetModel::~QAlarmReagentSetModel()
{
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
QAlarmReagentSetModel& QAlarmReagentSetModel::Instance()
{
	static QAlarmReagentSetModel model;
	return model;
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
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
bool QAlarmReagentSetModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	if (role != Qt::EditRole)
	{
		return false;
	}

	// 获取编辑的行和列
	const int col = index.column();
	const int row = index.row();

	if (row >= m_data.size() || col > m_headerNames.size())
	{
		return false;
	}

	// 改动数据
    int32_t val = 0;
    auto& rowData = m_data.at(row).second;

    if (col == (int)COL::BOTTLEWARNING || col == (int)COL::ASSAYWARNING)
    {
        if (rowData.generalAssayInfo.has_value())
        {
            val = value.toInt();
            if ((val <= 0 || val > 99999999) && !value.toString().isEmpty())
            {
                TipDlg(tr("提示"), tr("值错误：1≤报警值≤99999999")).exec();
                return false;
            }
        }
        else
        {
            // 耗材最大容量500ml
            QRegExp re("^([0-9]{1,3}(\\.[0-9]{0,4})?)?$");
            if (!re.exactMatch(value.toString()) && !value.toString().isEmpty())
            {
                TipDlg(tr("提示"), tr("值错误：耗材报警值必须为正数，最多可设置整数位三位、小数位四位")).exec();
                return false;
            }

            // 单位由ml转为0.1ul,放大10000倍
            // 为避免直接转换成double再放大10000倍因精度导致的误差,先转成字符串再补0
            QStringList tempList = value.toString().split(".");
            if (tempList.size() < 2)
            {
                val = tempList[0].toInt() * 10000;
            }
            else
            {
                QString tempStr(tempList[0] + tempList[1]);
                for (int i = tempList[1].size(); i < 4; i++)
                {
                    tempStr += "0";
                }

                val = tempStr.toInt();
            }

            if (!value.toString().isEmpty() && val == 0)
            {
                TipDlg(tr("提示"), tr("值错误：耗材报警值不能设置为0")).exec();
                return false;
            }

            if (val > 500 * 10000)
            {
                TipDlg(tr("提示"), tr("值错误：耗材最大容量500ml")).exec();
                return false;
            }
        }
    }

	switch ((COL)col)
	{
        // 项目告警门限
        case COL::BOTTLEWARNING:
            {
                m_data.at(row).second.set_bottleAlarmThreshold(val);
                break;
            }

            // 项目告警门限
        case COL::ASSAYWARNING:
            {
                m_data.at(row).second.set_assayAlarmThreshold(val);
                break;
            }

		default:
			return false;
	}

	// 标识数据被改动过
	m_data.at(row).first = true;
	return true;
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
Qt::ItemFlags QAlarmReagentSetModel::flags(const QModelIndex &index) const
{
	const int col = index.column();
	const int row = index.row();

	const Qt::ItemFlags default_flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if (col == static_cast<int>(COL::ASSAYWARNING) || col == static_cast<int>(COL::BOTTLEWARNING))
	{
		return default_flag | Qt::ItemIsEditable;
	}

	return default_flag;
}

///
/// @brief 更新模式里面的数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
void QAlarmReagentSetModel::Update()
{
    m_data.clear();

	tf::GeneralAssayInfoQueryCond queryCond;
	tf::GeneralAssayInfoQueryResp respGeneral;
	DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(respGeneral, queryCond);
	if (respGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return;
	}

    // 更新数据
    beginResetModel();

    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
	for (const auto& assayInfo : respGeneral.lstAssayInfos)
	{
        // 排查L、H、I
        if (assayInfo.assayCode <= ch::tf::g_ch_constants.ASSAY_CODE_I
            && assayInfo.assayCode >= ch::tf::g_ch_constants.ASSAY_CODE_L)
        {
            continue;
        }

		if (assayInfo.assayCode <= ::tf::AssayCodeRange::CH_RANGE_MAX)
		{
            // 排除ISE项目,如果是免疫则不显示生化项目
            if ((assayInfo.assayCode >= ise::tf::g_ise_constants.ASSAY_CODE_ISE
                    && assayInfo.assayCode <= ise::tf::g_ise_constants.ASSAY_CODE_CL)
                || (type == IMMUNE))
            {
                continue;
            }

            //生化试剂项目添加
            m_data.push_back(std::make_pair(false, assayInfo));
		}
        else if(type == IMMUNE || type == CHEMISTRY_AND_IMMUNE)
        {
            //免疫试剂项目添加
            m_data.push_back(std::make_pair(false, assayInfo));
        }
	}

    // 仓类耗材显示到试剂提示页面
    if (type == CHEMISTRY || type == CHEMISTRY_AND_IMMUNE)
    {
        // 生化
        ::ch::tf::SupplyAttributeQueryCond queryCond;
        ::ch::tf::SupplyAttributeQueryResp respGeneral;
        ch::LogicControlProxy::QuerySupplyAttribute(respGeneral, queryCond);
        if (respGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to query all ch supplyattribute.");
            return;
        }

        for (const auto& supplyAttribute : respGeneral.lstSupplyAttributes)
        {
            if ((supplyAttribute.type >= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MIN
                && supplyAttribute.type <= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MAX)
                || (supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY
                    && !supplyAttribute.sampleProbe)
                || (supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY
                    && !supplyAttribute.sampleProbe)
                || supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_HITERGENT)
            {
                //添加生化仓内耗材
                m_data.push_back(std::make_pair(false, supplyAttribute));
            }
        }
    }

    if (type == IMMUNE || type == CHEMISTRY_AND_IMMUNE)
    {
        // 免疫稀释液和特殊清洗液信息存储在耗材表
        // 特殊处理更改到在试剂信息显示 bug 0022102 by ldx
        ::im::tf::SupplyAttributeQueryCond imQryCond;
        ::im::tf::SupplyAttributeQueryResp imQryRespGeneral;
        im::LogicControlProxy::QuerySupplyAttribute(imQryRespGeneral, imQryCond);
        if (imQryRespGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to query all im supplyattribute.");
            return;
        }

        // 免疫试剂存在版本升级情况，会出现重复名字试剂，使用map去重
        std::map<int, ::im::tf::SupplyAttribute> imSupplyAttribute;
        for (const auto& supplyAttribute : imQryRespGeneral.lstSupplyAttributes)
        {
            if (supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT
                || supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH)
            {
                //添加免疫特殊清洗液和稀释液
                imSupplyAttribute[supplyAttribute.type] = supplyAttribute;
            }
        }

        for (auto& iter : imSupplyAttribute)
        {
            //添加免疫特殊清洗液和稀释液
            m_data.push_back(std::make_pair(false, iter.second));
        }
    }

	endResetModel();
}

///
/// @brief 保存报警参数
///
///
/// @return true表示保存成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月1日，新建函数
///
bool QAlarmReagentSetModel::SaveAlarmParameter(bool &hasChangeParam, bool &changeSupply)
{
	// 依次保存报警参数
	for (auto& iter : m_data)
	{
		// 若参数没有变动
		if (!iter.first)
		{
			continue;
		}

		hasChangeParam = true;

        // 项目参数
        if (iter.second.generalAssayInfo.has_value())
        {
            // 项目参数
            auto& assay = iter.second.generalAssayInfo.value();
            //修改所有同一通道的数据库字段
            tf::GeneralAssayInfo modifyParameter;
            // 设置参数id
            modifyParameter.__set_id(assay.id);
            // 瓶限制，所有名字相同通道保存报警值
            modifyParameter.__set_bottleAlarmThreshold(assay.bottleAlarmThreshold);
            // 项目限制，所有名字相同通道保存报警值
            modifyParameter.__set_assayAlarmThreshold(assay.assayAlarmThreshold);
            // 修改配置参数
            DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(modifyParameter);

            //若为免疫单机版
            if (CommonInformationManager::GetInstance()->GetImisSingle())
            {
                //更新下位机中的试剂报警
                im::LogicControlProxy::UpdateLowerAlarmValue(::im::tf::SuppliesType::type::SUPPLIES_TYPE_DILUENT);
                im::LogicControlProxy::UpdateLowerAlarmValue(::im::tf::SuppliesType::type::SUPPLIES_TYPE_SPECIAL_WASH);
            }
            
        }
        else if(iter.second.supplyAttributeCh.has_value())
        {
			changeSupply = true;

            // 生化仓内耗材
            auto& chData = iter.second.supplyAttributeCh.value();
            ch::tf::SupplyAttribute modifyParameter;
            // 设置参数id
            modifyParameter.__set_id(chData.id);
			// 设置类型
			modifyParameter.__set_type(chData.type);
            // 瓶限制
            modifyParameter.__set_bottleAlarmThreshold(chData.bottleAlarmThreshold);
            // 项目限制
            modifyParameter.__set_assayAlarmThreshold(chData.assayAlarmThreshold);
            // 修改配置参数
            ch::LogicControlProxy::ModifySupplyAttribute(modifyParameter);
        }
        else
        {
			changeSupply = true;

            // 免疫仓内耗材
            auto& imData = iter.second.supplyAttributeIm.value();
            im::tf::SupplyAttribute modifyParameter;
            // 设置参数id
            modifyParameter.__set_id(imData.id);
            // 耗材类型
            modifyParameter.__set_type(imData.type);
            // 瓶限制
            modifyParameter.__set_bottleAlarmThreshold(imData.bottleAlarmThreshold);
            // 项目限制
            modifyParameter.__set_assayAlarmThreshold(imData.assayAlarmThreshold);
            // 修改配置参数
            im::LogicControlProxy::ModifySupplyAttribute(modifyParameter);
        }

        iter.first = false;
	}

	return true;
}

bool QAlarmReagentSetModel::isExistChangedData()
{
    for (auto iter : m_data)
    {
        // 若参数变动
        if (iter.first)
        {
            return true;
        }
    }

    return false;
}

int QAlarmReagentSetModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int QAlarmReagentSetModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant QAlarmReagentSetModel::data(const QModelIndex &index, int role) const
{
    // 对齐方式
    if (role == Qt::TextAlignmentRole) 
    {
        return  Qt::AlignCenter;
    }

	const int row = index.row();
	const int column = index.column();

	// 只处理显示角色(顺序和模块名称三个项目)
	if (role != Qt::DisplayRole && std::set<COL>({ COL::UNIT, COL::MODEL, COL::NAME }).count(COL(column)))
	{
		return QVariant();
	}

	// 越界控制
	if (row >= m_data.size())
	{
		return QVariant();
	}

    auto& data = m_data[row].second;
    // 获取数据
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (COL(column))
        {
            case QAlarmReagentSetModel::COL::MODEL:
                {
                    if (data.supplyAttributeCh.has_value())
                    {
                        return tr("生化");
                    }
                    else if (data.supplyAttributeIm.has_value())
                    {
                        return tr("免疫");
                    }
                    else
                    {
                        return ConvertTfEnumToQString(data.generalAssayInfo.value().assayClassify);
                    }
                }
               
            case QAlarmReagentSetModel::COL::NAME:
                {
                    if (data.supplyAttributeCh.has_value())
                    {
                        auto& chData = data.supplyAttributeCh.value();
                        return ThriftEnumTrans::GetSupplyName(chData.type, !chData.sampleProbe);
                    }
                    else if (data.supplyAttributeIm.has_value())
                    {
                        auto& imData = data.supplyAttributeIm.value();
                        return ThriftEnumTrans::GetImSupplyName(imData.type);
                    }
                    else
                    {
                        return QString::fromStdString(data.generalAssayInfo.value().assayName);
                    }
                }

            case QAlarmReagentSetModel::COL::BOTTLEWARNING:
                {
                    if (data.supplyAttributeCh.has_value())
                    {
                        auto& chData = data.supplyAttributeCh.value();
                        if (chData.bottleAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(double(chData.bottleAlarmThreshold) / 10000.0); // 0.1ul转为ml
                        }
                    }
                    else if (data.supplyAttributeIm.has_value())
                    {
                        auto& imData = data.supplyAttributeIm.value();
                        if (imData.bottleAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(double(imData.bottleAlarmThreshold / 10000.0)); // 0.1ul转为ml
                        }
                    }
                    else
                    {
                        auto& assayData = data.generalAssayInfo.value();
                        if (assayData.bottleAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(assayData.bottleAlarmThreshold); // 项目的瓶报警门限，单位：个
                        }
                    }
                }

            case QAlarmReagentSetModel::COL::ASSAYWARNING:
                {
                    if (data.supplyAttributeCh.has_value())
                    {
                        auto& chData = data.supplyAttributeCh.value();
                        if (chData.assayAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(double(chData.assayAlarmThreshold) / 10000.0); // 0.1ul转为ml
                        }
                    }
                    else if (data.supplyAttributeIm.has_value())
                    {
                        auto& imData = data.supplyAttributeIm.value();
                        if (imData.assayAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(double(imData.assayAlarmThreshold / 10000.0)); // 0.1ul转为ml
                        }
                    }
                    else
                    {
                        auto& assayData = data.generalAssayInfo.value();
                        if (assayData.assayAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(assayData.assayAlarmThreshold); // 项目的项目报警门限，单位：个
                        }
                    }
                }

            case QAlarmReagentSetModel::COL::UNIT:
                //免疫特殊清洗液和清洗液特殊处理
                if (data.generalAssayInfo.has_value())
                {
                    return tr("个");
                }
                return tr("ml");

            default:
                return QVariant();
        }
    }
    else
    {
        return QVariant();
    }
}

QVariant QAlarmReagentSetModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
	    return m_headerNames.at(section);
	}

    if ((section == (int)QAlarmReagentSetModel::COL::BOTTLEWARNING || section == (int)QAlarmReagentSetModel::COL::ASSAYWARNING)
        && role == Qt::ForegroundRole && orientation == Qt::Horizontal)
    {
        return QColor(UI_HEADER_FONT_COLOR);
    }

	return QAbstractTableModel::headerData(section, orientation, role);
}

QAlarmWasteSetModel& QAlarmWasteSetModel::Instance()
{
    static QAlarmWasteSetModel model;
    return model;
}

bool QAlarmWasteSetModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    // 获取编辑的行和列
    const int col = index.column();
    const int row = index.row();

    if (row > m_supplyAttributes.size() + m_chWasteNum || col > m_headerNames.size())
    {
        return false;
    }

    // 改动数据
    switch ((COL)col)
    {
        // 报警值
        case COL::WARNING:
            {
                if (row < m_supplyAttributes.size() &&
                    im::tf::SuppliesType::SUPPLIES_TYPE_SOLID == m_supplyAttributes[row].second.type)
                {
                    // 免疫废弃反应杯
                    QRegExp re("^[0-9]\\d{0,4}$");
                    if (!re.exactMatch(value.toString()) && !value.toString().isEmpty())
                    {
                        TipDlg(tr("提示"), tr("值错误：1≤废弃反应杯≤99999")).exec();
                        return false;
                    }
                }
                else
                {
                    // 废液桶最大容量20L
                    QRegExp re("^([0-9]{1,5}(\\.[0-9]{0,4})?)?$");

                    if (!re.exactMatch(value.toString()) && !value.toString().isEmpty())
                    {
                        TipDlg(tr("提示"), tr("值错误：废液的报警值必须为正数，最多可设置五位整数、四位小数")).exec();
                        return false;
                    }
                }

                // 单位由ml转为0.1ul,放大10000倍
                // 为避免直接转换成double再放大10000倍因精度导致的误差,先转成字符串再补0
                int val = 0;
                QStringList tempList = value.toString().split(".");
                if (tempList.size() < 2)
                {
                    val = tempList[0].toInt() * 10000;
                }
                else
                {
                    QString tempStr(tempList[0] + tempList[1]);
                    for (int i = tempList[1].size(); i < 4; i++)
                    {
                        tempStr += "0";
                    }

                    val = tempStr.toInt();
                }

                if (!value.toString().isEmpty() && val == 0)
                {
                    TipDlg(tr("提示"), tr("值错误：废液桶的报警值不能设置为0")).exec();
                    return false;
                }

                if (val > 20 * 1000 * 10000)
                {
                    TipDlg(tr("提示"), tr("值错误：废液桶最大容量20L")).exec();
                    return false;
                }

                if (row < m_supplyAttributes.size())
                {
                    m_supplyAttributes.at(row).second.__set_bottleAlarmThreshold(val);// ml转为0.1ul
                }
                else
                {
                    m_chWasteWarning = round(val);// ml转为0.1ul
                    return true;
                }
                break; 
            }

        default:
            return false;

    }

    // 标识数据被改动过
    m_supplyAttributes.at(row).first = true;
    return true;
}

Qt::ItemFlags QAlarmWasteSetModel::flags(const QModelIndex &index) const
{
    const int col = index.column();
    const int row = index.row();

    const Qt::ItemFlags default_flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (col == static_cast<int>(COL::WARNING))
    {
        return default_flag | Qt::ItemIsEditable;
    }

    return default_flag;
}

void QAlarmWasteSetModel::Update()
{
    if (m_softType == CHEMISTRY || m_softType == CHEMISTRY_AND_IMMUNE)
    {
        //没有生成数据会引起免疫读取不了数据，暂时屏蔽返回
        DictionaryQueryManager::GetWasteBucketWarningValueCh(m_chWasteWarning);
    }

    ::im::tf::SupplyAttributeQueryResp imQryRespGeneral;
    if (m_softType == IMMUNE || m_softType == CHEMISTRY_AND_IMMUNE)
    {
        //免疫
        ::im::tf::SupplyAttributeQueryCond imQryCond;
        im::LogicControlProxy::QuerySupplyAttribute(imQryRespGeneral, imQryCond);
        if (imQryRespGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to query all im supplyattribute.");
            return;
        }
    }

    // 更新数据
    beginResetModel();

    m_supplyAttributes.clear();

    // 免疫的值
    for (const auto& supplyAttribute : imQryRespGeneral.lstSupplyAttributes)
    {
        //获取废液桶和废料桶
        if (supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID
            || supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID)
        {
            m_supplyAttributes.push_back(std::make_pair(false, im::tf::SupplyAttribute(supplyAttribute)));
        }
    }

    endResetModel();
}

bool QAlarmWasteSetModel::SaveAlarmParameter()
{
    // 保存生化废液桶
    if (m_chWasteNum)
    {
		DictionaryQueryManager::GetInstance()->SaveWasteBucketWarningValueCh(m_chWasteWarning);
    }

    // 依次保存免疫报警参数
    for (auto& supplyAttribute : m_supplyAttributes)
    {
        // 若参数没有变动
        if (!supplyAttribute.first)
        {
            continue;
        }

        // 免疫参数
        auto& ImData = supplyAttribute.second;
        im::tf::SupplyAttribute modifyParameter;
        // 设置参数id
        modifyParameter.__set_id(ImData.id);
        // 设置参数类型
        modifyParameter.__set_type(ImData.type);
        // 瓶限制
        modifyParameter.__set_bottleAlarmThreshold(ImData.bottleAlarmThreshold);
        // 项目限制
        modifyParameter.__set_assayAlarmThreshold(ImData.assayAlarmThreshold);
        // 稳定天数
        modifyParameter.__set_stableDays(ImData.stableDays);
        // 修改配置参数
        im::LogicControlProxy::ModifySupplyAttribute(modifyParameter);

        //通知下位机更新报警值
        im::LogicControlProxy::UpdateLowerAlarmValue(ImData.type);

        supplyAttribute.first = false;
    }

    return true;
}

bool QAlarmWasteSetModel::isExistChangedData()
{
    // 保存生化废液桶
    if (m_chWasteNum)
    {
        int tempData;
        DictionaryQueryManager::GetWasteBucketWarningValueCh(tempData);
        if (tempData != m_chWasteWarning)
        {
            return true;
        }
    }

    for (auto supplyAttribute : m_supplyAttributes)
    {
        // 若参数变动
        if (supplyAttribute.first)
        {
            return true;
        }
    }

    return false;
}

int QAlarmWasteSetModel::rowCount(const QModelIndex &parent) const
{
    return m_supplyAttributes.size() + m_chWasteNum;
}

int QAlarmWasteSetModel::columnCount(const QModelIndex &parent) const
{
    return m_headerNames.size();
}

QVariant QAlarmWasteSetModel::data(const QModelIndex &index, int role) const
{
    // 对齐方式
    if (role == Qt::TextAlignmentRole)
    {
        return  Qt::AlignCenter;
    }

    const int row = index.row();
    const int column = index.column();

    // 只处理显示角色(顺序和模块名称三个项目)
    if (role != Qt::DisplayRole && std::set<COL>({ COL::UNIT, COL::MODEL, COL::NAME }).count(COL(column)))
    {
        return QVariant();
    }

    // 获取数据
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (COL(column))
        {
            case QAlarmWasteSetModel::COL::MODEL: 
                {
                    if (row < m_supplyAttributes.size())
                    {
                        return tr("免疫");
                    }

                    // 生化
                    return tr("生化");
                }

            case QAlarmWasteSetModel::COL::NAME:
                {
                    //先显示免疫的废液/废料
                    if (row < m_supplyAttributes.size())
                    {
                        return ThriftEnumTrans::GetImSupplyName(m_supplyAttributes[row].second.type);
                    }

                    //再显示生化的废液/废料
                    return tr("废液");
                }

            case QAlarmWasteSetModel::COL::WARNING:
                {
                    if (row < m_supplyAttributes.size())
                    {
                        if (m_supplyAttributes[row].second.bottleAlarmThreshold == 0)
                        {
                            return "";
                        } 
                        else
                        {
                           return QString::number(double(m_supplyAttributes[row].second.bottleAlarmThreshold) / 10000.0); // 0.1ul转为ml
                        }
                    }

                    // 生化废液桶
                    return m_chWasteWarning == 0 ? "" : QString::number(double(m_chWasteWarning) / 10000.0); // 0.1ul转为ml
                }
                
            case QAlarmWasteSetModel::COL::UNIT:
                {
                    if (row < m_supplyAttributes.size())
                    {
                        if (m_supplyAttributes[row].second.type == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID)
                        {
                            return tr("个");
                        }
                    }

                    return tr("ml");
                }

            default:
                return QVariant();
                break;
        }
    }
    return QVariant();
}

QVariant QAlarmWasteSetModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal  && role == Qt::DisplayRole) 
    {
        return m_headerNames.at(section);
    }

    if ((section == (int)QAlarmWasteSetModel::COL::WARNING) && (role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        return QColor(UI_HEADER_FONT_COLOR);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QAlarmWasteSetModel::QAlarmWasteSetModel()
{
    m_headerNames << tr("系统") << tr("名称") << tr("报警值") << tr("单位");

    m_softType = CommonInformationManager::GetInstance()->GetSoftWareType();

    if (m_softType == IMMUNE)
    {
        m_chWasteNum = 0;
    }
    else
    {
        m_chWasteNum = 1;
    }

    Update();
}

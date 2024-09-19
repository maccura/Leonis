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
/// @file     QAlarmSupplySetModel.h
/// @brief    耗材报警设定
///
/// @author   5774/WuHongTao
/// @date     2022年7月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAlarmSupplySetModel.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ise/ISELogicControlProxy.h"

QAlarmSupplySetModel::QAlarmSupplySetModel()
	: QAbstractTableModel()
{
	//m_headerNames << tr("No") << tr("模块") << tr("项目名称") << tr("瓶注意级别（ml") << tr("项目注意级别（ml)") << tr("开瓶有效期（天)");
    m_headerNames << tr("系统") << tr("耗材名称") << tr("报警值") << tr("单位");
	
    m_softType = CommonInformationManager::GetInstance()->GetSoftWareType();
    
    Update();
}

QAlarmSupplySetModel::~QAlarmSupplySetModel()
{
}

///
/// @brief 获取模式的单例
///
///
/// @return 单例
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月5日，新建函数
///
QAlarmSupplySetModel& QAlarmSupplySetModel::Instance()
{
	static QAlarmSupplySetModel model;
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
bool QAlarmSupplySetModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	if (role != Qt::EditRole)
	{
		return false;
	}

	// 获取编辑的行和列
	const int col = index.column();
	const int row = index.row();

	if (row >= m_supplyAttributes.size() || col > m_headerNames.size())
	{
		return false;
	}

    auto& rowData = m_supplyAttributes.at(row);

	// 改动数据
	switch ((COL)col)
	{
		// 瓶告警门限
		case COL::ASSAYWARNING: 
            {
                int val = 0;

                // 免疫反应杯输入限制
                if (rowData.second.supplyAttributeIm.has_value())
                {
                    auto& imData = rowData.second.supplyAttributeIm.value();
                    if (imData.type == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
                    {
                        val = value.toInt();
                        if ((val <= 0 || val > 99999) && !value.toString().isEmpty())
                        {
                            TipDlg(tr("提示"), tr("值错误：1≤报警值≤99999")).exec();
                            return false;
                        }
                    }
                }
                else
                {
                    // 耗材最大容量500ml
                    QRegExp re("^([0-9]{1,3}(\\.[0-9]{0,4})?)?$");
                    if (!re.exactMatch(value.toString()))
                    {
                        TipDlg(tr("提示"), tr("值错误：耗材报警值必须为正数且最多可设置整数位三位、小数位四位")).exec();
                        return false;
                    }
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
                    TipDlg(tr("提示"), tr("值错误：耗材报警值不能为0")).exec();
                    return false;
                }

                if (val > 500 * 10000)
                {
                    TipDlg(tr("提示"), tr("值错误：耗材最大容量500ml")).exec();
                    return false;
                }

                m_supplyAttributes.at(row).second.set_bottleAlarmThreshold(val);// 免疫使用的是bottlealarm数值
                break;
            }

		default:
			return false;
			break;
	}

	// 标识数据被改动过
	m_supplyAttributes.at(row).first = true;
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
Qt::ItemFlags QAlarmSupplySetModel::flags(const QModelIndex &index) const
{
	const int col = index.column();
	const int row = index.row();

	const Qt::ItemFlags default_flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if (col == static_cast<int>(COL::ASSAYWARNING))
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
void QAlarmSupplySetModel::Update()
{
    // 更新数据
    beginResetModel();

    m_supplyAttributes.clear();

    if (m_softType == CHEMISTRY || m_softType == CHEMISTRY_AND_IMMUNE)
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
            // 仓类耗材显示到试剂提示页面
            if ((supplyAttribute.type >= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MIN
                 && supplyAttribute.type <= ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MAX)
                || supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY
                || supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY
                || supplyAttribute.type == ch::tf::SuppliesType::SUPPLIES_TYPE_HITERGENT)
            {
                continue;
            }

            m_supplyAttributes.push_back(std::make_pair(false, SupplyAttribute(supplyAttribute)));
        };

        if (!CommonInformationManager::GetInstance()->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_ISE1005 }).empty())
        {
            // ISE
            ::ise::tf::SupplyAttributeQueryCond iseQueryCond;
            ::ise::tf::SupplyAttributeQueryResp iseRespGeneral;
            ise::LogicControlProxy::QuerySupplyAttribute(iseRespGeneral, iseQueryCond);
            if (iseRespGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_WARN, "Failed to query all ise supplyattribute.");
                return;
            }

            for (const auto& supplyAttribute : iseRespGeneral.lstSupplyAttributes)
            {
                m_supplyAttributes.push_back(std::make_pair(false, SupplyAttribute(supplyAttribute)));
            }
        }
    }

    if (m_softType == IMMUNE || m_softType == CHEMISTRY_AND_IMMUNE)
    {
        //免疫
        ::im::tf::SupplyAttributeQueryCond imQryCond;
        ::im::tf::SupplyAttributeQueryResp imQryRespGeneral;
        im::LogicControlProxy::QuerySupplyAttribute(imQryRespGeneral, imQryCond);
        if (imQryRespGeneral.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to query all im supplyattribute.");
            return;
        }

        for (const auto& supplyAttribute : imQryRespGeneral.lstSupplyAttributes)
        {
            //免疫耗材报警设置仅需要设置底物、清洗缓冲液、反应杯
            //底物液需要统一设定，因此统一使用底物液A
            if (supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A
                || supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID
                || supplyAttribute.type == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
            {
                m_supplyAttributes.push_back(std::make_pair(false, SupplyAttribute(supplyAttribute)));
            }
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
bool QAlarmSupplySetModel::SaveAlarmParameter(bool &hasChangeParam)
{
	// 依次保存报警参数
	for (auto& supplyAttribute : m_supplyAttributes)
	{
		// 若参数没有变动
		if (!supplyAttribute.first)
		{
			continue;
		}

		hasChangeParam = true;

		// 生化参数
		if (supplyAttribute.second.supplyAttributeCh.has_value())
		{
			// 生化参数
			auto& chData = supplyAttribute.second.supplyAttributeCh.value();
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
        else if (supplyAttribute.second.supplyAttributeIm.has_value())
        {
            // 免疫参数
            auto& ImData = supplyAttribute.second.supplyAttributeIm.value();
            im::tf::SupplyAttribute modifyParameter;
            // 设置参数id
            modifyParameter.__set_id(ImData.id);
            // 设置耗材类型
            modifyParameter.__set_type(ImData.type);
            // 瓶限制
            modifyParameter.__set_bottleAlarmThreshold(ImData.bottleAlarmThreshold);
            // 项目限制
            modifyParameter.__set_assayAlarmThreshold(ImData.assayAlarmThreshold);

            // 修改配置参数，若修改值为底物液A，同步修改底物液B
            im::LogicControlProxy::ModifySupplyAttribute(modifyParameter);
        }
		else
		{
			// ISE参数
			auto& iseData = supplyAttribute.second.supplyAttributeISE.value();
			ise::tf::SupplyAttribute modifyParameter;
			// 设置参数id
			modifyParameter.__set_id(iseData.id);
			// 瓶限制
			modifyParameter.__set_bottleAlarmThreshold(iseData.bottleAlarmThreshold);
			// 项目限制
			modifyParameter.__set_assayAlarmThreshold(iseData.assayAlarmThreshold);
			// 修改配置参数
			ise::LogicControlProxy::ModifySupplyAttribute(modifyParameter);
		}

        supplyAttribute.first = false;
	}

	return true;
}

int QAlarmSupplySetModel::rowCount(const QModelIndex &parent) const
{
	return m_supplyAttributes.size();
}

int QAlarmSupplySetModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant QAlarmSupplySetModel::data(const QModelIndex &index, int role) const
{
    // 对齐方式
    if (role == Qt::TextAlignmentRole)
    {
        return  Qt::AlignCenter;
    }

	const int row = index.row();
	const int column = index.column();

	if (role != Qt::DisplayRole && std::set<COL>({ COL::UNIT, COL::MODEL, COL::NAME }).count(COL(column)))
	{
		return QVariant();
	}

	// 越界控制
	if (row >= m_supplyAttributes.size())
	{
		return QVariant();
	}

	auto& data = m_supplyAttributes[row].second;

	// 获取数据
	if (role == Qt::DisplayRole || role == Qt::EditRole) {
		switch (COL(column))
		{
			case QAlarmSupplySetModel::COL::MODEL: 
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
                        return tr("ISE");
                    }
                }

			case QAlarmSupplySetModel::COL::NAME:
                {
                    if (data.supplyAttributeCh.has_value())
                    {
                        auto& chData = data.supplyAttributeCh.value();
                        return ThriftEnumTrans::GetSupplyName(chData.type, !chData.sampleProbe);
                    }
                    else if (data.supplyAttributeIm.has_value())
                    {
                        auto& imData = data.supplyAttributeIm.value();
                        return ThriftEnumTrans::GetImSupplyAlarmSettingName(imData.type);
                    }
                    else
                    {
                        auto& iseData = data.supplyAttributeISE.value();
                        return ThriftEnumTrans::GetSupplyName(iseData.type);
                    }
                }

			case QAlarmSupplySetModel::COL::ASSAYWARNING:
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
                        auto& iseData = data.supplyAttributeISE.value();
                        if (iseData.bottleAlarmThreshold == 0)
                        {
                            return "";
                        }
                        else
                        {
                            return QString::number(double(iseData.bottleAlarmThreshold) / 10000.0); // 0.1ul转为ml
                        }
                    }
                }

            case QAlarmSupplySetModel::COL::UNIT:
                {
                    if (data.supplyAttributeIm.has_value())
                    {
                        auto& imData = data.supplyAttributeIm.value();
                        if (imData.type == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
                        {
                            return tr("个");
                        }
                    }
                    return tr("ml");
                }

			default:
				return QVariant();
		}
	}

    return QVariant();
}

QVariant QAlarmSupplySetModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
    {
        return m_headerNames.at(section);
	}
	
    if (section == 2  && (role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        return QColor(UI_HEADER_FONT_COLOR);
    }

	return QAbstractTableModel::headerData(section, orientation, role);
}

bool QAlarmSupplySetModel::isExistChangedData()
{
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

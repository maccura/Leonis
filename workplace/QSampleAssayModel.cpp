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
/// @file     QSampleAssayModel.h
/// @brief    样本模块module
///
/// @author   5774/WuHongTao
/// @date     2022年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSampleAssayModel.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/ReagentCommon.h"

#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/common/StringUtil.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "manager/ResultStatusCodeManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"

#define  PAGE_LIMIT_ITEM                                (1000)      // 分页查询（每一页的样本数）
#define  INVALID_VALUE									(-1)
#define  TIMEINTERVAL                                   (1000)      // 定时器刷新时间

#define COL_DATAMODE_CHECK   0   // 勾选框列
#define COL_DATAMODE_STATUS  1   // 状态列

#define SMAPLE_COLUMN_NUMBER 11  // 样本表的列数


///
/// @bref
///		获取单位
///
QString GetTestItemUnit(const tf::TestItem& dataItem)
{
    QString strAssay = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(dataItem.assayCode);
    if (!QStringList({ "H", "L", "I" }).contains(strAssay))
    {
        return CommonInformationManager::GetInstance()->GetCurrentUnit(dataItem.assayCode);
    }

    return "";
}

QSampleAssayModel::QSampleAssayModel()
{
	m_moudleType = VIEWMOUDLE::ASSAYSELECT;
	m_currentRow = INVALID_VALUE;
	m_assaySelectHeader << tr("状态") << tr("样本号") << tr("样本条码");
    m_sampleFilter = 0;
	m_isDataChanged = false;
    m_assayBrowseHeader << " " << tr("状态");
    m_sampleBrowseHeader = m_assayBrowseHeader;
    // 按照项目浏览
	m_assayBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::ASSAYLIST);
    // 按照样本浏览
	m_sampleBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLE);
    // 数据更新
    connect(&DataPrivate::Instance(), &DataPrivate::sampleChanged, this, [&](DataPrivate::ACTIONTYPE mode, int start, int end)
    {
        switch (mode)
        {
			case DataPrivate::ACTIONTYPE::ADDSAMPLE :
            {
                auto indexstart = this->index(start, 0);
                beginInsertRows(indexstart.parent(), start, end);
                endInsertRows();
            }
            break;

			case DataPrivate::ACTIONTYPE::MODIFYSAMPLE:
            {
                auto indexstart = this->index(start, 0);
                auto indexend = this->index(end, this->columnCount() - 1);
				DoDataChanged(indexstart, indexend);
                //emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
            }
            break;

			case DataPrivate::ACTIONTYPE::DELETESAMPLE:
            {
                auto indexstart = this->index(start, 0, QModelIndex());
                if (!indexstart.isValid())
                {
                    this->beginResetModel();
                    this->endResetModel();
                    return;
                }

                beginRemoveRows(indexstart.parent(), start, end);
                endRemoveRows();
            }
            break;

            default:
                break;
        }
    });


    // 数据更新
    connect(&SampleColumn::Instance(), &SampleColumn::ColumnChanged, this, [&]()
    {
        m_assayBrowseHeader.clear();
        m_sampleBrowseHeader.clear();
        m_assayBrowseHeader << " "<< tr("状态");
        m_sampleBrowseHeader = m_assayBrowseHeader;
		// 按照项目浏览
		m_assayBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::ASSAYLIST);
		// 按照样本浏览
		m_sampleBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLE);
        this->beginResetModel();
        this->endResetModel();
    });
}

bool QSampleAssayModel::DoDataChanged(const QModelIndex& startIndex, const QModelIndex& endIndex)
{
	if (!m_isDataChanged)
	{
		m_isDataChanged = true;
		emit dataChanged(startIndex, endIndex, { Qt::DisplayRole });
		m_isDataChanged = false;
	}

	return true;
}

///
/// @brief 获取样本的显示数据(按项目显示)
///
/// @param[in]  index  位置
/// @param[in]  role   角色
///
/// @return 显示数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月3日，新建函数
///
QVariant QSampleAssayModel::DataAssay(const QModelIndex &index, int role) const
{
	const int row = index.row();
	const int column = index.column();

	// 获取对应行的数据
	auto dataItem = DataPrivate::Instance().GetTestItemByRow(row);
	// 未找到对应的样本数据
	if (dataItem == Q_NULLPTR)
	{
        return QVariant();
	}

    // 判断项目对应的样本是否存在
    auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(dataItem->sampleInfoId);
    if (!testItems)
    {
        return QVariant();
    }

    // 确定是否在测试中
    StartTestTimeCountDown(*dataItem);
    // 获取样本数据
    auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(dataItem->sampleInfoId);
    if (spSampleInfo == Q_NULLPTR)
    {
        return QVariant();
    }

	// 获取数据
	if (role == Qt::DisplayRole) 
    {
        return QDataItemInfo::Instance().GetAssayData(index, std::make_pair(spSampleInfo, *dataItem));
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) 
    {
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}
    // 额外数据
    else if (role == (Qt::UserRole + 1)) 
    {
        // 稀释结果
        auto columnValue = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::SAMPLEASSAY::ASSAY));
        if (columnValue && columnValue.value().first == column)
        {
            return QDataItemInfo::GetSuckType(*dataItem);
        }
    }

    return QVariant();
}

QSampleAssayModel::~QSampleAssayModel()
{
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
QSampleAssayModel& QSampleAssayModel::Instance()
{
	static QSampleAssayModel model;
	return model;
}

///
/// @brief 获取样本量和样本测试数目
///
///
/// @return 样本量和样本测试数
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月28日，新建函数
///
std::pair<int, int> QSampleAssayModel::GetSampleTestInfo()
{
    std::lock_guard<std::recursive_mutex> lockRead(DataPrivate::Instance().m_readWriteLock);
    DataPrivate::Instance().m_sampleCount = DataPrivate::Instance().m_samplePostionVec.size();
    return std::make_pair(DataPrivate::Instance().m_sampleCount, DataPrivate::Instance().m_testItemCount);
}

///
/// @brief 设置模式类型（项目选择，项目浏览-（按样本展示，按项目展示））
///
/// @param[in]  module  模式类型
///
/// @return 设置成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月21日，新建函数
///
bool QSampleAssayModel::SetSampleModuleType(VIEWMOUDLE module)
{
    ULOG(LOG_INFO, "%s( module:%d)", __FUNCTION__, module);

	// 从项目浏览到样本浏览页面
	if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE
		&& (module == VIEWMOUDLE::ASSAYSELECT || module == VIEWMOUDLE::DATABROWSE))
	{
		auto itemData = DataPrivate::Instance().GetTestItemByRow(m_currentRow);
		if (itemData == Q_NULLPTR)
		{
			m_currentRow = INVALID_VALUE;
		}
		else if(!DataPrivate::Instance().GetSampleByDb(itemData->sampleInfoId))
		{
			m_currentRow = INVALID_VALUE;
		}
		else
		{
			m_currentRow = DataPrivate::Instance().GetSampleByDb(itemData->sampleInfoId).value();
		}
	}

	// 样本浏览到项目浏览页面
	if ((m_moudleType == VIEWMOUDLE::ASSAYSELECT || m_moudleType == VIEWMOUDLE::DATABROWSE)
        && module == VIEWMOUDLE::ASSAYBROWSE)
	{
        do 
        {
            // 获取当前行的样本
            auto sample = DataPrivate::Instance().GetSampleByRow(m_currentRow);
            if (!sample)
            {
                m_currentRow = INVALID_VALUE;
                break;
            }

            // 获取样本对应的项目信息
            auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(sample.value().id);
            if (!testItems)
            {
                m_currentRow = INVALID_VALUE;
                break;
            }

            auto& itemVec = testItems.value();
            if (itemVec.empty())
            {
                m_currentRow = INVALID_VALUE;
                break;
            }

            // 对应的项目展示的第一个item（映射关系变化)
            auto testItemRow = DataPrivate::Instance().GetTestItemRowByDb(itemVec[0]);
            if (testItemRow.has_value())
            {
                m_currentRow = testItemRow.value();
            }

        } while (false);
	}

    DataPrivate::Instance().m_modeType = int(module);
	m_moudleType = module;

	beginResetModel();
	endResetModel();

	return true;
}

///
/// @brief 若项目正在运行，则开始推算项目测试完毕的倒计时
///
/// @param[in]  testItemInfo  项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月15日，新建函数
///
void QSampleAssayModel::StartTestTimeCountDown(const tf::TestItem & testItemInfo) const
{
	DataPrivate::Instance().StartTestTimeCountDown(testItemInfo);
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
Qt::ItemFlags QSampleAssayModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///
///  @brief 根据索引获取样本类型
///
///
///  @param[in]   index  索引
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月9日，新建函数
///
boost::optional<::tf::SampleType::type> QSampleAssayModel::GetSampleTypeByIndex(const QModelIndex index)
{
    //ULOG(LOG_INFO, "%s()", __FUNCTION__); // too frequency.

	if (!index.isValid())
	{
		return boost::none;
	}
	if (m_moudleType == VIEWMOUDLE::DATABROWSE)
	{
		auto optSample = DataPrivate::Instance().GetSampleByRow(index.row());
		if (!optSample.has_value())
		{
			return boost::none;
		}
		return boost::make_optional(optSample.value().sampleType);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYSELECT)
	{
		auto optSample = DataPrivate::Instance().GetSampleByRow(index.row());
		if (!optSample.has_value())
		{
			return boost::none;
		}
		return boost::make_optional(optSample.value().sampleType);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		auto optTestItem = DataPrivate::Instance().GetTestItemByRow(index.row());
		if (optTestItem == nullptr)
		{
			return boost::none;
		}
		int64_t sampleId = optTestItem->sampleInfoId;
		auto optSample = DataPrivate::Instance().GetSampleInfoByDb(sampleId);
		if (optSample == nullptr)
		{
			return boost::none;
		}
		return boost::make_optional(optSample->sampleType);
	}
	return boost::none;
}

///
///  @brief 根据索引获取主键
///
///
///  @param[in]   index  索引
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月9日，新建函数
///
boost::optional<int64_t> QSampleAssayModel::GetIdByIndex(const QModelIndex index)
{
	if (!index.isValid())
	{
		return boost::none;
	}
	if (m_moudleType == VIEWMOUDLE::DATABROWSE)
	{
		auto optSample = DataPrivate::Instance().GetSampleByRow(index.row());
		if (!optSample.has_value())
		{
			return boost::none;
		}
		return boost::make_optional(optSample.value().id);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYSELECT)
	{
		auto optSample = DataPrivate::Instance().GetSampleByRow(index.row());
		if (!optSample.has_value())
		{
			return boost::none;
		}
		return boost::make_optional(optSample.value().id);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		auto optTestItem = DataPrivate::Instance().GetTestItemByRow(index.row());
		if (optTestItem == nullptr)
		{
			return boost::none;
		}
		return boost::make_optional(optTestItem->id);
	}
	return boost::none;
}

///
///  @brief 按样本显示
///
///
///  @param[in]   mode   模式
///  @param[in]   index  索引
///  @param[in]   role   角色
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
///
QVariant QSampleAssayModel::SampleShow(const int mode, const QModelIndex & index, int role) const
{
    auto funcIsStatuCol = [](int mode, int column)->bool {
        return (mode == 1 && column == 1) || (mode != 1 && column == 0);
    };

    // 代表checkBox的数据
    int column = index.column();
    int statusColumn = (mode == 1) ? 1 : 0;

	// 获取数据
	if (role == Qt::DisplayRole) {

        if (column <= statusColumn)
        {
            return QVariant();
        }

		SampleColumn::SAMPLEMODE sampleMode = ((mode == 0) ? SampleColumn::SAMPLEMODE::SAMPLEAPP : SampleColumn::SAMPLEMODE::SAMPLE);
		return DataPrivate::Instance().GetSampleDisplayData(sampleMode, index.row(), column);
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) {
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}
    // 额外数据
    else if (role == (Qt::UserRole + 1)) {
        // 特殊处理第一列
        if ((mode == 1) && column == 0)
        {
            return DataPrivate::Instance().GetSelectFlag(0, index.row());
        }

        if (!funcIsStatuCol(mode, column))
        {
            return QVariant();
        }

        auto sampleInfo = DataPrivate::Instance().GetSampleByRow(index.row());
        if (!sampleInfo)
        {
            return QVariant();
        }

        tf::SampleInfo& sample = sampleInfo.value();
        return WorkpageCommon::GetTestStatusPixmap(sample.__isset.status ? sample.status : -1);
    }
    else if (role == (Qt::UserRole + 2)) {
        if (!funcIsStatuCol(mode, column))
        {
            return QVariant();
        }

        auto sampleInfo = DataPrivate::Instance().GetSampleByRow(index.row());
        if (!sampleInfo)
        {
            return QVariant();
        }

        const auto& vecTestItems = DataPrivate::Instance().GetSampleTestItems(sampleInfo.value().id);
        return WorkpageCommon::GetItemReCheckPixmap(vecTestItems);
    }
    else if (role == (Qt::UserRole + 3)) {
        if (!funcIsStatuCol(mode, column))
        {
            return QVariant();
        }

        auto sampleInfo = DataPrivate::Instance().GetSampleByRow(index.row());
        if (!sampleInfo)
        {
            return QVariant();
        }

        return WorkpageCommon::GetUploadPixmap(sampleInfo.value(), QDataItemInfo::Instance().IsAiRecognition());
    }
	else if (role == (Qt::UserRole + 4)) {
		if (!funcIsStatuCol(mode, column))
		{
			return QVariant();
		}

		auto sampleInfo = DataPrivate::Instance().GetSampleByRow(index.row());
		if (!sampleInfo)
		{
			return QVariant();
		}

		return WorkpageCommon::GetSampleAiStatus(sampleInfo.value());
	}

	return QVariant();
}

///
///  @brief 按项目显示
///
///
///  @param[in]   mode   模式
///  @param[in]   index  索引
///  @param[in]   role   角色
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
///
QVariant QSampleAssayModel::ItemShow(const int mode, const QModelIndex & index, int role) const
{
    // 代表checkBox的数据
    int column = index.column();
	auto testItem = *DataPrivate::Instance().GetTestItemByRow(index.row());
	// 确定是否在测试中
	StartTestTimeCountDown(testItem);
	// 获取数据
	if (role == Qt::DisplayRole) {
        if (column <= 1)
        {
            return QVariant();
        }

		auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(testItem.sampleInfoId);
		if (spSampleInfo == Q_NULLPTR)
		{
			return QVariant();
		}

		int col = static_cast<int> (SampleColumn::Instance().IndexToStatus(index.column()));
		 return DataPrivate::Instance().GetItemData(col, { spSampleInfo, testItem });
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) {
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	// 额外数据
	else if (role == (Qt::UserRole + 1)) 
    {
        // 特殊处理第一列
        if (column == COL_DATAMODE_CHECK)
        {
            return DataPrivate::Instance().GetSelectFlag(1, index.row());
        }

		// 稀释结果（modify bug3353 by wuht）
		auto columnValue = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::ASSAY));
		if (columnValue && columnValue.value().first == column)
		{
			// 是否计算项目(计算项目不显示稀释倍数)
			bool isCalc = CommonInformationManager::IsCalcAssay(testItem.assayCode);
			if (isCalc)
			{
				return QVariant();
			}

			return QDataItemInfo::GetDiluTionShow(testItem);
		}
		else if(column == COL_DATAMODE_STATUS)
		{
			// modify bug0010776 by wuht 
			return WorkpageCommon::GetTestStatusPixmap(testItem.__isset.status ? testItem.status : -1);
		}

		// 若显示吸光度则没有背景色
		auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(testItem.sampleInfoId);
		if (spSampleInfo == Q_NULLPTR)
		{
			return QVariant();
		}

		const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (!WorkpageCommon::IsShowConc(sampleSet, spSampleInfo->sampleType))
		{
			return QVariant();
		}

		// 检测结果和复查检测结果的值的状态
		int type = QDataItemInfo::Instance().ResultRangeStatus(testItem, column, false).toInt();
		// 若没有异常，需要再次判断
		if (ColorType::NORMALCOLOR == type)
		{
			auto status = QDataItemInfo::Instance().ResultDangerStatus(testItem, column, false);
			if (!status.has_value())
			{
				return type;
			}

			if (status.value())
			{
				return ColorType::UPPERCOLOR;
			}
			else
			{
				return ColorType::LOWERCOLOR;
			}
		}

		return type;
	}
    else if (role == (Qt::UserRole + 2)) 
    {
        if (column == COL_DATAMODE_STATUS)
        {
			// modify bug0010776 by wuht 
			return WorkpageCommon::GetItemReCheckPixmap(testItem);
        }
        else
        {
            return QVariant();
        }

    }
    else if (role == (Qt::UserRole + 3)) 
    {
        if (column == COL_DATAMODE_STATUS)
        {
			auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(testItem.sampleInfoId);
			if (spSampleInfo == Q_NULLPTR)
			{
				return QVariant();
			}
			// modify bug0011463 by wuht
			return WorkpageCommon::GetUploadPixmap(*spSampleInfo, QDataItemInfo::Instance().IsAiRecognition());
        }
        else
        {
            return QVariant();
        }

    }
	else if (role == (Qt::UserRole + 4)) 
	{
		if (column == COL_DATAMODE_STATUS)
		{
			auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(testItem.sampleInfoId);
			if (spSampleInfo == Q_NULLPTR)
			{
				return QVariant();
			}

			tf::SampleInfo sample;
			return WorkpageCommon::GetSampleAiStatus(*spSampleInfo);
		}
		else
		{
			return QVariant();
		}
	}
	else if (role == (Qt::UserRole + 5))
	{
		
#if 0 // 应需求和工业设计需求去掉数据报警的背景色
		auto AlarmBackGroudColor = [&](tf::TestItem& testItem, bool isRecheck)->QVariant
		{
			auto resultStatusCode = ResultStatusCodeManager::GetInstance()->GetChemistryDisplayResCode(testItem, isRecheck);
			if (!resultStatusCode.isEmpty())
			{
				return QVariant("#FFFF99");
			}

			return QVariant();
		};

		// 数据告警
		auto columnValue = QDataItemInfo::Instance().GetAttributeByType(QDataItemInfo::SAMPLEMODE::ASSAYLIST, int(QDataItemInfo::COLASSAY::RESULTSTATUS));
		// 复查数据告警
		auto lastColumnValue = QDataItemInfo::Instance().GetAttributeByType(QDataItemInfo::SAMPLEMODE::ASSAYLIST, int(QDataItemInfo::COLASSAY::RECHECKSTATUS));
		// 初测
		if (columnValue && columnValue.value().first == index.column())
		{
			return AlarmBackGroudColor(testItem, false);
		}
		// 复查
		else if (lastColumnValue && lastColumnValue.value().first == index.column())
		{
			return AlarmBackGroudColor(testItem, true);
		}
#endif

		// 若显示吸光度则没有背景色
		auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(testItem.sampleInfoId);
		if (spSampleInfo == Q_NULLPTR)
		{
			return QVariant();
		}

		const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (!WorkpageCommon::IsShowConc(sampleSet, spSampleInfo->sampleType))
		{
			return QVariant();
		}

		//modify bug0012239  by wuht
		auto status = QDataItemInfo::Instance().ResultDangerStatus(testItem, column, false);
		if (!status.has_value())
		{
			return QVariant();
		}

		if (status.value())
		{
			return QColor("#FFDAD6");
		}
		else
		{
			return QColor("#E5F2FF");
		}
	}

	return QVariant();
}

int QSampleAssayModel::rowCount(const QModelIndex &parent) const
{
	std::lock_guard<std::recursive_mutex> lockRead(DataPrivate::Instance().m_readWriteLock);
	if (m_moudleType == VIEWMOUDLE::DATABROWSE || m_moudleType == VIEWMOUDLE::ASSAYSELECT)
	{
		return DataPrivate::Instance().m_sampInfoData.size();
	}
	else if(m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		return DataPrivate::Instance().m_testItemData.size();
	}
	else
	{
		return 0;
	}
}

int QSampleAssayModel::columnCount(const QModelIndex &parent) const
{
	switch (m_moudleType)
	{
		case QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT:
			return m_assaySelectHeader.size();
		case QSampleAssayModel::VIEWMOUDLE::DATABROWSE:
			return m_sampleBrowseHeader.size();
		case QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
			return m_assayBrowseHeader.size();
		default:
            break;
    }

    return m_assaySelectHeader.size();
}

QVariant QSampleAssayModel::data(const QModelIndex &index, int role) const
{
    switch (m_moudleType)
    {
    case QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT:
        return SampleShow(0, index, role);
    case QSampleAssayModel::VIEWMOUDLE::DATABROWSE:
        return SampleShow(1, index, role);
    case QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
        return ItemShow(0, index, role);
    default:
        break;
    }

	return QVariant();
}

bool QSampleAssayModel::ClearSelectedFlag(int mode)
{
    ULOG(LOG_INFO, "%s(mode: %d)", __FUNCTION__, mode);
    std::vector<int> updateRows;
    auto keyMap = DataPrivate::Instance().GetAllSelectFlag(mode);
    // 按样本展示
    if (0 == mode)
    {
        for (auto key : keyMap)
        {
            auto rowOption = DataPrivate::Instance().GetSampleByDb(key);
            if (rowOption)
            {
                updateRows.push_back(rowOption.value());
            }
        }
    }
    // 按项目展示
    else
    {
        for (auto key : keyMap)
        {
            auto rowOption = DataPrivate::Instance().GetTestItemRowByDb(key);
            if (rowOption)
            {
                updateRows.push_back(rowOption.value());
            }
        }
    }

	QStringList rows;
    DataPrivate::Instance().ClearSelected(mode);
    for (auto& row : updateRows)
    {
        auto indexstart = this->index(row, 0);
        auto indexend = this->index(row, 0);
		rows.push_back(QString::number(row));
		DoDataChanged(indexstart, indexend);
        //emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
    }

	ULOG(LOG_INFO, "%s(clear rows: %s )", __FUNCTION__, rows.join("|").toStdString());

	// 状态发生变化
	emit selectItemChanged();
    return true;
}

bool QSampleAssayModel::SetAllSelectedFlag(int mode)
{
    ULOG(LOG_INFO, "%s(mode:%d)", __FUNCTION__, mode);
    if (DataPrivate::Instance().SetAllSelectedFlag(mode))
    {
        if (rowCount() < 15)
        {
            auto rangLast = ((rowCount() - 1) < 0) ? 0 : (rowCount() - 1);
            DataPrivate::Instance().SetRange(0, rangLast);
        }

        auto range  = DataPrivate::Instance().m_rangVisble.GetRange();
        auto indexstart = this->index(range.first, 0, QModelIndex());
        auto indexend = this->index(range.second, 0, QModelIndex());
		DoDataChanged(indexstart, indexend);
    }

	// 状态发生变化
	emit selectItemChanged();
    return true;
}

bool QSampleAssayModel::SetSelectFlag(int mode, int row, bool flag, bool isfromAll)
{
    ULOG(LOG_INFO, "%s(row: %d)", __FUNCTION__, row);
    // 设置成功则刷新
    if (DataPrivate::Instance().SetSelectFlag(mode, row, flag))
    {
        auto indexstart = this->index(row, 0);
        auto indexend = this->index(row, 0);
		if (isfromAll)
		{
			DoDataChanged(indexstart, indexend);
		}
		// 状态发生变化
		emit selectItemChanged();
    }

    return true;
}

QVariant QSampleAssayModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section < 0)
    {
        return QVariant();
    }

	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
            switch (m_moudleType)
            {
                case QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT:
                    return m_assaySelectHeader.at(section);
                    break;
                case QSampleAssayModel::VIEWMOUDLE::DATABROWSE:
                    return m_sampleBrowseHeader.at(section);
                    break;
                case QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
                    return m_assayBrowseHeader.at(section);
                    break;
                default:
                    return m_assaySelectHeader.at(section);
                    break;
            }
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QDataItemInfo& QDataItemInfo::Instance()
{
    static QDataItemInfo dataItemInfo;
    return dataItemInfo;
}

QDataItemInfo::QDataItemInfo()
{
    // 初始化
    Init();
}

template <typename T>
boost::optional<std::pair<int, int>> GetAttributePair(const QMap<int, T>& srcData, int iType)
{
    for (const auto& item : srcData)
    {
        if (iType == int(item.type))
        {
            return std::make_pair(item.index, item.width);
        }
    }

    return boost::none;
}

///
/// @brief 获取当前行对应的数据（样本）
///
/// @param[in]  index       位置信息
/// @param[in]  sampleInfo  原始数据
///
/// @return 返回数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月16日，新建函数
///
QVariant QDataItemInfo::GetSampleData(SampleColumn::SAMPLEMODE mode, const QModelIndex &index, const tf::SampleInfo& sampleInfo)
{
    switch (SampleColumn::Instance().IndexSampleToStatus(mode, index.column()))
    {
        case SampleColumn::COL::STATUS:
            return GetStatus(sampleInfo);
        case SampleColumn::COL::SEQNO:
            return GetSeqNo(sampleInfo);
        case SampleColumn::COL::BARCODE:
            return QString::fromStdString(sampleInfo.barcode);
        case SampleColumn::COL::TYPE:
            return ThriftEnumTrans::GetSourceTypeName(sampleInfo.sampleSourceType);
        case SampleColumn::COL::POS:
            return GetPos(sampleInfo);
        case SampleColumn::COL::ENDTIME:
            return(sampleInfo.__isset.endTestTime ? QString::fromStdString(sampleInfo.endTestTime) : "-");
        case SampleColumn::COL::RECHECKENDTIME:
            return(sampleInfo.__isset.endRetestTime ? QString::fromStdString(sampleInfo.endRetestTime) : "-");
        default:
            break;
    }

    return QVariant();
}

QVariant QDataItemInfo::GetDisplyResCodeVariant(const tf::TestItem& dataItem, const std::string& strResult, bool isReTest, int sampType)
{
    std::string status = isReTest ? \
        (dataItem.__isset.retestResultStatusCodes ? dataItem.retestResultStatusCodes : "") : \
        (dataItem.__isset.resultStatusCodes ? dataItem.resultStatusCodes : "");

    int refJudge = ::tf::RangeJudge::NORMAL;
    int criJudge = ::tf::RangeJudge::NORMAL;
    if (isReTest)
    {
        if (dataItem.__isset.retestRefRangeJudge)
        {
            refJudge = dataItem.retestRefRangeJudge;
        }
        if (dataItem.__isset.retestCrisisRangeJudge)
        {
            criJudge = dataItem.retestCrisisRangeJudge;
        }
    }
    else
    {
        if (dataItem.__isset.firstRefRangeJudge)
        {
            refJudge = dataItem.firstRefRangeJudge;
        }
        if (dataItem.__isset.firstCrisisRangeJudge)
        {
            criJudge = dataItem.firstCrisisRangeJudge;
        }
    }
    std::string orginput = isReTest ? dataItem.retestResultStatusCodes : dataItem.resultStatusCodes;
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    auto spAssayInfo = assayMgr->GetAssayInfo(dataItem.assayCode);
	if (Q_NULLPTR == spAssayInfo)
	{
		return ResultStatusCodeManager::GetInstance()->GetChemistryDisplayResCode(dataItem, isReTest);
	}

    if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        ResultStatusCodeManager::GetInstance()->GetDisplayResCode(orginput, status, strResult, refJudge, criJudge, sampType);
    }
	// 生化，或者ISE等
	// modify bug0011922 by wuht
	else
	{
		return ResultStatusCodeManager::GetInstance()->GetChemistryDisplayResCode(dataItem, isReTest);
	}

    return QString::fromStdString(status);
}

QString QDataItemInfo::GetSINDStatusInfo(const int assayCode, const tf::QualJudge::type judgeType)
{
	int start = judgeType - tf::QualJudge::CH_SIND_LEVEL1;
	auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(assayCode, start);
	return status;
}

///
/// @brief 获取项目信息
///
/// @param[in]  index           位置信息
/// @param[in]  testItemInfo    项目信息
///
/// @return 项目信息（对应的）
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月24日，新建函数
///
QVariant QDataItemInfo::GetOnlyAssay(const QModelIndex &index, const DataPara& dataPara)
{
    const auto& data = std::get<0>(dataPara);
    const auto& dataItem = std::get<1>(dataPara);

    const int row = index.row();
    const int column = index.column();
    switch (SampleColumn::Instance().IndexAssayToStatus(column))
    {
		case SampleColumn::SAMPLEASSAY::RECHECKOPTION:
            return "";
		case SampleColumn::SAMPLEASSAY::ASSAY:
            return CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(dataItem.assayCode);
		case SampleColumn::SAMPLEASSAY::RESULT:
            return GetAssayResultWithCountdown(false, dataItem, data);
		case SampleColumn::SAMPLEASSAY::RESULTSTATUS:
            return GetDisplyResCodeVariant(dataItem, GetAssayResultWithCountdown(false, dataItem, data).toStdString(), false, data->sampleType);
		case SampleColumn::SAMPLEASSAY::RECHECKRESULT:
            return GetAssayResultWithCountdown(true, dataItem, data);
		case SampleColumn::SAMPLEASSAY::RECHECKSTATUS:
            return GetDisplyResCodeVariant(dataItem, GetAssayResultWithCountdown(true, dataItem, data).toStdString(), true, data->sampleType);
		case SampleColumn::SAMPLEASSAY::UNIT:
        {
			// 样本显示的设置
			const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
			// 显示非浓度的情况下，不需要显示单位
			if (!WorkpageCommon::IsShowConc(sampleSet, data->sampleType))
			{
				return "";
			}

            return GetTestItemUnit(dataItem);
        }
		case SampleColumn::SAMPLEASSAY::MODULE:
            return QString::fromStdString(dataItem.__isset.deviceSN ? CommonInformationManager::GetDeviceName(dataItem.deviceSN, dataItem.moduleIndex) : "");
		case SampleColumn::SAMPLEASSAY::FIRSTABSORB:
			return QString::fromStdString(dataItem.__isset.RLU ? dataItem.RLU : "");
		case SampleColumn::SAMPLEASSAY::REABSORB:
			return QString::fromStdString(dataItem.__isset.retestRLU ? dataItem.retestRLU : "");
        default:
            return QVariant();
    }
}

QString QDataItemInfo::GetAssayResultWithCountdown(bool reCheck, const tf::TestItem& testItem, const std::shared_ptr<tf::SampleInfo>& pSample)
{
    QString result = GetAssayResult(reCheck, testItem, pSample);
    if (IsShowCountDown(reCheck, testItem))
    {
        auto runtimes = DataPrivate::Instance().GetRuntimesFromId(testItem.id);
        if (runtimes)
        {
            if (runtimes.value() > 0)
            {
                result = ConvertTime(runtimes.value());
            }
        }
    }

    return result;
}

bool QDataItemInfo::IsShowCountDown(const bool reCheck, const tf::TestItem& testItem)
{
	// 初测（设置了初测标志，但是初测浓度和复查浓度都没有设置）
	if (!reCheck && !testItem.__isset.conc && !testItem.__isset.retestConc)
	{
		return true;
	}
	// 初测（设置了初测标志，设置了初测浓度，但是初测结果无法计算）
	if (!reCheck && testItem.__isset.conc && testItem.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
	{
		return true;
    }
    // 初测，浓度值<0 被认为是中途中断或测试错误
    if (!reCheck && testItem.__isset.conc && testItem.conc < 0)
    {
        return true;
    }

	// 复查(当设置了复查，同时初测数据被设置且合法，同时初测结果不是无法计算)
	if (reCheck && testItem.__isset.conc && testItem.conc >= 0 && 
        testItem.resultStatusCodes.find(u8"Calc.?") == std::string::npos)
	{
		return true;
	}

	return false;
}

///
/// @brief 获取当前行对应的数据
///
/// @param[in]  index  位置信息
///
/// @return 返回数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月7日，新建函数
///
QVariant QDataItemInfo::GetAssayData(const QModelIndex &index, const DataPara& dataPara)
{
    // 获取样本数据
    auto& data = std::get<0>(dataPara);
    if (data == Q_NULLPTR)
    {
        return QVariant();
    }
    auto& dataItem = std::get<1>(dataPara);

    const int row = index.row();
    const int column = index.column();
    switch (SampleColumn::Instance().IndexToStatus(column))
    {
		case SampleColumn::COLASSAY::STATUS:
            return GetStatus(dataItem);
		case SampleColumn::COLASSAY::SEQNO:
            return GetSeqNo(*data);
		case SampleColumn::COLASSAY::BARCODE:
            return QString::fromStdString((*data).barcode);
		case SampleColumn::COLASSAY::TYPE:
            return ThriftEnumTrans::GetSourceTypeName((*data).sampleSourceType);
		case SampleColumn::COLASSAY::POS:
            return GetPos((*data));
		case SampleColumn::COLASSAY::ENDTIME:
        {
            auto endtime = dataItem.__isset.endTime ? dataItem.endTime : "-";
            return QString::fromStdString(endtime);
        }
		case SampleColumn::COLASSAY::RECHECKENDTIME:
        {
            auto reEndTime = dataItem.__isset.retestEndTime ? dataItem.retestEndTime : "-";
            return QString::fromStdString(reEndTime);
        }
		case SampleColumn::COLASSAY::ASSAY:
            return CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(dataItem.assayCode);
		case SampleColumn::COLASSAY::RESULT:
            return GetAssayResultWithCountdown(false, dataItem, data);
		case SampleColumn::COLASSAY::RESULTSTATUS:
        {
            auto status = dataItem.__isset.resultStatusCodes ? dataItem.resultStatusCodes : "";
            return QString::fromStdString(status);
        }
        case SampleColumn::COLASSAY::RECHECKRESULT:
            return GetAssayResultWithCountdown(true, dataItem, data);
        case SampleColumn::COLASSAY::RECHECKSTATUS:
        {
            auto status = dataItem.__isset.retestResultStatusCodes ? dataItem.retestResultStatusCodes : "";
            return QString::fromStdString(status);
        }
        case SampleColumn::COLASSAY::UNIT:
		{
			// 样本显示的设置
			const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
			// 显示非浓度的情况下，不需要显示单位
			if (!WorkpageCommon::IsShowConc(sampleSet, data->sampleType))
			{
				return "";
			}

			return GetTestItemUnit(dataItem);
		}
        case SampleColumn::COLASSAY::MODULE:
            return QString::fromStdString(dataItem.__isset.deviceSN ? CommonInformationManager::GetDeviceName(dataItem.deviceSN, dataItem.moduleIndex) : "");
        default:
            return QVariant();
    }
}

///
/// @brief 初始化列表映射选项
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月7日，新建函数
///
void QDataItemInfo::Init()
{
    QStringList assaySelectHeader;
    QStringList sampleBrowseHeader;
    QStringList assayBrowseHeader;
     assaySelectHeader << tr("状态") << tr("样本号") << tr("样本条码");
    (sampleBrowseHeader += assaySelectHeader) << tr("样本类型") << tr("架/位置") << tr("检测完成时间") << tr("复查完成时间") << tr("审核");
    (assayBrowseHeader += assaySelectHeader) << tr("样本类型") << tr("架/位置") << tr("检测完成时间") << tr("复查完成时间") << tr("项目") << tr("检测结果") << tr("结果状态") << tr("复查结果")
        << tr("结果状态") << tr("单位") << tr("审核") << tr("模块");

    if (!DictionaryQueryManager::GetPageset(m_workSet))
    {
        ULOG(LOG_ERROR, "Failed to get workset config.");
        return;
    }
   
    REGISTER_HANDLER(MSG_ID_SAMPLE_SHOWSET_UPDATE, this, UpdateResultShow);
}

bool QDataItemInfo::IsFirstTested(const tf::TestItem& testItem)
{
	if (!testItem.__isset.conc || testItem.conc < 0)
	{
		return false;
	}

	if (testItem.resultStatusCodes.find("calc") != std::string::npos)
	{
		return false;
	}

	return true;
}

void QDataItemInfo::UpdateResultShow()
{
    m_sampleResultShowSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
}

///
/// @brief 获取样本的测试状态
///
/// @param[in]  sample  样本信息
///
/// @return 样本状态字符串（O,P,H）
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月21日，新建函数
///
QString QDataItemInfo::GetStatus(const tf::SampleInfo& sample)
{
    // 获取样本测试状态
    QString testStatus("");

    if (sample.__isset.uploaded && sample.uploaded)
    {
        // 样本已上传
        testStatus += "H";
    }

    if (sample.__isset.status)
    {
        if (sample.status == ::tf::SampleStatus::SAMPLE_STATUS_PENDING)
        {
            testStatus += "O";
        }
        else if (sample.status == ::tf::SampleStatus::SAMPLE_STATUS_TESTING)
        {
            testStatus += "P";
        }
    }

    return testStatus;
}

///
/// @brief 获取样本的测试状态
///
/// @param[in]  sample  样本信息
///
/// @return 样本状态字符串（O,P,H）
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月21日，新建函数
///
QString QDataItemInfo::GetStatus(const tf::TestItem& item)
{
    // 获取项目测试状态
    QString testStatus("");

    if (item.__isset.status)
    {
        if (item.status == ::tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING)
        {
            testStatus += "O";
        }
        else if (item.status == ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTING)
        {
            testStatus += "P";
        }
    }

    return testStatus;
}

///
/// @brief 获取样本号
///
/// @param[in]  sample  样本信息
///
/// @return 样本号
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月22日，新建函数
///
QString QDataItemInfo::GetSeqNo(const tf::SampleInfo& sample)
{
	// 若样本在条码模式下，序号只是作为备注，所以不需要加前缀，返回样本的原始信息即可
	if (sample.__isset.testMode && sample.testMode == tf::TestMode::BARCODE_MODE && sample.sampleType == ::tf::SampleType::SAMPLE_TYPE_PATIENT)
	{
		return QString::fromStdString(sample.seqNo);
	}

    // 构造样本类别
    QString strCategory("");
    if (sample.sampleType == ::tf::SampleType::SAMPLE_TYPE_PATIENT)
    {
        strCategory = sample.stat ? "E" : "N";
    }
    else if (sample.sampleType == ::tf::SampleType::SAMPLE_TYPE_QC)
    {
        return("QC" + QString::fromStdString(sample.seqNo));
		// 20240712需求要求校准和质控均范围原始序号，不需要增加前缀
		// bug0013769要求增加前缀，故改回去
		//return QString::fromStdString(sample.seqNo);
    }
    // 校准只需要直接返回
    else if (sample.sampleType == ::tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
    {
        return QString::fromStdString(sample.seqNo);
    }

    // 构造样本序号，只有常规样本才有序号
    QString strSeqNo("");
    if (sample.__isset.seqNo)
    {
        strSeqNo = QString::fromStdString(UiCommon::CombinSeqName(strCategory.toStdString(), sample.seqNo));
    }

    return strSeqNo;
}

///
/// @brief 获取位置
///
/// @param[in]  sample  样本信息
///
/// @return 位置信息（x-x）
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月21日，新建函数
///
QString QDataItemInfo::GetPos(const tf::SampleInfo& sample)
{
    QString strPos("");
    if (sample.__isset.rack)
    {
        strPos += QString::fromStdString(sample.rack);
    }
    strPos += "-";
    if (sample.__isset.pos)
    {
        strPos += QString::number(sample.pos);
    }

    return strPos;
}

///
/// @brief 获取测试结束时间
///
/// @param[in]  assayAnyResult  测试结果
///
/// @return 结束时间
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月24日，新建函数
///
QString QDataItemInfo::GetAssayEndTime(const AssayResult& assayAnyResult)
{
    if (!assayAnyResult.has_value())
    {
        return "";
    }

    if (typeid(ch::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ch::tf::AssayTestResult>(assayAnyResult.value());
        return(assayResult.__isset.endTime ? QString::fromStdString(assayResult.endTime) : "-");
    }
    // 免疫
    else if (typeid(im::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(assayAnyResult.value());
        return(assayResult.__isset.endTime ? QString::fromStdString(assayResult.endTime) : "-");
    }
    // ISE
    else if (typeid(ise::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ise::tf::AssayTestResult>(assayAnyResult.value());
        return(assayResult.__isset.endTime ? QString::fromStdString(assayResult.endTime) : "-");
    }
    // 计算项目
    else if (typeid(tf::CalcAssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<tf::CalcAssayTestResult>(assayAnyResult.value());
        return(assayResult.__isset.dateTime ? QString::fromStdString(assayResult.dateTime) : "-");
    }


    return "";
}

///
/// @brief 获取项目测试结果显示字符串
///
/// @param[in]  assayAnyResult  项目结果
///
/// @return 结果字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月3日，新建函数
///
QString QDataItemInfo::GetAssayResult(const AssayResult& assayAnyResult)
{
    if (!assayAnyResult.has_value())
    {
        return "";
    }

    QString conc;
    QString referenceJudge;
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = Q_NULLPTR;
    if (typeid(ch::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ch::tf::AssayTestResult>(assayAnyResult.value());
        // 当有编辑结果的时候使用编辑结果
        conc = (assayResult.__isset.concEdit && !assayResult.concEdit.empty()) ? QString::fromStdString(assayResult.concEdit) :
            (assayResult.__isset.conc? QString::number(assayResult.conc):"");

        if (assayResult.__isset.qualitativeJudge)
        {
            return QString::number(assayResult.qualitativeJudge);
        }

        // 获取项目的配置信息
        spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayResult.assayCode);
    }
    // 免疫
    else if (typeid(im::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(assayAnyResult.value());
		conc = CommonInformationManager::GetInstance()->GetImDisplyResultStr(assayResult.assayCode, assayResult);
		return conc;
    }
    // ISE
    else if (typeid(ise::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ise::tf::AssayTestResult>(assayAnyResult.value());
        // 当有编辑结果的时候使用编辑结果
        conc = (assayResult.__isset.concEdit && !assayResult.concEdit.empty()) ? QString::fromStdString(assayResult.concEdit) :
            (assayResult.__isset.conc ? QString::number(assayResult.conc) : "");
        
        if (assayResult.__isset.qualitativeJudge && !assayResult.qualitativeJudge.empty())
        {
            return QString::fromStdString(assayResult.qualitativeJudge);
        }
        
        // 获取项目的配置信息
        spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayResult.assayCode);
    }
    // 计算项目
    else if (typeid(tf::CalcAssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<tf::CalcAssayTestResult>(assayAnyResult.value());
        // 当有编辑结果的时候使用编辑结果
        conc = (assayResult.__isset.editResult && !assayResult.editResult.empty()) ? QString::fromStdString(assayResult.editResult) :
            QString::fromStdString(assayResult.result);

        auto pCalcAssay = CommonInformationManager::GetInstance()->GetCalcAssayInfo((int)assayResult.assayCode);
        if (pCalcAssay != nullptr)
        {
            return QString::number(conc.toDouble(), 'f', pCalcAssay->decimalDigit);
        }

        return (conc + referenceJudge);
    }

    if (spAssayInfo == Q_NULLPTR)
    {
        return (conc + referenceJudge);
    }

    for (const tf::AssayUnit& unit : spAssayInfo->units)
    {
        // 当前使用的单位
        if (unit.isCurrent)
        {
            if (conc.isEmpty())
            {
                return conc;
            }

            return (QString::number(conc.toDouble() * unit.factor) + referenceJudge);
        }
    }

    return (conc + referenceJudge);
}

///
/// @brief 根据结果和设置显示结果范围
///
/// @param[in]  reCheck    true:复查，false:初测
/// @param[in]  testItem   项目信息
/// @param[in]  resultProm 结果提示
///
/// @return 超出上限显示：↑和H，超出下限显示↓和L
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月12日，新建函数
///
QString QDataItemInfo::ResultRangeFlag(bool reCheck, const tf::TestItem& testItem, const ResultPrompt& resultProm)
{
	QString refFlag;
	QStringList refFlags;
	if (resultProm.enabledOutRange)
	{
		if (resultProm.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	QString lowerdangeFlag;
	QString upperdangeFlag;
	if (resultProm.enabledCritical)
	{
		// modify bug0011817 by wuht
		if (resultProm.outRangeMarkType == 1)
		{
			lowerdangeFlag = "↓!";
			upperdangeFlag = "↑!";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			lowerdangeFlag = "L!";
			upperdangeFlag = "H!";
		}
	}

	if (resultProm.enabledOutRange)
	{
		// 初测--初测参考范围标志
		if (!reCheck && testItem.__isset.firstRefRangeJudge && !refFlags.empty())
		{
			if (testItem.firstRefRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				refFlag = refFlags[0];
			}
			else if (testItem.firstRefRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				refFlag = refFlags[1];
			}
		}
		// 复查--复查参考范围标志
		else if (reCheck && testItem.__isset.retestRefRangeJudge && !refFlags.empty())
		{
			if (testItem.retestRefRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				refFlag = refFlags[0];
			}
			else if (testItem.retestRefRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				refFlag = refFlags[1];
			}
		}
	}

	// modify for bug0011251 by wuht-5774
	if (resultProm.enabledCritical)
	{
		if (!reCheck)
		{
			// 初测--初测危急范围标志
			if (testItem.firstCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				refFlag = upperdangeFlag;
			}
			// 初测--初测危急范围标志
			else if (testItem.firstCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				refFlag = lowerdangeFlag;
			}
		}
		else
		{
			// 复查--复查危急范围标志
			if (testItem.retestCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				refFlag = upperdangeFlag;
			}
			// 复查--复查危急范围标志
			else if (testItem.retestCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				refFlag = lowerdangeFlag;
			}
		}
	}

	// 若是LHI，需要考虑+++状态
	QString status;
	bool isSind = CommonInformationManager::GetInstance()->IsAssaySIND(testItem.assayCode);
	if (isSind)
	{
		// 复查
		if (reCheck)
		{
			if (testItem.__isset.retestQualitativeJudge && tf::QualJudge::type::FuzzyRegion < testItem.retestQualitativeJudge)
			{
				int start = testItem.retestQualitativeJudge - tf::QualJudge::CH_SIND_LEVEL1;
				status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(testItem.assayCode, start);
			}
		}
		// 初测
		else
		{
			if (testItem.__isset.firstQualitativeJudge && tf::QualJudge::type::FuzzyRegion < testItem.firstQualitativeJudge)
			{
				int start = testItem.firstQualitativeJudge - tf::QualJudge::CH_SIND_LEVEL1;
				status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(testItem.assayCode, start);
			}
		}
	}

	// ++++状态放在最前面
	refFlag = status + refFlag;
	return refFlag;
}

QVariant QDataItemInfo::ResultRangeStatus(tf::TestItem& dataItem, int column, bool isSample) const
{
	// 结果提示
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();

	boost::optional<std::pair<int, int>> columnFirstResult = boost::none;
	boost::optional<std::pair<int, int>> columnLastResult = boost::none;
	// 按样本展示的项目结果
	if (isSample)
	{
		// 初测行
		columnFirstResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLEASSAY, int(SampleColumn::SAMPLEASSAY::RESULT));
		// 复查行
		columnLastResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLEASSAY, int(SampleColumn::SAMPLEASSAY::RECHECKRESULT));
	}
	// 按项目展示的结果
	else
	{
		// 初测行
		columnFirstResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::RESULT));
		// 复查行
		columnLastResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::RECHECKRESULT));
	}

	// 初测
	if (columnFirstResult && columnFirstResult.value().first == column)
	{
		if (dataItem.__isset.firstRefRangeJudge && resultTips.enabledOutRange)
		{
			if (dataItem.firstRefRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				return ColorType::UPPERCOLOR;
			}
			else if (dataItem.firstRefRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				return ColorType::LOWERCOLOR;
			}
		}
	}
	// 复查
	else if (columnLastResult && columnLastResult.value().first == column)
	{
		// 若存在正在倒计时的项目，字体显示为黑色（bug0014374 modify by wuht）
		auto runtimes = DataPrivate::Instance().GetRuntimesFromId(dataItem.id);
		if (runtimes.has_value())
		{
			return ColorType::NORMALCOLOR;
		}

		if (dataItem.__isset.retestRefRangeJudge && resultTips.enabledOutRange)
		{
			if (dataItem.retestRefRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				return  ColorType::UPPERCOLOR;
			}
			else if (dataItem.retestRefRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				return ColorType::LOWERCOLOR;
			}
		}
	}

	return ColorType::NORMALCOLOR;
}

boost::optional<bool> QDataItemInfo::ResultDangerStatus(tf::TestItem& dataItem, int column, bool isSample) const
{
	// 结果提示
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();

	boost::optional<std::pair<int, int>> columnFirstResult = boost::none;
	boost::optional<std::pair<int, int>> columnLastResult = boost::none;
	// 按样本展示的项目结果
	if (isSample)
	{
		// 初测行
		columnFirstResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLEASSAY, int(SampleColumn::SAMPLEASSAY::RESULT));
		// 复查行
		columnLastResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLEASSAY, int(SampleColumn::SAMPLEASSAY::RECHECKRESULT));
	}
	// 按项目展示的结果
	else
	{
		// 初测行
		columnFirstResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::RESULT));
		// 复查行
		columnLastResult = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::RECHECKRESULT));
	}

	// 初测
	if (columnFirstResult && columnFirstResult.value().first == column && resultTips.enabledCritical)
	{
		// 若是无法计算，不显示背景色
		auto pos = dataItem.resultStatusCodes.find("Calc");
		if (pos != std::string::npos)
		{
			return boost::none;
		}

		// 初测--初测危急范围标志
		if (dataItem.__isset.firstCrisisRangeJudge && dataItem.firstCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			return true;
		}
		else if (dataItem.__isset.firstCrisisRangeJudge && dataItem.firstCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			return false;
		}
	}
	// 复查
	else if (columnLastResult && columnLastResult.value().first == column && resultTips.enabledCritical)
	{
		// 若存在正在倒计时的项目，那么复查结果的背景色不能显示（需求)
		auto runtimes = DataPrivate::Instance().GetRuntimesFromId(dataItem.id);
		if (runtimes.has_value())
		{
			return boost::none;
		}

		// 若是无法计算，不显示背景色
		auto pos = dataItem.resultStatusCodes.find("Calc");
		if (pos != std::string::npos)
		{
			return boost::none;
		}

		if (dataItem.__isset.retestCrisisRangeJudge)
		{
			if (dataItem.retestCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
			{
				return true;
			}
			else if (dataItem.retestCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
			{
				return false;
			}
		}
	}

	return boost::none;
}

bool QDataItemInfo::IsAiRecognition()
{
	// 只有在联机模式下才有AI
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		return m_workSet.aiRecognition;
	}

	return false;
}

///
/// @brief 根据保留小数位数转换字符串
///
/// @param[in]  rawData  原始字符串
/// @param[in]  key		 分隔符
/// @param[in]  left	 保留小数位数
///
/// @return 转换后的字符串，当转化失败返回原始字符串
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月7日，新建函数
///
QString QDataItemInfo::GetValueFromLeft(const QString& rawData, const QString& key, int left)
{
	if (rawData.isEmpty() || key.isEmpty() || left < 0)
	{
		return rawData;
	}

	// 分割电动势的符号
	QStringList datas = rawData.split(key);
	for (QString& strData : datas)
	{
		// 若其中有不是纯数字的字符串直接返回原字符
		bool ok = false;
		double tmpData = strData.toDouble(&ok);
		if (!ok)
		{
			return rawData;
		}

		// 按照小数位数要求格式
        strData = QString::number(tmpData, 'f', left);
	}

	return datas.join(key);
}



///
/// @brief 获取项目测试结果显示字符串
///
/// @param[in]  reCheck  是否复查（true:代表复查,false:代表初次检查）
/// @param[in]  testItem 测试项目
///
/// @return 结果字符串
///
/// @par History:
/// @li 5774/WuHongTao，2023年5月19日，新建函数
///
QString QDataItemInfo::GetAssayResult(bool reCheck, const tf::TestItem& testItem, std::shared_ptr<tf::SampleInfo> data, bool bPrint)
{
    // 小数点后保留几位小数(默认2位)
    int left = 2;
    // 单位倍率
    double factor = 1.0;
    // 项目类型
    tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

	bool isCalc = CommonInformationManager::IsCalcAssay(testItem.assayCode);
    if (isCalc)
    {
        auto spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(testItem.assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            left = spCalcInfo->decimalDigit;
        }
    }
    else
    {
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(testItem.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            left = spAssayInfo->decimalPlace;
            classi = spAssayInfo->assayClassify;
            for (const tf::AssayUnit& unit: spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    factor = unit.factor;
                    break;
                }
            }
        }
    }

	bool isIse = IsISE(testItem.assayCode);
	// 样本显示的设置
    const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	// 结果提示的设置
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();

    QString flag = "", refFlag = "";
    QString result = "";
    double conc = -1;
    int diluFactor = 1;
    // 复查
    if (reCheck)
    {
        if (!testItem.__isset.retestConc)
        {
            return result;
        }

        conc = testItem.retestConc;
        // 是否是编辑的后的浓度（校准品质控品不能修改）
        if (testItem.__isset.retestEditConc && testItem.retestEditConc)
        {
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR 
                && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN
                && !bPrint)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}
        }

		// 校准质控无重测，不判定
		refFlag = ResultRangeFlag(true, testItem, resultTips);

        // 校准品/质控品直接显示信号值
        if (!WorkpageCommon::IsShowConc(sampleSet, data->sampleType))
        {
			if (testItem.__isset.retestRLU && !testItem.retestRLU.empty())
			{
				int leftvalue = 0;
				// 获取按照小数位数格式化的字符串
				if (isIse)
				{
					leftvalue = left;
				}
				result = GetValueFromLeft(QString::fromStdString(testItem.retestRLU), "/", leftvalue);
			}

			// ISE是没有吸光度，直接返回电动势
			if (isIse)
			{
				return result;
			}
			// 吸光度是否符合显示要求
			else if (!WorkpageCommon::IsRLUValid(testItem.retestRLU))
			{
				return "";
			}

			return result;
        }   

        if (testItem.__isset.retestTotalDilutionFactor)
        {
            diluFactor = testItem.retestTotalDilutionFactor;
        }
    }
    // 初测
    else
    {
        if (!testItem.__isset.conc)
        {
            return result;
        }

        conc = testItem.conc;
        // 是否是编辑的后的浓度（校准品质控品不能修改）
        if (testItem.__isset.editConc && testItem.editConc)
        {
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR 
                && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN
                && !bPrint)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}
        }

		if (data->sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
		{
			refFlag = ResultRangeFlag(false, testItem, resultTips);
		}

        // 校准品/质控品直接显示信号值
        if (!WorkpageCommon::IsShowConc(sampleSet, data->sampleType))
        {
			if (testItem.__isset.RLU && !testItem.RLU.empty())
			{
				// 获取按照小数位数格式化的字符串
				int leftvalue = 0;
				// 获取按照小数位数格式化的字符串
				if (isIse)
				{
					leftvalue = left;
				}
				result = GetValueFromLeft(QString::fromStdString(testItem.RLU), "/", leftvalue);
			}

			// ISE是没有吸光度，直接返回电动势
			if (isIse)
			{
				return result;
			}
			// 吸光度是否符合显示要求
			else if (!WorkpageCommon::IsRLUValid(testItem.RLU))
			{
				return "";
			}

            return result;
        }

        if (testItem.__isset.firstTotalDilutionFactor)
        {
            diluFactor = testItem.firstTotalDilutionFactor;
        }
    }

    // 免疫
    if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        if (conc < 0)
        {
            //result = tr("未计算");
            result = tr(""); //所有未计算都要求显示空白
            return result;
        }

        result = flag + QString::number(conc*factor, 'f', left)+ refFlag;
        // 校准品不显示线性范围-bug19441
        if (data->sampleType == tf::SampleType::type::SAMPLE_TYPE_CALIBRATOR)
        {
            return result;
        }

        // 查询通用项目信息
        std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
        CommonInformationManager::GetInstance()->GetImmuneAssayInfos(imGAIs, testItem.assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
        if (imGAIs.size() <= 0)
        {
            ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
            return result;
        }

        // 线性范围(<*或>*)也需要显示参考范围标志bug24912   
        if (!imGAIs[0]->extenL && conc < imGAIs[0]->techRangeMin*diluFactor)
        {
            result = "<" + QString::number(imGAIs[0]->techRangeMin*factor*diluFactor, 'f', left) + refFlag;;
            return result;
        }
        else if (!imGAIs[0]->extenH && conc > imGAIs[0]->techRangeMax*diluFactor)
        {
            result = ">" + QString::number(imGAIs[0]->techRangeMax*factor*diluFactor, 'f', left) + refFlag;;
            return result;
        }
    }
    else
    {
        result = flag + QString::number(conc*factor, 'f', left) + refFlag;
    }

	// 当无法计算的时候浓度也不显示
	if (reCheck)
	{	
		if (!WorkpageCommon::IsConcValid(isCalc, testItem.retestResultStatusCodes, testItem.retestConc))
		{
			return "";
		}
	}
	else
	{
		if (!WorkpageCommon::IsConcValid(isCalc, testItem.resultStatusCodes, testItem.conc))
		{
			return "";
		}
	}

	// 若是计算项目，则只有在（DBL_MAX）的时候才不显示
	if (isCalc)
	{
		if (DBL_MAX == conc)
		{
			result = "";
		}
	}
	else
	{
		// 普通项目，当是负值的时候，不显示
		if (conc < 0)
		{
			result = "";
		}
	}

    return result;
}

///
/// @brief 为给定的结果设置偏离标记
///
/// @param[in]  dValue      给定的结果值（原始值或修改后的值）
/// @param[in]  reCheck     是否复查（true:代表复查,false:代表初次检查）
/// @param[in]  testItem    测试项目
/// @param[in]  sampleType  样本类型
///
/// @return 结果字符串
///
/// @par History:
/// @li 6889/ChenWei，2023年10月31日，新建函数
///
QString QDataItemInfo::SetAssayResultFlag(double dValue, bool reCheck, bool isOriginal, const tf::TestItem& testItem, tf::SampleType::type sampleType, bool bPrint)
{
    // 小数点后保留几位小数(默认2位)
    int left = 2;

    // 单位倍率
    double factor = 1.0;

    // 项目类型
    tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;
    bool isCalc = CommonInformationManager::IsCalcAssay(testItem.assayCode);
    if (isCalc)
    {
        auto spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(testItem.assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            left = spCalcInfo->decimalDigit;
        }
    }
    else
    {
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(testItem.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            left = spAssayInfo->decimalPlace;
            classi = spAssayInfo->assayClassify;
            for (const tf::AssayUnit& unit : spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    factor = unit.factor;
                    break;
                }
            }
        }
    }

    // 样本显示的设置
    const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

    // 结果提示的设置
    const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();
    QString flag = "";
    QString refFlag = "";
    QString result = "";
    double conc = dValue;
    int diluFactor = 1;

    // 复查
    if (reCheck)
    {
        if (!testItem.__isset.retestConc)
        {
            return result;
        }

        // 是否是编辑的后的浓度（校准品质控品不能修改）
        if (testItem.__isset.retestEditConc && testItem.retestEditConc)
        {
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN && !bPrint)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}
        }

        // 校准质控无重测，不判定
        refFlag = ResultRangeFlag(true, testItem, resultTips);
        if (testItem.__isset.retestTotalDilutionFactor)
        {
            diluFactor = testItem.retestTotalDilutionFactor;
        }
    }
    else // 初测
    {
        if (!testItem.__isset.conc)
        {
            return result;
        }

        // 是否是编辑的后的浓度（校准品质控品不能修改）
        if (testItem.__isset.editConc && testItem.editConc)
        {
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN && !bPrint)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}
        }

        if (sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
        {
            refFlag = ResultRangeFlag(false, testItem, resultTips);
        }

        if (testItem.__isset.firstTotalDilutionFactor)
        {
            diluFactor = testItem.firstTotalDilutionFactor;
        }
    }

    // 免疫
    if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        if (conc < 0)
        {
            result = tr("");
            return result;
        }

        // 原始结果不需要其他标志（张浩楠说的 2024.6.21）
        if (isOriginal)
        {
            result = QString::number(conc * factor, 'f', left);
            return result;
        }

        result = flag + QString::number(conc * factor, 'f', left) + refFlag;

        // 校准品不显示线性范围-bug19441
        if (sampleType == tf::SampleType::type::SAMPLE_TYPE_CALIBRATOR)
        {
            return result;
        }

        // 查询通用项目信息
        std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
        CommonInformationManager::GetInstance()->GetImmuneAssayInfos(imGAIs, testItem.assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
        if (imGAIs.size() <= 0)
        {
            ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
            return result;
        }
        // 定性项目稀释倍数为1-bug32256补充
        if (imGAIs[0]->caliType == 0)
        {
            diluFactor = 1;
        }

        // 线性范围(<*或>*)加参考范围标志bug24912      
        if (!imGAIs[0]->extenL && conc < imGAIs[0]->techRangeMin * diluFactor)
        {
            result = "<" + QString::number(imGAIs[0]->techRangeMin * factor * diluFactor, 'f', left) + refFlag;
            return result;
        }
        else if (!imGAIs[0]->extenH && conc > imGAIs[0]->techRangeMax * diluFactor)
        {
            result = ">" + QString::number(imGAIs[0]->techRangeMax * factor * diluFactor, 'f', left) + refFlag;
            return result;
        }
    }
    else
    {
        if (conc < std::numeric_limits<double>::lowest())
        {
            result = "";
        }
        else
        {
            result = flag + QString::number(conc * factor, 'f', left) + refFlag;
        }
    }

    // 若是计算项目，则只有在（DBL_MAX）的时候才不显示
    if (isCalc)
    {
        if (DBL_MAX == conc)
        {
            result = "";
        }
    }
    else
    {
        // 普通项目，当是负值的时候，不显示
        if (conc < 0)
        {
            result = "";
        }
    }

    return result;
}


///
/// @brief 获取结果状态
///
/// @param[in]  assayResult  项目结果
///
/// @return 结果状态字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月4日，新建函数
///
QString QDataItemInfo::GetAssayResultStatus(const AssayResult& assayAnyResult, const std::string& conc)
{
    if (!assayAnyResult.has_value())
    {
        return "";
    }


    if (typeid(ch::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ch::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.resultStatusCodes)
        {
            return "";
        }

        // 连接结果状态
        auto resultStatus = assayResult.resultStatusCodes;
        auto status = std::accumulate(resultStatus.begin(), resultStatus.end(), std::string(""), [&](const std::string& a, const auto& b)
        {return (a + b); });

        return QString::fromStdString(status);
    }
    // 免疫
    else if (typeid(im::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.resultStatusCodes)
        {
            return "";
        }

        // 连接结果状态
        auto resultStatus = assayResult.resultStatusCodes;
		std::string status;
        ResultStatusCodeManager::GetInstance()->GetDisplayResCode(resultStatus, status, 
            conc, assayResult.refRangeJudge, assayResult.crisisRangeJudge, assayResult.sampType);

        return QString::fromStdString(status);
    }
    // ISE
    else if (typeid(ise::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ise::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.resultStatusCodes)
        {
            return "";
        }

        // 连接结果状态
        auto resultStatus = assayResult.resultStatusCodes;
        auto status = std::accumulate(resultStatus.begin(), resultStatus.end(), std::string(""), [&](std::string a, const auto& b)
        {return (a + b); });

        return QString::fromStdString(status);
    }
    // 计算项目
    else if (typeid(tf::CalcAssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<tf::CalcAssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.resultStatusCodes)
        {
            return "";
        }

        // 连接结果状态
        auto resultStatus = assayResult.resultStatusCodes;
        auto status = std::accumulate(resultStatus.begin(), resultStatus.end(), std::string(""), [&](std::string a, const auto& b)
        {return (a + b); });

        return QString::fromStdString(status);
    }

    return "";
}

///
/// @brief 获取项目当前单位的字符串
///
/// @param[in]  assayCode  项目编号
///
/// @return 字符串单位
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月3日，新建函数
///
std::string QDataItemInfo::GetAssayResultUnitName(int assayCode)
{
    if (CommonInformationManager::IsCalcAssay(assayCode))
    {
        auto pAssayCalc = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
        return (pAssayCalc == nullptr) ? "" : pAssayCalc->resultUnit;
    }
    else
    {
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
        if (spAssayInfo != nullptr)
        {
            for (const tf::AssayUnit& unit : spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    return unit.name;
                }
            }
        }
    }

    return "";
}

///
/// @brief 获取设备名称
///
/// @param[in]  assayAnyResult  测试结果
///
/// @return 设备名称
///
/// @par History:
/// @li 5774/WuHongTao，2023年4月28日，新建函数
///
QString QDataItemInfo::GetDeivceName(const AssayResult& assayAnyResult)
{
    if (!assayAnyResult.has_value())
    {
        return "";
    }

    if (typeid(ch::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ch::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.deviceSN)
        {
            return "";
        }

        auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(assayResult.deviceSN);
        if (deviceInfo == Q_NULLPTR)
        {
            return "";
        }

        return QString::fromStdString(deviceInfo->groupName + deviceInfo->name);
    }
    // 免疫
    else if (typeid(im::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.deviceSN)
        {
            return "";
        }

        auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(assayResult.deviceSN);
        if (deviceInfo == Q_NULLPTR)
        {
            return "";
        }

        return QString::fromStdString(deviceInfo->groupName + deviceInfo->name);
    }
    // ISE
    else if (typeid(ise::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ise::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.deviceSN)
        {
            return "";
        }

        auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(assayResult.deviceSN);
        if (deviceInfo == Q_NULLPTR)
        {
            return "";
        }

        return QString::fromStdString(deviceInfo->groupName + deviceInfo->name);
    }

    return "";
}

///
/// @brief 获取参考范围字符串
///
/// @param[in]  assayAnyResult  测试结果
///
/// @return 设备名称
///
/// @par History:
/// @li 1226/zhangjing，2023年8月8日，新建函数
/// @li 6889/ChenWei，2023年11月22日，增加样本类型和病人信息参数用于获取计算项目的参考区间
///
QString QDataItemInfo::GetRefStr(const AssayResult& assayAnyResult, int sourceType, std::shared_ptr<tf::PatientInfo> pPatientInfo)
{
	auto GetRange = [](const int assayCode, const std::string range)->QString
	{
		std::string strRef = range;
		std::istringstream iss(strRef);
		char delimiter;
		double min, max, factor = 1;
		// 符合“数字-数字”格式       
		if (iss >> min >> delimiter >> max && delimiter == '-')
		{
			auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
			if (spAssayInfo != Q_NULLPTR)
			{
				for (const tf::AssayUnit& unit : spAssayInfo->units)
				{
					if (unit.isCurrent)
					{
						factor = unit.factor;
						break;
					}
				}
				if (factor != 1)
				{
					int left = spAssayInfo->decimalPlace;
					strRef = (QString::number(min*factor, 'f', left) + "-" + QString::number(max*factor, 'f', left)).toStdString();
				}
			}

		}
		return QString::fromStdString(strRef);
	};

    if (!assayAnyResult.has_value())
    {
        return "";
    }

    if (typeid(ch::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ch::tf::AssayTestResult>(assayAnyResult.value());
		if (!assayResult.__isset.refRange)
		{
			return "";
		}

		return GetRange(assayResult.assayCode, assayResult.refRange);
    }
    // 免疫
    else if (typeid(im::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(assayAnyResult.value());
        if (!assayResult.__isset.referenceStr)
        {
            return "";
        }
        std::string strRef = assayResult.referenceStr;
        std::istringstream iss(strRef);
        char delimiter;
        double min, max, factor = 1;
        // 符合“数字-数字”格式       
        if (iss >> min >> delimiter >> max && delimiter == '-')
        {
            auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayResult.assayCode);
            if (spAssayInfo != Q_NULLPTR)
            {
                for (const tf::AssayUnit& unit : spAssayInfo->units)
                {
                    if (unit.isCurrent)
                    {
                        factor = unit.factor;
                        break;
                    }
                }
                if (factor != 1)
                {
                    int left = spAssayInfo->decimalPlace;
                    strRef = (QString::number(min*factor, 'f', left) + "-" + QString::number(max*factor, 'f', left)).toStdString();
                }
            }

        }
        return QString::fromStdString(strRef);
    }
    // ISE
    else if (typeid(ise::tf::AssayTestResult) == assayAnyResult.value().type())
    {
        auto assayResult = boost::any_cast<ise::tf::AssayTestResult>(assayAnyResult.value());
		if (!assayResult.__isset.refRange)
		{
			return "";
		}

		return GetRange(assayResult.assayCode, assayResult.refRange);
    }
    // 计算项目
    else if (typeid(tf::CalcAssayTestResult) == assayAnyResult.value().type())
    {
        std::shared_ptr<CommonInformationManager> cmPtr = CommonInformationManager::GetInstance();
        if (cmPtr == nullptr)
        {
            return "";
        }

        auto assayResult = boost::any_cast<tf::CalcAssayTestResult>(assayAnyResult.value());
        auto calcAssayInfo = cmPtr->GetCalcAssayInfo(assayResult.assayCode);
        ::tf::AssayReferenceItem matchRef;
        cmPtr->GetCalcAssayResultReference(assayResult.assayCode, sourceType, pPatientInfo, matchRef);
        if (matchRef.fUpperRefRang == DBL_MAX || matchRef.fUpperRefRang == 0)
        {
            return " ";
        }
        else
        {
            QString strRef = QString::number(matchRef.fLowerRefRang, 'f', calcAssayInfo->decimalDigit) + "-" 
                            + QString::number(matchRef.fUpperRefRang, 'f', calcAssayInfo->decimalDigit);
            return strRef;
        }
    }

    return "";
}

///
/// @brief 获取试剂吸取类型
///
/// @param[in]  itemData  数据
///
/// @return 试剂吸取类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月6日，新建函数
///
QVariant QDataItemInfo::GetSuckType(tf::TestItem& itemData)
{
    if (itemData.dilutionFactor == 1)
    {
        QPixmap pixmap;
        if (itemData.suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_DEC)
        {
            pixmap.load(QString(":/Leonis/resource/image/icon-dec.png"));
        }
        else if (itemData.suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_INC)
        {
            pixmap.load(QString(":/Leonis/resource/image/icon-inc.png"));
        }
        else
        {
            return QVariant();
        }
        return QVariant(pixmap);
    }
    else
    {
        return QVariant(itemData.dilutionFactor);
    }
}

QVariant QDataItemInfo::GetDiluTionShow(tf::TestItem& dataItem)
{
	QVariant dilition;
	if (dataItem.preDilutionFactor <= 1)
	{
		dilition = QDataItemInfo::GetSuckType(dataItem);
	}
	else
	{
		dilition = dataItem.preDilutionFactor;
	}

	// 手工稀释若没
	if (!dataItem.__isset.preDilutionFactor)
	{
		dilition = QVariant();
	}

	auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(dataItem.assayCode);
	if (spAssayInfo == nullptr)
	{
		return dilition;
	}
	
	// 若非生化项目直接返回
	if (spAssayInfo->assayClassify != tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
	{
		return dilition;
	}

	// 稀释倍数
	int dilutionFactor = -1;
	// 机外稀释
	if (dataItem.preDilutionFactor > 1)
	{
		dilutionFactor = dataItem.preDilutionFactor;
		dilition = dataItem.preDilutionFactor;
	}

	// 若没有设置机内稀释，直接返回
	if (!dataItem.__isset.dilutionFactor)
	{
		return dilition;
	}

	// 机内稀释，按倍数稀释
	if (dataItem.dilutionFactor > 1)
	{
		dilutionFactor = (dilutionFactor > 1) ? (dilutionFactor * dataItem.dilutionFactor) : dataItem.dilutionFactor;
	}
	// 机内稀释，按常量增量减量稀释
	else if (1 == dataItem.dilutionFactor)
	{
		//已做机外稀释
		if (dilutionFactor > 1)
		{
			auto spDeviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(dataItem.deviceSN);
			if (spDeviceInfo == nullptr)
			{
				return dilition;
			}

			auto chGAI = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(dataItem.assayCode, spDeviceInfo->deviceType);
			if (chGAI == nullptr)
			{
				return dilition;
			}

			auto spChemistryInfo = CommonInformationManager::GetInstance()->GetHighestPrioritySpecialAssay(dataItem.assayCode, spDeviceInfo->deviceType, chGAI->version);
			if (spChemistryInfo == nullptr)
			{
				return dilition;
			}

			int size = spChemistryInfo->sampleAspirateVols.size();
			if (dataItem.suckVolType >= size)
			{
				return dilition;
			}

			auto sav = spChemistryInfo->sampleAspirateVols[dataItem.suckVolType];
			int innerFactor = 1;
			if (sav.originalSample > 0 && sav.diluent > 0)
			{
				innerFactor = sav.diluent / sav.originalSample + 1;
			}

			dilutionFactor = (innerFactor > 1) ? (dilutionFactor * innerFactor) : dilutionFactor;
		}
	}

    return (dilutionFactor < 0) ? dilition : dilutionFactor;
}

///
/// @brief 根据秒数将倒计时转化为（00：00：00）
///
/// @param[in]  second  秒数
///
/// @return 倒计时
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月15日，新建函数
///
QString QDataItemInfo::ConvertTime(int second)
{
    QString time;
    time = "%1:%2:%3";

    if (second <= 0)
    {
        return time.arg("00").arg("00").arg("00");
    }

    int hour = second / 3600;
    int minute = (second % 3600) / 60;
    int secondleft = (second % 3600) % 60;

    QString hours;
    // 小时
    if (hour == 0)
    {
        hours = "00";
    }
    else
    {
        hours = QString::number(hour);

        // 只有一位的时候补足两位
        if (hours.size() == 1)
        {
            hours = "0" + hours;
        }
    }

    QString minutes;
    // 分钟
    if (minute == 0)
    {
        minutes = "00";
    }
    else
    {
        minutes = QString::number(minute);

        // 只有一位的时候补足两位
        if (minutes.size() == 1)
        {
            minutes = "0" + minutes;
        }
    }

    QString seconds;
    // 秒
    if (secondleft == 0)
    {
        seconds = "00";
    }
    else
    {
        seconds = QString::number(secondleft);

        // 只有一位的时候补足两位
        if (seconds.size() == 1)
        {
            seconds = "0" + seconds;
        }
    }

    return time.arg(hours).arg(minutes).arg(seconds);
}

///
/// @brief 若项目正在运行，则开始推算项目测试完毕的倒计时
///
/// @param[in]  testItemInfo  项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月15日，新建函数
///
void DataPrivate::StartTestTimeCountDown(const tf::TestItem& testItemInfo) const
{
	// 若项目正在测试状态
    if (testItemInfo.__isset.samplingCompleted
        && testItemInfo.samplingCompleted
		// 与刘俊商量，项目必须在测试中才能有倒计时
		&& testItemInfo.__isset.status
		&& (testItemInfo.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTING)
		// modify bug0012277 by wuht
		&& testItemInfo.__isset.needReactionSeconds
		&& testItemInfo.needReactionSeconds > 0)
	{
        // 反应时间获取函数
        auto function = [&](decltype(testItemInfo) arg)->int {return this->GetReactionTime(arg); };
        // 添加
        m_countDown.addElement(testItemInfo.id, testItemInfo, function);
	}
	else
	{
        // 减少
        m_countDown.removeElement(testItemInfo.id);
	}
}

///
/// @brief 根据id获取剩余运行时间
///
/// @param[in]  id  项目的数据库主键
///
/// @return 剩余执行时间
///
/// @par History:
/// @li 5774/WuHongTao，2023年4月3日，新建函数
///
boost::optional<int> DataPrivate::GetRuntimesFromId(int64_t id) const
{
    return m_countDown.getValue(id);
}

///
/// @brief 根据项目信息获取反应时间（单位秒）
///
/// @param[in]  testItemInfo 项目信息  
///
/// @return 反应时间秒
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月15日，新建函数
///
int DataPrivate::GetReactionTime(const tf::TestItem& testItemInfo) const
{
	// 剩余反应时间
	int leftReactionTime = INVALID_VALUE;
	// 开始测试时间以及设置(加样后开始反应，界面会显示倒计时)
	if (testItemInfo.__isset.samplingCompleted 
        && testItemInfo.samplingCompleted)
	{
        // 若设置预计反应时间
        if (testItemInfo.__isset.needReactionSeconds
            && testItemInfo.needReactionSeconds > 0)
        {
            leftReactionTime = testItemInfo.needReactionSeconds;
        }

		auto lostTime = boost::posix_time::second_clock::local_time() - TimeStringToPosixTime(testItemInfo.latestReactionTime);
		auto lostSecond = lostTime.total_seconds();
		if (leftReactionTime > lostSecond)
		{
			leftReactionTime = leftReactionTime - lostSecond;
		}
		else
		{
			leftReactionTime = INVALID_VALUE;
		}
	}

	return leftReactionTime;
}

bool DataPrivate::ResetTestItemDulition(tf::TestItem& testItemInfo)
{
    tf::TestResultKeyInfo testKey;
    // 若有复查结果，则取复查的稀释倍数重置
    if (testItemInfo.__isset.lastTestResultKey)
    {
        testKey = testItemInfo.lastTestResultKey;
    }
    // 若无复查结果，检查是否初测结果
    else if (testItemInfo.__isset.firstTestResultKey)
    {
        testKey = testItemInfo.firstTestResultKey;
    }
    // 若没有结果，不能使用此函数
    else
    {
        return false;
    }

    // 只处理生化和免疫的结果
    if ((testKey.deviceType != tf::DeviceType::DEVICE_TYPE_C1000)
        && (testKey.deviceType != tf::DeviceType::DEVICE_TYPE_I6000))
    {
        return false;
    }

    // 获取测试结果
    // 生化项目
    if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
    {
        ch::tf::AssayTestResultQueryResp qryResp;
        ch::tf::AssayTestResultQueryCond qryCond;
        if (testKey.assayTestResultId > 0)
        {
            qryCond.__set_id(testKey.assayTestResultId);
            if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond)
                || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || qryResp.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s( ch)", __FUNCTION__);
                return false;
            }

			// 机内稀释
            auto chResult = qryResp.lstAssayTestResult[0];
            if (chResult.__isset.dilutionFactor)
            {
                testItemInfo.__set_dilutionFactor(chResult.dilutionFactor);
            }

            if (chResult.__isset.suckVolType)
            {
                testItemInfo.__set_suckVolType(tf::SuckVolType::type(chResult.suckVolType));
            }

			// 手工稀释
			if (chResult.__isset.preDilutionFactor)
			{
				testItemInfo.__set_preDilutionFactor(chResult.preDilutionFactor);
			}
        }
    }
    // 免疫项目
    else if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        im::tf::AssayTestResultQueryCond queryAssaycond;
        im::tf::AssayTestResultQueryResp assayTestResult;
        if (testKey.assayTestResultId > 0)
        {
            queryAssaycond.__set_id(testKey.assayTestResultId);
            if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s(im by id:%lld)", __FUNCTION__, testKey.assayTestResultId);
				return false;
            }

            auto imResult = assayTestResult.lstAssayTestResult[0];
            if (imResult.__isset.dilutionFactor)
            {
                testItemInfo.__set_dilutionFactor(imResult.dilutionFactor);
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

///
///  @brief 项目设置界面修改项目后刷新按项目显示表格
///
///
///  @param[in]   assayCode  项目通道号
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月30日，新建函数
///
void DataPrivate::UpdateItemShowDatas(int32_t assayCode)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);

	for (auto &it : m_itemShowMap)
    {
        if (it.second.isEmpty())
        {
            continue;
        }
        std::shared_ptr<tf::TestItem> spTestItem = m_testItemData.at(it.first);
        if (spTestItem->assayCode != assayCode)
        {
            continue;
        }
		if (m_sampleShowMap.count(spTestItem->sampleInfoId) > 0 && m_sampInfoData.count(spTestItem->sampleInfoId) > 0)
		{
            QString strSeq = QDataItemInfo::GetSeqNo(*m_sampInfoData[spTestItem->sampleInfoId]);
			m_sampleShowMap[spTestItem->sampleInfoId][static_cast<int>(SampleColumn::COL::SEQNO)] = strSeq;
		}
		for (int i = 0; i < it.second.size(); ++i)
		{
			it.second[i] = GetItemShowData(i, *spTestItem);
		}
	}
}

DataPrivate::DataPrivate()
{
    Init();
}

DataPrivate& DataPrivate::Instance()
{
    static DataPrivate dataPrivate;
    return dataPrivate;
}

DataPrivate::~DataPrivate()
{

}

bool DataPrivate::GetAssayAndTestTimes(tf::TestMode::type curTestCode, std::map<int, int>& result)
{
	for (const auto& sampleInfo : m_sampInfoData)
	{
		auto spSample = sampleInfo.second;
		if (spSample == Q_NULLPTR)
		{
			continue;
		}

		if (spSample->testMode != curTestCode)
		{
			continue;
		}

		auto iter = m_sampleMap.find(spSample->id);
		if (iter == m_sampleMap.end())
		{
			continue;
		}

		auto testDbs = iter->second;
		for (const auto& db : testDbs)
		{
			auto spTestItem = DataPrivate::Instance().GetTestItemInfoByDb(db);
			if (spTestItem == Q_NULLPTR)
			{
				continue;
			}

			if (spTestItem->status != tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING)
			{
				continue;
			}

			auto iterCodeMap = result.find(spTestItem->assayCode);
			if (iterCodeMap == result.end())
			{
				result[spTestItem->assayCode] = 1;
			}
			else
			{
				iterCodeMap->second++;
			}
		}
	}

	return true;
}

QVariant DataPrivate::GetMannualNumber(int64_t db)
{
	auto iter = m_sampleMap.find(db);
	if (iter == m_sampleMap.end())
	{
		return QVariant();
	}

	auto testItemdbs = iter->second;
	for (const auto db : testItemdbs)
	{
		auto testItemInfo = GetTestItemInfoByDb(db);
		if (testItemInfo == Q_NULLPTR)
		{
			continue;
		}

		// 必须是待测样本
		if (testItemInfo->__isset.status && testItemInfo->status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING)
		{
			// 同时是手工稀释
			if (testItemInfo->__isset.preDilutionFactor && testItemInfo->preDilutionFactor > 1)
			{
				return testItemInfo->preDilutionFactor;
			}
		}
	}

	return QVariant();
}

///
/// @brief 根据结果的关键信息获取结果
///
/// @param[in]  testKey  结果关键信息
///
/// @return 测试结果
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月24日，新建函数
///
AssayResult DataPrivate::GetAssayResultByTestKey(tf::TestResultKeyInfo& testKey) const
{
    // 生化项目
    if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
    {
        ch::tf::AssayTestResultQueryResp qryResp;
        ch::tf::AssayTestResultQueryCond qryCond;
        if (testKey.assayTestResultId > 0)
        {
            qryCond.__set_id(testKey.assayTestResultId);
            if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond)
                || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || qryResp.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s(failed query by id:%lld)", __FUNCTION__, testKey.assayTestResultId);
                return boost::none;
            }

			// 免疫陈伟修改建议
			auto result = qryResp.lstAssayTestResult[0];
			if (!WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
			{
				return boost::none;
			}

            boost::any resultAny = qryResp.lstAssayTestResult[0];
            return std::move(boost::make_optional(resultAny));
        }
    }
    // 免疫项目
    else if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        im::tf::AssayTestResultQueryCond queryAssaycond;
        im::tf::AssayTestResultQueryResp assayTestResult;
        if (testKey.assayTestResultId > 0)
        {
            queryAssaycond.__set_id(testKey.assayTestResultId);
            if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s(failed query by id:%lld)", __FUNCTION__, testKey.assayTestResultId);
                return boost::none;
            }

			// 免疫陈伟修改建议
			auto result = assayTestResult.lstAssayTestResult[0];
			if (!WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
			{
				return boost::none;
			}

            boost::any resultAny = assayTestResult.lstAssayTestResult[0];
            return std::move(boost::make_optional(resultAny));
        }
    }
    else if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
    {
        ise::tf::AssayTestResultQueryCond queryAssaycond;
        ise::tf::AssayTestResultQueryResp assayTestResult;
        if (testKey.assayTestResultId > 0)
        {
            queryAssaycond.__set_id(testKey.assayTestResultId);
            if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s(failed query by id:%lld)", __FUNCTION__, testKey.assayTestResultId);
                return boost::none;
            }

			// 免疫陈伟修改建议
			auto result = assayTestResult.lstAssayTestResult[0];
			if (!WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
			{
				return boost::none;
			}

            boost::any resultAny = assayTestResult.lstAssayTestResult[0];
            return std::move(boost::make_optional(resultAny));
        }
    }
    // 计算项目(上面强制设置为无效设备)
    else if (testKey.deviceType == tf::DeviceType::DEVICE_TYPE_INVALID)
    {
        tf::CalcAssayTestResultQueryCond queryAssaycond;
        tf::CalcAssayTestResultQueryResp assayTestResult;
        if (testKey.assayTestResultId > 0)
        {
            queryAssaycond.__set_id(testKey.assayTestResultId);
            if (!DcsControlProxy::GetInstance()->QueryCalcAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstCalcAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s(failed query by id:%lld)", __FUNCTION__, testKey.assayTestResultId);
                return boost::none;
            }

			// 免疫陈伟修改建议
			if (assayTestResult.lstCalcAssayTestResult[0].resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				return boost::none;
			}

            boost::any result = assayTestResult.lstCalcAssayTestResult[0];
            return std::move(boost::make_optional(result));
        }
    }

    return boost::none;
}

///
/// @brief 根据测试数据库主键获取测试结果
///
/// @param[in]  assayId  项目主键
///
/// @return 测试结果
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月10日，新建函数
///
AssayResults DataPrivate::GetAssayResultByTestItem(const tf::TestItem& testItem)
{
    AssayVec assayResultVec;
    assayResultVec.push_back(boost::none);
    assayResultVec.push_back(boost::none);

    // 判断是否计算项目
    bool isCalc = false;
    if (testItem.assayCode >= tf::AssayCodeRange::CALC_RANGE_MIN
        && testItem.assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
    {
        isCalc = true;
    }

    if (testItem.__isset.firstTestResultKey)
    {
        auto testKey = testItem.firstTestResultKey;
        if (isCalc)
        {
            testKey.deviceType = tf::DeviceType::DEVICE_TYPE_INVALID;
        }
        assayResultVec[0] = GetAssayResultByTestKey(testKey);
    }

    if (testItem.__isset.lastTestResultKey)
    {
        auto testKey = testItem.lastTestResultKey;
        if (isCalc)
        {
            testKey.deviceType = tf::DeviceType::DEVICE_TYPE_INVALID;
        }
        assayResultVec[1] = GetAssayResultByTestKey(testKey);
    }
    return std::move(std::make_shared<AssayVec>(assayResultVec));
}

/// @brief 通过行号获取样本数据
///
/// @param[in]  row  行号
///
/// @return 样本数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月22日，新建函数
///
boost::optional<tf::SampleInfo> DataPrivate::GetSampleByRow(const int row) const
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);

    if (row < 0 || row >= m_samplePostionVec.size())
    {
        return boost::none;
    }

    // 获取对应行的数据
    auto iter = m_sampInfoData.find(m_samplePostionVec[row]);
    if (iter != m_sampInfoData.end())
    {
        return *(iter->second);
    }

    return boost::none;
}

///
/// @brief 根据行号获取项目信息
///
/// @param[in]  row  行号
///
/// @return 项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月5日，新建函数
///
std::shared_ptr<tf::TestItem> DataPrivate::GetTestItemByRow(const int row) const
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);

    if (row < 0 || row >= m_testitemPostionVec.size())
    {
        return Q_NULLPTR;
    }

    // 获取对应行的数据
    auto iter = m_testItemData.find(m_testitemPostionVec[row]);
    if (iter != m_testItemData.end())
    {
        return iter->second;
    }

    return Q_NULLPTR;
}

///
/// @brief 根据样本信息获取行号
///
/// @param[in]  sample  样本信息
///
/// @return 行号
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月5日，新建函数
///
boost::optional<int> DataPrivate::GetRowBySample(tf::SampleInfo& sample)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    if (m_sampInfoData.count(sample.id) > 0)
    {
        int distance = std::distance(m_sampInfoData.begin(), m_sampInfoData.find(sample.id));
        return boost::make_optional(distance);
    }
    else
    {
        return boost::none;
    }
}

///
/// @brief 根据序号类型获取样本信息
///
/// @param[in]  seqNo		序号
/// @param[in]  type		类型
/// @param[in]  IsEmerge	是否急诊样板
/// @param[in]  mode	    样本所属模式
///
/// @return 样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月10日，新建函数
///
boost::optional<tf::SampleInfo> DataPrivate::GetSampleBySeq(tf::TestMode::type mode, const std::string& seqNo, tf::SampleType::type type, bool IsEmerge)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    auto ret = std::find_if(m_sampInfoData.begin(), m_sampInfoData.end(), [&](const auto& sample)->bool {
        return ((sample.second->seqNo == seqNo)
            && (sample.second->testMode == mode)
            && (sample.second->sampleType == type)
            && (sample.second->stat == IsEmerge)); });

    if (ret == m_sampInfoData.end())
    {
        return boost::none;
    }
    else
    {
        return boost::make_optional(*(ret->second));
    }
}

///
/// @brief 通过样本条码获取样本
///
/// @param[in]  barCode  样本条目
/// @param[in]  type     类型 
/// @param[in]  mode	 样本所属模式
///
/// @return 样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月12日，新建函数
///
boost::optional<tf::SampleInfo> DataPrivate::GetSampleByBarCode(tf::TestMode::type mode, 
    const std::string& barCode, tf::SampleType::type type)
{
    if (barCode.empty())
    {
        return boost::none;
    }

    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    auto ret = std::find_if(m_sampInfoData.begin(), m_sampInfoData.end(), [&](const auto& sample)->bool {
        return ((sample.second->barcode == barCode)
            && (sample.second->testMode == mode)
            && (sample.second->sampleType == type)); });

    if (ret == m_sampInfoData.end())
    {
        return boost::none;
    }
    else
    {
        return boost::make_optional(*(ret->second));
    }
}

///
/// @brief 根据数据库主键获取对应的行号
///
/// @param[in]  db  数据库主键
///
/// @return 正确范围行号
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
boost::optional<int> DataPrivate::GetSampleByDb(int64_t db)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    if (db < 0)
    {
        return boost::none;
    }

    if (m_sampInfoData.count(db) > 0)
    {
        return boost::make_optional((int)std::distance(std::begin(m_sampInfoData), m_sampInfoData.find(db)));
    }

    return boost::none;
}

///
///  @brief  通过数据库主键获取项目行号
///
///
///  @param[in]   db  数据库主键
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月31日，新建函数
///
boost::optional<int> DataPrivate::GetTestItemRowByDb(const int64_t db) const
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
	if (m_testItemData.count(db) > 0)
	{
		int distance = std::distance(m_testItemData.begin(), m_testItemData.find(db));
		return boost::make_optional(distance);
	}
	else
	{
		return boost::none;
	}
}

///
/// @brief 获取样本的项目的数据库主键集合
///
/// @param[in]  db  样本数据库主键
///
/// @return 项目数据库主键集合
///
/// @par History:
/// @li 5774/WuHongTao，2023年5月26日，新建函数
///
boost::optional<std::vector<int64_t>> DataPrivate::GetSampleTestItemMap(int64_t db)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    auto iter = m_sampleMap.find(db);
    if (iter == m_sampleMap.end())
    {
        return boost::none;
    }

    return iter->second;
}

std::vector<std::shared_ptr<tf::TestItem>> DataPrivate::GetSampleTestItems(int64_t db)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    auto iter = m_sampleMap.find(db);
    if (iter == m_sampleMap.end())
    {
        return {};
    }

    std::vector<std::shared_ptr<tf::TestItem>> testItemVec;
    for (auto& itemid : iter->second)
    {
        auto iterItem = m_testItemData.find(itemid);
        if (iterItem != m_testItemData.end())
        {
            testItemVec.push_back(iterItem->second);
        }
    }

    return std::move(testItemVec);
}

///
/// @brief 设置某行是否被选中
///
/// @param[in]  mode 0:代表按样本展示模式，否则是按项目展示模式
/// @param[in]  row  行坐标
/// @param[in]  flag 选中与否
///
/// @return true:表示是否设置选中
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月2日，新建函数
///
bool DataPrivate::SetSelectFlag(int mode, int row, bool flag)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    // 按样本展示
    if (mode == 0)
    {
        if (row < 0 || row >= m_samplePostionVec.size())
        {
            return false;
        }
        else
        {
            if (flag)
            {
                m_selectedSampleMap.insert(m_samplePostionVec[row]);
            }
            else
            {
                auto iter = m_selectedSampleMap.find(m_samplePostionVec[row]);
                if (iter != m_selectedSampleMap.end())
                {
                    m_selectedSampleMap.erase(iter);
                }
            }
        }
    }
    // 按项目展示
    else
    {
        if (row < 0 || row >= m_testitemPostionVec.size())
        {
            return false;
        }
        else
        {
            if (flag)
            {
                m_selectedItemMap.insert(m_testitemPostionVec[row]);
            }
            else
            {
                auto iter = m_selectedItemMap.find(m_testitemPostionVec[row]);
                if (iter != m_selectedItemMap.end())
                {
                    m_selectedItemMap.erase(iter);
                }
            }
        }
    }

    return true;
}

bool DataPrivate::SetAllSelectedFlag(int mode)
{
    // 按样本展示
    if (mode == 0)
    {
        for (auto& sampleDb : m_samplePostionVec)
        {
            m_selectedSampleMap.insert(sampleDb);
        }
    }
    // 按项目展示
    else
    {
        for (auto& testDb : m_testitemPostionVec)
        {
            m_selectedItemMap.insert(testDb);
        }
    }

    return true;
}

QVariant DataPrivate::GetSelectFlag(int mode, int row)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    if (0 == mode)
    {
        if (row < 0 || row >= m_samplePostionVec.size())
        {
            return QVariant();
        }
        else
        {
            auto keyDb = m_samplePostionVec[row];
            auto iter = m_selectedSampleMap.find(keyDb);
            if (iter != m_selectedSampleMap.end())
            {
                return true;
            }
        }
    }
    else
    {
        if (row < 0 || row >= m_testitemPostionVec.size())
        {
            return QVariant();
        }
        else
        {
            auto keyDb = m_testitemPostionVec[row];
            auto iter = m_selectedItemMap.find(keyDb);
            if (iter != m_selectedItemMap.end())
            {
                return true;
            }
        }
    }

    return false;
}

bool DataPrivate::ClearSelected(int mode)
{
    if ( 0 == mode)
    {
        m_selectedSampleMap.clear();
    }
    else
    {
        m_selectedItemMap.clear();
    }

    // 刷新
    return true;
}

int DataPrivate::GetPrevSelectedRow(int mode)
{
    if (0 == mode)
    {
        for (auto si : m_selectedSampleMap)
        {
            for (int i = 0; i< m_samplePostionVec.size(); ++i)
            {
                if (m_samplePostionVec[i] == si)
                {
                    return i;
                }
            }
        }
    }
    else
    {
        for (auto ti : m_selectedItemMap)
        {
            for (int i=0; i<m_testitemPostionVec.size(); ++i)
            {
                if (m_testitemPostionVec[i] == ti)
                {
                    return i;
                }
            }
        }
    }

    return -1;
}

std::set<int> DataPrivate::GetSampleAssayCodes(int64_t db)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
    auto iter = m_sampleMap.find(db);
    if (iter == m_sampleMap.end())
    {
        return {};
    }

    std::set<int> assayCodes;
    for (auto& itemid : iter->second)
    {
        auto iterItem = m_testItemData.find(itemid);
        if (iterItem != m_testItemData.end()
            && assayCodes.find(iterItem->second->assayCode) == assayCodes.end())
        {
            assayCodes.insert(iterItem->second->assayCode);
        }
    }

    return std::move(assayCodes);
}

///
///  @brief 获取按样本显示的显示数据
/// 
///
///  @param[in]   mode   模式
///  @param[in]   index  索引
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
///
QVariant DataPrivate::GetSampleDisplayData(SampleColumn::SAMPLEMODE mode, int row, int column)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
	if (row < 0 || row >= m_samplePostionVec.size() ||
        column < 0 || column >= 10)
	{
		return QVariant();
    }

    auto iter = m_sampleShowMap.find(m_samplePostionVec[row]);
    if (iter == m_sampleShowMap.end())
	{
		return QVariant();
	}
    auto dataVec = iter->second;
	if (dataVec.isEmpty())
	{
		return QVariant();
	}

	int col = static_cast<int> (SampleColumn::Instance().IndexSampleToStatus(mode, column));
	return dataVec[col];
}

///
///  @brief 更新倒计时
///
///
///  @param[in]   itemId  项目id
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
///
void DataPrivate::UpdateCountDown(int64_t itemId)
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
	if (m_itemShowMap.count(itemId) <= 0)
	{
		return;
	}

	if (m_itemShowMap.at(itemId).isEmpty())
	{
		return;
	}
	if (m_testItemData.count(itemId) <= 0)
	{
		return;
	}

	auto testItem = m_testItemData.at(itemId);
	int col = -1;

	if (testItem->__isset.status && testItem->status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTING)
	{
		if (!testItem->__isset.conc)
		{
			col = static_cast<int> (SampleColumn::COLASSAY::RESULT);
		}
		else
		{
			col = static_cast<int> (SampleColumn::COLASSAY::RECHECKRESULT);
		}
	}

	if (col <0 || col >= m_itemShowMap.at(itemId).size())
	{
		return;
	}

    auto times = m_countDown.getValue(itemId);
	if (!times)
	{
		return;
	}
	m_itemShowMap.at(itemId)[col] = QDataItemInfo::ConvertTime(times.value());
}

///
/// @brief 初始化函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月19日，新建函数
///
void DataPrivate::Init()
{
    m_moduleStatus = MOUDLESTATUS::UNKONW;
	m_needPrintSample = true;
    m_isAssayUpdate = true;
    m_sampleCount = 0;
    m_testItemCount = 0;
    // 初始化为按样本展示
    m_modeType = int(QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT);

    // 监听样本信息
    REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnSampleUpdate);
	// 监听项目的更新
	REGISTER_HANDLER(MSG_ID_SAMPLE_TEST_ITEM_UPDATE, this, OnTestItemUpdate);
	REGISTER_HANDLER(MSG_ID_DICTIONARY_UPDATE, this, OnUpDateTestItemResultFlag);

	// 查询样本数据
	m_sampleFuture = std::async(std::launch::async, [&]()->bool {return Update(); });
	// 查询项目数据
	m_assayFuture = std::async(std::launch::async, [&]()->bool {return UpdateTestItem(); });
    // 完成数据地图
    FinishMap();

    // 递减
    connect(&m_countDown, &MyCountDown::counterDecremented, this, [&]()
    {
        // 按项目展示
        if (m_modeType == int(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE))
        {
            // 更新倒计时数据
			int rowCount = m_testitemPostionVec.size();
            auto map = m_countDown.GetRuntimeMap();
            for (auto iter = map.begin(); iter != map.end(); iter++)
            {
                UpdateCountDown(iter.key());
				auto row = GetTestItemRowByDb(iter.key());
				if (!row)
				{
					continue;
				}

				int index = row.value();
				if (index < 0 || index >= rowCount)
				{
					continue;
				}

				emit sampleChanged(ACTIONTYPE::MODIFYSAMPLE, index, index);
            }
            return;
        }

        emit runAssayTimerChanged();
    });
}

///
/// @brief 打印样本信息
///
/// @param[in]  vSIs  样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月5日，新建函数
///
void DataPrivate::PrintSampleInfo(std::vector<tf::SampleInfo>& vSIs)
{
	ULOG(LOG_INFO, "%s(sample size: %d)", __FUNCTION__, vSIs.size());

	auto spCom = CommonInformationManager::GetInstance();
	if (spCom == Q_NULLPTR)
	{
		return;
	}

	QStringList logInfos;
	for (auto& sampleInfo : vSIs)
	{
		auto barCode = QString::fromStdString(sampleInfo.barcode);
		auto seqCode = QString::fromStdString(sampleInfo.seqNo);
		auto rackCode = QString::fromStdString(sampleInfo.rack);
		auto postion = QString::number(sampleInfo.pos);

		auto sampleLog = QString("sampleInfoId=%1, barCode=%2, seqNo=%3, pos=%4-%5, status=%6")
			.arg(sampleInfo.id).arg(barCode).arg(seqCode).arg(rackCode).arg(postion).arg(QDataItemInfo::GetStatus(sampleInfo));
		logInfos.push_back(sampleLog);
		logInfos.push_back("{");
		for (auto& testItem : sampleInfo.testItems)
		{
			auto assayCode = testItem.assayCode;
			auto status = QDataItemInfo::GetStatus(testItem);
			auto testItemLog = QString("id: %1, assayCode: %2, status: %3, statusCode: %4, samplingCompleted: %5, conc: %6, restestConc: %7, sampleId: %8")
				.arg(testItem.id)
                .arg(assayCode)
				.arg(status)
                .arg(testItem.status)
                .arg(testItem.samplingCompleted)
				.arg(testItem.conc, 0, 'f', 2)
				.arg(testItem.retestConc, 0, 'f', 2)
                .arg(testItem.sampleInfoId);

			logInfos.push_back(testItemLog);
		}
		logInfos.push_back("}");
	}

	auto printLog = logInfos.join("\n").toStdString();
	ULOG_EX(LOG_INFO, "printData: %s", printLog);
}

void DataPrivate::printTestItemInfo(const tf::TestItem& testItem)
{
	ULOG(LOG_INFO, "%s(print testItem id: %d)", __FUNCTION__, testItem.id);
	QStringList logInfos;
	logInfos.push_back("{");
	auto assayCode = testItem.assayCode;
	auto status = QDataItemInfo::GetStatus(testItem);
	auto testItemLog = QString("id: %1, assayCode: %2, status: %3, statusCode: %4, samplingCompleted: %5, conc: %6, restestConc: %7, sampleId: %8")
		.arg(testItem.id)
		.arg(assayCode)
		.arg(status)
		.arg(testItem.status)
		.arg(testItem.samplingCompleted)
		.arg(testItem.conc, 0, 'f', 2)
		.arg(testItem.retestConc, 0, 'f', 2)
		.arg(testItem.sampleInfoId);

	logInfos.push_back(testItemLog);
	logInfos.push_back("}");

	auto printLog = logInfos.join("\n").toStdString();
	ULOG(LOG_INFO, "printItemData: %s", printLog);
}

///
/// @brief 更新模式里面的数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
bool DataPrivate::Update()
{
    // 构造查询条件
    ::tf::SampleInfoQueryCond querySampleCond;
    
    ::tf::LimitCond sampleLimit;
    sampleLimit.offset = 0;
    sampleLimit.count = PAGE_LIMIT_ITEM;
    ::tf::OrderByCond orderByCond;
    orderByCond.__set_asc(true);
    orderByCond.__set_fieldName("id");
    querySampleCond.__set_limit(sampleLimit);
    /*	querySampleCond.__set_containTestItems(true);*/
    querySampleCond.orderByConds.push_back(orderByCond);
    m_moduleStatus = MOUDLESTATUS::PROCESS;
    // 清空样本数据
    m_sampInfoData.clear();
    m_samplePostionVec.clear();
    m_sampleCount = 0;

    do
    {
        // 执行查询条件
		::tf::SampleInfoQueryResp sampleResults;
        if (!DcsControlProxy::GetInstance()->QuerySampleInfo(querySampleCond, sampleResults)
            || sampleResults.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || sampleResults.lstSampleInfos.empty())
        {
            break;
        }

        auto sampleResultVec = sampleResults.lstSampleInfos;
        // 更新数据
        m_readWriteLock.lock();
        //beginResetModel();
        // 在数据之后插入一段数据
        int startRow = m_samplePostionVec.size();
        for (const auto& sample : sampleResultVec)
        {
            m_sampInfoData[sample.id] = std::make_shared<::tf::SampleInfo>(sample);
            m_samplePostionVec.push_back(sample.id);
        }
        int endRow = m_samplePostionVec.size();
        //endResetModel();
        emit sampleChanged(ACTIONTYPE::ADDSAMPLE, startRow, endRow);
        m_readWriteLock.unlock();

        // 查询范围递增
        sampleLimit.offset += sampleResultVec.size();
        sampleLimit.count = PAGE_LIMIT_ITEM;
        querySampleCond.__set_limit(sampleLimit);

    } while (true);

    m_sampleCount = m_samplePostionVec.size();
    return true;
}

///
/// @brief 更新项目信息
///
///
/// @return true获取成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月26日，新建函数
///
bool DataPrivate::UpdateTestItem()
{
    // 构造查询条件
    ::tf::TestItemQueryCond queryTestItemCond;
    ::tf::OrderByCond orderByCond;
    orderByCond.__set_asc(true);
    orderByCond.__set_fieldName("id");
    ::tf::LimitCond limitParameter;
    limitParameter.offset = 0;
    limitParameter.count = PAGE_LIMIT_ITEM * 10;
    queryTestItemCond.__set_limit(limitParameter);
    queryTestItemCond.orderByConds.push_back(orderByCond);
    m_moduleStatus = MOUDLESTATUS::PROCESS;
    m_testItemData.clear();
    m_sampleMap.clear();
    m_testitemPostionVec.clear();
    m_testItemCount = 0;

    do
    {
        // 执行查询条件
		::tf::TestItemQueryResp TestItemResults;
        if (!DcsControlProxy::GetInstance()->QueryTestItems(queryTestItemCond, TestItemResults)
            || TestItemResults.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || TestItemResults.lstTestItems.empty())
        {
            break;
        }

        auto testitemResultVec = TestItemResults.lstTestItems;
        // 更新数据
        m_readWriteLock.lock();
        // 插入项目数据
        for (const auto& item : testitemResultVec)
        {
            m_testItemData[item.id] = std::make_shared<::tf::TestItem>(item);
            m_testitemPostionVec.push_back(item.id);
            // 非计算项目纳入项目测试数目的统计
            if (!CommonInformationManager::IsCalcAssay(item.assayCode))
            {
                m_testItemCount++;
            }
        }
        m_readWriteLock.unlock();

        // 查询范围递增
        limitParameter.offset += testitemResultVec.size();
        limitParameter.count = PAGE_LIMIT_ITEM * 10;
        queryTestItemCond.__set_limit(limitParameter);

    } while (true);

    return true;
}

///
/// @brief 完成数据地图
///
///
/// @return 成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月4日，新建函数
///
bool DataPrivate::FinishMap()
{
    m_sampleFuture.get();
    m_assayFuture.get();

    // 完成样本地图
    for (const auto& item : m_testItemData)
    {
        if (m_sampleMap.count(item.second->sampleInfoId) > 0)
        {
            m_sampleMap[item.second->sampleInfoId].push_back(item.first);
        }
        else
        {
            // 添加
            std::vector<int64_t> testItems;
            testItems.push_back(item.first);
            m_sampleMap[item.second->sampleInfoId] = testItems;
        }
    }
	for (auto & lsi : m_sampInfoData)
	{
		// modify bug2624 by wuht
		if (m_sampleMap.count(lsi.second->id) <= 0)
		{
			m_sampleMap[lsi.second->id] = {};
		}

		m_sampleShowMap[lsi.second->id] = GetSampleShowDatas(*lsi.second);
	}
	for (auto & lti : m_testItemData)
	{
		m_itemShowMap[lti.second->id] = GetItemShowDatas(*lti.second);
	}
    // 初始化完成
    m_moduleStatus = MOUDLESTATUS::FINISH;
    return true;
}

///
///  @brief 创建样本显示数据（一行）
///
///
///  @param[in]   sample  样本信息
///
///  @return	样本信息显示数据列表
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年1月4日，新建函数
///
QVector<QString> DataPrivate::GetSampleShowDatas(const tf::SampleInfo & sample)
{
	QVector<QString> vecShowData;
    vecShowData.reserve(SMAPLE_COLUMN_NUMBER);
	for (int i = 0; i < SMAPLE_COLUMN_NUMBER; ++i)
	{
        switch (SampleColumn::COL(i))
        {
        case SampleColumn::COL::STATUS:
            vecShowData.push_back(QDataItemInfo::GetStatus(sample));
            break;
        case SampleColumn::COL::SEQNO:
            vecShowData.push_back(QDataItemInfo::GetSeqNo(sample));
            break;
        case SampleColumn::COL::BARCODE:
            vecShowData.push_back(QString::fromStdString(sample.barcode));
            break;
        case SampleColumn::COL::TYPE:
            vecShowData.push_back(ThriftEnumTrans::GetSourceTypeName(sample.sampleSourceType));
            break;
        case SampleColumn::COL::POS:
            vecShowData.push_back(QDataItemInfo::GetPos(sample));
            break;
        case SampleColumn::COL::ENDTIME:
            vecShowData.push_back(sample.__isset.endTestTime ? QString::fromStdString(sample.endTestTime) : "-");
            break;
        case SampleColumn::COL::RECHECKENDTIME:
            vecShowData.push_back(sample.__isset.endRetestTime ? QString::fromStdString(sample.endRetestTime) : "-");
            break;
        case SampleColumn::COL::AUDIT:
        {
            QString strAut = (sample.__isset.audit) ? (sample.audit ? QString(tr("已审核")) : QString(tr("未审核"))) : "";
            vecShowData.push_back(strAut);
            break;
        }
        case SampleColumn::COL::PRINT:
        {
            QString strPrt = (sample.__isset.printed) ? (sample.printed ? QString(tr("已打印")) : QString(tr("未打印"))) : "";
            vecShowData.push_back(strPrt);
            break;
        }
        default:
            vecShowData.push_back("");
            break;
        }
	}

	return std::move(vecShowData);
}

///
///  @brief 获取当前行对应的数据
///
///
///  @param[in]   column    列位置信息
///  @param[in]   dataPara  原始数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
///
QString DataPrivate::GetItemData(const int column, const DataPara & dataPara) const
{
	// 获取样本数据
	const auto& data = std::get<0>(dataPara);
	const auto& dataItem = std::get<1>(dataPara);

    auto funcCountdownResult = [&](bool reCheck, const tf::TestItem& testItem, const std::shared_ptr<tf::SampleInfo>& pSample) -> QString{
        QString result = QDataItemInfo::GetAssayResult(reCheck, testItem, pSample);

        // 初次检测没有结果，复查也没有--显示初次倒计时（说明在测试第一次）
        // 只要初检结果存在，就显示复查结果倒计时
        if (QDataItemInfo::IsShowCountDown(reCheck, testItem))
        {
            auto runtimes = GetRuntimesFromId(testItem.id);
            if (runtimes)
            {
                if (runtimes.value() > 0)
                {
                    result = QDataItemInfo::ConvertTime(runtimes.value());
                }
            }
        }
        return result;
    };

	switch (SampleColumn::COLASSAY(column))
	{
	case SampleColumn::COLASSAY::STATUS:
		return QDataItemInfo::GetStatus(dataItem);
	case SampleColumn::COLASSAY::SEQNO:
		return QDataItemInfo::GetSeqNo(*data);
	case SampleColumn::COLASSAY::BARCODE:
		return QString::fromStdString(data->barcode);
	case SampleColumn::COLASSAY::TYPE:
		return ThriftEnumTrans::GetSourceTypeName(data->sampleSourceType);
	case SampleColumn::COLASSAY::POS:
		return QDataItemInfo::GetPos(*data);
	case SampleColumn::COLASSAY::ENDTIME:
    {
        auto endtime = dataItem.__isset.endTime ? dataItem.endTime : "-";
        return QString::fromStdString(endtime);
    }
	case SampleColumn::COLASSAY::RECHECKENDTIME:
    {
        auto reEndTime = dataItem.__isset.retestEndTime ? dataItem.retestEndTime : "-";
        return QString::fromStdString(reEndTime);
    }

	case SampleColumn::COLASSAY::AUDIT:
	{
		if (data->__isset.audit)
		{
            return (data->audit ? QString(tr("已审核")) : QString(tr("未审核")));
		}

		return QString();
	}

	case SampleColumn::COLASSAY::PRINT:
	{
		if (data->__isset.printed)
		{
            return (data->printed ? QString(tr("已打印")) : QString(tr("未打印")));
		}

		return QString();
	}

	case SampleColumn::COLASSAY::ASSAY:
		return CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(dataItem.assayCode);
	case SampleColumn::COLASSAY::RESULT:
		return funcCountdownResult(false, dataItem, data);
	case SampleColumn::COLASSAY::RESULTSTATUS:
    {
        QString strResult = funcCountdownResult(false, dataItem, data);
		auto resultStatusCode = QDataItemInfo::Instance().GetDisplyResCodeVariant(dataItem, strResult.toStdString(), false, data->sampleType);
		return resultStatusCode.toString();
    }
	case SampleColumn::COLASSAY::RECHECKRESULT:
		return funcCountdownResult(true, dataItem, data);
	case SampleColumn::COLASSAY::RECHECKSTATUS:
    {
        QString strResult = funcCountdownResult(true, dataItem, data);
		auto resultStatusCode = QDataItemInfo::Instance().GetDisplyResCodeVariant(dataItem, strResult.toStdString(), true, data->sampleType);
		return resultStatusCode.toString();
    }
	case SampleColumn::COLASSAY::UNIT:
	{
		// 样本显示的设置
		// modify bug0011941 by wuht
		const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		// 显示非浓度的情况下，不需要显示单位
		if (!WorkpageCommon::IsShowConc(sampleSet, data->sampleType))
		{
			return "";
		}

		return GetTestItemUnit(dataItem);
	}
	case SampleColumn::COLASSAY::MODULE:
		return QString::fromStdString(dataItem.__isset.deviceSN ? CommonInformationManager::GetDeviceName(dataItem.deviceSN, dataItem.moduleIndex) : "");
	case SampleColumn::COLASSAY::FIRSTABSORB:
		return QString::fromStdString(dataItem.__isset.RLU ? dataItem.RLU : "");
	case SampleColumn::COLASSAY::REABSORB:
		return QString::fromStdString(dataItem.__isset.retestRLU ? dataItem.retestRLU : "");
	default:
		return QString();
	}
}

///
///  @brief 创建样本显示数据（一行）
///
///
///  @param[in]   sample  样本信息
///
///  @return	样本信息显示数据列表
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年1月4日，新建函数
///
QVector<QString> DataPrivate::GetItemShowDatas(const tf::TestItem & testItem)
{
	QVector<QString> vecShowData;
	for (int i = 0; i < 19; ++i)
	{
		vecShowData.push_back(GetItemShowData(i, testItem));
	}
	return vecShowData;
}

///
/// @brief 获取当前行对应的数据（样本）
///
/// @param[in]  column       位置信息
/// @param[in]  sampleInfo  原始数据
///
/// @return 返回数据
///
/// @par History:
///  @li 7656/zhang.changjiang，2022年12月15日，新建函数
///
QString DataPrivate::GetItemShowData(const int column, const tf::TestItem & dataItem) const
{
    std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
	if (m_sampInfoData.count(dataItem.sampleInfoId) <= 0)
	{
		return QString();
	}

	return GetItemData(column, std::make_pair(m_sampInfoData.at(dataItem.sampleInfoId), dataItem));
}

///
/// @brief 添加样本信息
///
/// @param[in]  samples  样本信息列表
///
/// @return true表示添加成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
bool DataPrivate::AddSamples(const std::vector<::tf::SampleInfo>& samples)
{
    if (samples.empty())
    {
        return true;
    }

    // 需要增加的项目行
    std::vector<int> alterAssayRows;
    // 需要增加的样本行
    std::vector<int> alterSampleRows;
    // 更新数据
    m_readWriteLock.lock();
    // 在数据之后插入一段数据
    for (const auto& sample : samples)
    {
        m_sampInfoData[sample.id] = std::make_shared<::tf::SampleInfo>(sample);
		m_sampleShowMap[sample.id] = GetSampleShowDatas(sample);
        // 样本地图
        m_samplePostionVec.push_back(sample.id);
        // 记录需要增加的样本行
        alterSampleRows.push_back(m_samplePostionVec.size() - 1);
        std::vector<int64_t> itemVec;
        for (const auto& item : sample.testItems)
        {
            // 添加项目
            m_testItemData[item.id] = std::make_shared<::tf::TestItem>(item);
			m_itemShowMap[item.id] = GetItemShowDatas(item);
            // 添加位置
            m_testitemPostionVec.push_back(item.id);
            // 需要增加的项目行
            alterAssayRows.push_back(m_testitemPostionVec.size() - 1);
            // 添加项目信息
            itemVec.push_back(item.id);
            // 非计算项目纳入项目测试数目的统计
            if (!CommonInformationManager::IsCalcAssay(item.assayCode))
            {
                m_testItemCount++;
            }
        }
        m_sampleMap[sample.id] = itemVec;
    }
    m_readWriteLock.unlock();

    // [增加样本/项目]
	ACTIONTYPE mode = ACTIONTYPE::ADDSAMPLE;
    // 若是按项目展示（当前模式）
    if (m_modeType == int(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE))
    {
        for (auto index : alterAssayRows)
        {
            emit sampleChanged(mode, index, index);
        }
    }
    // 若是按样本展示
    else
    {
        for (auto index : alterSampleRows)
        {
            emit sampleChanged(mode, index, index);
        }
    }

    return true;
}

///
/// @brief 删除样本信息
///
/// @param[in]  samples  样本信息列表
///
/// @return true表示删除成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
bool DataPrivate::DelSamples(const std::vector<::tf::SampleInfo>& samples)
{
    if (samples.empty())
    {
        return true;
    }

    // 需要删除的项目行
    std::vector<int> alterAssayRows;
    // 需要删除的样本行
    std::vector<int> alterSampleRows;

    // 更新数据
    m_readWriteLock.lock();
    // 填写数据和行号的对应关系
    for (const auto& sample : samples)
    {
        // 能查到
        auto iterSample = m_sampInfoData.find(sample.id);
        if (iterSample != m_sampInfoData.end())
        {
            // 清除对应的地图中的位置信息
            int row = std::distance(m_sampInfoData.begin(), iterSample);
            if (row >= 0)
            {
                m_samplePostionVec.erase(m_samplePostionVec.begin() + row);
                alterSampleRows.push_back(row);
            }
            // 删除数据
            m_sampInfoData.erase(iterSample);
        }

		// 删除显示数据
		auto iterSampleShow = m_sampleShowMap.find(sample.id);
		if (iterSampleShow != m_sampleShowMap.end())
		{
			m_sampleShowMap.erase(iterSampleShow);
		}

        if (m_sampleMap.count(sample.id) > 0)
        {
            for (const auto& itemid : m_sampleMap[sample.id])
            {
                // 删除项目数据
                auto iterItem = m_testItemData.find(itemid);
                if (iterItem != m_testItemData.end())
                {
                    // 清除对应的地图中的位置信息
                    int row = std::distance(m_testItemData.begin(), iterItem);
                    if (row >= 0)
                    {
                        m_testitemPostionVec.erase(m_testitemPostionVec.begin() + row);
                        // 非计算项目纳入项目测试数目的统计
                        if (!CommonInformationManager::IsCalcAssay(iterItem->second->assayCode))
                        {
                            m_testItemCount--;
                        }
                        alterAssayRows.push_back(row);
                    }
                    m_testItemData.erase(iterItem);
                }

				// 删除显示数据
				auto iterItemShow = m_itemShowMap.find(itemid);
				if (iterItemShow != m_itemShowMap.end())
				{
					m_itemShowMap.erase(iterItemShow);
				}
            }

            auto iterSampleMap = m_sampleMap.find(sample.id);
            if (iterSampleMap != m_sampleMap.end())
            {
                m_sampleMap.erase(iterSampleMap);
            }
        }
    }
    m_readWriteLock.unlock();

    // [删除样本/项目]
	ACTIONTYPE mode = ACTIONTYPE::DELETESAMPLE;
    // 若是按项目展示（当前模式）
    if (m_modeType == int(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE))
    {
        for (auto index : alterAssayRows)
        {
            emit sampleChanged(mode, index, index);
        }
    }
    // 若是按样本展示
    else
    {
        for (auto index : alterSampleRows)
        {
            emit sampleChanged(mode, index, index);
        }
    }

    return true;
}

///
/// @brief 修改样本信息
///
/// @param[in]  samples  样本信息
///
/// @return true表示修改成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
bool DataPrivate::ModifySamples(const std::vector<::tf::SampleInfo>& samples)
{
    if (samples.empty())
    {
        return true;
    }

    std::vector<int> addAssayRows;      // 需要增加的项目行   
    std::vector<int> removeAssayRows;   // 需要删除的项目行
    std::vector<int> alterAssayRows;    // 需要改变的项目行    
    std::vector<int> alterSampleRows;   // 需要改变的样本行

    // 更新数据
    m_readWriteLock.lock();
    for (const ::tf::SampleInfo& sample : samples)
    {
        if (m_sampInfoData.count(sample.id) > 0)
        {
            // 修改样本数据
            m_sampInfoData[sample.id] = std::make_shared<::tf::SampleInfo>(sample);
			m_sampleShowMap[sample.id] = GetSampleShowDatas(sample);

            // 1）要求有此样本数据，要求开启项目更新
            if (m_isAssayUpdate && m_sampleMap.count(sample.id) > 0)
            {
                // 修改或者新增项目信息
                auto& itemVec = m_sampleMap[sample.id];
                for (const tf::TestItem& item : sample.testItems)
                {
                    // 新增项目信息
                    if (std::find(itemVec.begin(), itemVec.end(), item.id) == itemVec.end())
                    {
                        itemVec.push_back(item.id);
                        m_testitemPostionVec.push_back(item.id);
                        // 非计算项目纳入项目测试数目的统计
                        if (!CommonInformationManager::IsCalcAssay(item.assayCode))
                        {
                            m_testItemCount++;
                        }
                        // 添加需要增加的项目行
                        addAssayRows.push_back(m_testitemPostionVec.size() - 1);
                    }
                    else
                    {
                        auto iterItem = m_testItemData.find(item.id);
                        if (iterItem != m_testItemData.end())
                        {
                            // 清除对应的地图中的位置信息
                            int row = std::distance(m_testItemData.begin(), iterItem);
                            if (row >= 0)
                            {
                                // 当前行的数据需要更新
                                alterAssayRows.push_back(row);
                            }
                        }
                    }
                    m_testItemData[item.id] = std::make_shared<::tf::TestItem>(item);
					m_itemShowMap[item.id] = GetItemShowDatas(item);
                }

                // 获取需要删除
                std::vector<int64_t> delVec;
                for (const auto& itemId : itemVec)
                {
                    auto iter = std::find_if(sample.testItems.begin(), sample.testItems.end(), [&](auto& item)->bool {
                        return(itemId == item.id); });
                    if (iter == sample.testItems.end())
                    {
                        delVec.push_back(itemId);
                    }
                }

                // 执行需要删除
                for (const auto& del : delVec)
                {
                    auto iter = std::find(itemVec.begin(), itemVec.end(), del);
                    if (iter != itemVec.end())
                    {
                        // 删除项目
                        itemVec.erase(iter);
                        // 删除项目数据
                        auto iterTestItem = m_testItemData.find(del);
                        if (iterTestItem != m_testItemData.end())
                        {
                            // 清除对应的地图中的位置信息
                            int row = std::distance(m_testItemData.begin(), iterTestItem);
                            if (row >= 0)
                            {
                                m_testitemPostionVec.erase(m_testitemPostionVec.begin() + row);
                                // 非计算项目纳入项目测试数目的统计
                                if (!CommonInformationManager::IsCalcAssay(iterTestItem->second->assayCode))
                                {
                                    m_testItemCount--;
                                }
                                removeAssayRows.push_back(row);
                            }
                            m_testItemData.erase(iterTestItem);
                        }

						// 删除显示数据
						auto iterTestItemShow = m_itemShowMap.find(del);
						if (iterTestItemShow != m_itemShowMap.end())
						{
							m_itemShowMap.erase(iterTestItemShow);
						}
                    }
                }
            }
        }

        auto indexValue = GetSampleByDb(sample.id);
        if (indexValue)
        {
            alterSampleRows.push_back(indexValue.value());
        }
    }
    m_readWriteLock.unlock();

    // 项目处理
    if (m_modeType == int(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE))
    {
        // 删除总行数
        auto removeCount = removeAssayRows.size();
        // 增加的行数
        for (auto addRow : addAssayRows)
        {
            int adjustRow = addRow - removeCount;
            emit sampleChanged(ACTIONTYPE::ADDSAMPLE, adjustRow, adjustRow);
        }

        // 改变的行数
        for (auto alterRow : alterAssayRows)
        {
            // 不在更新范围内，不更新，提高效率
			/*
            if (!m_rangVisble.IsRangIn(alterRow))
            {
                continue;
            }*/

            int adjustRow = alterRow - removeCount;
            emit sampleChanged(ACTIONTYPE::MODIFYSAMPLE, adjustRow, adjustRow);
        }

        // 删除的行数
        for (int removeRow : removeAssayRows)
        {
            emit sampleChanged(ACTIONTYPE::DELETESAMPLE, removeRow, removeRow);
        }

    }
    // 样本处理
    else
    {
        // 改变的行数(样本)
        for (int alterRow : alterSampleRows)
        {
            emit sampleChanged(ACTIONTYPE::MODIFYSAMPLE, alterRow, alterRow);
        }
    }

    return true;
}

bool DataPrivate::ModifyTestItem(const std::vector<::tf::TestItem>& testItems)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(testItems));
	std::set<int64_t>  alertItems;
	for (const auto& itemData : testItems)
	{
		auto iter = m_testItemData.find(itemData.id);
		if (iter == m_testItemData.end())
		{
			continue;
		}

		// 打印项目信息
		if (m_needPrintSample)
		{
			printTestItemInfo(itemData);
		}

		// 记录更新的项目的id信息
		alertItems.insert(itemData.id);
		// 更新项目信息
		iter->second = std::make_shared<::tf::TestItem>(itemData);		
	}

	// 项目处理
	if (m_modeType == int(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE))
	{
		if (alertItems.empty())
		{
			return true;
		}

		// 改变的行数
		for (auto item : alertItems)
		{
			auto rowOption = GetTestItemRowByDb(item);
			if (!rowOption.has_value())
			{
				continue;
			}

			int row = rowOption.value();
			// 不在更新范围内，不更新，提高效率		
// 			if (!m_rangVisble.IsRangIn(row))
// 			{
// 				continue;
// 			}

			// 更新行
			emit sampleChanged(ACTIONTYPE::MODIFYSAMPLE, row, row);
		}
	}

	// 更新数据
	emit testItemChanged(alertItems);
	return true;
}

bool DataPrivate::UpdateTestItemData(const ::tf::TestItem& srcData, std::shared_ptr<::tf::TestItem> dstData)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (dstData == Q_NULLPTR)
	{
		ULOG(LOG_INFO, "%s(Update testItem id: %d failed, the dst is null)", __FUNCTION__, srcData.id);
		return false;
	}

	dstData = std::make_shared<::tf::TestItem>(srcData);
	return true;
}

///
/// @brief 样本信息更新
///
/// @param[in]  enUpdateType  更新类型
/// @param[in]  vSIs		  更新的样本信息数组
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
void DataPrivate::OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs)
{
    ULOG(LOG_INFO, "%s(%d,)", __FUNCTION__, int(enUpdateType));

    // 如果样本信息模型还未初始化，则不处理样本更新消息
    if (m_moduleStatus == MOUDLESTATUS::UNKONW)
    {
        ULOG(LOG_ERROR, "m_moduleStatus == UNKONW");
        return;
    }

    // 等待样本初始化完毕以后再处理消息
    if (m_moduleStatus == MOUDLESTATUS::PROCESS)
    {
        ULOG(LOG_INFO, "m_enInitStatus == INIT_STATUS_PROCESS");
        // 等待数据查询完毕，在进行下一步动作（为完成的时候，会阻塞)
        m_sampleFuture.get();
        m_assayFuture.get();
    }

    // 根据样本更新类型分别处理
    switch (enUpdateType)
    {
        case ::tf::UpdateType::type::UPDATE_TYPE_ADD:
        {
            AddSamples(vSIs);
        }
            break;
        case ::tf::UpdateType::type::UPDATE_TYPE_DELETE:
        {
            DelSamples(vSIs);
            return;
        }
            break;
        case ::tf::UpdateType::type::UPDATE_TYPE_MODIFY:
        {
			if (m_needPrintSample)
			{
				PrintSampleInfo(vSIs);
			}

            ModifySamples(vSIs);
        }
            break;
        default:
            break;
    }
}

void DataPrivate::OnTestItemUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::TestItem, std::allocator<tf::TestItem>> vSIs)
{
	ULOG(LOG_INFO, "%s(type: %d, size: %d)", __FUNCTION__, int(enUpdateType), vSIs.size());
	// 如果样本信息模型还未初始化，则不处理样本更新消息
	if (m_moduleStatus == MOUDLESTATUS::UNKONW)
	{
		ULOG(LOG_ERROR, "m_moduleStatus == UNKONW");
		return;
	}

	// 等待样本初始化完毕以后再处理消息
	if (m_moduleStatus == MOUDLESTATUS::PROCESS)
	{
		ULOG(LOG_INFO, "m_enInitStatus == INIT_STATUS_PROCESS");
		// 等待数据查询完毕，在进行下一步动作（为完成的时候，会阻塞)
		m_sampleFuture.get();
		m_assayFuture.get();
	}

	// 根据样本更新类型分别处理
	switch (enUpdateType)
	{
		case ::tf::UpdateType::type::UPDATE_TYPE_ADD:
		{
			ULOG(LOG_WARN, "TestItem don't allow add the testItem");
			return;
		}
		break;
		case ::tf::UpdateType::type::UPDATE_TYPE_DELETE:
		{
			ULOG(LOG_WARN, "TestItem don't allow delete the testItem");
			return;
		}
		break;
		case ::tf::UpdateType::type::UPDATE_TYPE_MODIFY:
		{
			ModifyTestItem(vSIs);
		}
		break;
		default:
			break;
	}
}

///
/// @brief 当结果提示变化以后，需要同等更新按项目展示的结果信息的flag
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月15日，新建函数
///
void DataPrivate::OnUpDateTestItemResultFlag()
{
	std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
	for (auto testItem : m_testItemData)
	{
		// 没有结果信息，直接不处理
		if (!testItem.second->__isset.conc)
		{
			continue;
		}

		auto data = DataPrivate::Instance().GetSampleInfoByDb(testItem.second->sampleInfoId);
		if (data == Q_NULLPTR)
		{
			continue;
		}

		auto iter = m_itemShowMap.find(testItem.first);
		if (iter == m_itemShowMap.end())
		{
			continue;
		}

		int firstResult = static_cast<int> (SampleColumn::COLASSAY::RESULT);
		QString result = QDataItemInfo::GetAssayResult(false, *(testItem.second), data);
		iter->second[firstResult] = result;

		// 查看需要处理复查结果否
		if (!testItem.second->__isset.retestConc)
		{
			continue;
		}

		int lastResult = static_cast<int> (SampleColumn::COLASSAY::RECHECKRESULT);
		QString checkresult = QDataItemInfo::GetAssayResult(true, *(testItem.second), data);
		iter->second[lastResult] = checkresult;
	}
}

QSampleDetailModel& QSampleDetailModel::Instance()
{
    static QSampleDetailModel model;
    return model;
}

void QSampleDetailModel::SetRange(int first, int last)
{
    m_assayVisble.SetRange(first, last);
}

std::shared_ptr<tf::TestItem> QSampleDetailModel::GetCurrentItem(int row)
{
    if (row >= m_testItems.size() || row < 0)
    {
        return Q_NULLPTR;
    }

    return Q_NULLPTR;
}

///
/// @brief 初始化显示顺序表
///
///
/// @return true初始化成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月1日，新建函数
///
bool QSampleDetailModel::InitDisplayOrder()
{
    OrderAssay data;
    if (!DictionaryQueryManager::GetAssayShowOrder(data))
    {
        ULOG(LOG_ERROR, "Failed to get orderassay config.");
        return false;
    }

	m_showOrder.clear();
    // 使用自定义的排序
    if (data.enableCustom)
    {
        int postion = 0;
        for (auto order : data.orderVec)
        {
            m_showOrder.insert(std::make_pair(order, postion));
            postion++;
        }
    }
	// 使用默认顺序
	else
	{
		m_showOrder = CommonInformationManager::GetInstance()->GetAssayDefaultMap();
	}

    return true;
}

bool QSampleDetailModel::SortItem(std::vector<std::shared_ptr<tf::TestItem>>& testItems, std::vector<int64_t>& dbs, std::map<int, int>& showOrder)
{
    if (dbs.size() == 0)
    {
        return true;
    }

	if (showOrder.size() == 0)
	{
		showOrder = m_showOrder;
	}

    auto cmp = [&](std::shared_ptr<tf::TestItem> a, std::shared_ptr<tf::TestItem> b)
    {
		// 特殊情况，都不在序列中的情况(按照通道号来排序，小的排在前面， Bug0011854 by wuht)
		if (showOrder.count(a->assayCode) <= 0 && showOrder.count(b->assayCode) <= 0)
		{
			return (a->assayCode < b->assayCode) ? true : false;
		}

        if (showOrder.count(a->assayCode) <= 0)
        {
            return false;
        }

        if (showOrder.count(b->assayCode) <= 0)
        {
            return true;
        }

        if (showOrder[a->assayCode] < showOrder[b->assayCode])
        {
            return true;
        }

        // 编号相同的时候，看id号，id号越小则优先级越高
        if (showOrder[a->assayCode] == showOrder[b->assayCode])
        {
			// 当计算项目的位置号跟非计算项目的位置号相同的时候，计算项目的优先级更高
			// 当计算项目位置号跟计算项目位置号相同的情况，未做规定(相同的时候，按照数据库主键来排, bug0012241 by wuht)
			bool isaCalc = CommonInformationManager::IsCalcAssay(a->assayCode);
			bool isbCalc = CommonInformationManager::IsCalcAssay(b->assayCode);
			if (!(isaCalc && isbCalc))
			{
				return isaCalc ? true : false;
			}

            return (a->id < b->id) ? true : false;
        }

        return false;
    };

	auto isNeedShow = [](std::shared_ptr<tf::TestItem>& spTestItme)->bool
	{
		if (spTestItme == Q_NULLPTR)
		{
			return false;
		}

		// 判断是否计算项目
		bool isCalc = false;
		if (spTestItme->assayCode >= tf::AssayCodeRange::CALC_RANGE_MIN
			&& spTestItme->assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
		{
			isCalc = true;
		}

		// modify bug0010707;具体需求是，当项目编号找不到对应的项目信息的时候，不显示
		// 计算项目(计算项目有标志，确定是否显示)
		if (isCalc)
		{
			auto spCalcAssay = CommonInformationManager::GetInstance()->GetCalcAssayInfo(spTestItme->assayCode);
			if (spCalcAssay == Q_NULLPTR)
			{
				ULOG(LOG_ERROR, "Failed to get calcassayinfo by assayCode:%d.", spTestItme->assayCode);
				return false;
			}

			return spCalcAssay->display;
		}
		// 普通项目(普通项目当查询不到对应的项目信息的时候，不显示)modify bug0010707
		else
		{
			auto spCalcAssay = CommonInformationManager::GetInstance()->GetAssayInfo(spTestItme->assayCode);
			if (spCalcAssay == Q_NULLPTR)
			{
				ULOG(LOG_ERROR, "Failed to get assayInfo by assayCode:%d.", spTestItme->assayCode);
				return false;
			}

			return true;
		}
	};

	// 根据项目db获取项目详细信息
	auto GetTestItemFromDb = [&](int db)->std::shared_ptr<tf::TestItem>
	{
		auto iter = DataPrivate::Instance().m_testItemData.find(db);
		if (iter == DataPrivate::Instance().m_testItemData.end())
		{
			ULOG(LOG_INFO, "%s(Can not find testItem: %d)", __FUNCTION__, db);
			return Q_NULLPTR;
		}

		if (!isNeedShow(iter->second))
		{
			ULOG(LOG_INFO, "%s(not need show: %d)", __FUNCTION__, db);
			return Q_NULLPTR;
		}

		return std::make_shared<tf::TestItem>(*(iter->second));
	};

    for (const auto testItenDb : dbs)
    {
		auto needItem = GetTestItemFromDb(testItenDb);
		if (needItem == Q_NULLPTR)
		{
			continue;
		}

        if (testItems.empty())
        {
            testItems.push_back(needItem);
            continue;
        }

        int insertPos = 0;
        bool isSet = false;
        for (const auto& item : testItems)
        {
            if (cmp(needItem, item))
            {
				//m_assayPostionMap[needItem->id] = insertPos;
                auto iter = testItems.begin() + insertPos;
                testItems.insert(iter, needItem);
                isSet = true;
                break;
            }

            insertPos++;
        }

        if (!isSet)
        {
            testItems.push_back(needItem);
        }
    }

	int pos = 0;
	for (const auto& testItem : testItems)
	{
		m_assayPostionMap[testItem->id] = pos++;
	}

    return true;
}

void QSampleDetailModel::SetData(const boost::optional<tf::SampleInfo>& spSampleInfo, tf::TestMode::type sampleTestMode)
{
    DataPrivate::Instance().ClearRuntimeList();
	m_sampleTestMode = sampleTestMode;
	m_assayPostionMap.clear();
    // 清空显示页面，当设置样本数据是空的时候
    if (!spSampleInfo)
    {
        this->beginResetModel();
        m_testItems.clear();
        this->endResetModel();
        return;
    }

//     if (m_currentSample && spSampleInfo 
//         && m_currentSample->id == spSampleInfo->id)
//     {
//         return;
//     }

    this->beginResetModel();
    m_testItems.clear();

    m_currentSample = spSampleInfo;
    if (!spSampleInfo)
    {
        this->endResetModel();
        return;
    }

    auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(spSampleInfo->id);
    if (!testItems)
    {
        this->endResetModel();
        return;
    }

	auto testItmeValues = testItems.value();
    SortItem(m_testItems, testItems.value(), m_showOrder);
    this->endResetModel();

    // 初始化刷新行
    auto rowCount = m_testItems.size();
    if (rowCount > 15)
    {
        m_assayVisble.SetRange(0, 15);
    }
    else
    {
        m_assayVisble.SetRange(0, rowCount);
    }
}

bool QSampleDetailModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if ((role == (Qt::UserRole + 1))
        && (index.column() == 0)
        && (m_currentSample.has_value()))
    {
        auto row = index.row();
        int size = m_testItems.size();
        if (row < 0 || row >= size)
        {
            return false;
        }

        auto spTestItem = m_testItems[row];
        if (spTestItem == Q_NULLPTR)
        {
            return false;
        }

		if (spTestItem->status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED
			&& spTestItem->rerun == false)
		{
			spTestItem->__set_rerun(true);
			spTestItem->__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
			this->beginResetModel();
			this->endResetModel();
			// 通知数据浏览页面有项目被选中，一边更新复查按钮的状态
			emit selectStatusChanged();
		}
		// 取消复查(根据最新需求，勾选的时候，需要确认，取消直接取消-20240125与周晓峰和胡荣沟通)
		else
		{		
			::tf::SampleInfoQueryCond sampQryCond;
			::tf::SampleInfoQueryResp sampQryResp;
			sampQryCond.__set_id(m_currentSample.value().id);
			// 查询数据的时候，把对应的项目信息页查询出来
			sampQryCond.__set_containTestItems(true);
			// 执行查询条件
			if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
				|| sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
				|| sampQryResp.lstSampleInfos.empty())
			{
				return false;
			}

			// 根据数据库主键来确定对应的项目
			bool findIten = false;
			for (auto& testItem : sampQryResp.lstSampleInfos[0].testItems)
			{
				if (testItem.id == spTestItem->id)
				{
					// 取消复查(复查状态，直接取消复查)
					if (WorkpageCommon::IsItemReCheckStatus(testItem))
					{
						testItem.__set_rerun(false);
						testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED);
						// 取消复查需要将复查的稀释倍数设置回上一次的稀释倍数
						DataPrivate::Instance().ResetTestItemDulition(testItem);
						findIten = true;
					}

					spTestItem->__set_rerun(false);
					spTestItem->__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED);

					break;
				}
			}

			// 若没有找到对应的项目，无需做修改
			if (!findIten)
			{
				this->beginResetModel();
				this->endResetModel();
				return false;
			}

			tf::SampleInfo sampleInfo;
			sampleInfo.__set_testItems(sampQryResp.lstSampleInfos[0].testItems);
			sampleInfo.__set_id(m_currentSample.value().id);
			// 更新样本信息
			if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
			{
				return false;
			}
		}
    }

    return QAbstractTableModel::setData(index, value, role);
}

std::map<int64_t, std::shared_ptr<tf::TestItem>> QSampleDetailModel::GetTestSelectedItems()
{
	std::map<int64_t, std::shared_ptr<tf::TestItem>> selectMap;
	for (auto item : m_testItems)
	{
		if (item == Q_NULLPTR)
		{
			continue;
		}

		if (!WorkpageCommon::IsItemReCheckStatus(*item))
		{
			continue;
		}
		selectMap[item->id] = item;
	}

	return selectMap;
}

bool QSampleDetailModel::IsEnableRecheck(boost::optional<tf::SampleInfo> sampleValue, const tf::TestItem& dataItem) const
{
	// 是否是已经选择了复查的项目
	auto isRechecked = [](const tf::TestItem& dataItem)->bool
	{
		return (dataItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING && dataItem.rerun == true);
	};

	// 是否是已经完成的项目
	auto isFinished = [](const tf::TestItem& dataItem)->bool
	{
		return (dataItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED && dataItem.rerun == false);
	};

	if (!sampleValue)
	{
		return false;
	}

	auto sampleInfo = sampleValue.value();

	// 若是在不同的模式下不允许修改
	if (m_sampleTestMode != sampleInfo.testMode)
	{
		return false;
	}

	// 1:若样本正在测试，不能做选择
	if (sampleInfo.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
	{
		return false;
	}

	// 2:计算项目不能复查
	bool isCalc = CommonInformationManager::IsCalcAssay(dataItem.assayCode);
	if (isCalc)
	{
		return false;
	}

	// 3：已经审核的样本不能复查
	if (sampleInfo.__isset.audit && sampleInfo.audit)
	{
		return false;
	}

	// 4: 非病人样本不能复查（校准质控不能复查）
	if (sampleInfo.sampleType != tf::SampleType::SAMPLE_TYPE_PATIENT)
	{
		return false;
	}

	// 特殊: 已经点了复查的项目, 可以直接取消
	if ((isRechecked(dataItem)))
	{
		return true;
	}

	// 5: 项目既不是已经完成的项目, 不允许复选框使能
	if (!(isFinished(dataItem)))
	{
		return false;
	}

	// 6：判断当前样本有无相同的项目，若有，是否已经有相同的项目已经被选中
	if (IsAssayCodeSetReChecked(dataItem, m_testItems))
	{
		return false;
	}

	return true;
}


bool QSampleDetailModel::IsAssayCodeSetReChecked(const tf::TestItem& testItemData, std::vector<std::shared_ptr<tf::TestItem>> testItems) const
{
	auto isRechecked = [](tf::TestItem& dataItem)->bool
	{
		return (dataItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING && dataItem.rerun == true);
	};

	for (const auto& testItem : testItems)
	{
		if (testItem == Q_NULLPTR)
		{
			continue;
		}

		if (testItem->id == testItemData.id)
		{
			continue;
		}

		if (testItem->assayCode != testItemData.assayCode)
		{
			continue;
		}

		if (isRechecked(*testItem))
		{
			return true;
		}
	}

	return false;
}

Qt::ItemFlags QSampleDetailModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QSampleDetailModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_testItems.size();
}

int QSampleDetailModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_hearData.size();
}

QVariant QSampleDetailModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const int column = index.column();

    if (row < 0 || row >= m_testItems.size()
        || column < 0 || column >= m_hearData.size())
    {
        return QVariant();
    }

	// 结果提示
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();
    // 获取对应行的数据
	if (nullptr == m_testItems[row])
	{
		return QVariant();
	}
    auto dataItem = *(m_testItems[row]);
    auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(dataItem.sampleInfoId);
    if (spSampleInfo == Q_NULLPTR)
    {
        return QVariant();
    }

    // 确定是否在测试中
    QSampleAssayModel::Instance().StartTestTimeCountDown(dataItem);
    // 获取数据
    if (role == Qt::DisplayRole) {

        return QDataItemInfo::Instance().GetOnlyAssay(index, std::make_pair(spSampleInfo, dataItem));
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    // 额外数据
    else if (role == (Qt::UserRole + 1)) {

        // 第一列代表复选框(是否选中)
        if (column == COL_DATAMODE_CHECK)
        {
            if (dataItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING
                && dataItem.rerun == true)
            {
                return true;
            }

            return false;
        }

        // 稀释结果
		auto columnValue = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLEASSAY, int(SampleColumn::SAMPLEASSAY::ASSAY));
        if (columnValue && columnValue.value().first == column)
        {
			// 是否计算项目(计算项目不显示稀释倍数)
			bool isCalc = CommonInformationManager::IsCalcAssay(dataItem.assayCode);
			if (isCalc)
			{
				return QVariant();
			}

			return QDataItemInfo::GetDiluTionShow(dataItem);
        }

		const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (!WorkpageCommon::IsShowConc(sampleSet, spSampleInfo->sampleType))
		{
			return QVariant();
		}

		// 检测结果和复查检测结果的值的状态
		int type = QDataItemInfo::Instance().ResultRangeStatus(dataItem, column, true).toInt();
		// 若没有异常，需要再次判断
		if (ColorType::NORMALCOLOR == type)
		{
			auto status = QDataItemInfo::Instance().ResultDangerStatus(dataItem, column, true);
			if (!status.has_value())
			{
				return type;
			}

			if (status.value())
			{
				return ColorType::UPPERCOLOR;
			}
			else
			{
				return ColorType::LOWERCOLOR;
			}
		}

		return type;
    }
    // 复选框是否使能
    else if (role == (Qt::UserRole + 2)) {
		return IsEnableRecheck(m_currentSample, dataItem);
    }
	else if (role == (Qt::UserRole + 5)) {

#if 0 // 应需求和工业设计需求去掉数据报警的背景色
		auto AlarmBackGroudColor = [&](tf::TestItem& testItem, bool isRecheck)->QVariant
		{
			auto resultStatusCode = ResultStatusCodeManager::GetInstance()->GetChemistryDisplayResCode(dataItem, isRecheck);
			if (!resultStatusCode.isEmpty())
			{
				return QVariant("#FFFF99");
			}

			return QVariant();
		};

		// 数据告警
		auto columnValue = QDataItemInfo::Instance().GetAttributeByType(QDataItemInfo::SAMPLEMODE::SAMPLEASSAY, int(QDataItemInfo::SAMPLEASSAY::RESULTSTATUS));
		// 复查数据告警
		auto lastColumnValue = QDataItemInfo::Instance().GetAttributeByType(QDataItemInfo::SAMPLEMODE::SAMPLEASSAY, int(QDataItemInfo::SAMPLEASSAY::RECHECKSTATUS));
		// 初测
		if (columnValue && columnValue.value().first == index.column())
		{
			return AlarmBackGroudColor(dataItem, false);
		}
		// 复查
		else if (lastColumnValue && lastColumnValue.value().first == index.column())
		{
			return AlarmBackGroudColor(dataItem, true);
		}
#endif

		const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (!WorkpageCommon::IsShowConc(sampleSet, spSampleInfo->sampleType))
		{
			return QVariant();
		}

		//modify bug0012239  by wuht
		auto status = QDataItemInfo::Instance().ResultDangerStatus(dataItem, column, true);
		if (!status.has_value())
		{
			return QVariant();
		}

		if (status.value())
		{
			return QColor("#FFDAD6");
		}
		else
		{
			return QColor("#E5F2FF");
		}
		}

    return QVariant();
}

QVariant QSampleDetailModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section < 0 || section >= m_hearData.size())
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_hearData.at(section);
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QSampleDetailModel::QSampleDetailModel()
{
        m_currentSample = boost::none;
        m_hearData << tr("复查") << tr("项目") << tr("检测结果") <<
        tr("结果状态") << tr("复查结果") << tr("结果状态") << tr("单位") << tr("模块");
        //m_hearData = QDataItemInfo::Instance().GetHeaders(2);
		m_hearData = SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLEASSAY);
        // 初始化显示顺序
        InitDisplayOrder();
        // 注册项目更新消息
        REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, InitDisplayOrder);
        REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, InitDisplayOrder);

		// 更新项目状态信息
		connect(&DataPrivate::Instance(), &DataPrivate::testItemChanged, this, [&](std::set<int64_t> updateDbs)
		{
			if (m_testItems.empty() || updateDbs.empty())
			{
				return;
			}

			for (const auto db : updateDbs)
			{
				auto iter = m_assayPostionMap.find(db);
				if (iter == m_assayPostionMap.end())
				{
					continue;
				}

				int postion = iter->second;
				// 数据改变，更新对应的信息
				if (postion < 0 || postion >= m_testItems.size())
				{
					continue;
				}

				auto spTestItem = DataPrivate::Instance().GetTestItemInfoByDb(db);
				if (spTestItem == Q_NULLPTR)
				{
					continue;
				}

				m_testItems[postion] = std::make_shared<tf::TestItem>(*spTestItem);
				auto indexstart = this->index(postion, 0);
				auto indexend = this->index(postion, this->columnCount() - 1);
				emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
			}

		});

        connect(&DataPrivate::Instance(), &DataPrivate::sampleChanged, this, [&](DataPrivate::ACTIONTYPE mode, int start, int end)
        {
            // 按样本展示的时候，才能更新(正在运行也不能修改信息)
            if(DataPrivate::Instance().m_modeType != int(QSampleAssayModel::VIEWMOUDLE::DATABROWSE))
            {
                return;
            }

            switch (mode)
            {
				case DataPrivate::ACTIONTYPE::MODIFYSAMPLE:
                {
                    // 当前没有样本
                    if (!m_currentSample)
                    {
                        return;
                    }

                    auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(m_currentSample->id);
                    if (!testItems)
                    {
                        return;
                    }

                    auto& testItemValue = testItems.value();
                    auto nowsize = testItemValue.size();
                    auto oldSize = m_testItems.size();
                    m_testItems.clear();
                    if (nowsize > 0)
                    {
                        SortItem(m_testItems, testItemValue, m_showOrder);
                    }

                    auto indexstart = this->index(0, 0);
                    // 增加了行数
                    if (nowsize > oldSize)
                    {
                        beginInsertRows(indexstart.parent(), oldSize, nowsize -1);
                        endInsertRows();
                    }
                    // 减少
                    else if (nowsize < oldSize)
                    {
                        beginRemoveRows(indexstart.parent(), nowsize, oldSize-1);
                        endRemoveRows();
                    }
                }
                break;

                case DataPrivate::ACTIONTYPE::DELETESAMPLE:
                {
                    auto indexstart = this->index(start, 0);
                    if (!indexstart.isValid())
                    {
                        return;
                    }
                }
                break;

                default:
                    break;
            }
        });

        // 数据更新
        connect(&SampleColumn::Instance(), &SampleColumn::ColumnChanged, this, [&]()
        {
            // 更新信息
			m_hearData = SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLEASSAY);
            this->beginResetModel();
            this->endResetModel();
        });

        // 刷新倒计时
        connect(&DataPrivate::Instance(), &DataPrivate::runAssayTimerChanged, this, [&]()
        {
            if (m_testItems.size() < 1)
            {
                return;
            }

            auto currentRange = m_assayVisble.GetRange();
            auto indexstart = this->index(currentRange.first, 0);
            auto indexend = this->index(currentRange.second, this->columnCount() - 1);
            emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
        });
}

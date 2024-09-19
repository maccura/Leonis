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
/// @file     alarmModel.h
/// @brief    报警信息列表模型
///
/// @author   7951/LuoXin
/// @date     2023年1月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年1月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QPainter>
#include "alarmModel.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/common/TimeUtil.h"
#include "src/public/ConfigDefine.h"
#include "manager/DictionaryQueryManager.h"

#define DELEGATE_COLOR_OFFSET_POS           5               // 只读单元格代理中，颜色数据的存储位置相对于Qt::UserRole的偏移
#define DELEGATE_PICTURE_OFFSET_POS         6               // 报警码未读标记图片存储位置相对于Qt::UserRole的偏移

AlarmModel::AlarmModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ tr("模块")
	, tr("报警代码")
	, tr("报警级别")
	, tr("报警名称")
	, tr("报警时间")}
{
}

AlarmModel::~AlarmModel()
{
}

AlarmModel& AlarmModel::Instance()
{
	static AlarmModel model(nullptr);
	return model;
}

void AlarmModel::SetData(const std::vector<std::shared_ptr<AlarmRowItem>>& data)
{
	beginResetModel();
	m_data = data;
	endResetModel();
}

void AlarmModel::AppendData(std::shared_ptr<AlarmRowItem> item)
{
    int start = m_data.size();
    beginInsertRows(index(start, 0).parent(), start, start);
    m_data.insert(m_data.begin(),std::move(item));
    endInsertRows();
}


int AlarmModel::GetCurrentAlarmLevel()
{
    int level = 0;
    for (const auto& ad : m_data)
    {
        // 获取当前未处理的最高的报警级别
        if (!ad->isDeal && ad->level > level )
        {
            level = ad->level;
        }

        if (level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
        {
            return level;
        }
    }

    return level;
}

void AlarmModel::RemoveRowByDevSn(QStringList snList)
{
    beginResetModel();

    if (snList.contains(tr("全部")))
    {
        m_data.clear();
    } 
    else
    {
        for (auto iter = m_data.begin(); iter != m_data.end();)
        {
            if (snList.contains(QString::fromStdString((*iter)->devSn)))
            {
                iter = m_data.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }

    endResetModel();
}

void AlarmModel::SetAlarmIsDealByDevSn(const std::string& devSn)
{
    for (auto& item : m_data)
    {
        if (item->devSn == devSn
            && item->level > ::tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION)
        {          
            item->isDeal = true;
        }
    }

    m_listStopDevice.remove(devSn);
    if (!m_listStopDevice.empty())
    {
        return;
    }

    // 设置“用户手动点击停止”已经处理，报警码93-3-1
    for (auto& item : m_data)
    {
        if (item->mainCode == USER_STOP_ALARM_MAINCODE
            && item->middleCode == USER_STOP_ALARM_MIDCODE
            && item->subCode == USER_STOP_ALARM_SUBCODE)
        {
            item->isDeal = true;
        }
    }      
}

bool AlarmModel::IsExistReagentAlarmBySn(const std::string& devSn)
{
    for (auto& item : m_data)
    {
        if ((item->devSn == devSn || devSn.empty())
            && !item->isDeal 
            && item->unreadFlag
            && item->mainCode == REAGENT_ALARM_MAINCODE
            && item->middleCode == REAGENT_ALARM_MIDCODE)
        {
             return true;
        }
    }

    return false;
}

const std::vector<std::shared_ptr<AlarmModel::AlarmRowItem>>& AlarmModel::GetData()
{
    return m_data;
}

std::shared_ptr<AlarmModel::AlarmRowItem> AlarmModel::GetDataByIndex(const QModelIndex &parent)
{
    int row = parent.row();
    if (row >= m_data.size() || row < 0)
    {
        return nullptr;
    }

    return  m_data[row];
}

int AlarmModel::DeleteData(const tf::AlarmDesc& ad)
{
    beginResetModel();

    // 在缓存中删除指定的报警
    // 取消报警的功能只用于试剂、耗材的余量不足的告警
    // 此类告警应该是去重的，界面上始终只显示一条
    int row = 0;
    for (auto iter = m_data.begin(); iter != m_data.end(); iter++)
    {
        row++;
        if ( (*iter)->devSn == ad.deviceSN && (*iter)->devType == ad.deviceType
            && (*iter)->mainCode == ad.mainCode && (*iter)->middleCode == ad.middleCode
            && (*iter)->subCode == ad.subCode && (*iter)->subName.toStdString() == ad.subModuleName)
        {
            m_data.erase(iter);
            break;
        }
    }

    endResetModel();

    return row;
}

void AlarmModel::UpdateAllData()
{
    beginResetModel();
    endResetModel();
}

void AlarmModel::UpdateStopDeviceList(std::string devSn)
{
    bool isPipe = DictionaryQueryManager::GetInstance()->GetPipeLine();
    if (devSn.empty())
    {
        m_listStopDevice.clear();
        for (auto dev : CIM_INSTANCE->GetDeviceMaps())
        {
            if (!isPipe && dev.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
            {
                continue;
            }

            m_listStopDevice.push_back(dev.first);
        }
    }
}

int AlarmModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int AlarmModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant AlarmModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole && section < m_headerNames.size() && section >= 0) {
			return m_headerNames.at(section);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant AlarmModel::data(const QModelIndex& index, int role) const
{
	const QVariant null;

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

	// 获取当前显示列
	const int colIdx = index.column();
	const int rowIdx = index.row();

    // 获取选中行的数据
    if (rowIdx >= m_data.size() || rowIdx < 0)
    {
        return null;
    }
    auto& rowData = m_data[rowIdx];

    if (role == Qt::UserRole + DELEGATE_PICTURE_OFFSET_POS && colIdx == COLUNM_CODE && rowData->unreadFlag)
    {
        return QPixmap(":/Leonis/resource/image/alarm-red-point.png");
    }

    // 屏蔽显示灰色字体
    if (role == Qt::TextColorRole && rowData->isShield && rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION)
    {
        return QBrush(UI_SHIELD_WARNCOLOR);
    }

    // 字体颜色
    if (role == Qt::TextColorRole && colIdx == COLUNM_LEVEL)
    {
        if (rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
        {
            // 停机显示红色字体
            return QBrush(UI_REAGENT_WARNFONT);
        }
        else if(rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING
            || (rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION
                && rowData->mainCode == REAGENT_ALARM_MAINCODE
                && rowData->middleCode == REAGENT_ALARM_MIDCODE))
        {
            // 加样停显示黄色字体
            // 试剂耗材类的报警是注意级别，但是显示黄色(此类报警的报警码99-21-xxx)
            return QBrush(UI_ALARM_YELLOW_WARNFONT);
        }
    }

    // 背景色
    if (role == Qt::UserRole + DELEGATE_COLOR_OFFSET_POS && colIdx == COLUNM_LEVEL)
    {
        if (rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
        {
            // 停机显示红色背景
            return UI_REAGENT_WARNCOLOR;
        }
        else if (rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING
             || (rowData->level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION
                && rowData->mainCode == REAGENT_ALARM_MAINCODE
                && rowData->middleCode == REAGENT_ALARM_MIDCODE))
        {
            // 加样停显示黄色背景
            // 试剂耗材类的报警是注意级别，但是显示黄色(此类报警的报警码99-21-xxx)
            return UI_ALARM_YELLOW_WARNCOLOR;
        }
    }

	// 只处理显示角色
	if (role != Qt::DisplayRole )
	{
		return null;
	}

	switch (colIdx)
	{
		// 行号
	case COLUNM_MODEL:
        return rowData->subName.isEmpty() ? rowData->devName  : rowData->devName + "-" + rowData->subName;
		// 项目名
	case COLUNM_CODE:
		return  UiCommon::AlarmCodeToString(rowData->mainCode, rowData->middleCode, rowData->subCode);
	case COLUNM_LEVEL:
		return ConvertTfEnumToQString((tf::AlarmLevel::type)rowData->level);
	case COLUNM_DESCRIBE:
		return QString::fromStdString(rowData->describe);
	case COLUNM_TIME:
		return QString::fromStdString(PosixTimeToTimeString(boost::posix_time::from_iso_string(rowData->time)));
	default:
		break;
	}

	return null;
}

void QAlarmCodeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    // 计算文本长度、高度
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    QFontMetrics fontMetrics(painter->font());
    int texWidth = fontMetrics.width(text);
    int textHight = fontMetrics.height();

    // 计算图片位置
    auto& rt = option.rect;
    int x = rt.right() - ((rt.width() - texWidth) / 2);
    int y = rt.top() + ((rt.height() - textHight) / 2);

    auto flag = index.data(Qt::UserRole + DELEGATE_PICTURE_OFFSET_POS);
    if (flag.isValid())
    {
        // 绘制标志
        if (flag.canConvert<QPixmap>())
        {
            auto flagPix = flag.value<QPixmap>();
            painter->drawPixmap(x, y, flagPix);

        }
    }

    QStyleOptionViewItem optionNew = option;
    auto fontColorType = index.model()->data(index, Qt::TextColorRole);
    if (fontColorType.isValid())
    {
        if (fontColorType.canConvert<QColor>())
        {
            const auto& color = fontColorType.value<QColor>();
            optionNew.palette.setColor(QPalette::HighlightedText, color);
            optionNew.palette.setColor(QPalette::Text, color);
        }
    }

    QStyledItemDelegate::paint(painter, optionNew, index);
}

AlarmModel::AlarmRowItem::AlarmRowItem(const tf::AlarmDesc& ad, const QString& name)
{
    devName         = name;
    subName         = QString::fromStdString(ad.subModuleName);
    mainCode        = ad.mainCode;
    middleCode      = ad.middleCode;
    subCode         = ad.subCode;
    devType         = ad.deviceType;
    level           = ad.alarmLevel;
    describe        = ad.alarmName;
    time            = ad.alarmTime;
    detail          = ad.alarmDetail;
    customSolution  = ad.customSolution;
    managerSolution = ad.managerSolution;
    devSn           = ad.deviceSN;
    isVisible       = ad.isVisible;
    isDistinct      = ad.isDistinct;
    unreadFlag      = true;
    isShield        = false;
    isDeal          = false;
}

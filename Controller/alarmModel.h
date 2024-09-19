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
#pragma once
#include <array>
#include <memory>
#include <QStyledItemDelegate>
#include <QAbstractTableModel>

#define USER_STOP_ALARM_MAINCODE            93              // 用户手动暂停的报警码的主报警码
#define USER_STOP_ALARM_MIDCODE             3               // 用户手动暂停的报警码的中报警码
#define USER_STOP_ALARM_SUBCODE             1               // 用户手动暂停的报警码的次报警码

namespace tf {
    class AlarmDesc;
};

class AlarmModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	///
	/// @brief
	///     报警信息在表格中的对应列
	///
	enum DATA_COLUNM
	{
		COLUNM_MODEL,					// 模块
		COLUNM_CODE,					// 报警码
		COLUNM_LEVEL,					// 报警等级
		COLUNM_DESCRIBE,				// 描述
		COLUNM_TIME,					// 时间
	};

	///
	/// @brief
	///     显示数据结构
	///
	struct AlarmRowItem
	{
        QString     devName;                        // 设备名称
        QString     subName;                        // 子模块名称
        std::string	devSn;						    // 设备序列号
        int         devType;                        // 设备类型
		int32_t		mainCode;						// 故障码主分类
		int32_t		middleCode;						// 故障码中分类
		int32_t		subCode;						// 故障码子分类
		int32_t		level;							// 报警级别
		std::string	describe;						// 描述
		std::string	time;							// 时间
		std::string detail;							// 详情
		std::string customSolution;					// 用户解决方案
        std::string managerSolution;				// 工程师解决方案
        bool        isVisible;                      // 是否可见
        bool        isDistinct;                     // 是否去重
        bool        unreadFlag;                     // 消息未读标志
        bool        isShield;                       // 是否被屏蔽
        bool        isDeal;                         // 是否已处理
       
        AlarmRowItem() 
            : mainCode(-1)
            , devType(-1)
            , middleCode(-1)
            , subCode(-1)
            , level(-1)
            , isVisible(true)
            , isDistinct(false)
            , unreadFlag(true)
            , isShield(false)
            , isDeal(false)
        {}

        AlarmRowItem(const tf::AlarmDesc& ad, const QString& name);

		bool operator==(const AlarmRowItem& rhs)
		{
			return devSn == rhs.devSn
                && devType == rhs.devType
				&& mainCode == rhs.mainCode
				&& middleCode == rhs.middleCode
				&& subCode == rhs.subCode;
		}
	};

private:
	AlarmModel(QObject *parent);

public:
	virtual ~AlarmModel();

public:
	///
	/// @brief
	///     获取单例
	///
	/// @return 单例对象
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	static AlarmModel& Instance();

	///
	/// @brief
	///     设置数据到模型
	///
	///@param[in]    data 显示数据
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	void SetData(const std::vector<std::shared_ptr<AlarmRowItem>>& data);

	///
	/// @brief
	///     追加显示数据到模型
	///
	/// @param[in]  data  
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	void AppendData(std::shared_ptr<AlarmRowItem> item);

    ///
    /// @brief
    ///     获取当前最高报警级别
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月15日，新建函数
    ///
    int GetCurrentAlarmLevel();

	///
	/// @brief 清空所有行
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	void RemoveRowByDevSn(QStringList snList);

    ///
    /// @brief 设置报警已处理
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月31日，新建函数
    ///
    void SetAlarmIsDealByDevSn(const std::string& devSn);

    ///
    /// @brief 指定设备是否存在试剂耗材类报警
    ///
    /// @param[in]  devSn  设备序列号，空字符串表示全部设备
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年7月12日，新建函数
    ///
    bool IsExistReagentAlarmBySn(const std::string& devSn = "");

	///
	/// @brief
	///     获取数据
	///
	/// @return 被显示的数据
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	const std::vector<std::shared_ptr<AlarmRowItem>>& GetData();

	///
	/// @brief
	///     获取数据
	///
	/// @return 被显示的数据
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
    std::shared_ptr<AlarmRowItem> GetDataByIndex(const QModelIndex &parent);

    ///
    /// @brief
    ///     删除指定的行
    ///
    /// @return 被删除的行号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月18日，新建函数
    ///
    int DeleteData(const tf::AlarmDesc& ad);

    ///
    /// @brief	更新界面显示
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月12日，新建函数
    ///
    void UpdateAllData();

    ///
    /// @brief	更新设备停止队列
    ///
    /// @param[in]  devSn  设备序列号，空字符串表示新增全部设备，非空表示移除指定设备
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月12日，新建函数
    ///
    void UpdateStopDeviceList(std::string devSn = "");

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:                                                         		

	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList                     m_headerNames;              /// 表头名称集合
	std::vector<std::shared_ptr<AlarmRowItem>>	m_data;         /// 显示的数据
    std::list<std::string>        m_listStopDevice;             /// 用户手动点击停止的仪器列表
};

// 报警码未读代理
class QAlarmCodeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ///
    /// @brief 报警码未读代理delegate
    ///
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月12日，新建函数
    ///
    QAlarmCodeDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;
};

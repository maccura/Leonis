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
#pragma once
#include <QAbstractTableModel>
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <boost/optional.hpp>

class QAlarmReagentSetModel :
	public QAbstractTableModel
{
	Q_OBJECT

public:

    typedef struct ReagentInfo
    {
        boost::optional<::tf::GeneralAssayInfo> generalAssayInfo;
        boost::optional<ch::tf::SupplyAttribute> supplyAttributeCh;
        boost::optional<im::tf::SupplyAttribute> supplyAttributeIm;

        // 常规项目
        ReagentInfo(const ::tf::GeneralAssayInfo& assay)
        {
            generalAssayInfo = boost::make_optional(assay);
        }

        // 生化仓内耗材
        ReagentInfo(const ch::tf::SupplyAttribute& attribute)
        {
            supplyAttributeCh = boost::make_optional(attribute);
        }

        //免疫仓内耗材
        ReagentInfo(const im::tf::SupplyAttribute& attribute)
        {
            supplyAttributeIm = boost::make_optional(attribute);
        }

        void set_bottleAlarmThreshold(int32_t bottle)
        {
            if (supplyAttributeCh)
            {
                supplyAttributeCh->__set_bottleAlarmThreshold(bottle);
            }
            if (supplyAttributeIm)
            {
                supplyAttributeIm->__set_bottleAlarmThreshold(bottle);
            }
            if (generalAssayInfo)
            {
                generalAssayInfo->__set_bottleAlarmThreshold(bottle);
            }
        }

        void set_assayAlarmThreshold(int32_t assay)
        {
            if (supplyAttributeCh)
            {
                supplyAttributeCh->__set_assayAlarmThreshold(assay);
            }
            if (supplyAttributeIm)
            {
                supplyAttributeIm->__set_assayAlarmThreshold(assay);
            }
            if (generalAssayInfo)
            {
                generalAssayInfo->__set_assayAlarmThreshold(assay);
            }
        }
    }ReagentInfo;

	using ShowData = std::pair<bool, ReagentInfo>;

	enum class COL
	{
		//SEQNO = 0,						/// 序号
		MODEL = 0,                      /// 模块名称
		NAME,							/// 项目名称
        BOTTLEWARNING,					/// 瓶注意级别
		ASSAYWARNING,					/// 项目注意级别
		UNIT,					        /// 单位
	};

	virtual ~QAlarmReagentSetModel();

	///
	/// @brief 获取单例对象
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	static QAlarmReagentSetModel& Instance();

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
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	///
	/// @brief 更新模式里面的数据
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	void Update();

	///
	/// @brief 保存报警参数
	///
	/// @param[out]    hasChangeParam    参数是否改变
	/// @param[out]    changeSupply      是否改变了耗材的信息
	///
	/// @return true表示保存成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月1日，新建函数
	///
	bool SaveAlarmParameter(bool &hasChangeParam, bool &changeSupply);

    ///
    /// @brief	当前界面是否有未保存的数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月14日，新建函数
    ///
    bool isExistChangedData();

protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QAlarmReagentSetModel();

private:
	QStringList                     m_headerNames;              /// 表头名称集合
	std::vector<ShowData>		    m_data;					    ///< 数据列表
};

class QAlarmWasteSetModel :
    public QAbstractTableModel
{
    Q_OBJECT

public:

    using SupplyStatus = std::pair<bool, im::tf::SupplyAttribute>;

    enum class COL
    {
        MODEL = 0,                      /// 模块名称
        NAME,							/// 项目名称
        WARNING,					    /// 报警值
        UNIT,					        /// 单位
    };

    virtual ~QAlarmWasteSetModel() {};

    ///
    /// @brief 获取单例对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    static QAlarmWasteSetModel& Instance();

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
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    ///
    /// @brief 更新模式里面的数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    void Update();

    ///
    /// @brief 保存报警参数
    ///
    ///
    /// @return true表示保存成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月1日，新建函数
    ///
    bool SaveAlarmParameter();

    ///
    /// @brief	当前界面是否有未保存的数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月14日，新建函数
    ///
    bool isExistChangedData();

protected:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QAlarmWasteSetModel();

private:
    QStringList                     m_headerNames;              ///< 表头名称集合
    int                             m_softType;                 ///< 软件类型
    int                             m_chWasteWarning;           ///< 生化废液桶的报警值
    std::vector<SupplyStatus>		m_supplyAttributes;			///< 耗材属性列表
    int                             m_chWasteNum;               ///< 生化废料/废液数
};


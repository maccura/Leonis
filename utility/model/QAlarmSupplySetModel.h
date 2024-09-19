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
#pragma once
#include <QAbstractTableModel>
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include <boost/optional.hpp>

class QAlarmSupplySetModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	typedef struct SupplyAttribute
	{
		boost::optional<ch::tf::SupplyAttribute> supplyAttributeCh;
		boost::optional<ise::tf::SupplyAttribute> supplyAttributeISE;
        boost::optional<im::tf::SupplyAttribute> supplyAttributeIm;
		// 生化
		SupplyAttribute(const ch::tf::SupplyAttribute& attribute)
		{
			supplyAttributeCh = boost::make_optional(attribute);
		}

		// ISE
		SupplyAttribute(const ise::tf::SupplyAttribute& attribute)
		{
			supplyAttributeISE = boost::make_optional(attribute);
		}

        //免疫
        SupplyAttribute(const im::tf::SupplyAttribute& attribute)
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
			if(supplyAttributeISE)
			{
				supplyAttributeISE->__set_bottleAlarmThreshold(bottle);
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
                //免疫没有在耗材使用项目报警，暂时使用耗材报警容量
                supplyAttributeIm->__set_assayAlarmThreshold(assay);
            }
			if(supplyAttributeISE)
			{
				supplyAttributeISE->__set_assayAlarmThreshold(assay);
			}
		}
	}SupplyAttribute;

	using SupplyStatus = std::pair<bool, SupplyAttribute>;
	enum class COL
	{
		MODEL = 0,                      /// 模块名称
		NAME,							/// 项目名称
		ASSAYWARNING,					/// 项目注意级别(ml)
        UNIT                            /// 单位
	};

	virtual ~QAlarmSupplySetModel();

	///
	/// @brief 获取模式的单例
	///
	///
	/// @return 单例
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月5日，新建函数
	///
	static QAlarmSupplySetModel& Instance();

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
	///
	/// @return true表示保存成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月1日，新建函数
	///
	bool SaveAlarmParameter(bool &hasChangeParam);

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
	QAlarmSupplySetModel();

private:
    int                             m_softType;                 ///< 软件版本
	QStringList                     m_headerNames;              ///< 表头名称集合
	std::vector<SupplyStatus>		m_supplyAttributes;			///< 耗材属性列表
};

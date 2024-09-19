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
/// @file     MaintainGroupCfgModel.h
/// @brief 	  维护项目组配置模型
///
/// @author   7656/zhang.changjiang
/// @date      2022年9月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QAbstractTableModel>
#include <src/thrift/gen-cpp/defs_types.h>

class MaintainGroupCfgModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	// 显示数据
	struct ItemData
	{
		ItemData() :repetition(1), hasOtherCfg(false) {}
		
		int				 repetition;	// 重复次数
		bool			 hasOtherCfg;	// 是否有参数
		tf::MaintainItem item;			// 维护项
	};
private:
	MaintainGroupCfgModel(QObject *parent = Q_NULLPTR);

public:
	virtual ~MaintainGroupCfgModel();

	///
	///  @brief 获取单例
	///
	///
	///
	///  @return	单例对象
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月16日，新建函数
	///
	static MaintainGroupCfgModel& Instance();

	///
	///  @brief	设置维护项目数据
	///
	///
	///  @param[in]   data  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
	///
	void SetData(const std::vector<tf::MaintainItem> & data);

	///
	///  @brief	设置维护项目数据
	///
	///
	///  @param[in]   data  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
	///
	std::vector<tf::MaintainItem> GetData();

	///
	/// @brief  获取选中的维护项目
	///
	/// @param[in]  iRow  行号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年2月1日，新建函数
	///
	::tf::MaintainItem GetSelectedItemByRow(const int iRow);

	///
	///  @brief 获取行数
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
    int GetRows() { return m_showData.size(); };

	///
	///  @brief 将from位置的元素移动到to位置
	///
	///
	///  @param[in]   from  移动的初始位置
	///  @param[in]   to	移动的目标位置
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
	void MoveItem(int from, int to);
	
	///
	///  @brief 复制一个维护项到指定inde
	///
	///
	///  @param[in]   index  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void CopyItemByIndex(const QModelIndex &index);
	
	///
	///  @brief 删除指定index的维护项
	///
	///
	///  @param[in]   index  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void DelItemByIndex(const QModelIndex &index);

	///
	///  @brief 获取指定index的维护项
	///
	///  @param[in]   index  需要获取该维护项的索引
	///  @param[in]   itemData  用于返回的维护项信息
	///
	///  @return	true:获取成功
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
	///
	bool GetItemByIndex(const QModelIndex &index, MaintainGroupCfgModel::ItemData& itemData);

	///
	///  @brief 修改指定index的维护项
	///
	///  @param[in]   index  需要获取该维护项的索引
	///  @param[in]   itemData  修改的维护项信息
	///
	///  @return	true:修改成功
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
	///
	bool ModifyItemByIndex(const QModelIndex &index, const MaintainGroupCfgModel::ItemData& itemData);
	
	///
	///  @brief 修改制定index的维护项重复次数
	///
	///
	///  @param[in]   index  需要获取该维护项的索引
	///  @param[in]   repetition  重复次数
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
	///
	bool ModifyItemByIndex(const QModelIndex &index, int repetition);

	///
	///  @brief 全部清除
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void ClearData();

public slots:

	///
	///  @brief 配置数据
	///
	///
	///  @param[in]   isChecked    选中状态
	///  @param[in]   item		   配置的维护项
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
	///
	void ConfigData(bool isChecked, const tf::MaintainItem &item);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
protected:
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::DropActions supportedDropActions() const override;
	QMimeData *mimeData(const QModelIndexList &indexes) const override;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
private:

	///
	///  @brief 将from位置的元素移动到to位置
	///
	///
	///  @param[in]   array  数组
	///  @param[in]   from	 移动的初始位置
	///  @param[in]   to	 移动的目标位置
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
	///
	void Move(std::vector<ItemData> & array, int from, int to);
	
	///
	///  @brief 将维护项列表映射为显示数据列表
	///
	///
	///  @param[in]   data  维护项列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
	///
	std::vector<MaintainGroupCfgModel::ItemData> MapDataToShowData(const std::vector<tf::MaintainItem>& data);
	
	///
	///  @brief 将显示数据列表映射为维护项列表
	///
	///
	///  @param[in]   shwoData  显示数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
	///
	std::vector<tf::MaintainItem> MapShowDataToData(const std::vector<MaintainGroupCfgModel::ItemData>& shwoData);
	
	///
	///  @brief 获取显示字符串
	///
	///
	///  @param[in]   itemData  显示数据
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
	///
	QString GetShowStr(const MaintainGroupCfgModel::ItemData& itemData) const;
private:
	QStringList									m_headerNames;              // 表头名称集合
	std::vector<ItemData>						m_showData;                 // 显示的数据
};

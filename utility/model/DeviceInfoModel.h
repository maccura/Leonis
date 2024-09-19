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
/// @file     DeviceInfoModel.h
/// @brief 	  维护项目组模型
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

class DeviceInfoModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum MAINTAIN_GROUP_COL
	{
		INDEX = 0,                  // 序号
		NAME = 1					// 维护组项目组名
	};
	typedef struct DEVICE_INFO_DATA
	{
		DEVICE_INFO_DATA() : iCheckState(Qt::Unchecked) {}
		int iCheckState;
		tf::DeviceInfo dev;
	}DeviceInfoData;
private:
	DeviceInfoModel(QObject *parent = Q_NULLPTR);
public:
	virtual ~DeviceInfoModel();

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
	static DeviceInfoModel& Instance();

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
	void SetData(std::vector<tf::DeviceInfo> & data);

    ///
    ///  @brief 设置维护组选中状态(多选)
    ///
    ///
    ///  @param[in]   row          维护组所在行
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年10月12日，新建函数
    ///
    void SetCheckedState(int row);

	///
	///  @brief 获取选中的设备序列号
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
	///
	std::vector<std::string> GetSelectDevSN();

	///
	///  @brief 获取选中的设备
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
	///
	std::vector<tf::DeviceInfo> GetSelectDevs();
signals:
	
	///
	///  @brief	复选框状态改变信号
	///
	///
	///  @param[in]   iCheckState  状态
	///  @param[in]   item		   维护项
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
	///
	void CheckStateChanged(int iCheckState, tf::DeviceInfo item);
protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList									m_headerNames;              // 表头名称集合
	std::vector<DeviceInfoData>					m_data;                     // 显示的数据
};

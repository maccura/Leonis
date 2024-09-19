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
/// @file     QIseCaliAppManager.h
/// @brief    Ise校准申请页面数据管理
///
/// @author   8090/YeHuaNing
/// @date     2022年12月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QObject>
#include "src/thrift/ise/gen-cpp/ise_types.h"

class QStandardItem;


class QIseCaliAppManager : public QObject
{
    Q_OBJECT

    // 存储自定义数据
    enum UserDataRoles {
        ReagentInfo = 2,        // 试剂信息
        DatabaseId,             // 数据库id
        AssayClassify,          // 项目分类
        DefaultSort = 6         // 默认排序
    };

public:
    QIseCaliAppManager();
    ~QIseCaliAppManager();

    ///
    /// @brief 获取ise设备的校准申请数据
    ///
    /// @param[in]  devices  设备信息
    ///
    /// @return 表格列表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    QList<QList<QStandardItem*>> GetIseCaliItems(const std::vector<std::string>& devices);

	///
	/// @brief 获取ise设备的校准申请数据
	///        函数重载，可以根据样本类型获取
	///
	/// @param[in]  devices     设备信息
	/// @param[in]  sampleType  样本类型
	///
	/// @return 表格列表
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月3日，新建函数
	///
	QList<QList<QStandardItem*>> GetIseCaliItems(const std::vector<std::string>& devices, const ::ise::tf::SampleType::type& sampleType);

    ///
    /// @brief 更新校准状态信息
    ///
    /// @param[in]  infos  变化的模块信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月3日，新建函数
    ///
    void UpdateCaliStatus(std::vector<ise::tf::IseModuleInfo>& infos);

    ///
    /// @brief 清除数据（当校准申请表ISE相关数据被清空时，应调用此函数）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月17日，新建函数
    ///
    void OnClearTableModel() { items.clear(); }

private:
    ///
    /// @brief 返回设备列表对应的ise模块信息
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @return 模块信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    std::vector<::ise::tf::IseModuleInfo> GetIseModuleInfos(const std::vector<std::string>& devices);

	///
	/// @brief 返回设备列表对应的ise模块信息
	///        函数重载，可以根据样本类型获取
	///
	/// @param[in]  devices     设备列表
	/// @param[in]  sampleType  样本类型
	///
	/// @return 模块信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月3日，新建函数
	///
	std::vector<::ise::tf::IseModuleInfo> GetIseModuleInfos(const std::vector<std::string>& devices, const ::ise::tf::SampleType::type& sampleType);

    ///
    /// @brief 获取位置列数据
    ///
    /// @param[in]  info  模块信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    QStandardItem* GetItemPos(const ::ise::tf::IseModuleInfo& info);

    ///
    /// @brief 返回样本类型列数据
    ///
    /// @param[in]  type  样本类型
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    QStandardItem* GetItemName(const ::ise::tf::SampleType::type& type);

    ///
    /// @brief 获取使用状态
    ///
    /// @param[in]  info  模块信息
    ///
    /// @return 使用状态
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    QStandardItem* GetModuleStatus(const ::ise::tf::IseModuleInfo& info);

    ///
    /// @brief 获取曲线的有限期
    ///
    /// @param[in]  info  模块信息
    ///
    /// @return 当前曲线有效期
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    QStandardItem* GetValidDays(const ::ise::tf::IseModuleInfo& info);

    ///
    /// @brief 对模块信息排队
    ///
    /// @param[in]  infos  模块信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月3日，新建函数
    ///
    void SortIseModuleInfos(std::vector<ise::tf::IseModuleInfo>& infos);

private:
    QList<QList<QStandardItem*>> items;
};

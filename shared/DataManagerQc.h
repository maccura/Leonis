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
/// @file     DataManagerQc.h
/// @brief    质控数据暂存,以处理频繁访问查询接口去获取质控数据的
///
/// @author   8276/huchunli
/// @date     2023年5月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年5月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _INCLUDE_DATAMANAGERQC_H_
#define _INCLUDE_DATAMANAGERQC_H_

#include <mutex>
#include <set>
#include <vector>
#include <QObject>
#include <boost/noncopyable.hpp>
#include "src/thrift/gen-cpp/defs_types.h"


class DataManagerQc: public QObject, public boost::noncopyable
{
    Q_OBJECT
public:
    static std::shared_ptr<DataManagerQc> GetInstance();

    ~DataManagerQc();

    ///
    /// @bref
    ///		获取所有质控品的ID
    ///
    /// @return qcDocIds 返回的质控品ID列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月26日，新建函数
    ///
    inline std::set<int64_t>& GetQcDocIDs() { return m_qcDocIDs; };

    ///
    /// @bref
    ///		获取除ISE外所有的质控品名称
    ///
    /// @param[out] qcDocNames 输出的质控品名称
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月23日，新建函数
    ///
    void GetQcDocNames(std::vector<QString>& qcDocNames);

    ///
    /// @bref
    ///		获取所有的质控品项
    ///
    /// @param[out] qcDocs 质控品列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月23日，新建函数
    ///
    void GetQcDocs(std::vector<std::shared_ptr<tf::QcDoc>>& qcDocs);

    ///
    /// @bref
    ///		获取成分信息中包含指定项目的质控文档
    ///
    /// @param[out] qcDocs 找到的质控文档
    /// @param[in] assayCode 指定的项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月28日，新建函数
    ///
    void GetQcDocs(std::vector<std::shared_ptr<tf::QcDoc>>& qcDocs, int assayCode);

	///
	/// @brief 根据数据ID获取质控文档（现用于修改位置时调用此函数）
	///
	/// @param[in]  id  质控文档的数据库ID
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月18日，新建函数
	///
	std::shared_ptr<tf::QcDoc> GetQcDocByID(const int64_t id);

protected Q_SLOTS:

    ///
    /// @bref
    ///		质控品更新信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月26日，新建函数
    ///
    void OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos);

private:
    DataManagerQc();

    // 当前类的单例对象
    static std::shared_ptr<DataManagerQc> s_instance; ///< 单例对象
    static std::mutex m_dataqcMutx;

    ///
    /// @bref
    ///		初始化质控暂存数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月26日，新建函数
    ///
    void ReloadQcData();

private:
    bool m_qcDocChanged;                    // 质控数据是否存在更新
    std::set<int64_t> m_qcDocIDs;           // 所有质控品的id
    std::map<int64_t, std::shared_ptr<tf::QcDoc>> m_mapDocs;  // <id, qcdoc>

    std::vector<QString> m_qcDocNamesNoISE; // 不包含ISE项目的所有质控品名
};


#endif // _INCLUDE_DATAMANAGERQC_H_

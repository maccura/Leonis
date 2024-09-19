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
/// @file     OperationLogManager.h
/// @brief    操作日志管理类，单例对象
///
/// @author   1556/Chenjianlin
/// @date     2023年7月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <boost/noncopyable.hpp>
#include <memory>
#include <mutex>
#include <QObject>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

///
/// @brief 操作日志管理类单例对象
///
class COperationLogManager : public QObject, public boost::noncopyable
{
	Q_OBJECT

public:
	///
	/// @brief 获取单例对象
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年7月29日，新建函数
	///
	static std::shared_ptr<COperationLogManager> GetInstance();

	///
	/// @brief 
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年7月29日，新建函数
	///
	~COperationLogManager();

	///
	/// @brief 添加操作日志到数据库
	///
	/// @param[in]  eOptType  操作类型（添加、修改）
	/// @param[in]  newDoc   新质控文档
	/// @param[in]  spQcDoc  旧质控文档
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年7月29日，新建函数
	///
	bool AddQCOperationLog(::tf::OperationType::type eOptType, const tf::QcDoc& newDoc, const std::shared_ptr<tf::QcDoc> spQcDoc = nullptr);

	///
	/// @brief 获取靶值、标准差变更内容
	///
	/// @param[in]  newDocUi 界面设置的数据  
	/// @param[in]  spQcDoc 之前的数据  
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月15日，新建函数
	///
	std::string GetSdChangeOptLogstr(const ::tf::QcDoc& newOcDoc, const std::shared_ptr<::tf::QcDoc> spQcDoc);

	///
	/// @brief 添加免疫校准操作日志
	///
	/// @param[in]  eOptType  操作类型（添加、删除）
	/// @param[in]  caliDocGroup  校准文档
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年12月14日，新建函数
	///
	bool AddCaliOperationLog(::tf::OperationType::type eOptType, const im::tf::CaliDocGroup& caliDocGroup);

	///
	/// @brief 添加参考范围的操作日志
	///
	/// @param[in]  eOptType  操作类型
	/// @param[in]  strAssay  项目名称
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年12月14日，新建函数
	///
	bool AddRangeParamOperationLog(::tf::OperationType::type eOptType, const std::string strAssay);

	///
	/// @brief 添加免疫测试项目的操作日志
	///
	/// @param[in]  eOptType  操作类型
	/// @param[in]  seqNo  样本号
	/// @param[in]  barcode  样本条码
	/// @param[in]  assayCode  测试项目
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年12月15日，新建函数
	///
	bool AddTestItemOperationLog(::tf::OperationType::type eOptType, const std::string& seqNo, const std::string& barcode, int assayCode);

	///
	/// @brief 添加项目的操作日志（这里只处理添加项目的操作日志，修改和参数已经在AssayConfigWidget对应的函数中做了相关操作）
	///
	/// @param[in]  spAssayInfoBefore	项目信息
	/// @param[in]  spAssayInfoAfter	项目信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年9月11日，新建函数
	///
	bool AddGeneralAssayInfoLog(std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoBefore, std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoAfter);

private:
	///
	/// @brief 
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年7月29日，新建函数
	///
	COperationLogManager(QObject *parent = Q_NULLPTR);

	///
	/// @brief  日志接口中需要设备编号，随便取一个设备
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年12月14日，新建函数
	///
	std::string GetDeviceSn() const;	
	
	// 当前类的单例对象
	static std::shared_ptr<COperationLogManager> s_instance;
	static std::mutex m_mutxMgrMutx;
};


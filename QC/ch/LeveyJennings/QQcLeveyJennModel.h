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
/// @file     QQcLeveyJennModel.h
/// @brief    Level Jennings页面Model
///
/// @author   8090/YeHuaNing
/// @date     2022年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QStandardItemModel>
#include "src/thrift/gen-cpp/defs_types.h"


class QQcProjectModel : public QStandardItemModel
{
	Q_OBJECT

public:
	enum class COLDOC
	{
        SELECT = 0,             //选择
        DOCNO,                  //质控品编号
        DOCNAME,                //质控品名称
        DOCTYPE,                //质控品类型
        DOCLEVEL,               //质控品水平
        DOCLOT,                 //质控品批号
        TARGETVALUE,            //靶值
        SD,                     //SD
        CV,                     //CV%
        COMPUTEVALUE,           //计算靶值
        COMPUTESD,              //计算SD
        COMPUTECV,              //计算CV%
        COUNT                  //数量
	};

	virtual ~QQcProjectModel();

	///
	/// @brief 获取单例对象
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年11月14日，新建函数
	///
	static QQcProjectModel& Instance();

public slots:
	///
	/// @brief 更新模式里面的数据
	///
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年11月14日，新建函数
	///
    bool Update(const std::string& device, int assayCode, const QString& startDate, const QString& endDate);

    ///
    /// @brief 初始化质控
    ///
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void Init();

    ///
    /// @brief 获取计算靶值是否可用
    ///
    /// @param[in]  index  选中索引
    ///
    /// @return true 表示可用
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月16日，新建函数
    ///
    bool IsValidCalcTargetValue(const QModelIndex& index);

    ///
    /// @brief 获取质控文档对应的质控结果记录
    ///
    /// @param[in]  docId  文档ID
    /// @param[in]  device  设备序号
    /// @param[in]  assayCode  项目编号
    /// @param[in]  startDate  开始日期
    /// @param[in]  endDate  结束日期
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月16日，新建函数
    ///
    void GetQcResultDataList(int64_t docId, const std::string& device, int assayCode, const std::string& startDate, const std::string& endDate);

protected:

	///
	/// @brief 获取校准品的位置和杯类型
	///
	/// @param[in]  doc  校准品信息
	///
	/// @return 位置和类型字符串
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年11月14日，新建函数
	///
	QString GetPosAndCupType(const ::tf::QcDoc& doc) const;
	
    QQcProjectModel();
};

class QQcLJResultModel : public QStandardItemModel
{
	Q_OBJECT

public:
	enum class COLRESULT
	{
        CALC = 0,               ///< 计算
        SN,                     ///< 序号
        QCRESULT,               ///< 质控结果
        RESULT,                 ///< 结果
        STATUS,                 ///< 状态
        RULE,                   ///< 失控规则
        TARGETVALUE,            ///< 靶值
        SD,                     ///< 标准差
        REASON,                 ///< 失控原因
        SUGGESTION,             ///< 处理措施
        OPERATOR                ///< 操作者
	};

	QQcLJResultModel();

    ///
    /// @brief 判断当前行是否有数据
    ///
    /// @param[in]  index  索引
    ///
    /// @return true 表示有效
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月16日，新建函数
    ///
    bool IsValidLine(const QModelIndex& index);

public slots:
	///
	/// @brief 初始化代码
	///
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年11月14日，新建函数
	///
	void Init();
};

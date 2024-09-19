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
/// @file     QMachineFactor.h
/// @brief    仪器系数
///
/// @author   8090/YeHuaNing
/// @date     2022年9月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "../thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/gen-cpp/defs_types.h"


class QSerialModel;

namespace Ui {
    class QMachineFactor;
};

class QMachineFactor : public BaseDlg
{
    Q_OBJECT

public:
	QMachineFactor(QWidget *parent = Q_NULLPTR);
    ~QMachineFactor();	

public slots:

	///
	/// @brief 执行保存
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	void SaveMachineData();

	///
	/// @brief 当某一行选择后
	///
	/// @param[in]  index  被点击的项目
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
    void onSelectRow(const QModelIndex &index);

    ///
    /// @brief 项目配置发生变化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月9日，新建函数
    ///
    void OnAssayUpdated();

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

private:
    ///
    /// @brief 初始化表标题
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    void InitTableTitle();

    ///
    /// @brief 初始化设备
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    void InitMachines();

    ///
    /// @brief 当选择设备后，进行刷新
    ///
    /// @param[in]  devClassify  设备分类
    /// @param[in]  deviceName  设备名称
    /// @param[in]  moduleIndex  模块索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    void onSelectMachine(tf::AssayClassify::type  devClassify, const std::string& deviceName, int moduleIndex);

    ///
    /// @brief 设置表格中的项目信息
    ///
    /// @param[in]  devClassify  设备类型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月26日，新建函数
    ///
    void SetTableAssayInfo(tf::AssayClassify::type devClassify);

    ///
    /// @brief 将当前表格数据设置为生化信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月26日，新建函数
    ///
    void SetChAssayInfoToTable();

    ///
    /// @brief 将当前表格数据设置为免疫信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月26日，新建函数
    ///
    void SetImAssayInfoToTable();

    ///
    /// @brief 将当前表格数据设置为电解质项目信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月26日，新建函数
    ///
    void SetIseAssayInfoToTable();

    ///
    /// @brief 将double转为字符串
    ///
    /// @param[in]  value  输入值
    ///
    /// @return 字符串数值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月23日，新建函数
    ///
    QString DoubleToString(double value);

private:

	Ui::QMachineFactor*				ui;
	QSerialModel*					m_tableModel;				///< 表数据
	tf::MachineFactorQueryCond		m_mfQryCnd;					///< 当前行的查询条件
	std::vector<::tf::MachineFactor> m_curMachineFactors;		///< 当前仪器系数
    tf::AssayClassify::type         m_curDevType;               ///< 当前仪器的类型
    QString                         m_assayName;                ///< 项目名称
    QString                         m_factorA;                  ///< 项目名称
    QString                         m_factorB;                  ///< 项目名称
};

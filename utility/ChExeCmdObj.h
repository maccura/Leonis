/***************************************************************************
*   This file is part of the BiochemistryDeviceUser project               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   xuxiaolong@maccura.com                                                *
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
/// @file     ChExeCmdObj.h
/// @brief    生化执行指令类
///
/// @author   7997/XuXiaoLong
/// @date     2024年07月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年07月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QObject>
#include "ChDebugStruct.h"

using namespace std;

/* 生化执行指令类 */
class ChExeCmdObject : public QObject
{
	Q_OBJECT

public:

	// 指令类型
	enum ExeType
	{
		ET_EnterModule = 0,		// 点击模块
		ET_OutModule,			// 切出模块
		ET_Reset,				// 点击复位按钮
		ET_Exe,					// 点击执行按钮
		ET_EleCali				// 电流标定
	};

public:
	ChExeCmdObject();

	///
	/// @brief  
	///
	/// @param[in]  type  指令类型
	/// @param[in]  devSN  设备序列号
	/// @param[in]  cmds  指令列表
	/// @param[in]  eleCaliCmd  电流标定指令
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	void SetData(const ExeType& type, const std::string& devSN, const QVector<CanSendData>& cmds, const Cmd& eleCaliCmd);

	///
	/// @brief  将string转为16进制string显示
	///
	/// @param[in]  data  string字符串
	///
	/// @return 16进制string
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	static std::string toHexString(const std::string& data) 
	{
		std::string ret;
		for (auto c : data)
		{
			char tmp[8] = { 0 };
			sprintf(tmp, "%02x ", (uint8_t)c);
			ret += tmp;
		}
		return ret;
	}

public slots :

	///
	/// @brief  执行指令
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	void ExeCmd();

signals:

	///
	/// @brief  执行指令完成信号
	///
	/// @param[in]  type  执行指令类型，参考ExeType
	/// @param[in]  result  执行结果返回值
	///
	void finished(const int type, ::tf::StCanResult result);

private:

	std::string						m_devSN;					// 设备序列号
	ExeType							m_type;						// 指令类型
	QVector<CanSendData>			m_cmdDatas;					// 指令列表
	Cmd 							m_eleCaliCmd;				// 电流标定指令
};

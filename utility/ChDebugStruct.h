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
/// @file     ChDebugStruct.h
/// @brief    生化调试界面结构体
///
/// @author   7997/XuXiaoLong
/// @date     2024年3月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年3月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QString>
#include <QVector>
#include <QMap>
#include <memory>
#include "src/thrift/gen-cpp/defs_types.h"
#include "CmdDefine.h"

#define		CALI_PARAM		-999	// 当配置文件的执行指令中存在"caliParam"时，需将参数替换为CALI_PARAM

using namespace std;

// 模块类型
enum EnumModuleType
{
	EMT_BLANK = 0,				// 空白
	EMT_STEP,					// 步进模块
	EMT_VALVE,					// 阀模块
	EMT_PUMP,					// 泵模块
	EMT_SWITCH,					// 开关模块
	EMT_ULTRA_MIX				// 超声混匀模块
};

// 指令参数
struct Cmd
{
	string							cmdName;					// 指令名称
	tf::StCanSendData				scsd;						// 指令参数
};

// 向下位机发送的数据
struct CanSendData
{
	int								nodeId = -1;				// 节点id
	Cmd								cmd;						// 指令参数
};

// 方向
struct Direction
{
	QString							name;						// 方向名称
	int								id = -1;					// 节点id
	vector<int>						flashAddrs;					// flash地址列表
	int								caliParamType = -1;			// 校准参数类型
};

// 校准参数
struct CaliParam
{
	QString							name;						// 校准参数名称
	QString							tips;						// 点击校准参数名称提示
	QVector<CanSendData>			clickExeCmds;				// 点击执行按钮指令
	QVector<Direction>				directions;					// 方向列表
};

// 步进模块
struct StepModule
{
	QVector<CanSendData>			enterModuleCmds;			// 点击模块执行指令
	QVector<CanSendData>			outModuleCmds;				// 切出模块执行指令
	QVector<CanSendData>			clickResetCmds;				// 点击复位按钮执行指令
	QMap<int,CaliParam>				caliParams;					// 校准参数列表（校准id，校准参数）
};

// 泵/阀/开关模块
struct PVSModule
{
	QString							name;						// 节点名称
	int								id = -1;					// 节点id
	QMap<int, CanSendData>			sendDatas;					// 该节点需要执行的指令列表（指令索引，指令参数）

	// 默认构造
	PVSModule() {}

	// 自定义构造
	PVSModule(const QString& n, const int i)
	{
		name = n;
		id = i;
	}

	///
	/// @brief  查找 设置IO高电平（停止）指令索引
	///
	/// @return -1表示未找到，其他表示对应的指令索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月19日，新建函数
	///
	int findSetIoHIndex()
	{
		if (sendDatas.contains(ECT_SET_IO_HEIGHT))
		{
			return ECT_SET_IO_HEIGHT;
		}
		else if (sendDatas.contains(ECT_STOP))
		{
			return ECT_STOP;
		}

		return -1;
	}

	///
	/// @brief  查找 设置IO低电平（启动）指令索引
	///
	/// @return -1表示未找到，其他表示对应的指令索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月19日，新建函数
	///
	int findSetIoLIndex()
	{
		if (sendDatas.contains(ECT_SET_IO_LOW))
		{
			return ECT_SET_IO_LOW;
		}
		else if (sendDatas.contains(ECT_RUN))
		{
			return ECT_RUN;
		}

		return -1;
	}

	///
	/// @brief  查找 获取IO高电平（获取启动/停止）指令索引
	///
	/// @return -1表示未找到，其他表示对应的指令索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月19日，新建函数
	///
	int findGetIoIndex()
	{
		if (sendDatas.contains(ECT_GET_IO))
		{
			return ECT_GET_IO;
		}
		else if (sendDatas.contains(ECT_GET_RUN_STOP))
		{
			return ECT_GET_RUN_STOP;
		}

		return -1;
	}
};

// 超声混匀数据
struct UltraMix
{
	QString							name;						// 参数名称
	int								paramAddr;					// 参数地址
	int								idx = -1;					// 点击超声混匀某一行是否需要将获取到的电流显示到参考值上的索引，-1表示无需显示到参考值

	UltraMix() {}
	UltraMix(const QString& n, int addr, int i)
	{
		name = n;
		paramAddr = addr;
		idx = i;
	}
};

// 超声混匀模块
struct UltraMixModule
{
	Cmd								eleCaliCmd;					// 执行电流标定的指令
	QVector<UltraMix>				vecUltraMix;				// 超声混匀列表
};

// 生化节点调试数据
struct ChNodeDebug
{
	EnumModuleType					moduleType = EMT_BLANK;		// 模块类型
	QString							moduleName;					// 模块名称
	shared_ptr<StepModule>			spStepModuleInfo = nullptr;	// 步进模块数据
	shared_ptr<QVector<PVSModule>>	spVecCPVSModuleInfos = nullptr;	// 泵/阀/开关模块数据
	shared_ptr<UltraMixModule>		spUlMixModuleInfos = nullptr;// 超声混匀模块数据
};
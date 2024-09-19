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
/// @file     ChExeCmdObj.cpp
/// @brief    生化执行指令类
///
/// @author   7997/XuXiaoLong
/// @date     2023年02月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年02月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ChExeCmdObj.h"
#include "ChNodeDebugConfigManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include <QtEndian>

ChExeCmdObject::ChExeCmdObject()
{

}

void ChExeCmdObject::SetData(const ExeType& type, const std::string& devSN, const QVector<CanSendData>& cmds, const Cmd& eleCaliCmd)
{
	m_type = type;
	m_devSN = devSN;
	m_cmdDatas = cmds;
	m_eleCaliCmd = eleCaliCmd;
}

void ChExeCmdObject::ExeCmd()
{
	::tf::StCanResult ret;

	if (m_type == ET_EnterModule || m_type == ET_OutModule || m_type == ET_Reset || m_type == ET_Exe)
	{
		int size = m_cmdDatas.size();
		// 上一条记录的返回值
		string preRetStr;
		string preCmdName;
		for (int i = 0; i < size; i++)
		{
			auto csd = m_cmdDatas[i];

			// 当前指令是否需要替换校准参数
			bool bCaliParam = false;
			for (auto &param : csd.cmd.scsd.data.cmdData.params)
			{
				// 上一条指令执行读取flash 该条指令替换其返回值再执行
				if (param.paramType == tf::EmCanParamType::INT32
					&& param.paramVal.i32Val == CALI_PARAM)
				{
					if (preRetStr.size() != 4)
					{
						ULOG(LOG_ERROR, u8"上一条指令[%s]返回值要求为Int，下位机实际返回数据=%s, 位数=%d,", 
							preCmdName, toHexString(preRetStr), preRetStr.size());
						break;
					}

					int32_t targetPos = -1;
					try
					{
						targetPos = qFromBigEndian<int32_t>(preRetStr.data());
					}
					catch (const std::exception& ex)
					{
						ULOG(LOG_ERROR, "qFromBigEndian exception: %s", ex.what());
						break;
					}

					// 替换指令内容
					param.paramVal.__set_i32Val(targetPos);
				}
			}

			ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, csd.nodeId, csd.cmd.scsd);
			ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
				m_devSN, csd.cmd.cmdName, csd.nodeId, ToString(csd.cmd.scsd), ToString(ret));

			preRetStr = ret.canResp;
			preCmdName = csd.cmd.cmdName;

			// 一条指令失败直接返回，不继续执行后续指令
			if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, u8"指令[%s]执行失败.", csd.cmd.cmdName);
				break;
			}
		}
	}
	else if(m_type == ET_EleCali)
	{
		ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, m_eleCaliCmd.scsd);
		ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
			m_devSN, m_eleCaliCmd.cmdName, 0, ToString(m_eleCaliCmd.scsd), ToString(ret));

		// 一条指令失败直接返回，不继续执行后续指令
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, u8"指令[%s]执行失败.", m_eleCaliCmd.cmdName);
		}
	}

	// 完成信号
	emit finished((int)m_type, ret);
}

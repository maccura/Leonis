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
/// @file     devc1005.h
/// @brief    通用设备接口
///
/// @author   4170/TangChuXian
/// @date     2022年12月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "abstractdevice.h"
#include <QVariant>

// 具体设备生化1005
class  DevC1005 : public AbstractDevice
{
public:
    ///
    /// @brief
    ///     注册1005设备
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    static bool RegisterDevice();

    ///
    /// @brief
    ///     虚析构函数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual ~DevC1005();

    ///
    /// @brief
    ///     操作数据（增删查改等）
    ///
    /// @param[in]  iDataCode    数据码
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual bool OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer) override;

    ///
    /// @brief
    ///     执行命令
    ///
    /// @param[in]  iCmdCode   命令码
    /// @param[in]  paramList  参数列表
    /// @param[out] iRetCode   返回码
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual bool ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode) override;

    ///
    /// @brief
    ///     获取设备类型
    ///
    /// @return 设备类型编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual int DeviceType() override;

    ///
    /// @brief
    ///     设备类别(生化、免疫等)
    ///
    /// @return 设备类别编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月13日，新建函数
    ///
    virtual int DeviceClass() override;

    ///
    /// @brief
    ///     获取设备序列号
    ///
    /// @return 设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual QString DeviceSn() override;

    ///
    /// @brief
    ///     设备的模块数
    ///
    /// @return 模块数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月25日，新建函数
    ///
    virtual int ModuleCount() override;

    ///
    /// @bref
    ///		试剂槽位数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月28日，新建函数
    ///
    virtual int ReagentSlotNumber() override;

protected:
    DevC1005();

    ///
    /// @brief
    ///     操作数据（增删查改等）
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年2月3日，新建函数
    ///
    bool OperateQcDocData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作质控申请数据（增删查改等）
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    bool OperateQcApplyData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作默认质控数据（增删查改等）
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    bool OperateDefaultQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作备用瓶质控数据（增删查改等）
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    bool OperateBackupRgntQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作项目名列表数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月3日，新建函数
    ///
    bool OperateAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     质控Lj图项目名
    ///
    /// @param[in]  iDataOpCode     数据操作码
    /// @param[in]  paramList       参数列表
    /// @param[in]  outBuffer       输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月28日，新建函数
    ///
    bool OperateQcLjAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     质控Tp图项目名
    ///
    /// @param[in]  iDataOpCode     数据操作码
    /// @param[in]  paramList       参数列表
    /// @param[in]  outBuffer       输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月28日，新建函数
    ///
    bool OperateQcTpAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作质控靶值信息数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月28日，新建函数
    ///
    bool OperateQcConcInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作质控结果信息数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月28日，新建函数
    ///
    bool OperateQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作靶值SD数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
    ///
    bool OperateQcTargetValSDData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作单质控失控原因和失控处理措施数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool OperateSingleQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作联合质控失控原因和失控处理措施数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月3日，新建函数
    ///
    bool OperateTwinQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作单质控计算点数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool OperateSingleQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作联合质控计算点数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool OperateTwinQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

    ///
    /// @brief
    ///     操作联合质控信息数据
    ///
    /// @param[in]  iDataOpCode  数据操作码
    /// @param[in]  paramList    参数列表
    /// @param[out] outBuffer    输出缓冲区
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月30日，新建函数
    ///
    bool OperateTwinQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer);

private:
    // 记录设备类型和设备信息映射
    static QMap<QPair<QString, QString>, QVariant>  sm_mapDevNameInfo;

    // 友元类——设备工厂：支持工厂函数创建对象
    friend class DeviceFactory;
};

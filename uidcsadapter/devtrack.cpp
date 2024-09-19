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
/// @file     devtrack.cpp
/// @brief    Track设备接口
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
#include "devtrack.h"
#include <tuple>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QList>

#include "adaptertypedef.h"
#include "thrift/DcsControlProxy.h"

// 初始化静态成员变量
QMap<QString, QVariant>          DevTrack::sm_mapDevNameInfo;         // 设备名和设备序列号映射

// 注册元类型
Q_DECLARE_METATYPE(tf::DeviceInfo)

DevTrack::DevTrack()
{
}

DevTrack::~DevTrack()
{

}

///
/// @brief
///     注册Track设备
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
bool DevTrack::RegisterDevice()
{
    // 构造查询条件
    ::tf::DeviceInfoQueryResp devResp;
    ::tf::DeviceInfoQueryCond devCond;
    devCond.__set_deviceType(tf::DeviceType::DEVICE_TYPE_TRACK);

    // 查询设备信息
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devResp, devCond)
        || devResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || devResp.lstDeviceInfos.empty())
    {
        return false;
    }

    // 注册Track设备
    for (const auto& stuDevInfo: devResp.lstDeviceInfos)
    {
        // 注册设备
        DYNAMIC_REGISTER_DEVICE(DevTrack, QString::fromStdString(stuDevInfo.name), QString::fromStdString(stuDevInfo.groupName));

        // 设置设备信息
        sm_mapDevNameInfo.insert(QString::fromStdString(stuDevInfo.name), QVariant::fromValue(stuDevInfo));
    }

    return true;
}

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
bool DevTrack::OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    return true;
}

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
bool DevTrack::ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode)
{
    return true;
}

///
/// @brief
///     设备类别(生化、免疫等)
///
/// @return 设备类别编号
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月13日，新建函数
///
int DevTrack::DeviceClass()
{
    auto it = sm_mapDevNameInfo.find(DeviceName());
    if (it == sm_mapDevNameInfo.end())
    {
        return -1;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return int(stuDevInfo.deviceClassify);
}

///
/// @brief
///     获取设备类型
///
/// @return 设备类型编号
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
int DevTrack::DeviceType()
{
    auto it = sm_mapDevNameInfo.find(DeviceName());
    if (it == sm_mapDevNameInfo.end())
    {
        return -1;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return int(stuDevInfo.deviceType);
}

///
/// @brief
///     获取设备序列号
///
/// @return 设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
QString DevTrack::DeviceSn()
{
    auto it = sm_mapDevNameInfo.find(DeviceName());
    if (it == sm_mapDevNameInfo.end())
    {
        return QString("");
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return QString::fromStdString(stuDevInfo.deviceSN);
}

///
/// @brief
///     设备的模块数
///
/// @return 模块数
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月25日，新建函数
///
int DevTrack::ModuleCount()
{
    auto it = sm_mapDevNameInfo.find(DeviceName());
    if (it == sm_mapDevNameInfo.end())
    {
        return 0;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return stuDevInfo.moduleCount;
}

///
/// @bref
///		试剂仓位数
///
/// @par History:
/// @li 8276/huchunli, 2022年12月28日，新建函数
///
int DevTrack::ReagentSlotNumber()
{
    return 0;
}

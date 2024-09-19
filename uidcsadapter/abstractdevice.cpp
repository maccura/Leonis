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
/// @file     abstractdevice.cpp
/// @brief    界面逻辑——抽象设备接口
///
/// @author   4170/TangChuXian
/// @date     2022年6月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年6月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "abstractdevice.h"

// 声明静态成员变量
QMap<QPair<QString, QString>, std::shared_ptr<AbstractDevice>> DeviceFactory::sm_mapRegster;

AbstractDevice::AbstractDevice() : m_iModuleNo(1)
{
}

AbstractDevice::~AbstractDevice()
{

}

///
/// @brief
///     获取设备对象
///
/// @param[in]  strDevName  设备名
///
/// @return 设备对象实例
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
/// @li 4170/TangChuXian，2023年5月25日，增加参数【组名】
///
std::shared_ptr<AbstractDevice> DeviceFactory::GetDevice(const QString& strDevName, const QString strGrpName /*= ""*/)
{
    // 寻找对应的构造函数
    auto it = sm_mapRegster.find(qMakePair(strDevName, strGrpName));
    if (it == sm_mapRegster.end())
    {
        return Q_NULLPTR;
    }

    // 判断对象实例
    return it.value();
}

///
/// @brief
///     获取所有设备名
///
/// @return 获取所有设备名
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
QList<QPair<QString, QString>> DeviceFactory::GetAllDeviceName()
{
    QList<QPair<QString, QString>> strDevNameList;
    for (auto it = sm_mapRegster.begin(); it != sm_mapRegster.end(); it++)
    {
        if (it.key().first.isEmpty())
        {
            continue;
        }

        strDevNameList.push_back(it.key());
    }

    return strDevNameList;
}

///
/// @brief 获取所有设备类型（不重复）
///
///
/// @return 设备类型set
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年10月17日，新建函数
///
QSet<tf::DeviceType::type> DeviceFactory::GetAllDeviceType()
{
	// 构造返回值
	QSet<tf::DeviceType::type> setRet;

	for (auto it = sm_mapRegster.begin(); it != sm_mapRegster.end(); it++)
	{
		if (it.key().first.isEmpty())
		{
			continue;
		}

		setRet.insert(static_cast<tf::DeviceType::type>(it.value()->DeviceType()));
	}

	return setRet;
}

///
/// @brief 根据是否联机获取所有设备Sn,单机过滤轨道
///
/// @param[in]  isPip  true:联机
///
/// @return 设备序列号
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年8月30日，新建函数
///
QVector<QString> DeviceFactory::GetAllDeviceSnWithoutSingleTrack(bool isPip)
{
	QVector<QString> vecRet;

	for (auto it = sm_mapRegster.begin(); it != sm_mapRegster.end(); it++)
	{
		if (it.key().first.isEmpty())
		{
			continue;
		}

		// 单机过滤轨道
		if (it.value()->DeviceType() == int(tf::DeviceType::DEVICE_TYPE_TRACK) && 
			!isPip)
		{
			continue;
		}

		vecRet.push_back(it.value()->DeviceSn());
	}

	// 排序
	std::sort(vecRet.begin(), vecRet.end(), [](const QString& sn1, const QString& sn2) {
		return sn1.toInt() < sn2.toInt();
	});
	return vecRet;
}

AdapterDataBuffer::AdapterDataBuffer()
{

}

///
/// @brief
///     获取单实例
///
/// @return 单例对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
AdapterDataBuffer* AdapterDataBuffer::GetInstance()
{
    static AdapterDataBuffer s_obj;
    return &s_obj;
}

///
/// @brief
///     将质控品信息赋值到质控申请信息
///
/// @param[in]  lQcId           质控品ID
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
bool AdapterDataBuffer::AssignQcInfoToQcApply(long long lQcId, QC_APPLY_INFO& stuQcApplyInfo)
{
    auto it = m_mapQcInfo.find(lQcId);
    if (it == m_mapQcInfo.end())
    {
        return false;
    }

    // 赋值
    stuQcApplyInfo.strQcDocID = it.value().strQcDocID;
    stuQcApplyInfo.strQcNo = it.value().strQcNo;
    stuQcApplyInfo.strQcName = it.value().strQcName;
    stuQcApplyInfo.strQcBriefName = it.value().strQcBriefName;
    stuQcApplyInfo.strQcSourceType = it.value().strQcSourceType;
    stuQcApplyInfo.strQcLevel = it.value().strQcLevel;
    stuQcApplyInfo.strQcLot = it.value().strQcLot;
    stuQcApplyInfo.strQcExpDate = it.value().strQcExpDate;
    return true;
}

void AdapterDataBuffer::MapQcInfoToQcApply(long long lQcId, const QC_APPLY_INFO& stuQcApplyInfo)
{
    m_mapQcInfo.insert(lQcId, stuQcApplyInfo);
}

///
/// @brief
///     将项目信息赋值到质控申请信息
///
/// @param[in]  iAssayCode      项目编号
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
bool AdapterDataBuffer::AssignAssayInfoToQcApply(int iAssayCode, QC_APPLY_INFO& stuQcApplyInfo)
{
    auto it = m_mapAssayInfo.find(iAssayCode);
    if (it == m_mapAssayInfo.end())
    {
        return false;
    }

    // 赋值
    stuQcApplyInfo.strAssayName = it.value().strAssayName;
    stuQcApplyInfo.strAssayCode = it.value().strAssayCode;
    return true;
}

void AdapterDataBuffer::MapAssayInfoToQcApply(int iAssayCode, const QC_APPLY_INFO& stuQcApplyInfo)
{
    m_mapAssayInfo.insert(iAssayCode, stuQcApplyInfo);
}

///
/// @brief
///     将试剂信息赋值到质控申请信息
///
/// @param[in]  lRgntID         数据库主键
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
bool AdapterDataBuffer::AssignRgntInfoToQcApply(const QString& strDevSn, int iPos, QC_APPLY_INFO& stuQcApplyInfo)
{
    auto itDev = m_mapBackupRgntInfo.find(strDevSn);
    if (itDev == m_mapBackupRgntInfo.end())
    {
        return false;
    }

    auto it = itDev.value().find(iPos);
    if (it == itDev.value().end())
    {
        return false;
    }

    // 赋值
    stuQcApplyInfo.iBackupRgntPos = it.value().iBackupRgntPos;
    stuQcApplyInfo.strPos = it.value().strPos;
    stuQcApplyInfo.strRgntUseStatus = it.value().strRgntUseStatus;
    stuQcApplyInfo.strRgntNo = it.value().strRgntNo;
    stuQcApplyInfo.strRgntLot = it.value().strRgntLot;
    return true;
}

void AdapterDataBuffer::MapRgntInfoToQcApply(const QString& strDevSn, int iPos, const QC_APPLY_INFO& stuQcApplyInfo)
{
    auto itDev = m_mapBackupRgntInfo.find(strDevSn);
    if (itDev == m_mapBackupRgntInfo.end())
    {
        itDev = m_mapBackupRgntInfo.insert(strDevSn, QMap<int, QC_APPLY_INFO>());
        itDev.value().insert(iPos, stuQcApplyInfo);
    }
    else
    {
        itDev.value().insert(iPos, stuQcApplyInfo);
    }
}


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
/// @file     AbstractDevice.h
/// @brief    界面逻辑——抽象设备接口
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

#include <boost/any.hpp>
#include <memory>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QPair>
#include <QList>
#include "adaptertypedef.h"
#include "src/thrift/gen-cpp/defs_types.h"

// 注册具体设备
#define DYNAMIC_REGISTER_DEVICE(deviceClass, deviceName, deviceGrp)    DeviceFactory::RegisterDevice<deviceClass>(deviceName, deviceGrp)
#define UNREGISTER_DEVICE(deviceClass, deviceName, deviceGrp)          DeviceFactory::UnRegisterDevice<deviceClass>(deviceName, deviceGrp)

// 抽象设备
class AbstractDevice
{
public:
    ///
    /// @brief
    ///     虚析构函数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual ~AbstractDevice();

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
    virtual bool OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer) = 0;

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
    virtual bool ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode) = 0;

    ///
    /// @brief
    ///     获取设备类型
    ///
    /// @return 设备类型编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual int DeviceType() = 0;

    ///
    /// @brief
    ///     设备类别(生化、免疫等)
    ///
    /// @return 设备类别编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月13日，新建函数
    ///
    virtual int DeviceClass() = 0;

    ///
    /// @brief
    ///     获取设备序列号
    ///
    /// @return 设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    virtual QString DeviceSn() = 0;

    ///
    /// @brief
    ///     设备的模块数
    ///
    /// @return 模块数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月25日，新建函数
    ///
    virtual int ModuleCount() = 0;

    ///
    /// @bref
    ///     获取设备的试剂仓位数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月28日，新建函数
    ///
    virtual int ReagentSlotNumber() = 0;

    ///
    /// @brief
    ///     获取设备名
    ///
    /// @return 设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    inline const QString& DeviceName() { return m_strDeviceName; }

	///
	/// @brief
	///     获取设备名
	///
	/// @return 设备名
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月21日，新建函数
	///
	inline const QString& DevGroupName() { return m_strDevGroupName; }

    ///
    /// @brief
    ///     获取模块号
    ///
    /// @return 模块号,0表示整机
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月17日，新建函数
    ///
    inline int ModuleNo() { return m_iModuleNo; }

    ///
    /// @brief
    ///     设置模块号
    ///
    /// @param[in]  iModuleNo   模块号，0表示整机
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月17日，新建函数
    ///
    void SetModuleNo(int iModuleNo) { m_iModuleNo = iModuleNo; }

protected:
    ///
    /// @brief
    ///     构造函数，不允许使用多态构造，统一用工厂函数创建
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    AbstractDevice();

private:
    // 设备名称（注册设备时即初始化）
    QString m_strDeviceName;

	// 设备组名称（注册设备时即初始化）
	QString m_strDevGroupName;

    // 模块号
    int m_iModuleNo;

    // 友元类——设备工厂：支持工厂函数创建对象
    friend class DeviceFactory;
};

// 设备工厂
class DeviceFactory
{
public:
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
    static std::shared_ptr<AbstractDevice> GetDevice(const QString& strDevName, const QString strGrpName = "");

    ///
    /// @brief
    ///     获取所有设备名
    ///
    /// @return 获取所有设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    static QList<QPair<QString, QString>> GetAllDeviceName();

	///
	/// @brief 获取所有设备类型（不重复）
	///
	///
	/// @return 设备类型set
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年10月17日，新建函数
	///
	static QSet<tf::DeviceType::type> GetAllDeviceType();

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
	static QVector<QString> GetAllDeviceSnWithoutSingleTrack(bool isPip);

    ///
    /// @brief
    ///     注册具体设备(模板参数T代表对应的具体设备类)
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  strGrpName  组名（设备不属于任何组则为空）
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    template<typename T>
    static bool RegisterDevice(const QString& strDevName, const QString strGrpName = "")
    {
        // 已经注册了则返回
        if (sm_mapRegster.find(qMakePair(strDevName, strGrpName)) != sm_mapRegster.end())
        {
            return true;
        }

        // 注册设备
        sm_mapRegster.insert(qMakePair(strDevName, strGrpName), std::shared_ptr<AbstractDevice>(new T()));
        sm_mapRegster[qMakePair(strDevName, strGrpName)]->m_strDeviceName = strDevName;
		sm_mapRegster[qMakePair(strDevName, strGrpName)]->m_strDevGroupName = strGrpName;
        return true;
    }

    ///
    /// @brief
    ///     反注册具体设备
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  strGrpName  组名（设备不属于任何组则为空）
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    template<typename T>
    static bool UnRegisterDevice(const QString& strDevName, const QString strGrpName = "")
    {
        // 未注册则返回
        if (sm_mapRegster.find(qMakePair(strDevName, strGrpName)) == sm_mapRegster.end())
        {
            return false;
        }

        // 反注册类
        sm_mapRegster.remove(qMakePair(strDevName, strGrpName));
        return true;
    }

private:
    // 实例注册器
    static QMap<QPair<QString, QString>, std::shared_ptr<AbstractDevice>> sm_mapRegster;
};

// 具体设备注册类
template <typename T>
class DeviceRegister
{
public:
    DeviceRegister(const QString& strDevName, const QString strGrpName = "") : m_cstrDevNameGrpPair(strDevName, strGrpName)
    {
        DeviceFactory::RegisterDevice<T>(strDevName, strGrpName);
    }

    ~DeviceRegister()
    {
        DeviceFactory::UnRegisterDevice<T>(m_cstrDevNameGrpPair.first, m_cstrDevNameGrpPair.second);
    }

private:
    const QPair<QString, QString> m_cstrDevNameGrpPair;
};

// 适配数据缓存
class AdapterDataBuffer
{
public:
    ~AdapterDataBuffer() {}

    ///
    /// @brief
    ///     获取单实例
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月23日，新建函数
    ///
    static AdapterDataBuffer* GetInstance();

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
    bool AssignQcInfoToQcApply(long long lQcId, QC_APPLY_INFO& stuQcApplyInfo);
    void MapQcInfoToQcApply(long long lQcId, const QC_APPLY_INFO& stuQcApplyInfo);

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
    bool AssignAssayInfoToQcApply(int iAssayCode, QC_APPLY_INFO& stuQcApplyInfo);
    void MapAssayInfoToQcApply(int iAssayCode, const QC_APPLY_INFO& stuQcApplyInfo);

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
    bool AssignRgntInfoToQcApply(const QString& strDevSn, int iPos, QC_APPLY_INFO& stuQcApplyInfo);
    void MapRgntInfoToQcApply(const QString& strDevSn, int iPos, const QC_APPLY_INFO& stuQcApplyInfo);

    ///
    /// @brief
    ///     重置质控申请缓存
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月23日，新建函数
    ///
    inline void ResetQcApply() 
    {
        m_mapQcInfo.clear();
        m_mapAssayInfo.clear();
        m_mapBackupRgntInfo.clear();
    }

protected:
    AdapterDataBuffer();

private:
    QMap<long long, QC_APPLY_INFO>                                  m_mapQcInfo;                // 质控信息映射
    QMap<int, QC_APPLY_INFO>                                        m_mapAssayInfo;             // 项目信息映射
    QMap<QString, QMap<int, QC_APPLY_INFO>>                         m_mapBackupRgntInfo;        // 备用瓶信息映射
};

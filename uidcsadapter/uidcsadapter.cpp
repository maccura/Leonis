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
/// @file     uidcsadapter.cpp
/// @brief    UI-DCS接口适配器
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
#include "uidcsadapter.h"
#include <QDate>
#include "devgeneral.h"
#include "devi6000.h"
#include "devc1005.h"
#include "devise1005.h"
#include "devtrack.h"
#include "CommonInformationManager.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/ReagentCommon.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

UiDcsAdapter::UiDcsAdapter() : m_iModuleNo(1)
{
    // 初始化组合设备映射
    // 构造查询条件
    ::tf::DeviceInfoQueryResp devResp;
    ::tf::DeviceInfoQueryCond devCond;

    // 查询设备信息
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devResp, devCond)
        || devResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || devResp.lstDeviceInfos.empty())
    {
        return;
    }

    // 构造组合设备映射
    for (const auto& stuDevInfo : devResp.lstDeviceInfos)
    {
        // 如果不是组合设备的子设备，则跳过
        if (stuDevInfo.groupName.empty())
        {
            QStringList strSubDevList;
            strSubDevList.push_back(QString::fromStdString(stuDevInfo.name));
            m_mapDevGroup.insert(QString::fromStdString(stuDevInfo.name), strSubDevList);
            continue;
        }

        // 在容器中查找该组合设备
        auto it = m_mapDevGroup.find(QString::fromStdString(stuDevInfo.groupName));
        if (it == m_mapDevGroup.end())
        {
            QStringList strSubDevList;
            strSubDevList.push_back(QString::fromStdString(stuDevInfo.name));
            m_mapDevGroup.insert(QString::fromStdString(stuDevInfo.groupName), strSubDevList);
            continue;
        }

        // 追加子设备
        it.value().push_back(QString::fromStdString(stuDevInfo.name));
    }
}

UiDcsAdapter::~UiDcsAdapter()
{

}

///
/// @brief
///     注册所有设备
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
bool UiDcsAdapter::RegisterDevice()
{
    // 注册所有设备
    DevGeneral::RegisterDevice();
    DevIse1005::RegisterDevice();
    DevC1005::RegisterDevice();
    DevI6000::RegisterDevice();
    DevTrack::RegisterDevice();

    return true;
}

///
/// @brief
///     获取单例对象
///
/// @param[in]  strDevName  设备名
/// @param[in]  iModuleNo   模块号
///
/// @return 单例对象
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
UiDcsAdapter* UiDcsAdapter::GetInstance(const QString strDevName /*= ""*/, int iModuleNo)
{
    static UiDcsAdapter obj;
    obj.m_strGrpName = "";
    obj.m_strDevName = strDevName;
    obj.m_iModuleNo = iModuleNo;
    return &obj;
}

///
/// @brief
///     获取单例对象
///
/// @param[in]  strDevName  设备名
/// @param[in]  strGrpName  组名
/// @param[in]  iModuleNo   模块号
///
/// @return 单例对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月25日，新建函数
///
UiDcsAdapter* UiDcsAdapter::GetInstance(const QString& strDevName, const QString& strGrpName, int iModuleNo /*= 1*/)
{
    static UiDcsAdapter obj;
    obj.m_strGrpName = strGrpName;
    obj.m_strDevName = strDevName;
    obj.m_iModuleNo = iModuleNo;
    return &obj;
}

///
/// @brief
///     获取设备类别
///
/// @param[in]  strDevName  设备名
/// @param[in]  strGrpName  组名
///
/// @return 设备类别
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月6日，新建函数
///
DEVICE_CLASSIFY UiDcsAdapter::GetDeviceClasssify(const QString& strDevName, const QString strGrpName /*= ""*/)
{
    // 设备名为空，则返回设备类别为其他
    if (strDevName.isEmpty())
    {
        return DEVICE_CLASSIFY_OTHER;
    }

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName, strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return DEVICE_CLASSIFY_OTHER;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 获取设备类别
    DEVICE_CLASSIFY enDevClassify = (DEVICE_CLASSIFY)spIDev->DeviceClass();
    return enDevClassify;
}

///
/// @brief 获取设备类型
///
/// @param[in]  strDevName  设备名
/// @param[in]  strGrpName  组名
///
/// @return 设备类型
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月9日，新建函数
///
tf::DeviceType::type UiDcsAdapter::GetDeviceType(const QString& strDevName, const QString strGrpName /*= ""*/)
{
	// 设备名为空，则返回设备类别为其他
	if (strDevName.isEmpty())
	{
		return tf::DeviceType::DEVICE_TYPE_INVALID;
	}

	// 获取对应设备
	std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName, strGrpName);
	if (spIDev == Q_NULLPTR)
	{
		return tf::DeviceType::DEVICE_TYPE_INVALID;
	}
	spIDev->SetModuleNo(m_iModuleNo);

	// 获取设备类型
	return (tf::DeviceType::type)spIDev->DeviceType();
}

///
/// @brief
///     是否包含其他子设备
///
/// @param[in]  strDevName  设备名
///
/// @return true表示是组设备
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月18日，新建函数
///
bool UiDcsAdapter::WhetherContainOtherSubDev(const QString& strDevName)
{
    // 在组设备容器中查找组设备
    auto it = m_mapDevGroup.find(strDevName);
    if (it == m_mapDevGroup.end())
    {
        return false;
    }

    // 如果其子设备为空，则不是
    if (it.value().isEmpty())
    {
        return false;
    }

    // 如果其子设备只有一个且和组设备名相同，则不是
    if ((it.value().size() == 1) && (it.value().first() == strDevName))
    {
        return false;
    }

    return true;
}

///
/// @brief
///     是否是单机版
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月31日，新建函数
///
bool UiDcsAdapter::WhetherSingleDevMode()
{
    return !DictionaryQueryManager::GetInstance()->GetPipeLine();
}

///
/// @brief
///     是否只有免疫设备
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月31日，新建函数
///
bool UiDcsAdapter::WhetherOnlyImDev()
{
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是轨道设备则跳过
        if (spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 是免疫设备则跳过
        if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            continue;
        }

        return false;
    }

    return true;
}

///
/// @brief
///     是否只有生化设备
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月26日，新建函数
///
bool UiDcsAdapter::WhetherOnlyChDev()
{
	QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
	for (const auto& strDevName : strAllDevNameList)
	{
		// 获取对应设备
		std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
		if (spIDev == Q_NULLPTR)
		{
			continue;
		}

		// 是轨道设备则跳过
		if (spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
		{
			continue;
		}

		// 是生化设备则跳过
		if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY || 
			spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
		{
			continue;
		}

		return false;
	}

	return true;
}

///
/// @brief
///     是否包含免疫设备
///
/// @return true表示包含免疫设备
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月4日，新建函数
///
bool UiDcsAdapter::WhetherContainImDev()
{
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是免疫设备则跳过
        if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            return true;
        }
    }

    return false;
}

///
/// @brief
///     获取免疫设备序列号列表
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月7日，新建函数
///
QStringList UiDcsAdapter::GetImDevSnLst()
{
    QStringList strImDevSnList;
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是免疫设备则跳过
        if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            strImDevSnList.push_back(spIDev->DeviceSn());
        }
    }

    return strImDevSnList;
}

///
/// @brief
///     是否包含目标设备(测试项目类型分类)
///
/// @param[in]  devClassify  目标设备类型
///
/// @return true表示包含
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
///
bool UiDcsAdapter::WhetherContainTargetDevClassify(const tf::AssayClassify::type &devClassify)
{
	QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
	for (const auto& strDevName : strAllDevNameList)
	{
		// 获取对应设备
		std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
		if (spIDev == Q_NULLPTR)
		{
			continue;
		}

		// 是否为目标设备类型
		if (spIDev->DeviceClass() == devClassify)
		{
			return true;
		}
	}

	return false;

}

///
/// @brief 目前的机型是否支持该维护项
///
/// @param[in]  maintainItem  
///
/// @return true:支持
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年10月17日，新建函数
///
bool UiDcsAdapter::WhetherSupportTargetMaintainItem(const ::tf::MaintainItem& maintainItem)
{
	// 获取全部设备类型(不重复)
	QSet<tf::DeviceType::type> allDevType = DeviceFactory::GetAllDeviceType();
	std::vector<tf::DeviceType::type> maintainDevTypes;
	maintainDevTypes.clear();

	// 维护组表中没有设置维护项的设备信息
	if (!maintainItem.__isset.deviceTypes || maintainItem.deviceTypes.empty())
	{
		// 单项表中查询该维护项
		tf::MaintainItemQueryCond itemQryCond;
		itemQryCond.__set_itemType(maintainItem.itemType);
		tf::MaintainItemQueryResp itemQryResp;

		// 执行查询
		if (!DcsControlProxy::GetInstance()->QueryMaintainItem(itemQryResp, itemQryCond) 
			|| (itemQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			|| (itemQryResp.lstMaintainItems.empty()))
		{
			ULOG(LOG_ERROR, "%s(), QueryMaintainItem() failed", __FUNCTION__);
			return false;
		}

		maintainDevTypes = itemQryResp.lstMaintainItems.front().deviceTypes;
	}
	else
	{
		maintainDevTypes = maintainItem.deviceTypes;
	}

	// 遍历维护项支持的机型(只要有一个即支持)
	bool isSupport = false;
	for (const auto& devType : maintainDevTypes)
	{
		if (allDevType.contains(devType))
		{
			isSupport = true;
			break;
		}
	}

	return isSupport;
}

///
/// @brief
///     获取设备名列表
///
/// @param[in]  strDevNameList  设备名列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月19日，新建函数
///
bool UiDcsAdapter::GetDevNameList(QList<QPair<QString, QString>>& strDevNameList)
{
    // 清空输出缓冲区
    strDevNameList.clear();
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是轨道设备则跳过
        if (spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        strDevNameList.push_back(strDevName);
    }

    return true;
}

///
/// @brief
///     获取设备名列表（包含轨道）
///
/// @param[in]  strDevNameList  设备名列表
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月30日，新建函数
///
bool UiDcsAdapter::GetDevNameListWithTrack(QList<QPair<QString, QString>>& strDevNameList)
{
    // 清空输出缓冲区
    strDevNameList.clear();
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 如果是单机版，则过滤轨道
        if (WhetherSingleDevMode() && spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        strDevNameList.push_back(strDevName);
    }

    return true;
}

///
/// @brief
///     获取组设备和独立设备列表
///
/// @param[in]  strDevNameList  获取组设备和独立设备列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月18日，新建函数
///
bool UiDcsAdapter::GetGroupDevNameList(QStringList& strDevNameList)
{
    // 清空设备列表
    strDevNameList.clear();

    // 遍历容器
    for (auto it = m_mapDevGroup.begin(); it != m_mapDevGroup.end(); it++)
    {
        // 过滤轨道
        if (it.value().isEmpty())
        {
            continue;
        }

        // 获取首个子设备名
        const QString& strSubDevName = it.value().first();

        // 组名和设备名相同，则组为空
        QString strGrpName = (it.key() == strSubDevName) ? "" : it.key();

        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strSubDevName, strGrpName);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 获取设备类别,轨道则过滤
        if (spIDev->DeviceType() == int(tf::DeviceType::DEVICE_TYPE_TRACK))
        {
            continue;
        }

        // 将组设备名压入列表
        strDevNameList.push_back(it.key());
    }

    return true;
}

///
/// @brief
///     获取组设备和独立设备列表
///
/// @param[in]  strDevNameList  获取组设备和独立设备列表
/// @param[in]  enTfDevType     设备类型
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
bool UiDcsAdapter::GetGroupDevNameList(QStringList& strDevNameList, tf::DeviceType::type enTfDevType)
{
    // 清空设备列表
    strDevNameList.clear();

    // 遍历容器
    for (auto it = m_mapDevGroup.begin(); it != m_mapDevGroup.end(); it++)
    {
        // 过滤轨道
        if (it.value().isEmpty())
        {
            continue;
        }

        // 获取首个子设备名
        const QString& strSubDevName = it.value().first();

        // 组名和设备名相同，则组为空
        QString strGrpName = (it.key() == strSubDevName) ? "" : it.key();

        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strSubDevName, strGrpName);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 获取设备类别,轨道则过滤
        if (spIDev->DeviceType() != enTfDevType)
        {
            continue;
        }

        // 将组设备名压入列表
        strDevNameList.push_back(it.key());
    }

    return true;
}

///
/// @brief
///     获取组设备和独立设备列表
///
/// @param[out]  strDevNameList  获取组设备和独立设备列表
/// @param[in]  needSnSort  需要排序(true:按照sn升序排列, false:按照名称排序)
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月25日，新建函数
///
bool UiDcsAdapter::GetGroupDevNameListWithTrack(QStringList& strDevNameList, bool needSnSort /*= false*/)
{
    // 清空设备列表
    strDevNameList.clear();

    // 遍历容器(名称，设备)
	QVector<QPair<QString, std::shared_ptr<AbstractDevice>>> vecNameDev;
    for (auto it = m_mapDevGroup.begin(); it != m_mapDevGroup.end(); it++)
    {
		if (it.value().isEmpty())
		{
			continue;
		}

		// 获取首个子设备名（维护组以首个子设备来看）
		const QString& strSubDevName = it.value().first();

		// 组名和设备名相同，则组为空
		QString strGrpName = (it.key() == strSubDevName) ? "" : it.key();

		// 获取对应设备
		std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strSubDevName, strGrpName);
		if (spIDev == Q_NULLPTR)
		{
			continue;
		}

		// 压入
		vecNameDev.push_back({ it.key(), spIDev });
    }

	// 是否需要排序
	if (needSnSort)
	{
		// 将设备按照设备序列号从小到大排序
		std::sort(vecNameDev.begin(), vecNameDev.end(),[](const auto &a, const auto &b) {

			return (a.second->DeviceSn().toInt() < b.second->DeviceSn().toInt());
		});
	}

	// 遍历返回
	for (const auto& dev : vecNameDev)
	{
		// 获取设备类别,轨道则过滤
		if (dev.second->DeviceType() == int(tf::DeviceType::DEVICE_TYPE_TRACK))
		{
			// 联机版本则放入轨道
			if (DictionaryQueryManager::GetInstance()->GetPipeLine())
			{
				// 将组设备名压入列表(轨道放在最前面)——mod_tcx_bug0028673
				strDevNameList.push_front(dev.first);
			}
		}
		else
		{
			// 将组设备名压入列表
			strDevNameList.push_back(dev.first);
		}
	}

    return true;
}

///
/// @brief 获取设备序列号
///
/// @param[out]  vecDevSn  获取的设备序列号，按照sn升序排列
///
/// @return true:获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年9月13日，新建函数
///
bool UiDcsAdapter::GetDevSnVectorWithTrack(QVector<QString>& vecDevSn)
{
	// 清空
	vecDevSn.clear();

	// 联机版本则放入轨道
	bool isPip = DictionaryQueryManager::GetInstance()->GetPipeLine();
	
	vecDevSn = DeviceFactory::GetAllDeviceSnWithoutSingleTrack(isPip);

	return true;
}

///
/// @brief
///     获取组设备的子设备列表
///
/// @param[in]  strGrpDevName   组设备名
/// @param[in]  strDevNameList  子设备列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月18日，新建函数
///
bool UiDcsAdapter::GetSubDevNameList(const QString& strGrpDevName, QStringList& strDevNameList)
{
    // 清空子设备列表
    strDevNameList.clear();

    // 查找组设备
    auto it = m_mapDevGroup.find(strGrpDevName);

    // 如果没找到，则返回失败
    if (it == m_mapDevGroup.end())
    {
        return false;
    }

    // 赋值子设备列表
    strDevNameList = it.value();
    return true;
}

///
/// @brief
///     获取组设备的子设备列表，例如CH1,CH3A,CH3B
///
/// @param[in]  strDevNameList  子设备列表
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月30日，新建函数
///
bool UiDcsAdapter::GetSubDevCombineNameList(QStringList& strDevNameList)
{
	// 清空子设备列表
	strDevNameList.clear();

	QString strGrpDevName = m_strGrpName.isEmpty() ? m_strDevName : m_strGrpName;

	// 查找组设备
	auto it = m_mapDevGroup.find(strGrpDevName);

	// 如果没找到，则返回失败
	if (it == m_mapDevGroup.end())
	{
		return false;
	}

	// 获取对应设备
	std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
	if (spIDev == Q_NULLPTR)
	{
		return false;
	}

	// ISE组合名称单独处理,如果只有一个模块常规处理
	if (GetDeviceClasssify(m_strDevName, m_strGrpName) == DEVICE_CLASSIFY::DEVICE_CLASSIFY_ISE &&
		spIDev->ModuleCount() > 1)
	{
		// 获取设备信息
		int iModuleCnt = spIDev->ModuleCount();
		for (int moduleIndex = 1; moduleIndex <= iModuleCnt; moduleIndex++)
		{
			strDevNameList.push_back(m_strDevName + QString(QChar('A' + moduleIndex - 1)));
		}
	}
	else
	{
		// 赋值子设备列表
		strDevNameList = it.value();

		// 是否有子设备判断
		if (!strDevNameList.empty())
		{
			if (WhetherContainOtherSubDev(strGrpDevName))
			{
				for (int i = 0; i < strDevNameList.size(); i++)
				{
					strDevNameList[i] = strGrpDevName + strDevNameList[i];
				}
			}
		}

	}

	

	return true;
}

///
/// @brief
///     通过设备序列号获取设备名
///
/// @param[in]  strDevSn  设备序列号
///
/// @return (设备名，组名)
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月20日，新建函数
///
QPair<QString, QString> UiDcsAdapter::GetDevNameBySn(const QString& strDevSn)
{
    // 获取设备名列表
    QList<QPair<QString, QString>> strDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是ISE设备则跳过
        if (spIDev->DeviceSn() != strDevSn)
        {
            continue;
        }

        return strDevName;
    }

    return QPair<QString, QString>();
}

///
/// @brief
///     根据设备名获取设备序列号
///
/// @param[in]  strDevName  设备名
/// @param[in]  strGrpName  组名
///
/// @return 设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
QString UiDcsAdapter::GetDevSnByName(const QString& strDevName, const QString strGrpName /*= ""*/)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName, strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return "";
    }

    // 返回设备序列号
    return spIDev->DeviceSn();
}

///
/// @brief
///     通过设备序列号获取设备类型
///
/// @param[in]  strDevSn  设备序列号
///
/// @return 设备名
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
///
tf::DeviceType::type UiDcsAdapter::GetDevTypeBySn(const QString& strDevSn)
{
	// 获取设备名列表
	QList<QPair<QString, QString>> strDevNameList = DeviceFactory::GetAllDeviceName();
	for (const auto& strDevName : strDevNameList)
	{
		// 获取对应设备
		std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
		if (spIDev == Q_NULLPTR)
		{
			continue;
		}

		// 是ISE设备则跳过
		if (spIDev->DeviceSn() != strDevSn)
		{
			continue;
		}

		return static_cast<tf::DeviceType::type>(spIDev->DeviceType());
	}

	return tf::DeviceType::DEVICE_TYPE_INVALID;

}

///
/// @brief
///     获取ISE以外的设备名列表
///
/// @param[in]  strDevNameList  ISE以外设备名列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年2月3日，新建函数
///
bool UiDcsAdapter::GetDevNameListButIse(QList<QPair<QString, QString>>& strDevNameList)
{
    // 清空输出缓冲区
    strDevNameList.clear();
    QList<QPair<QString, QString>> strAllDevNameList = DeviceFactory::GetAllDeviceName();
    for (const auto& strDevName : strAllDevNameList)
    {
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 是轨道设备则跳过
        if (spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 是ISE设备则跳过
        if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
        {
            continue;
        }

        strDevNameList.push_back(strDevName);
    }

    return true;
}

///
/// @brief
///     获取组设备列表（ISE设备除外）
///
/// @param[in]  strDevNameList  获取组设备和独立设备列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月18日，新建函数
///
bool UiDcsAdapter::GetGroupDevNameListButIse(QStringList& strDevNameList)
{
    // 清空设备列表
    strDevNameList.clear();

    // 遍历容器
    for (auto it = m_mapDevGroup.begin(); it != m_mapDevGroup.end(); it++)
    {
        // 默认置为不是ISE设备
        bool bIseDev = false;
        bool bTrackDev = false;
        for (const QString& strDevName : it.value())
        {
            // 获取对应设备
            std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName);
            if (spIDev == Q_NULLPTR)
            {
                continue;
            }

            // 是轨道设备则跳过
            if (spIDev->DeviceType() == tf::DeviceType::DEVICE_TYPE_TRACK)
            {
                bTrackDev = true;
                break;
            }

            // 是ISE设备则跳过
            if (spIDev->DeviceClass() == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
            {
                bIseDev = true;
                break;
            }
        }

        // 如果包含ISE子设备，则跳过
        if (bIseDev || bTrackDev)
        {
            continue;
        }

        // 将组设备名压入列表
        strDevNameList.push_back(it.key());
    }

    return true;
}

///
/// @brief
///     获取质控品名称列表
///
/// @param[out]  strQcDocList  质控品名称列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::GetQcDocList(QStringList& strQcDocList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_DOC;
    int iDataOpCode = int(DATA_OPERATE_TYPE_QUERY);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        strQcDocList.push_back(boost::any_cast<QString>(outData));
    }

    return true;
}

///
/// @brief
///     获取ISE以外的质控品名称列表
///
/// @param[out]  strQcDocList  ISE以外的质控品名称列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年2月3日，新建函数
///
bool UiDcsAdapter::GetQcDocListButIse(QStringList& strQcDocList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice("");
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(-1);

    // 构造参数
    int iDataCode = DATA_CODE_QC_DOC;
    int iDataOpCode = int(DATA_OPERATE_TYPE_QUERY);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数
    paramList.push_back(tf::AssayClassify::ASSAY_CLASSIFY_ISE);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        strQcDocList.push_back(boost::any_cast<QString>(outData));
    }

    return true;
}

///
/// @brief
///     查询质控申请信息
///
/// @param[in]  strQcName        质控名称
/// @param[out] stuQcApplyInfo   质控申请信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::QueryQcApplyTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo)
{
    // 清空输出缓冲区
    stuQcApplyInfo.clear();

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_APPLY;
    int iDataOpCode = int(DATA_OPERATE_TYPE_QUERY);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(strQcName);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuQcApplyInfo.push_back(boost::any_cast<QC_APPLY_INFO>(outData));
    }

    AdapterDataBuffer::GetInstance()->ResetQcApply();
    return true;
}

///
/// @brief
///     更新质控申请信息是否选择
///
/// @param[in&out]  stuQcApplyItem  质控申请信息(传入原质控申请信息，选择更新后传回新的质控申请信息)
/// @param[in]      bSelected       是否选择
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::UpdateQcApplyItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected /*= true*/)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_APPLY;
    int iDataOpCode = bSelected ? int(DATA_OPERATE_TYPE_ADD) : int(DATA_OPERATE_TYPE_DELETE);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcApplyItem);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcApplyItem = boost::any_cast<QC_APPLY_INFO>(outBuffer.front());
    return true;
}

///
/// @brief
///     查询默认质控信息
///
/// @param[in]  strQcName        质控名称
/// @param[out] stuQcApplyInfo   质控申请信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::QueryDefaultQcTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo)
{
    // 清空输出缓冲区
    stuQcApplyInfo.clear();

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_DEFAULT_QC;
    int iDataOpCode = int(DATA_OPERATE_TYPE_QUERY);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(strQcName);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuQcApplyInfo.push_back(boost::any_cast<QC_APPLY_INFO>(outData));
    }

    AdapterDataBuffer::GetInstance()->ResetQcApply();
    return true;
}

///
/// @brief
///     更新默认质控信息是否选择
///
/// @param[in]  stuQcApplyItem  质控申请信息
/// @param[in]  bSelected       是否选择
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::UpdateDefaultQcItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected /*= true*/)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_DEFAULT_QC;
    int iDataOpCode = bSelected ? int(DATA_OPERATE_TYPE_ADD) : int(DATA_OPERATE_TYPE_DELETE);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcApplyItem);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcApplyItem = boost::any_cast<QC_APPLY_INFO>(outBuffer.front());
    return true;
}

///
/// @brief
///     查询备用瓶质控信息
///
/// @param[in]  strQcName        质控名称
/// @param[out] stuQcApplyInfo   质控申请信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::QueryBackupRgntQcTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo)
{
    // 清空输出缓冲区
    stuQcApplyInfo.clear();

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_BACKUP_RGNT_QC;
    int iDataOpCode = int(DATA_OPERATE_TYPE_QUERY);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(strQcName);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuQcApplyInfo.push_back(boost::any_cast<QC_APPLY_INFO>(outData));
    }

    AdapterDataBuffer::GetInstance()->ResetQcApply();
    return true;
}

///
/// @brief
///     更新备用瓶质控信息是否选择
///
/// @param[in]  stuQcApplyItem  质控申请信息
/// @param[in]  bSelected       是否选择
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
bool UiDcsAdapter::UpdateBackupRgntItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected /*= true*/)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_BACKUP_RGNT_QC;
    int iDataOpCode = bSelected ? int(DATA_OPERATE_TYPE_ADD) : int(DATA_OPERATE_TYPE_DELETE);
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcApplyItem);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出的质控结果信息
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcApplyItem = boost::any_cast<QC_APPLY_INFO>(outBuffer.front());
    return true;
}

///
/// @brief
///     获取项目列表
///
/// @param[in]  strAssayList  项目列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::GetAssayList(QStringList& strAssayList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_ASSAY_NAME_LIST;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        strAssayList.push_back(boost::any_cast<QString>(outData));
    }

    return true;
}

/// @brief
///     获取质控LJ项目列表
///
/// @param[in]  strAssayList  项目列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::GetQcLjAssayList(QStringList& strAssayList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_RLT_ASSAY_LIST_SINGLE;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        strAssayList.push_back(boost::any_cast<QString>(outData));
    }

    return true;
}

/// @brief
///     获取质控TP项目列表
///
/// @param[in]  strAssayList  项目列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::GetQcTpAssayList(QStringList& strAssayList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_RLT_ASSAY_LIST_TWIN;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        strAssayList.push_back(boost::any_cast<QString>(outData));
    }

    return true;
}

///
/// @brief
///     查询质控靶值信息
///
/// @param[in]   qryCond            查询条件
/// @param[out]  stuQcConcInfoList  查询结果列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::QueryQcDocConcInfo(const QC_CONC_INFO_QUERY_COND& qryCond, QList<QC_DOC_CONC_INFO>& stuQcConcInfoList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Failed to execute GetDevice(%s, %s)", m_strDevName, m_strGrpName);
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_CONC_INFO;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数
    paramList.push_back(qryCond);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        ULOG(LOG_WARN, "Failed to execute OperateData(%d, %d, )", iDataCode, iDataOpCode);
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuQcConcInfoList.push_back(boost::any_cast<QC_DOC_CONC_INFO>(outData));
    }

    return true;
}

///
/// @brief
///     查询质控结果信息
///
/// @param[in]   qryCond            查询条件
/// @param[out]  stuQcRltList       查询结果列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::QueryQcRltInfo(const QC_RESULT_QUERY_COND& qryCond, QList<QC_RESULT_INFO>& stuQcRltList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_RLT_INFO;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(qryCond);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuQcRltList.push_back(boost::any_cast<QC_RESULT_INFO>(outData));
    }

    return true;
}

///
/// @brief
///     更新靶值SD
///
/// @param[in]  stuDocInfo      质控品信息
/// @param[in]  strAssayName    项目名
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月27日，新建函数
///
bool UiDcsAdapter::UpdateQcTargetValSD(::tf::ResultLong& _return, const QC_DOC_CONC_INFO& stuDocInfo, const QString& strAssayName)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_TARGET_VAL_SD;
    int iDataOpCode = DATA_OPERATE_TYPE_MODIFY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuDocInfo);
    paramList.push_back(strAssayName);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
		// 返回失败原因
		if (!outBuffer.empty())
		{
			_return = boost::any_cast<::tf::ResultLong>(outBuffer.front());
		}

        return false;
    }

    return true;
}

///
/// @brief
///     更新质控失控原因和处理措施
///
/// @param[in]  stuQcRlt        质控结果
/// @param[in]  strReason       失控原因
/// @param[in]  strSolution     处理措施
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月29日，新建函数
///
bool UiDcsAdapter::UpdateQcOutCtrlReasonAndSolution(QC_RESULT_INFO& stuQcRlt, const QString& strReason, const QString& strSolution)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_REASON_SOLUTION_SINGLE;
    int iDataOpCode = DATA_OPERATE_TYPE_MODIFY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcRlt);
    paramList.push_back(strReason);
    paramList.push_back(strSolution);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出的质控结果信息
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcRlt = boost::any_cast<QC_RESULT_INFO>(outBuffer.front());
    return true;
}

///
/// @brief
///     更新质控失控原因和处理措施
///
/// @param[in]  stuQcRlt        质控结果
/// @param[in]  strReason       失控原因
/// @param[in]  strSolution     处理措施
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月3日，新建函数
///
bool UiDcsAdapter::UpdateQcOutCtrlReasonAndSolution(TWIN_QC_RESULT_INFO& stuQcRlt, const QString& strReason, const QString& strSolution)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_REASON_SOLUTION_TWIN;
    int iDataOpCode = DATA_OPERATE_TYPE_MODIFY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcRlt);
    paramList.push_back(strReason);
    paramList.push_back(strSolution);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出的质控结果信息
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcRlt = boost::any_cast<TWIN_QC_RESULT_INFO>(outBuffer.front());
    return true;
}

///
/// @brief
///     更新质控计算点
///
/// @param[in]  stuQcRlt    质控结果
/// @param[in]  bCalculate  是否计算
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月29日，新建函数
///
bool UiDcsAdapter::UpdateQcCalcPoint(QC_RESULT_INFO& stuQcRlt, bool bCalculate)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_CALCULATE_POINT_SINGLE;
    int iDataOpCode = DATA_OPERATE_TYPE_MODIFY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcRlt);
    paramList.push_back(bCalculate);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出的质控结果信息
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcRlt = boost::any_cast<QC_RESULT_INFO>(outBuffer.front());

    return true;
}

///
/// @brief
///     更新联合质控计算点
///
/// @param[in]  stuQcRlt    质控结果
/// @param[in]  bCalculate  是否计算
/// @param[in]  bTwinQc     是否是联合质控
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月29日，新建函数
///
bool UiDcsAdapter::UpdateQcCalcPoint(TWIN_QC_RESULT_INFO& stuQcRlt, bool bCalculate)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_CALCULATE_POINT_TWIN;
    int iDataOpCode = DATA_OPERATE_TYPE_MODIFY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(stuQcRlt);
    paramList.push_back(bCalculate);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        return false;
    }

    // 获取输出的质控结果信息
    if (outBuffer.empty())
    {
        return false;
    }

    stuQcRlt = boost::any_cast<TWIN_QC_RESULT_INFO>(outBuffer.front());

    return true;
}

///
/// @brief
///     查询联合质控信息
///
/// @param[in]  qryCond             查询条件
/// @param[in]  stuTwinQcRltList    结果列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月30日，新建函数
///
bool UiDcsAdapter::QueryQcYoudenRltInfo(const QC_RESULT_QUERY_COND& qryCond, QList<TWIN_QC_RESULT_INFO>& stuTwinQcRltList)
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Failed to execute GetDevice(%s, %s)", m_strDevName, m_strGrpName);
        return false;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    // 构造参数
    int iDataCode = DATA_CODE_QC_TWIN_RLT_INFO;
    int iDataOpCode = DATA_OPERATE_TYPE_QUERY;
    QList<boost::any> paramList;
    std::list<boost::any> outBuffer;

    // 构造参数列表
    paramList.push_back(qryCond);

    // 调用操作数据接口
    if (!spIDev->OperateData(iDataCode, iDataOpCode, paramList, outBuffer))
    {
        ULOG(LOG_WARN, "Failed to execute OperateData(%d, %d, )", iDataCode, iDataOpCode);
        return false;
    }

    // 获取输出缓存
    for (auto& outData : outBuffer)
    {
        stuTwinQcRltList.push_back(boost::any_cast<TWIN_QC_RESULT_INFO>(outData));
    }

    return true;
}

///
/// @brief
///     获取耗材需求信息
///
/// @param[out] stuSplReqInfoList   耗材需求信息列表
/// @param[in]  iDevClassify        设备类别
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月25日，新建函数
///
bool UiDcsAdapter::GetSplReqInfo(QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList, int iDevClassify)
{
    // 默认返回true
    bool bOk = true;

    // 清空缓存
    stuSplReqInfoList.clear();

    // 构建映射
    QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO> mapSplReqInfo;

    // 生化
    if (iDevClassify & DEVICE_CLASSIFY_CHEMISTRY)
    {
        // 加载生化仓内试剂和耗材信息
        bOk = bOk && LoadChRgntSpyInfoToMap(mapSplReqInfo);
    }

    // 免疫
    if (iDevClassify & DEVICE_CLASSIFY_IMMUNE)
    {
        // 加载免疫仓内试剂信息
        bOk = bOk && LoadImRgntInfoToMap(mapSplReqInfo);
        bOk = bOk && LoadImDltInfoToMap(mapSplReqInfo);
    }

    // 加载耗材动态计算结果
    bOk = bOk && LoadDynCalcInfoToMap(mapSplReqInfo, iDevClassify);

    // 加载耗材需求信息
    bOk = bOk && LoadSplReqInfoToMap(mapSplReqInfo, iDevClassify);

    // 构造显示字符串字段
    bOk = bOk && MakeStrForSplReqInfo(mapSplReqInfo, stuSplReqInfoList);

    return bOk;
}

///
/// @brief
///     保存耗材需求信息
///
/// @param[in]  stuSplReqInfoList  耗材需求信息列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
bool UiDcsAdapter::SaveSplReqInfo(const QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList)
{
    // 遍历信息
    QVector<tf::SplRequireInfo> vecSplReqInfo;
    for (const auto& stuSplReqInfo : stuSplReqInfoList)
    {
        // 构造一个耗材需求信息
        tf::SplRequireInfo stuTfSplReqInfo;

        // 有ID则修改
        if (stuSplReqInfo.lReqID > 0)
        {
            stuTfSplReqInfo.__set_id(stuSplReqInfo.lReqID);
        }
        else
        {
            stuTfSplReqInfo.__set_code(stuSplReqInfo.iSplCode);
            stuTfSplReqInfo.__set_bIsRgnt(stuSplReqInfo.bIsRgnt);
            stuTfSplReqInfo.__set_deviceClassify(tf::AssayClassify::type(stuSplReqInfo.iAssayClassify));
        }

        stuTfSplReqInfo.__set_bottleSpec(stuSplReqInfo.iBottleSpec);
        if (stuSplReqInfo.vecIReqVal.size() >= Qt::Sunday)
        {
            stuTfSplReqInfo.__set_reqAtMon(stuSplReqInfo.vecIReqVal[Qt::Monday - 1]);
            stuTfSplReqInfo.__set_reqAtTue(stuSplReqInfo.vecIReqVal[Qt::Tuesday - 1]);
            stuTfSplReqInfo.__set_reqAtWed(stuSplReqInfo.vecIReqVal[Qt::Wednesday - 1]);
            stuTfSplReqInfo.__set_reqAtThur(stuSplReqInfo.vecIReqVal[Qt::Thursday - 1]);
            stuTfSplReqInfo.__set_reqAtFri(stuSplReqInfo.vecIReqVal[Qt::Friday - 1]);
            stuTfSplReqInfo.__set_reqAtSat(stuSplReqInfo.vecIReqVal[Qt::Saturday - 1]);
            stuTfSplReqInfo.__set_reqAtSun(stuSplReqInfo.vecIReqVal[Qt::Sunday - 1]);
        }

        vecSplReqInfo.push_back(stuTfSplReqInfo);
    }

    // 遍历要保存的数据
    for (auto stuTfSplReqInfo : vecSplReqInfo)
    {
        // 有ID则修改
        if (stuTfSplReqInfo.__isset.id)
        {
            DcsControlProxy::GetInstance()->ModifySplReqInfo(stuTfSplReqInfo);
        }
        else
        {
            // 否则则添加
            tf::ResultLong ret;
            DcsControlProxy::GetInstance()->AddSplReqInfo(ret, stuTfSplReqInfo);
        }
    }

    return true;
}

///
/// @brief
///     加载免疫试剂信息到耗材需求映射表
///
/// @param[in]  mapSplReqInfo  耗材需求映射表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月27日，新建函数
///
bool UiDcsAdapter::LoadImRgntInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo)
{
    // 获取所有免疫设备序列号
    std::vector<std::string> deviceSn;
    std::vector<tf::DeviceType::type> deviceTypes;
    deviceTypes.push_back(tf::DeviceType::DEVICE_TYPE_I6000);
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecSpDevInfo = CIM_INSTANCE->GetDeviceFromType(deviceTypes);
    for (auto spDevInfo : vecSpDevInfo)
    {
        // 指针为空则跳过
        if (spDevInfo == Q_NULLPTR)
        {
            continue;
        }

        deviceSn.push_back(spDevInfo->deviceSN);
    }

    // 查询所有试剂信息
    // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
    ::im::tf::ReagentInfoTableQueryResp qryRgntResp;
    ::im::tf::ReagTableUIQueryCond qryRgntCond;

    // 筛选条件
    qryRgntCond.__set_deviceSNs(deviceSn);

    // 执行查询
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryRgntResp, qryRgntCond);
    if (!bRet || qryRgntResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    // 遍历结果
    for (const auto& stuRgntInfo : qryRgntResp.lstReagentInfos)
    {
        // 获取对应设备
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CIM_INSTANCE->GetDeviceInfo(stuRgntInfo.deviceSN);
        if (spDevInfo == Q_NULLPTR)
        {
            continue;;
        }

        // 余量
        int iResidualTestNum = (stuRgntInfo.residualTestNum > 0 && stuRgntInfo.bottleIdx == 0) ? stuRgntInfo.residualTestNum : 0;

        // 构造map中的key
        std::tuple<int, bool, int> tupKey(spDevInfo->deviceClassify, false, stuRgntInfo.assayCode);

        // 在map中查找对应项
        auto it = mapSplReqInfo.find(tupKey);

        // 如果没有则添加
        if (it == mapSplReqInfo.end())
        {
            // 获取项目信息
            SPL_REQ_VOL_INFO stuSplReqInfo;
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CIM_INSTANCE->GetAssayInfo(stuRgntInfo.assayCode);
            if (Q_NULLPTR == spAssayInfo)
            {
                continue;
            }

            // 构造需求信息
            stuSplReqInfo.lReqID = -1;
            stuSplReqInfo.strSplName = QString::fromStdString(spAssayInfo->assayName);
            stuSplReqInfo.iSplCode = stuRgntInfo.assayCode;
            stuSplReqInfo.bIsRgnt = true;
            stuSplReqInfo.iAssayClassify = spDevInfo->deviceClassify;
            stuSplReqInfo.mapDevResidual.insert(QString::fromStdString(spDevInfo->name), iResidualTestNum);
            stuSplReqInfo.iCurResidual = iResidualTestNum;
            stuSplReqInfo.iBottleSpec = -1;
            mapSplReqInfo.insert(tupKey, stuSplReqInfo);
            continue;
        }

        // 找到对应项，则更新
        auto itDevResidual = it.value().mapDevResidual.find(QString::fromStdString(spDevInfo->name));
        if (itDevResidual == it.value().mapDevResidual.end())
        {
            it.value().mapDevResidual.insert(QString::fromStdString(spDevInfo->name), iResidualTestNum);
        }
        else
        {
            itDevResidual.value() += iResidualTestNum;
        }

        // 更新当前余量
        it.value().iCurResidual += iResidualTestNum;
    }

    return true;
}

///
/// @brief
///     加载免疫稀释液信息到耗材需求映射表
///
/// @param[in]  mapSplReqInfo  耗材需求映射表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月27日，新建函数
///
bool UiDcsAdapter::LoadImDltInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo)
{
    // 获取所有免疫设备序列号
    std::vector<std::string> deviceSn;
    std::vector<tf::DeviceType::type> deviceTypes;
    deviceTypes.push_back(tf::DeviceType::DEVICE_TYPE_I6000);
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecSpDevInfo = CIM_INSTANCE->GetDeviceFromType(deviceTypes);
    for (auto spDevInfo : vecSpDevInfo)
    {
        // 指针为空则跳过
        if (spDevInfo == Q_NULLPTR)
        {
            continue;
        }

        deviceSn.push_back(spDevInfo->deviceSN);
    }

    // 查询所有稀释液信息
    // 构造查询条件和查询结果
    ::im::tf::DiluentInfoTableQueryResp qryDltResp;
    ::im::tf::ReagTableUIQueryCond qryDltCond;

    // 筛选条件
    qryDltCond.__set_deviceSNs(deviceSn);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryDiluentInfoForUI(qryDltResp, qryDltCond);
    if (!bRet || qryDltResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    // 遍历结果
    for (const auto& stuDltInfo : qryDltResp.lstDiluentInfos)
    {
        // 获取对应设备
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CIM_INSTANCE->GetDeviceInfo(stuDltInfo.deviceSN);
        if (spDevInfo == Q_NULLPTR)
        {
            continue;;
        }

        // 余量
        double dResidualVol = stuDltInfo.curVol > 0 ? stuDltInfo.curVol / 1000.0 : 0;

        // 获取稀释液类型
        im::tf::SuppliesType::type enDltType = im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT;
        if (IsSpecialWashing(stuDltInfo))
        {
            enDltType = im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH;
        }

        // 构造map中的key;免疫的耗材编号需要转换成全局的（::im::tf::SuppliesType::type 转 tf::ConsumablesType::type）
        std::tuple<int, bool, int> tupKey(spDevInfo->deviceClassify, true, stuDltInfo.diluentNumber);

        // 在map中查找对应项
        auto it = mapSplReqInfo.find(tupKey);

        // 如果没有则添加
        if (it == mapSplReqInfo.end())
        {
            // 构造需求信息
            SPL_REQ_VOL_INFO stuSplReqInfo;
            stuSplReqInfo.lReqID = -1;
            stuSplReqInfo.iSplCode = stuDltInfo.diluentNumber;
            stuSplReqInfo.bIsRgnt = false;
            stuSplReqInfo.iAssayClassify = spDevInfo->deviceClassify;
            stuSplReqInfo.strSplName =QString::fromStdString(stuDltInfo.diluentName);
            stuSplReqInfo.mapDevResidual.insert(QString::fromStdString(spDevInfo->name), dResidualVol);
            stuSplReqInfo.iCurResidual = dResidualVol;
            stuSplReqInfo.iBottleSpec = -1;
            mapSplReqInfo.insert(tupKey, stuSplReqInfo);
            continue;
        }

        // 找到对应项，则更新
        auto itDevResidual = it.value().mapDevResidual.find(QString::fromStdString(spDevInfo->name));
        if (itDevResidual == it.value().mapDevResidual.end())
        {
            it.value().mapDevResidual.insert(QString::fromStdString(spDevInfo->name), dResidualVol);
        }
        else
        {
            itDevResidual.value() += dResidualVol;
        }

        // 更新当前余量
        it.value().iCurResidual += dResidualVol;
    }

    return true;
}

bool UiDcsAdapter::LoadChRgntSpyInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo)
{
    // 获取所有生化设备
    std::vector<tf::DeviceType::type> deviceTypes;
    deviceTypes.push_back(tf::DeviceType::DEVICE_TYPE_C1000);
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecSpDevInfo = CIM_INSTANCE->GetDeviceFromType(deviceTypes);

	// 遍历设备信息
    for (auto spDevInfo : vecSpDevInfo)
    {
        // 指针为空则跳过
        if (spDevInfo == nullptr)
        {
            continue;
        }

		// 获取指定设备序列号的仓内信息
		auto mapChSi = CIM_INSTANCE->GetChDiskReagentSupplies(spDevInfo->deviceSN);
		for (const auto &pair : mapChSi)
		{
			auto si = pair.second.supplyInfo;

			// 无效试剂则跳过
			if (si.suppliesCode <= 0)
			{
				continue;
			}

			// 仓内试剂
			if (si.type == ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
			{
				LoadChRgtInfo(spDevInfo, pair.second.reagentInfos, mapSplReqInfo);
			}
			// 仓内耗材
			else
			{
				LoadChSpyInfo(spDevInfo, si, mapSplReqInfo);
			}
		}
    }

    return true;
}

void UiDcsAdapter::LoadChRgtInfo(std::shared_ptr<const tf::DeviceInfo> spDevInfo, const std::vector<::ch::tf::ReagentGroup>& rgtInfos,
	QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo)
{
	for (const auto& rgt : rgtInfos)
	{
		// 依次计算腔信息，总量
		int iResidualVal = rgt.remainCount;

		// 液位探测失败和项目未分配的将余量置为0
		bool unAllocated = CIM_INSTANCE->IsUnAllocatedAssay(spDevInfo->deviceSN, rgt.assayCode);
		if (rgt.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL
			|| unAllocated)
		{
			iResidualVal = 0;
		}

		// 组合设备名（组名+设备名）
		QString strDevName = QString::fromStdString(spDevInfo->groupName) + QString::fromStdString(spDevInfo->name);

		// 获取项目名称
		QString strSplName = QString::number(rgt.assayCode);
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CIM_INSTANCE->GetAssayInfo(rgt.assayCode);
		if (spAssayInfo != nullptr)
		{
			strSplName = QString::fromStdString(spAssayInfo->assayName);
		}

		// 构造map中的key
		std::tuple<int, bool, int> tupKey(spDevInfo->deviceClassify, false, rgt.assayCode);

		// 在map中查找对应项
		auto it = mapSplReqInfo.find(tupKey);

		// 如果没有则添加
		if (it == mapSplReqInfo.end())
		{
			// 构造需求信息
			SPL_REQ_VOL_INFO stuSplReqInfo;
			stuSplReqInfo.lReqID = -1;
			stuSplReqInfo.iSplCode = rgt.assayCode;
			stuSplReqInfo.bIsRgnt = true;
			stuSplReqInfo.iAssayClassify = spDevInfo->deviceClassify;
			stuSplReqInfo.strSplName = strSplName;
			stuSplReqInfo.mapDevResidual.insert(strDevName, iResidualVal);
			stuSplReqInfo.iCurResidual = iResidualVal;
			stuSplReqInfo.iBottleSpec = -1;
			mapSplReqInfo.insert(tupKey, stuSplReqInfo);
			continue;
		}

		// 找到对应项，则更新
		auto itDevResidual = it.value().mapDevResidual.find(strDevName);
		if (itDevResidual == it.value().mapDevResidual.end())
		{
			it.value().mapDevResidual.insert(strDevName, iResidualVal);
		}
		else
		{
			itDevResidual.value() += iResidualVal;
		}

		// 更新当前余量
		it.value().iCurResidual += iResidualVal;
	}
}

void UiDcsAdapter::LoadChSpyInfo(std::shared_ptr<const tf::DeviceInfo> spDevInfo, const ::ch::tf::SuppliesInfo& supplyInfo,
	QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo)
{
	// 依次计算腔信息，总量
	int iResidualVal = GetSupplyResidual(supplyInfo);
	int iResidualValML = 0;
	if (iResidualVal <= 0)
	{
		iResidualValML = 0;
	}
	else
	{
		iResidualValML = (iResidualVal >= 10000) ? (iResidualVal / 10000) : 1;
	}

	// 获取耗材名称
	QString strSplName = ThriftEnumTrans::GetSupplyName(supplyInfo.type, true);

	// 组合设备名（组名+设备名）
	QString strDevName = QString::fromStdString(spDevInfo->groupName) + QString::fromStdString(spDevInfo->name);

	// 构造map中的key
	std::tuple<int, bool, int> tupKey(spDevInfo->deviceClassify, true, supplyInfo.suppliesCode);

	// 在map中查找对应项
	auto it = mapSplReqInfo.find(tupKey);

	// 如果没有则添加
	if (it == mapSplReqInfo.end())
	{
		// 构造需求信息
		SPL_REQ_VOL_INFO stuSplReqInfo;
		stuSplReqInfo.lReqID = -1;
		stuSplReqInfo.iSplCode = supplyInfo.suppliesCode;
		stuSplReqInfo.bIsRgnt = false;
		stuSplReqInfo.iAssayClassify = spDevInfo->deviceClassify;
		stuSplReqInfo.strSplName = strSplName;
		stuSplReqInfo.mapDevResidual.insert(strDevName, iResidualValML);
		stuSplReqInfo.iCurResidual = iResidualValML;
		stuSplReqInfo.iBottleSpec = -1;
		mapSplReqInfo.insert(tupKey, stuSplReqInfo);
		return;
	}

	// 找到对应项，则更新
	auto itDevResidual = it.value().mapDevResidual.find(strDevName);
	if (itDevResidual == it.value().mapDevResidual.end())
	{
		it.value().mapDevResidual.insert(strDevName, iResidualValML);
	}
	else
	{
		itDevResidual.value() += iResidualValML;
	}

	// 更新当前余量
	it.value().iCurResidual += iResidualValML;
}

///
/// @brief
///     加载耗材需求映射表
///
/// @param[in]  mapSplReqInfo  耗材需求映射表
/// @param[in]  iDevClassify   设备类别
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月27日，新建函数
///
bool UiDcsAdapter::LoadSplReqInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, int iDevClassify)
{
    RgntReqCalcCfg stuReqCalcCfg;
    if (!DictionaryQueryManager::GetDynCalcConfig(stuReqCalcCfg))
    {
        ULOG(LOG_WARN, "Failed to get dyncalc config.");
        return false;
    }

    // 查询耗材需求信息表
    // 构造查询条件和查询结果
    tf::SplRequireInfoQueryResp qrySplReqResp;
    tf::SplRequireInfoQueryCond qrySplReqCond;

    // 查询所有试剂信息
    bool bRet = DcsControlProxy::GetInstance()->QuerySplReqInfo(qrySplReqResp, qrySplReqCond);
    if (!bRet || qrySplReqResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    // 遍历结果
    for (const auto& stuTfSplReqInfo : qrySplReqResp.lstSplRequireInfos)
    {
        // 设备类别不匹配则跳过
        if (!(iDevClassify & stuTfSplReqInfo.deviceClassify))
        {
            continue;
        }

        // 构造map中的key
        std::tuple<int, bool, int> tupKey(stuTfSplReqInfo.deviceClassify, !stuTfSplReqInfo.bIsRgnt, stuTfSplReqInfo.code);

        // 在map中查找对应项
        auto it = mapSplReqInfo.find(tupKey);

        // 如果没有则添加
        if (it == mapSplReqInfo.end())
        {
            // 获取项目信息
            SPL_REQ_VOL_INFO stuSplReqInfo;

            // 获取耗材名称
            QString strSplName = GetNameBySplReqInfo(stuTfSplReqInfo);
            if (strSplName.isEmpty())
            {
                continue;
            }

            // 构造需求信息
            stuSplReqInfo.lReqID = stuTfSplReqInfo.id;
            stuSplReqInfo.strSplName = strSplName;
            stuSplReqInfo.iSplCode = stuTfSplReqInfo.code;
            stuSplReqInfo.bIsRgnt = stuTfSplReqInfo.bIsRgnt;
            stuSplReqInfo.iAssayClassify = stuTfSplReqInfo.deviceClassify;
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtMon);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtTue);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtWed);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtThur);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtFri);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtSat);
            stuSplReqInfo.vecIReqVal.push_back(stuTfSplReqInfo.reqAtSun);
            stuSplReqInfo.iBottleSpec = stuTfSplReqInfo.bottleSpec;
            if (stuReqCalcCfg.bEnable)
            {
                // 启用动态计算
                stuSplReqInfo.vecILackingVal = stuSplReqInfo.vecIReqDynCalcVal;
            }
            else
            {
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtMon);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtTue);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtWed);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtThur);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtFri);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtSat);
                stuSplReqInfo.vecILackingVal.push_back(stuTfSplReqInfo.reqAtSun);
            }

            if (stuSplReqInfo.iBottleSpec > 0)
            {
                std::for_each(stuSplReqInfo.vecILackingVal.begin(), stuSplReqInfo.vecILackingVal.end(),
                    [&stuSplReqInfo](int iLack)
                {
                    int iBottleLacking = (iLack / stuSplReqInfo.iBottleSpec) + int(iLack % stuSplReqInfo.iBottleSpec != 0);
                    stuSplReqInfo.vecStrBottleLacking.push_back(iBottleLacking > 0 ? QString::number(iBottleLacking) : "");
                });
            }
            stuSplReqInfo.iCurResidual = 0;
            mapSplReqInfo.insert(tupKey, stuSplReqInfo);
            continue;
        }

        // 找到对应项，则更新
        it.value().lReqID = stuTfSplReqInfo.id;
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtMon);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtTue);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtWed);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtThur);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtFri);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtSat);
        it.value().vecIReqVal.push_back(stuTfSplReqInfo.reqAtSun);
        it.value().iBottleSpec = stuTfSplReqInfo.bottleSpec;
        if (stuReqCalcCfg.bEnable)
        {
            std::for_each(it.value().vecIReqDynCalcVal.begin(), it.value().vecIReqDynCalcVal.end(),
                [&it](int iDynReq)
            {
                it.value().vecILackingVal.push_back(iDynReq > it.value().iCurResidual ? iDynReq - it.value().iCurResidual : 0);
            });
        }
        else
        {
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtMon > it.value().iCurResidual ? stuTfSplReqInfo.reqAtMon - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtTue > it.value().iCurResidual ? stuTfSplReqInfo.reqAtTue - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtWed > it.value().iCurResidual ? stuTfSplReqInfo.reqAtWed - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtThur > it.value().iCurResidual ? stuTfSplReqInfo.reqAtThur - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtFri > it.value().iCurResidual ? stuTfSplReqInfo.reqAtFri - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtSat > it.value().iCurResidual ? stuTfSplReqInfo.reqAtSat - it.value().iCurResidual : 0);
            it.value().vecILackingVal.push_back(stuTfSplReqInfo.reqAtSun > it.value().iCurResidual ? stuTfSplReqInfo.reqAtSun - it.value().iCurResidual : 0);
        }
        if (it.value().iBottleSpec > 0)
        {
            std::for_each(it.value().vecILackingVal.begin(), it.value().vecILackingVal.end(),
                [&it](int iLack)
            {
                int iBottleLacking = (iLack / it.value().iBottleSpec) + int(iLack % it.value().iBottleSpec != 0);
            it.value().vecStrBottleLacking.push_back(iBottleLacking > 0 ? QString::number(iBottleLacking) : "");
            });
        }
    }

    // 遍历所有项目
    AssayIndexCodeMaps mapAssayInfo =  CIM_INSTANCE->GetAssayInfo();
    for (auto it = mapAssayInfo.begin(); it != mapAssayInfo.end(); it++)
    {
        // 设备类型不匹配则跳过
        if (!(iDevClassify & it->second->assayClassify))
        {
            continue;
        }

		// 排除L、H、I
		if (::ch::tf::g_ch_constants.ASSAY_CODE_L == it->second->assayCode
			|| ::ch::tf::g_ch_constants.ASSAY_CODE_H == it->second->assayCode
			|| ::ch::tf::g_ch_constants.ASSAY_CODE_I == it->second->assayCode)
		{
			continue;
		}

        // 如果已经存在需求则跳过
        // 构造map中的key
        std::tuple<int, bool, int> tupKey(it->second->assayClassify, false, it->second->assayCode);

        // 在map中查找对应项
        auto itSplReq = mapSplReqInfo.find(tupKey);
        if (itSplReq != mapSplReqInfo.end())
        {
            continue;
        }

        // 不存在则构造一个需求项
        // 获取项目信息
        SPL_REQ_VOL_INFO stuSplReqInfo;

        // 构造需求信息
        stuSplReqInfo.strSplName = QString::fromStdString(it->second->assayName);
        stuSplReqInfo.iSplCode = it->second->assayCode;
        stuSplReqInfo.bIsRgnt = true;
        stuSplReqInfo.iAssayClassify = it->second->assayClassify;
        stuSplReqInfo.iBottleSpec = -1;
        stuSplReqInfo.iCurResidual = 0;
        mapSplReqInfo.insert(tupKey, stuSplReqInfo);
    }

    return true;
}

///
/// @brief
///     加载耗材动态计算值
///
/// @param[out]  mapSplReqInfo  耗材需求映射表
/// @param[out]  iDevClassify   设备类别
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
bool UiDcsAdapter::LoadDynCalcInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, int iDevClassify)
{
    RgntReqCalcCfg stuReqCalcCfg;
    if (!DictionaryQueryManager::GetDynCalcConfig(stuReqCalcCfg))
    {
        ULOG(LOG_WARN, "Failed to get dyncalc config.");
        return false;
    }

    // 构造起止日期
    const int ciDaysOfWeek = 7;
    QDate qStartDate;
    QDate qEndDate = QDate::currentDate().addDays(-1);
    if (stuReqCalcCfg.iCircleUnit == tf::TimeUnit::TIME_UNIT_DAY)
    {
        qStartDate = QDate::currentDate().addDays(-stuReqCalcCfg.iCircle);
    }
    else if (stuReqCalcCfg.iCircleUnit == tf::TimeUnit::TIME_UNIT_WEEK)
    {
        qStartDate = QDate::currentDate().addDays(-stuReqCalcCfg.iCircle * ciDaysOfWeek);
    }
    else if (stuReqCalcCfg.iCircleUnit == tf::TimeUnit::TIME_UNIT_MONTH)
    {
        qStartDate = QDate::currentDate().addMonths(-stuReqCalcCfg.iCircle);
    }
    else if (stuReqCalcCfg.iCircleUnit == tf::TimeUnit::TIME_UNIT_YEAR)
    {
        qStartDate = QDate::currentDate().addYears(-stuReqCalcCfg.iCircle);
    }
    else
    {
        return false;
    }

    // 起止日期字符串格式化
    std::string strStartDate = qStartDate.toString(UI_DATE_FORMAT).toStdString();
    std::string strEndDate = qEndDate.toString(UI_DATE_FORMAT).toStdString();

    // 构造时间范围
    tf::TimeRangeCond trc;
    trc.__set_startTime(strStartDate);
    trc.__set_endTime(strEndDate);

    // 将动态计算结果更新到映射表中
    auto UpdateDunCalcRltToMap = [&mapSplReqInfo, &stuReqCalcCfg](const ::tf::DynamicCalcQueryResp& qryResp, QPair<int, bool> pairCond)
    {
        // 如果结果返回值不为成功，则直接返回
        if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return;
        }

        // 遍历结果
        for (const auto& stuDynCalcRlt : qryResp.lstDynamicCalcVals)
        {
            // 国外0代表周天，特殊处理0
            int iDayOfWeek = stuDynCalcRlt.dayOfWeek;
            if (iDayOfWeek == 0)
            {
                iDayOfWeek = Qt::Sunday;
            }

            // 参数检查
            if ((iDayOfWeek < Qt::Monday) || (iDayOfWeek > Qt::Sunday))
            {
                continue;
            }

            // 找到对应的需求计算数据
            auto tpKey = std::make_tuple(pairCond.first, !pairCond.second, stuDynCalcRlt.assayCode);
            auto it = mapSplReqInfo.find(tpKey);
            if (it == mapSplReqInfo.end())
            {
                continue;
            }

            // 查询需求计算数组是否已经赋值
            if (it.value().vecIReqDynCalcVal.size() < Qt::Sunday)
            {
                it.value().vecIReqDynCalcVal = QVector<int>(Qt::Sunday, 0);
            }

            if (it.value().vecStrReqDynCalcVal.size() < Qt::Sunday)
            {
                it.value().vecStrReqDynCalcVal = QVector<QString>(Qt::Sunday, "");
            }

            /// 获取单位
            QString strUnit = pairCond.second ? "" : "ml";

            // 如果是耗材，动态计算结果单位（0.1ul）需要转换为ml
            int iDynamicCalcVal = pairCond.second ? stuDynCalcRlt.dynamicCalcVal : stuDynCalcRlt.dynamicCalcVal / 10000;

            // 批量调整动态计算值
            iDynamicCalcVal = iDynamicCalcVal * stuReqCalcCfg.iAdjustPercent / 100;

            // 更新对应的值
            it.value().vecIReqDynCalcVal[iDayOfWeek - 1] = iDynamicCalcVal;
            it.value().vecStrReqDynCalcVal[iDayOfWeek - 1] = QString::number(iDynamicCalcVal) + strUnit;

            // 如果要使用动态计算值，则更新需求计算值
//             if (!stuReqCalcCfg.bEnable)
//             {
//                 continue;
//             }
// 
//             // 更新需求计算值
//             if (it.value().vecIReqVal.size() < Qt::Sunday)
//             {
//                 it.value().vecIReqVal = it.value().vecIReqDynCalcVal;
//             }
//             else
//             {
//                 it.value().vecIReqVal[stuDynCalcRlt.dayOfWeek - 1] = it.value().vecIReqDynCalcVal[stuDynCalcRlt.dayOfWeek - 1];
//             }
// 
//             if (it.value().vecStrReqVal.size() < Qt::Sunday)
//             {
//                 it.value().vecStrReqVal = it.value().vecStrReqDynCalcVal;
//             }
//             else
//             {
//                 it.value().vecStrReqVal[stuDynCalcRlt.dayOfWeek - 1] = it.value().vecStrReqDynCalcVal[stuDynCalcRlt.dayOfWeek - 1];
//             }
        }
    };

    // 查询动态计算结果
    QMap<QPair<int, bool>, tf::DynamicCalcQueryResp> mapDynCalcRlt;
    QPair<int, bool> pairChRgnt(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, true);
    QPair<int, bool> pairChSpl(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, false);
    QPair<int, bool> pairImRgnt(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE, true);
    QPair<int, bool> pairImSpl(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE, false);
    QVector<QPair<int, bool>> vecPairCond;
    vecPairCond.push_back(pairChRgnt);
    vecPairCond.push_back(pairChSpl);
    vecPairCond.push_back(pairImRgnt);
    vecPairCond.push_back(pairImSpl);

    // 周几
    std::vector<int> vecWeekDays;
    for (int iWeekDay = Qt::Monday; iWeekDay <= Qt::Sunday; iWeekDay++)
    {
        // 周天特殊处理，后台0代表周天
        if (iWeekDay == Qt::Sunday)
        {
            vecWeekDays.push_back(0);
        }
        else
        {
            vecWeekDays.push_back(iWeekDay);
        }
    }

    // 获取编码
    auto GetCodeFromMap = [&mapSplReqInfo](QPair<int, bool> pairCond)
    {
        // 遍历map
        std::vector<int> vecCodes;
        for (auto it = mapSplReqInfo.begin(); it != mapSplReqInfo.end(); it++)
        {
            // 匹配条件
            if (std::get<0>(it.key()) != pairCond.first || std::get<1>(it.key()) == pairCond.second)
            {
                continue;
            }

            int iCode = std::get<2>(it.key());
            vecCodes.push_back(iCode);
        }

        // 排序
        std::sort(vecCodes.begin(), vecCodes.end());
        return vecCodes;
    };

    // 查询动态计算结果并把结果放到容器中
    for (const auto& pairCond : vecPairCond)
    {
        // 构造查询条件
        ::tf::DynamicCalcQueryResp qryResp;
        ::tf::DynamicCalcQueryCond qryCond;
        qryCond.__set_classify(pairCond.first);
        qryCond.__set_timeRange(trc);
        qryCond.__set_lstDayOfWeeks(vecWeekDays);
        qryCond.__set_lstAssayCodes(GetCodeFromMap(pairCond));

        // 执行查询
        bool bRet = DcsControlProxy::GetInstance()->QueryDynamicCalVal(qryResp, pairCond.second, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            continue;
        }

        // 将动态计算结果更新到映射表中
        UpdateDunCalcRltToMap(qryResp, pairCond);
    }

    return true;
}

///
/// @brief
///     耗材需求信息构造字符串并压入列表
///
/// @param[in]  mapSplReqInfo       耗材需求映射表
/// @param[in]  stuSplReqInfoList   耗材需求信息列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月27日，新建函数
///
bool UiDcsAdapter::MakeStrForSplReqInfo(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList)
{
    // 构造字符串字段输出返回值
    for (auto it = mapSplReqInfo.begin(); it != mapSplReqInfo.end(); it++)
    {
        // 获取单位
        QString strUnit = it.value().bIsRgnt ? "" : "ml";

        // 模块余量分布
        QStringList strDevResidualList;
        for (auto itDevResidual = it.value().mapDevResidual.begin(); itDevResidual != it.value().mapDevResidual.end(); itDevResidual++)
        {
            QString strDevResidual = itDevResidual.key() + ":" + QString::number(itDevResidual.value()) + strUnit;
            strDevResidualList.push_back(strDevResidual);
        }
        it.value().strDevResidual = strDevResidualList.join("    ");

        // 需求量
//         for (int iDayOfWeek = Qt::Monday; iDayOfWeek < Qt::Sunday; iDayOfWeek++)
//         {
//             it.value().vecStrReqVal.push_back(it.value().vecIReqVal.size() < iDayOfWeek ? "" :  QString::number(it.value().vecIReqVal[iDayOfWeek - 1]) + strUnit);
//         }
        for (int iReqVal : it.value().vecIReqVal)
        {
            it.value().vecStrReqVal.push_back((iReqVal <= 0) ? "" : QString::number(iReqVal) + strUnit);
        }

        // 当前余量
        it.value().strCurResidual = QString::number(it.value().iCurResidual) + strUnit;

        // 需求缺口
        for (int iLackVal : it.value().vecILackingVal)
        {
            QString strLackVal = iLackVal <= 0 ? "" : QString::number(iLackVal) + strUnit;
            it.value().vecStrLackingVal.push_back(strLackVal);
        }

        // 瓶规格
        it.value().strBottleSpec = (it.value().iBottleSpec > 0) ? (QString::number(it.value().iBottleSpec) + strUnit) : "";

        // 压入输出列表
        stuSplReqInfoList.push_back(it.value());
    }

    return true;
}

int UiDcsAdapter::GetReagentSlotNumber()
{
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_strDevName, m_strGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return -1;
    }
    spIDev->SetModuleNo(m_iModuleNo);

    return spIDev->ReagentSlotNumber();
}

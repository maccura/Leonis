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
/// @file     maintaindatamng.cpp
/// @brief    应用->维护保养界面
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintaindatamng.h"
#include "maintainopdlg.h"
#include "QMaintainSetPage.h"
#include "uidcsadapter/abstractdevice.h"
#include "uidcsadapter/uidcsadapter.h"
#include "thrift/DcsControlProxy.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/SystemPowerManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include <QDateTime>

MaintainDataMng::MaintainDataMng(QObject *parent)
    : QObject(parent),
      m_pTipDlg(new TipDlg()),
      m_enMaintainMode(MAINTAIN_MODE_NORMAL)
{
    // 提示框标题默认为维护失败
    m_pTipDlg->SetTitleName(tr("维护失败"));
    m_pTipDlg->SetButtonModel(TipDlgType::SINGLE_BUTTON);

    // 初始化信号槽
    InitConnect();

    // 加载数据
    LoadData();
}

MaintainDataMng::~MaintainDataMng()
{
    setParent(Q_NULLPTR);
    m_pTipDlg->deleteLater();
}

///
/// @brief
///     获取单例
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
MaintainDataMng* MaintainDataMng::GetInstance()
{
    static MaintainDataMng s_obj;
    return &s_obj;
}

///
/// @brief
///     获取维护组设备类型列表
///
/// @param[in]  strMaintainGrpName  维护组名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月19日，新建函数
///
QSet<tf::DeviceType::type> MaintainDataMng::GetDeviceTypesForMaintainGrp(const QString& strMaintainGrpName)
{
    // 构造返回值
    QSet<tf::DeviceType::type> setRet;

    // 根据维护组名称拿到维护组信息
    tf::MaintainGroup stuTfMtGrp = GetMaintainGrpByName(strMaintainGrpName);

    // 获取维护类型
    for (const auto& stuMtItem : stuTfMtGrp.items)
    {
        // 获取真实item
        tf::MaintainItem stuRealItem = GetMaintainItemByType(stuMtItem.itemType);

        // 将设备类型插入容器中
        std::for_each(stuRealItem.deviceTypes.begin(), stuRealItem.deviceTypes.end(), [&setRet](const tf::DeviceType::type enDevTyep)
        {
            setRet.insert(enDevTyep);
        });
    }

    // 返回结果
    return setRet;
}

///
/// @brief
///     加载数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月15日，新建函数
///
void MaintainDataMng::LoadData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询维护组
	QueryMaintainGroup();

	// 查询维护项
	QueryMaintainItem();

	// 查询最近执行信息
	QueryLatestMaintainInfo();
    
	// 更新缓存(设备状态与维护状态不匹配，则更新缓存)
	UpdateCache();
}

///
/// @brief
///     设备模块是否正在执行
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  iModuleIdx  模块索引
///
/// @return true表示正在执行
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月26日，新建函数
///
bool MaintainDataMng::IsDevModuleMaintaining(const QString& strDevSn, int iModuleIdx)
{
    // 参数检查
    if (strDevSn.isEmpty() || iModuleIdx <= 0)
    {
        return false;
    }

    // 根据设备名获取维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end())
    {
        return false;
    }

    // 获取维护详情
    tf::LatestMaintainInfo& stuMtDetail = it.value();

    // 遍历维护项
    for (const auto& stuMtItemRlt : stuMtDetail.resultDetail)
    {
        // 模块号不匹配，则跳过
        if ((stuMtItemRlt.moduleIndex != iModuleIdx))
        {
            continue;
        }

        // 正在维护
        if ((stuMtItemRlt.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING &&
			stuMtDetail.exeResult == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING) ||
			stuMtDetail.exeResult == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING 
			)
        {
			return true;
        }
    }

    return false;
}

///
/// @brief
///     获取所有维护组
///
/// @param[in]  grpList  维护组列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月15日，新建函数
///
bool MaintainDataMng::GetAllMaintainGrp(QList<tf::MaintainGroup>& grpList)
{
    // 清空维护组
    grpList = m_stuMtGrpList;
//     for (auto it = m_mapMaintainGrp.begin(); it != m_mapMaintainGrp.end(); it++)
//     {
//         grpList.push_back(it.value());
//     }
// 
//     // 维护单项
//     for (auto it = m_mapMaintainItemGrp.begin(); it != m_mapMaintainItemGrp.end(); it++)
//     {
//         grpList.push_back(it.value());
//     }

    return true;
}

///
/// @brief
///     获取所有维护详情
///
/// @param[in]  grpList  维护组列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
bool MaintainDataMng::GetAllMaintainDetail(QList<tf::LatestMaintainInfo>& detailList)
{
    // 清空维护详情
    detailList.clear();
    for (auto it = m_mapMaintainDetail.begin(); it != m_mapMaintainDetail.end(); it++)
    {
        detailList.push_back(it.value());
    }

    return true;
}

///
/// @brief
///     通过维护名获取维护组信息
///
/// @param[in]  strName  维护名
///
/// @return 维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月15日，新建函数
///
tf::MaintainGroup MaintainDataMng::GetMaintainGrpByName(QString strName)
{
    // 先从维护组中找
    {
        auto it = m_mapMaintainGrp.find(strName);
        if (it != m_mapMaintainGrp.end())
        {
            return it.value();
        }
    }

    // 再从维护单项中找
    {
        auto it = m_mapMaintainItemGrp.find(strName);
        if (it != m_mapMaintainItemGrp.end())
        {
            return it.value();
        }
    }

    // 再从维护过滤项中找
    {
        auto it = m_mapMaintainFilterGrp.find(strName);
        if (it != m_mapMaintainFilterGrp.end())
        {
            return it.value();
        }
    }

    return tf::MaintainGroup();
}

///
/// @brief 根据维护单项类型获取单项维护组（只能查找单项维护组）
///
/// @param[in]  itemType  维护项类型
///
/// @return 单项维护组信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
tf::MaintainGroup MaintainDataMng::GetSingleMaintainGrpByType(::tf::MaintainItemType::type itemType)
{
	// 单项维护组中查找
	for (auto it = m_mapMaintainItemGrp.begin(); it != m_mapMaintainItemGrp.end(); it++)
	{
		for (auto item : it.value().items)
		{
			if (item.itemType == itemType)
			{
				return it.value();
			}
		}
	}

	return tf::MaintainGroup();
}

///
/// @brief 根据维护组类型获取维护组（只能查找组合维护组）
///
/// @param[in]  groupType  维护组类型
///
/// @return 维护组信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
tf::MaintainGroup MaintainDataMng::GetCombinMaintainGrpByType(::tf::MaintainGroupType::type groupType)
{
	// 维护组中查找
	for (auto it = m_mapMaintainGrp.begin(); it != m_mapMaintainGrp.end(); it++)
	{
		if (it.value().groupType == groupType)
		{
			return it.value();
		}
	}
	return tf::MaintainGroup();
}

///
/// @brief
///     通过维护组ID获取维护组信息
///
/// @param[in]  lMtGrpId  维护组ID
///
/// @return 维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
tf::MaintainGroup MaintainDataMng::GetMaintainGrpById(long long lMtGrpId)
{
    // 先从维护组中找
    {
        for (auto it = m_mapMaintainGrp.begin(); it != m_mapMaintainGrp.end(); it++)
        {
            if (it.value().id == lMtGrpId)
            {
                return it.value();
            }
        }
    }

    // 再从维护单项中找
    {
        for (auto it = m_mapMaintainItemGrp.begin(); it != m_mapMaintainItemGrp.end(); it++)
        {
            if (it.value().id == lMtGrpId)
            {
                return it.value();
            }
        }
    }

    // 再从维护单项中找
    {
        for (auto it = m_mapMaintainFilterGrp.begin(); it != m_mapMaintainFilterGrp.end(); it++)
        {
            if (it.value().id == lMtGrpId)
            {
                return it.value();
            }
        }
    }

    return tf::MaintainGroup();
}

///
/// @brief
///     通过设备序列号获取维护详情信息
///
/// @param[in]  strDevSn  设备序列号
///
/// @return 维护详情信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
tf::LatestMaintainInfo MaintainDataMng::GetMaintainDetailByDevSn(QString strDevSn)
{
    // 查找对应项
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it != m_mapMaintainDetail.end())
    {
        return it.value();
    }

    return tf::LatestMaintainInfo();
}

///
/// @brief
///     通过设备组名获取维护详情
///
/// @param[in]  strDevGrpName  设备组名
///
/// @return 设备组维护详情数组
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月18日，新建函数
///
QVector<tf::LatestMaintainInfo> MaintainDataMng::GetMaintainDetailByDevGrpName(QString strDevGrpName)
{
    // 判断是否是设备组名
    QVector<tf::LatestMaintainInfo> vMaintainDetail;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(strDevGrpName))
    {
        // 不是设备组名，返回一个默认的维护详情
        return vMaintainDetail;
    }

    // 获取子设备列表
    QStringList strSubDevNameList;
    if (!gUiAdapterPtr()->GetSubDevNameList(strDevGrpName, strSubDevNameList))
    {
        return vMaintainDetail;
    }

    // 获取每一个子设备的维护详情然后合并
    for (const QString& strSubDevName : strSubDevNameList)
    {
        // 获取所有维护详情
        tf::LatestMaintainInfo stuDetailInfo = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(gUiAdapterPtr()->GetDevSnByName(strSubDevName, strDevGrpName));
        if (stuDetailInfo.id <= 0)
        {
            continue;
        }

        // 压入设备详情数组
        vMaintainDetail.push_back(stuDetailInfo);
    }

    return vMaintainDetail;
}

///
/// @brief
///     根据维护项类型获取维护名
///
/// @param[in]  enItemType  维护项类型
///
/// @return 维护名
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月15日，新建函数
///
QString MaintainDataMng::GetMaintainItemNameByType(tf::MaintainItemType::type enItemType)
{
    // 遍历维护单项
    for (auto it = m_mapMaintainItem.begin(); it != m_mapMaintainItem.end(); it++)
    {
        if (it.value().itemType == enItemType)
        {
            return it.key();
        }
    }

    return "";
}

///
/// @brief
///     根据维护项类型获取维护项
///
/// @param[in]  enItemType  维护项类型
///
/// @return 维护项信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月12日，新建函数
///
tf::MaintainItem MaintainDataMng::GetMaintainItemByType(tf::MaintainItemType::type enItemType)
{
    // 遍历维护单项
    for (auto it = m_mapMaintainItem.begin(); it != m_mapMaintainItem.end(); it++)
    {
        if (it.value().itemType == enItemType)
        {
            return it.value();
        }
    }

    return tf::MaintainItem();
}

///
/// @brief
///     根据维护项名称获取维护项
///
/// @param[in]  strItemName  维护项名称
///
/// @return 维护项信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月12日，新建函数
///
tf::MaintainItem MaintainDataMng::GetMaintainItemByName(const QString& strItemName)
{
    // 遍历维护单项
    auto it = m_mapMaintainItem.find(strItemName);
    if (it == m_mapMaintainItem.end())
    {
        return tf::MaintainItem();
    }

    return it.value();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainDataMng::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 监听维护失败提示对话框已读信息
    connect(m_pTipDlg, SIGNAL(finished(int)), this, SLOT(OnMaintainFailedCauseRead()));

    // 监听系统关机
    connect(SystemPowerManager::GetInstance().get(), SIGNAL(ReadyToExit()), m_pTipDlg, SLOT(reject()));

    // 监听维护数据初始化
    //REGISTER_HANDLER(MSG_ID_MAINTAIN_DATA_INIT, this, OnMaintainDataInit);

	// 监听维护组更新
	REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_UPDATE, this, OnUpdateMaintainGroup);

	// 监听维护项更新
	REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_UPDATE, this, OnUpdatedMaintainItem);

    // 监听维护组组合设置更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_CFG_UPDATE, this, UpdateMaintainGrpData);

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

    // 监听维护组进度
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE, this, OnMaintainGrpProgressUpdate);

    // 监听维护组执行时间更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_TIME_UPDATE, this, OnMaintainGrpTimeUpdate);

    // 监听维护项时间更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_TIME_UPDATE, this, OnMaintainItemTimeUpdate);

    // 监听维护项状态更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnMaintainItemStatusUpdate);

    // 监听维护项进度更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PROGRESS_UPDATE, this, OnMaintainItemProgressUpdate);

	// 监听维护失败原因
	REGISTER_HANDLER(MSG_ID_MAINTAIN_FAIL_CAUSE, this, OnMaintainFailCauseUpdate);
}

///
/// @brief
///     设备是否可执行某个维护项
///
/// @param[in]  strDevName  设备名
/// @param[in]  enItemType  维护项类型
///
/// @return true表示可执行
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月12日，新建函数
///
bool MaintainDataMng::WhetherDevColudExeItem(const QPair<QString, QString>& strDevName, tf::MaintainItemType::type enItemType)
{
    // 获取维护项信息
    tf::MaintainItem stuMtItem = GetMaintainItemByType(enItemType);

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
    if (spIDev == Q_NULLPTR)
    {
        return false;
    }

    // 记录设备类型
    tf::DeviceType::type enDevType = (tf::DeviceType::type)spIDev->DeviceType();

    // 遍历项目支持的设备类型，存在该设备类型，则返回true
    for (auto enSptDevType : stuMtItem.deviceTypes)
    {
        // 匹配返回true
        if (enDevType == enSptDevType)
        {
            return true;
        }
    }

    // 否则返回false
    return false;
}

///
/// @brief
///     更新对应设备的维护详情
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月29日，新建函数
///
void MaintainDataMng::UpdateMaintainDetail(const QString& strDevSn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
	// 获取设备信息
	const auto& devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn.toStdString());
	if (devInfo == Q_NULLPTR )
	{
		ULOG(LOG_ERROR, "%s(), GetDeviceInfo(%s) failed", __FUNCTION__, strDevSn.toStdString());
		return;
	}

	// 获取记录的维护组ID（由于免疫不能够以时间进行判断正在维护，但ISE存在多模块问题，故暂时采用此方式进行处理-2024.8.14-tcx&gzq）
	auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end() || devInfo->deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        // 清空数据缓存
        m_mapMaintainDetail.remove(strDevSn);

        // 将结果插入缓存之中
        UpdateLatestMaintainGroup(strDevSn.toStdString());
        return;
    }

    // 更新对应的维护详情
    tf::LatestMaintainInfoQueryCond detailQryCond;
    tf::LatestMaintainInfoQueryResp detailQryResp;
    detailQryCond.__set_groupId(it.value().groupId);
    detailQryCond.__set_deviceSN(strDevSn.toStdString());

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(detailQryResp, detailQryCond) || (detailQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "%s(), QueryLatestMaintainInfo() failed", __FUNCTION__);
        return;
    }

    // 如果结果为空，则返回
    if (detailQryResp.lstLatestMaintainInfo.empty())
    {
        ULOG(LOG_INFO, "%s(), detailQryResp.lstLatestMaintainInfo.empty()", __FUNCTION__);
        return;
    }

    // 更新缓存
    m_mapMaintainDetail.insert(strDevSn, detailQryResp.lstLatestMaintainInfo[0]);
}

void MaintainDataMng::UpdateLatestMaintainGroup(const std::string& devSN)
{    
    // 查询最近维护记录
    tf::LatestMaintainInfoQueryCond detailQryCond;
    tf::LatestMaintainInfoQueryResp detailQryResp;
    detailQryCond.__set_deviceSN(devSN);

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(detailQryResp, detailQryCond) || (detailQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "%s(), QueryLatestMaintainInfo() failed", __FUNCTION__);
        return;
    }

    // 默认入参是按开始时间降序
    for (int iPos = 0; iPos< detailQryResp.lstLatestMaintainInfo.size(); ++iPos)
    {
        const tf::LatestMaintainInfo& stuLastestMtInfo = detailQryResp.lstLatestMaintainInfo[iPos];

        // 找是否已存在对应设备的维护详情
        auto it = m_mapMaintainDetail.find(QString::fromStdString(stuLastestMtInfo.deviceSN));
		if (it != m_mapMaintainDetail.end() && it.value().exeTime.empty())
		{
			continue;
		}
        if (it != m_mapMaintainDetail.end() && !stuLastestMtInfo.exeTime.empty() && it.value().exeTime > stuLastestMtInfo.exeTime)
        {
            continue;
        }
        // 如果日期比当前日期大，则跳过
        if (!stuLastestMtInfo.exeTime.empty() && 
            QDateTime::fromString(QString::fromStdString(stuLastestMtInfo.exeTime), UI_DATE_TIME_FORMAT) > QDateTime::currentDateTime())
        {
            continue;
        }
        // 更新最新的维护详细
        m_mapMaintainDetail.insert(QString::fromStdString(stuLastestMtInfo.deviceSN), stuLastestMtInfo);
        break;
    }
}

///
///  @brief 更新维护组
///
///
///  @param[in]   lmi  最近执行情况
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月2日，新建函数
///
void MaintainDataMng::OnUpdateMaintainGroup(tf::LatestMaintainInfo lmi)
{
	// 时间更新
	if (lmi.__isset.exeTime || lmi.__isset.finishTime)
	{
		QString maintainTime = "";
		if (lmi.__isset.finishTime && !lmi.finishTime.empty())
		{
			maintainTime = QString::fromStdString(lmi.finishTime);
		}
		else
		{
			maintainTime = QString::fromStdString(lmi.exeTime);
		}

		POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_TIME_UPDATE, QString::fromStdString(lmi.deviceSN), lmi.groupId, maintainTime);
	}
	
	// 进度更新
	if (lmi.__isset.progress)		
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE, lmi);
	}
	
	// 结果更新
	if (lmi.__isset.exeResult)		
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, QString::fromStdString(lmi.deviceSN), lmi.groupId, lmi.exeResult);
	}
}

///
/// @brief
///     维护数据初始化
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月15日，新建函数
///
void MaintainDataMng::OnMaintainDataInit(QString strDevSn)
{
    // 如果维护项数据为空，则加载数据
    if (m_mapMaintainItem.isEmpty())
    {
        LoadData();
    }

    // 清空数据缓存
    m_mapMaintainDetail.remove(strDevSn);

    // 查询最近维护记录
    tf::LatestMaintainInfoQueryCond detailQryCond;
    tf::LatestMaintainInfoQueryResp detailQryResp;
    detailQryCond.__set_deviceSN(strDevSn.toStdString());

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(detailQryResp, detailQryCond) || (detailQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "%s(), QueryLatestMaintainInfo() failed", __FUNCTION__);
        return;
    }

    // 将结果插入缓存之中
    for (const auto& stuLastestMtInfo : detailQryResp.lstLatestMaintainInfo)
    {
        // 找是否已存在对应设备的维护详情
        auto it = m_mapMaintainDetail.find(QString::fromStdString(stuLastestMtInfo.deviceSN));
        if (it != m_mapMaintainDetail.end() && it.value().exeTime.empty())
        {
            continue;
        }

        if (it != m_mapMaintainDetail.end() && !stuLastestMtInfo.exeTime.empty() && it.value().exeTime > stuLastestMtInfo.exeTime)
        {
            continue;
        }

        // 如果日期比当前日期大，则跳过
        if (!stuLastestMtInfo.exeTime.empty() && QDateTime::fromString(QString::fromStdString(stuLastestMtInfo.exeTime), UI_DATE_TIME_FORMAT) > QDateTime::currentDateTime())
        {
            continue;
        }

        // 一个设备只有一个维护详情
        m_mapMaintainDetail.insert(QString::fromStdString(stuLastestMtInfo.deviceSN), stuLastestMtInfo);
    }
}

///
///  @brief 更新维护项
///
///
///  @param[in]   devSN		设备序列号
///  @param[in]   groupId	维护组id
///  @param[in]   mier		执行结果
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月2日，新建函数
///
void MaintainDataMng::OnUpdatedMaintainItem(QString devSN, long long groupId, tf::MaintainItemExeResult mier)
{
	// 时间更新
	if (mier.__isset.exeTime)	
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_ITEM_TIME_UPDATE, devSN, groupId, mier.itemType, QString::fromStdString(mier.exeTime));
	}

	// 结果更新
	if (mier.__isset.result)
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, devSN, mier.moduleIndex, groupId, mier.itemType, mier.result);
	}

	// 进度更新
	if (mier.__isset.progress)	
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_ITEM_PROGRESS_UPDATE, devSN, groupId, mier.itemType, mier.progress);
	}

	// 失败原因
	if (mier.__isset.failCause && mier.failCause != ::tf::MaintainFailCause::CASE_PROCESS_NORMAL)
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_FAIL_CAUSE, devSN, groupId, int(mier.failCause));
	}

	// 阶段更新
	if (mier.__isset.currentStage && mier.__isset.totalStage)
	{
		POST_MESSAGE(MSG_ID_MAINTAIN_ITEM_STAGE_UPDATE, devSN, groupId, mier);
	}
}

///
/// @brief
///     更新维护组按钮
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月23日，新建函数
///
void MaintainDataMng::UpdateMaintainGrpData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询所有维护组
	QueryMaintainGroup();
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void MaintainDataMng::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(lGrpId);
    // 重置维护执行步骤
    auto ResetExeStep = [this](const QString& strDevSn)
    {
        auto it = m_mapMaintainItemExeStep.begin();
        while (it != m_mapMaintainItemExeStep.end())
        {
            if (it.key().first == strDevSn)
            {
                it = m_mapMaintainItemExeStep.erase(it);
                continue;
            }

            // 迭代器自增
            ++it;
        }
    };

    // 等待状态由特殊用途
    if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
    {
        return;
    }

    tf::MaintainGroup stuMtGrp = GetMaintainGrpById(lGrpId);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);

    // 如果维护阶段变为正在执行，表明维护开始执行，此时重置维护详情
    if ((m_enMaintainMode != MAINTAIN_MODE_REMAINTAIN) &&
        (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING) &&
        (it == m_mapMaintainDetail.end() || it.value().exeResult != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING))
    {
        // 获取设备名
        QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);

        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
        if (spIDev == Q_NULLPTR)
        {
            return;
        }

        // 如果是多模块设备，维护详情需要重新查询，不能重置
        if (spIDev->ModuleCount() > 1)
        {
            // 更新对应设备的维护详情
            UpdateMaintainDetail(strDevSn);
            ResetExeStep(strDevSn);
            return;
        }

        tf::LatestMaintainInfo stuMtDetail;
        stuMtDetail.__set_deviceSN(strDevSn.toStdString());
        stuMtDetail.__set_groupId(lGrpId);
        stuMtDetail.__set_exeResult(enPhaseType);
        stuMtDetail.__set_progress(0);
        stuMtDetail.__set_exeTime("");
        stuMtDetail.__set_id(007);

        std::vector<tf::MaintainItemExeResult> vecItemDetail;
        for (auto enItem : stuMtGrp.items)
        {
            // 如果设备不允许执行该项目，则跳过
            if (!WhetherDevColudExeItem(strDevNamePair, enItem.itemType))
            {
                continue;
            }

            tf::MaintainItemExeResult stuItemDetail;
            stuItemDetail.__set_itemType(enItem.itemType);
            stuItemDetail.__set_exeTime("");
            stuItemDetail.__set_progress(0);
            stuItemDetail.__set_result(tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN);
            vecItemDetail.push_back(std::move(stuItemDetail));
        }
        stuMtDetail.__set_resultDetail(vecItemDetail);

        // 覆盖维护详情
        m_mapMaintainDetail.insert(strDevSn, stuMtDetail);

        // 维护项执行步数清空
        ResetExeStep(strDevSn);
        return;
    }

    // 有维护详情则更新执行状态
    if (it != m_mapMaintainDetail.end())
    {
        // 更新维护详情中的维护阶段
        it.value().exeResult = enPhaseType;

        // 如果状态为完成，进度置为100%
        if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
        {
            it.value().progress = 100;
        }
    }

    // 维护状态不为正在维护了，变为失败或完成了，改变维护模式为正常维护
    if ((enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS) || (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_FAIL))
    {
        m_enMaintainMode = MAINTAIN_MODE_NORMAL;
    }

    // 周维护时，如果执行完成，通知SystemPowerManager，关机时不再执行周维护
    if (stuMtGrp.groupType == tf::MaintainGroupType::type::MAINTAIN_GROUP_WEEK && enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
    {
        POST_MESSAGE(MSG_ID_UPDATE_STOP_MAINTAIN_TYPE, strDevSn, (int)tf::MaintainGroupType::type::MAINTAIN_GROUP_STOP);
    }
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  lmi     正在执行的维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void MaintainDataMng::OnMaintainGrpProgressUpdate(tf::LatestMaintainInfo lmi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(QString::fromStdString(lmi.deviceSN));
    if (it == m_mapMaintainDetail.end())
    {
        return;
    }

    // 更新维护详情中的维护进度
    it.value().progress = lmi.progress;
}

///
/// @brief
///     维护组执行时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  strMaintainTime 维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainDataMng::OnMaintainGrpTimeUpdate(QString strDevSn, long long lGrpId, QString strMaintainTime)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(lGrpId);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end())
    {
        return;
    }

    // 更新维护详情中的维护进度
    it.value().exeTime = strMaintainTime.toStdString();
}

///
/// @brief
///     维护项状态更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
/// @param[in]  lGrpId			维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainDataMng::OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	Q_UNUSED(devModuleIndex);
	Q_UNUSED(lGrpId);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end())
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
    if (spIDev == Q_NULLPTR)
    {
        return;
    }

    // 如果是多模块设备，维护详情需要重新查询，不能重置
    if (spIDev->ModuleCount() > 1)
    {
        // 更新对应设备的维护详情
        UpdateMaintainDetail(strDevSn);
        return;
    }

    // 获取维护项执行步数
    int iItemExeStep = 1;
    auto itStep = m_mapMaintainItemExeStep.find(qMakePair(strDevSn,enItemType));
    if (itStep == m_mapMaintainItemExeStep.end())
    {
        m_mapMaintainItemExeStep.insert(qMakePair(strDevSn, enItemType), 1);
		itStep = m_mapMaintainItemExeStep.end();
    }
    else
    {
        iItemExeStep = itStep.value();
    }

    // 更新维护详情中的维护阶段
    int iSameItemTypeCnt = 0;
    for (auto& stuItemRlt : it.value().resultDetail)
    {
        // 维护项类型不匹配则跳过
        if (stuItemRlt.itemType != enItemType)
        {
            continue;
        }

        // 单项索引自增
        ++iSameItemTypeCnt;

        // 如果执行步数不够，则跳过
        if (iSameItemTypeCnt < iItemExeStep)
        {
            continue;
        }

        // 更新维护状态
        stuItemRlt.result = enPhaseType;
        break;
    }

    // 状态变为成功时，维护项执行步数增加
    if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
    {
		// 防御一下
		if (itStep != m_mapMaintainItemExeStep.end())
		{
			++(itStep.value());
		}
       
    }
}

///
/// @brief
///     维护项时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  strTime         时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainDataMng::OnMaintainItemTimeUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, QString strTime)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(lGrpId);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end())
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
    if (spIDev == Q_NULLPTR)
    {
        return;
    }

    // 如果是多模块设备，维护详情需要重新查询，不能重置
    if (spIDev->ModuleCount() > 1)
    {
        // 更新对应设备的维护详情
        UpdateMaintainDetail(strDevSn);
        return;
    }

    // 获取维护项执行步数
    int iItemExeStep = 1;
    auto itStep = m_mapMaintainItemExeStep.find(qMakePair(strDevSn, enItemType));
    if (itStep == m_mapMaintainItemExeStep.end())
    {
        m_mapMaintainItemExeStep.insert(qMakePair(strDevSn, enItemType), 1);
		itStep = m_mapMaintainItemExeStep.end();
    }
    else
    {
        iItemExeStep = itStep.value();
    }

    // 更新维护详情中的维护阶段
    int iSameItemTypeCnt = 0;
    for (auto& stuItemRlt : it.value().resultDetail)
    {
        // 维护项类型不匹配则跳过
        if (stuItemRlt.itemType != enItemType)
        {
            continue;
        }

        // 单项索引自增
        ++iSameItemTypeCnt;

        // 如果执行步数不够，则跳过
        if (iSameItemTypeCnt < iItemExeStep)
        {
            continue;
        }

        // 更新维护状态
        stuItemRlt.exeTime = strTime.toStdString();
        break;
    }
}

///
/// @brief
///     维护项时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  iProgress       进度
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainDataMng::OnMaintainItemProgressUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, int iProgress)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(lGrpId);
    // 更新维护详情
    auto it = m_mapMaintainDetail.find(strDevSn);
    if (it == m_mapMaintainDetail.end())
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);

    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
    if (spIDev == Q_NULLPTR)
    {
        return;
    }

    // 如果是多模块设备，维护详情需要重新查询，不能重置
    if (spIDev->ModuleCount() > 1)
    {
        // 更新对应设备的维护详情
        UpdateMaintainDetail(strDevSn);
        return;
    }

    // 更新维护详情中的维护阶段
    for (auto& stuItemRlt : it.value().resultDetail)
    {
        // 维护项类型不匹配则跳过
        if (stuItemRlt.itemType != enItemType)
        {
            continue;
        }

        // 更新维护状态
        stuItemRlt.progress = iProgress;
        break;
    }
}

///
/// @brief
///     维护失败原因已经被阅读
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月4日，新建函数
///
void MaintainDataMng::OnMaintainFailedCauseRead()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空文本
    m_strMaintainFailedCause.clear();
    m_pTipDlg->SetText(m_strMaintainFailedCause);
}

///
///  @brief 维护失败原因
///
///
///  @param[in]   strDevSn		设备序列号
///  @param[in]   lGrpId		维护组ID
///  @param[in]   failCause		失败原因  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月29日，新建函数
///
void MaintainDataMng::OnMaintainFailCauseUpdate(QString strDevSn, long long lGrpId, int failCause)
{
    ULOG(LOG_INFO, "%s(%s, %d, %d)", __FUNCTION__, strDevSn.toStdString().c_str(), int(lGrpId), int(failCause));
    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);
    if (strDevNamePair.first.isEmpty())
    {
        return;
    }
    QString strDevName = strDevNamePair.second + strDevNamePair.first;

    // 获取维护组名
    tf::MaintainGroup stuMtGrp = GetMaintainGrpById(lGrpId);
    QString strMtGrpName = QString::fromStdString(stuMtGrp.groupName);

    // 构造提示信息
    QString strTip = QString("【") + strDevName + ("】") + tr("执行") + ("【") + strMtGrpName + ("】") + tr("失败") + tr("：") + UiCommon::Instance()->ConvertMaintainFailCauseToString(failCause);

    // 显示追加
    if (m_strMaintainFailedCause.isEmpty())
    {
        m_strMaintainFailedCause = strTip;
    }
    else
    {
        m_strMaintainFailedCause = m_strMaintainFailedCause + "\n" + strTip;
    }
    m_pTipDlg->SetText(m_strMaintainFailedCause);
    m_pTipDlg->show();
}

///
///  @brief 查询所有维护组
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void MaintainDataMng::QueryMaintainGroup()
{
	// 清空数据缓存
    m_stuMtGrpList.clear();
	m_mapMaintainGrp.clear();
	m_mapMaintainItemGrp.clear();

	// 查询所有维护组
	tf::MaintainGroupQueryCond grpQryCond;
	tf::MaintainGroupQueryResp grpQryResp;

	// 执行查询
	if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(grpQryResp, grpQryCond) || (grpQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
	{
		ULOG(LOG_ERROR, "%s(), QueryMaintainGroup() failed", __FUNCTION__);
		return;
	}

	// 过滤ui不显示的维护组
	FilterGroups(grpQryResp.lstMaintainGroups);

	// 将结果插入缓存之中
	for (tf::MaintainGroup& stuMaintainGrp : grpQryResp.lstMaintainGroups)
	{
		if (stuMaintainGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE && 
            stuMaintainGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_CUSTOM)
		{
			stuMaintainGrp.__set_groupName(ConvertTfEnumToQString(stuMaintainGrp.groupType).toStdString());
		}

		if (stuMaintainGrp.groupType == tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
		{
			std::vector<tf::MaintainItem>& items = stuMaintainGrp.items;
			if (stuMaintainGrp.__isset.items && !items.empty())
			{
				stuMaintainGrp.__set_groupName(ConvertTfEnumToQString(items.front().itemType).toStdString());
			}

            // 单维护
            m_mapMaintainItemGrp.insert(QString::fromStdString(stuMaintainGrp.groupName), stuMaintainGrp);
		}
		else
		{
            // 组合维护
			m_mapMaintainGrp.insert(QString::fromStdString(stuMaintainGrp.groupName), stuMaintainGrp);
		}

        m_stuMtGrpList.push_back(stuMaintainGrp);
	}
}

///
///  @brief 查询所有维护项
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void MaintainDataMng::QueryMaintainItem()
{
	// 清空数据缓存
	m_mapMaintainItem.clear();
    m_mapWithPermissionItem.clear();

	// 查询所有维护项
	tf::MaintainItemQueryCond itemQryCond;
	tf::MaintainItemQueryResp itemQryResp;

	// 执行查询
	if (!DcsControlProxy::GetInstance()->QueryMaintainItem(itemQryResp, itemQryCond) || 
        (itemQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
	{
		ULOG(LOG_ERROR, "%s(), QueryMaintainItem() failed", __FUNCTION__);
		return;
	}

    std::shared_ptr<UserInfoManager> userMgr = UserInfoManager::GetInstance();

	// 将结果插入缓存之中
	for (tf::MaintainItem& stuMtItem : itemQryResp.lstMaintainItems)
	{
        // 不支持该维护项
        if (!gUiAdapterPtr()->WhetherSupportTargetMaintainItem(stuMtItem))
        {
            continue;
        }

        // 获取翻译后的名称
        stuMtItem.__set_itemName(ConvertTfEnumToQString(stuMtItem.itemType).toStdString());

        // 一个设备只有一个维护详情
        m_mapMaintainItem.insert(QString::fromStdString(stuMtItem.itemName), stuMtItem);

        // 该维护项是否有权限显示
        if (!userMgr->IsPermissonMaintShow(stuMtItem.itemType))
        {
            continue;
        }

		// 一个设备只有一个维护详情
        m_mapWithPermissionItem.insert(QString::fromStdString(stuMtItem.itemName), stuMtItem);
	}
}

///
///  @brief 查询所有最近维护信息
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void MaintainDataMng::QueryLatestMaintainInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 清空数据缓存
	m_mapMaintainDetail.clear();

    std::map<std::string, std::shared_ptr<const tf::DeviceInfo>> devInfos = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (const auto& dev : devInfos)
	{
	// 将结果插入缓存之中
        UpdateLatestMaintainGroup(dev.second->deviceSN);
        }
}

///
///  @brief 更新缓存(设备状态与维护状态不匹配，则更新缓存)
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void MaintainDataMng::UpdateCache()
{
	// 构造查询条件(设备状态与维护状态不匹配，则更新缓存)
	::tf::DeviceInfoQueryResp devResp;
	::tf::DeviceInfoQueryCond devCond;

	// 查询设备信息
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devResp, devCond)
		|| devResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| devResp.lstDeviceInfos.empty())
	{
		return;
	}

	// 变量设备
	for (const tf::DeviceInfo& stuDevInfo : devResp.lstDeviceInfos)
	{
		auto it = m_mapMaintainDetail.find(QString::fromStdString(stuDevInfo.deviceSN));
		if (it == m_mapMaintainDetail.end())
		{
			continue;
		}

		// 如果状态不匹配
		if ((it.value().exeResult == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING) &&
			(stuDevInfo.status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
		{
			// 修改维护状态为维护失败
			it.value().exeResult = tf::MaintainResult::MAINTAIN_RESULT_FAIL;

			// 把正在执行的维护单项改为维护失败
			std::for_each(it.value().resultDetail.begin(), it.value().resultDetail.end(), [](tf::MaintainItemExeResult& stuItemResult){
				// 正在执行改为维护失败
				if (stuItemResult.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
				{
					stuItemResult.result = tf::MaintainResult::MAINTAIN_RESULT_FAIL;
				}
			});
		}
	}
}

///
///  @brief 过滤ui不显示的维护组
///
///
///  @param[in]   groups  维护组列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void MaintainDataMng::FilterGroups(std::vector<tf::MaintainGroup>& groups)
{
    std::shared_ptr<UserInfoManager> usrMgr = UserInfoManager::GetInstance();

	groups.erase(std::remove_if(groups.begin(), groups.end(), [&](::tf::MaintainGroup &group) {
		// 删除其他维护组
		if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_OTHER)
		{
			return true;
		}

		// 检查是否是单项维护组
		if (group.groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
		{
            // 如果维护组权限高于当前角色的
            std::shared_ptr<tf::UserInfo> curUser = usrMgr->GetLoginUserInfo();
            if (group.creatUserType != -1 && curUser != nullptr && 
                group.creatUserType >= tf::UserType::type::USER_TYPE_ENGINEER &&
                curUser->type < tf::UserType::type::USER_TYPE_ENGINEER)
            {
                return true;
            }
			return false;
		}

		// 如果单项维护组没有包含维护项
		if (group.items.empty())
		{
			return true;
		}

        // 如果是单项维护组，但没权限显示的
        if (!usrMgr->IsPermissonMaintShow(group.items[0].itemType))
        {
            return true;
        }

		// 如果不支持该维护单项
		if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
		{
			const tf::MaintainItem& item = group.items.front();
			if (!gUiAdapterPtr()->WhetherSupportTargetMaintainItem(item))
			{
				return true;
			}

			// 联机版没有进样器，不显示进样器复位
			if (item.itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_SAMPLE_HANDLE_SYS_RESET && 
                DictionaryQueryManager::GetInstance()->GetPipeLine())
			{
				return true;
			}
		}

		// 删除指定位置试剂扫描单项维护组
		auto itemType = group.items.front().itemType;
		if (itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN_POS ||
			itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_REACTTRANSTRAY_RESET 
            // 要求显示整机针特殊清洗 by ldx 20230913||
			//itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_SAMPLE_WASH_PIPE
            )
		{
            auto& items = group.items;
            if (group.__isset.items && !items.empty())
            {
                group.__set_groupName(ConvertTfEnumToQString(items.front().itemType).toStdString());
            }

            m_mapMaintainFilterGrp.insert(QString::fromStdString(group.groupName), group);
			return true;
		}

		return false;
	}), groups.end());
}

///
/// @brief
///     当前用户是否可以执行维护单项
///
/// @param[in]  strCurMtItemName  维护单项名称
///
/// @return true表示允许执行
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月11日，新建函数
///
bool MaintainDataMng::WhetherItemCanExeByCurUser(const QString& strCurMtItemName)
{
    auto it = m_mapWithPermissionItem.find(strCurMtItemName);
    if (it == m_mapWithPermissionItem.end())
    {
        return false;
    }

    return true;
}

///
/// @brief 获取比色管路填充参数
///
/// @param[in]  bEACS   酸性清洗液是否启用
/// @param[in]  bEALCS  碱性清洗液是否启用
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetChFluidPerfusionParam(const bool& bEACS, const bool& bEALCS)
{
	MiChFluidPerfusionParams mcfpp;

	mcfpp.bEnableAcidCleanSolution = bEACS;
	mcfpp.bEnableAlkalinityCleanSolution = bEALCS;

	return Encode2Json(std::move(mcfpp));
}

///
/// @brief 获取光度计检查参数
///
/// @param[in]  bECC   修正校验是否启用
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetChPhotometerCheckParam(const bool& bECC)
{
	MiChPhotometerCheckParams mcpcp;

	mcpcp.bEnableCorrectCheck = bECC;

	return Encode2Json(std::move(mcpcp));
}

///
/// @brief 获取比色反应杯更换参数
///
/// @param[in]  uniteNum  需要更换的联排
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetChChangeReactionCupParam(const std::vector<int>& uniteNum)
{
	MiExchReactCupParams mercp;

	mercp.uniteNum = uniteNum;

	return Encode2Json(std::move(mercp));
}

///
/// @brief 获取针清洗参数
///
/// @param[in]  bESPC  样本针清洗是否启用
/// @param[in]  bERPC  试剂针清洗是否启用
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetProbeCleanParam(const bool& bESPC, const bool& bERPC)
{
	MiProbeCleanParams mpcp;

	mpcp.bEnableReagentProbeClean = bERPC;
	mpcp.bEnableSampleProbeClean = bESPC;

	return Encode2Json(std::move(mpcp));
}

///
/// @brief 获取ISE电极更换参数
///
/// @param[in]  bNa  Na电极是否更换
/// @param[in]  bK  K电极是否更换
/// @param[in]  bCl  Cl电极是否更换
/// @param[in]  bRef  Ref电极是否更换
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetIseElecReplaceParam(const bool& bNa, const bool& bK, const bool& bCl, const bool& bRef)
{
	MiElecReplaceParams merp;

	merp.bExchangeNaElec = bNa;
	merp.bExchangeKElec = bK;
	merp.bExchangeClElec = bCl;
	merp.bExchangeRefElec = bRef;

	return Encode2Json(std::move(merp));
}

///
/// @brief 获取ISE管路填充参数
///
/// @param[in]  bESS  内部标准液是否启用
/// @param[in]  bEB   缓冲液是否启用
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetIseFluidPerfusionParam(const bool& bESS, const bool& bEB)
{
	MiIseFluidPerfusionParams mifpp;

	mifpp.bEnableBuffer = bEB;
	mifpp.bEnableStandardSolution = bESS;

	return Encode2Json(std::move(mifpp));
}

///
/// @brief 获取ISE清洗液余量探测参数
///
/// @param[in]  vecPos  探测位置信息
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年8月13日，新建函数
///
std::string MaintainDataMng::GetIseSurplusDetectionParam(const std::vector<int>& vecPos)
{
	MiIseSurplusDetectionParams misdp;

	misdp.posInfo = vecPos;

	return Encode2Json(std::move(misdp));
}

///
/// @brief 获取反应槽水更换参数
///
/// @param[in]  iMode  更换模式
///
/// @return 失败返回空字符串
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
///
std::string MaintainDataMng::GetReacTankWaterReplaceParam(const int& iMode)
{
	MiIncubWaterExchParams miwep;

	miwep.iExchangeWaterMode = iMode;

	return Encode2Json(std::move(miwep));
}

///
/// @brief 检查用户输入的维护参数数据，次数：（1-99），时间：（1-60min）
///
/// @param[in]  frequencyVec    次数参数
/// @param[in]  timeVec			时间参数
///
/// @return true:检查通过
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月12日，新建函数
///
bool MaintainDataMng::CheckMaintainItemInputParams(const QVector<int>& frequencyVec, const QVector<int>& timeVec)
{
	// 检查次数参数
	for (const auto& fParam : frequencyVec)
	{
		if (fParam < 1 || fParam > 99)
		{
			return false;
		}
	}

	// 检查时间参数
	for (const auto& tParam : timeVec)
	{
		if (tParam < 1 || tParam > 60)
		{
			return false;
		}
	}

	return true;
}

bool MaintainDataMng::CheckDevIsReadyForMaintain(QString& noticeInfo, const ::tf::DevicekeyInfo& dev, const tf::MaintainGroup& mtGroup)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	// 清空提示
	noticeInfo.clear();

	// 获取设备类型
	auto devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(dev.sn);
	if (devInfo == nullptr)
	{
		noticeInfo = tr("%1设备信息异常。").arg(QString::fromStdString(dev.sn));
		return false;
	}

	// 根据设备类型进行分类
	switch (devInfo->deviceClassify)
	{
	// 生化检查
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
	{
		return CheckChDevIsReadyForMaintain(noticeInfo, devInfo, mtGroup);
	}
	break;

	// ISE检查
	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
	{
		return CheckIseDevIsReadyForMaintain(noticeInfo, devInfo, mtGroup);
	}
	break;

	// 免疫检查
	case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
	{
		return CheckImDevIsReadyForMaintain(noticeInfo, devInfo, mtGroup);
	}
	break;

	// 轨道检查
	case tf::AssayClassify::ASSAY_CLASSIFY_OTHER:
	{
		return CheckTrackDevIsReadyForMaintain(noticeInfo, devInfo, mtGroup);
	}
	break;

	default:
		break;
	}

	return true;
}

bool MaintainDataMng::ChBindDevStatusCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 单项维护判断
	if (mtGroup.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
		mtGroup.items.size() != 1)
	{
		return true;
	}

	// 需要下电维护项判断(反应杯更换、清洗反应槽、排空供水桶)
	auto maintainItem = mtGroup.items[0];
	if (maintainItem.itemType != tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP &&
		maintainItem.itemType != tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS &&
		maintainItem.itemType != tf::MaintainItemType::MAINTAIN_ITEM_CH_DRAIN_BUCKET)
	{
		return true;
	}

	// 获取绑定设备
	auto bindSnDev = CommonInformationManager::GetInstance()->GetBindDeviceInfoBySn(devInfo->deviceSN);
	if (bindSnDev == nullptr)
	{
		return true;
	}

	// 状态检查(由于要下电，需要对绑定的设备状态进行判断)
	if (bindSnDev->status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY &&
		bindSnDev->status != tf::DeviceWorkState::DEVICE_STATUS_HALT)
	{
		auto bindDevName = bindSnDev->__isset.groupName ? (bindSnDev->groupName + bindSnDev->name) : bindSnDev->name;
		noticeInfo = tr("%1设备不处于待机或停止状态。").arg(QString::fromStdString(bindDevName));
		return false;
	}

	return true;
}

bool MaintainDataMng::ChFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 故障信息
	auto faultInfo = devInfo->faats;
	if (faultInfo.empty())
	{
		// 为空直接返回
		return true;
	}

	// 遍历维护组中所有维护项
	for (const auto& stuTfMaintainItem : mtGroup.items)
	{
		auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuTfMaintainItem.itemType);
		auto it = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devInfo->deviceType);

		// 不支持则跳过
		if (it == stuItem.deviceTypes.end())
		{
			continue;
		}

		switch (stuItem.itemType)
		{
		// 试剂扫描
		case tf::MaintainItemType::MAINTAIN_ITEM_CH_REAGENT_SCAN:
		{
			// 试剂舱盖未关闭
			if (faultInfo.find(tf::FaultAffectAppendTest::FAAT_RackRecycleAreaFull) != faultInfo.end())
			{
				noticeInfo = tr("%1设备试剂舱盖未关闭，不能执行试剂扫描。").arg(QString::fromStdString(devInfo->name));
				return false;
			}

			// 仪器前盖开启
			if (faultInfo.find(tf::FaultAffectAppendTest::FAAT_FrontCoverOpen) != faultInfo.end())
			{
				noticeInfo = tr("%1设备仪器前盖开启，不能执行试剂扫描。").arg(QString::fromStdString(devInfo->name));
				return false;
			}

			// 仪器后盖开启
			if (faultInfo.find(tf::FaultAffectAppendTest::FAAT_BackCoverOpen) != faultInfo.end())
			{
				noticeInfo = tr("%1设备仪器后盖开启，不能执行试剂扫描。").arg(QString::fromStdString(devInfo->name));
				return false;
			}

		}break;

		default:
			break;
		}
	}

	return true;

}

bool MaintainDataMng::CheckChDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 状态检查(运行或断开不可执行，生化下位机已支持停机状态下执行--2024.8.14-GZQ-bug14066)
	if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		noticeInfo = tr("%1设备处于运行态，不能执行维护。").arg(QString::fromStdString(devInfo->groupName + devInfo->name));
		return false;
	}
	else if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
	{
		noticeInfo = tr("%1设备断开连接，不能执行维护。").arg(QString::fromStdString(devInfo->groupName + devInfo->name));
		return false;
	}

	// 绑定设备状态判断
	if (!ChBindDevStatusCheck(noticeInfo, devInfo, mtGroup))
	{
		return false;
	}

	// 故障检查
	if (!ChFaultCheck(noticeInfo, devInfo, mtGroup))
	{
		return false;
	}

	// 耗材检查

	return true;
}

bool MaintainDataMng::IseBindDevStatusCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 单项维护判断
	if (mtGroup.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
		mtGroup.items.size() != 1)
	{
		return true;
	}

	// 需要对电极清洗进行判断，联机时，轨道只能在待机态才可执行
	auto maintainItem = mtGroup.items[0];
	if (maintainItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_ELECTRODE &&
		DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		// 获取轨道设备
		auto devList = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_TRACK });
		if (devList.size() != 1 ||
			devList[0] == nullptr)
		{
			noticeInfo = tr("获取轨道设备失败。");
			return false;
		}

		// 判断状态
		if (devList[0]->status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
		{
			noticeInfo = tr("电极清洗必须在轨道处于待机态才可执行。");
			return false;
		}
	}

	return true;
}

bool MaintainDataMng::IseFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 故障信息
	auto faultInfo = devInfo->faats;
	if (faultInfo.empty())
	{
		// 为空直接返回
		return true;
	}

	// 遍历维护组中所有维护项
	for (const auto& stuTfMaintainItem : mtGroup.items)
	{
		auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuTfMaintainItem.itemType);
		auto it = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devInfo->deviceType);

		// 不支持则跳过
		if (it == stuItem.deviceTypes.end())
		{
			continue;
		}

		switch (stuItem.itemType)
		{
		
		default:
			break;
		}
	}

	return true;
}

bool MaintainDataMng::CheckIseDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 状态检查(运行或断开不可执行，ISE下位机已支持停机状态下执行--2024.8.14-GZQ-bug14066)
	if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		noticeInfo = tr("%1设备处于运行态，不能执行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}
	else if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
	{
		noticeInfo = tr("%1设备断开连接，不能执行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}

	// 绑定设备状态判断
	if (!IseBindDevStatusCheck(noticeInfo, devInfo, mtGroup))
	{
		return false;
	}

	// 故障检查
	if (!IseFaultCheck(noticeInfo, devInfo, mtGroup))
	{
		return false;
	}

	// 耗材检查

	return true;
}

bool MaintainDataMng::CheckImDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 判断改维护是否是排空
	auto IsDrainMaintain = [](const tf::MaintainGroup& stuMtGrp)
	{
		// 遍历维护项
		for (auto MtItems : stuMtGrp.items)
		{
			// 如果为整机管理排空或在线配液排空，则返回true
			if (MtItems.itemType == tf::MaintainItemType::MAINTAIN_ITEM_DRAIN_PIP || MtItems.itemType == tf::MaintainItemType::MAINTAIN_ITEM_CONFECT_LIQUID_DRAIN)
			{
				return true;
			}
		}

		return false;
	};

	// 第一个要执行的是否是仪器复位
	auto IsFirstItemCanRun = [](tf::DeviceType::type devType, const tf::MaintainGroup& stuMtGrp)
	{
		// 遍历维护组中所有维护项
		for (const auto& stuTfMaintainItem : stuMtGrp.items)
		{
			auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuTfMaintainItem.itemType);
			auto it = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devType);

			// 不支持则跳过
			if (it == stuItem.deviceTypes.end())
			{
				continue;
			}

			// 第一个支持且如果是复位则返回true，否则返回false
			return (stuTfMaintainItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_RESET ||
				stuTfMaintainItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_AUTO_FAULT_REPAIR ||
				stuTfMaintainItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN || //0028581: [应用] 停机状态下无法执行“自动关机” added by zhang.changjiang 20240603
				stuTfMaintainItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_CONFECT_LIQUID_CHECK);//0030543: [应用] 停机状态无法执行“在线配液自检” added by zhang.changjiang 20240724
		}

		// 没有支持该设备的维护项，直接返回false
		return false;
	};

	// 状态检查
	// 停止态需要特殊判断
	if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_HALT &&
		!IsFirstItemCanRun(devInfo->deviceType, mtGroup))
	{
		noticeInfo = tr("%1仪器处于停机状态，单项维护只能执行仪器复位、仪器关机、在线配液自检和故障自动修复，组合维护首个维护项目只能是仪器复位或故障自动修复。").arg(QString::fromStdString(devInfo->name));
		return false;
	}
	// 运行态不可执行
	else if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		noticeInfo = tr("%1仪器处于运行状态，不能进行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}
	// 断开连接
	else if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
	{
		noticeInfo = tr("%1仪器断开连接，不能进行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}

	// 故障检查
	// TODO

	// 耗材检查
	// TODO

	// 免疫执行排空弹框提示
	if (IsDrainMaintain(mtGroup))
	{
		noticeInfo = tr("免疫仪器执行此维护后，无论成功与否，必须清空纯水桶、缓存桶、配液桶中残余液体；若需使用仪器，需重新开关电源后，重启上位机软件。");
	}

	return true;
}

bool MaintainDataMng::TrackFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 故障信息
	auto faultInfo = devInfo->faats;
	if (faultInfo.empty())
	{
		// 为空直接返回
		return true;
	}

	// 遍历维护组中所有维护项
	for (const auto& stuTfMaintainItem : mtGroup.items)
	{
		auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuTfMaintainItem.itemType);
		auto it = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devInfo->deviceType);

		// 不支持则跳过
		if (it == stuItem.deviceTypes.end())
		{
			continue;
		}

		switch (stuItem.itemType)
		{
		// 清除样本架
		case tf::MaintainItemType::MAINTAIN_ITEM_CLEAN_TRACK:
		{
			// 样本回收区满
			if (faultInfo.find(tf::FaultAffectAppendTest::FAAT_RackRecycleAreaFull) != faultInfo.end())
			{
				noticeInfo = tr("%1设备样本架回收区已满，不能执行清除样本架。").arg(QString::fromStdString(devInfo->name));
				return false;
			}

		}break;

		default:
			break;
		}
	}

	return true;
}

bool MaintainDataMng::CheckTrackDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup)
{
	// 防御
	if (devInfo == nullptr)	return false;
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devInfo->deviceSN);

	// 清空提示
	noticeInfo.clear();

	// 状态检查(停止和待机状态下都可执行复位和清除样本架)
	if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		noticeInfo = tr("%1设备处于运行态，不能执行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}
	else if (devInfo->status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
	{
		noticeInfo = tr("%1设备断开连接，不能执行维护。").arg(QString::fromStdString(devInfo->name));
		return false;
	}

	// 故障检查
	if (!TrackFaultCheck(noticeInfo, devInfo, mtGroup))
	{
		return false;
	}

	return true;
}

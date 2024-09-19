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
/// @file     rgntnoticedatamng.cpp
/// @brief    试剂提醒数据管理器
///
/// @author   4170/TangChuXian
/// @date     2023年8月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "rgntnoticedatamng.h"
#include "src/common/Mlog/mlog.h"

#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/ReagentCommon.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"

#include "uidcsadapter/uidcsadapter.h"

#include "src/public/im/ImConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"

RgntNoticeDataMng::RgntNoticeDataMng(QObject *parent)
    : QObject(parent),
      m_bInit(false),
      m_enLastAction(ACTION_TYPE_NONE),
      m_bRgntInfoUpdateReq(true),
      m_bDltInfoUpdateReq(true),
      m_bSplInfoUpdateReq(true)
{
    // 初始化信号槽
    InitConnect();

    // 加载数据
    LoadData();
}

RgntNoticeDataMng::~RgntNoticeDataMng()
{
    setParent(Q_NULLPTR);
}

///
/// @brief
///     获取单例
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
RgntNoticeDataMng* RgntNoticeDataMng::GetInstance()
{
    static RgntNoticeDataMng s_obj;
    if (!s_obj.m_bInit)
    {
        s_obj.LoadData();
    }
    return &s_obj;
}

///
/// @brief
///     试剂剩余测试数是否提醒
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  iResidetest 剩余测试数
///
/// @return true表示需要提醒
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月10日，新建函数
///
bool RgntNoticeDataMng::IsRgntResidualTestNotice(int iAssayCode, int iResidetest)
{
    // 获取报警值
    // 从缓存中获取项目信息
    auto it = m_mapRgntBottleVol.find(iAssayCode);
    if (it == m_mapRgntBottleVol.end() || iResidetest > it.value())
    {
        return false;
    }

    return true;
}

///
/// @brief
///     试剂盘上是否存在空试剂或空稀释液
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月9日，新建函数
///
bool RgntNoticeDataMng::IsExistEmptyRgntOrDlt()
{
    // 构造查询条件和查询结果
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 筛选条件
    qryCond.__set_deviceSNs(m_vStrAllImDevSns);

    // 查询所有试剂信息
    bool bRet = QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return false;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::ReagentInfoTable& stuRgntInfo : qryResp.lstReagentInfos)
    {
        // 液位探测失败，也是空瓶
        if (stuRgntInfo.lLDError > 0)
        {
            return true;
        }

        // 如果有空瓶，返回true
        if (stuRgntInfo.residualTestNum <= 0)
        {
            return true;
        }
    }

    // 查询稀释液
    // 构造查询条件和查询结果
    ::im::tf::DiluentInfoTableQueryResp qryDltResp;

    // 查询所有试剂信息
    bRet = QueryDiluentInfoForUI(qryDltResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDiluentInfoForUI failed.");
        return false;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::DiluentInfoTable& stuDltInfo : qryDltResp.lstDiluentInfos)
    {
        // 液位探测失败，也是空瓶
        if (stuDltInfo.LLDErr > 0)
        {
            return true;
        }

        // 如果有空瓶，返回true
        if (stuDltInfo.curVol < DBL_EPSILON)
        {
            return true;
        }
    }

    // 返回false
    return false;
}

///
/// @brief
///     试剂可用测试数是否提醒
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  iAvatest    可用测试数
///
/// @return true表示提醒
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月3日，新建函数
///
bool RgntNoticeDataMng::IsRgntAvaTestNotice(int iAssayCode, int iAvatest)
{
    // 获取报警值
    // 从缓存中获取项目信息
    auto it = m_mapAssayAlarmVol.find(iAssayCode);
    if (it == m_mapAssayAlarmVol.end() || iAvatest > it.value())
    {
        return false;
    }

    return true;
}

///
/// @brief
///     稀释液剩余量是否需要提醒
///
/// @param[in]  iDltNum  稀释液编号
/// @param[in]  dReVol   剩余量
///
/// @return true表示需要提醒
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月10日，新建函数
///
bool RgntNoticeDataMng::IsDltCurVolNotice(int iDltNum, double dReVol)
{
    // 获取稀释液类型
    DltType enDltType = DLT_TYPE_DILUENT;
    if (IsSpecialWashing(iDltNum))
    {
        enDltType = DLT_TYPE_SPEC_WASHING;
    }

    // 获取报警值
    // 从缓存中获取项目信息
    auto itAlarm = m_mapDltBottleAlarmVol.find(enDltType);
    if (itAlarm == m_mapDltBottleAlarmVol.end() || dReVol > itAlarm.value())
    {
        return false;
    }

    return true;
}

///
/// @brief
///     稀释液可用量是否需要提醒
///
/// @param[in]  iDltNum  稀释液编号
/// @param[in]  dReVol   剩余量
///
/// @return true表示需要提醒
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月10日，新建函数
///
bool RgntNoticeDataMng::IsDltAvaVolNotice(int iDltNum, double dAvaVol)
{
    // 获取稀释液类型
    DltType enDltType = DLT_TYPE_DILUENT;
    if (IsSpecialWashing(iDltNum))
    {
        enDltType = DLT_TYPE_SPEC_WASHING;
    }

    // 获取报警值
    // 从缓存中获取项目信息
    auto itAlarm = m_mapDltTypeAlarmVol.find(enDltType);
    if (itAlarm == m_mapDltTypeAlarmVol.end() || dAvaVol > itAlarm.value())
    {
        return false;
    }

    return true;
}

///
/// @brief
///     耗材余量是否需要提醒
///
/// @param[in]  iSplType  耗材类型
/// @param[in]  dReVol    耗材余量
///
/// @return true表示需要提醒
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月17日，新建函数
///
bool RgntNoticeDataMng::IsSplReVolNotice(int iSplType, double dReVol)
{
    // 获取报警值
    // 从缓存中获取项目信息
    auto itAlarm = m_mapSplAlarmVol.find(iSplType);
    if (itAlarm == m_mapSplAlarmVol.end())
    {
        return false;
    }

    // 如果是废料桶、废液桶，大于报警值报警
    if (iSplType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID || iSplType == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID)
    {
        if (dReVol < itAlarm.value())
        {
            return false;
        }
    }
    else
    {
        if (dReVol > itAlarm.value())
        {
            return false;
        }
    }

    return true;
}

///
/// @brief 试剂加载机构试剂槽是否存在试剂盒
///
/// @param[oyut]	bExist  true-存在，false-不存在
/// @param[in]		deviceSN  设备编号
///
/// @return true-获取成功，false-获取失败
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月4日，新建函数
///
bool RgntNoticeDataMng::ReagentLoadExistReag(bool& bExist, std::string& deviceSN)
{
    ULOG(LOG_INFO, "%s(%d,%s)", __FUNCTION__, int(bExist), deviceSN.c_str());
    // 加载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
    ::tf::ResultBool _return_existReag;
    im::LogicControlProxy::QueryReagentLoadUnloadExistReagent(_return_existReag, deviceSN);
    if (_return_existReag.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }
    bExist = _return_existReag.value;

    return true;
}

///
/// @brief
///     查询试剂信息
///
/// @param[in]  qryResp  返回结果
/// @param[in]  qryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
bool RgntNoticeDataMng::QueryReagentInfoForUI(::im::tf::ReagentInfoTableQueryResp& qryResp, const ::im::tf::ReagTableUIQueryCond& qryCond)
{
    // 如果是筛选，则直接查询数据库
    if (qryCond.__isset.reagMask ||
        qryCond.__isset.caliMask ||
        qryCond.__isset.onUse ||
        qryCond.__isset.backUp ||
        qryCond.__isset.stateEmpty ||
        qryCond.__isset.caliSucc ||
        qryCond.__isset.noCali ||
        qryCond.__isset.beCaling ||
        qryCond.__isset.caliFail ||
        qryCond.__isset.caliEmpty ||
        qryCond.__isset.scanFail ||
        qryCond.__isset.normalReag)
    {
        ULOG(LOG_INFO, "QueryReagentInfoForUI screen.");
        return ::im::LogicControlProxy::QueryReagentInfoForUIDirectly(qryResp, qryCond);
    }

    // 如果数据需要更新，则查询数据库并更新缓存
    if (m_bRgntInfoUpdateReq)
    {
        // 构造查询条件和查询结果
        ::im::tf::ReagentInfoTableQueryResp qryDataResp;
        ::im::tf::ReagTableUIQueryCond qryDataCond;

        // 筛选条件
        std::vector<std::string> vtDv;
        qryDataCond.__set_deviceSNs(m_vStrAllImDevSns);

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUIDirectly(qryDataResp, qryDataCond);
        if (!bRet || qryDataResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            // 后台设计问题，结果为空也会返回失败
            ULOG(LOG_ERROR, "QueryReagentInfoForUIDirectly failed.");
        }

        // 更新缓存
        m_mapRgntInfo.clear();
        for (const auto& stuRgntInfo : qryDataResp.lstReagentInfos)
        {
            m_mapRgntInfo.insert(stuRgntInfo.id, stuRgntInfo);
        }

        // 更新标志
        m_bRgntInfoUpdateReq = false;
    }

    // 是否匹配查询条件
    auto IsRgntInfoMatchQryCond = [](const im::tf::ReagentInfoTable& stuRgntInfo, const ::im::tf::ReagTableUIQueryCond& qryCond)
    {
        // 项目编号
        if (qryCond.__isset.assayCode && (stuRgntInfo.assayCode != qryCond.assayCode))
        {
            return false;
        }

        // ID
        if (qryCond.__isset.reagentId && (stuRgntInfo.id != qryCond.reagentId))
        {
            return false;
        }

        // ID
        if (qryCond.__isset.reagentId && (stuRgntInfo.id != qryCond.reagentId))
        {
            return false;
        }

        // 设备序列号
        if (!qryCond.deviceSNs.empty() && std::find(qryCond.deviceSNs.begin(), qryCond.deviceSNs.end(), stuRgntInfo.deviceSN) == qryCond.deviceSNs.end())
        {
            return false;
        }

        return true;
    };

    // 遍历缓存，筛选结果
    qryResp.lstReagentInfos.clear();
    qryResp.__set_result(tf::ThriftResult::THRIFT_RESULT_SUCCESS);
    for (auto it = m_mapRgntInfo.begin(); it != m_mapRgntInfo.end(); it++)
    {
        // 不匹配查询条件则跳过
        if (!IsRgntInfoMatchQryCond(it.value(), qryCond))
        {
            continue;
        }

        // 插入试剂信息
        qryResp.lstReagentInfos.push_back(it.value());
    }

    return true;
}

///
/// @brief
///     查询稀释液信息
///
/// @param[in]  qryResp  返回结果
/// @param[in]  qryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
bool RgntNoticeDataMng::QueryDiluentInfoForUI(::im::tf::DiluentInfoTableQueryResp& qryResp, const ::im::tf::ReagTableUIQueryCond& qryCond)
{
    // 如果是筛选，则直接查询数据库
    if (qryCond.__isset.reagMask ||
        qryCond.__isset.caliMask ||
        qryCond.__isset.onUse ||
        qryCond.__isset.backUp ||
        qryCond.__isset.stateEmpty ||
        qryCond.__isset.caliSucc ||
        qryCond.__isset.noCali ||
        qryCond.__isset.beCaling ||
        qryCond.__isset.caliFail ||
        qryCond.__isset.caliEmpty ||
        qryCond.__isset.scanFail ||
        qryCond.__isset.normalReag)
    {
        ULOG(LOG_INFO, "QueryDiluentInfoForUIDirectly screen.");
        return ::im::LogicControlProxy::QueryDiluentInfoForUIDirectly(qryResp, qryCond);
    }

    // 如果数据需要更新，则查询数据库并更新缓存
    if (m_bDltInfoUpdateReq)
    {
        // 构造查询条件和查询结果
        ::im::tf::DiluentInfoTableQueryResp qryDataResp;
        ::im::tf::ReagTableUIQueryCond qryDataCond;

        // 筛选条件
        std::vector<std::string> vtDv;
        qryDataCond.__set_deviceSNs(m_vStrAllImDevSns);

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryDiluentInfoForUIDirectly(qryDataResp, qryDataCond);
        if (!bRet || qryDataResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QueryDiluentInfoForUIDirectly failed.");
        }

        // 更新缓存
        m_mapDltInfo.clear();
        for (const auto& stuDltInfo : qryDataResp.lstDiluentInfos)
        {
            m_mapDltInfo.insert(stuDltInfo.id, stuDltInfo);
        }

        // 更新标志
        m_bDltInfoUpdateReq = false;
    }

    // 是否匹配查询条件
    auto IsDltInfoMatchQryCond = [](const im::tf::DiluentInfoTable& stuDltInfo, const ::im::tf::ReagTableUIQueryCond& qryCond)
    {
        // ID
        if (qryCond.__isset.reagentId && (stuDltInfo.id != qryCond.reagentId))
        {
            return false;
        }

        // 设备序列号
        if (!qryCond.deviceSNs.empty() && std::find(qryCond.deviceSNs.begin(), qryCond.deviceSNs.end(), stuDltInfo.deviceSN) == qryCond.deviceSNs.end())
        {
            return false;
        }

        return true;
    };

    // 遍历缓存，筛选结果
    qryResp.lstDiluentInfos.clear();
    qryResp.__set_result(tf::ThriftResult::THRIFT_RESULT_SUCCESS);
    for (auto it = m_mapDltInfo.begin(); it != m_mapDltInfo.end(); it++)
    {
        // 不匹配查询条件则跳过
        if (!IsDltInfoMatchQryCond(it.value(), qryCond))
        {
            continue;
        }

        // 插入试剂信息
        qryResp.lstDiluentInfos.push_back(it.value());
    }

    return true;
}

///
/// @brief
///     查询耗材信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
bool RgntNoticeDataMng::QuerySuppliesInfo(::im::tf::SuppliesInfoTableQueryResp& qryResp, const ::im::tf::SuppliesInfoTableQueryCond& qryCond)
{
    // 如果是查询未加载的耗材，直接查询数据库（只缓存已上机的耗材）
    if (qryCond.__isset.isLoad && (bool(qryCond.isLoad) == false))
    {
        ULOG(LOG_WARN, "bool(qryCond.isLoad) == false).");
        return ::im::LogicControlProxy::QuerySuppliesInfoDirectly(qryResp, qryCond);
    }

    // 如果数据需要更新，则查询数据库并更新缓存
    if (m_bSplInfoUpdateReq)
    {
        // 构造查询条件和查询结果
        ::im::tf::SuppliesInfoTableQueryResp qryDataResp;
        ::im::tf::SuppliesInfoTableQueryCond qryDataCond;

        // 筛选条件
        std::vector<std::string> vtDv;
        qryDataCond.__set_isLoad(true);

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QuerySuppliesInfoDirectly(qryDataResp, qryDataCond);
        if (!bRet || qryDataResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QuerySuppliesInfoDirectly failed.");
        }

        // 更新缓存
        m_mapSplInfo.clear();
        for (const auto& stuSplInfo : qryDataResp.lstSuppliesInfos)
        {
            m_mapSplInfo.insert(stuSplInfo.iD, stuSplInfo);
        }

        // 更新标志
        m_bSplInfoUpdateReq = false;
    }

    // 是否匹配查询条件
    auto IsSplInfoMatchQryCond = [](const im::tf::SuppliesInfoTable& stuSplInfo, const ::im::tf::SuppliesInfoTableQueryCond& qryCond)
    {
        // ID
        if (qryCond.__isset.id && (stuSplInfo.iD != qryCond.id))
        {
            return false;
        }

        // 设备序列号
        if (qryCond.__isset.deviceSN && (qryCond.deviceSN != stuSplInfo.deviceSN))
        {
            return false;
        }

        // 类型
        if (qryCond.__isset.supType && (stuSplInfo.supType != qryCond.supType))
        {
            return false;
        }

        // 批号
        if (qryCond.__isset.supLot && (stuSplInfo.supLot != qryCond.supLot))
        {
            return false;
        }

        // 瓶号
        if (qryCond.__isset.strSerial && (stuSplInfo.supSerial != qryCond.strSerial))
        {
            return false;
        }

        // 组号
        if (qryCond.__isset.groupNum && (stuSplInfo.groupNum != qryCond.groupNum))
        {
            return false;
        }

        // 使用状态
        if (qryCond.__isset.usingStatus && (stuSplInfo.usingStatus != qryCond.usingStatus))
        {
            return false;
        }

        return true;
    };

    // 遍历缓存，筛选结果
    qryResp.lstSuppliesInfos.clear();
    qryResp.__set_result(tf::ThriftResult::THRIFT_RESULT_SUCCESS);
    for (auto it = m_mapSplInfo.begin(); it != m_mapSplInfo.end(); it++)
    {
        // 不匹配查询条件则跳过
        if (!IsSplInfoMatchQryCond(it.value(), qryCond))
        {
            continue;
        }

        // 插入试剂信息
        qryResp.lstSuppliesInfos.push_back(it.value());
    }

    return true;
}

///
/// @brief
///     是否可以加卸载试剂
///
/// @param[in]  strDevSn  设备序列号
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
bool RgntNoticeDataMng::IsCanLoadReagent(const QString& strDevSn)
{
    // 获取当前设备信息
    auto stuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn.toStdString());
    if (stuTfDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), stuTfDevInfo == Q_NULLPTR", __FUNCTION__);
        return false;
    }

    // 如果不是维护状态，直接返回true
    if (stuTfDevInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
    {
        return true;
    }

    // 如果被包含在可加卸载试剂设备列表中，直接返回true
    if (m_strCanLoadRgntDevSnLst.contains(strDevSn))
    {
        return true;
    }

    return false;
}

///
/// @brief
///     是否可以加卸载试剂
///
/// @param[in]  strDevSn  设备序列号
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
bool RgntNoticeDataMng::IsCanLoadReagent(const std::string& strDevSn)
{
    Q_UNUSED(strDevSn);
    // 由于后台无法显示相关流程，暂时屏蔽该判断，直接返回true
    return true;
    //return IsCanLoadReagent(QString::fromStdString(strDevSn));
}

///
/// @brief
///     试剂加载功能是否被屏蔽
///
/// @param[in]  bMask       是否屏蔽
/// @param[in]  deviceSN    设备序列号
///
/// @return true表示执行成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
bool RgntNoticeDataMng::IsReagentLoadIsMask(bool& bMask, const std::string& deviceSN)
{
    // 查询缓存
    auto it = m_mapDevRgntLoadMask.find(deviceSN);
    if (it != m_mapDevRgntLoadMask.end())
    {
        bMask = it.value();
        return true;
    }

    // 从后台查询
    // 检查试剂在线加载是否被屏蔽
    ::tf::ResultLong _return_mask;
    im::i6000::LogicControlProxy::CheckLoadUnloadIsMask(_return_mask, deviceSN);
    bool bRet = false;
    if (_return_mask.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
    }
    else
    {
        bMask = _return_mask.value;
        bRet = true;
    }

    // 更新标志
    m_mapDevRgntLoadMask.insert(deviceSN, bMask);
    return bRet;
}

///
/// @brief
///     设置装载装置复位正在执行
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bExecute  是否正在执行
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月22日，新建函数
///
void RgntNoticeDataMng::SetDevExcuteRgntLoaderReset(const std::string& strDevSn, bool bExecute)
{
    if (bExecute)
    {
        m_setDevRgntLoaderReset.insert(QString::fromStdString(strDevSn));
    }
    else
    {
        m_setDevRgntLoaderReset.remove(QString::fromStdString(strDevSn));
    }
}

///
/// @brief
///     设置设备装载装置执行失败
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bFaulst   是否加卸载失败
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月22日，新建函数
///
void RgntNoticeDataMng::SetDevRgntLoaderFault(const std::string& strDevSn, bool bFaulst)
{
    if (bFaulst)
    {
        m_setDevRgntLoaderFault.insert(QString::fromStdString(strDevSn));
    }
    else
    {
        m_setDevRgntLoaderFault.remove(QString::fromStdString(strDevSn));
    }
}

///
/// @brief
///     试剂装载装置是否正在复位
///
/// @param[in]  strDevSn  设备序列号
///
/// @return true正在复位
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月22日，新建函数
///
bool RgntNoticeDataMng::IsRgntLoaderResetting(const std::string& strDevSn)
{
    auto it = m_setDevRgntLoaderReset.find(QString::fromStdString(strDevSn));
    return (it != m_setDevRgntLoaderReset.end());
}

///
/// @brief
///     是否上一次试剂加卸载失败
///
/// @param[in]  strDevSn  设备序列还
///
/// @return true表示上一次加卸载失败
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月22日，新建函数
///
bool RgntNoticeDataMng::IsRgntLoaderFault(const std::string& strDevSn)
{
    auto it = m_setDevRgntLoaderFault.find(QString::fromStdString(strDevSn));
    return (it != m_setDevRgntLoaderFault.end());
}

///
/// @brief
///     获取耗材停用信息
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月27日，新建函数
///
ImSuppliesEnableInfo RgntNoticeDataMng::GetSplEnableInfo(const std::string& strDevSn)
{
    // 如果已缓存，则直接取缓存中的数据
    auto it = m_mapSplEnable.find(strDevSn);
    if (it != m_mapSplEnable.end())
    {
        return it.value();
    }

    // 构造返回值
    ImSuppliesEnableInfo retSplEnable;

    // 否则查询数据库
    // 直接查询设备
    tf::DeviceInfoQueryCond qryCond;
    tf::DeviceInfoQueryResp qryResp;
    qryCond.__set_deviceSN(strDevSn);
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(qryResp, qryCond))
    {
        ULOG(LOG_ERROR, "%s(), QueryDeviceInfo faild", __FUNCTION__);
        return retSplEnable;
    }

    // 如果结果为空，则返回
    if (qryResp.lstDeviceInfos.empty())
    {
        ULOG(LOG_INFO, "%s(), qryResp.lstDeviceInfos.empty()", __FUNCTION__);
        return retSplEnable;
    }

    // 更新设备信息
    auto& curDevIndo = qryResp.lstDeviceInfos[0];

    // 解析otherinfo
    ImDeviceOtherInfo imdoi;
    if (!DecodeJson(imdoi, curDevIndo.otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
        return retSplEnable;
    }

    // 更新耗材使能信息
    retSplEnable.bBaseLiquidGrp1 = imdoi.bSubstrate1;
    retSplEnable.bBaseLiquidGrp2 = imdoi.bSubstrate2;
    retSplEnable.bCleanBuffer1 = imdoi.bConfectLiquid1;
    retSplEnable.bCleanBuffer2 = imdoi.bConfectLiquid2;

    // 更新缓存
    m_mapSplEnable.insert(strDevSn, retSplEnable);
    return retSplEnable;
}

///
/// @brief
///     加载数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月15日，新建函数
///
void RgntNoticeDataMng::LoadData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果没有免疫设备，则直接跳过加载数据（免疫生化剩余测试数提醒显示需求差异）
    if (!gUiAdapterPtr()->WhetherContainImDev())
    {
        // 无需加载数据
        m_bInit = true;
        return;
    }

    // 获取所有免疫设备
    QStringList strImDevSnList;
    strImDevSnList = gUiAdapterPtr()->GetImDevSnLst();
    for (const QString& strDevSn : strImDevSnList)
    {
        m_vStrAllImDevSns.push_back(strDevSn.toStdString());
    }

    // 更新报警值
    UpdateDltAlarmVol();
    UpdateAssayAlarmVol();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void RgntNoticeDataMng::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果没有免疫设备，则直接跳过加载数据（免疫生化剩余测试数提醒显示需求差异）
    if (!gUiAdapterPtr()->WhetherContainImDev())
    {
        // 无需加载数据
        return;
    }

    // 拿走试剂盒提示消息
    REGISTER_HANDLER(MSG_ID_REAGENT_TAKE_MESSAGE, this, OnTakeReagentMessage);

    // 试剂报警值更新
    REGISTER_HANDLER(MSG_ID_RGNT_ALARM_VOL_UPDATE, this, UpdateAssayAlarmVol);
    REGISTER_HANDLER(MSG_ID_RGNT_ALARM_VOL_UPDATE, this, UpdateDltAlarmVol);

    // 耗材报警值更新
    REGISTER_HANDLER(MSG_ID_SPL_ALARM_VAL_UPDATE, this, UpdateDltAlarmVol);

    // 试剂加卸载状态更新
    REGISTER_HANDLER(MSG_ID_REAGENT_LOAD_STATUS_UPDATE, this, OnRgntLoadStatusChanged);

    // 申请更换试剂结果消息
    REGISTER_HANDLER(MSG_ID_MANUAL_HANDL_REAG, this, OnManualHandleReagResult);

    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateReagentInfo);

    // 监听稀释信息更新
    REGISTER_HANDLER(MSG_ID_IM_DILUENT_INFO_UPDATE, this, UpdateImDltChanged);

    // 监听耗材信息更新
    REGISTER_HANDLER(MSG_ID_IM_SUPPLY_INFO_UPDATE, this, UpdateImSplChanged);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);

    // 功能管理更新
    REGISTER_HANDLER(MSG_ID_UTILITY_FUNCTION_MNG_UPDATE, this, OnFunctionMngUpdate);

    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DCS_REPORT_STATUS_CHANGED, this, UpdateDeviceStatus);

    // 监听耗材管理设置更新
    REGISTER_HANDLER(MSG_ID_CONSUMABLES_MANAGE_UPDATE, this, OnSplMngCfgChanged);

    // 试剂装载装置复位结束
    REGISTER_HANDLER(MSG_ID_IM_RGNT_LOADER_RESET_FINISH, this, OnRgntLoaderResetFinished);
}

///
/// @brief
///     弹框提示请取走试剂盒
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月23日，新建函数
///
void RgntNoticeDataMng::PopTakeRgntTip(const QString& strDevSn)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, strDevSn.toStdString().c_str());
    static QStringList s_strDevSnList;
    if (s_strDevSnList.contains(strDevSn))
    {
        // 获取写权限失败
        ULOG(LOG_INFO, "%s(%s), !s_Locker.tryLockForWrite()", __FUNCTION__, strDevSn.toStdString().c_str());
        return;
    }
    s_strDevSnList.push_back(strDevSn);

    // 获取设备名
    QPair<QString, QString> strPairName = gUiAdapterPtr()->GetDevNameBySn(strDevSn);

    // 提示用户拿走试剂盒
    QString strTip = QString("【") + strPairName.first + QString("】") + tr("：请取走试剂盒！");
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(strTip, TipDlgType::SINGLE_BUTTON));

    // 最多提示三次
    const int iLoopAsk = 3;
    bool bExist = false;
    for (int i = 0; i < iLoopAsk; i++)
    {
        // 弹框提示
        pTipDlg->exec();

        // 检查试剂盒是否被取走
        if (ReagentLoadExistReag(bExist, strDevSn.toStdString()))
        {
            ULOG(LOG_ERROR, "%s(%s), ReagentLoadExistReag failed", __FUNCTION__, strDevSn.toStdString().c_str());
            break;;
        }

        // 如果试剂盒已经不存在，则退出
        if (!bExist)
        {
            break;
        }

    }

    // 试剂盒依然没被取出（事不过三，不再提醒）
    if (bExist)
    {
        // 报警提示用户拿走试剂盒 TODO
        ULOG(LOG_WARN, "%s(%s), reagent has not been take out.", __FUNCTION__, strDevSn.toStdString().c_str());
    }

    // 解锁
    s_strDevSnList.removeOne(strDevSn);
}

///
/// @brief
///     更新稀释液报警值
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月10日，新建函数
///
void RgntNoticeDataMng::UpdateDltAlarmVol()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空缓存
    m_mapDltBottleAlarmVol.clear();
    m_mapDltTypeAlarmVol.clear();
    m_mapSplAlarmVol.clear();

    // 查询稀释液报警值
    ::im::tf::SupplyAttributeQueryResp splAttrQryResp;
    ::im::tf::SupplyAttributeQueryCond splAttrQryCond;

    // 查询耗材属性
    bool bRet = ::im::LogicControlProxy::QuerySupplyAttribute(splAttrQryResp, splAttrQryCond);
    if (!bRet || splAttrQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed to query all im supplyattribute.");
        return;
    }

    // 遍历耗材属性，插入稀释液和特殊清洗液的报警值
    for (const auto& stuSplAttr : splAttrQryResp.lstSupplyAttributes)
    {
        // 单位转换（0.1ul -> ml）
        double iBottleAlarmVolML = stuSplAttr.bottleAlarmThreshold / 10000.0;
        double iAssayAlarmVolML = stuSplAttr.assayAlarmThreshold / 10000.0;

        // 稀释液
        if (stuSplAttr.type == im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT)
        {
            m_mapDltBottleAlarmVol.insert(DLT_TYPE_DILUENT, iBottleAlarmVolML);
            m_mapDltTypeAlarmVol.insert(DLT_TYPE_DILUENT, iAssayAlarmVolML);
            continue;
        }

        // 特殊清洗液
        if (stuSplAttr.type == im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH)
        {
            m_mapDltBottleAlarmVol.insert(DLT_TYPE_SPEC_WASHING, iBottleAlarmVolML);
            m_mapDltTypeAlarmVol.insert(DLT_TYPE_SPEC_WASHING, iAssayAlarmVolML);
            continue;
        }

        // 其他耗材
        m_mapSplAlarmVol.insert(stuSplAttr.type, iBottleAlarmVolML);
    }

    // 初始化标志置为true
    m_bInit = true;
}

///
/// @brief
///     更新项目报建值
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月15日，新建函数
///
void RgntNoticeDataMng::UpdateAssayAlarmVol()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空映射
    m_mapRgntBottleVol.clear();
    m_mapAssayAlarmVol.clear();

    // 查询所有项目信息
    tf::GeneralAssayInfoQueryCond qryCond;
    tf::GeneralAssayInfoQueryResp qryResp;
    if (!DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QueryGeneralAssayInfo() failed");
        return;
    }

    // 遍历所有项目信息
    for (const auto& stuAssayInfo : qryResp.lstAssayInfos)
    {
        m_mapRgntBottleVol.insert(stuAssayInfo.assayCode, stuAssayInfo.bottleAlarmThreshold);
        m_mapAssayAlarmVol.insert(stuAssayInfo.assayCode, stuAssayInfo.assayAlarmThreshold);
    }

    // 初始化标志置为true
    m_bInit = true;
}

///
/// @brief
///     试剂仓加卸载是否正在运行
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bRunning  是否正在运行
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月30日，新建函数
///
void RgntNoticeDataMng::OnRgntLoadStatusChanged(QString strDevSn, bool bRunning)
{
    ULOG(LOG_INFO, "%s(%s,%d)", __FUNCTION__, strDevSn.toStdString().c_str(), int(bRunning));
    // 缓存设备动作状态
    ActionType enActType = GetLastAction();
    SetDevActionStatus(strDevSn, enActType, bRunning);
}

///
/// @brief
///     拿走试剂盒消息
///
/// @param[in]  strDevSn  设备编号
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月4日，新建函数
/// @li 4170/TangChuXian，2023年9月8日，逻辑完善
///
void RgntNoticeDataMng::OnTakeReagentMessage(QString strDevSn)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, strDevSn.toStdString().c_str());
    // 如果启动了卸载
    if (IsStartUnload(strDevSn))
    {
        PopTakeRgntTip(strDevSn);
    }

    // 重置卸载状态
    SetStartUnload(strDevSn, false);
}

///
/// @brief 响应申请试剂结果消息
///
/// @param[in]  deviceSN  设备编号
/// @param[in]  result    是否接受
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月11日，新建函数
/// @li 4170/TangChuXian，2024年1月18日，逻辑完善
///
void RgntNoticeDataMng::OnManualHandleReagResult(const QString& deviceSN, const int result)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, deviceSN.toStdString().c_str());
    // 收到申请成功消息
    if (result == RGNT_CHANGE_RLT_ACCEPT)
    {
        SetApplyChangeRgnt(deviceSN.toStdString(), RGNT_CHANGE_APPLY_STATUS_HANDLE);
    }
    else
    {
        SetApplyChangeRgnt(deviceSN.toStdString(), RGNT_CHANGE_APPLY_STATUS_ALLOW_APPLY);
    }

    // 获取结果——mod_tcx 改为状态栏提醒，不再弹框提示
//     QString strRlt("");
//     if (result == RGNT_CHANGE_RLT_CANCEL)
//     {
//         strRlt = tr("申请失败");
//     }
//     else if (RGNT_CHANGE_RLT_ACCEPT)
//     {
//         strRlt = tr("申请成功");
//     }
//     else
//     {
//         strRlt.clear();
//     }
// 
//     // 获取设备名
//     QPair<QString, QString> pairDevName = gUiAdapterPtr()->GetDevNameBySn(deviceSN);
//     QString strCombineName = pairDevName.first == pairDevName.second ? pairDevName.first : pairDevName.second + pairDevName.first;
// 
//     // 弹窗提示用户申请更换试剂结果
//     if (result != RGNT_CHANGE_RLT_FINISH)
//     {
//         TipDlg tipDlg(tr("申请更换试剂"), tr("设备【%1】申请更换试剂结果:%2").arg(strCombineName).arg(strRlt), TipDlgType::SINGLE_BUTTON);
//         tipDlg.exec();
//     }
}

///
/// @brief
///     更新试剂信息
///
/// @param[in]  srr   试剂信息
/// @param[in]  type  试剂更新类型
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
void RgntNoticeDataMng::UpdateReagentInfo(const im::tf::ReagentInfoTable& srr, const im::tf::ChangeType::type type)
{
    Q_UNUSED(srr);
    Q_UNUSED(type);
    m_bRgntInfoUpdateReq = true;
}

///
/// @brief
///     免疫稀释液信息更新
///
/// @param[in]  stuDltInfo   试剂信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
void RgntNoticeDataMng::UpdateImDltChanged(const im::tf::DiluentInfoTable& stuDltInfo, im::tf::ChangeType::type changeType)
{
    Q_UNUSED(stuDltInfo);
    Q_UNUSED(changeType);
    m_bDltInfoUpdateReq = true;
}

///
/// @brief
///     免疫耗材信息更新
///
/// @param[in]  stuSplInfo   耗材信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月24日，新建函数
///
void RgntNoticeDataMng::UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType)
{
    Q_UNUSED(stuSplInfo);
    Q_UNUSED(changeType);
    m_bSplInfoUpdateReq = true;
}

///
/// @brief
///     设备状态改变
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
void RgntNoticeDataMng::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
    // 设备状态转为运行，将更换申请按钮置为可点击
    if (deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
    {
        SetApplyChangeRgnt(deviceInfo.deviceSN, RGNT_CHANGE_APPLY_STATUS_ALLOW_APPLY);
    }

    // 如果状态不为维护，尝试从列表移除
    QString strDevSn = QString::fromStdString(deviceInfo.deviceSN);
    m_strCanLoadRgntDevSnLst.removeOne(strDevSn);
    if (deviceInfo.status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
    {
        return;
    }

    // 如果不能加卸载，则禁能
    if (!im::LogicControlProxy::IsCanLoadReagent(deviceInfo.deviceSN))
    {
        return;
    }

    // 假如可加卸载设备列表
    m_strCanLoadRgntDevSnLst.push_back(strDevSn);
}

///
/// @brief
///     功能管理更新
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
void RgntNoticeDataMng::OnFunctionMngUpdate()
{
    // 清空缓存记录
    m_mapDevRgntLoadMask.clear();
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月4日，新建函数
///
void RgntNoticeDataMng::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    Q_UNUSED(mapUpdateInfo);
    m_bRgntInfoUpdateReq = true;
}

///
/// @brief	设备信息更新
///     
/// @par History:
/// @li 7951/LuoXin，2023年6月5日，新建函数
/// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
///
void RgntNoticeDataMng::UpdateDeviceStatus(tf::DeviceInfo deviceInfo)
{
    if (deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_WAIT_OPEN_LID)
    {
        // 置为正在申请状态
        SetApplyChangeRgnt(deviceInfo.deviceSN, RGNT_CHANGE_APPLY_STATUS_APPLYING);
    }
}

///
/// @brief
///     耗材管理设置改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  stuSplMng   耗材管理
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月27日，新建函数
///
void RgntNoticeDataMng::OnSplMngCfgChanged(QString strDevSn, ImDeviceOtherInfo stuSplMng)
{
    Q_UNUSED(stuSplMng);
    m_mapSplEnable.remove(strDevSn.toStdString());
}

///
/// @brief
///     试剂装载装置复位结束
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  rlt         复位结果
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月19日，新建函数
///
void RgntNoticeDataMng::OnRgntLoaderResetFinished(QString strDevSn, im::i6000::tf::ReagentLoaderResetRlt::type rlt)
{
    // 获取结果——mod_tcx 改为状态栏提醒，不再弹框提示
    QString strRlt("");
    if (rlt == im::i6000::tf::ReagentLoaderResetRlt::REAGENT_LOADER_RESET_RLT_FAIL)
    {
        strRlt = tr("：执行装载装置复位失败。");
    }
    else if (rlt == im::i6000::tf::ReagentLoaderResetRlt::REAGENT_LOADER_RESET_RLT_SUCCUSS)
    {
        // 设置卸载结束
        strRlt = tr("：执行装载装置复位成功。");
    }
    else if (rlt == im::i6000::tf::ReagentLoaderResetRlt::REAGENT_LOADER_RESET_RLT_EXIST_REAGENT)
    {
        PopTakeRgntTip(strDevSn);
        return;
    }
    else
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> pairDevName = gUiAdapterPtr()->GetDevNameBySn(strDevSn);
    QString strCombineName = pairDevName.first == pairDevName.second ? pairDevName.first : pairDevName.second + pairDevName.first;

    // 弹窗提示用户申请更换试剂结果
    QString strTip = "【" + strCombineName + "】" + strRlt;
    TipDlg tipDlg(tr("装载装置复位"), strTip, TipDlgType::SINGLE_BUTTON);
    tipDlg.exec();

    // 重置卸载状态
    SetStartUnload(strDevSn, false);
}

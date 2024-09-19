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
/// @file     devc1005.cpp
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
#include "devc1005.h"
#include "boost/optional/optional.hpp"
#include "adaptertypedef.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_constants.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "shared/DataManagerQc.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "src/common/TimeUtil.h"
#include "src/common/defs.h"

#include <QSet>

// 初始化静态成员变量
QMap<QPair<QString, QString>, QVariant>          DevC1005::sm_mapDevNameInfo;         // 设备名和设备信息

// 注册元类型
Q_DECLARE_METATYPE(tf::DeviceInfo)

DevC1005::DevC1005()
{
}

DevC1005::~DevC1005()
{

}

///
/// @brief
///     注册i6000设备
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
bool DevC1005::RegisterDevice()
{
    // 构造查询条件
    ::tf::DeviceInfoQueryResp devResp;
    ::tf::DeviceInfoQueryCond devCond;
    devCond.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);

    // 查询设备信息
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devResp, devCond)
        || devResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || devResp.lstDeviceInfos.empty())
    {
        return false;
    }

    // 注册c1005设备
    for (const auto& stuDevInfo : devResp.lstDeviceInfos)
    {
        // 注册设备
        DYNAMIC_REGISTER_DEVICE(DevC1005, QString::fromStdString(stuDevInfo.name), QString::fromStdString(stuDevInfo.groupName));

        // 设置设备信息
        sm_mapDevNameInfo.insert(qMakePair(QString::fromStdString(stuDevInfo.name), QString::fromStdString(stuDevInfo.groupName)), QVariant::fromValue(stuDevInfo));
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
bool DevC1005::OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    bool bRet = false;
    switch (iDataCode)
    {
    case DATA_CODE_QC_DOC:
        bRet = OperateQcDocData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_APPLY:
        bRet = OperateQcApplyData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_DEFAULT_QC:
        bRet = OperateDefaultQcData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_BACKUP_RGNT_QC:
        bRet = OperateBackupRgntQcData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_ASSAY_NAME_LIST:
        bRet = OperateAssayNameListData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_RLT_ASSAY_LIST_SINGLE:
        bRet = OperateQcLjAssayNameListData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_RLT_ASSAY_LIST_TWIN:
        bRet = OperateQcTpAssayNameListData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_CONC_INFO:
        bRet = OperateQcConcInfoData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_RLT_INFO:
        bRet = OperateQcRltInfoData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_TARGET_VAL_SD:
        bRet = OperateQcTargetValSDData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_REASON_SOLUTION_SINGLE:
        bRet = OperateSingleQcReasonSolutionData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_REASON_SOLUTION_TWIN:
        bRet = OperateTwinQcReasonSolutionData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_CALCULATE_POINT_SINGLE:
        bRet = OperateSingleQcCalcPointData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_CALCULATE_POINT_TWIN:
        bRet = OperateTwinQcCalcPointData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_TWIN_RLT_INFO:
        bRet = OperateTwinQcRltInfoData(iDataOpCode, paramList, outBuffer);
        break;
    default:
        break;
    }

    return bRet;
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
bool DevC1005::ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode)
{
    return true;
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
int DevC1005::DeviceType()
{
    auto it = sm_mapDevNameInfo.find(qMakePair(DeviceName(), DevGroupName()));
    if (it == sm_mapDevNameInfo.end())
    {
        return -1;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return int(stuDevInfo.deviceType);
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
int DevC1005::DeviceClass()
{
    auto it = sm_mapDevNameInfo.find(qMakePair(DeviceName(), DevGroupName()));
    if (it == sm_mapDevNameInfo.end())
    {
        return -1;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return int(stuDevInfo.deviceClassify);
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
QString DevC1005::DeviceSn()
{
    auto it = sm_mapDevNameInfo.find(qMakePair(DeviceName(), DevGroupName()));
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
int DevC1005::ModuleCount()
{
    auto it = sm_mapDevNameInfo.find(qMakePair(DeviceName(), DevGroupName()));
    if (it == sm_mapDevNameInfo.end())
    {
        return 0;
    }

    tf::DeviceInfo stuDevInfo = it.value().value<tf::DeviceInfo>();
    return stuDevInfo.moduleCount;
}

///
/// @bref
///		试剂槽位数
///
/// @par History:
/// @li 8276/huchunli, 2022年12月28日，新建函数
///
int DevC1005::ReagentSlotNumber()
{
    return ::ch::c1005::tf::g_c1005_constants.REAGENT_DISK_SLOT;
}

///
/// @brief
///     获取质控原因
///
/// @param[in]  enQcType    质控类型
/// @param[in]  stuRgntGrp  试剂瓶组
/// @param[in]  stuQcDoc    质控文档信息
///
/// @return 质控原因
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月21日，新建函数
///
static boost::optional<ch::tf::QcApply> GetQcReason(ch::tf::QcType::type enQcType, const ch::tf::ReagentGroup& stuRgntGrp, const tf::QcDoc& stuQcDoc)
{
    ::ch::tf::QcApplyQueryResp _return;
    ::ch::tf::QcApplyQueryCond qcapcd;
    qcapcd.__set_qcDocId(stuQcDoc.id);
    qcapcd.__set_deviceSN(stuRgntGrp.deviceSN);
    qcapcd.__set_assayCode(stuRgntGrp.assayCode);
    qcapcd.__set_qcType(enQcType);

    // 备用时需要设置试剂的批号和瓶号
    if (enQcType == ch::tf::QcType::type::QC_TYPE_BACKUP)
    {
        // 默认置空
        qcapcd.__set_lot("");
        qcapcd.__set_sn("");

        // 必须加上试剂品的批号瓶号等详细信息
        if (!stuRgntGrp.reagentKeyInfos.empty())
        {
            auto keyInfo = stuRgntGrp.reagentKeyInfos[0];
            qcapcd.__set_lot(keyInfo.lot);
            qcapcd.__set_sn(keyInfo.sn);
        }
    }

    // 查询质控申请信息
    ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || _return.lstQcApplys.empty())
    {
        return boost::none;
    }

    return _return.lstQcApplys[0];
}

///
/// @brief
///     获取质控原因
///
/// @param[in]  stuQcApplyItem  质控申请信息
///
/// @return 质控原因
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月21日，新建函数
///
static boost::optional<ch::tf::QcApply> GetQcReason(const QC_APPLY_INFO& stuQcApplyItem)
{
    // 构造查询条件
    ::ch::tf::QcApplyQueryResp _return;
    ::ch::tf::QcApplyQueryCond qcapcd;

    // 解析查询条件
    QString strQcDocID("");
    QStringList strIDList = stuQcApplyItem.strID.split("-");
    if (strIDList.size() > 1)
    {
        strQcDocID = strIDList[1];
    }

    // 获取项目信息
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQcApplyItem.strAssayName.toStdString());
    if (Q_NULLPTR == spAssayInfo)
    {
        return boost::none;
    }

    // 获取具体设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName);
    if (spIDev == Q_NULLPTR)
    {
        return boost::none;
    }

    // 先查在用
    qcapcd.__set_qcDocId(strQcDocID.toLongLong());
    qcapcd.__set_deviceSN(spIDev->DeviceSn().toStdString());
    qcapcd.__set_assayCode(spAssayInfo->assayCode);
    qcapcd.__set_qcType(ch::tf::QcType::type::QC_TYPE_ONUSE);

    // 查询质控申请信息
    ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return boost::none;
    }

    // 如果非空，则返回
    if (!_return.lstQcApplys.empty())
    {
        return _return.lstQcApplys[0];
    }

    // 再查默认
    qcapcd.__set_qcType(ch::tf::QcType::type::QC_TYPE_DEFAULT);

    // 查询质控申请信息
    ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return boost::none;
    }

    // 如果非空，则返回
    if (!_return.lstQcApplys.empty())
    {
        _return.lstQcApplys[0].qcReason = ::tf::QcReason::QC_REASON_DEFAULT;
        return _return.lstQcApplys[0];
    }

    return boost::none;
}

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
bool DevC1005::OperateQcDocData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    auto IsQcDocContainsChAssaycode = [&](const tf::QcDoc& stuQcDoc)
    {
        for (const auto& stuQcRgnt : stuQcDoc.compositions)
        {
            if (stuQcRgnt.assayCode <= tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
            {
                return true;
            }
        }

        return false;
    };

    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 查询质控组合信息
//         ::tf::QcDocQueryResp qcDocResp;
//         if (!DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocResp) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
//         {
//             return false;
//         }

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 结果放入输出缓冲区
        for (auto& spQcDoc : allQcDocs)
        {
            // 参数检查
            if (spQcDoc == Q_NULLPTR)
            {
                continue;
            }
            auto& qcDoc = *spQcDoc;

            // （同一个质控品既可以质控ISE,又质控比色，bug11036）
            if (!IsQcDocContainsChAssaycode(qcDoc))
            {
                continue;
            }

            outBuffer.push_back(QString::fromStdString(qcDoc.name));
        }
        return true;
    }

    return false;
}

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
bool DevC1005::OperateQcApplyData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    auto IsQcDocMatchedRgnt = [](const tf::QcDoc& stuQcDoc, int iAssayCode)
    {
        for (const auto& stuQcRgnt : stuQcDoc.compositions)
        {
            if (stuQcRgnt.assayCode == iAssayCode)
            {
                return true;
            }
        }

        return false;
    };

    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QString strQcName = boost::any_cast<QString>(paramList.front());

        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<QPair<int, long long>, QString> mapQcReason;
        QMap<QPair<int, long long>, QPair<QString, QString>> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 没选择则跳过
            if (!stuQcApp.select)
            {
                continue;
            }

            // 判断质控原因
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_BACKUP)
            {
                // 如果是备用瓶质控则跳过
                continue;
            }

            // 如果对应的质控ID映射还未建立，则插入一个
            auto itId = mapQcApplyId.find(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId));
            if (itId == mapQcApplyId.end())
            {
                itId = mapQcApplyId.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), QPair<QString, QString>());
            }

            // 如果是在用类型，直接插入质控原因
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_ONUSE)
            {
                mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(stuQcApp.qcReason));
                itId.value().first = QString::number(stuQcApp.id);
                continue;
            }

            // 如果是默认质控，没有手工勾选就为默认质控
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_DEFAULT)
            {
                itId.value().second = QString::number(stuQcApp.id);
                if (mapQcReason.find(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId)) == mapQcReason.end())
                {
                    mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(tf::QcReason::QC_REASON_DEFAULT));
                }
                continue;
            }
        }

        // 查询质控组合信息
//         ::tf::QcDocQueryResp qcDocResp;
//         if (!DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocResp) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
//         {
//             return false;
//         }

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 从缓存中获取试剂信息
        std::map<int, ChReagentInfo> mapRgntInfo = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(DeviceSn().toStdString());

        // 按项目编号排序
        QSet<int> setAssayCode;
        for (auto it = mapRgntInfo.begin(); it != mapRgntInfo.end(); it++)
        {
            for (const auto& stuRgntGrp : it->second.reagentInfos)
            {
                setAssayCode.insert(stuRgntGrp.assayCode);
            }
        }

        // 获取设备类别
        tf::AssayClassify::type stuTfDevClass = tf::AssayClassify::type(DeviceClass());

        // 先计算返回数据数
//         int iOutBufferSize = 0;
//         for (const auto& spQcDoc : allQcDocs)
//         {
//             // 参数检查
//             if (spQcDoc == Q_NULLPTR)
//             {
//                 continue;
//             }
//             auto& stuQcDoc = *spQcDoc;
// 
//             // 参数不匹配则跳过
//             if (!strQcName.isEmpty() && (strQcName != QString::fromStdString(stuQcDoc.name)))
//             {
//                 continue;
//             }
// 
//             for (const auto& stuQcRgnt : stuQcDoc.compositions)
//             {
//                 // 获取项目信息
//                 std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQcRgnt.assayCode);
//                 if (Q_NULLPTR == spAssayInfo)
//                 {
//                     continue;
//                 }
// 
//                 // 数据缓存递增
//                 ++iOutBufferSize;
//             }
//         }

        // 组合质控申请信息
        QString strManApplyID("");
        QString strDefApplyID("");
        QString strQcReason("");
        QPair<int, long long> pairAssayDoc;
        QC_APPLY_INFO stuQcApplyInfo;
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1;//未使用，固定为1
        stuQcApplyInfo.strDevGrpName = DevGroupName();
        stuQcApplyInfo.strModuleName = DevGroupName().isEmpty() ? DeviceName() : (DevGroupName() + DeviceName());

        // 组合质控申请信息
        for (const auto& iAssayCode : setAssayCode)
        {
            // 获取项目信息
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(iAssayCode);
            if (Q_NULLPTR == spAssayInfo)
            {
                continue;
            }

            // 赋值项目信息
            if (!AdapterDataBuffer::GetInstance()->AssignAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo))
            {
                // 试剂相关信息
                stuQcApplyInfo.strAssayName = QString::fromStdString(spAssayInfo->assayName);
                stuQcApplyInfo.strAssayCode = QString::number(spAssayInfo->assayCode);
                AdapterDataBuffer::GetInstance()->MapAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo);
            }

            for (const auto& spQcDoc : allQcDocs)
            {
                // 参数检查
                if (spQcDoc == Q_NULLPTR)
                {
                    continue;
                }
                auto& stuQcDoc = *spQcDoc;

                // 赋值质控信息
                if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo))
                {
                    // 质控文档相关信息
                    stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc.sn);
                    stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc.name);
                    stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc.shortName);
                    stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc.sampleSourceType));
                    stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc.level);
                    stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc.lot);
                    stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc.id);
                    AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo);
                }

                // 参数不匹配则跳过
                if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
                {
                    continue;
                }

                // 质控品和试剂不匹配则跳过
                if (!IsQcDocMatchedRgnt(stuQcDoc, iAssayCode))
                {
                    continue;
                }

                // 获取质控状态(质控原因)
                strQcReason.clear();
                pairAssayDoc.first = iAssayCode;
                pairAssayDoc.second = stuQcDoc.id;
                auto itQcReason = mapQcReason.find(pairAssayDoc);
                if (itQcReason != mapQcReason.end())
                {
                    strQcReason = itQcReason.value();
                }

                // 构造质控申请ID
                pairAssayDoc.first = iAssayCode;
                pairAssayDoc.second = stuQcDoc.id;
                strManApplyID.clear();
                strDefApplyID.clear();
                auto itQcAppId = mapQcApplyId.find(pairAssayDoc);
                if (itQcAppId != mapQcApplyId.end())
                {
                    strManApplyID = itQcAppId.value().first;
                    strDefApplyID = itQcAppId.value().second;
                }

                // 构造是否选择
                bool bSel = !strManApplyID.isEmpty();

                // 构造质控申请信息
                stuQcApplyInfo.strQcReason = strQcReason;
                stuQcApplyInfo.bSelected = bSel;
                stuQcApplyInfo.strID = stuQcApplyInfo.strAssayCode + "-" + stuQcApplyInfo.strQcDocID;
                stuQcApplyInfo.strManApplyID = strManApplyID;
                stuQcApplyInfo.strDefApplyID = strDefApplyID;

                // 构造输出缓存
                //if (itOutBuffer == outBuffer.end())
                {
                    outBuffer.push_back(stuQcApplyInfo);
                    continue;
                }
            }
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 获取质控ID
        QStringList strIDList = stuApplyInfo.strID.split("-");
        long long lQcDocId = -1;
        if (strIDList.size() >= 2)
        {
            lQcDocId = strIDList[1].toLongLong();
        }

        // 查询有没有对应的记录，有就修改，没有则添加
        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());
        qcapcd.__set_assayCode(spAssayInfo->assayCode);
        qcapcd.__set_qcType(ch::tf::QcType::QC_TYPE_ONUSE);
        if (lQcDocId > 0)
        {
            qcapcd.__set_qcDocId(lQcDocId);
        }

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 如果结果不为空，则修改
        long long lQcAppId = -1;
        if (!_return.lstQcApplys.empty())
        {
            ch::tf::QcApply qa;
            qa.__set_id(_return.lstQcApplys[0].id);
            qa.__set_select(true);
            qa.__set_qcReason(tf::QcReason::QC_REASON_MANNUAL);
            if (!::ch::LogicControlProxy::ModifyQcApply(qa))
            {
                return false;
            }
            lQcAppId = _return.lstQcApplys[0].id;
        }
        else
        {
            // 添加质控申请信息
            ch::tf::QcApply qa;
            ::tf::ResultLong _return;

            // 构造质控申请信息
            qa.__set_assayCode(spAssayInfo->assayCode);
            qa.__set_deviceSN(DeviceSn().toStdString());
            qa.__set_lot(stuApplyInfo.strRgntLot.toStdString());
            qa.__set_sn(stuApplyInfo.strRgntNo.toStdString());
            qa.__set_qcReason(::tf::QcReason::QC_REASON_MANNUAL);
            qa.__set_qcType(ch::tf::QcType::type::QC_TYPE_ONUSE);
            qa.__set_select(true);
            if (lQcDocId > 0)
            {
                qa.__set_qcDocId(lQcDocId);
            }

            // 执行添加
            ::ch::LogicControlProxy::AddQcApply(_return, qa);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                return false;
            }
            lQcAppId = _return.value;
        }

        // 构造输出缓冲区
        stuApplyInfo.strManApplyID = QString::number(lQcAppId);
        stuApplyInfo.bSelected = true;
        stuApplyInfo.strQcReason = ConvertTfEnumToQString(::tf::QcReason::QC_REASON_MANNUAL);
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    // 质控申请取消选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_DELETE))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

		ch::tf::QcApplyQueryCond queryCond;
		queryCond.__set_id(stuApplyInfo.strManApplyID.toLongLong());
		if (!::ch::LogicControlProxy::DeleteQcApply(queryCond))
		{
			return false;
		}

        // 构造输出缓冲区
        stuApplyInfo.bSelected = false;
        stuApplyInfo.strManApplyID.clear();
        stuApplyInfo.strQcReason = stuApplyInfo.strDefApplyID.isEmpty() ? "" : ConvertTfEnumToQString(::tf::QcReason::QC_REASON_DEFAULT);
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateDefaultQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QString strQcName = boost::any_cast<QString>(paramList.front());

        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<QPair<int, long long>, QString> mapQcReason;
        QMap<QPair<int, long long>, QPair<QString, QString>> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 没选择则跳过
            if (!stuQcApp.select)
            {
                continue;
            }

            // 判断质控原因
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_BACKUP)
            {
                // 如果是备用瓶质控则跳过
                continue;
            }

            // 如果对应的质控ID映射还未建立，则插入一个
            auto itId = mapQcApplyId.find(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId));
            if (itId == mapQcApplyId.end())
            {
                itId = mapQcApplyId.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), QPair<QString, QString>());
            }

            // 如果是在用类型，直接插入质控原因
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_ONUSE)
            {
                mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(stuQcApp.qcReason));
                itId.value().first = QString::number(stuQcApp.id);
                continue;
            }

            // 如果是默认质控，没有手工勾选就为默认质控
            if (stuQcApp.qcType == ch::tf::QcType::QC_TYPE_DEFAULT)
            {
                itId.value().second = QString::number(stuQcApp.id);
                if (mapQcReason.find(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId)) == mapQcReason.end())
                {
                    mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(tf::QcReason::QC_REASON_DEFAULT));
                }
                continue;
            }
        }

        // 查询质控组合信息
//         ::tf::QcDocQueryResp qcDocResp;
//         if (!DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocResp) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
//         {
//             return false;
//         }

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 获取设备类别
        tf::AssayClassify::type stuTfDevClass = tf::AssayClassify::type(DeviceClass());

        // 先计算返回数据数
//         int iOutBufferSize = 0;
//         for (const auto& spQcDoc : allQcDocs)
//         {
//             // 参数检查
//             if (spQcDoc == Q_NULLPTR)
//             {
//                 continue;
//             }
//             auto stuQcDoc = *spQcDoc;
// 
//             // 参数不匹配则跳过
//             if (!strQcName.isEmpty() && (strQcName != QString::fromStdString(stuQcDoc.name)))
//             {
//                 continue;
//             }
// 
//             for (const auto& stuQcAssay : stuQcDoc.compositions)
//             {
//                 // 获取项目信息
//                 std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQcAssay.assayCode);
//                 if (Q_NULLPTR == spAssayInfo)
//                 {
//                     continue;
//                 }
// 
//                 // 设备类型不匹配则跳过
//                 if (spAssayInfo->assayClassify != stuTfDevClass)
//                 {
//                     continue;
//                 }
// 
//                 // 数据递增
//                 ++iOutBufferSize;
//             }
//         }

        // 组合质控申请信息
        QPair<int, long long> pairAssayDoc;
        QString strManApplyID("");
        QString strDefApplyID("");
        QString strQcReason("");
        QC_APPLY_INFO stuQcApplyInfo;
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1;//未使用，固定为1
        stuQcApplyInfo.strDevGrpName = DevGroupName();
        stuQcApplyInfo.strModuleName = DevGroupName().isEmpty() ? DeviceName() : (DevGroupName() + DeviceName());
        for (const auto& spQcDoc : allQcDocs)
        {
            // 参数检查
            if (spQcDoc == Q_NULLPTR)
            {
                continue;
            }
            auto stuQcDoc = *spQcDoc;

            // 赋值质控信息
            if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo))
            {
                // 质控文档相关信息
                stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc.sn);
                stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc.name);
                stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc.shortName);
                stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc.sampleSourceType));
                stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc.level);
                stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc.lot);
                stuQcApplyInfo.strQcExpDate = QString::fromStdString(stuQcDoc.expireTime);
                stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc.id);
                AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo);
            }

            // 参数不匹配则跳过
            if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
            {
                continue;
            }

            for (const auto& stuQcAssay : stuQcDoc.compositions)
            {
                // 获取项目信息
                std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQcAssay.assayCode);
                if (Q_NULLPTR == spAssayInfo)
                {
                    continue;
                }

                // 设备类型不匹配则跳过
                if (spAssayInfo->assayClassify != stuTfDevClass)
                {
                    continue;
                }

                // 赋值项目信息
                if (!AdapterDataBuffer::GetInstance()->AssignAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo))
                {
                    // 试剂相关信息
                    stuQcApplyInfo.strAssayName = QString::fromStdString(spAssayInfo->assayName);
                    stuQcApplyInfo.strAssayCode = QString::number(spAssayInfo->assayCode);
                    AdapterDataBuffer::GetInstance()->MapAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo);
                }

                // 获取质控状态(质控原因)
                pairAssayDoc.first = spAssayInfo->assayCode;
                pairAssayDoc.second = stuQcDoc.id;
                strQcReason.clear();
                auto itQcReason = mapQcReason.find(pairAssayDoc);
                if (itQcReason != mapQcReason.end())
                {
                    strQcReason = itQcReason.value();
                }

                // 构造质控申请ID
                strManApplyID.clear();
                strDefApplyID.clear();
                auto itQcAppId = mapQcApplyId.find(pairAssayDoc);
                if (itQcAppId != mapQcApplyId.end())
                {
                    strManApplyID = itQcAppId.value().first;
                    strDefApplyID = itQcAppId.value().second;
                }

                // 构造是否选择
                bool bSel = !strDefApplyID.isEmpty();

                // 构造质控申请信息
                stuQcApplyInfo.bSelected = bSel;
                stuQcApplyInfo.strID = stuQcApplyInfo.strAssayCode + "-" + stuQcApplyInfo.strQcDocID;
                stuQcApplyInfo.strQcReason = strQcReason;
                stuQcApplyInfo.strManApplyID = strManApplyID;
                stuQcApplyInfo.strDefApplyID = strDefApplyID;

                // 构造输出缓存
                //if (itOutBuffer == outBuffer.end())
                {
                    outBuffer.push_back(stuQcApplyInfo);
                    continue;
                }
            }
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 获取质控ID
        QStringList strIDList = stuApplyInfo.strID.split("-");
        long long lQcDocId = -1;
        if (strIDList.size() >= 2)
        {
            lQcDocId = strIDList[1].toLongLong();
        }

        // 查询有没有对应的记录，有就修改，没有则添加
        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());
        qcapcd.__set_assayCode(spAssayInfo->assayCode);
        qcapcd.__set_qcType(ch::tf::QcType::QC_TYPE_DEFAULT);
        if (lQcDocId > 0)
        {
            qcapcd.__set_qcDocId(lQcDocId);
        }

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 如果结果不为空，则修改
        long long lQcAppId = -1;
        if (!_return.lstQcApplys.empty())
        {
            ch::tf::QcApply qa;
            qa.__set_id(_return.lstQcApplys[0].id);
            qa.__set_select(true);
            if (!::ch::LogicControlProxy::ModifyQcApply(qa))
            {
                return false;
            }
            lQcAppId = _return.lstQcApplys[0].id;
        }
        else
        {
            // 添加质控申请信息
            ch::tf::QcApply qa;
            ::tf::ResultLong _return;

            // 构造质控申请信息
            qa.__set_assayCode(spAssayInfo->assayCode);
            qa.__set_deviceSN(DeviceSn().toStdString());
            qa.__set_lot(stuApplyInfo.strRgntLot.toStdString());
            qa.__set_sn(stuApplyInfo.strRgntNo.toStdString());
            qa.__set_qcReason(::tf::QcReason::QC_REASON_DEFAULT);
            qa.__set_qcType(ch::tf::QcType::type::QC_TYPE_DEFAULT);
            qa.__set_select(true);
            if (lQcDocId > 0)
            {
                qa.__set_qcDocId(lQcDocId);
            }

            // 执行添加
            ::ch::LogicControlProxy::AddQcApply(_return, qa);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                return false;
            }
            lQcAppId = _return.value;
        }

        // 构造输出缓冲区
        stuApplyInfo.strDefApplyID = QString::number(lQcAppId);
        stuApplyInfo.bSelected = true;
        if (stuApplyInfo.strManApplyID.isEmpty())
        {
            stuApplyInfo.strQcReason = ConvertTfEnumToQString(::tf::QcReason::QC_REASON_DEFAULT);
        }
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    // 质控申请取消选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_DELETE))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 执行删除
        ch::tf::QcApplyQueryCond queryCond;
        queryCond.__set_id(stuApplyInfo.strDefApplyID.toLongLong());
        if (!::ch::LogicControlProxy::DeleteQcApply(queryCond))
        {
            return false;
        }

        // 构造输出缓冲区
        stuApplyInfo.bSelected = false;
        stuApplyInfo.strDefApplyID.clear();
        if (stuApplyInfo.strManApplyID.isEmpty())
        {
            stuApplyInfo.strQcReason.clear();
        }
        outBuffer.push_back(stuApplyInfo);
        return true;
    }

    return true;
}

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
bool DevC1005::OperateBackupRgntQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    auto IsQcDocMatchedRgnt = [](const tf::QcDoc& stuQcDoc, int iAssayCode)
    {
        for (const auto& stuQcRgnt : stuQcDoc.compositions)
        {
            if (stuQcRgnt.assayCode == iAssayCode)
            {
                return true;
            }
        }

        return false;
    };

    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QString strQcName = boost::any_cast<QString>(paramList.front());

        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());
        qcapcd.__set_qcType(ch::tf::QcType::QC_TYPE_BACKUP);

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<std::tuple<int, long long, QString, QString>, QString> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 没选择则跳过
            if (!stuQcApp.select)
            {
                continue;
            }

            // 判断质控原因
            if (stuQcApp.qcType != ch::tf::QcType::QC_TYPE_BACKUP)
            {
                // 如果不是备用瓶质控则跳过
                continue;
            }

            // 更新映射
            QString strRgntLot = QString::fromStdString(stuQcApp.lot);
            QString strRgntSn = QString::fromStdString(stuQcApp.sn);
            mapQcApplyId.insert(std::make_tuple(stuQcApp.assayCode, stuQcApp.qcDocId, strRgntLot, strRgntSn), QString::number(stuQcApp.id));
        }

        // 查询质控组合信息
//         ::tf::QcDocQueryResp qcDocResp;
//         if (!DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocResp) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
//         {
//             return false;
//         }

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 从缓存中获取试剂信息
        std::map<int, ChReagentInfo> mapRgntInfo = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(DeviceSn().toStdString());

        // 按项目编号排序
        std::vector<ch::tf::ReagentGroup> vecRgntGrp;
        for (auto it = mapRgntInfo.begin(); it != mapRgntInfo.end(); it++)
        {
            for (const auto& stuRgntGrp : it->second.reagentInfos)
            {
                // 不为备用则跳过
                if (stuRgntGrp.usageStatus != ::tf::UsageStatus::USAGE_STATUS_BACKUP)
                {
                    continue;
                }

                // 记录数据
                vecRgntGrp.push_back(stuRgntGrp);
            }
        }

        // 先计算数据条数
//         int iOutBufferSize = 0;
//         for (const auto& stuRgntInfo : qryResp.lstReagentGroup)
//         {
//             for (const auto& spQcDoc : allQcDocs)
//             {
//                 // 参数判断
//                 if (spQcDoc == Q_NULLPTR)
//                 {
//                     continue;
//                 }
//                 auto stuQcDoc = *spQcDoc;
// 
//                 // 参数不匹配则跳过
//                 if (!strQcName.isEmpty() && (strQcName != QString::fromStdString(stuQcDoc.name)))
//                 {
//                     continue;
//                 }
// 
//                 // 质控品和试剂不匹配则跳过
//                 if (!IsQcDocMatchedRgnt(stuQcDoc, stuRgntInfo.assayCode))
//                 {
//                     continue;
//                 }
// 
//                 // 获取项目信息
//                 std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntInfo.assayCode);
//                 if (Q_NULLPTR == spAssayInfo)
//                 {
//                     continue;
//                 }
// 
//                 // 条数自增
//                 ++iOutBufferSize;
//             }
//         }

        // 组合质控申请信息
        QC_APPLY_INFO stuQcApplyInfo;
        QString strQcAppId("");
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1;//未使用，固定为1
        stuQcApplyInfo.strDevGrpName = DevGroupName();
        stuQcApplyInfo.strModuleName = DevGroupName().isEmpty() ? DeviceName() : (DevGroupName() + DeviceName());
        QPair<int, long long> pairAssayDoc;
        for (const auto& stuRgntInfo : vecRgntGrp)
        {
            for (const auto& spQcDoc : allQcDocs)
            {
                // 参数判断
                if (spQcDoc == Q_NULLPTR)
                {
                    continue;
                }
                auto stuQcDoc = *spQcDoc;

                // 赋值质控信息
                if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo))
                {
                    // 质控文档相关信息
                    stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc.sn);
                    stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc.name);
                    stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc.shortName);
                    stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc.sampleSourceType));
                    stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc.level);
                    stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc.lot);
                    stuQcApplyInfo.strQcExpDate = QString::fromStdString(stuQcDoc.expireTime);
                    stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc.id);
                    AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc.id, stuQcApplyInfo);
                }

                // 参数不匹配则跳过
                if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
                {
                    continue;
                }

                // 质控品和试剂不匹配则跳过
                if (!IsQcDocMatchedRgnt(stuQcDoc, stuRgntInfo.assayCode))
                {
                    continue;
                }

                // 获取项目信息
                std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntInfo.assayCode);
                if (Q_NULLPTR == spAssayInfo)
                {
                    continue;
                }

                // 赋值项目信息
                if (!AdapterDataBuffer::GetInstance()->AssignAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo))
                {
                    // 试剂相关信息
                    stuQcApplyInfo.strAssayName = QString::fromStdString(spAssayInfo->assayName);
                    stuQcApplyInfo.strAssayCode = QString::number(spAssayInfo->assayCode);
                    AdapterDataBuffer::GetInstance()->MapAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo);
                }

                // 获取试剂关键信息
                auto rgntKeyInfo = stuRgntInfo.reagentKeyInfos[0];

                // 赋值试剂信息
                if (!AdapterDataBuffer::GetInstance()->AssignRgntInfoToQcApply(DeviceSn(), stuRgntInfo.posInfo.pos, stuQcApplyInfo))
                {
                    // 试剂相关信息
                    stuQcApplyInfo.iBackupRgntPos = stuRgntInfo.posInfo.pos;
                    stuQcApplyInfo.strPos = stuQcApplyInfo.strModuleName + "-" + QString::number(stuRgntInfo.posInfo.pos);     // 待完善
                    stuQcApplyInfo.strRgntUseStatus = ConvertTfEnumToQString(stuRgntInfo.usageStatus) + QString::number(stuRgntInfo.backupTotal) + "-" + QString::number(stuRgntInfo.backupNum);
                    stuQcApplyInfo.strRgntNo = QString::fromStdString(rgntKeyInfo.sn);
                    stuQcApplyInfo.strRgntLot = QString::fromStdString(rgntKeyInfo.lot);
                    AdapterDataBuffer::GetInstance()->MapAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo);
                }

                // 获取质控申请ID
                strQcAppId.clear();
                auto itQcAppId = mapQcApplyId.find(std::make_tuple(spAssayInfo->assayCode, stuQcDoc.id, stuQcApplyInfo.strRgntLot, stuQcApplyInfo.strRgntNo));
                if (itQcAppId != mapQcApplyId.end())
                {
                    strQcAppId = itQcAppId.value();
                }

                // 构造质控申请信息
                stuQcApplyInfo.bSelected = !strQcAppId.isEmpty();
                stuQcApplyInfo.strID = stuQcApplyInfo.strPos + "-" + stuQcApplyInfo.strQcDocID;
                stuQcApplyInfo.strManApplyID = strQcAppId;

                // 构造输出缓存
                //if (itOutBuffer == outBuffer.end())
                {
                    outBuffer.push_back(stuQcApplyInfo);
                    continue;
                }

                // 直接赋值
                //*itOutBuffer = stuQcApplyInfo;
                //++itOutBuffer;
            }
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 查询有没有对应的记录，有就修改，没有则添加
        // 查询并缓存所有质控申请信息
        ::ch::tf::QcApplyQueryResp _return;
        ::ch::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());
        qcapcd.__set_assayCode(spAssayInfo->assayCode);
        qcapcd.__set_qcType(ch::tf::QcType::QC_TYPE_BACKUP);
        qcapcd.__set_lot(stuApplyInfo.strRgntLot.toStdString());
        qcapcd.__set_sn(stuApplyInfo.strRgntNo.toStdString());
        qcapcd.__set_qcDocId(stuApplyInfo.strQcDocID.toLongLong());

        // 查询质控申请信息
        ::ch::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 如果结果不为空，则修改
        long long lQcAppId = -1;
        if (!_return.lstQcApplys.empty())
        {
            ch::tf::QcApply qa;
            qa.__set_id(_return.lstQcApplys[0].id);
            qa.__set_select(true);
            if (!::ch::LogicControlProxy::ModifyQcApply(qa))
            {
                return false;
            }
            lQcAppId = _return.lstQcApplys[0].id;
        }
        else
        {
            // 添加质控申请信息
            ch::tf::QcApply qa;
            ::tf::ResultLong _return;

            // 构造质控申请信息
            qa.__set_assayCode(spAssayInfo->assayCode);
            qa.__set_deviceSN(DeviceSn().toStdString());
            qa.__set_lot(stuApplyInfo.strRgntLot.toStdString());
            qa.__set_sn(stuApplyInfo.strRgntNo.toStdString());
            qa.__set_qcReason(::tf::QcReason::QC_REASON_MANNUAL);
            qa.__set_qcType(ch::tf::QcType::type::QC_TYPE_BACKUP);
            qa.__set_select(true);
            qa.__set_qcDocId(stuApplyInfo.strQcDocID.toLongLong());

            // 执行添加
            ::ch::LogicControlProxy::AddQcApply(_return, qa);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                return false;
            }
            lQcAppId = _return.value;
        }

        // 构造输出缓冲区
        stuApplyInfo.strManApplyID = QString::number(lQcAppId);
        stuApplyInfo.bSelected = true;
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    // 质控申请取消选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_DELETE))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 删除对应记录
        ch::tf::QcApplyQueryCond queryCond;
        queryCond.__set_id(stuApplyInfo.strManApplyID.toLongLong());
        if (!::ch::LogicControlProxy::DeleteQcApply(queryCond))
        {
            return false;
        }

        // 构造输出缓冲区
        stuApplyInfo.bSelected = false;
        stuApplyInfo.strManApplyID.clear();
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ChAssayIndexUniqueOrderCodeMaps mapAssayCodeInfo;
        if (!CommonInformationManager::GetInstance()->GetChAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayCodeInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayCodeInfo.begin(); it != mapAssayCodeInfo.end(); it++)
        {
            // 指针判空
            if (it->second == Q_NULLPTR)
            {
                continue;
            }

            // 过滤掉血清指数
            if (it->first >= ch::tf::g_ch_constants.MAX_REAGENT_CODE)
            {
                continue;
            }

			// 转换为通用项目
			std::shared_ptr<tf::GeneralAssayInfo> spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(it->first);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

            strAssayList.push_back(QString::fromStdString(spAssay->assayName));
        }

        // 压入列表
        for (const auto& outData : strAssayList)
        {
            outBuffer.push_back(outData);
        }
        return true;
    }

    return true;
}

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
bool DevC1005::OperateQcLjAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ChAssayIndexUniqueOrderCodeMaps mapAssayInfo;
        if (!CommonInformationManager::GetInstance()->GetChAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayInfo.begin(); it != mapAssayInfo.end(); it++)
        {
            // 指针判空
            if (it->second == Q_NULLPTR)
            {
                continue;
            }

            // 过滤掉血清指数
            if (it->first >= ch::tf::g_ch_constants.MAX_REAGENT_CODE)
            {
                continue;
            }

			// 转换为通用项目
			std::shared_ptr<tf::GeneralAssayInfo> spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(it->first);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

            strAssayList.push_back(QString::fromStdString(spAssay->assayName));
        }

        // 压入列表
        for (const auto& outData : strAssayList)
        {
            outBuffer.push_back(outData);
        }
        return true;
    }

    return true;
}

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
bool DevC1005::OperateQcTpAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ChAssayIndexUniqueOrderCodeMaps mapAssayInfo;
        if (!CommonInformationManager::GetInstance()->GetChAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayInfo.begin(); it != mapAssayInfo.end(); it++)
        {
            // 指针判空
            if (it->second == Q_NULLPTR)
            {
                continue;
            }

            // 过滤掉血清指数
            if (it->first >= ch::tf::g_ch_constants.MAX_REAGENT_CODE)
            {
                continue;
            }

            // 转换为通用项目
            std::shared_ptr<tf::GeneralAssayInfo> spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(it->first);
            if (spAssay == Q_NULLPTR)
            {
                continue;
            }

            // 没有联合质控规则则跳过
            if (spAssay->qcRules.XQCDocID <= 0 || spAssay->qcRules.YQCDocID <= 0)
            {
                continue;
            }

            strAssayList.push_back(QString::fromStdString(spAssay->assayName));
        }

        // 压入列表
        for (const auto& outData : strAssayList)
        {
            outBuffer.push_back(outData);
        }
        return true;
    }

    return true;
}

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
bool DevC1005::OperateQcConcInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 获取质控品某个项目的靶值和标准差和CV
    auto LoadQcTargetValSdCv = [](const tf::QcDoc& stuQcDoc, int iAssayCode, double& dTargetVal, double& dSD, double& dCV)
    {
        // 清空输出字符串
		dTargetVal = 0.0;
		dSD = 0.0;
		dCV = 0.0;

        // 遍历成分，找到对应项目
        for (const auto& composition : stuQcDoc.compositions)
        {
            if (composition.assayCode != iAssayCode)
            {
                continue;
            }

            // 获取靶值、SD、CV
            dTargetVal = composition.targetValue;
            dSD = composition.sd;
            dCV = (fabs(composition.targetValue) < PRECESION_VALUE) ? 100 : (composition.sd / composition.targetValue) * 100;

            break;
        }
    };

	// 获取计算靶值、标准差、CV值和计算数量
	auto GetCalculateResultInfo = [](const std::vector<ch::tf::QcResult>& qcResults, int& iQcCalcNum, double& dCalcTargetVal, double& dCalcSD, double& dCalcCV)
	{
		// 为空直接返回
		if (qcResults.empty())
		{
			return;
		}

		// 计算靶值
		double totalTargetValue = 0;
		std::vector<double> calcResultsConc;
		for (const auto& result : qcResults)
		{
			// 计算的点
			if (result.bCalcSelect)
			{
				totalTargetValue += result.dConc;
				calcResultsConc.push_back(result.dConc);
			}
		}

		// 没有勾选计算结果
		if (calcResultsConc.empty())
		{
			return;
		}

		// 计算数量
		iQcCalcNum = calcResultsConc.size();

		// 计算靶值
		dCalcTargetVal = totalTargetValue / iQcCalcNum;

		// 计算标准差
		double totalSD = 0;
		for (const auto& result : calcResultsConc)
		{
			totalSD += pow((result - dCalcTargetVal), 2);
		}
		dCalcSD = (iQcCalcNum == 1) ? sqrt(totalSD) : sqrt((totalSD / (iQcCalcNum - 1)));

		// 计算变异系数CV
		dCalcCV = (fabs(dCalcTargetVal) < PRECESION_VALUE) ? 100 : (dCalcSD / dCalcTargetVal) * 100;
	};

    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QC_CONC_INFO_QUERY_COND stuQryCond = boost::any_cast<QC_CONC_INFO_QUERY_COND>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQryCond.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 查询质控组合信息
        ::tf::QcDocQueryResp qcDocResp;
        if (!DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocResp) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 获取起止日期
        boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
            stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
        boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
            stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

        // 设置起止日期
        tf::TimeRangeCond trCond;
        trCond.__set_startTime(PosixTimeToTimeString(startDate));
        trCond.__set_endTime(PosixTimeToTimeString(endDate));

        // 遍历质控品文档
        for (const auto& stuDocInfo : qcDocResp.lstQcDocs)
        {
            // 查询质控品文档对应设备的计算靶值、计算SD
            ch::tf::QcCalculateResultQueryResp qcCalcResp;
            ch::tf::QcCalculateResultQueryCond qcCalcCond;

            // 构造查询条件
            qcCalcCond.__set_iAssayCode(spAssayInfo->assayCode);
            qcCalcCond.__set_qcDocId(stuDocInfo.id);
            qcCalcCond.__set_strDevSN(DeviceSn().toStdString());
            qcCalcCond.__set_ptQcTime(trCond);

            // 构造靶值、sd、cv
            double dTargetVal = 0.0;
            double dSD = 0.0;
            double dCV = 0.0;

            // 获取靶值、sd、cv
            LoadQcTargetValSdCv(stuDocInfo, spAssayInfo->assayCode, dTargetVal, dSD, dCV);

            // 构造输出数据
            QC_DOC_CONC_INFO stuQcConcInfo;
            stuQcConcInfo.strID = QString::number(stuDocInfo.id);
            stuQcConcInfo.strQcNo = QString::fromStdString(stuDocInfo.sn);
            stuQcConcInfo.strQcName = QString::fromStdString(stuDocInfo.name);
            stuQcConcInfo.strQcBriefName = QString::fromStdString(stuDocInfo.shortName);
            stuQcConcInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuDocInfo.sampleSourceType));
            stuQcConcInfo.strQcLevel = QString::number(stuDocInfo.level);
            stuQcConcInfo.strQcLot = QString::fromStdString(stuDocInfo.lot);
            stuQcConcInfo.dQcTargetVal = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, dTargetVal);
            stuQcConcInfo.dQcSD = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, dSD);
            stuQcConcInfo.dQcCV = dCV;

            // 查询质控结果
            ch::tf::QcResultQueryResp qcRltResp;
            ch::tf::QcResultQueryCond qcRltCond;

            // 构造查询条件
            qcRltCond.__set_iAssayCode(spAssayInfo->assayCode);
            qcRltCond.__set_qcDocId(stuDocInfo.id);
            qcRltCond.__set_strDevSN(DeviceSn().toStdString());
            qcRltCond.__set_ptQcTime(trCond);

            // 执行查询
            ch::LogicControlProxy::QueryQcResult(qcRltResp, qcRltCond);
            if (qcRltResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                return false;
            }

			// 构造计算相关信息（计算数量，靶值，SD、CV）
			int iQcCalcNum = 0;
			double dCalcTargetVal = 0.0;
			double dCalcSD = 0.0;
			double dCalcCV = 0.0;
			GetCalculateResultInfo(qcRltResp.lstQcResults, iQcCalcNum, dCalcTargetVal, dCalcSD, dCalcCV);

			stuQcConcInfo.dQcCalcTargetVal = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, dCalcTargetVal);
			stuQcConcInfo.dQcCalcSD = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, dCalcSD);
			stuQcConcInfo.dQcCalcCV = dCalcCV;
			stuQcConcInfo.strQcRltCount = QString::number(iQcCalcNum);

            // 如果没有质控结果并且质控品不包含此项目，则跳过
            auto itFindCode = std::find_if(stuDocInfo.compositions.begin(), stuDocInfo.compositions.end(), [spAssayInfo](const tf::QcComposition& qcCps)
            {
                return (qcCps.assayCode == spAssayInfo->assayCode);
            });
            if (qcRltResp.lstQcResults.empty() && (itFindCode == stuDocInfo.compositions.end()))
            {
                continue;
            }

            // 压入列表
            outBuffer.push_back(stuQcConcInfo);
        }

        return true;
    }

    return true;
}

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
bool DevC1005::OperateQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QC_RESULT_QUERY_COND stuQryCond = boost::any_cast<QC_RESULT_QUERY_COND>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQryCond.strAssayName.toStdString());

        // 查询质控结果
        ch::tf::QcResultQueryResp qcRltResp;
        ch::tf::QcResultQueryCond qcRltCond;

        // 构造查询条件
        boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
            stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
        boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
            stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

        // 设置起止日期
        tf::TimeRangeCond trCond;
        trCond.__set_startTime(PosixTimeToTimeString(startDate));
        trCond.__set_endTime(PosixTimeToTimeString(endDate));

        // 构造查询条件
        if (Q_NULLPTR != spAssayInfo)
        {
            qcRltCond.__set_iAssayCode(spAssayInfo->assayCode);
        }

        if (!stuQryCond.strQcDocID.isEmpty())
        {
            qcRltCond.__set_qcDocId(stuQryCond.strQcDocID.toLongLong());
        }
        qcRltCond.__set_strDevSN(DeviceSn().toStdString());
        qcRltCond.__set_ptQcTime(trCond);

        // 执行查询
        ch::LogicControlProxy::QueryQcResult(qcRltResp, qcRltCond);
        if (qcRltResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 构造输出缓冲区
        for (auto& stuData : qcRltResp.lstQcResults)
        {
            // 构造失控规则
            QStringList strOutCtrlRuleList;
            for (auto& enOutCtrlRule : stuData.listTrigRules)
            {
                strOutCtrlRuleList.push_back(ConvertTfEnumToQString(enOutCtrlRule.ruleType));
            }

            // 获取项目信息
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuData.iAssayCode);
            if (Q_NULLPTR == spAssayInfo)
            {
                continue;
            }

            QC_RESULT_INFO stuQcRltInfo;
            stuQcRltInfo.bCalculated = stuData.bCalcSelect;
            stuQcRltInfo.strID = QString::number(stuData.id);
            stuQcRltInfo.strQcDocID = QString::number(stuData.qcDocId);
            stuQcRltInfo.strAssayName = QString::fromStdString(spAssayInfo->assayName);
            stuQcRltInfo.strRltDetailID = QString::number(stuData.assayTestResultId);
            stuQcRltInfo.strQcTime = QString::fromStdString(stuData.ptQcTime);
            stuQcRltInfo.dQcResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuData.dConc);
            stuQcRltInfo.enQcState = QC_STATE(stuData.emStat);
            stuQcRltInfo.strOutCtrlRule = strOutCtrlRuleList.join(",");
            stuQcRltInfo.dQcTargetVal = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuData.dTargetValue);
            stuQcRltInfo.dQcSD = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuData.dSTD);
            stuQcRltInfo.strOutCtrlReason = QString::fromStdString(stuData.qcUserEditInfo.outRuleReason);
            stuQcRltInfo.strSolution = QString::fromStdString(stuData.qcUserEditInfo.outRuleHandleScheme);
            stuQcRltInfo.strOperator = QString::fromStdString(stuData.qcUserEditInfo.userName);

            // 压入列表
            outBuffer.push_back(stuQcRltInfo);
        }

        return true;
    }

    return true;
}

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
bool DevC1005::OperateQcTargetValSDData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_MODIFY))
    {
        // 解析参数
        if (paramList.size() < 2)
        {
            return false;
        }
        QC_DOC_CONC_INFO stuQcConcInfo = boost::any_cast<QC_DOC_CONC_INFO>(paramList.front());
        QString strAssayName = boost::any_cast<QString>(paramList.at(1));

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 查询对应的质控信息
        ::tf::QcDocQueryResp qcDocResp;
        ::tf::QcDocQueryCond qcDocQryCond;
        qcDocQryCond.__set_ids({ stuQcConcInfo.strID.toLongLong() });
        if (!DcsControlProxy::GetInstance()->QueryQcDoc(qcDocResp, qcDocQryCond) || qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 结果为空则返回失败
        if (qcDocResp.lstQcDocs.empty())
        {
            return false;
        }

        // 构造新的质控品信息
        tf::QcDoc& qcDoc = qcDocResp.lstQcDocs[0];

        // 修改对应的靶值和SD
        bool bIsAssayValid = false;
        for (auto& composition : qcDoc.compositions)
        {
            // 项目不匹配则跳过
            if (composition.assayCode != spAssayInfo->assayCode)
            {
                continue;
            }

            // 更新靶值SD
            bIsAssayValid = true;
            composition.targetValue = CommonInformationManager::GetInstance()->ToMainUnitValue(spAssayInfo->assayCode, stuQcConcInfo.dQcCalcTargetVal);
            composition.sd = CommonInformationManager::GetInstance()->ToMainUnitValue(spAssayInfo->assayCode, stuQcConcInfo.dQcCalcSD);
            break;
        }

        // 无效项目
        if (!bIsAssayValid)
        {
            return false;
        }

        // 执行修改
        tf::ResultLong ret;
		if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, qcDoc))
		{
			// 接口调用失败
			outBuffer.push_back(ret);
			return false;
		}
		
    }

    return true;
}

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
bool DevC1005::OperateSingleQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_MODIFY))
    {
        // 解析参数
        if (paramList.size() < 3)
        {
            return false;
        }
        QC_RESULT_INFO stuQcRltInfo = boost::any_cast<QC_RESULT_INFO>(paramList.front());
        QString strReason = boost::any_cast<QString>(paramList.at(1));
        QString strSolution = boost::any_cast<QString>(paramList.at(2));

        // 构造用户编辑信息
        tf::QcUserEditInfo stuTfUserEditData;
        stuTfUserEditData.__set_userName(stuQcRltInfo.strOperator.toStdString());
        stuTfUserEditData.__set_outRuleReason(strReason.toStdString());
        stuTfUserEditData.__set_outRuleHandleScheme(strSolution.toStdString());

        // 构造质控结果
        ch::tf::QcResult stuTfQcRltInfo;
        stuTfQcRltInfo.__set_id(stuQcRltInfo.strID.toLongLong());
        stuTfQcRltInfo.__set_qcUserEditInfo(stuTfUserEditData);

        // 执行修改质控结果
        if (!ch::LogicControlProxy::ModifyQcResult(stuTfQcRltInfo))
        {
            return false;
        }

        // 更新质控结果数据
        stuQcRltInfo.strOutCtrlReason = strReason;
        stuQcRltInfo.strSolution = strSolution;

        // 压入输出缓冲区
        outBuffer.push_back(stuQcRltInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateTwinQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_MODIFY))
    {
        // 解析参数
        if (paramList.size() < 3)
        {
            return false;
        }
        TWIN_QC_RESULT_INFO stuQcRltInfo = boost::any_cast<TWIN_QC_RESULT_INFO>(paramList.front());
        QString strReason = boost::any_cast<QString>(paramList.at(1));
        QString strSolution = boost::any_cast<QString>(paramList.at(2));

        // 获取联合质控ID(默认ID = 联合质控ID-单质控ID)
        QStringList strIdList = stuQcRltInfo.strID.split("-");
        if (strIdList.size() < 2)
        {
            return false;
        }

        // 构造用户编辑信息
        tf::QcUserEditInfo stuTfUserEditData;
        stuTfUserEditData.__set_userName(stuQcRltInfo.strOperator.toStdString());
        stuTfUserEditData.__set_outRuleReason(strReason.toStdString());
        stuTfUserEditData.__set_outRuleHandleScheme(strSolution.toStdString());

        // 构造质控结果
        ch::tf::UnionQcResult stuTfQcRltInfo;
        stuTfQcRltInfo.__set_id(strIdList.front().toLongLong());
        if (stuQcRltInfo.bIsX)
        {
            stuTfQcRltInfo.__set_qcUserEditInfoForX(stuTfUserEditData);
        }
        else
        {
            stuTfQcRltInfo.__set_qcUserEditInfoForY(stuTfUserEditData);
        }

        // 执行修改质控结果
        if (!ch::LogicControlProxy::ModifyUnionQcResult(stuTfQcRltInfo))
        {
            return false;
        }

        // 更新质控结果数据
        stuQcRltInfo.strOutCtrlReason = strReason;
        stuQcRltInfo.strSolution = strSolution;

        // 压入输出缓冲区
        outBuffer.push_back(stuQcRltInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateSingleQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_MODIFY))
    {
        // 解析参数
        if (paramList.size() < 2)
        {
            return false;
        }
        QC_RESULT_INFO stuQcRltInfo = boost::any_cast<QC_RESULT_INFO>(paramList.front());
        bool bCalculate = boost::any_cast<bool>(paramList.at(1));

        // 构造质控结果
        ch::tf::QcResult stuTfQcRltInfo;
        stuTfQcRltInfo.__set_id(stuQcRltInfo.strID.toLongLong());
        stuTfQcRltInfo.__set_bCalcSelect(bCalculate);

        // 执行修改质控结果
        if (!ch::LogicControlProxy::ModifyQcResult(stuTfQcRltInfo))
        {
            return false;
        }

        // 更新质控结果数据
        stuQcRltInfo.bCalculated = bCalculate;

        // 压入输出缓冲区
        outBuffer.push_back(stuQcRltInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateTwinQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_MODIFY))
    {
        // 解析参数
        if (paramList.size() < 2)
        {
            return false;
        }
        TWIN_QC_RESULT_INFO stuQcRltInfo = boost::any_cast<TWIN_QC_RESULT_INFO>(paramList.front());
        bool bCalculate = boost::any_cast<bool>(paramList.at(1));

        // 获取联合质控ID(默认ID = 联合质控ID-单质控ID)
        QStringList strIdList = stuQcRltInfo.strID.split("-");
        if (strIdList.size() < 2)
        {
            return false;
        }

        // 构造质控结果
        ch::tf::UnionQcResult stuTfQcRltInfo;
        stuTfQcRltInfo.__set_id(strIdList.front().toLongLong());
        stuTfQcRltInfo.__set_bCalcSelect(bCalculate);

        // 执行修改质控结果
        if (!ch::LogicControlProxy::ModifyUnionQcResult(stuTfQcRltInfo))
        {
            return false;
        }

        // 更新质控结果数据
        stuQcRltInfo.bCalculated = bCalculate;

        // 压入输出缓冲区
        outBuffer.push_back(stuQcRltInfo);

        return true;
    }

    return true;
}

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
bool DevC1005::OperateTwinQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
	// 清空输出缓冲区
	outBuffer.clear();

	if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
	{
		// 解析参数
		QC_RESULT_QUERY_COND stuQryCond = boost::any_cast<QC_RESULT_QUERY_COND>(paramList.front());

		// 获取项目信息
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQryCond.strAssayName.toStdString());
		if (Q_NULLPTR == spAssayInfo)
		{
			return false;
		}

		// 构造质控结果的查询条件
		ch::tf::UnionQcResultQueryResp   qryResp;
		ch::tf::UnionQcResultQueryCond   qryCond;

		// 构造查询条件
		boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
			stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
		boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
			stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

		// 设置起止日期，bug2267 一个点在范围内也满足，故将全部查询出来后，再对时间进行筛选
		//tf::TimeRangeCond trCond;
		//trCond.__set_startTime(PosixTimeToTimeString(startDate));
		//trCond.__set_endTime(PosixTimeToTimeString(endDate));
		//qryCond.__set_ptQcTime(trCond);
		qryCond.__set_iAssayCode(std::int16_t(spAssayInfo->assayCode));
		qryCond.__set_strDevSN(DeviceSn().toStdString());

		// 查询所有试剂信息
		ch::LogicControlProxy::QueryUnionQcResult(qryResp, qryCond);
		if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			return false;
		}

		// 构造输出缓冲区
		int iIdx = 0;
		for (const auto& stuData : qryResp.lstQcResults)
		{

			// 构造质控结果
			TWIN_QC_RESULT_INFO stuQcRltInfoX;
			TWIN_QC_RESULT_INFO stuQcRltInfoY;

			// 查询X质控结果
			{
				// 查询质控结果
				ch::tf::QcResultQueryResp qcRltResp;
				ch::tf::QcResultQueryCond qcRltCond;

				// 构造查询条件
				qcRltCond.__set_id(stuData.XPointID);

				// 执行查询
				ch::LogicControlProxy::QueryQcResult(qcRltResp, qcRltCond);
				if (qcRltResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || qcRltResp.lstQcResults.empty())
				{
					continue;
				}

				// X质控结果
				ch::tf::QcResult& stuTfQcRltX = qcRltResp.lstQcResults[0];

				// 查询质控文档信息
				tf::QcDocQueryResp qcDocResp;
				tf::QcDocQueryCond qcDocCond;

				// 构造查询条件
                qcDocCond.__set_ids({ stuTfQcRltX.qcDocId });

				// 执行查询
				if (!DcsControlProxy::GetInstance()->QueryQcDoc(qcDocResp, qcDocCond) ||
					qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
					qcDocResp.lstQcDocs.empty())
				{
					continue;
				}

				// X质控文档
				tf::QcDoc& stuTfQcDocX = qcDocResp.lstQcDocs[0];

				// 构造失控规则
				QStringList strOutCtrlRuleList;
				for (auto& enOutCtrlRule : stuData.listTrigRules)
				{
                    // 在控则表示未触发失控规则
                    if ((enOutCtrlRule.XPointStat == tf::QcStat::QC_STAT_CONTROLLED) ||
                        (enOutCtrlRule.XPointStat == tf::QcStat::QC_STAT_NONE))
                    {
                        continue;
                    }

					strOutCtrlRuleList.push_back(ConvertTfEnumToQString(enOutCtrlRule.ruleType));
				}

				// 构造联合质控结果数据
				stuQcRltInfoX.bIsX = true;
				stuQcRltInfoX.iUnionIdx = iIdx + 1;
				stuQcRltInfoX.strQcNo = QString::fromStdString(stuTfQcDocX.sn);
				stuQcRltInfoX.strQcName = QString::fromStdString(stuTfQcDocX.name);
				stuQcRltInfoX.strQcBriefName = QString::fromStdString(stuTfQcDocX.shortName);
				stuQcRltInfoX.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuTfQcDocX.sampleSourceType));
				stuQcRltInfoX.strQcLevel = QString::number(stuTfQcDocX.level);
				stuQcRltInfoX.strQcLot = QString::fromStdString(stuTfQcDocX.lot);
				stuQcRltInfoX.bCalculated = stuData.bCalcSelect;
				stuQcRltInfoX.strID = QString::number(stuData.id) + "-" + QString::number(stuTfQcRltX.id);
				stuQcRltInfoX.strRltDetailID = QString::number(stuTfQcRltX.assayTestResultId);
				stuQcRltInfoX.strQcTime = QString::fromStdString(stuTfQcRltX.ptQcTime);
				stuQcRltInfoX.dQcResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltX.dConc);
				stuQcRltInfoX.enQcState = QC_STATE(stuData.XEmStat);
				stuQcRltInfoX.strOutCtrlRule = strOutCtrlRuleList.join(",");
				stuQcRltInfoX.dQcTargetVal = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltX.dTargetValue); 
				stuQcRltInfoX.dQcSD = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltX.dSTD);
				stuQcRltInfoX.strOutCtrlReason = QString::fromStdString(stuData.qcUserEditInfoForX.outRuleReason);
				stuQcRltInfoX.strSolution = QString::fromStdString(stuData.qcUserEditInfoForX.outRuleHandleScheme);
				stuQcRltInfoX.strOperator = QString::fromStdString(stuData.qcUserEditInfoForX.userName);
			}

			// 查询Y质控结果
			{
				// 查询质控结果
				ch::tf::QcResultQueryResp qcRltResp;
				ch::tf::QcResultQueryCond qcRltCond;

				// 构造查询条件
				qcRltCond.__set_id(stuData.YPointID);

				// 执行查询
				ch::LogicControlProxy::QueryQcResult(qcRltResp, qcRltCond);
				if (qcRltResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || qcRltResp.lstQcResults.empty())
				{
					continue;
				}

				// Y质控结果
				ch::tf::QcResult& stuTfQcRltY = qcRltResp.lstQcResults[0];

				// 查询质控文档信息
				tf::QcDocQueryResp qcDocResp;
				tf::QcDocQueryCond qcDocCond;

				// 构造查询条件
                qcDocCond.__set_ids({ stuTfQcRltY.qcDocId });

				// 执行查询
				if (!DcsControlProxy::GetInstance()->QueryQcDoc(qcDocResp, qcDocCond) ||
					qcDocResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
					qcDocResp.lstQcDocs.empty())
				{
					continue;
				}

				// Y质控文档
				tf::QcDoc& stuTfQcDocY = qcDocResp.lstQcDocs[0];

				// 构造失控规则
				QStringList strOutCtrlRuleList;
				for (auto& enOutCtrlRule : stuData.listTrigRules)
				{
                    // 在控则表示未触发失控规则
                    if ((enOutCtrlRule.YPointStat == tf::QcStat::QC_STAT_CONTROLLED) ||
                        (enOutCtrlRule.YPointStat == tf::QcStat::QC_STAT_NONE))
                    {
                        continue;
                    }

					strOutCtrlRuleList.push_back(ConvertTfEnumToQString(enOutCtrlRule.ruleType));
				}

				// 构造联合质控结果数据
				stuQcRltInfoY.bIsX = false;
				stuQcRltInfoY.iUnionIdx = iIdx + 1;
				stuQcRltInfoY.strQcNo = QString::fromStdString(stuTfQcDocY.sn);
				stuQcRltInfoY.strQcName = QString::fromStdString(stuTfQcDocY.name);
				stuQcRltInfoY.strQcBriefName = QString::fromStdString(stuTfQcDocY.shortName);
				stuQcRltInfoY.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuTfQcDocY.sampleSourceType));
				stuQcRltInfoY.strQcLevel = QString::number(stuTfQcDocY.level);
				stuQcRltInfoY.strQcLot = QString::fromStdString(stuTfQcDocY.lot);
				stuQcRltInfoY.bCalculated = stuData.bCalcSelect;
				stuQcRltInfoY.strID = QString::number(stuData.id) + "-" + QString::number(stuTfQcRltY.id);
				stuQcRltInfoY.strRltDetailID = QString::number(stuTfQcRltY.assayTestResultId);
				stuQcRltInfoY.strQcTime = QString::fromStdString(stuTfQcRltY.ptQcTime);
				stuQcRltInfoY.dQcResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltY.dConc);
				stuQcRltInfoY.enQcState = QC_STATE(stuData.YEmStat);
				stuQcRltInfoY.strOutCtrlRule = strOutCtrlRuleList.join(",");
				stuQcRltInfoY.dQcTargetVal = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltY.dTargetValue); 
				stuQcRltInfoY.dQcSD = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayInfo->assayCode, stuTfQcRltY.dSTD); 
				stuQcRltInfoY.strOutCtrlReason = QString::fromStdString(stuData.qcUserEditInfoForY.outRuleReason);
				stuQcRltInfoY.strSolution = QString::fromStdString(stuData.qcUserEditInfoForY.outRuleHandleScheme);
				stuQcRltInfoY.strOperator = QString::fromStdString(stuData.qcUserEditInfoForY.userName);
			}

			// 时间筛选，只要有X点的时间或Y点的时间在范围内即可
			QDateTime xDateTime = QDateTime::fromString(stuQcRltInfoX.strQcTime, "yyyy-MM-dd hh:mm:ss");
			QDateTime yDateTime = QDateTime::fromString(stuQcRltInfoY.strQcTime, "yyyy-MM-dd hh:mm:ss");
			// 如果时间转换不成功则不符合
			if (!xDateTime.isValid() || !yDateTime.isValid())
			{
				continue;
			}

			// 如果没有一个点在范围内，则不符合筛选
			if ((stuQryCond.startDate > xDateTime.date() || stuQryCond.endDate < xDateTime.date()) &&
				(stuQryCond.startDate > yDateTime.date() || stuQryCond.endDate < yDateTime.date())  )
			{
				continue;
			}

			// 压入列表
			outBuffer.push_back(stuQcRltInfoX);
			outBuffer.push_back(stuQcRltInfoY);

			// 索引自增
			++iIdx;
		}

		return true;
	}

	return true;
}


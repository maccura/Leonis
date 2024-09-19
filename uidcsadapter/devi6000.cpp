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
/// @file     devi6000.cpp
/// @brief    i6000设备接口
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
#include "devi6000.h"
#include <tuple>
#include <QVector>
#include <QPair>
#include <QMap>
#include <QList>
#include <QSet>

#include "adaptertypedef.h"
#include "shared/CommonInformationManager.h"
#include "shared/DataManagerQc.h"
#include "shared/uicommon.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_constants.h"
#include "src/common/TimeUtil.h"
#include "QC/QcRules.h"

// 初始化静态成员变量
QMap<QString, QVariant>          DevI6000::sm_mapDevNameInfo;         // 设备名和设备序列号映射

// 注册元类型
Q_DECLARE_METATYPE(tf::DeviceInfo)

DevI6000::DevI6000()
{
}

DevI6000::~DevI6000()
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
bool DevI6000::RegisterDevice()
{
    // 构造查询条件
    ::tf::DeviceInfoQueryResp devResp;
    ::tf::DeviceInfoQueryCond devCond;
    devCond.__set_deviceType(tf::DeviceType::DEVICE_TYPE_I6000);

    // 查询设备信息
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devResp, devCond)
        || devResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || devResp.lstDeviceInfos.empty())
    {
        return false;
    }

    // 注册i6000设备
    for (const auto& stuDevInfo: devResp.lstDeviceInfos)
    {
        // 注册设备
        DYNAMIC_REGISTER_DEVICE(DevI6000, QString::fromStdString(stuDevInfo.name), QString::fromStdString(stuDevInfo.groupName));

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
bool DevI6000::OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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
bool DevI6000::ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode)
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
int DevI6000::DeviceClass()
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
int DevI6000::DeviceType()
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
QString DevI6000::DeviceSn()
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
int DevI6000::ModuleCount()
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
///		试剂槽位数
///
/// @par History:
/// @li 8276/huchunli, 2022年12月28日，新建函数
///
int DevI6000::ReagentSlotNumber()
{
    return ::im::i6000::tf::g_i6000_constants.REAGENT_DISK_SLOT;
}

///
/// @brief
///     是否存在对应的质控申请信息
///
/// @param[in]  enQcType    质控类型
/// @param[in]  deviceSN    设备序列号
/// @param[in]  qcDocId     质控品ID
/// @param[in]  iAssayCode  项目编号
/// @param[in]  rgntLot     试剂批号
/// @param[in]  rgntSn      试剂瓶号
///
/// @return true表示存在
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月31日，新建函数
///
static bool WhetherHasQcApply(im::tf::QcType::type enQcType, const std::string& deviceSN, std::int64_t qcDocId, int iAssayCode,
                              const std::string& rgntLot = "", const std::string& rgntSn = "")
{
    // 构造查询条件
    im::tf::QcApplyQueryCond qryCond;
    im::tf::QcApplyQueryResp qryResp;

    // 将参数传入查询条件
    qryCond.__set_qcType(enQcType);
    qryCond.__set_deviceSN(deviceSN);
    qryCond.__set_qcDocId(qcDocId);
    qryCond.__set_assayCode(iAssayCode);
    if (enQcType == im::tf::QcType::QC_TYPE_BACKUP)
    {
        qryCond.__set_lot(rgntLot);
        qryCond.__set_sn(rgntSn);
    }

    // 执行查询
    if (!im::LogicControlProxy::QueryQcApply(qryResp, qryCond) || (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        return false;
    }

    // 结果为空，返回失败
    if (qryResp.lstQcApplys.empty())
    {
        return false;
    }

    return true;
}

///
/// @brief
///     通过试剂列表配对试剂瓶组
///
/// @param[in]  stuRgntList  试剂列表
///
/// @return 试剂瓶组列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月13日，新建函数
///
QList<QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>> GetRgntGrpByRgntList(const std::vector<im::tf::ReagentInfoTable>& stuRgntList)
{
    // 测试
//     std::vector<im::tf::ReagentInfoTable> stuRgntListTest;
//     im::tf::ReagentInfoTable imRgnt1;
//     im::tf::ReagentInfoTable imRgnt2;
//     im::tf::ReagentInfoTable imRgnt3;
//     imRgnt1.__set_id(1);
//     imRgnt2.__set_id(2);
//     imRgnt3.__set_id(3);
//     imRgnt1.__set_reagentPos(1);
//     imRgnt2.__set_reagentPos(2);
//     imRgnt3.__set_reagentPos(3);
//     imRgnt1.__set_assayCode(905);
//     imRgnt2.__set_assayCode(905);
//     imRgnt3.__set_assayCode(906);
//     imRgnt1.__set_reagentLot("1");
//     imRgnt2.__set_reagentLot("1");
//     imRgnt3.__set_reagentLot("2");
//     imRgnt1.__set_reagentSN("aaa");
//     imRgnt2.__set_reagentSN("bbb");
//     imRgnt3.__set_reagentSN("ccc");
//     imRgnt1.__set_subStep("bbb");
//     imRgnt2.__set_subStep("aaa");
//     imRgnt1.__set_bottleIdx(1);
//     imRgnt2.__set_bottleIdx(0);
//     stuRgntListTest.push_back(imRgnt1);
//     stuRgntListTest.push_back(imRgnt2);
//     stuRgntListTest.push_back(imRgnt3);

    // 构造容器
    QList<QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>> rgntGrpList;

    // 用于缓存未配对的试剂信息
    QMap<std::tuple<int, std::string, std::string>, im::tf::ReagentInfoTable> mapNoMatchRgnt;
    for (const auto& stuRgntInfo : stuRgntList)
    {
        // 构造可配对键（项目编号、试剂批号、试剂瓶号匹配才能配对）
        std::tuple<int, std::string, std::string> tupKey(stuRgntInfo.assayCode, stuRgntInfo.reagentLot, stuRgntInfo.reagentSN);
        std::tuple<int, std::string, std::string> tupMatchKey(stuRgntInfo.assayCode, stuRgntInfo.reagentLot, stuRgntInfo.subStep);

        // 在未配对列表中寻找配对
        auto it = mapNoMatchRgnt.find(tupMatchKey);
        if ((it == mapNoMatchRgnt.end()) || (it.value().bottleIdx == stuRgntInfo.bottleIdx))
        {
            // 未配对成功，则加入未配对列表
            mapNoMatchRgnt.insert(tupKey, stuRgntInfo);
            continue;
        }

        // 配对成功则构造配对信息
        QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable> pairRgntInfo;
        if (bool(stuRgntInfo.bottleIdx))
        {
            // 预处理试剂放前面
            pairRgntInfo.first = stuRgntInfo;
            pairRgntInfo.second = it.value();
        }
        else
        {
            // 预处理试剂放后面
            pairRgntInfo.first = it.value();
            pairRgntInfo.second = stuRgntInfo;
        }

        // 配对成功则将其从未配对列表中移除
        mapNoMatchRgnt.erase(it);

        // 插入试剂组列表
        rgntGrpList.push_back(std::move(pairRgntInfo));
    }

    // 将不能配对的试剂作为单试剂的组插入试剂组表
    for (auto it = mapNoMatchRgnt.begin(); it != mapNoMatchRgnt.end(); it++)
    {
        // 统一放在第一个位置
        QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable> pairRgntInfo;
        pairRgntInfo.first = it.value();

        // 插入试剂组列表
        rgntGrpList.push_back(std::move(pairRgntInfo));
    }

    // 排序
    std::sort(rgntGrpList.begin(), rgntGrpList.end(), 
        [](const QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>& pairRgntInfo1, const QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>& pairRgntInfo2)
    {
        // 通道号相同，则比较位置
        if (pairRgntInfo1.first.assayCode == pairRgntInfo2.second.assayCode)
        {
            return pairRgntInfo1.first.reagentPos < pairRgntInfo2.first.reagentPos;
        }

        return pairRgntInfo1.first.assayCode < pairRgntInfo2.second.assayCode;
    });

    return rgntGrpList;
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
bool DevI6000::OperateQcDocData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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
        // 查询质控组合信息
        std::vector<QString> vecQcNames;
        DataManagerQc::GetInstance()->GetQcDocNames(vecQcNames);
        for (const QString& str : vecQcNames)
        {
            outBuffer.push_back(str);
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
bool DevI6000::OperateQcApplyData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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

    // 查询数据
    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QString strQcName = boost::any_cast<QString>(paramList.front());

        // 查询并缓存所有质控申请信息
        ::im::tf::QcApplyQueryResp _return;
        ::im::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());

        // 查询质控申请信息
        ::im::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<QPair<int, long long>, QString> mapQcReason;
        QMap<QPair<int, long long>, QPair<QString, QString>> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 判断质控原因
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_BACKUP)
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
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_ONUSE)
            {
                mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(stuQcApp.qcReason));
                itId.value().first = QString::number(stuQcApp.id);
                continue;
            }

            // 如果是默认质控，没有手工勾选就为默认质控
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_DEFAULT)
            {
                itId.value().second = QString::number(stuQcApp.id);
                if (mapQcReason.find(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId)) == mapQcReason.end())
                {
                    mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(tf::QcReason::QC_REASON_DEFAULT));
                }
                continue;
            }
        }

        // 构造查询条件和查询结果
        ::im::tf::ReagTableUIQueryCond qryCond;
        ::im::tf::ReagentInfoTableQueryResp qryResp;

        // 只获取在机的在用瓶（该模块）
        //qryCond.__set_onUse(true);(按照需求，只要有在机试剂，项目都显示)

        // 设备SN列表
        std::vector<std::string> vecStrSns;
        vecStrSns.push_back(DeviceSn().toStdString());
        qryCond.__set_deviceSNs(vecStrSns);

        // 获取在用试剂信息
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 获取试剂组
        auto stuRgntGrpList = GetRgntGrpByRgntList(qryResp.lstReagentInfos);

        // 试剂按项目编号排序
        std::sort(stuRgntGrpList.begin(), stuRgntGrpList.end(), 
                [](const QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>& c1, const QPair<im::tf::ReagentInfoTable, im::tf::ReagentInfoTable>& c2)
                {return c1.first.assayCode < c2.first.assayCode; });

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 记录已使用的项目编号
        QSet<int> setUsedAssayCode;

        // 组合质控申请信息
        QC_APPLY_INFO stuQcApplyInfo;
        QString strManApplyID("");
        QString strDefApplyID("");
        QString strQcReason("");
        QPair<int, long long> pairAssayDoc;
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1; //未使用，固定为1
        stuQcApplyInfo.strModuleName = DeviceName();
        for (const auto& stuRgntPairInfo : stuRgntGrpList)
        {
            // 查找项目编号
            if (setUsedAssayCode.find(stuRgntPairInfo.first.assayCode) != setUsedAssayCode.end())
            {
                continue;
            }
            setUsedAssayCode.insert(stuRgntPairInfo.first.assayCode);

            // 获取项目信息
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntPairInfo.first.assayCode);
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

            for (const auto& stuQcDoc : allQcDocs)
            {
                // 赋值质控信息
                if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo))
                {
                    // 质控文档相关信息
                    stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc->sn);
                    stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc->name);
                    stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc->shortName);
                    stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc->sampleSourceType));
                    stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc->level);
                    stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc->lot);
                    stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc->id);
                    AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo);
                }

                // 参数不匹配则跳过
                if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
                {
                    continue;
                }

                // 质控品和试剂不匹配则跳过
                if (!IsQcDocMatchedRgnt(*stuQcDoc, stuRgntPairInfo.first.assayCode))
                {
                    continue;
                }

                // 获取质控状态(质控原因)
                strQcReason.clear();
                pairAssayDoc.first = stuRgntPairInfo.first.assayCode;
                pairAssayDoc.second = stuQcDoc->id;
                auto itQcReason = mapQcReason.find(pairAssayDoc);
                if (itQcReason != mapQcReason.end())
                {
                    strQcReason = itQcReason.value();
                }

                // 构造质控申请ID
                strManApplyID.clear();
                strDefApplyID.clear();
                pairAssayDoc.first = stuRgntPairInfo.first.assayCode;
                pairAssayDoc.second = stuQcDoc->id;
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
                outBuffer.push_back(stuQcApplyInfo);
            }
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 添加质控申请信息
        im::tf::QcApply qa;
        ::tf::ResultLong _return;

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 获取质控ID
        QStringList strIDList = stuApplyInfo.strID.split("-");
        if (strIDList.size() >= 2)
        {
            qa.__set_qcDocId(strIDList[1].toLongLong());
        }

        // 构造质控申请信息
        qa.__set_assayCode(spAssayInfo->assayCode);
        qa.__set_deviceSN(DeviceSn().toStdString());
        qa.__set_qcType(im::tf::QcType::QC_TYPE_ONUSE);
        qa.__set_qcReason(im::tf::QcReason::QC_REASON_MANUAL);

        // 执行添加
        im::LogicControlProxy::AddQcApply(_return, qa);
        if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 构造输出缓冲区
        //im::tf::QcReason::type eQcStatus = im::LogicControlProxy::GetQcApplyReason(DeviceSn().toStdString(), qa.qcDocId, spAssayInfo->assayCode);
        im::tf::QcReason::type eQcStatus = im::tf::QcReason::QC_REASON_MANUAL;
        stuApplyInfo.strManApplyID = QString::number(_return.value);
        stuApplyInfo.bSelected = true;
        stuApplyInfo.strQcReason = ConvertTfEnumToQString(eQcStatus);
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    // 质控申请取消选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_DELETE))
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
        std::int64_t qcDocId = -1;
        QStringList strIDList = stuApplyInfo.strID.split("-");
        if (strIDList.size() >= 2)
        {
            qcDocId = strIDList[1].toLongLong();
        }

        // 构造删除条件
        im::tf::QcApplyQueryCond qrycond;
        qrycond.__set_id(stuApplyInfo.strManApplyID.toLongLong());
        if (!im::LogicControlProxy::DeleteQcApply(qrycond))
        {
            return false;
        }

        // 构造输出缓冲区
        //im::tf::QcReason::type eQcStatus = im::LogicControlProxy::GetQcApplyReason(DeviceSn().toStdString(), qcDocId, spAssayInfo->assayCode);
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
bool DevI6000::OperateDefaultQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QString strQcName = boost::any_cast<QString>(paramList.front());

        // 查询并缓存所有质控申请信息
        ::im::tf::QcApplyQueryResp _return;
        ::im::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());

        // 查询质控申请信息
        ::im::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<QPair<int, long long>, QString> mapQcReason;
        QMap<QPair<int, long long>, QPair<QString, QString>> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 判断质控原因
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_BACKUP)
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
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_ONUSE)
            {
                mapQcReason.insert(qMakePair(stuQcApp.assayCode, stuQcApp.qcDocId), ConvertTfEnumToQString(stuQcApp.qcReason));
                itId.value().first = QString::number(stuQcApp.id);
                continue;
            }

            // 如果是默认质控，没有手工勾选就为默认质控
            if (stuQcApp.qcType == im::tf::QcType::QC_TYPE_DEFAULT)
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
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 组合质控申请信息
        QC_APPLY_INFO stuQcApplyInfo;
        QString strManApplyID("");
        QString strDefApplyID("");
        QString strQcReason("");
        QPair<int, long long> pairAssayDoc;
        tf::AssayClassify::type enTfClassfy = tf::AssayClassify::type(DeviceClass());
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1;//未使用，固定为1
        stuQcApplyInfo.strModuleName = DeviceName();

        // 组合质控申请信息
        // 组合质控申请信息
        std::multimap<int, QC_APPLY_INFO> sortBuffer;
        for (const auto& stuQcDoc : allQcDocs)
        {
            // 赋值质控信息
            if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo))
            {
                // 质控文档相关信息
                stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc->sn);
                stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc->name);
                stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc->shortName);
                stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc->sampleSourceType));
                stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc->level);
                stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc->lot);
                stuQcApplyInfo.strQcExpDate = QString::fromStdString(stuQcDoc->expireTime);
                stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc->id);
                AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo);
            }

            // 参数不匹配则跳过
            if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
            {
                continue;
            }

            for (const auto& stuQcAssay : stuQcDoc->compositions)
            {
                // 获取项目信息
                std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQcAssay.assayCode);
                if (Q_NULLPTR == spAssayInfo)
                {
                    continue;
                }

                // 设备类型不匹配则跳过
                if (spAssayInfo->assayClassify != enTfClassfy)
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
                strQcReason.clear();
                pairAssayDoc.first = spAssayInfo->assayCode;
                pairAssayDoc.second = stuQcDoc->id;
                auto itQcReason = mapQcReason.find(pairAssayDoc);
                if (itQcReason != mapQcReason.end())
                {
                    strQcReason = itQcReason.value();
                }

                // 构造质控申请ID
                strManApplyID.clear();
                strDefApplyID.clear();
                pairAssayDoc.first = spAssayInfo->assayCode;
                pairAssayDoc.second = stuQcDoc->id;
                auto itQcAppId = mapQcApplyId.find(pairAssayDoc);
                if (itQcAppId != mapQcApplyId.end())
                {
                    strManApplyID = itQcAppId.value().first;
                    strDefApplyID = itQcAppId.value().second;
                }

                // 构造是否选择
                bool bSel = !strDefApplyID.isEmpty();

                // 构造质控申请信息
                stuQcApplyInfo.strQcReason = strQcReason;
                stuQcApplyInfo.bSelected = bSel;
                stuQcApplyInfo.strID = stuQcApplyInfo.strAssayCode + "-" + stuQcApplyInfo.strQcDocID;
                stuQcApplyInfo.strManApplyID = strManApplyID;
                stuQcApplyInfo.strDefApplyID = strDefApplyID;

                // 构造输出缓存
                sortBuffer.insert(std::make_pair(spAssayInfo->assayCode, stuQcApplyInfo));
            }
        }

        // 插入缓存
        for (const auto& pairQcApplyInfo : sortBuffer)
        {
            outBuffer.push_back(pairQcApplyInfo.second);
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 添加质控申请信息
        im::tf::QcApply qa;
        ::tf::ResultLong _return;

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 获取质控ID
        QStringList strIDList = stuApplyInfo.strID.split("-");
        if (strIDList.size() >= 2)
        {
            qa.__set_qcDocId(strIDList[1].toLongLong());
        }

        // 构造质控申请信息
        qa.__set_assayCode(spAssayInfo->assayCode);
        qa.__set_deviceSN(DeviceSn().toStdString());
        qa.__set_qcType(im::tf::QcType::QC_TYPE_DEFAULT);
        qa.__set_qcReason(im::tf::QcReason::QC_REASON_DEFAULT);

        // 执行添加
        im::LogicControlProxy::AddQcApply(_return, qa);
        if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 构造输出缓冲区
        //im::tf::QcReason::type eQcStatus = im::LogicControlProxy::GetQcApplyReason(DeviceSn().toStdString(), qa.qcDocId, spAssayInfo->assayCode);
        stuApplyInfo.bSelected = true;
        stuApplyInfo.strDefApplyID = QString::number(_return.value);
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

        // 构造删除条件
        im::tf::QcApplyQueryCond qrycond;
        qrycond.__set_id(stuApplyInfo.strDefApplyID.toLongLong());
        if (!im::LogicControlProxy::DeleteQcApply(qrycond))
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
bool DevI6000::OperateBackupRgntQcData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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
        ::im::tf::QcApplyQueryResp _return;
        ::im::tf::QcApplyQueryCond qcapcd;
        qcapcd.__set_deviceSN(DeviceSn().toStdString());
        qcapcd.__set_qcType(im::tf::QcType::QC_TYPE_BACKUP);

        // 查询质控申请信息
        ::im::LogicControlProxy::QueryQcApply(_return, qcapcd);
        if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 缓存质控申请数据
        QMap<std::tuple<int, long long, QString, QString>, QString> mapQcApplyId;
        for (const auto& stuQcApp : _return.lstQcApplys)
        {
            // 判断质控原因
            if (stuQcApp.qcType != im::tf::QcType::QC_TYPE_BACKUP)
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
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 构造查询条件和查询结果
        ::im::tf::ReagTableUIQueryCond qryCond;
        ::im::tf::ReagentInfoTableQueryResp qryResp;

        // 只获取在机的备用瓶（该模块）
        qryCond.__set_backUp(true);

        // 设备SN列表
        std::vector<std::string> vecStrSns;
        vecStrSns.push_back(DeviceSn().toStdString());
        qryCond.__set_deviceSNs(vecStrSns);

        // 获取在用试剂信息
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 获取试剂组
        auto stuRgntGrpList = GetRgntGrpByRgntList(qryResp.lstReagentInfos);

        // 组合质控申请信息
        QC_APPLY_INFO stuQcApplyInfo;
        QString strQcAppId("");
        QString strQcReason("");
        std::tuple<int, long long, QString, QString> tupleRgntDoc;
        tf::AssayClassify::type enTfClassfy = tf::AssayClassify::type(DeviceClass());
        stuQcApplyInfo.strDevName = DeviceName();
        stuQcApplyInfo.iModuleNo = 1;//未使用，固定为1
        stuQcApplyInfo.strModuleName = DeviceName();

        // 组合质控申请信息
        for (const auto& stuRgntPairInfo : stuRgntGrpList)
        {
            // 获取项目信息
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntPairInfo.first.assayCode);
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

            // 赋值试剂信息
            if (!AdapterDataBuffer::GetInstance()->AssignRgntInfoToQcApply(DeviceSn(), stuRgntPairInfo.first.reagentPos, stuQcApplyInfo))
            {
                // 获取使用状态
                QString strUseStatus = ConvertTfEnumToQString(stuRgntPairInfo.first.reagStatusUI.status);
                if (stuRgntPairInfo.first.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
                {
                    strUseStatus += QString::number(stuRgntPairInfo.first.reagStatusUI.backupCnt) + "-" + QString::number(stuRgntPairInfo.first.reagStatusUI.backupIndx);
                }

                // 构造位置字符串和瓶号字符串
                QString strRgntPos = DeviceName() + "-" + QString::number(stuRgntPairInfo.first.reagentPos);
                QString strRgntSn = QString::fromStdString(stuRgntPairInfo.first.reagentSN);
                if (stuRgntPairInfo.second.id > 0)
                {
                    strRgntPos += QObject::tr("，") + QString::number(stuRgntPairInfo.second.reagentPos);
                    strRgntSn += QObject::tr("，") + QString::fromStdString(stuRgntPairInfo.second.reagentSN);
                }

                // 试剂相关信息
                stuQcApplyInfo.iBackupRgntPos = stuRgntPairInfo.first.reagentPos;
                stuQcApplyInfo.strPos = strRgntPos;
                stuQcApplyInfo.strRgntUseStatus = strUseStatus;
                stuQcApplyInfo.strRgntNo = strRgntSn;
                stuQcApplyInfo.strRgntLot = QString::fromStdString(stuRgntPairInfo.first.reagentLot);
                AdapterDataBuffer::GetInstance()->MapAssayInfoToQcApply(spAssayInfo->assayCode, stuQcApplyInfo);
            }

            for (const auto& stuQcDoc : allQcDocs)
            {
                // 赋值质控信息
                if (!AdapterDataBuffer::GetInstance()->AssignQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo))
                {
                    // 质控文档相关信息
                    stuQcApplyInfo.strQcNo = QString::fromStdString(stuQcDoc->sn);
                    stuQcApplyInfo.strQcName = QString::fromStdString(stuQcDoc->name);
                    stuQcApplyInfo.strQcBriefName = QString::fromStdString(stuQcDoc->shortName);
                    stuQcApplyInfo.strQcSourceType = ConvertTfEnumToQString(tf::SampleSourceType::type(stuQcDoc->sampleSourceType));
                    stuQcApplyInfo.strQcLevel = QString::number(stuQcDoc->level);
                    stuQcApplyInfo.strQcLot = QString::fromStdString(stuQcDoc->lot);
                    stuQcApplyInfo.strQcExpDate = QString::fromStdString(stuQcDoc->expireTime);
                    stuQcApplyInfo.strQcDocID = QString::number(stuQcDoc->id);
                    AdapterDataBuffer::GetInstance()->MapQcInfoToQcApply(stuQcDoc->id, stuQcApplyInfo);
                }

                // 参数不匹配则跳过
                if (!strQcName.isEmpty() && (strQcName != stuQcApplyInfo.strQcName))
                {
                    continue;
                }

                // 质控品和试剂不匹配则跳过
                if (!IsQcDocMatchedRgnt(*stuQcDoc, stuRgntPairInfo.first.assayCode))
                {
                    continue;
                }

                // 获取瓶号
                QStringList strRgntNoList = stuQcApplyInfo.strRgntNo.split(QObject::tr("，"));
                if (strRgntNoList.isEmpty())
                {
                    // 取最后一个瓶号（分析试剂的瓶号）
                    continue;
                }

                // 获取质控申请ID
                strQcAppId.clear();
                std::get<0>(tupleRgntDoc) = spAssayInfo->assayCode;
                std::get<1>(tupleRgntDoc) = stuQcDoc->id;
                std::get<2>(tupleRgntDoc) = stuQcApplyInfo.strRgntLot;
                std::get<3>(tupleRgntDoc) = strRgntNoList.last();
                auto itQcAppId = mapQcApplyId.find(tupleRgntDoc);
                if (itQcAppId != mapQcApplyId.end())
                {
                    strQcAppId = itQcAppId.value();
                }

                // 是否选择
                bool bSelect = !strQcAppId.isEmpty();

                // 构造质控申请信息
                stuQcApplyInfo.bSelected = bSelect;
                stuQcApplyInfo.strID = stuQcApplyInfo.strPos + "-" + stuQcApplyInfo.strQcDocID;
                stuQcApplyInfo.strManApplyID = strQcAppId;

                // 构造输出缓存
                outBuffer.push_back(stuQcApplyInfo);
            }
        }

        return true;
    }

    // 质控申请选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_ADD))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 添加质控申请信息
        im::tf::QcApply qa;
        ::tf::ResultLong _return;

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuApplyInfo.strAssayName.toStdString());
        if (Q_NULLPTR == spAssayInfo)
        {
            return false;
        }

        // 获取瓶号
        QStringList strRgntNoList = stuApplyInfo.strRgntNo.split(QObject::tr("，"));
        if (!strRgntNoList.isEmpty())
        {
            // 取最后一个瓶号（分析试剂的瓶号）
            qa.__set_sn(strRgntNoList.last().toStdString());
        }

        // 构造质控申请信息
        qa.__set_assayCode(spAssayInfo->assayCode);
        qa.__set_qcDocId(stuApplyInfo.strQcDocID.toLongLong());
        qa.__set_deviceSN(DeviceSn().toStdString());
        qa.__set_lot(stuApplyInfo.strRgntLot.toStdString());
        qa.__set_qcType(im::tf::QcType::QC_TYPE_BACKUP);

        // 执行添加
        im::LogicControlProxy::AddQcApply(_return, qa);
        if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 构造输出缓冲区
        stuApplyInfo.bSelected = true;
        stuApplyInfo.strManApplyID = QString::number(_return.value);
        outBuffer.push_back(stuApplyInfo);

        return true;
    }

    // 质控申请取消选择
    if (iDataOpCode == int(DATA_OPERATE_TYPE_DELETE))
    {
        // 解析参数
        QC_APPLY_INFO stuApplyInfo = boost::any_cast<QC_APPLY_INFO>(paramList.front());

        // 构造删除条件
        im::tf::QcApplyQueryCond qrycond;
        qrycond.__set_id(stuApplyInfo.strManApplyID.toLongLong());

        // 执行删除
        if (!im::LogicControlProxy::DeleteQcApply(qrycond))
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
bool DevI6000::OperateAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ImAssayNameMaps mapAssayNameInfo;
        if (!CommonInformationManager::GetInstance()->GetImAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayNameInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayNameInfo.begin(); it != mapAssayNameInfo.end(); it++)
        {
            strAssayList.push_back(QString::fromStdString(it->first));
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
bool DevI6000::OperateQcLjAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ImAssayNameMaps mapAssayNameInfo;
        if (!CommonInformationManager::GetInstance()->GetImAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayNameInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayNameInfo.begin(); it != mapAssayNameInfo.end(); it++)
        {
            strAssayList.push_back(QString::fromStdString(it->first));
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
bool DevI6000::OperateQcTpAssayNameListData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        ImAssayNameMaps mapAssayNameInfo;
        if (!CommonInformationManager::GetInstance()->GetImAssayMaps((tf::DeviceType::type)DeviceType(), mapAssayNameInfo))
        {
            return false;
        }

        // 构造项目名
        QStringList strAssayList;
        for (auto it = mapAssayNameInfo.begin(); it != mapAssayNameInfo.end(); it++)
        {
            // 没有联合质控规则则跳过
            if (it->second->qcRules.XQCDocID <= 0 || it->second->qcRules.YQCDocID <= 0)
            {
                continue;
            }

            // 压入项目列表
            strAssayList.push_back(QString::fromStdString(it->first));
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
bool DevI6000::OperateQcConcInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
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

        // 构造查询条件
        // 获取起止日期
        // 获取起止日期
        boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
            stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
        boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
            stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

        // 初始化质控信息和查询条件
        im::tf::QcLjAssayResp       qryResp;
        im::tf::QcResultQueryCond   qryCond;

        // 设置起止日期
        tf::TimeRangeCond trCond;
        trCond.__set_startTime(PosixTimeToTimeString(startDate));
        trCond.__set_endTime(PosixTimeToTimeString(endDate));
        qryCond.__set_timeRange(trCond);
        qryCond.__set_assayCode(spAssayInfo->assayCode);
        qryCond.__set_deviceSN(DeviceSn().toStdString());

        // 执行查询
        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryQcLjAssay(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        // 构造输出缓冲区
        for (const auto& stuData : qryResp.lstQcLjAssay)
        {
            QC_DOC_CONC_INFO stuQcConcInfo;
            stuQcConcInfo.strID = QString::number(stuData.qcDocId);
            stuQcConcInfo.strQcNo = QString::fromStdString(stuData.qcSerial);
            stuQcConcInfo.strQcName = QString::fromStdString(stuData.qcName);
            stuQcConcInfo.strQcBriefName = QString::fromStdString(stuData.qcShortName);
            stuQcConcInfo.strQcSourceType = ConvertTfEnumToQString(stuData.sampleSourceType);
            stuQcConcInfo.strQcLevel = QString::number(stuData.qcLevel);
            stuQcConcInfo.strQcLot = QString::fromStdString(stuData.qcBatch);
            stuQcConcInfo.dQcTargetVal = stuData.qcTarget;
            stuQcConcInfo.dQcSD = stuData.qcSD;
            stuQcConcInfo.dQcCV = stuData.qcCV;
            stuQcConcInfo.dQcCalcTargetVal = stuData.qcCalcTarget;
            stuQcConcInfo.dQcCalcSD = stuData.qcCalcSD;
            stuQcConcInfo.dQcCalcCV = stuData.qcCalcCV;
            stuQcConcInfo.strQcRltCount = QString::number(stuData.qcResultCount);

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
bool DevI6000::OperateQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 清空输出缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QC_RESULT_QUERY_COND stuQryCond = boost::any_cast<QC_RESULT_QUERY_COND>(paramList.front());

        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuQryCond.strAssayName.toStdString());

        // 构造质控结果的查询条件
        im::tf::QcLjMaterialResp    qryResp;
        im::tf::QcResultQueryCond   qryCond;

        // 构造查询条件
        boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
            stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
        boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
            stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

        // 设置起止日期
        tf::TimeRangeCond trCond;
        trCond.__set_startTime(PosixTimeToTimeString(startDate));
        trCond.__set_endTime(PosixTimeToTimeString(endDate));
        qryCond.__set_timeRange(trCond);
        if (Q_NULLPTR != spAssayInfo)
        {
            qryCond.__set_assayCode(spAssayInfo->assayCode);
        }

        if (!stuQryCond.strQcDocID.isEmpty())
        {
            qryCond.__set_qcDocId(stuQryCond.strQcDocID.toLongLong());
        }
        qryCond.__set_deviceSN(DeviceSn().toStdString());

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryQcLjMaterial(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        QcRules* qcRulesObj = QcRules::GetInstance();
        // 构造输出缓冲区
        for (const auto& stuData : qryResp.lstQcLjMaterial)
        {
            // 获取项目信息
            spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuData.assayCode);
            if (Q_NULLPTR == spAssayInfo)
            {
                continue;
            }

            QC_RESULT_INFO stuQcRltInfo;
            stuQcRltInfo.bCalculated = !stuData.ljCalcExcluded;
            stuQcRltInfo.strID = QString::number(stuData.dbId);
            stuQcRltInfo.strQcDocID = QString::number(stuData.qcDocId);
            stuQcRltInfo.strAssayName = QString::fromStdString(spAssayInfo->assayName);
            stuQcRltInfo.strRltDetailID = QString::number(stuData.testResultId);
            stuQcRltInfo.strQcTime = QString::fromStdString(stuData.qcTime);
            stuQcRltInfo.dQcResult = stuData.qcResult;
            stuQcRltInfo.enQcState = QC_STATE(stuData.qcStatus);
            stuQcRltInfo.strOutCtrlRule = qcRulesObj->TranseRuleCode(stuData.qcBrackRules);
            stuQcRltInfo.dQcTargetVal = stuData.qcTargt;
            stuQcRltInfo.dQcSD = stuData.sd;
            stuQcRltInfo.strOutCtrlReason = QString::fromStdString(stuData.breakReason);
            stuQcRltInfo.strSolution = QString::fromStdString(stuData.solution);
            stuQcRltInfo.strOperator = QString::fromStdString(stuData.handler);

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
bool DevI6000::OperateQcTargetValSDData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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
            composition.targetValue = stuQcConcInfo.dQcCalcTargetVal;
            composition.sd = stuQcConcInfo.dQcCalcSD;
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
			// 返回失败原因
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
bool DevI6000::OperateSingleQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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

        // 保存失控原因和失控处理
        if (!im::LogicControlProxy::UpdateQcBreakReasonAndSolution(stuQcRltInfo.strID.toLongLong(), im::tf::controlWay::CONTROL_SINGLE, strReason.toStdString(), strSolution.toStdString()))
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
bool DevI6000::OperateTwinQcReasonSolutionData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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

        // 保存失控原因和失控处理
        if (!im::LogicControlProxy::UpdateQcBreakReasonAndSolution(stuQcRltInfo.strID.toLongLong(), im::tf::controlWay::CONTROL_UNION, strReason.toStdString(), strSolution.toStdString()))
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
bool DevI6000::OperateSingleQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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

        // 获取质控方式
        im::tf::controlWay::type enCtrlWay = im::tf::controlWay::CONTROL_SINGLE;

        // 执行更新排除计算点
        if (!im::LogicControlProxy::UpdateQcResultExclude(stuQcRltInfo.strID.toLongLong(), enCtrlWay, !bCalculate))
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
bool DevI6000::OperateTwinQcCalcPointData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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

        // 获取质控方式
        im::tf::controlWay::type enCtrlWay = im::tf::controlWay::CONTROL_UNION;

        // 执行更新排除计算点
        if (!im::LogicControlProxy::UpdateQcResultExclude(stuQcRltInfo.strID.toLongLong(), enCtrlWay, !bCalculate))
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
bool DevI6000::OperateTwinQcRltInfoData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
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
        im::tf::QcYoudenResp        qryResp;
        im::tf::QcResultQueryCond   qryCond;

        // 构造查询条件
        boost::posix_time::ptime startDate(boost::gregorian::date(stuQryCond.startDate.year(),
            stuQryCond.startDate.month(), stuQryCond.startDate.day()), boost::posix_time::hours(0) + boost::posix_time::minutes(0) + boost::posix_time::seconds(0));
        boost::posix_time::ptime endDate(boost::gregorian::date(stuQryCond.endDate.year(),
            stuQryCond.endDate.month(), stuQryCond.endDate.day()), boost::posix_time::hours(23) + boost::posix_time::minutes(59) + boost::posix_time::seconds(59));

        // 设置起止日期
        tf::TimeRangeCond trCond;
        trCond.__set_startTime(PosixTimeToTimeString(startDate));
        trCond.__set_endTime(PosixTimeToTimeString(endDate));
        qryCond.__set_timeRange(trCond);
        qryCond.__set_assayCode(spAssayInfo->assayCode);
        qryCond.__set_deviceSN(DeviceSn().toStdString());

        // 查询所有试剂信息
        bool bRet = ::im::LogicControlProxy::QueryQcYouden(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return false;
        }

        QcRules *qcRuleObj = QcRules::GetInstance();
        // 构造输出缓冲区
        int iIdx = 0;
        for (const auto& stuData : qryResp.lstYoudenRows)
        {
            TWIN_QC_RESULT_INFO stuQcRltInfo;
            stuQcRltInfo.bIsX = (iIdx % 2 == 0); // 容器中的存储顺序为[x、y、x、y、x、y]
            stuQcRltInfo.iUnionIdx = (iIdx / 2) + 1;
            stuQcRltInfo.strQcNo = QString::fromStdString(stuData.qcSerial);
            stuQcRltInfo.strQcName = QString::fromStdString(stuData.qcName);
            stuQcRltInfo.strQcBriefName = QString::fromStdString(stuData.qcShortName);
            stuQcRltInfo.strQcSourceType = ConvertTfEnumToQString(stuData.qcSampleType);
            stuQcRltInfo.strQcLevel = QString::number(stuData.qcLevel);
            stuQcRltInfo.strQcLot = QString::fromStdString(stuData.qcBatch);
            stuQcRltInfo.bCalculated = !stuData.qcCalcExclude;
            stuQcRltInfo.strID = QString::number(stuData.qcResultId);
            stuQcRltInfo.strRltDetailID = QString::number(stuData.testResultId);
            stuQcRltInfo.strQcTime = QString::fromStdString(stuData.qcTime);
            stuQcRltInfo.dQcResult = stuData.qcResult;
            stuQcRltInfo.enQcState = QC_STATE(stuData.qcStatu);
            stuQcRltInfo.strOutCtrlRule = qcRuleObj->TranseRuleCode(stuData.qcBreakRules);
            stuQcRltInfo.dQcTargetVal = stuData.targetValue;
            stuQcRltInfo.dQcSD = stuData.qcSD;
            stuQcRltInfo.strOutCtrlReason = QString::fromStdString(stuData.breakReason);
            stuQcRltInfo.strSolution = QString::fromStdString(stuData.solution);
            stuQcRltInfo.strOperator = QString::fromStdString(stuData.handler);

            // 压入列表
            outBuffer.push_back(stuQcRltInfo);

            // 索引自增
            ++iIdx;
        }

        return true;
    }

    return true;
}

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
/// @file     devgeneral.cpp
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
#include "devgeneral.h"
#include "adaptertypedef.h"
#include "shared/DataManagerQc.h"
#include "thrift/DcsControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

DevGeneral::DevGeneral()
{
}

DevGeneral::~DevGeneral()
{

}

///
/// @brief
///     注册通用设备
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月12日，新建函数
///
bool DevGeneral::RegisterDevice()
{
    // 注册设备
    return DYNAMIC_REGISTER_DEVICE(DevGeneral, "", "");
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
bool DevGeneral::OperateData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    bool bRet = false;
    switch (iDataCode)
    {
    case DATA_CODE_QC_DOC:
        bRet = OperateQcDocData(iDataOpCode, paramList, outBuffer);
        break;
    case DATA_CODE_QC_APPLY:
    case DATA_CODE_DEFAULT_QC:
    case DATA_CODE_BACKUP_RGNT_QC:
        bRet = OperateQcApplyData(iDataCode, iDataOpCode, paramList, outBuffer);
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
bool DevGeneral::ExcuteCmd(int iCmdCode, const std::list<boost::any>& paramList, int& iRetCode)
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
int DevGeneral::DeviceClass()
{
    return -1;
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
int DevGeneral::DeviceType()
{
    return -1;
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
QString DevGeneral::DeviceSn()
{
    return QString("");
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
int DevGeneral::ModuleCount()
{
    return 0;
}

///
/// @bref
///		试剂仓位数
///
/// @par History:
/// @li 8276/huchunli, 2022年12月28日，新建函数
///
int DevGeneral::ReagentSlotNumber()
{
    return -1;
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
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
bool DevGeneral::OperateQcDocData(int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    auto IsIseDoc = [](const tf::QcDoc& stuQcDoc)
    {
        bool bContainIseAssay = false;
        bool bContainOtherAssay = false;
        for (const auto& stuQcRgnt : stuQcDoc.compositions)
        {
            if (stuQcRgnt.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_NA)
            {
                bContainIseAssay = true;
                continue;
            }

            if (stuQcRgnt.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_K)
            {
                bContainIseAssay = true;
                continue;
            }

            if (stuQcRgnt.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
            {
                bContainIseAssay = true;
                continue;
            }

            return false;
        }

        return bContainIseAssay;
    };

    // 清空缓冲区
    outBuffer.clear();

    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 解析参数
        QList<tf::AssayClassify::type> devClassifyTypeLst;
        for (const auto& inData : paramList)
        {
            devClassifyTypeLst.push_back(boost::any_cast<tf::AssayClassify::type>(inData));
        }

        // 判断是否屏蔽ISE质控品
        bool bIseMask = devClassifyTypeLst.contains(tf::AssayClassify::ASSAY_CLASSIFY_ISE);

        // 查询质控组合信息
        std::vector<std::shared_ptr<tf::QcDoc>> allQcDocs;
        DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);

        // 结果放入输出缓冲区
        for (auto spQcDoc : allQcDocs)
        {
            // 参数检查
            if (spQcDoc == Q_NULLPTR)
            {
                continue;
            }
            auto& qcDoc = *spQcDoc;

            // 如果屏蔽ISE，则跳过ISE质控品
            if (bIseMask && IsIseDoc(qcDoc))
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
///     操作质控申请数据（包括默认质控和备用瓶质控增删查改等）
///
/// @param[in]  iDataCode    数据码
/// @param[in]  iDataOpCode  数据操作码
/// @param[in]  paramList    参数列表
/// @param[out] outBuffer    输出缓冲区
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月21日，新建函数
///
bool DevGeneral::OperateQcApplyData(int iDataCode, int iDataOpCode, const QList<boost::any>& paramList, std::list<boost::any>& outBuffer)
{
    // 查询蓑鲉设备
    if (iDataOpCode == int(DATA_OPERATE_TYPE_QUERY))
    {
        // 清空输出缓冲区
        outBuffer.clear();

        // 获取所有设备
        QList<QPair<QString, QString>> strDevNameList = DeviceFactory::GetAllDeviceName();

        // 逐个查询设备中的数据
        for (const auto& strDevName : strDevNameList)
        {
            // 构建输出缓冲区
            std::list<boost::any> outDevBuffer;
            std::shared_ptr<AbstractDevice> apIDev = DeviceFactory::GetDevice(strDevName.first, strDevName.second);
            if (apIDev == Q_NULLPTR)
            {
                continue;
            }
            apIDev->SetModuleNo(ModuleNo());

            // 调用设备操作数据接口
            apIDev->OperateData(iDataCode, iDataOpCode, paramList, outDevBuffer);

            // 将设备输出缓冲区追加到输出缓冲区中
            outBuffer.insert(outBuffer.end(), outDevBuffer.begin(), outDevBuffer.end());
        }
    }

    return true;
}

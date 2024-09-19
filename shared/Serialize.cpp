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
/// @file     Serializa.h
/// @brief    免疫打印数据序列化接口
///
/// @author   6889/ChenWei
/// @date     2023年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2023年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "Serialize.h"
#include "src/common/defs.h"
#include <rapidjson/filewritestream.h>
#include <QObject>
#include <QString>
#include <QMap>
std::shared_ptr<SerializerMap> SerializerMap::s_Instance = nullptr;

std::string Serializer::getFieldName(int iIndex)
{
    if (iIndex >= m_fields.size())
        return "";

    return m_fields.at(iIndex).getFieldName();
}

std::string Serializer::getDisplayName(int iIndex)
{
    if (iIndex >= m_fields.size())
        return "";

    return m_fields.at(iIndex).getDisplayName().toStdString();
}

std::string Serializer::getFieldType(int iIndex)
{
    if (iIndex >= m_fields.size())
        return "";

    return m_fields.at(iIndex).getType();
}

int Serializer::getFieldsCount()
{
    return int(m_fields.size());
}

///
///  @brief 序列化结构体
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
template <typename StructType>
void Serializer::Serialize(StructType& Item, rapidjson::Document& docJson)
{
    rapidjson::Document::AllocatorType& allocator = docJson.GetAllocator();
    for (int i = 0; i < m_fields.size(); i++)
    {
        std::string strFType = m_fields.at(i).getType();
        if (strFType == "std::string")
        {
            std::string strFieldName = m_fields.at(i).getFieldName();
            std::string strFieldValue = Item.*(m_fields.at(i).getFPtr<StructType, std::string>());
            rapidjson::Value Key(strFieldName.c_str(), allocator);
            rapidjson::Value Value(rapidjson::kStringType);
            Value.SetString(strFieldValue.c_str(), strFieldValue.size(), allocator);
            if (!Value.IsNull())
            {
                docJson.AddMember(Key, Value, allocator);
            }
        }
        else if (strFType == "BS")
        {
            // BS类型为图片字符串
            std::string strFieldName = m_fields.at(i).getFieldName();
            std::string strFieldValue = Item.*(m_fields.at(i).getFPtr<StructType, std::string>());
            rapidjson::Value Key(strFieldName.c_str(), allocator);
            rapidjson::Value Value(rapidjson::kStringType);
            Value.SetString(strFieldValue.c_str(), strFieldValue.size(), allocator);
            if (!Value.IsNull())
            {
                docJson.AddMember(Key, Value, allocator);
            }
        }
        else if (strFType == "std::vector<SampleExportInfo>")
        {
            Serialize_vec<SampleExportInfo>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<SampleExportInfo>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ItemInfo>")
        {
            Serialize_vec<ItemInfo>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ItemInfo>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ItemResult>")
        {
            Serialize_vec<ItemResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ItemResult>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ItemSampleResult>")
        {
            Serialize_vec<ItemSampleResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ItemSampleResult>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ReagentItem>")
        {
            Serialize_vec<ReagentItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ReagentItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<SupplyItem>")
        {
            Serialize_vec<SupplyItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<SupplyItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliResultData>")
        {
            Serialize_vec<CaliResultData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliResultData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliRLUInfo>")
        {
            Serialize_vec<CaliRLUInfo>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliRLUInfo>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliRack>")
        {
            Serialize_vec<CaliRack>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliRack>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliRecord>")
        {
            Serialize_vec<CaliRecord>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliRecord>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ChCaliRecord>")
        {
            Serialize_vec<ChCaliRecord>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCaliRecord>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<QCDailyResult>")
        {
            Serialize_vec<QCDailyResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<QCDailyResult>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<QCResult>")
        {
            Serialize_vec<QCResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<QCResult>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<QCRecord>")
        {
            Serialize_vec<QCRecord>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<QCRecord>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<OperationLogItem>")
        {
            Serialize_vec<OperationLogItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<OperationLogItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ChangeRecord>")
        {
            Serialize_vec<ChangeRecord>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChangeRecord>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<AlarmInfo>")
        {
            Serialize_vec<AlarmInfo>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<AlarmInfo>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<MaintenanceItem>")
        {
            Serialize_vec<MaintenanceItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<MaintenanceItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
		else if (strFType == "std::vector<ChCupTestResult>")
		{
			Serialize_vec<ChCupTestResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCupTestResult>>())), m_fields.at(i).getFieldName(), docJson);
		}
		else if (strFType == "std::vector<ChPhotoCheckResult>")
		{
			Serialize_vec<ChPhotoCheckResult>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChPhotoCheckResult>>())), m_fields.at(i).getFieldName(), docJson);
		}
        else if (strFType == "std::vector<CupHistoryItem>")
        {
            Serialize_vec<CupHistoryItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CupHistoryItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<LiquidItem>")
        {
            Serialize_vec<LiquidItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<LiquidItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<TemperatureInfo>")
        {
            Serialize_vec<TemperatureInfo>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<TemperatureInfo>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<StatisticalUnit>")
        {
            Serialize_vec<StatisticalUnit>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<StatisticalUnit>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<LowerCpUnit>")
        {
            Serialize_vec<LowerCpUnit>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<LowerCpUnit>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<SampleStatistics>")
        {
            Serialize_vec<SampleStatistics>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<SampleStatistics>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<SampleStatisticsSum>")
        {
            Serialize_vec<SampleStatisticsSum>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<SampleStatisticsSum>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ReagentStatistics>")
        {
            Serialize_vec<ReagentStatistics>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ReagentStatistics>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliStatistics>")
        {
            Serialize_vec<CaliStatistics>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliStatistics>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ChCaliParameters>")
        {
            Serialize_vec<ChCaliParameters>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCaliParameters>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ChCaliResultData>")
        {
            Serialize_vec<ChCaliResultData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCaliResultData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<IseCaliResultData>")
        {
            Serialize_vec<IseCaliResultData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<IseCaliResultData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<CaliCurveLvData>")
        {
            Serialize_vec<CaliCurveLvData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliCurveLvData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<ChCaliHisLvData>")
        {
            Serialize_vec<ChCaliHisLvData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCaliHisLvData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<IseCaliHisLvData>")
        {
            Serialize_vec<IseCaliHisLvData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<IseCaliHisLvData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "std::vector<RequireCalcItem>")
        {
            Serialize_vec<RequireCalcItem>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<RequireCalcItem>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "grp<ChCaliHisTimesData>")
        {
            Serialize_grp<ChCaliHisTimesData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<ChCaliHisTimesData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "grp<CaliCurveTimesData>")
        {
            Serialize_grp<CaliCurveTimesData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<CaliCurveTimesData>>())), m_fields.at(i).getFieldName(), docJson);
        }
        else if (strFType == "grp<IseCaliHisTimesData>")
        {
            Serialize_grp<IseCaliHisTimesData>((Item.*(m_fields.at(i).getFPtr<StructType, std::vector<IseCaliHisTimesData>>())), m_fields.at(i).getFieldName(), docJson);
        }
    }
}

///
///  @brief 注册字段信息
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
template <typename StructType, typename FieldType>
void Serializer::RegisterField(const std::string& Type_, const std::string& name_, const QString& disname_,FieldType StructType::*pointer)
{
    AnyField f1(Type_, name_, disname_, pointer);
    m_fields.push_back(f1);
}

///
///  @brief 序列化数组
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
template<typename StructType>
void Serialize_vec(std::vector<StructType, std::allocator<StructType>>& vecItem, std::string strName, rapidjson::Document& docJson)
{
    std::shared_ptr<Serializer> pSerializer = SerializerMap::GetInstance()->getSerializer(typeid(StructType).name());
    if (pSerializer == nullptr)
        return;

    rapidjson::Document::AllocatorType& allocator = docJson.GetAllocator();
    rapidjson::Value Vector(rapidjson::kObjectType);
    rapidjson::Value HeardArray(rapidjson::kArrayType);
    rapidjson::Value FieldNameValue(rapidjson::kStringType);

    // 生成表头
    int iFCount = pSerializer->getFieldsCount();
    for (int i = 0; i < iFCount; i++)
    {
        std::string strName = pSerializer->getFieldName(i);
        FieldNameValue.SetString(strName.c_str(), strName.size(), allocator);
        HeardArray.PushBack(FieldNameValue, allocator);
    }

    // 添加数组值
    rapidjson::Value ItmeArray(rapidjson::kArrayType);
    for (int i = 0; i < vecItem.size(); i++)
    {
        // 生成数据字符串
        rapidjson::Value ItmeValues(rapidjson::kArrayType);
        StructType& item = vecItem.at(i);
        for (int j = 0; j < iFCount; j++)
        {
            if (pSerializer->getFieldType(j) == "int")
            {
                int iValue = item.*(pSerializer->FieldPtr<StructType, int>(j));
                ItmeValues.PushBack(iValue, allocator);
            }
            else if (pSerializer->getFieldType(j) == "std::string")
            {
                std::string strValue = item.*(pSerializer->FieldPtr<StructType, std::string>(j));
                rapidjson::Value Value(rapidjson::kStringType);
                Value.SetString(strValue.c_str(), rapidjson::SizeType(strValue.size()), allocator);
                ItmeValues.PushBack(Value, allocator);
            }
            else if (pSerializer->getFieldType(j) == "QString")
            {
                QString strValue = item.*(pSerializer->FieldPtr<StructType, QString>(j));
                std::string strUtf8 = strValue.toUtf8().data();
                rapidjson::Value Value(rapidjson::kStringType);
                Value.SetString(strUtf8.c_str(), rapidjson::SizeType(strUtf8.size()), allocator);
                ItmeValues.PushBack(Value, allocator);
            }
        }

        ItmeArray.PushBack(ItmeValues, allocator);
    }

    // 添加数组表头和值
    Vector.AddMember("header", HeardArray, allocator);
    Vector.AddMember("items", ItmeArray, allocator);

    rapidjson::Value Key(strName.c_str(), allocator);
    docJson.AddMember(Key, Vector, allocator);
}

///
///  @brief 序列化组
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
template<typename StructType>
void Serialize_grp(std::vector<StructType, std::allocator<StructType>>& vecItem, std::string strName, rapidjson::Document& docJson)
{
    std::shared_ptr<Serializer> pSerializer = SerializerMap::GetInstance()->getSerializer(typeid(StructType).name());
    if (pSerializer == nullptr)
        return;

    rapidjson::Document::AllocatorType& allocator = docJson.GetAllocator();
    rapidjson::Value grpValue(rapidjson::kObjectType);

    auto findItr = [&](const rapidjson::Value& value, const std::string& name) {
        for (auto itr = grpValue.MemberBegin(); itr != grpValue.MemberEnd(); ++itr)
        {
            auto str = itr->name.GetString();
            if (str != name)
                continue;

            return itr;
        }

        return grpValue.MemberEnd();
    };

    // 循环序列化对象数据
    for (int i = 0; i < vecItem.size(); i++)
    {
        // 初始化临时对象，用于序列化对象，可以隔离数据
        rapidjson::Document tempDocJson;
        tempDocJson.SetObject();
        StructType& item = vecItem.at(i);
        pSerializer->Serialize<StructType>(item, tempDocJson);

        // 从临时文档中获取数据，放到节点中
        for (auto itr = tempDocJson.MemberBegin(); 
            itr != tempDocJson.MemberEnd(); ++itr)
        {
            auto str = itr->name.GetString();
            auto nameItr = findItr(grpValue, str);
            
            // object类型是一个数组
            if (itr->value.GetType() == rapidjson::kObjectType)
            {
                // 没有就需要新增
                if (nameItr == grpValue.MemberEnd())
                {
                    for (auto innerItr = itr->value.MemberBegin();
                        innerItr != itr->value.MemberEnd(); ++innerItr)
                    {
                        const std::string& strKeyName = innerItr->name.GetString();
                        if (strKeyName != "items")
                            continue;

                        // 生成一个新的，支持array
                        rapidjson::Value cpKey;
                        rapidjson::Value cpValue;
                        rapidjson::Value valueArray(rapidjson::kArrayType);

                        cpKey.CopyFrom(innerItr->name, allocator);
                        cpValue.CopyFrom(innerItr->value, allocator);

                        valueArray.PushBack(cpValue, allocator);
                        itr->value.RemoveMember(innerItr);
                        itr->value.AddMember(cpKey, valueArray, allocator);
                        break;
                    }


                    rapidjson::Value cpKey;
                    rapidjson::Value cpValue;
                    cpKey.CopyFrom(itr->name, allocator);
                    cpValue.CopyFrom(itr->value, allocator);
                    // 先放入object
                    grpValue.AddMember(cpKey, cpValue, allocator);
                }
                else
                {
                    for (auto innerItr = itr->value.MemberBegin();
                        innerItr != itr->value.MemberEnd(); ++innerItr)
                    {
                        const std::string& strKeyName = innerItr->name.GetString();
                        if (strKeyName != "items")
                            continue;

                        auto keyItr = nameItr->value.FindMember(strKeyName.c_str());
                        if (keyItr != nameItr->value.MemberEnd())
                        {
                            rapidjson::Value cpValue;
                            cpValue.CopyFrom(innerItr->value, allocator);
                            keyItr->value.PushBack(cpValue, allocator);
                        }
                        else
                        {
                            // 生成一个新的，支持array
                            rapidjson::Value cpKey;
                            rapidjson::Value cpValue;
                            rapidjson::Value valueArray(rapidjson::kArrayType);

                            cpKey.CopyFrom(innerItr->name, allocator);
                            cpValue.CopyFrom(innerItr->value, allocator);

                            valueArray.PushBack(cpValue, allocator);
                            itr->value.RemoveMember(innerItr);
                            itr->value.AddMember(cpKey, valueArray, allocator);
                            break;
                        }
                    }
                }
            }
            else
            {
                rapidjson::Value cpKey;
                rapidjson::Value cpValue;
                cpKey.CopyFrom(itr->name, allocator);
                cpValue.CopyFrom(itr->value, allocator);

                // 没有就需要新增
                if (nameItr == grpValue.MemberEnd())
                {
                    rapidjson::Value valueArray(rapidjson::kArrayType);
                    //valueKey.SetString(str, str.GetStringLength(), allocator);
                    valueArray.PushBack(cpValue, allocator);
                    grpValue.AddMember(cpKey, valueArray, allocator);
                }
                // 有就直接加入
                else
                {
                    nameItr->value.PushBack(cpValue, allocator);
                }
            }
        }
    }

    rapidjson::Value Key(strName.c_str(), allocator);
    rapidjson::Value grpObject(rapidjson::kArrayType);
    grpObject.PushBack(grpValue, allocator);
    docJson.AddMember(Key, grpObject, allocator);
}

SerializerMap::SerializerMap(QObject *parent) 
    : QObject(parent)
{
    InitMap();
};

std::shared_ptr<SerializerMap> SerializerMap::GetInstance()
{
    // 获取实例对象
    if (nullptr == s_Instance)
    {
        s_Instance.reset(new SerializerMap());
    }

    return s_Instance;
}

std::shared_ptr<Serializer> SerializerMap::getSerializer(std::string TypeName_)
{
    auto iter = m_SerializerMap.find(TypeName_);
    if (iter != m_SerializerMap.end())
        return iter->second;
    else
        return nullptr;
}

///
///  @brief 初始化管理器，并注册需要序列化的结构体元信息
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
void SerializerMap::InitMap()
{
    std::map<std::string, std::shared_ptr<Serializer>>& SerMap = m_SerializerMap;
    
    // 样本信息导出模板
    {
        Serializer  SampleExportSer;
        SampleExportSer.RegisterField("QString", "strSampleNo", tr("样本号"), &SampleExportInfo::strSampleNo);
        SampleExportSer.RegisterField("QString", "strBarcode", tr("样本条码"), &SampleExportInfo::strBarcode);
        SampleExportSer.RegisterField("QString", "strSampleSourceType", tr("样本类型"), &SampleExportInfo::strSampleSourceType);
        SampleExportSer.RegisterField("QString", "strItemName", tr("项目名称"), &SampleExportInfo::strItemName);
        SampleExportSer.RegisterField("QString", "strTestResult", tr("检测结果"), &SampleExportInfo::strTestResult);
        SampleExportSer.RegisterField("QString", "strUnit", tr("单位"), &SampleExportInfo::strUnit);
        SampleExportSer.RegisterField("QString", "strRUL", tr("信号值"), &SampleExportInfo::strRUL);
        SampleExportSer.RegisterField("QString", "strEndTestTime", tr("检测完成时间"), &SampleExportInfo::strEndTestTime);
        SampleExportSer.RegisterField("QString", "strPreDilutionFactor", tr("手工稀释倍数"), &SampleExportInfo::strPreDilutionFactor);
        SampleExportSer.RegisterField("QString", "strDilutionFactor", tr("机内稀释倍数"), &SampleExportInfo::strDilutionFactor);
        SampleExportSer.RegisterField("QString", "strReagentBatchNo", tr("试剂批号"), &SampleExportInfo::strReagentBatchNo);
        SampleExportSer.RegisterField("QString", "strSubstrateBatchNo", tr("底物液批号"), &SampleExportInfo::strSubstrateBatchNo);
        SampleExportSer.RegisterField("QString", "strCleanFluidBatchNo", tr("清洗缓冲液批号"), &SampleExportInfo::strCleanFluidBatchNo);
        SampleExportSer.RegisterField("QString", "strAssayCupBatchNo", tr("反应杯批号"), &SampleExportInfo::strAssayCupBatchNo);
        SampleExportSer.RegisterField("QString", "strDiluentBatchNo", tr("稀释液批号"), &SampleExportInfo::strDiluentBatchNo);
        SampleExportSer.RegisterField("QString", "strRgtRegisterT", tr("试剂上机时间"), &SampleExportInfo::strRgtRegisterT);
        SampleExportSer.RegisterField("QString", "strReagentExpTime", tr("试剂失效日期"), &SampleExportInfo::strReagentExpTime);
        SampleExportSer.RegisterField("QString", "strCaliLot", tr("校准品批号"), &SampleExportInfo::strCaliLot);
        SampleExportSer.RegisterField("QString", "strFirstCurCaliTime", tr("当前工作曲线"), &SampleExportInfo::strFirstCurCaliTime);
        
        SerMap[std::string(typeid(SampleExportInfo).name())] = std::make_shared<Serializer>(SampleExportSer);

        Serializer  SampleExportModuleSer;
        SampleExportModuleSer.RegisterField("std::string", "strExportTime", tr("导出时间"), &SampleExportModule::strExportTime);
        SampleExportModuleSer.RegisterField("std::string", "strTitle", tr("原始结果记录"), &SampleExportModule::strTitle);
        SampleExportModuleSer.RegisterField("std::vector<SampleExportInfo>", "vecResults", tr("原始记录"), &SampleExportModule::vecResults);

        SerMap[typeid(SampleExportModule).name()] = std::make_shared<Serializer>(SampleExportModuleSer);

    }

    // 样本信息
    {
        // 项目信息
        Serializer ItemInfoSer;
        ItemInfoSer.RegisterField("std::string", "strIndex", tr("序号"), &ItemInfo::strIndex);
        ItemInfoSer.RegisterField("std::string", "strItemName", tr("项目名称"), &ItemInfo::strItemName);
        ItemInfoSer.RegisterField("std::string", "strShortName", tr("简称"), &ItemInfo::strShortName);
        ItemInfoSer.RegisterField("std::string", "strResult", tr("检测结果"), &ItemInfo::strResult);
        ItemInfoSer.RegisterField("std::string", "strUnit", tr("单位"), &ItemInfo::strUnit);
        ItemInfoSer.RegisterField("std::string", "strResultState", tr("数据报警"), &ItemInfo::strResultState);
        ItemInfoSer.RegisterField("std::string", "strRefRange", tr("参考范围"), &ItemInfo::strRefRange);
        ItemInfoSer.RegisterField("std::string", "strModule", tr("检测模块"), &ItemInfo::strModule);
        SerMap[typeid(ItemInfo).name()] = std::make_shared<Serializer>(ItemInfoSer);

        Serializer SpISer;
        SpISer.RegisterField("std::string", "strPatientName", tr("姓名:"), &SampleInfo::strPatientName);
        SpISer.RegisterField("std::string", "strGender", tr("性别:"), &SampleInfo::strGender);
        SpISer.RegisterField("std::string", "strAge", tr("年龄:"), &SampleInfo::strAge);
        SpISer.RegisterField("std::string", "strSourceType", tr("样本类型:"), &SampleInfo::strSourceType);
        SpISer.RegisterField("std::string", "strSequenceNO", tr("样本号:"), &SampleInfo::strSequenceNO);
        SpISer.RegisterField("std::string", "strSampleID", tr("样本条码:"), &SampleInfo::strSampleID);
        SpISer.RegisterField("std::string", "strCaseNo", tr("病历号:"), &SampleInfo::strCaseNo);
        SpISer.RegisterField("std::string", "strDepartment", tr("科室:"), &SampleInfo::strDepartment);
        SpISer.RegisterField("std::string", "strDoctor", tr("主治医师:"), &SampleInfo::strDoctor);
        SpISer.RegisterField("std::string", "strBedNo", tr("病床号:"), &SampleInfo::strBedNo);
        SpISer.RegisterField("std::string", "strCompTime", tr("检测完成时间:"), &SampleInfo::strCompTime);
        SpISer.RegisterField("std::string", "strInpatientWard", tr("病区:"), &SampleInfo::strInpatientWard);
        SpISer.RegisterField("std::string", "strClinicalDiagnosis", tr("临床诊断:"), &SampleInfo::strClinicalDiagnosis);
        SpISer.RegisterField("std::string", "strInspectionPersonnel", tr("检测者:"), &SampleInfo::strInspectionPersonnel);
        SpISer.RegisterField("std::string", "strDateAudit", tr("审核时间:"), &SampleInfo::strDateAudit);
        SpISer.RegisterField("std::string", "strReviewers", tr("审核者:"), &SampleInfo::strReviewers);
        SpISer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &SampleInfo::strInstrumentModel);
        SpISer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &SampleInfo::strPrintTime);
        SpISer.RegisterField("std::string", "strTitle", tr("检测报告"), &SampleInfo::strTitle);
        SpISer.RegisterField("std::string", "strNotes", tr("备注"), &SampleInfo::strNotes);
        SpISer.RegisterField("std::string", "strStateMent", tr("本报告仅对本标本负责，结果供医师参考，如有疑问，请及时联系"), &SampleInfo::strStateMent);
        SpISer.RegisterField("std::vector<ItemInfo>", "vecItems", tr("测试项目"), &SampleInfo::vecItems);

        SerMap[typeid(SampleInfo).name()] = std::make_shared<Serializer>(SpISer);
    }

    // 按样本打印
    {
        // 项目结果
        Serializer ItemResultSer;
        ItemResultSer.RegisterField("std::string", "strSequenceNO", tr("样本号"), &ItemResult::strSequenceNO);
        ItemResultSer.RegisterField("std::string", "strSampleID", tr("样本条码"), &ItemResult::strSampleID);
        ItemResultSer.RegisterField("std::string", "strItemName", tr("项目名称"), &ItemResult::strItemName);
        ItemResultSer.RegisterField("std::string", "strResult", tr("检测结果"), &ItemResult::strResult);
        ItemResultSer.RegisterField("std::string", "strCompTime", tr("检测完成时间"), &ItemResult::strCompTime);
        ItemResultSer.RegisterField("std::string", "strModule", tr("检测模块"), &ItemResult::strModule);
        SerMap[typeid(ItemResult).name()] = std::make_shared<Serializer>(ItemResultSer);
        
        Serializer SimpleSampleInfoSer;
        SimpleSampleInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号："), &SimpleSampleInfo::strInstrumentModel);
        SimpleSampleInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &SimpleSampleInfo::strPrintTime);
        SimpleSampleInfoSer.RegisterField("std::string", "strTitle", tr("样本结果"), &SimpleSampleInfo::strTitle);
        SimpleSampleInfoSer.RegisterField("std::vector<ItemResult>", "vecItemResults", tr("检测项目"), &SimpleSampleInfo::vecItemResults);

        SerMap[typeid(SimpleSampleInfo).name()] = std::make_shared<Serializer>(SimpleSampleInfoSer);
    }

    // 按项目打印
    {
        // 项目样本结果
        Serializer ItemSampleResultSer;
        ItemSampleResultSer.RegisterField("std::string", "strSampleID", tr("样本条码"), &ItemSampleResult::strSampleID);
        ItemSampleResultSer.RegisterField("std::string", "strSampleNumber", tr("样本号"), &ItemSampleResult::strSampleNumber);
        ItemSampleResultSer.RegisterField("std::string", "strItemName", tr("项目名称"), &ItemSampleResult::strItemName);
        ItemSampleResultSer.RegisterField("std::string", "strResult", tr("检测结果"), &ItemSampleResult::strResult);
        ItemSampleResultSer.RegisterField("std::string", "strUnit", tr("单位"), &ItemSampleResult::strUnit);
        ItemSampleResultSer.RegisterField("std::string", "strRLU", tr("信号值"), &ItemSampleResult::strRLU);
        ItemSampleResultSer.RegisterField("std::string", "strDetectionTime", tr("检测完成时间"), &ItemSampleResult::strDetectionTime);
        //ItemSampleResultSer.RegisterField("std::string", "strReagentLot", tr("试剂批号"), &ItemSampleResult::strReagentLot);
        //ItemSampleResultSer.RegisterField("std::string", "strReagentValidityDate", tr("试剂有效期"), &ItemSampleResult::strReagentValidityDate);
        //ItemSampleResultSer.RegisterField("std::string", "strModule", tr("检测模块"), &ItemSampleResult::strModule);

        SerMap[typeid(ItemSampleResult).name()] = std::make_shared<Serializer>(ItemSampleResultSer);

        Serializer ItemSampleInfoSer;
        ItemSampleInfoSer.RegisterField("std::string", "strFactory", tr("试剂厂家"), &ItemSampleInfo::strFactory);
        ItemSampleInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &ItemSampleInfo::strInstrumentModel);
        ItemSampleInfoSer.RegisterField("std::string", "strTemperature", tr("实验室温度:"), &ItemSampleInfo::strTemperature);
        ItemSampleInfoSer.RegisterField("std::string", "strHumidness", tr("实验室湿度:"), &ItemSampleInfo::strHumidness);
        ItemSampleInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ItemSampleInfo::strPrintTime);
        ItemSampleInfoSer.RegisterField("std::string", "strTitle", tr("项目结果"), &ItemSampleInfo::strTitle);
        ItemSampleInfoSer.RegisterField("std::vector<ItemSampleResult>", "vecResult", tr("项目结果"), &ItemSampleInfo::vecResult);

        SerMap[typeid(ItemSampleInfo).name()] = std::make_shared<Serializer>(ItemSampleInfoSer);
    }

    // 试剂信息
    {
        Serializer ReagentItemSer;
        ReagentItemSer.RegisterField("std::string", "strReagentPos", tr("试剂位"), &ReagentItem::strReagentPos);
        ReagentItemSer.RegisterField("std::string", "strReagentName", tr("名称"), &ReagentItem::strReagentName);
        ReagentItemSer.RegisterField("std::string", "strValidityTestNum", tr("可用测试数"), &ReagentItem::strValidityTestNum);
        ReagentItemSer.RegisterField("std::string", "strResidualTestNum", tr("剩余测试数"), &ReagentItem::strResidualTestNum);
        ReagentItemSer.RegisterField("std::string", "strUseStatus", tr("使用状态"), &ReagentItem::strUseStatus);
        ReagentItemSer.RegisterField("std::string", "strCaliStatus", tr("校准状态"), &ReagentItem::strCaliStatus);
        ReagentItemSer.RegisterField("std::string", "strReagentLot", tr("试剂批号"), &ReagentItem::strReagentLot);
        ReagentItemSer.RegisterField("std::string", "strReagentSN", tr("瓶号"), &ReagentItem::strReagentSN);
        ReagentItemSer.RegisterField("std::string", "strCaliValidityDays", tr("当前曲线有效期"), &ReagentItem::strCaliValidityDays);
        ReagentItemSer.RegisterField("std::string", "strBottleValidityDays", tr("开瓶有效期(天)"), &ReagentItem::strBottleValidityDays);
        //ReagentItemSer.RegisterField("std::string", "strReagentValidityDays", tr("试剂有效天数"), &ReagentItem::strReagentValidityDays);
        ReagentItemSer.RegisterField("std::string", "strRegisterDate", tr("上机时间"), &ReagentItem::strRegisterDate);
        ReagentItemSer.RegisterField("std::string", "strReagentValidityDate", tr("失效日期"), &ReagentItem::strReagentValidityDate);
        SerMap[typeid(ReagentItem).name()] = std::make_shared<Serializer>(ReagentItemSer);

        Serializer ReagentInfoSer;
        ReagentInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号："), &ReagentInfo::strInstrumentModel);
        ReagentInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间："), &ReagentInfo::strPrintTime);
        ReagentInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间："), &ReagentInfo::strPrintTime);
        ReagentInfoSer.RegisterField("std::string", "strTitle", tr("试剂信息"), &ReagentInfo::strTitle);
        ReagentInfoSer.RegisterField("std::vector<ReagentItem>", "vecReagent", tr("试剂信息"), &ReagentInfo::vecReagent);

        SerMap[typeid(ReagentInfo).name()] = std::make_shared<Serializer>(ReagentInfoSer);
    }

    // 需求计算
    {
        Serializer RequireCalcItemSer;
        RequireCalcItemSer.RegisterField("std::string", "strName", tr("名称"), &RequireCalcItem::strName);
        RequireCalcItemSer.RegisterField("std::string", "strModuleRemain", tr("模块余量分布"), &RequireCalcItem::strModuleRemain);
        RequireCalcItemSer.RegisterField("std::string", "strDynamicCalc", tr("动态计算"), &RequireCalcItem::strDynamicCalc);
        RequireCalcItemSer.RegisterField("std::string", "strReqVol", tr("需求量"), &RequireCalcItem::strReqVol);
        RequireCalcItemSer.RegisterField("std::string", "strRemainVol", tr("当前余量"), &RequireCalcItem::strRemainVol);
        RequireCalcItemSer.RegisterField("std::string", "strReqLack", tr("需求缺口"), &RequireCalcItem::strReqLack);
        RequireCalcItemSer.RegisterField("std::string", "strBottleSpeciffic", tr("瓶规格"), &RequireCalcItem::strBottleSpeciffic);
        RequireCalcItemSer.RegisterField("std::string", "strReqBottleCount", tr("需载入瓶数"), &RequireCalcItem::strReqBottleCount);
        SerMap[typeid(RequireCalcItem).name()] = std::make_shared<Serializer>(RequireCalcItemSer);

        Serializer RequireCalcInfoSer;
        RequireCalcInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &RequireCalcInfo::strPrintTime);
        RequireCalcInfoSer.RegisterField("std::string", "strTitle", tr("需求计算"), &RequireCalcInfo::strTitle);
        RequireCalcInfoSer.RegisterField("std::vector<RequireCalcItem>", "ItemVector", tr("需求计算"), &RequireCalcInfo::ItemVector);
        SerMap[typeid(RequireCalcInfo).name()] = std::make_shared<Serializer>(RequireCalcInfoSer);
    }

    // 耗材信息
    {
        Serializer SupplyItemSer;
        SupplyItemSer.RegisterField("std::string", "strModule", tr("模块"), &SupplyItem::strModule);
        SupplyItemSer.RegisterField("std::string", "strType", tr("名称"), &SupplyItem::strType);
        SupplyItemSer.RegisterField("std::string", "strUsageStatus", tr("使用状态"), &SupplyItem::strUsageStatus);
        SupplyItemSer.RegisterField("std::string", "strRemainQuantity", tr("余量"), &SupplyItem::strRemainQuantity);
        SupplyItemSer.RegisterField("std::string", "strLotNumber", tr("耗材批号"), &SupplyItem::strLotNumber);
        SupplyItemSer.RegisterField("std::string", "strSerialNumber", tr("瓶号/序列号"), &SupplyItem::strSerialNumber);
        SupplyItemSer.RegisterField("std::string", "strOpenEffectDays", tr("开瓶有效期(天)"), &SupplyItem::strOpenEffectDays);
        SupplyItemSer.RegisterField("std::string", "strLoadDate", tr("上机时间"), &SupplyItem::strLoadDate);
        SupplyItemSer.RegisterField("std::string", "strExpirationDate", tr("失效日期"), &SupplyItem::strExpirationDate);

        SerMap[typeid(SupplyItem).name()] = std::make_shared<Serializer>(SupplyItemSer);

        Serializer SupplyInfoSer;
        SupplyInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号："), &SupplyInfo::strInstrumentModel);
        SupplyInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间："), &SupplyInfo::strPrintTime);
        SupplyInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间："), &SupplyInfo::strPrintTime);
        SupplyInfoSer.RegisterField("std::string", "strTitle", tr("耗材信息"), &SupplyInfo::strTitle);
        SupplyInfoSer.RegisterField("std::vector<SupplyItem>", "vecSupply", tr("耗材信息"), &SupplyInfo::vecSupply);

        SerMap[typeid(SupplyInfo).name()] = std::make_shared<Serializer>(SupplyInfoSer);
    }

    // 校准结果数据
    {
        Serializer CaliResultDataSer;
        CaliResultDataSer.RegisterField("std::string", "strLevel", tr("校准品水平"), &CaliResultData::strLevel);
        CaliResultDataSer.RegisterField("std::string", "strSign", tr("信号值"), &CaliResultData::strSign);
        CaliResultDataSer.RegisterField("std::string", "strConc", tr("浓度"), &CaliResultData::strConc);

        SerMap[typeid(CaliResultData).name()] = std::make_shared<Serializer>(CaliResultDataSer);
    }

    // 校准品测试信息
    {
        Serializer CaliRLUInfoSer;
        CaliRLUInfoSer.RegisterField("std::string", "strCalibrator", tr("校准品"), &CaliRLUInfo::strCalibrator);
        CaliRLUInfoSer.RegisterField("std::string", "strConc", tr("浓度"), &CaliRLUInfo::strConc);
        CaliRLUInfoSer.RegisterField("std::string", "strRLU1", tr("RLU1"), &CaliRLUInfo::strRLU1);
        CaliRLUInfoSer.RegisterField("std::string", "strRLU2", tr("RLU2"), &CaliRLUInfo::strRLU2);
        CaliRLUInfoSer.RegisterField("std::string", "strRLU", tr("RLU"), &CaliRLUInfo::strRLU);
        CaliRLUInfoSer.RegisterField("std::string", "strCV", tr("CV"), &CaliRLUInfo::strCV);
        CaliRLUInfoSer.RegisterField("std::string", "strK1", tr("K1"), &CaliRLUInfo::strK1);
        CaliRLUInfoSer.RegisterField("std::string", "strK2", tr("K2"), &CaliRLUInfo::strK2);
        CaliRLUInfoSer.RegisterField("std::string", "strK", tr("K"), &CaliRLUInfo::strK);

        SerMap[typeid(CaliRLUInfo).name()] = std::make_shared<Serializer>(CaliRLUInfoSer);
    }

    // 校准结果信息
    {
        Serializer CaliResultInfoSer;
        CaliResultInfoSer.RegisterField("std::string", "strReagentName", tr("项目名称:"), &CaliResultInfo::strName);
        CaliResultInfoSer.RegisterField("std::string", "strDeviceNum", tr("仪器编号:"), &CaliResultInfo::strDeviceNum);
        CaliResultInfoSer.RegisterField("std::string", "strSoftVersion", tr("软件版本号:"), &CaliResultInfo::strSoftVersion);
        CaliResultInfoSer.RegisterField("std::string", "strCalibrateDate", tr("校准时间:"), &CaliResultInfo::strCalibrateDate);

        CaliResultInfoSer.RegisterField("std::string", "strModelName", tr("模块:"), &CaliResultInfo::strModelName);
        CaliResultInfoSer.RegisterField("std::string", "strReagentLot", tr("试剂批号:"), &CaliResultInfo::strReagentLot);
        CaliResultInfoSer.RegisterField("std::string", "strReagentSN", tr("试剂瓶号:"), &CaliResultInfo::strReagentSN);
        CaliResultInfoSer.RegisterField("std::string", "strCalibratorLot", tr("校准品批号:"), &CaliResultInfo::strCalibratorLot);
        CaliResultInfoSer.RegisterField("std::string", "strCalibratorPos", tr("校准品位置:"), &CaliResultInfo::strCalibratorPos);

        CaliResultInfoSer.RegisterField("std::string", "strSubstrateLot", tr("底物液批号:"), &CaliResultInfo::strSubstrateLot);
        CaliResultInfoSer.RegisterField("std::string", "strSubstrateSN", tr("底物液瓶号:"), &CaliResultInfo::strSubstrateSN);
        CaliResultInfoSer.RegisterField("std::string", "strCleanFluidLot", tr("清洗缓冲液批号:"), &CaliResultInfo::strCleanFluidLot);
        CaliResultInfoSer.RegisterField("std::string", "strCleanFluidSN", tr("清洗缓冲液瓶号:"), &CaliResultInfo::strCleanFluidSN);
        CaliResultInfoSer.RegisterField("std::string", "strCupLot", tr("反应杯批号:"), &CaliResultInfo::strCupLot);
        CaliResultInfoSer.RegisterField("std::string", "strCupSN", tr("反应杯序列号:"), &CaliResultInfo::strCupSN);
        CaliResultInfoSer.RegisterField("std::string", "strCutoff", tr("cutoff值:"), &CaliResultInfo::strCutoff);
        CaliResultInfoSer.RegisterField("std::string", "strAlarm", tr("数据报警:"), &CaliResultInfo::strAlarm);
        CaliResultInfoSer.RegisterField("std::string", "strCaliResult", tr("校准结果:"), &CaliResultInfo::strCaliResult);
        CaliResultInfoSer.RegisterField("std::string", "strOperater", tr("检测者:"), &CaliResultInfo::strOperater);
        CaliResultInfoSer.RegisterField("std::string", "strCaliCurve", tr("校准曲线"), &CaliResultInfo::strCaliCurve);
        CaliResultInfoSer.RegisterField("BS", "strCaliCurveImage", tr("校准曲线(图片)"), &CaliResultInfo::strCaliCurveImage);
        CaliResultInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &CaliResultInfo::strPrintTime);
        CaliResultInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &CaliResultInfo::strPrintTime);
        CaliResultInfoSer.RegisterField("std::string", "strTitle", tr("校准结果"), &CaliResultInfo::strTitle);

        CaliResultInfoSer.RegisterField("std::vector<CaliResultData>", "vecCaliResultData", tr("校准水平"), &CaliResultInfo::vecCaliResultData);
        CaliResultInfoSer.RegisterField("std::vector<CaliRLUInfo>", "vecCaliRLUInfo", tr("校准结果"), &CaliResultInfo::vecCaliRLUInfo);

        SerMap[typeid(CaliResultInfo).name()] = std::make_shared<Serializer>(CaliResultInfoSer);
    }

    // 校准架概况
    {
        Serializer CaliRackSer;
        CaliRackSer.RegisterField("std::string", "strItemName", tr("项目名称"), &CaliRack::strItemName);
        CaliRackSer.RegisterField("std::string", "strCaliName", tr("校准品名称"), &CaliRack::strCaliName);
        CaliRackSer.RegisterField("std::string", "strCaliLot", tr("校准品批号"), &CaliRack::strCaliLot);
        CaliRackSer.RegisterField("std::string", "strExpirationDate", tr("失效日期"), &CaliRack::strExpirationDate);
        CaliRackSer.RegisterField("std::string", "strCalibrator1", tr("校准品1"), &CaliRack::strCalibrator1);
        CaliRackSer.RegisterField("std::string", "strPos1", tr("位置1"), &CaliRack::strPos1);
        CaliRackSer.RegisterField("std::string", "strCalibrator2", tr("校准品2"), &CaliRack::strCalibrator2);
        CaliRackSer.RegisterField("std::string", "strPos2", tr("位置2"), &CaliRack::strPos2);
        CaliRackSer.RegisterField("std::string", "strCalibrator3", tr("校准品3"), &CaliRack::strCalibrator3);
        CaliRackSer.RegisterField("std::string", "strPos3", tr("位置3"), &CaliRack::strPos3);
        CaliRackSer.RegisterField("std::string", "strCalibrator4", tr("校准品4"), &CaliRack::strCalibrator4);
        CaliRackSer.RegisterField("std::string", "strPos4", tr("位置4"), &CaliRack::strPos4);
        CaliRackSer.RegisterField("std::string", "strCalibrator5", tr("校准品5"), &CaliRack::strCalibrator5);
        CaliRackSer.RegisterField("std::string", "strPos5", tr("位置5"), &CaliRack::strPos5);
        CaliRackSer.RegisterField("std::string", "strCalibrator6", tr("校准品6"), &CaliRack::strCalibrator6);
        CaliRackSer.RegisterField("std::string", "strPos6", tr("位置6"), &CaliRack::strPos6);

        SerMap[typeid(CaliRack).name()] = std::make_shared<Serializer>(CaliRackSer);

        Serializer CaliRackInfoSer;
        CaliRackInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号"), &CaliRackInfo::strInstrumentModel);
        CaliRackInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间："), &CaliRackInfo::strPrintTime);
        CaliRackInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间："), &CaliRackInfo::strPrintTime);
        CaliRackInfoSer.RegisterField("std::string", "strTitle", tr("校准架概况"), &CaliRackInfo::strTitle);
        CaliRackInfoSer.RegisterField("std::vector<CaliRack>", "vecCaliRack", tr("校准结果"), &CaliRackInfo::vecCaliRack);
        SerMap[typeid(CaliRackInfo).name()] = std::make_shared<Serializer>(CaliRackInfoSer);
    }

    // 校准历史
    {
        Serializer CaliHistoryInfoSer;
        CaliHistoryInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &CaliHistoryInfo::strInstrumentModel);
        CaliHistoryInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &CaliHistoryInfo::strPrintTime);
        CaliHistoryInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &CaliHistoryInfo::strPrintTime);
        CaliHistoryInfoSer.RegisterField("std::string", "strTitle", tr("校准历史"), &CaliHistoryInfo::strTitle);
        CaliHistoryInfoSer.RegisterField("std::string", "strFailureReason", tr("校准失败原因"), &CaliHistoryInfo::strFailureReason);

        CaliHistoryInfoSer.RegisterField("std::string", "strName", tr("项目名称"), &CaliHistoryInfo::strName);
        CaliHistoryInfoSer.RegisterField("std::string", "strCalibrateDate", tr("校准时间"), &CaliHistoryInfo::strCalibrateDate);
        CaliHistoryInfoSer.RegisterField("std::string", "strModelName", tr("模块"), &CaliHistoryInfo::strModelName);
        CaliHistoryInfoSer.RegisterField("std::string", "strCalibratorLot", tr("校准品批号"), &CaliHistoryInfo::strCalibratorLot);
        CaliHistoryInfoSer.RegisterField("std::string", "strReagentLot", tr("试剂批号"), &CaliHistoryInfo::strReagentLot);
        CaliHistoryInfoSer.RegisterField("std::string", "strReagentSN", tr("试剂瓶号"), &CaliHistoryInfo::strReagentSN);
        CaliHistoryInfoSer.RegisterField("std::string", "strRegisterDate", tr("试剂上机时间"), &CaliHistoryInfo::strRegisterDate);
        CaliHistoryInfoSer.RegisterField("std::string", "strSubstrateLot", tr("底物液批号"), &CaliHistoryInfo::strSubstrateLot);
        CaliHistoryInfoSer.RegisterField("std::string", "strSubstrateSN", tr("底物液瓶号"), &CaliHistoryInfo::strSubstrateSN);
        CaliHistoryInfoSer.RegisterField("std::string", "strCleanFluidLot", tr("清洗缓冲液批号"), &CaliHistoryInfo::strCleanFluidLot);
        CaliHistoryInfoSer.RegisterField("std::string", "strCleanFluidSN", tr("清洗缓冲液瓶号"), &CaliHistoryInfo::strCleanFluidSN);
        CaliHistoryInfoSer.RegisterField("std::string", "strCupLot", tr("反应杯批号"), &CaliHistoryInfo::strCupLot);
        CaliHistoryInfoSer.RegisterField("std::string", "strCupSN", tr("反应杯序列号"), &CaliHistoryInfo::strCupSN);
        CaliHistoryInfoSer.RegisterField("std::string", "strCalibrator1", tr("校准品1"), &CaliHistoryInfo::strCalibrator1);
        CaliHistoryInfoSer.RegisterField("std::string", "strConc1", tr("Cal-1浓度:"), &CaliHistoryInfo::strConc1);
        CaliHistoryInfoSer.RegisterField("std::string", "strSignalValue1", tr("Cal-1主标信号值:"), &CaliHistoryInfo::strSignalValue1);
        CaliHistoryInfoSer.RegisterField("std::string", "strC1_RUL1", tr("Cal-1 RLU1:"), &CaliHistoryInfo::strC1_RUL1);
        CaliHistoryInfoSer.RegisterField("std::string", "strC1_RUL2", tr("Cal-1 RLU2:"), &CaliHistoryInfo::strC1_RUL2);
        CaliHistoryInfoSer.RegisterField("std::string", "strC1_RUL", tr("Cal-1 RLU:"), &CaliHistoryInfo::strC1_RUL);
        CaliHistoryInfoSer.RegisterField("std::string", "strCalibrator2", tr("校准品2"), &CaliHistoryInfo::strCalibrator2);
        CaliHistoryInfoSer.RegisterField("std::string", "strConc2", tr("Cal-2浓度:"), &CaliHistoryInfo::strConc2);
        CaliHistoryInfoSer.RegisterField("std::string", "strSignalValue2", tr("Cal-2主标信号值:"), &CaliHistoryInfo::strSignalValue2);
        CaliHistoryInfoSer.RegisterField("std::string", "strC2_RUL1", tr("Cal-2 RLU1:"), &CaliHistoryInfo::strC2_RUL1);
        CaliHistoryInfoSer.RegisterField("std::string", "strC2_RUL2", tr("Cal-2 RLU2:"), &CaliHistoryInfo::strC2_RUL2);
        CaliHistoryInfoSer.RegisterField("std::string", "strC2_RUL", tr("Cal-2 RLU:"), &CaliHistoryInfo::strC2_RUL);

        SerMap[typeid(CaliHistoryInfo).name()] = std::make_shared<Serializer>(CaliHistoryInfoSer);
    }

    // 校准设置记录
    {
        Serializer CaliRecordSer;
        CaliRecordSer.RegisterField("std::string", "strIndex", tr("序号"), &CaliRecord::strIndex);
        CaliRecordSer.RegisterField("std::string", "strCaliName", tr("校准品名称"), &CaliRecord::strCaliName);
        CaliRecordSer.RegisterField("std::string", "strCaliLot", tr("校准品批号"), &CaliRecord::strCaliLot);
        CaliRecordSer.RegisterField("std::string", "strExpirationDate", tr("失效日期"), &CaliRecord::strExpirationDate);
        CaliRecordSer.RegisterField("std::string", "strCalibrator1", tr("校准品1"), &CaliRecord::strCalibrator1);
        CaliRecordSer.RegisterField("std::string", "strCalibrator2", tr("校准品2"), &CaliRecord::strCalibrator2);
        CaliRecordSer.RegisterField("std::string", "strCalibrator3", tr("校准品3"), &CaliRecord::strCalibrator3);
        CaliRecordSer.RegisterField("std::string", "strCalibrator4", tr("校准品4"), &CaliRecord::strCalibrator4);
        CaliRecordSer.RegisterField("std::string", "strCalibrator5", tr("校准品5"), &CaliRecord::strCalibrator5);
        CaliRecordSer.RegisterField("std::string", "strCalibrator6", tr("校准品6"), &CaliRecord::strCalibrator6);
        SerMap[typeid(CaliRecord).name()] = std::make_shared<Serializer>(CaliRecordSer);

        Serializer CaliRecordInfoSer;
        CaliRecordInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号："), &CaliRecordInfo::strInstrumentModel);
        CaliRecordInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间："), &CaliRecordInfo::strPrintTime);
        CaliRecordInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间："), &CaliRecordInfo::strPrintTime);
        CaliRecordInfoSer.RegisterField("std::string", "strTitle", tr("校准品记录"), &CaliRecordInfo::strTitle);
        CaliRecordInfoSer.RegisterField("std::vector<CaliRecord>", "vecRecord", tr("校准品记录"), &CaliRecordInfo::vecRecord);

        SerMap[typeid(CaliRecordInfo).name()] = std::make_shared<Serializer>(CaliRecordInfoSer);
    }

    // 生化校准设置记录
    {
        Serializer CaliRecordSer;
        CaliRecordSer.RegisterField("std::string", "strIndex", QObject::tr("序号"), &ChCaliRecord::strIndex);
        CaliRecordSer.RegisterField("std::string", "strCaliName", QObject::tr("校准品名称"), &ChCaliRecord::strCaliName);
        CaliRecordSer.RegisterField("std::string", "strCaliLot", QObject::tr("校准品批号"), &ChCaliRecord::strCaliLot);
        CaliRecordSer.RegisterField("std::string", "strCaliAssays", QObject::tr("校准项目"), &ChCaliRecord::strCaliAssays);
        CaliRecordSer.RegisterField("std::string", "strExpirationDate", QObject::tr("失效日期"), &ChCaliRecord::strExpirationDate);
        CaliRecordSer.RegisterField("std::string", "strCalibrator1", QObject::tr("校准品1"), &ChCaliRecord::strCalibrator1);
        CaliRecordSer.RegisterField("std::string", "strCalibrator2", QObject::tr("校准品2"), &ChCaliRecord::strCalibrator2);
        CaliRecordSer.RegisterField("std::string", "strCalibrator3", QObject::tr("校准品3"), &ChCaliRecord::strCalibrator3);
        CaliRecordSer.RegisterField("std::string", "strCalibrator4", QObject::tr("校准品4"), &ChCaliRecord::strCalibrator4);
        CaliRecordSer.RegisterField("std::string", "strCalibrator5", QObject::tr("校准品5"), &ChCaliRecord::strCalibrator5);
        CaliRecordSer.RegisterField("std::string", "strCalibrator6", QObject::tr("校准品6"), &ChCaliRecord::strCalibrator6);
        SerMap[typeid(ChCaliRecord).name()] = std::make_shared<Serializer>(CaliRecordSer);

        Serializer CaliRecordInfoSer;
        CaliRecordInfoSer.RegisterField("std::string", "strInstrumentModel", QObject::tr("仪器型号："), &ChCaliRecordInfo::strInstrumentModel);
        CaliRecordInfoSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间："), &ChCaliRecordInfo::strPrintTime);
        CaliRecordInfoSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间："), &ChCaliRecordInfo::strPrintTime);
        CaliRecordInfoSer.RegisterField("std::string", "strTitle", QObject::tr("校准品记录"), &ChCaliRecordInfo::strTitle);
        CaliRecordInfoSer.RegisterField("std::vector<ChCaliRecord>", "vecRecord", QObject::tr("校准品记录"), &ChCaliRecordInfo::vecRecord);

        SerMap[typeid(ChCaliRecordInfo).name()] = std::make_shared<Serializer>(CaliRecordInfoSer);
    }

    // 单日质控
    {
        // 单日质控结果
        Serializer QCDailyResultSer;
        QCDailyResultSer.RegisterField("std::string", "strQCDate", tr("质控时间"), &QCDailyResult::strQCDate);
        QCDailyResultSer.RegisterField("std::string", "strQCID", tr("质控品编号"), &QCDailyResult::strQCID);
        QCDailyResultSer.RegisterField("std::string", "strQCName", tr("质控品名称"), &QCDailyResult::strQCName);
        QCDailyResultSer.RegisterField("std::string", "strQCBriefName", tr("质控品简称"), &QCDailyResult::strQCBriefName);
        QCDailyResultSer.RegisterField("std::string", "strQCSourceType", tr("质控品类型"), &QCDailyResult::strQCSourceType);
        QCDailyResultSer.RegisterField("std::string", "strQCLevel", tr("质控品水平"), &QCDailyResult::strQCLevel);
        QCDailyResultSer.RegisterField("std::string", "strQCLot", tr("质控品批号"), &QCDailyResult::strQCLot);
        QCDailyResultSer.RegisterField("std::string", "strItemName", tr("项目名称"), &QCDailyResult::strItemName);
        QCDailyResultSer.RegisterField("std::string", "strResult", tr("结果"), &QCDailyResult::strResult);
        QCDailyResultSer.RegisterField("std::string", "strUnit", tr("单位"), &QCDailyResult::strUnit);
        QCDailyResultSer.RegisterField("std::string", "strTargetMean", tr("靶值"), &QCDailyResult::strTargetMean);
        QCDailyResultSer.RegisterField("std::string", "strMaxRange", tr("参考范围高值"), &QCDailyResult::strMaxRange);
        QCDailyResultSer.RegisterField("std::string", "strMinRange", tr("参考范围低值"), &QCDailyResult::strMinRange);
        QCDailyResultSer.RegisterField("std::string", "strBreakRule", tr("失控规则"), &QCDailyResult::strBreakRule);
        SerMap[typeid(QCDailyResult).name()] = std::make_shared<Serializer>(QCDailyResultSer);

        Serializer QCDailyInfoSer;
        QCDailyInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &QCDailyInfo::strInstrumentModel);
        QCDailyInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &QCDailyInfo::strPrintTime);
        QCDailyInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &QCDailyInfo::strPrintTime);
        QCDailyInfoSer.RegisterField("std::string", "strTitle", tr("质控单日报告"), &QCDailyInfo::strTitle);
        QCDailyInfoSer.RegisterField("std::vector<QCDailyResult>", "vecResult", tr("质控结果"), &QCDailyInfo::vecResult);

        SerMap[typeid(QCDailyInfo).name()] = std::make_shared<Serializer>(QCDailyInfoSer);
    }

    // 质控结果
    {
        Serializer QCResultSer;
        QCResultSer.RegisterField("std::string", "strDisplayIndex", tr("序号"), &QCResult::strDisplayIndex);
        QCResultSer.RegisterField("std::string", "strQCDate", tr("质控时间"), &QCResult::strQCDate);
        QCResultSer.RegisterField("std::string", "strResult", tr("结果"), &QCResult::strResult);

        SerMap[typeid(QCResult).name()] = std::make_shared<Serializer>(QCResultSer);
    }

    // 质控LJ图信息
    {
        Serializer QCLJInfoSer;
        
        //QCLJInfoSer.RegisterField("std::string", "strQCDate", tr("质控日期"), &QCLJInfo::strQCDate);
        QCLJInfoSer.RegisterField("std::string", "strItemName", tr("项目名称:"), &QCLJInfo::strItemName);
        QCLJInfoSer.RegisterField("std::string", "strQCID", tr("质控品编号:"), &QCLJInfo::strQCID);
        QCLJInfoSer.RegisterField("std::string", "strQCName", tr("质控品名称:"), &QCLJInfo::strQCName);
        QCLJInfoSer.RegisterField("std::string", "strQcBriefName", tr("质控品简称:"), &QCLJInfo::strQcBriefName);
        QCLJInfoSer.RegisterField("std::string", "strQcSourceType", tr("质控品类型:"), &QCLJInfo::strQcSourceType);
        QCLJInfoSer.RegisterField("std::string", "strQcLevel", tr("质控品水平:"), &QCLJInfo::strQcLevel);
        QCLJInfoSer.RegisterField("std::string", "strQCLot", tr("质控品批号:"), &QCLJInfo::strQCLot);
        QCLJInfoSer.RegisterField("std::string", "strTargetMean", tr("靶值:"), &QCLJInfo::strTargetMean);
        QCLJInfoSer.RegisterField("std::string", "strTargetSD", tr("SD:"), &QCLJInfo::strTargetSD);
        QCLJInfoSer.RegisterField("std::string", "strTargetCV", tr("CV%:"), &QCLJInfo::strTargetCV);
        QCLJInfoSer.RegisterField("std::string", "strActualMean", tr("计算靶值:"), &QCLJInfo::strActualMean);
        QCLJInfoSer.RegisterField("std::string", "strActualSD", tr("计算SD:"), &QCLJInfo::strActualSD);
        QCLJInfoSer.RegisterField("std::string", "strActualCV", tr("计算CV%:"), &QCLJInfo::strActualCV);
        QCLJInfoSer.RegisterField("std::string", "strQcRltCount", tr("数量:"), &QCLJInfo::strQcRltCount);
        QCLJInfoSer.RegisterField("std::string", "strModelName", tr("模块:"), &QCLJInfo::strModelName);

        QCLJInfoSer.RegisterField("std::string", "strLJ", tr("L-J图:"), &QCLJInfo::strLJ);
        QCLJInfoSer.RegisterField("BS", "strLJImage", tr("L-J图片"), &QCLJInfo::strLJImage);
        QCLJInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号："), &QCLJInfo::strInstrumentModel);
        QCLJInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间："), &QCLJInfo::strPrintTime);
        QCLJInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间："), &QCLJInfo::strPrintTime);
        QCLJInfoSer.RegisterField("std::string", "strTitle", tr("项目质控信息"), &QCLJInfo::strTitle);
        QCLJInfoSer.RegisterField("std::string", "strQCAnalysis", tr("质控分析"), &QCLJInfo::strQCAnalysis);
        QCLJInfoSer.RegisterField("std::string", "strPrinter", tr("打印人："), &QCLJInfo::strPrinter);
        QCLJInfoSer.RegisterField("std::string", "strExporter", tr("导出人："), &QCLJInfo::strPrinter);
        //QCLJInfoSer.RegisterField("std::string", "strChart", tr("图表"), &QCLJInfo::strChart);
        QCLJInfoSer.RegisterField("std::vector<QCResult>", "vecResult", tr("质控结果"), &QCLJInfo::vecResult);

        SerMap[typeid(QCLJInfo).name()] = std::make_shared<Serializer>(QCLJInfoSer);
    }

    // 质控YD图信息
    {
        Serializer QCYDInfoSer;
        QCYDInfoSer.RegisterField("std::string", "strItemName", tr("项目名称:"), &QCYDInfo::strItemName);
        QCYDInfoSer.RegisterField("std::string", "strTimeQuantum", tr("质控日期:"), &QCYDInfo::strTimeQuantum);
        QCYDInfoSer.RegisterField("std::string", "strModelName", tr("模块:"), &QCYDInfo::strModelName);
        QCYDInfoSer.RegisterField("std::string", "strTP", tr("T-P图:"), &QCYDInfo::strTP);
        QCYDInfoSer.RegisterField("BS", "strTPImage", tr("T-P图片"), &QCYDInfo::strTPImage);
        QCYDInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &QCYDInfo::strInstrumentModel);
        QCYDInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &QCYDInfo::strPrintTime);
        QCYDInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &QCYDInfo::strPrintTime);
        QCYDInfoSer.RegisterField("std::string", "strTitle", tr("项目质控信息"), &QCYDInfo::strTitle);
        QCYDInfoSer.RegisterField("std::string", "strQCAnalysis", tr("质控分析"), &QCYDInfo::strQCAnalysis);
        QCYDInfoSer.RegisterField("std::string", "strPrinter", tr("打印人："), &QCYDInfo::strPrinter);
        QCYDInfoSer.RegisterField("std::string", "strExporter", tr("导出人："), &QCYDInfo::strPrinter);
        QCYDInfoSer.RegisterField("std::vector<QCResult>", "vecResult", tr("质控结果"), &QCYDInfo::vecResult);

        SerMap[typeid(QCYDInfo).name()] = std::make_shared<Serializer>(QCYDInfoSer);
    }

    // 质控品记录
    {
        Serializer QCItemSer;
        QCItemSer.RegisterField("std::string", "strDocNo", tr("文档号"), &QCRecord::strDocNo);
        QCItemSer.RegisterField("std::string", "strQCID", tr("质控品编号"), &QCRecord::strQCID);
        QCItemSer.RegisterField("std::string", "strQCName", tr("质控品名称"), &QCRecord::strQCName);
        QCItemSer.RegisterField("std::string", "strQcBriefName", tr("质控品简称"), &QCRecord::strQcBriefName);
        QCItemSer.RegisterField("std::string", "strQcSourceType", tr("质控品类型"), &QCRecord::strQcSourceType);
        QCItemSer.RegisterField("std::string", "strQcLevel", tr("质控品水平"), &QCRecord::strQcLevel);
        QCItemSer.RegisterField("std::string", "strQCLot", tr("质控品批号"), &QCRecord::strQCLot);
        QCItemSer.RegisterField("std::string", "strExpirationDate", tr("失效日期"), &QCRecord::strExpirationDate);
        QCItemSer.RegisterField("std::string", "strRegMethod", tr("登记方式"), &QCRecord::strRegMethod);
        QCItemSer.RegisterField("std::string", "strPos", tr("位置/样本管"), &QCRecord::strPos);
        SerMap[typeid(QCRecord).name()] = std::make_shared<Serializer>(QCItemSer);

        Serializer QCItemRecordSer;
        QCItemRecordSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &QCItemRecord::strInstrumentModel);
        QCItemRecordSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &QCItemRecord::strPrintTime);
        QCItemRecordSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &QCItemRecord::strPrintTime);
        QCItemRecordSer.RegisterField("std::string", "strTitle", tr("质控品记录"), &QCItemRecord::strTitle);
        QCItemRecordSer.RegisterField("std::vector<QCRecord>", "vecRecord", tr("质控品记录"), &QCItemRecord::vecRecord);
        SerMap[typeid(QCItemRecord).name()] = std::make_shared<Serializer>(QCItemRecordSer);
    }

    // 操作日志
    {
        Serializer OperationLogItemSer;
        OperationLogItemSer.RegisterField("std::string", "strIndex", tr("序号"), &OperationLogItem::strIndex);
        OperationLogItemSer.RegisterField("std::string", "strUserName", tr("用户名"), &OperationLogItem::strUserName);
        OperationLogItemSer.RegisterField("std::string", "strOperationType", tr("操作类型"), &OperationLogItem::strOperationType);
        OperationLogItemSer.RegisterField("std::string", "strRecord", tr("操作记录"), &OperationLogItem::strRecord);
        OperationLogItemSer.RegisterField("std::string", "strTime", tr("操作时间"), &OperationLogItem::strTime);
        SerMap[typeid(OperationLogItem).name()] = std::make_shared<Serializer>(OperationLogItemSer);

        Serializer OperationLogSer;
        OperationLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &OperationLog::strInstrumentModel);
        OperationLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &OperationLog::strPrintTime);
        OperationLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &OperationLog::strPrintTime);
        OperationLogSer.RegisterField("std::string", "strTitle", tr("操作日志"), &OperationLog::strTitle);
        OperationLogSer.RegisterField("std::vector<OperationLogItem>", "vecRecord", tr("操作日志"), &OperationLog::vecRecord);

        SerMap[typeid(OperationLog).name()] = std::make_shared<Serializer>(OperationLogSer);
    }

    // 试剂（耗材）更换记录
    {
        Serializer ChangeRecordSer;
        ChangeRecordSer.RegisterField("std::string", "strIndex", tr("序号"), &ChangeRecord::strIndex);
        ChangeRecordSer.RegisterField("std::string", "strModule", tr("模块"), &ChangeRecord::strModule);
        ChangeRecordSer.RegisterField("std::string", "strName", tr("名称"), &ChangeRecord::strName);
        ChangeRecordSer.RegisterField("std::string", "strOperationType", tr("操作类型"), &ChangeRecord::strOperationType);
        ChangeRecordSer.RegisterField("std::string", "strState", tr("状态"), &ChangeRecord::strState);
        ChangeRecordSer.RegisterField("std::string", "strUserName", tr("用户名"), &ChangeRecord::strUserName);
        ChangeRecordSer.RegisterField("std::string", "strLot", tr("批号"), &ChangeRecord::strLot);
        ChangeRecordSer.RegisterField("std::string", "strReagentSN", tr("瓶号/序列号"), &ChangeRecord::strReagentSN);
        ChangeRecordSer.RegisterField("std::string", "strTime", tr("更换时间"), &ChangeRecord::strTime);

        SerMap[typeid(ChangeRecord).name()] = std::make_shared<Serializer>(ChangeRecordSer);

        // 试剂（耗材）更换日志
        Serializer ReagentChangeLogSer;
        ReagentChangeLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &ReagentChangeLog::strInstrumentModel);
        ReagentChangeLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ReagentChangeLog::strPrintTime);
        ReagentChangeLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &ReagentChangeLog::strPrintTime);
        ReagentChangeLogSer.RegisterField("std::string", "strTitle", tr("试剂/耗材更换日志"), &ReagentChangeLog::strTitle);
        ReagentChangeLogSer.RegisterField("std::vector<ChangeRecord>", "vecRecord", tr("更换记录"), &ReagentChangeLog::vecRecord);

        SerMap[typeid(ReagentChangeLog).name()] = std::make_shared<Serializer>(ReagentChangeLogSer);
    }

    // 报警信息
    {
        Serializer AlarmInfoSer;
        AlarmInfoSer.RegisterField("std::string", "strIndex", tr("序号"), &AlarmInfo::strIndex);
        AlarmInfoSer.RegisterField("std::string", "strModule", tr("模块"), &AlarmInfo::strModule);
        AlarmInfoSer.RegisterField("std::string", "strAlarmCode", tr("报警代码"), &AlarmInfo::strAlarmCode);
        AlarmInfoSer.RegisterField("std::string", "strAlarmLevel", tr("报警级别"), &AlarmInfo::strAlarmLevel);
        AlarmInfoSer.RegisterField("std::string", "strAlarmName", tr("报警名称"), &AlarmInfo::strAlarmName);
        AlarmInfoSer.RegisterField("std::string", "strAlarmTime", tr("报警时间"), &AlarmInfo::strAlarmTime);
        SerMap[typeid(AlarmInfo).name()] = std::make_shared<Serializer>(AlarmInfoSer);

        Serializer AlarmInfoLogSer;
        AlarmInfoLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &AlarmInfoLog::strInstrumentModel);
        AlarmInfoLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &AlarmInfoLog::strPrintTime);
        AlarmInfoLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &AlarmInfoLog::strPrintTime);
        AlarmInfoLogSer.RegisterField("std::string", "strTitle", tr("报警日志"), &AlarmInfoLog::strTitle);
        AlarmInfoLogSer.RegisterField("std::vector<AlarmInfo>", "vecRecord", tr("报警信息"), &AlarmInfoLog::vecRecord);
        SerMap[typeid(AlarmInfoLog).name()] = std::make_shared<Serializer>(AlarmInfoLogSer);
    }

    // 维护日志
    {
        Serializer MaintenanceItemSer;
        MaintenanceItemSer.RegisterField("std::string", "strIndex", tr("序号"), &MaintenanceItem::strIndex);
        MaintenanceItemSer.RegisterField("std::string", "strGroupMaint", tr("组合维护"), &MaintenanceItem::strGroupMaint);
        MaintenanceItemSer.RegisterField("std::string", "strSingleMaint", tr("单项维护"), &MaintenanceItem::strSingleMaint);
        MaintenanceItemSer.RegisterField("std::string", "strMaintenanceStatus", tr("状态"), &MaintenanceItem::strMaintenanceStatus);
        MaintenanceItemSer.RegisterField("std::string", "strMaintenaceDate", tr("维护完成时间"), &MaintenanceItem::strMaintenaceDate);
        MaintenanceItemSer.RegisterField("std::string", "strModule", tr("模块"), &MaintenanceItem::strModule);
        MaintenanceItemSer.RegisterField("std::string", "strUserName", tr("用户名"), &MaintenanceItem::strUserName);

        SerMap[typeid(MaintenanceItem).name()] = std::make_shared<Serializer>(MaintenanceItemSer);

        Serializer MaintenanceLogSer;
        MaintenanceLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &MaintenanceLog::strInstrumentModel);
        MaintenanceLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &MaintenanceLog::strPrintTime);
        MaintenanceLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &MaintenanceLog::strPrintTime);
        MaintenanceLogSer.RegisterField("std::string", "strTitle", tr("维护日志"), &MaintenanceLog::strTitle);
        MaintenanceLogSer.RegisterField("std::vector<MaintenanceItem>", "vecRecord", tr("维护信息"), &MaintenanceLog::vecRecord);
        SerMap[typeid(MaintenanceLog).name()] = std::make_shared<Serializer>(MaintenanceLogSer);

    }

	// 杯空白测定结果
	{
		Serializer ChCupTestResultSer;
		ChCupTestResultSer.RegisterField("std::string", "strCupNum", tr("杯号"), &ChCupTestResult::strCupNum);
		ChCupTestResultSer.RegisterField("std::string", "str340Value", tr("340nm"), &ChCupTestResult::str340Value);
		ChCupTestResultSer.RegisterField("std::string", "str380Value", tr("380nm"), &ChCupTestResult::str380Value);
		ChCupTestResultSer.RegisterField("std::string", "str405Value", tr("405nm"), &ChCupTestResult::str405Value);
		ChCupTestResultSer.RegisterField("std::string", "str450Value", tr("450nm"), &ChCupTestResult::str450Value);
		ChCupTestResultSer.RegisterField("std::string", "str480Value", tr("480nm"), &ChCupTestResult::str480Value);
		ChCupTestResultSer.RegisterField("std::string", "str505Value", tr("505nm"), &ChCupTestResult::str505Value);
		ChCupTestResultSer.RegisterField("std::string", "str546Value", tr("546nm"), &ChCupTestResult::str546Value);
		ChCupTestResultSer.RegisterField("std::string", "str570Value", tr("570nm"), &ChCupTestResult::str570Value);
		ChCupTestResultSer.RegisterField("std::string", "str600Value", tr("600nm"), &ChCupTestResult::str600Value);
		ChCupTestResultSer.RegisterField("std::string", "str660Value", tr("660nm"), &ChCupTestResult::str660Value);
		ChCupTestResultSer.RegisterField("std::string", "str700Value", tr("700nm"), &ChCupTestResult::str700Value);
		ChCupTestResultSer.RegisterField("std::string", "str750Value", tr("750nm"), &ChCupTestResult::str750Value);
		ChCupTestResultSer.RegisterField("std::string", "str800Value", tr("800nm"), &ChCupTestResult::str800Value);
		
		SerMap[typeid(ChCupTestResult).name()] = std::make_shared<Serializer>(ChCupTestResultSer);

		Serializer ChCupBlankTestInfoSer;
		ChCupBlankTestInfoSer.RegisterField("std::string", "strModel", tr("模块:"), &ChCupBlankTestInfo::strModel);
		ChCupBlankTestInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ChCupBlankTestInfo::strPrintTime);
		ChCupBlankTestInfoSer.RegisterField("std::string", "strTestTime", tr("测定时间:"), &ChCupBlankTestInfo::strTestTime);
		ChCupBlankTestInfoSer.RegisterField("std::string", "strValueType", tr("示值类型:"), &ChCupBlankTestInfo::strValueType);
		ChCupBlankTestInfoSer.RegisterField("std::string", "strErrorCupNums", tr("异常反应杯号:"), &ChCupBlankTestInfo::strErrorCupNums);
		ChCupBlankTestInfoSer.RegisterField("std::vector<ChCupTestResult>", "vecCupResult", tr("数据结果"), &ChCupBlankTestInfo::vecCupResult);
		SerMap[typeid(ChCupBlankTestInfo).name()] = std::make_shared<Serializer>(ChCupBlankTestInfoSer);

	}

	// 杯空白计算结果
	{
		Serializer ChCupCalculateResultSer;
		ChCupCalculateResultSer.RegisterField("std::string", "strCupNum", tr("杯号"), &ChCupTestResult::strCupNum);
		ChCupCalculateResultSer.RegisterField("std::string", "str340Value", tr("340nm"), &ChCupTestResult::str340Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str380Value", tr("380nm"), &ChCupTestResult::str380Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str405Value", tr("405nm"), &ChCupTestResult::str405Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str450Value", tr("450nm"), &ChCupTestResult::str450Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str480Value", tr("480nm"), &ChCupTestResult::str480Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str505Value", tr("505nm"), &ChCupTestResult::str505Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str546Value", tr("546nm"), &ChCupTestResult::str546Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str570Value", tr("570nm"), &ChCupTestResult::str570Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str600Value", tr("600nm"), &ChCupTestResult::str600Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str660Value", tr("660nm"), &ChCupTestResult::str660Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str700Value", tr("700nm"), &ChCupTestResult::str700Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str750Value", tr("750nm"), &ChCupTestResult::str750Value);
		ChCupCalculateResultSer.RegisterField("std::string", "str800Value", tr("800nm"), &ChCupTestResult::str800Value);

		SerMap[typeid(ChCupTestResult).name()] = std::make_shared<Serializer>(ChCupCalculateResultSer);

		Serializer ChCupBlankCalculateInfoSer;
		ChCupBlankCalculateInfoSer.RegisterField("std::string", "strModel", tr("模块:"), &ChCupBlankCalculateInfo::strModel);
		ChCupBlankCalculateInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ChCupBlankCalculateInfo::strPrintTime);
		ChCupBlankCalculateInfoSer.RegisterField("std::string", "strTestTime1", tr("测定时间1:"), &ChCupBlankCalculateInfo::strTestTime1);
		ChCupBlankCalculateInfoSer.RegisterField("std::string", "strTestTime2", tr("测定时间2:"), &ChCupBlankCalculateInfo::strTestTime2);
		ChCupBlankCalculateInfoSer.RegisterField("std::vector<ChCupTestResult>", "vecCupResult", tr("数据结果"), &ChCupBlankCalculateInfo::vecCupResult);
		SerMap[typeid(ChCupBlankCalculateInfo).name()] = std::make_shared<Serializer>(ChCupBlankCalculateInfoSer);

	}

	// 光度计检查结果
	{
		Serializer ChPhotoCheckResultSer;
		ChPhotoCheckResultSer.RegisterField("std::string", "strWave", tr("波长"), &ChPhotoCheckResult::strWave);
		ChPhotoCheckResultSer.RegisterField("std::string", "strMeanValue", tr("均值"), &ChPhotoCheckResult::strMeanValue);
		ChPhotoCheckResultSer.RegisterField("std::string", "strRangeValue", tr("极差"), &ChPhotoCheckResult::strRangeValue);

		SerMap[typeid(ChPhotoCheckResult).name()] = std::make_shared<Serializer>(ChPhotoCheckResultSer);

		Serializer ChPhotoCheckResultInfoSer;
		ChPhotoCheckResultInfoSer.RegisterField("std::string", "strModel", tr("模块:"), &ChPhotoCheckResultInfo::strModel);
		ChPhotoCheckResultInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ChPhotoCheckResultInfo::strPrintTime);
		ChPhotoCheckResultInfoSer.RegisterField("std::string", "strCurTestTime", tr("当前测定时间:"), &ChPhotoCheckResultInfo::strCurTestTime);
		ChPhotoCheckResultInfoSer.RegisterField("std::string", "strHisTestTime", tr("历史测定时间:"), &ChPhotoCheckResultInfo::strHisTestTime);
		ChPhotoCheckResultInfoSer.RegisterField("std::vector<ChPhotoCheckResult>", "vecCurResult", tr("当前数据结果"), &ChPhotoCheckResultInfo::vecCurResult);
		ChPhotoCheckResultInfoSer.RegisterField("std::vector<ChPhotoCheckResult>", "vecHisResult", tr("历史数据结果"), &ChPhotoCheckResultInfo::vecHisResult);
		SerMap[typeid(ChPhotoCheckResultInfo).name()] = std::make_shared<Serializer>(ChPhotoCheckResultInfoSer);

	}

    // 反应杯历史
    {
        Serializer CupHistoryItemSer;
        CupHistoryItemSer.RegisterField("std::string", "strCupNum", tr("杯号"), &CupHistoryItem::strCupNum);
        CupHistoryItemSer.RegisterField("std::string", "strStatus", tr("状态"), &CupHistoryItem::strStatus);
        CupHistoryItemSer.RegisterField("std::string", "strTestTimes", tr("总测试数"), &CupHistoryItem::strTestTimes);
        CupHistoryItemSer.RegisterField("std::string", "strTestingAssay", tr("正在检查的项目"), &CupHistoryItem::strTestingAssay);
        CupHistoryItemSer.RegisterField("std::string", "strTestedAssay", tr("前十次项目"), &CupHistoryItem::strTestedAssay);

        SerMap[typeid(CupHistoryItem).name()] = std::make_shared<Serializer>(CupHistoryItemSer);

        Serializer CupHistoryInfoSer;
        CupHistoryInfoSer.RegisterField("std::string", "strModule", tr("模块:"), &CupHistoryInfo::strModule);
        CupHistoryInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &CupHistoryInfo::strPrintTime);
        CupHistoryInfoSer.RegisterField("std::vector<CupHistoryItem>", "vecRecord", tr("反应杯历史"), &CupHistoryInfo::vecRecord);
        SerMap[typeid(CupHistoryInfo).name()] = std::make_shared<Serializer>(CupHistoryInfoSer);
    }

    // 温度信息
    {
        Serializer TemperatureInfoSer;
        TemperatureInfoSer.RegisterField("std::string", "strModule", tr("模块"), &TemperatureInfo::strModule);
        TemperatureInfoSer.RegisterField("std::string", "strPartName", tr("部件名称"), &TemperatureInfo::strPartName);
        TemperatureInfoSer.RegisterField("std::string", "strCurTem", tr("当前温度"), &TemperatureInfo::strCurTem);
        TemperatureInfoSer.RegisterField("std::string", "strTemControlRange", tr("温控范围"), &TemperatureInfo::strTemControlRange);
        TemperatureInfoSer.RegisterField("std::string", "strTemControlStatu", tr("温控状态"), &TemperatureInfo::strTemControlStatu);

        SerMap[typeid(TemperatureInfo).name()] = std::make_shared<Serializer>(TemperatureInfoSer);

        Serializer TemperatureLogSer;
        TemperatureLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &TemperatureLog::strInstrumentModel);
        TemperatureLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &TemperatureLog::strPrintTime);
        TemperatureLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &TemperatureLog::strPrintTime);
        TemperatureLogSer.RegisterField("std::string", "strTitle", tr("温度"), &TemperatureLog::strTitle);
        TemperatureLogSer.RegisterField("std::vector<TemperatureInfo>", "vecRecord", tr("温度信息"), &TemperatureLog::vecRecord);
        SerMap[typeid(TemperatureLog).name()] = std::make_shared<Serializer>(TemperatureLogSer);
    }

    // 计数
    {
        Serializer StatisticalUnitSer;
        StatisticalUnitSer.RegisterField("std::string", "strModule", tr("模块"), &StatisticalUnit::strModule);
        StatisticalUnitSer.RegisterField("std::string", "strUnitName", tr("单元名称"), &StatisticalUnit::strUnitName);
        StatisticalUnitSer.RegisterField("std::string", "strPartName", tr("部件名称"), &StatisticalUnit::strPartName);
        StatisticalUnitSer.RegisterField("std::string", "strSN", tr("部件序列号"), &StatisticalUnit::strSN);
        StatisticalUnitSer.RegisterField("std::string", "strCurUseNum", tr("当前使用数目"), &StatisticalUnit::strCurUseNum);
        StatisticalUnitSer.RegisterField("std::string", "strAllUseNum", tr("累计使用数目"), &StatisticalUnit::strAllUseNum);
        StatisticalUnitSer.RegisterField("std::string", "strUseUpperLimit", tr("使用上限"), &StatisticalUnit::strUseUpperLimit);
        StatisticalUnitSer.RegisterField("std::string", "strResetTime", tr("重置时间"), &StatisticalUnit::strResetTime);
        SerMap[typeid(StatisticalUnit).name()] = std::make_shared<Serializer>(StatisticalUnitSer);

        Serializer StatisticalInfoSer;
        StatisticalInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &StatisticalInfo::strInstrumentModel);
        StatisticalInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &StatisticalInfo::strPrintTime);
        StatisticalInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &StatisticalInfo::strPrintTime);
        StatisticalInfoSer.RegisterField("std::string", "strTitle", tr("计数"), &StatisticalInfo::strTitle);
        StatisticalInfoSer.RegisterField("std::vector<StatisticalUnit>", "vecRecord", tr("统计信息"), &StatisticalInfo::vecRecord);
        SerMap[typeid(StatisticalInfo).name()] = std::make_shared<Serializer>(StatisticalInfoSer);
    }

    // 液路
    {
        Serializer LiquidItemSer;
        LiquidItemSer.RegisterField("std::string", "strModule", tr("模块"), &LiquidItem::strModule);
        LiquidItemSer.RegisterField("std::string", "strName", tr("名称"), &LiquidItem::strName);
        LiquidItemSer.RegisterField("std::string", "strStatus", tr("状态"), &LiquidItem::strStatus);
        SerMap[typeid(LiquidItem).name()] = std::make_shared<Serializer>(LiquidItemSer);

        Serializer LiquidInfoSer;
        LiquidInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &LiquidInfo::strPrintTime);
        LiquidInfoSer.RegisterField("std::vector<LiquidItem>", "vecRecord", tr("液路状态"), &LiquidInfo::vecRecord);
        SerMap[typeid(LiquidInfo).name()] = std::make_shared<Serializer>(LiquidInfoSer);
    }

    // 下位机单元
    {
        Serializer LowerCpUnitSer;
        LowerCpUnitSer.RegisterField("std::string", "strIndex", tr("序号"), &LowerCpUnit::strIndex);
        LowerCpUnitSer.RegisterField("std::string", "strModule", tr("模块"), &LowerCpUnit::strModule);
        LowerCpUnitSer.RegisterField("std::string", "strName", tr("名称"), &LowerCpUnit::strName);
        LowerCpUnitSer.RegisterField("std::string", "strState", tr("状态"), &LowerCpUnit::strState);
        SerMap[typeid(LowerCpUnit).name()] = std::make_shared<Serializer>(LowerCpUnitSer);

        Serializer LowerCpLogSer;
        LowerCpLogSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &LowerCpLog::strInstrumentModel);
        LowerCpLogSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &LowerCpLog::strPrintTime);
        LowerCpLogSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &LowerCpLog::strPrintTime);
        LowerCpLogSer.RegisterField("std::string", "strTitle", tr("下位机单元"), &LowerCpLog::strTitle);
        LowerCpLogSer.RegisterField("std::vector<LowerCpUnit>", "vecRecord", tr("单元信息"), &LowerCpLog::vecRecord);
        SerMap[typeid(LowerCpLog).name()] = std::make_shared<Serializer>(LowerCpLogSer);
    }

    // 样本量统计
    {
        Serializer SampleStatisticsSer;
        SampleStatisticsSer.RegisterField("std::string", "strModule", tr("模块"), &SampleStatistics::strModule);
        SampleStatisticsSer.RegisterField("std::string", "strReagentLot", tr("试剂批号"), &SampleStatistics::strReagentLot);
        SampleStatisticsSer.RegisterField("std::string", "strItemName", tr("项目名称"), &SampleStatistics::strItemName);
        SampleStatisticsSer.RegisterField("std::string", "strTotal", tr("样本量"), &SampleStatistics::strTotal);
        SampleStatisticsSer.RegisterField("std::string", "strConvSample", tr("常规"), &SampleStatistics::strConvSample);
        SampleStatisticsSer.RegisterField("std::string", "strEmSample", tr("急诊"), &SampleStatistics::strEmSample);
        SampleStatisticsSer.RegisterField("std::string", "strQc", tr("质控"), &SampleStatistics::strQc);
        SampleStatisticsSer.RegisterField("std::string", "strCali", tr("校准"), &SampleStatistics::strCali);
        SerMap[typeid(SampleStatistics).name()] = std::make_shared<Serializer>(SampleStatisticsSer);

        Serializer SampleStatisticsSumSer;
        SampleStatisticsSumSer.RegisterField("std::string", "strType", tr("类型"), &SampleStatisticsSum::strType);
        SampleStatisticsSumSer.RegisterField("std::string", "strTotalOrder", tr("样本量"), &SampleStatisticsSum::strTotalOrder);
        SampleStatisticsSumSer.RegisterField("std::string", "strConvSample", tr("常规"), &SampleStatisticsSum::strConvSample);
        SampleStatisticsSumSer.RegisterField("std::string", "strEmSample", tr("急诊"), &SampleStatisticsSum::strEmSample);
        SampleStatisticsSumSer.RegisterField("std::string", "strQc", tr("质控"), &SampleStatisticsSum::strQc);
        SampleStatisticsSumSer.RegisterField("std::string", "strCali", tr("校准"), &SampleStatisticsSum::strCali);
        SerMap[typeid(SampleStatisticsSum).name()] = std::make_shared<Serializer>(SampleStatisticsSumSer);

        Serializer SampleStatisticsInfoSer;
        SampleStatisticsInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &SampleStatisticsInfo::strInstrumentModel);
        SampleStatisticsInfoSer.RegisterField("std::string", "strTitle", tr("样本量统计"), &SampleStatisticsInfo::strTitle);
        SampleStatisticsInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &SampleStatisticsInfo::strPrintTime);
        SampleStatisticsInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &SampleStatisticsInfo::strPrintTime);
        SampleStatisticsInfoSer.RegisterField("std::string", "strDate", tr("日期:"), &SampleStatisticsInfo::strDate);
        SampleStatisticsInfoSer.RegisterField("std::string", "strModule", tr("模块:"), &SampleStatisticsInfo::strModule);
        SampleStatisticsInfoSer.RegisterField("std::string", "strReagentLot", tr("试剂批号:"), &SampleStatisticsInfo::strReagentLot);
        SampleStatisticsInfoSer.RegisterField("std::vector<SampleStatisticsSum>", "vecRecordSum", tr("总量统计信息"), &SampleStatisticsInfo::vecRecordSum);
        SampleStatisticsInfoSer.RegisterField("std::vector<SampleStatistics>", "vecRecord", tr("统计信息"), &SampleStatisticsInfo::vecRecord);

        SerMap[typeid(SampleStatisticsInfo).name()] = std::make_shared<Serializer>(SampleStatisticsInfoSer);
    }

    // 试剂统计
    {
        Serializer ReagentStatisticsSer;
        ReagentStatisticsSer.RegisterField("std::string", "strIndex", tr("日期"), &ReagentStatistics::strIndex);
        ReagentStatisticsSer.RegisterField("std::string", "strReagentCount", tr("试剂工作量"), &ReagentStatistics::strReagentCount);
        ReagentStatisticsSer.RegisterField("std::string", "strSampleCount", tr("样本工作量"), &ReagentStatistics::strSampleCount);
        ReagentStatisticsSer.RegisterField("std::string", "strRetestCount", tr("复查工作量"), &ReagentStatistics::strRetestCount);
        ReagentStatisticsSer.RegisterField("std::string", "strQcCount", tr("质控工作量"), &ReagentStatistics::strQcCount);
        ReagentStatisticsSer.RegisterField("std::string", "strCaliCount", tr("校准工作量"), &ReagentStatistics::strCaliCount);
        SerMap[typeid(ReagentStatistics).name()] = std::make_shared<Serializer>(ReagentStatisticsSer);

        Serializer ReagentStatisticsInfoSer;
        ReagentStatisticsInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &ReagentStatisticsInfo::strInstrumentModel);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strTitle", tr("试剂统计"), &ReagentStatisticsInfo::strTitle);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strModule", tr("模块:"), &ReagentStatisticsInfo::strModule);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &ReagentStatisticsInfo::strPrintTime);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &ReagentStatisticsInfo::strPrintTime);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strDate", tr("日期:"), &ReagentStatisticsInfo::strDate);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strReagentName", tr("试剂名称:"), &ReagentStatisticsInfo::strReagentName);
        ReagentStatisticsInfoSer.RegisterField("std::string", "strImageTitle", tr("折线图"), &ReagentStatisticsInfo::strImageTitle);
        ReagentStatisticsInfoSer.RegisterField("BS", "strImage", tr("折线图片"), &ReagentStatisticsInfo::strImage);
        ReagentStatisticsInfoSer.RegisterField("std::vector<ReagentStatistics>", "vecRecord", tr("统计信息"), &ReagentStatisticsInfo::vecRecord);

        SerMap[typeid(ReagentStatisticsInfo).name()] = std::make_shared<Serializer>(ReagentStatisticsInfoSer);

    }

    // 校准统计
    {
        Serializer CaliStatisticsSer;
        CaliStatisticsSer.RegisterField("std::string", "strModule", tr("模块"), &CaliStatistics::strModule);
        CaliStatisticsSer.RegisterField("std::string", "strItemName", tr("项目名称"), &CaliStatistics::strItemName);
        CaliStatisticsSer.RegisterField("std::string", "strReagentLot", tr("试剂批号"), &CaliStatistics::strReagentLot);
        CaliStatisticsSer.RegisterField("std::string", "strCaliNum", tr("校准次数"), &CaliStatistics::strCaliNum);

        SerMap[typeid(CaliStatistics).name()] = std::make_shared<Serializer>(CaliStatisticsSer);

        Serializer CaliStatisticsInfoSer;
        CaliStatisticsInfoSer.RegisterField("std::string", "strInstrumentModel", tr("仪器型号:"), &CaliStatisticsInfo::strInstrumentModel);
        CaliStatisticsInfoSer.RegisterField("std::string", "strTitle", tr("校准统计"), &CaliStatisticsInfo::strTitle);
        CaliStatisticsInfoSer.RegisterField("std::string", "strModule", tr("模块:"), &CaliStatisticsInfo::strModule);
        CaliStatisticsInfoSer.RegisterField("std::string", "strPrintTime", tr("打印时间:"), &CaliStatisticsInfo::strPrintTime);
        CaliStatisticsInfoSer.RegisterField("std::string", "strExportTime", tr("导出时间:"), &CaliStatisticsInfo::strPrintTime);
        CaliStatisticsInfoSer.RegisterField("std::string", "strDate", tr("日期:"), &CaliStatisticsInfo::strDate);
        CaliStatisticsInfoSer.RegisterField("std::string", "strItemName", tr("项目名称:"), &CaliStatisticsInfo::strItemName);
        CaliStatisticsInfoSer.RegisterField("std::string", "strReagentLot", tr("试剂批号:"), &CaliStatisticsInfo::strReagentLot);
        CaliStatisticsInfoSer.RegisterField("std::vector<CaliStatistics>", "vecRecord", tr("统计信息"), &CaliStatisticsInfo::vecRecord);

        SerMap[typeid(CaliStatisticsInfo).name()] = std::make_shared<Serializer>(CaliStatisticsInfoSer);
    }

    // 生化校准结果
    {
        Serializer CaliParameterSer;
        CaliParameterSer.RegisterField("std::string", "strCaliParameter", QObject::tr("校准参数"), &ChCaliParameters::strCaliParameter);
        CaliParameterSer.RegisterField("std::string", "strParameterValue", QObject::tr("结果"), &ChCaliParameters::strParameterValue);
        SerMap[typeid(ChCaliParameters).name()] = std::make_shared<Serializer>(CaliParameterSer);

        Serializer CaliResultDataSer;
        CaliResultDataSer.RegisterField("std::string", "strLevel", QObject::tr("校准品水平"), &ChCaliResultData::strLevel);
        CaliResultDataSer.RegisterField("std::string", "strCalibratorPos", QObject::tr("校准品位置"), &ChCaliResultData::strCalibratorPos);
        CaliResultDataSer.RegisterField("std::string", "strFirstCup", QObject::tr("首次杯号"), &ChCaliResultData::strFirstCup);
        CaliResultDataSer.RegisterField("std::string", "strFirstCupPreAssay", QObject::tr("首次同杯前反应"), &ChCaliResultData::strFirstCupPreAssay);
        CaliResultDataSer.RegisterField("std::string", "strSecCup", QObject::tr("第二次杯号"), &ChCaliResultData::strSecCup);
        CaliResultDataSer.RegisterField("std::string", "strSecCupPreAssay", QObject::tr("第二次同杯前反应"), &ChCaliResultData::strSecCupPreAssay);
        CaliResultDataSer.RegisterField("std::string", "strAbs1", QObject::tr("吸光度1"), &ChCaliResultData::strAbs1);
        CaliResultDataSer.RegisterField("std::string", "strAbs2", QObject::tr("吸光度2"), &ChCaliResultData::strAbs2);
        CaliResultDataSer.RegisterField("std::string", "strAvgAbs", QObject::tr("吸光度均值"), &ChCaliResultData::strAvgAbs);
        CaliResultDataSer.RegisterField("std::string", "strConc", QObject::tr("浓度"), &ChCaliResultData::strConc);
        SerMap[typeid(ChCaliResultData).name()] = std::make_shared<Serializer>(CaliResultDataSer);

        Serializer CaliResultInfoChSer;
        CaliResultInfoChSer.RegisterField("std::string", "strModelName", QObject::tr("模块"), &CaliResultInfoCh::strModelName);
        CaliResultInfoChSer.RegisterField("std::string", "strItemName", QObject::tr("项目名称"), &CaliResultInfoCh::strItemName);
        CaliResultInfoChSer.RegisterField("std::string", "strUnit", QObject::tr("单位"), &CaliResultInfoCh::strUnit);
        CaliResultInfoChSer.RegisterField("std::string", "strDeviceNum", QObject::tr("仪器编号"), &CaliResultInfoCh::strDeviceNum);
        CaliResultInfoChSer.RegisterField("std::string", "strSoftVersion", QObject::tr("软件版本"), &CaliResultInfoCh::strSoftVersion);
        CaliResultInfoChSer.RegisterField("std::string", "strReagentLot", QObject::tr("试剂批号"), &CaliResultInfoCh::strReagentLot);
        CaliResultInfoChSer.RegisterField("std::string", "strReagentSN", QObject::tr("试剂瓶号"), &CaliResultInfoCh::strReagentSN);
        CaliResultInfoChSer.RegisterField("std::string", "strCalibratorLot", QObject::tr("校准品批号"), &CaliResultInfoCh::strCalibratorLot);
        CaliResultInfoChSer.RegisterField("std::string", "strCalibrateDate", QObject::tr("校准时间"), &CaliResultInfoCh::strCalibrateDate);
        CaliResultInfoChSer.RegisterField("std::string", "strAcidityLot", QObject::tr("酸性清洗液批号"), &CaliResultInfoCh::strAcidityLot);
        CaliResultInfoChSer.RegisterField("std::string", "strAlkalinityLot", QObject::tr("碱性清洗液批号"), &CaliResultInfoCh::strAlkalinityLot);
        CaliResultInfoChSer.RegisterField("std::string", "strCaliType", QObject::tr("校准方法"), &CaliResultInfoCh::strCaliType);
        CaliResultInfoChSer.RegisterField("std::string", "strCaliMode", QObject::tr("执行方法"), &CaliResultInfoCh::strCaliMode);
        CaliResultInfoChSer.RegisterField("std::string", "strAlarm", QObject::tr("数据报警"), &CaliResultInfoCh::strAlarm);
        CaliResultInfoChSer.RegisterField("std::string", "strExportBy", QObject::tr("导出人"), &CaliResultInfoCh::strExportBy);
        CaliResultInfoChSer.RegisterField("std::string", "strPrintBy", QObject::tr("打印人"), &CaliResultInfoCh::strPrintBy);
        //CaliResultInfoChSer.RegisterField("std::string", "strCaliCurve", QObject::tr("校准曲线"), &CaliResultInfoCh::strCaliCurve);
        //CaliResultInfoChSer.RegisterField("std::string", "strCaliCurveImage", QObject::tr("校准曲线(图片)"), &CaliResultInfoCh::strCaliCurveImage);
        CaliResultInfoChSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间"), &CaliResultInfoCh::strPrintTime);
        CaliResultInfoChSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间"), &CaliResultInfoCh::strExportTime);
        CaliResultInfoChSer.RegisterField("std::vector<ChCaliParameters>", "vecCaliParameters", QObject::tr("参数列表"), &CaliResultInfoCh::vecCaliParameters);
        CaliResultInfoChSer.RegisterField("std::vector<ChCaliResultData>", "vecCaliResultData", QObject::tr("校准品数据"), &CaliResultInfoCh::vecCaliResultData);

        SerMap[typeid(CaliResultInfoCh).name()] = std::make_shared<Serializer>(CaliResultInfoChSer);
    }

    // ISE校准结果
    {
        Serializer IseCaliResultDataSer;
        IseCaliResultDataSer.RegisterField("std::string", "strAssayName", QObject::tr("项目名称"), &IseCaliResultData::strAssayName);
        IseCaliResultDataSer.RegisterField("std::string", "strLowSamleEmf", QObject::tr("低值"), &IseCaliResultData::strLowSamleEmf);
        IseCaliResultDataSer.RegisterField("std::string", "strLowBaseEmf", QObject::tr("低值基准"), &IseCaliResultData::strLowBaseEmf);
        IseCaliResultDataSer.RegisterField("std::string", "strHighSamleEmf", QObject::tr("高值"), &IseCaliResultData::strHighSamleEmf);
        IseCaliResultDataSer.RegisterField("std::string", "strHighBaseEmf", QObject::tr("高值基准"), &IseCaliResultData::strHighBaseEmf);
        IseCaliResultDataSer.RegisterField("std::string", "strSlope", QObject::tr("斜率"), &IseCaliResultData::strSlope);
        IseCaliResultDataSer.RegisterField("std::string", "strDilutionRatio", QObject::tr("稀释倍数"), &IseCaliResultData::strDilutionRatio);
        IseCaliResultDataSer.RegisterField("std::string", "strAlarm", QObject::tr("数据报警"), &IseCaliResultData::strAlarm);
        SerMap[typeid(IseCaliResultData).name()] = std::make_shared<Serializer>(IseCaliResultDataSer);

        Serializer CaliResultInfoIseSer;
        CaliResultInfoIseSer.RegisterField("std::string", "strReagentName", QObject::tr("项目名称"), &CaliResultInfoIse::strReagentName);
        CaliResultInfoIseSer.RegisterField("std::string", "strPos", QObject::tr("位置"), &CaliResultInfoIse::strPos);
        CaliResultInfoIseSer.RegisterField("std::string", "strCalibrateDate", QObject::tr("校准时间"), &CaliResultInfoIse::strCalibrateDate);
        CaliResultInfoIseSer.RegisterField("std::string", "strUnit", QObject::tr("单位"), &CaliResultInfoIse::strUnit);
        CaliResultInfoIseSer.RegisterField("std::string", "strCalibratorLot", QObject::tr("校准品批号"), &CaliResultInfoIse::strCalibratorLot);
        CaliResultInfoIseSer.RegisterField("std::string", "strCalibrator1", QObject::tr("高浓度校准品位置"), &CaliResultInfoIse::strCalibrator1);
        CaliResultInfoIseSer.RegisterField("std::string", "strCalibrator2", QObject::tr("低浓度校准品位置"), &CaliResultInfoIse::strCalibrator2);
        CaliResultInfoIseSer.RegisterField("std::string", "strIsLot", QObject::tr("IC批号"), &CaliResultInfoIse::strIsLot);
        CaliResultInfoIseSer.RegisterField("std::string", "strDiluLot", QObject::tr("BS批号"), &CaliResultInfoIse::strDiluLot);
        CaliResultInfoIseSer.RegisterField("std::string", "strDeviceNum", QObject::tr("仪器编号"), &CaliResultInfoIse::strDeviceNum);
        CaliResultInfoIseSer.RegisterField("std::string", "strSoftVersion", QObject::tr("软件版本号"), &CaliResultInfoIse::strSoftVersion);
        CaliResultInfoIseSer.RegisterField("std::string", "strCaliResult", QObject::tr("校准结果"), &CaliResultInfoIse::strCaliResult);
        CaliResultInfoIseSer.RegisterField("std::string", "strTitle", QObject::tr("标题"), &CaliResultInfoIse::strTitle);
        CaliResultInfoIseSer.RegisterField("std::string", "strModelName", QObject::tr("模块"), &CaliResultInfoIse::strModelName);
        CaliResultInfoIseSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间"), &CaliResultInfoIse::strPrintTime);
        CaliResultInfoIseSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间"), &CaliResultInfoIse::strExportTime);
        CaliResultInfoIseSer.RegisterField("std::string", "strPrintBy", QObject::tr("打印人"), &CaliResultInfoIse::strPrintBy);
        CaliResultInfoIseSer.RegisterField("std::string", "strExportBy", QObject::tr("导出人"), &CaliResultInfoIse::strExportBy);
        CaliResultInfoIseSer.RegisterField("std::vector<IseCaliResultData>", "vecCaliResultData", QObject::tr("校准品记录"), &CaliResultInfoIse::vecCaliResultData);

        SerMap[typeid(CaliResultInfoIse).name()] = std::make_shared<Serializer>(CaliResultInfoIseSer);
    }

    // 生化反应曲线
    {
        Serializer CaliCurveLvDataSer;
        CaliCurveLvDataSer.RegisterField("std::string", "strPonitIndex", QObject::tr("测光点"), &CaliCurveLvData::strPonitIndex);
        CaliCurveLvDataSer.RegisterField("std::string", "strPrimaryWave", QObject::tr("主波长"), &CaliCurveLvData::strPrimaryWave);
        CaliCurveLvDataSer.RegisterField("std::string", "strSubWave", QObject::tr("次波长"), &CaliCurveLvData::strSubWave);
        CaliCurveLvDataSer.RegisterField("std::string", "strWaveDiff", QObject::tr("主-次波长"), &CaliCurveLvData::strWaveDiff);
        SerMap[typeid(CaliCurveLvData).name()] = std::make_shared<Serializer>(CaliCurveLvDataSer);

        Serializer CaliCurveTimesDataSer;
        //CaliCurveTimesDataSer.RegisterField("std::string", "strReactCurve", QObject::tr("反应曲线"), &CaliCurveTimesData::strReactCurve);
        CaliCurveTimesDataSer.RegisterField("std::string", "strCaliSeqNo", QObject::tr("校准品次序"), &CaliCurveTimesData::strCaliSeqNo);
        //CaliCurveTimesDataSer.RegisterField("std::string", "strReactCurveImage", QObject::tr("反应曲线(图片)"), &CaliCurveTimesData::strReactCurveImage);
        CaliCurveTimesDataSer.RegisterField("std::vector<CaliCurveLvData>", "vecCaliLvData", QObject::tr("校准品反应数据"), &CaliCurveTimesData::vecCaliLvData);
        SerMap[typeid(CaliCurveTimesData).name()] = std::make_shared<Serializer>(CaliCurveTimesDataSer);

        Serializer ReactCurveDataSer;
        ReactCurveDataSer.RegisterField("std::string", "strTitle", QObject::tr("标题"), &ReactCurveData::strTitle);
        ReactCurveDataSer.RegisterField("std::string", "strReagentName", QObject::tr("项目名称"), &ReactCurveData::strReagentName);
        ReactCurveDataSer.RegisterField("std::string", "strModelName", QObject::tr("模块"), &ReactCurveData::strModelName);
        ReactCurveDataSer.RegisterField("std::string", "strCaliName", QObject::tr("校准品名称"), &ReactCurveData::strCaliName);
        ReactCurveDataSer.RegisterField("std::string", "strReagentLot", QObject::tr("试剂批号"), &ReactCurveData::strReagentLot);
        ReactCurveDataSer.RegisterField("std::string", "strReagentSN", QObject::tr("试剂瓶号"), &ReactCurveData::strReagentSN);
        ReactCurveDataSer.RegisterField("std::string", "strCaliMode", QObject::tr("执行方法"), &ReactCurveData::strCaliMode);
        ReactCurveDataSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间"), &ReactCurveData::strPrintTime);
        ReactCurveDataSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间"), &ReactCurveData::strExportTime);
        ReactCurveDataSer.RegisterField("grp<CaliCurveTimesData>", "grpCurveTimes", QObject::tr("反应曲线次序数据"), &ReactCurveData::vecCaliTimesData);
       

        SerMap[typeid(ReactCurveData).name()] = std::make_shared<Serializer>(ReactCurveDataSer);
    }

    // 生化校准历史
    {
        Serializer ChCaliHisLvDataSer;
        ChCaliHisLvDataSer.RegisterField("std::string", "strCalibrator", QObject::tr("校准品"), &ChCaliHisLvData::strCalibrator);
        ChCaliHisLvDataSer.RegisterField("std::string", "strCalibratorPos", QObject::tr("校准品位置"), &ChCaliHisLvData::strCalibratorPos);
        ChCaliHisLvDataSer.RegisterField("std::string", "strFirstCup", QObject::tr("首次杯号"), &ChCaliHisLvData::strFirstCup);
        ChCaliHisLvDataSer.RegisterField("std::string", "strFirstCupPreAssay", QObject::tr("首次同杯前反应"), &ChCaliHisLvData::strFirstCupPreAssay);
        ChCaliHisLvDataSer.RegisterField("std::string", "strSecCup", QObject::tr("第二次杯号"), &ChCaliHisLvData::strSecCup);
        ChCaliHisLvDataSer.RegisterField("std::string", "strSecCupPreAssay", QObject::tr("第二次同杯前反应"), &ChCaliHisLvData::strSecCupPreAssay);
        ChCaliHisLvDataSer.RegisterField("std::string", "strAbs1", QObject::tr("吸光度1"), &ChCaliHisLvData::strAbs1);
        ChCaliHisLvDataSer.RegisterField("std::string", "strAbs2", QObject::tr("吸光度2"), &ChCaliHisLvData::strAbs2);
        ChCaliHisLvDataSer.RegisterField("std::string", "strAvgAbs", QObject::tr("吸光度均值"), &ChCaliHisLvData::strAvgAbs);
        ChCaliHisLvDataSer.RegisterField("std::string", "strConc", QObject::tr("浓度"), &ChCaliHisLvData::strConc);
        SerMap[typeid(ChCaliHisLvData).name()] = std::make_shared<Serializer>(ChCaliHisLvDataSer);

        Serializer ChCaliHisTimesDataSer;

        ChCaliHisTimesDataSer.RegisterField("std::string", "strCaliSeqNo", QObject::tr("校准次序"), &ChCaliHisTimesData::strCaliSeqNo);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strCalibratorLot", QObject::tr("校准品批号"), &ChCaliHisTimesData::strCalibratorLot);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strReagentLot", QObject::tr("试剂批号"), &ChCaliHisTimesData::strReagentLot);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strReagentSN", QObject::tr("试剂瓶号"), &ChCaliHisTimesData::strReagentSN);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strCalibrateDate", QObject::tr("校准时间"), &ChCaliHisTimesData::strCalibrateDate);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strCaliResult", QObject::tr("校准结果"), &ChCaliHisTimesData::strCaliResult);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strCaliMode", QObject::tr("执行方法"), &ChCaliHisTimesData::strCaliMode);
        ChCaliHisTimesDataSer.RegisterField("std::string", "strAlarm", QObject::tr("数据报警"), &ChCaliHisTimesData::strAlarm);
        ChCaliHisTimesDataSer.RegisterField("std::vector<ChCaliHisLvData>", "vecCaliLvData", QObject::tr("生化校准品反应数据"), &ChCaliHisTimesData::vecCaliLvData);
        SerMap[typeid(ChCaliHisTimesData).name()] = std::make_shared<Serializer>(ChCaliHisTimesDataSer);

        Serializer CaliHistoryInfoChSer;
        CaliHistoryInfoChSer.RegisterField("std::string", "strTitle", QObject::tr("校准历史"), &CaliHistoryInfoCh::strTitle);
        CaliHistoryInfoChSer.RegisterField("std::string", "strName", QObject::tr("项目名称"), &CaliHistoryInfoCh::strName);
        CaliHistoryInfoChSer.RegisterField("std::string", "strUnit", QObject::tr("单位"), &CaliHistoryInfoCh::strUnit);
        CaliHistoryInfoChSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间"), &CaliHistoryInfoCh::strExportTime);
        CaliHistoryInfoChSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间"), &CaliHistoryInfoCh::strPrintTime);
        CaliHistoryInfoChSer.RegisterField("std::string", "strModelName", QObject::tr("模块"), &CaliHistoryInfoCh::strModelName);
        CaliHistoryInfoChSer.RegisterField("grp<ChCaliHisTimesData>", "grpCaliTimes", QObject::tr("校准次序数据"), &CaliHistoryInfoCh::grpTimesData);
        SerMap[typeid(CaliHistoryInfoCh).name()] = std::make_shared<Serializer>(CaliHistoryInfoChSer);
    }

    // ISE校准历史
    {
        Serializer IseCaliHisLvDataSer;
        IseCaliHisLvDataSer.RegisterField("std::string", "strCalibrator", QObject::tr("校准品"), &IseCaliHisLvData::strCalibrator);
        IseCaliHisLvDataSer.RegisterField("std::string", "strConc", QObject::tr("浓度"), &IseCaliHisLvData::strConc);
        IseCaliHisLvDataSer.RegisterField("std::string", "strEmf", QObject::tr("电动势"), &IseCaliHisLvData::strEmf);
        IseCaliHisLvDataSer.RegisterField("std::string", "strBase", QObject::tr("基准"), &IseCaliHisLvData::strBase);
        
        SerMap[typeid(IseCaliHisLvData).name()] = std::make_shared<Serializer>(IseCaliHisLvDataSer);

        Serializer IseCaliHisTimesDataSer;
        IseCaliHisTimesDataSer.RegisterField("std::string", "strCaliSeqNo", QObject::tr("校准次序"), &IseCaliHisTimesData::strCaliSeqNo);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strCalibratorLot", QObject::tr("校准品批号"), &IseCaliHisTimesData::strCalibratorLot);
        IseCaliHisTimesDataSer.RegisterField("std::string", "IsLot", QObject::tr("IC批号"), &IseCaliHisTimesData::IsLot);
        IseCaliHisTimesDataSer.RegisterField("std::string", "IsSn", QObject::tr("IC瓶号"), &IseCaliHisTimesData::IsSn);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strDiluLot", QObject::tr("BS批号"), &IseCaliHisTimesData::strDiluLot);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strDiluSn", QObject::tr("BS瓶号"), &IseCaliHisTimesData::strDiluSn);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strCalibrateDate", QObject::tr("校准时间"), &IseCaliHisTimesData::strCalibrateDate);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strCaliResult", QObject::tr("校准结果"), &IseCaliHisTimesData::strCaliResult);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strAlarm", QObject::tr("数据报警"), &IseCaliHisTimesData::strAlarm);
        IseCaliHisTimesDataSer.RegisterField("std::string", "strSlop", QObject::tr("斜率值"), &IseCaliHisTimesData::strSlop);
        IseCaliHisTimesDataSer.RegisterField("std::vector<IseCaliHisLvData>", "vecCaliLvData", QObject::tr("ISE校准品反应数据"), &IseCaliHisTimesData::vecCaliLvData);

        SerMap[typeid(IseCaliHisTimesData).name()] = std::make_shared<Serializer>(IseCaliHisTimesDataSer);

        Serializer CaliHistoryInfoIseSer;
        CaliHistoryInfoIseSer.RegisterField("std::string", "strTitle", QObject::tr("校准历史"), &CaliHistoryInfoIse::strTitle);
        CaliHistoryInfoIseSer.RegisterField("std::string", "strName", QObject::tr("项目名称"), &CaliHistoryInfoIse::strName);
        CaliHistoryInfoIseSer.RegisterField("std::string", "strUnit", QObject::tr("单位"), &CaliHistoryInfoIse::strUnit);
        CaliHistoryInfoIseSer.RegisterField("std::string", "strExportTime", QObject::tr("导出时间"), &CaliHistoryInfoIse::strExportTime);
        CaliHistoryInfoIseSer.RegisterField("std::string", "strPrintTime", QObject::tr("打印时间"), &CaliHistoryInfoIse::strPrintTime);
        CaliHistoryInfoIseSer.RegisterField("std::string", "strPos", QObject::tr("位置"), &CaliHistoryInfoIse::strPos);
        CaliHistoryInfoIseSer.RegisterField("grp<IseCaliHisTimesData>", "grpCaliTimesData", QObject::tr("ISE校准品次序数据"), &CaliHistoryInfoIse::vecCaliTimesData);

        SerMap[typeid(CaliHistoryInfoIse).name()] = std::make_shared<Serializer>(CaliHistoryInfoIseSer);
    }


    /*return SerMap;*/
}
//std::map<std::string, std::shared_ptr<Serializer>> SerializerMap::m_SerializerMap = SerializerMap::InitMap();

///
///  @brief 将传入的结构体对象转换成json字符串
///
///
///  @param[in]   Info  结构体对象
///
///  @return	string json 字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename Type>
std::string GetSerializerString(Type& Info)
{
    std::string strInfoName = typeid(Type).name();
    std::shared_ptr<Serializer> pSer = SerializerMap::GetInstance()->getSerializer(strInfoName);
    if (pSer == nullptr)
        return " ";

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    doc.AddMember("print_device", APP_NAME, allocator);

    rapidjson::Value Module_Name(rapidjson::kStringType);
    Module_Name.SetString(strInfoName.c_str(), rapidjson::SizeType(strInfoName.size()), allocator);
    doc.AddMember("print_module", Module_Name, allocator);

    pSer->Serialize(Info, doc);

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    //std::ofstream ofs("testinfo.json", std::ios::out | std::ios::app);
    //if (ofs.is_open())
    //{
    //    ofs << data << std::endl;
    //}

    //ofs.close();
    return data;

}

///
///  @brief 将传入的结构体数组对象转换成json字符串
///
///
///  @param[in]   vecItem  结构体数组
///
///  @return	string json 字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename StructType, typename... Types>
std::string GetSerializerString(std::vector<StructType, Types...>& vecItem)
{
    rapidjson::Document doc;
    doc.SetObject();

    Serialize_vec<StructType>(vecItem, "vecItems", doc);

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    return data;
}

///
///  @brief 将结构体元信息放入 json数组中
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename StructType>
void GetStructInfoValue(rapidjson::Document::AllocatorType& allocator_, rapidjson::Value& FieldArray_)
{
    std::shared_ptr<Serializer> pSerializer = SerializerMap::GetInstance()->getSerializer(typeid(StructType).name());
    if (pSerializer == nullptr)
        return;

    rapidjson::Document::AllocatorType& allocator = allocator_;
    int iFCount = pSerializer->getFieldsCount();
    for (int i = 0; i < iFCount; i++)
    {
        rapidjson::Value FieldArray(rapidjson::kArrayType);
        std::string Info;
        rapidjson::Value Value(rapidjson::kStringType);
        std::string strFType = pSerializer->getFieldType(i);
        if (strFType == "std::string")
        {
            Info = pSerializer->getFieldName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            Info = pSerializer->getDisplayName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            FieldArray.PushBack("ST", allocator);
        }
        else if (strFType == "QString")
        {
            Info = pSerializer->getFieldName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            Info = pSerializer->getDisplayName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);
            FieldArray.PushBack("ST", allocator);
        }
        else if (strFType == "int")
        {
            Info = pSerializer->getFieldName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            Info = pSerializer->getDisplayName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            FieldArray.PushBack("NM", allocator);
        }
        else if (strFType == "BS")
        {
            Info = pSerializer->getFieldName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            Info = pSerializer->getDisplayName(i);
            Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
            FieldArray.PushBack(Value, allocator);

            FieldArray.PushBack("BS", allocator);
        }
        else if (strFType == "std::vector<SampleExportInfo>")
        {
            GetVectorValue<SampleExportInfo>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ItemInfo>")
        {
            GetVectorValue<ItemInfo>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ItemResult>")
        {
            GetVectorValue<ItemResult>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ItemSampleResult>")
        {
            GetVectorValue<ItemSampleResult>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ReagentItem>")
        {
            GetVectorValue<ReagentItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<SupplyItem>")
        {
            GetVectorValue<SupplyItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliResultData>")
        {
            GetVectorValue<CaliResultData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliRLUInfo>")
        {
            GetVectorValue<CaliRLUInfo>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliRack>")
        {
            GetVectorValue<CaliRack>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliRecord>")
        {
            GetVectorValue<CaliRecord>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ChCaliRecord>")
        {
            GetVectorValue<ChCaliRecord>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<QCDailyResult>")
        {
            GetVectorValue<QCDailyResult>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<QCResult>")
        {
            GetVectorValue<QCResult>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<QCRecord>")
        {
            GetVectorValue<QCRecord>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<OperationLogItem>")
        {
            GetVectorValue<OperationLogItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ChangeRecord>")
        {
            GetVectorValue<ChangeRecord>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<AlarmInfo>")
        {
            GetVectorValue<AlarmInfo>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<MaintenanceItem>")
        {
            GetVectorValue<MaintenanceItem>(pSerializer, i, FieldArray, allocator);
        }
		else if (strFType == "std::vector<ChCupTestResult>")
		{
			GetVectorValue<ChCupTestResult>(pSerializer, i, FieldArray, allocator);
		}
		else if (strFType == "std::vector<ChPhotoCheckResult>")
		{
			GetVectorValue<ChPhotoCheckResult>(pSerializer, i, FieldArray, allocator);
		}
        else if (strFType == "std::vector<CupHistoryItem>")
        {
            GetVectorValue<CupHistoryItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<LiquidItem>")
        {
            GetVectorValue<LiquidItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<TemperatureInfo>")
        {
            GetVectorValue<TemperatureInfo>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<StatisticalUnit>")
        {
            GetVectorValue<StatisticalUnit>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<LowerCpUnit>")
        {
            GetVectorValue<LowerCpUnit>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<SampleStatistics>")
        {
            GetVectorValue<SampleStatistics>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<SampleStatisticsSum>")
        {
            GetVectorValue<SampleStatisticsSum>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ReagentStatistics>")
        {
            GetVectorValue<ReagentStatistics>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliStatistics>")
        {
            GetVectorValue<CaliStatistics>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ChCaliParameters>")
        {
            GetVectorValue<ChCaliParameters>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ChCaliResultData>")
        {
            GetVectorValue<ChCaliResultData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<IseCaliResultData>")
        {
            GetVectorValue<IseCaliResultData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<CaliCurveLvData>")
        {
            GetVectorValue<CaliCurveLvData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<ChCaliHisLvData>")
        {
            GetVectorValue<ChCaliHisLvData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<IseCaliHisLvData>")
        {
            GetVectorValue<IseCaliHisLvData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "std::vector<RequireCalcItem>")
        {
            GetVectorValue<RequireCalcItem>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "grp<ChCaliHisTimesData>")
        {
            GetGroupValue<ChCaliHisTimesData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "grp<CaliCurveTimesData>")
        {
            GetGroupValue<CaliCurveTimesData>(pSerializer, i, FieldArray, allocator);
        }
        else if (strFType == "grp<IseCaliHisTimesData>")
        {
            GetGroupValue<IseCaliHisTimesData>(pSerializer, i, FieldArray, allocator);
        }

        FieldArray_.PushBack(FieldArray, allocator);
    }
}

template<typename StructType>
void GetVectorValue(std::shared_ptr<Serializer>& pSerializer, int iIndex, rapidjson::Value& FieldArray, rapidjson::Document::AllocatorType& allocator)
{
    std::string Info = pSerializer->getFieldName(iIndex);
    rapidjson::Value Value(rapidjson::kStringType);
    Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
    FieldArray.PushBack(Value, allocator);

    Info = pSerializer->getDisplayName(iIndex);
    Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
    FieldArray.PushBack(Value, allocator);
    FieldArray.PushBack("TB", allocator);

    rapidjson::Value vecValue(rapidjson::kArrayType);
    GetStructInfoValue<StructType>(allocator, vecValue);
    FieldArray.PushBack(vecValue, allocator);
}

template<typename StructType>
void GetGroupValue(std::shared_ptr<Serializer>& pSerializer, int iIndex, rapidjson::Value& FieldArray, rapidjson::Document::AllocatorType& allocator)
{
    std::string Info = pSerializer->getFieldName(iIndex);
    rapidjson::Value Value(rapidjson::kStringType);
    Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
    FieldArray.PushBack(Value, allocator);

    Info = pSerializer->getDisplayName(iIndex);
    Value.SetString(Info.c_str(), rapidjson::SizeType(Info.size()), allocator);
    FieldArray.PushBack(Value, allocator);
    FieldArray.PushBack("GRP", allocator);

    rapidjson::Value vecValue(rapidjson::kArrayType);
    GetStructInfoValue<StructType>(allocator, vecValue);
    FieldArray.PushBack(vecValue, allocator);
}

///
///  @brief 获取结构体元信息
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename StructType>
void GetStructInfo(rapidjson::Document::AllocatorType& allocator, rapidjson::Value& vecStructInfo, QString strModleName)
{
    rapidjson::Value FieldArray(rapidjson::kArrayType);
    rapidjson::Value ModleName(rapidjson::kStringType);

    // 先加入模块名
    std::string strMN = strModleName.toStdString();
    ModleName.SetString(strMN.c_str(), rapidjson::SizeType(strMN.size()), allocator);
    FieldArray.PushBack(ModleName, allocator);
    GetStructInfoValue<StructType>(allocator, FieldArray);
    std::string strInfoName = typeid(StructType).name();
    rapidjson::Value KeyValue(rapidjson::kStringType);
    KeyValue.SetString(strInfoName.c_str(), rapidjson::SizeType(strInfoName.size()), allocator);
    vecStructInfo.AddMember(KeyValue, FieldArray, allocator);
}

///
///  @brief 将所有要打印的结构体元信息转成json字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
void SerializerMap::GetAllStructInfoString()
{
    GetPublicStructInfoString();
    GetImStructInfoString();
    GetChStructInfoString();
    GetIseStructInfoString();
}

///
///  @brief 将所有打印的结构体元信息转成json字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
std::string SerializerMap::GetPublicStructInfoString()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    doc.AddMember("device", APP_NAME, allocator);
    rapidjson::Value AllST(rapidjson::kArrayType);

    // 将各模块的数据信息为 json 格式放入数组中
    rapidjson::Value SampleExportModuleValue(rapidjson::kObjectType);
    GetStructInfo<SampleExportModule>(allocator, SampleExportModuleValue, tr("原始结果记录"));
    AllST.PushBack(SampleExportModuleValue, allocator);

    rapidjson::Value SampleInfoValue(rapidjson::kObjectType);
    GetStructInfo<SampleInfo>(allocator, SampleInfoValue, tr("样本信息"));
    AllST.PushBack(SampleInfoValue, allocator);

    rapidjson::Value SimpleSampleInfoValue(rapidjson::kObjectType);
    GetStructInfo<SimpleSampleInfo>(allocator, SimpleSampleInfoValue, tr("按样本打印"));
    AllST.PushBack(SimpleSampleInfoValue, allocator);

    rapidjson::Value ItemSampleInfoValue(rapidjson::kObjectType);
    GetStructInfo<ItemSampleInfo>(allocator, ItemSampleInfoValue, tr("按项目打印"));
    AllST.PushBack(ItemSampleInfoValue, allocator);

    rapidjson::Value ReagentInfoValue(rapidjson::kObjectType);
    GetStructInfo<ReagentInfo>(allocator, ReagentInfoValue, tr("试剂信息"));
    AllST.PushBack(ReagentInfoValue, allocator);

    rapidjson::Value RequireCalcInfoValue(rapidjson::kObjectType);
    GetStructInfo<RequireCalcInfo>(allocator, RequireCalcInfoValue, tr("需求计算"));
    AllST.PushBack(RequireCalcInfoValue, allocator);

    rapidjson::Value SupplyInfoValue(rapidjson::kObjectType);
    GetStructInfo<SupplyInfo>(allocator, SupplyInfoValue, tr("耗材信息"));
    AllST.PushBack(SupplyInfoValue, allocator);

    /*rapidjson::Value CaliResultInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliResultInfo>(allocator, CaliResultInfoValue, tr("校准结果"));
    AllST.PushBack(CaliResultInfoValue, allocator);*/

    rapidjson::Value CaliRackInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliRackInfo>(allocator, CaliRackInfoValue, tr("校准架概况"));
    AllST.PushBack(CaliRackInfoValue, allocator);

    /*rapidjson::Value CaliRecordInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliRecordInfo>(allocator, CaliRecordInfoValue, tr("校准品记录"));
    AllST.PushBack(CaliRecordInfoValue, allocator);*/

    rapidjson::Value QCDailyResultValue(rapidjson::kObjectType);
    GetStructInfo<QCDailyInfo>(allocator, QCDailyResultValue, tr("单日质控报告"));
    AllST.PushBack(QCDailyResultValue, allocator);

    rapidjson::Value QCLJInfoValue(rapidjson::kObjectType);
    GetStructInfo<QCLJInfo>(allocator, QCLJInfoValue, tr("质控L-J信息"));
    AllST.PushBack(QCLJInfoValue, allocator);

    rapidjson::Value QCYDInfoValue(rapidjson::kObjectType);
    GetStructInfo<QCYDInfo>(allocator, QCYDInfoValue, tr("质控T-P信息"));
    AllST.PushBack(QCYDInfoValue, allocator);

    rapidjson::Value QCItemRecordValue(rapidjson::kObjectType);
    GetStructInfo<QCItemRecord>(allocator, QCItemRecordValue, tr("质控品记录"));
    AllST.PushBack(QCItemRecordValue, allocator);

    rapidjson::Value OperationLogValue(rapidjson::kObjectType);
    GetStructInfo<OperationLog>(allocator, OperationLogValue, tr("操作日志"));
    AllST.PushBack(OperationLogValue, allocator);

    rapidjson::Value ReagentChangeLogValue(rapidjson::kObjectType);
    GetStructInfo<ReagentChangeLog>(allocator, ReagentChangeLogValue, tr("试剂/耗材更换日志"));
    AllST.PushBack(ReagentChangeLogValue, allocator);

    rapidjson::Value AlarmInfoLogValue(rapidjson::kObjectType);
    GetStructInfo<AlarmInfoLog>(allocator, AlarmInfoLogValue, tr("报警日志"));
    AllST.PushBack(AlarmInfoLogValue, allocator);

    rapidjson::Value MaintenanceLogValue(rapidjson::kObjectType);
    GetStructInfo<MaintenanceLog>(allocator, MaintenanceLogValue, tr("维护日志"));
    AllST.PushBack(MaintenanceLogValue, allocator);

    rapidjson::Value LiquidInfoValue(rapidjson::kObjectType);
    GetStructInfo<LiquidInfo>(allocator, LiquidInfoValue, tr("液路状态"));
    AllST.PushBack(LiquidInfoValue, allocator);

    rapidjson::Value TemperatureLogValue(rapidjson::kObjectType);
    GetStructInfo<TemperatureLog>(allocator, TemperatureLogValue, tr("温度信息"));
    AllST.PushBack(TemperatureLogValue, allocator);

    rapidjson::Value StatisticalInfoValue(rapidjson::kObjectType);
    GetStructInfo<StatisticalInfo>(allocator, StatisticalInfoValue, tr("计数"));
    AllST.PushBack(StatisticalInfoValue, allocator);

    rapidjson::Value LowerCpLogValue(rapidjson::kObjectType);
    GetStructInfo<LowerCpLog>(allocator, LowerCpLogValue, tr("下位机单元"));
    AllST.PushBack(LowerCpLogValue, allocator);

    rapidjson::Value SampleStatisticsInfoValue(rapidjson::kObjectType);
    GetStructInfo<SampleStatisticsInfo>(allocator, SampleStatisticsInfoValue, tr("样本量统计"));
    AllST.PushBack(SampleStatisticsInfoValue, allocator);

    rapidjson::Value ReagentStatisticsInfoValue(rapidjson::kObjectType);
    GetStructInfo<ReagentStatisticsInfo>(allocator, ReagentStatisticsInfoValue, tr("试剂耗材统计"));
    AllST.PushBack(ReagentStatisticsInfoValue, allocator);

    rapidjson::Value CaliStatisticsInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliStatisticsInfo>(allocator, CaliStatisticsInfoValue, tr("校准统计"));
    AllST.PushBack(CaliStatisticsInfoValue, allocator);

    doc.AddMember("print_module_ds_list", AllST, allocator);

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    std::ofstream ofs("PrintConfiguration_zh.json", std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
        ofs << data << std::endl;
    }

    ofs.close();

    return data;
}

///
///  @brief 将所有免疫的结构体元信息转成json字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
std::string SerializerMap::GetImStructInfoString()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    doc.AddMember("device", APP_NAME, allocator);
    rapidjson::Value AllST(rapidjson::kArrayType);

    rapidjson::Value CaliResultInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliResultInfo>(allocator, CaliResultInfoValue, tr("校准结果"));
    AllST.PushBack(CaliResultInfoValue, allocator);

    rapidjson::Value CaliHistoryInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliHistoryInfo>(allocator, CaliHistoryInfoValue, tr("校准历史"));
    AllST.PushBack(CaliHistoryInfoValue, allocator);

    rapidjson::Value CaliRecordInfoValue(rapidjson::kObjectType);
    GetStructInfo<CaliRecordInfo>(allocator, CaliRecordInfoValue, tr("校准品记录"));
    AllST.PushBack(CaliRecordInfoValue, allocator);

    doc.AddMember("print_module_ds_list", AllST, allocator);
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    std::ofstream ofs("ImPrintConfiguration_zh.json", std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
        ofs << data << std::endl;
    }

    ofs.close();

    return data;

}

///
///  @brief 将所有生化的结构体元信息转成json字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
std::string SerializerMap::GetChStructInfoString()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    doc.AddMember("device", APP_NAME, allocator);
    rapidjson::Value AllST(rapidjson::kArrayType);

    rapidjson::Value ChCaliRecordInfoValue(rapidjson::kObjectType);
    GetStructInfo<ChCaliRecordInfo>(allocator, ChCaliRecordInfoValue, QObject::tr("生化校准品记录"));
    AllST.PushBack(ChCaliRecordInfoValue, allocator);

    rapidjson::Value ChCupBlankTestInfoValue(rapidjson::kObjectType);
    GetStructInfo<ChCupBlankTestInfo>(allocator, ChCupBlankTestInfoValue, tr("杯空白结果"));
    AllST.PushBack(ChCupBlankTestInfoValue, allocator);

    rapidjson::Value ChCupBlankCalculateInfoValue(rapidjson::kObjectType);
    GetStructInfo<ChCupBlankCalculateInfo>(allocator, ChCupBlankCalculateInfoValue, tr("杯空白计算结果"));
    AllST.PushBack(ChCupBlankCalculateInfoValue, allocator);

    rapidjson::Value ChPhotoCheckResultInfoValue(rapidjson::kObjectType);
    GetStructInfo<ChPhotoCheckResultInfo>(allocator, ChPhotoCheckResultInfoValue, tr("光度计检查结果"));
    AllST.PushBack(ChPhotoCheckResultInfoValue, allocator);

    rapidjson::Value CupHistoryInfoValue(rapidjson::kObjectType);
    GetStructInfo<CupHistoryInfo>(allocator, CupHistoryInfoValue, tr("反应杯历史"));
    AllST.PushBack(CupHistoryInfoValue, allocator);

    rapidjson::Value CaliResultInfoChValue(rapidjson::kObjectType);
    GetStructInfo<CaliResultInfoCh>(allocator, CaliResultInfoChValue, QObject::tr("生化校准结果"));
    AllST.PushBack(CaliResultInfoChValue, allocator);


    rapidjson::Value ReactCurveDataValue(rapidjson::kObjectType);
    GetStructInfo< ReactCurveData>(allocator, ReactCurveDataValue, QObject::tr("生化反应曲线"));
    AllST.PushBack(ReactCurveDataValue, allocator);

    rapidjson::Value CaliHistoryInfoChValue(rapidjson::kObjectType);
    GetStructInfo<CaliHistoryInfoCh>(allocator, CaliHistoryInfoChValue, QObject::tr("生化校准历史"));
    AllST.PushBack(CaliHistoryInfoChValue, allocator);

    doc.AddMember("print_module_ds_list", AllST, allocator);
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    std::ofstream ofs("ChPrintConfiguration_zh.json", std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
        ofs << data << std::endl;
    }

    ofs.close();

    return data;
}

///
///  @brief 将所有Ise的结构体元信息转成json字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
std::string SerializerMap::GetIseStructInfoString()
{
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    doc.AddMember("device", APP_NAME, allocator);
    rapidjson::Value AllST(rapidjson::kArrayType);

    rapidjson::Value CaliResultInfoIseValue(rapidjson::kObjectType);
    GetStructInfo<CaliResultInfoIse>(allocator, CaliResultInfoIseValue, QObject::tr("ISE校准结果"));
    AllST.PushBack(CaliResultInfoIseValue, allocator);

    rapidjson::Value CaliHistoryInfoIseValue(rapidjson::kObjectType);
    GetStructInfo<CaliHistoryInfoIse>(allocator, CaliHistoryInfoIseValue, QObject::tr("ISE校准历史"));
    AllST.PushBack(CaliHistoryInfoIseValue, allocator);

    doc.AddMember("print_module_ds_list", AllST, allocator);
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);
    std::string data = strBuf.GetString();
    std::ofstream ofs("IsePrintConfiguration_zh.json", std::ios::out | std::ios::app);
    if (ofs.is_open())
    {
        ofs << data << std::endl;
    }

    ofs.close();

    return data;
}

///
///  @brief 将传入的结构体对象转换成json字符串
///
///
///  @param[in]   Info  结构体对象
///
///  @return	string json 字符串
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
std::string GetJsonString(SampleExportModule& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(SampleInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(SimpleSampleInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ItemSampleInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ReagentInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(SupplyInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliResultInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliRackInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliHistoryInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliRecordInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ChCaliRecordInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(QCDailyInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(QCLJInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(QCYDInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(QCItemRecord& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(OperationLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ReagentChangeLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(AlarmInfoLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(MaintenanceLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ChCupBlankTestInfo& Info)
{
	return GetSerializerString(Info);
}

std::string GetJsonString(ChCupBlankCalculateInfo& Info)
{
	return GetSerializerString(Info);
}

std::string GetJsonString(ChPhotoCheckResultInfo& Info)
{
	return GetSerializerString(Info);
}

std::string GetJsonString(TemperatureLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CupHistoryInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(LiquidInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(StatisticalInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(LowerCpLog& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(SampleStatisticsInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ReagentStatisticsInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliStatisticsInfo& Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliResultInfoCh & Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliResultInfoIse & Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(ReactCurveData & Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliHistoryInfoCh & Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(CaliHistoryInfoIse & Info)
{
    return GetSerializerString(Info);
}

std::string GetJsonString(RequireCalcInfo& Info)
{
    return GetSerializerString(Info);
}


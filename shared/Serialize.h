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
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <QObject>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <QString>
#include <fstream>
#include "PrintExportDefine.h"
#include <boost/noncopyable.hpp>

///
///  @brief 字段类， 用于保存结构体中各字段的元信息
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
class AnyField
{
    class FieldBase
    {
    public:
        FieldBase(const std::string& Type_, const std::string& name, const QString& disname)
            : m_Type(Type_)
            , m_FieldName(name)
            , m_DisplayName(disname)
        {
        }

        virtual FieldBase * clone() = 0;
        std::string m_Type;
        std::string m_FieldName;
        QString m_DisplayName;
    };

    template <typename StructType, typename FieldType>
    class Field : public FieldBase
    {
    public:
        Field(const std::string& Type_, const std::string& name, const QString& disname, FieldType StructType::*pointer)
            : FieldBase(Type_, name, disname)
            , m_pField(pointer)
        {
        }

        virtual FieldBase* clone()
        {
            return new Field<StructType, FieldType>(m_Type, m_FieldName, m_DisplayName, m_pField);
        }

        FieldType StructType::* getFieldPointer()
        {
            return m_pField;
        }
    private:

        FieldType StructType::*m_pField;
    };

public:
    template <typename StructType, typename FieldType>
    AnyField(const std::string& Type_, const std::string& name, const QString& disname, FieldType StructType::*pointer)
    {
        m_pField = new Field<StructType, FieldType>(Type_, name, disname, pointer);
    }

    AnyField(const AnyField& rhs)
    {
        m_pField = rhs.m_pField->clone();
    }

    AnyField& operator=(const AnyField& rhs)
    {
        if (m_pField)delete m_pField;
        m_pField = rhs.m_pField->clone();
        return *this;
    }

    AnyField(AnyField&& rhs) noexcept
    {
        m_pField = rhs.m_pField;
        rhs.m_pField = 0;
    }

    AnyField& operator=(AnyField&& rhs) noexcept
    {
        if (m_pField)
            delete m_pField;

        m_pField = rhs.m_pField;
        rhs.m_pField = 0;
        return *this;
    }

    ~AnyField()
    {
        if (m_pField)
            delete m_pField;
    }

    template <typename StructType, typename FieldType>
    FieldType StructType::* getFPtr()
    {
        Field<StructType, FieldType>* Fptr = (Field<StructType, FieldType>*)m_pField;
        return Fptr->getFieldPointer();
    }

    std::string getType()
    {
        if (m_pField)
            return m_pField->m_Type;
        else
            return std::string(" ");
    }

    std::string getFieldName()
    {
        if (m_pField)
            return m_pField->m_FieldName;
        else
            return std::string(" ");
    }

    QString getDisplayName()
    {
        if (m_pField)
            return m_pField->m_DisplayName;
        else
            return QString(" ");
    }

private:
    FieldBase* m_pField;

};

///
///  @brief 序列化类， 用于管理结构体的元信息并将其序列化
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
class Serializer
{
public:
    Serializer() {};
    ~Serializer() {};
    std::string getFieldName(int iIndex);

    std::string getDisplayName(int iIndex);

    std::string getFieldType(int iIndex);

    int getFieldsCount();

    // 序列化结构体
    template <typename StructType>
    void Serialize(StructType& Item, rapidjson::Document& docJson);

	///
	///  @brief 获取字段地址
	///
	///  @par History: 
	///  @li 6889/ChenWei，2023年11月16日，新建函数
	///
	template <typename StructType, typename FieldType>
	FieldType StructType::* FieldPtr(int iIndex)
	{
		if (iIndex >= m_fields.size())
			return nullptr;

		return m_fields.at(iIndex).getFPtr<StructType, FieldType>();
	}

    // 注册字段信息
    template <typename StructType, typename FieldType>
    void RegisterField(const std::string& Type_, const std::string& name_, const QString& disname_, FieldType StructType::*pointer);

private:
    std::vector<AnyField> m_fields;

};

///
///  @brief 序列化管理器， 管理各种序列化器
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月16日，新建函数
///
class SerializerMap : public QObject, public boost::noncopyable
{
    Q_OBJECT
public:
    SerializerMap(QObject *parent = Q_NULLPTR);

    // 获取单例
    static std::shared_ptr<SerializerMap> GetInstance();

    std::shared_ptr<Serializer> getSerializer(std::string TypeName_);

    // 将所有要打印的结构体元信息转成json字符串
    static void GetAllStructInfoString();

    // 将所有公共的结构体元信息转成json字符串
    static std::string GetPublicStructInfoString();

    // 将所有免疫的结构体元信息转成json字符串
    static std::string GetImStructInfoString();

    // 将所有生化的结构体元信息转成json字符串
    static std::string GetChStructInfoString();

    // 将所有Ise的结构体元信息转成json字符串
    static std::string GetIseStructInfoString();
private:
    void InitMap();

private:
    static std::shared_ptr<SerializerMap> s_Instance;
    std::map<std::string, std::shared_ptr<Serializer>> m_SerializerMap;
};

///
///  @brief 序列化数组
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename StructType>
void Serialize_vec(std::vector<StructType, std::allocator<StructType>>& vecItem, std::string strName, rapidjson::Document& docJson);

///
///  @brief 序列化组对象
///
///  @par History: 
///  @li 6889/ChenWei，2023年11月3日，新建函数
///
template<typename StructType>
void Serialize_grp(std::vector<StructType, std::allocator<StructType>>& vecItem, std::string strName, rapidjson::Document& docJson);

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
std::string GetJsonString(SampleExportModule& Info);
std::string GetJsonString(SampleInfo& Info);
std::string GetJsonString(SimpleSampleInfo& Info);
std::string GetJsonString(ItemSampleInfo& Info);
std::string GetJsonString(ReagentInfo& Info);
std::string GetJsonString(SupplyInfo& Info);
std::string GetJsonString(CaliResultInfo& Info);
std::string GetJsonString(CaliRackInfo& Info);
std::string GetJsonString(CaliHistoryInfo& Info);
std::string GetJsonString(CaliRecordInfo& Info);
std::string GetJsonString(ChCaliRecordInfo& Info);
std::string GetJsonString(QCDailyInfo& Info);
std::string GetJsonString(QCLJInfo& Info);
std::string GetJsonString(QCYDInfo& Info);
std::string GetJsonString(QCItemRecord& Info);
std::string GetJsonString(OperationLog& Info);
std::string GetJsonString(ReagentChangeLog& Info);
std::string GetJsonString(AlarmInfoLog& Info);
std::string GetJsonString(MaintenanceLog& Info);
std::string GetJsonString(ChCupBlankTestInfo& Info);
std::string GetJsonString(ChCupBlankCalculateInfo& Info);
std::string GetJsonString(ChPhotoCheckResultInfo& Info);
std::string GetJsonString(TemperatureLog& Info);
std::string GetJsonString(CupHistoryInfo& Info);
std::string GetJsonString(LiquidInfo& Info);
std::string GetJsonString(StatisticalInfo& Info);
std::string GetJsonString(LowerCpLog& Info);
std::string GetJsonString(SampleStatisticsInfo& Info);
std::string GetJsonString(ReagentStatisticsInfo& Info);
std::string GetJsonString(CaliStatisticsInfo& Info);
std::string GetJsonString(CaliResultInfoCh& Info);
std::string GetJsonString(CaliResultInfoIse& Info);
std::string GetJsonString(ReactCurveData& Info);
std::string GetJsonString(CaliHistoryInfoCh& Info);
std::string GetJsonString(CaliHistoryInfoIse& Info);
std::string GetJsonString(RequireCalcInfo& Info);


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
std::string GetSerializerString(std::vector<StructType, Types...>& vecItem);





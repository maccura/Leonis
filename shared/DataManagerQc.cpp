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
/// @file     DataManagerQc.cpp
/// @brief    质控数据暂存,以处理频繁访问查询接口去获取质控数据的
///
/// @author   8276/huchunli
/// @date     2023年5月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年5月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DataManagerQc.h"
#include "msgiddef.h"
#include "messagebus.h"
#include "thrift/DcsControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief 初始化单例对象
///
std::shared_ptr<DataManagerQc> DataManagerQc::s_instance = nullptr;

///
/// @brief 初始化单例锁
///
std::mutex DataManagerQc::m_dataqcMutx;


DataManagerQc::DataManagerQc()
{
    // 初始化需要缓存的质控数据
    ReloadQcData();
    m_qcDocChanged = true;

    // 监听质控更新信息
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocUpdate);
}

std::shared_ptr<DataManagerQc> DataManagerQc::GetInstance()
{
    if (s_instance == nullptr)
    {
        std::unique_lock<std::mutex> autoLock(m_dataqcMutx);
        if (s_instance == nullptr)
        {
            s_instance.reset(new DataManagerQc());
        }
    }

    return s_instance;
}

DataManagerQc::~DataManagerQc()
{

}

void DataManagerQc::GetQcDocNames(std::vector<QString>& qcDocNames)
{
    // 更新缓存的质控品名称
    if (m_qcDocChanged)
    {
        m_qcDocNamesNoISE.clear();
        std::map<int64_t, std::shared_ptr<tf::QcDoc>>::iterator it = m_mapDocs.begin();
        for (; it != m_mapDocs.end(); ++it)
        {
            if (it->second == nullptr)
            {
                continue;
            }
            for (const auto& cp : it->second->compositions)
            {
                if (cp.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_NA || 
                    cp.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_K ||
                    cp.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
                {
                    continue;
                }
                m_qcDocNamesNoISE.push_back(QString::fromStdString(it->second->name));
            }
        }
    }

    qcDocNames = std::move(m_qcDocNamesNoISE);
}

void DataManagerQc::GetQcDocs(std::vector<std::shared_ptr<tf::QcDoc>>& qcDocs)
{
    std::map<int64_t, std::shared_ptr<tf::QcDoc>>::iterator it = m_mapDocs.begin();
    for (; it != m_mapDocs.end(); ++it)
    {
        if (it->second == nullptr)
        {
            continue;
        }
        qcDocs.push_back(it->second);
    }
}

///
/// @brief 根据数据ID获取质控文档（现用于修改位置时调用此函数）
///
/// @param[in]  id  质控文档的数据库ID
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月18日，新建函数
///
std::shared_ptr<tf::QcDoc> DataManagerQc::GetQcDocByID(const int64_t id)
{
	if (id < 0)
	{
		return nullptr;
	}
	const auto atDoc = m_mapDocs.find(id);
	if (atDoc == m_mapDocs.end())
	{
		return nullptr;
	}
	if (nullptr == atDoc->second)
	{
		return nullptr;
	}
	return atDoc->second;
}

void DataManagerQc::GetQcDocs(std::vector<std::shared_ptr<tf::QcDoc>>& qcDocs, int assayCode)
{
    qcDocs.clear();

    std::map<int64_t, std::shared_ptr<tf::QcDoc>>::iterator it = m_mapDocs.begin();
    for (; it != m_mapDocs.end(); ++it)
    {
        if (it->second == nullptr)
        {
            continue;
        }
        const std::vector<tf::QcComposition> &qcCom = it->second->compositions;
        for (const tf::QcComposition& cp : qcCom)
        {
            if (cp.assayCode == assayCode)
            {
                qcDocs.push_back(it->second);
                break;
            }
        }
    }
}

///
/// @bref
///		初始化质控暂存数据
///
/// @par History:
/// @li 8276/huchunli, 2023年5月26日，新建函数
///
void DataManagerQc::ReloadQcData()
{
    m_qcDocIDs.clear();

    tf::QcDocQueryResp qcDocs;
    if (DcsControlProxy::GetInstance()->QueryQcDocAll(qcDocs))
    {
        for (const tf::QcDoc& qcItem : qcDocs.lstQcDocs)
        {
            m_qcDocIDs.insert(qcItem.id);
            m_mapDocs.insert(std::pair<int64_t, std::shared_ptr<tf::QcDoc>>(qcItem.id, std::make_shared<tf::QcDoc>(qcItem)));
        }
    }
}

///
/// @bref
///		质控品更新信息
///
/// @par History:
/// @li 8276/huchunli, 2023年5月26日，新建函数
///
void DataManagerQc::OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos)
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_qcDocChanged = true;
    if (infos.size() == 0)
    {
        ReloadQcData();
        return;
    }

    // 多线程访问互斥
    std::unique_lock<std::mutex> autoLock(m_dataqcMutx);

    // 重新初始化数据，因为目前update的add时，infos数据中的id不是真实值。
    for (const auto& upItem : infos)
    {
        switch (upItem.updateType)
        {
        case tf::UpdateType::UPDATE_TYPE_ADD:
        case tf::UpdateType::type::UPDATE_TYPE_MODIFY:
        {
            ::tf::QcDocQueryResp tempResp;
            ::tf::QcDocQueryCond tempCond;
            tempCond.__set_ids({ upItem.db });
            if (DcsControlProxy::GetInstance()->QueryQcDoc(tempResp, tempCond) && tempResp.lstQcDocs.size() > 0)
            {
                m_mapDocs[upItem.db] = std::make_shared<tf::QcDoc>(tempResp.lstQcDocs[0]);
            }
            m_qcDocIDs.insert(upItem.db);
            break;
        }
        case tf::UpdateType::UPDATE_TYPE_DELETE:
        {
            std::map<int64_t, std::shared_ptr<tf::QcDoc>>::iterator it = m_mapDocs.find(upItem.db);
            if (it != m_mapDocs.end())
            {
                m_mapDocs.erase(it);
            }
            std::set<int64_t>::iterator kIter = m_qcDocIDs.find(upItem.db);
            if (kIter != m_qcDocIDs.end())
            {
                m_qcDocIDs.erase(kIter);
            }
            break;
        }
        default:
            break;
        }
    }
}

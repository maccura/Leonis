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
/// @file     QIseCaliAppManager.cpp
/// @brief    Ise校准申请页面数据管理
///
/// @author   8090/YeHuaNing
/// @date     2022年12月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QIseCaliAppManager.h"

#include <QStandardItem>
#include "shared/ReagentCommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "manager/DictionaryQueryManager.h"

Q_DECLARE_METATYPE(::ise::tf::IseModuleInfo);

QIseCaliAppManager::QIseCaliAppManager()
{
}

QIseCaliAppManager::~QIseCaliAppManager()
{
}

///
/// @brief 获取ise设备的校准申请数据
///
/// @param[in]  devices  设备信息
///
/// @return 表格列表
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建函数
///
QList<QList<QStandardItem*>> QIseCaliAppManager::GetIseCaliItems(const std::vector<std::string>& devices)
{
    ULOG(LOG_INFO, "%s().", __FUNCTION__);

    const std::vector<::ise::tf::IseModuleInfo>& infos = GetIseModuleInfos(devices);
    items.clear();

    if (infos.empty())
        return QList<QList<QStandardItem*>>();
    
    for (const auto& info : infos)
    {
        ULOG(LOG_INFO, "info: %s.", ToString(info));

        QList<QStandardItem*> lineData;
        QStandardItem* itemPostion = GetItemPos(info);
        itemPostion->setData(QVariant::fromValue<::ise::tf::IseModuleInfo>(info), Qt::UserRole + ReagentInfo);      
        itemPostion->setData(info.id, Qt::UserRole + DatabaseId);
        itemPostion->setData(tf::AssayClassify::ASSAY_CLASSIFY_ISE, Qt::UserRole + AssayClassify);
        QString sotrPost = itemPostion->text().split('-')[0] + "-" + QString::number(info.moduleIndex).rightJustified(3, '0');
        itemPostion->setData(sotrPost, Qt::UserRole + DefaultSort);
        lineData.push_back(SetItemTextAligCenter(itemPostion));                                    // 位置
        lineData.push_back(SetItemTextAligCenter(GetItemName(info.sampleType)));                   // 项目名称
        lineData.push_back(CenterAligmentItem("-"));                                               // 试剂批号
        // 因禅道3059，和何洋老师商量，电解质使用状态从 "-" 更改为 "在用/校准屏蔽"
        lineData.push_back(SetItemTextAligCenter(GetModuleStatus(info)));                          // 使用状态
        lineData.push_back(SetItemTextAligCenter(GetCalibrateShowStatus(info.caliStatus, 
            info.caliCurveId > 0)));                                                               // 校准状态
        lineData.push_back(SetItemTextAligCenter(GetValidDays(info)));                             // 当前曲线有效期
        lineData.push_back(CenterAligmentItem("-"));                                                // 上机时间
        lineData.push_back(SetItemTextAligCenter(GetCaliBrateReason(info.caliReason)));            // 校准原因
        lineData.push_back(SetItemTextAligCenter(GetCaliBrateMethod(info.caliMode)));              // 执行方法

        if (info.caliMasked)
        {
            for (const auto& i : lineData)
            {
                //i->setForeground(QColor(Qt::gray));
                i->setData(QColor(Qt::gray), Qt::UserRole + 1);
            }
        }

        QStandardItem* item = new QStandardItem;
        item->setData(info.caliSelect);
        lineData.push_back(SetItemTextAligCenter(item));                                           // 选择

        items.push_back(lineData);
    }

    return items;
}

///
/// @brief 获取ise设备的校准申请数据,函数重载，可以根据样本类型获取
///
/// @param[in]  devices     设备信息
/// @param[in]  sampleType  样本类型
///
/// @return 表格列表
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月3日，新建函数
///
QList<QList<QStandardItem*>> QIseCaliAppManager::GetIseCaliItems(const std::vector<std::string>& devices, const ::ise::tf::SampleType::type& sampleType)
{
    ULOG(LOG_INFO, "%s().", __FUNCTION__);

	const std::vector<::ise::tf::IseModuleInfo>& infos = GetIseModuleInfos(devices, sampleType);
    items.clear();

	if (infos.empty())
		return QList<QList<QStandardItem*>>();

	for (const auto& info : infos)
    {
        ULOG(LOG_INFO, "info: %s.", ToString(info));

		QList<QStandardItem*> lineData;
		QStandardItem* itemPostion = GetItemPos(info);
		itemPostion->setData(QVariant::fromValue<::ise::tf::IseModuleInfo>(info), Qt::UserRole + ReagentInfo);
		itemPostion->setData(info.id, Qt::UserRole + DatabaseId);
		itemPostion->setData(tf::AssayClassify::ASSAY_CLASSIFY_ISE, Qt::UserRole + AssayClassify);
		lineData.push_back(SetItemTextAligCenter(itemPostion));                                    // 位置
		lineData.push_back(SetItemTextAligCenter(GetItemName(info.sampleType)));                   // 项目名称
		lineData.push_back(CenterAligmentItem("-"));                                               // 试剂批号
        // 因禅道3059，和何洋老师商量，电解质使用状态从 "-" 更改为 "在用/校准屏蔽"
        lineData.push_back(SetItemTextAligCenter(GetModuleStatus(info)));                          // 使用状态
		lineData.push_back(SetItemTextAligCenter(GetCalibrateShowStatus(info.caliStatus, 
            info.caliCurveId > 0)));                                                               // 校准状态
		lineData.push_back(SetItemTextAligCenter(GetValidDays(info)));                             // 当前曲线有效期
		lineData.push_back(CenterAligmentItem("-"));                                               // 上机时间
		lineData.push_back(SetItemTextAligCenter(GetCaliBrateReason(info.caliReason)));            // 校准原因
		lineData.push_back(SetItemTextAligCenter(GetCaliBrateMethod(info.caliMode)));              // 执行方法

        if (info.caliMasked)
        {
            for (const auto& i : lineData)
            {
                //i->setForeground(QColor(Qt::gray));
                i->setData(QColor(Qt::gray), Qt::UserRole + 1);
            }
        }

        QStandardItem* item = new QStandardItem;
        item->setData(info.caliSelect);
        lineData.push_back(SetItemTextAligCenter(item));                                           // 选择

		items.push_back(lineData);
	}

	return items;

}

///
/// @brief 更新校准状态信息
///
/// @param[in]  infos  变化的模块信息
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月3日，新建函数
///
void QIseCaliAppManager::UpdateCaliStatus(std::vector<ise::tf::IseModuleInfo>& infos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    // 本函数只针对模块信息不会突然增加或者减少的情况进行更新
    SortIseModuleInfos(infos);

    auto resetFunc = [this](QList<QStandardItem*>& item, int index, QStandardItem* newItem) {
        if (newItem == nullptr)
            return;

        if (index < item.size())
        {
            // 设置新值
            auto& oldItem = item.at(index);
            if (oldItem != nullptr)
            {
                oldItem->setText(newItem->text());
                // 是否设置了颜色
                //oldItem->setData(newItem->data(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS), Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
                //oldItem->setData(newItem->data(Qt::TextColorRole), Qt::TextColorRole);
                oldItem->setData(newItem->data(Qt::UserRole + 1), Qt::UserRole + 1);
            }
                
        }

        delete newItem;
    };

    for (const auto& info : infos)
    {
        ULOG(LOG_INFO, "info is %s", ToString(info));
        for (auto& item : items)
        {
            const auto& iseModuleInfo = item[0]->data(Qt::UserRole + ReagentInfo).value<ise::tf::IseModuleInfo>();
            if ((info.deviceSn == iseModuleInfo.deviceSn) &&
                (info.moduleIndex == iseModuleInfo.moduleIndex) &&
                (info.sampleType == iseModuleInfo.sampleType))
            {
                // 先重置一次颜色
                for (const auto& it : item)
                {
                    //it->setData(item.at(9)->data(Qt::TextColorRole), Qt::TextColorRole);
                    it->setData(QVariant());
                }

                // 更新自定义数据
                item.at(0)->setData(QVariant::fromValue<::ise::tf::IseModuleInfo>(info), Qt::UserRole + ReagentInfo);

                // 更新使用状态
                resetFunc(item, 3, GetModuleStatus(info));

                // 更新校准状态
                resetFunc(item, 4, GetCalibrateShowStatus(info.caliStatus, info.caliCurveId > 0));

                //更新曲线有效期
                resetFunc(item, 5, GetValidDays(info));

                //更新校准原因
                resetFunc(item, 7, GetCaliBrateReason(info.caliReason));

                // 更新执行方法
                resetFunc(item, 8, GetCaliBrateMethod(info.caliMode));

                if (info.caliMasked)
                {
                    int loopi = 0;
                    for (const auto& it : item)
                    {
                        if (++loopi >= item.size())
                            continue;
                        //it->setForeground(Qt::gray);
                        it->setData(QColor(Qt::gray), Qt::UserRole + 1);
                    }
                }
                /*else if(iseModuleInfo.caliMasked && !info.caliMasked)
                {
                    for (const auto& it : item)
                    {
                        it->setForeground(item.at(3)->foreground());
                        it->setData(item.at(3)->data(Qt::UserRole + 1), Qt::UserRole + 1);
                    }
                }*/

                //更新选择状态
                item.at(9)->setData(info.caliSelect);
                
                break;
            }
        }
    }
    
}

///
/// @brief 返回设备列表对应的ise模块信息
///
/// @param[in]  devices  设备列表
///
/// @return 模块信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建函数
///
std::vector<::ise::tf::IseModuleInfo> QIseCaliAppManager::GetIseModuleInfos(const std::vector<std::string>& devices)
{
    ::ise::tf::IseModuleInfoQueryCond qryCond;
    ::ise::tf::IseModuleInfoQueryResp qryResp;

    qryCond.__set_deviceSn(devices);

    ::ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
    if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryIseModuleInfo failed.");
        return std::vector<::ise::tf::IseModuleInfo>();
    }

    // 如果结果为空则返回
    if (qryResp.lstIseModuleInfos.empty())
    {
        ULOG(LOG_INFO, "qryResp.lstReagentInfos.empty().");
        return std::vector<::ise::tf::IseModuleInfo>();
    }

    SortIseModuleInfos(qryResp.lstIseModuleInfos);

    return qryResp.lstIseModuleInfos;
}

///
/// @brief 返回设备列表对应的ise模块信息
///        函数重载，可以根据样本类型获取
///
/// @param[in]  devices     设备列表
/// @param[in]  sampleType  样本类型
///
/// @return 模块信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月3日，新建函数
///
std::vector<::ise::tf::IseModuleInfo> QIseCaliAppManager::GetIseModuleInfos(const std::vector<std::string>& devices, const ::ise::tf::SampleType::type& sampleType)
{
	::ise::tf::IseModuleInfoQueryCond qryCond;
	::ise::tf::IseModuleInfoQueryResp qryResp;

	qryCond.__set_deviceSn(devices);
	qryCond.__set_sampleType(sampleType);
	
	::ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QueryIseModuleInfo failed.");
		return std::vector<::ise::tf::IseModuleInfo>();
	}

	// 如果结果为空则返回
	if (qryResp.lstIseModuleInfos.empty())
	{
		ULOG(LOG_INFO, "qryResp.lstReagentInfos.empty().");
		return std::vector<::ise::tf::IseModuleInfo>();
	}

	qStableSort(qryResp.lstIseModuleInfos.begin(), qryResp.lstIseModuleInfos.end(), [](const ::ise::tf::IseModuleInfo &s1, const ::ise::tf::IseModuleInfo &s2) {


		if (s1.deviceSn == s2.deviceSn)
		{
			if (s1.moduleIndex == s2.moduleIndex)
			{
				return s1.sampleType < s2.sampleType;
			}

			return  s1.moduleIndex < s2.moduleIndex;
		}

		return s1.deviceSn < s2.deviceSn;
	});

	return qryResp.lstIseModuleInfos;

}

///
/// @brief 获取位置列数据
///
/// @param[in]  info  模块信息
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建函数
///
QStandardItem * QIseCaliAppManager::GetItemPos(const::ise::tf::IseModuleInfo & info)
{
    const auto& devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(info.deviceSn);

    QString moduleName;
    if (devInfo != nullptr && devInfo->moduleCount > 1)
        moduleName = GetIseModuleName(info.moduleIndex);

    QString postion = QString::fromStdString(CommonInformationManager::GetDeviceName(info.deviceSn))  + moduleName;
    return new QStandardItem(postion);
}

///
/// @brief 返回样本类型列数据
///
/// @param[in]  type  样本类型
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建函数
///
QStandardItem * QIseCaliAppManager::GetItemName(const ::ise::tf::SampleType::type& type)
{
    QStandardItem* item = new QStandardItem;
    const QString& name = GetIseSampleName(type);

    if (!name.isEmpty())
    {
        item->setText("ISE-" + name);
    }

    return item;
}

QStandardItem * QIseCaliAppManager::GetModuleStatus(const::ise::tf::IseModuleInfo & info)
{
    // 因禅道3059，和何洋老师商量，电解质使用状态从 "-" 更改为 "在用/校准屏蔽"
    if (info.caliMasked)
        return new QStandardItem(tr("校准屏蔽"));

    return new QStandardItem(tr("在用"));
}

///
/// @brief 获取曲线的有限期
///
/// @param[in]  curveId  曲线id
///
/// @return 当前曲线有效期
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月26日，新建函数
///
QStandardItem * QIseCaliAppManager::GetValidDays(const ::ise::tf::IseModuleInfo& info)
{
    QStandardItem* item = new QStandardItem;
    // 先补充至10位数值，不足的部分补0
    item->setData(QString("9999999999").rightJustified(10, '0'), Qt::UserRole + 3);
    //item->setData(QVariant(), Qt::TextColorRole);
    //SetItemColor(item, UI_REAGENT_WARNCOLOR);

    if (info.caliCurveId < 0)
        return item;

    //查询曲线详细数据
    ::ise::tf::IseCaliCurveQueryResp qryResp;
    ::ise::tf::IseCaliCurveQueryCond qryCond;
    qryCond.__set_id(info.caliCurveId);

    ::ise::ise1005::LogicControlProxy::QueryIseCaliCurve(qryResp, qryCond);
    if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryIseCaliCurve failed.");
        return item;
    }

    // 如果结果为空则返回
    if (qryResp.lstIseCaliCurves.empty())
    {
        ULOG(LOG_INFO, "qryResp.lstIseCaliCurves.empty().");
        return item;
    }

    auto spIseAssayMap = CommonInformationManager::GetInstance()->GetIseAssayIndexCodeMaps();

    // 如果取不到，则直接为空
    if (spIseAssayMap.empty() || spIseAssayMap.begin()->second == nullptr)
        return item;

    // 电解质项目的有效期都是预置到数据库，有效时长都是一样的，且不能修改
    // 所以此处只需要获取Na K Cl其中一个就有效期可以了
    int timeout = spIseAssayMap.begin()->second->caliSuggest.timeoutCali.timeout;
    if (timeout > 0)
    {
        tf::TimeUnit::type timeUnitType = spIseAssayMap.begin()->second->caliSuggest.timeoutCali.timeUnit;

        int hours = 0;

        //统一转为小时，计算过期时间
        if (timeUnitType == ::tf::TimeUnit::TIME_UNIT_DAY)
        {
            hours = timeout * 24;
        }
        else if (timeUnitType == ::tf::TimeUnit::TIME_UNIT_WEEK)
        {
            hours = timeout * 7 * 24;
        }
        else
        {
            hours = timeout;
        }

        boost::posix_time::ptime  endTime = boost::posix_time::ptime(
            boost::posix_time::time_from_string(qryResp.lstIseCaliCurves[0].caliTime)) + boost::posix_time::hours(hours);
        boost::posix_time::ptime  curTime = boost::posix_time::second_clock::local_time();

        // 设置日期大于0的才认为是正常值
        if (endTime > curTime)
        {
            boost::posix_time::time_duration duration = endTime - curTime;

            // 过期时间按照24小时一天，并且向上取整（参照bug1315)
            int daysValid = ceil((1.0 *  duration.total_seconds()) / (3600 * 24));

            item->setText(QString::number(daysValid <= 0 ? 0 : daysValid));
            item->setData(QString::fromStdString(PosixTimeToTimeString(endTime)), Qt::UserRole + 10);
            if (daysValid <= 0 && DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
            {
                //SetItemColor(item, UI_REAGENT_WARNCOLOR);
                //item->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
                item->setData(QColor(UI_REAGENT_NOTEFONT), Qt::UserRole + 1);
            }
        }
        else
        {
            item->setText("0");
            if (DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
            {
                //SetItemColor(item, UI_REAGENT_WARNCOLOR);
                //item->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
                item->setData(QColor(UI_REAGENT_NOTEFONT), Qt::UserRole + 1);
            }
        }
    }

    // 设置一个用户数据，用于排序
    //auto&& list = item->text().split(":");
    //if (list.size() == 2)
    //{
    //    QString sortData;
    //    // 先补充至10位数值，不足的部分补0
    //    sortData = list[0] + list[1].rightJustified(10, '0');
    //    item->setData(sortData);
    //}
    //else
        item->setData(item->text().rightJustified(10, '0'), Qt::UserRole + 3);

    return item;
}

///
/// @brief 对模块信息排队
///
/// @param[in]  infos  模块信息
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2023年1月3日，新建函数
///
void QIseCaliAppManager::SortIseModuleInfos(std::vector<ise::tf::IseModuleInfo>& infos)
{
    if (infos.size() <= 1)
        return;

    qStableSort(infos.begin(), infos.end(), [](const ::ise::tf::IseModuleInfo &s1, const ::ise::tf::IseModuleInfo &s2) {
        if (s1.deviceSn == s2.deviceSn)
        {
            if (s1.moduleIndex == s2.moduleIndex)
            {
                return s1.sampleType < s2.sampleType;
            }

            return  s1.moduleIndex < s2.moduleIndex;
        }

        return s1.deviceSn < s2.deviceSn;
    });
}

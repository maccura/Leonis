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
/// @file     ReagentCommon.h
/// @brief    试剂通用文档
///
/// @author   5774/WuHongTao
/// @date     2021年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ReagentCommon.h"
#include <QVariant>
#include <QDateTime>
#include <QStandardItemModel>
#include <QMap>

#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/ThriftEnumTransform.h"
#include "reagent/rgntnoticedatamng.h"

#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"

#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ConfigSerialize.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"

// 特殊清洗液编号
static const int                                        s_ciSpecialWashNum = 10;
static QMap<QString, RgntChangeApplyStatus>             s_mapDevApplyChangeRgnt;

///
/// @brief
///     获取试剂校准状态
///
/// @param[in]  reagentInfo  试剂信息
/// @param[in]  existCurveId 存在校准曲线
///
/// @return 试剂校准状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数 caliStatus
///
QStandardItem* GetCalibrateShowStatus(::tf::CaliStatus::type reagentCaliStatus, bool existCurveId, QStandardItem* item)
{
    if (item == nullptr)
        item = new QStandardItem();

    RowCellInfo rowInfo;
    MakeCaliShowStatus(reagentCaliStatus, existCurveId, rowInfo);

    item->setText(rowInfo.m_text);
    //SetItemColor(item, rowInfo.m_backgroudColor.isEmpty() ? QVariant() : rowInfo.m_backgroudColor);
    //item->setData(rowInfo.m_fontColor.isEmpty() ? QVariant() : QColor(rowInfo.m_fontColor), Qt::TextColorRole);
    item->setData(rowInfo.m_fontColor.isEmpty() ? QVariant() : QColor(rowInfo.m_fontColor), Qt::UserRole + 1);

    return item;
}

void MakeCaliShowStatus(::tf::CaliStatus::type reagentCaliStatus, bool existCurveId, RowCellInfo& cellInfo)
{
    cellInfo.m_text = ThriftEnumTrans::GetCalibrateStatus(reagentCaliStatus);

	cellInfo.m_backgroudColor.clear();
    if (reagentCaliStatus == tf::CaliStatus::CALI_STATUS_FAIL)
    {
        //设置颜色为红色
        cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
    }

    if (reagentCaliStatus == tf::CaliStatus::CALI_STATUS_NOT)
    {
        if (!existCurveId)
        {
            //设置颜色为红色
            cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
        }
        else
        {
            cellInfo.m_text = "";
        }
    }
}

///
/// @brief 
///     获取校准剩余有效天数
///
/// @param[in]  reagentInfo  
///
/// @return 校准有效天数控件
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月2日，新建函数
///
QStandardItem * GetCaliCurveExpirateDate(const ch::tf::ReagentGroup& reagentInfo, QStandardItem* item)
{
    if (item == nullptr)
	    item = new QStandardItem();

    RowCellInfo cellInfo;
    const QString& expirateData = MakeCaliCurveExpirateDate(reagentInfo, cellInfo);

    item->setData(cellInfo.m_dataExt, Qt::UserRole + 3);
    //SetItemColor(item, cellInfo.m_backgroudColor.isEmpty() ? QVariant() : cellInfo.m_backgroudColor);
    //item->setData(cellInfo.m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo.m_fontColor), Qt::TextColorRole);
    item->setData(cellInfo.m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo.m_fontColor), Qt::UserRole + 1);
    item->setText(cellInfo.m_text);
    if (!expirateData.isEmpty())
        item->setData(expirateData, Qt::UserRole + 10);
    else
        item->setData(QVariant(), Qt::UserRole + 10);

	return item;
}

QString MakeCaliCurveExpirateDate(const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo)
{
    QString expirateData;
    // 校准曲线--是否存在
    if (!reagentInfo.__isset.caliCurveId || reagentInfo.caliCurveId <= 0)
    {
        return expirateData;
    }

    // 获取校准曲线
    ::ch::tf::CaliCurveQueryCond queryCurve;
    queryCurve.__set_id(reagentInfo.caliCurveId);
    ::ch::tf::CaliCurveQueryResp carveResult;
    ch::c1005::LogicControlProxy::QueryCaliCurve(carveResult, queryCurve);
    if (carveResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || carveResult.lstCaliCurves.empty())
    {
        return expirateData;
    }

    // 如果是编辑的曲线，有效期直接返回空
    if (carveResult.lstCaliCurves[0].isEditCurve) {
        return expirateData;
    }

    if (carveResult.lstCaliCurves[0].caliMode == ::tf::CaliMode::CALI_MODE_INTELLIGENT)
    {
        cellInfo.m_text = "AC";
        return expirateData;
    }

    QString type;
    GetCalibrateCurveTypeBrif(reagentInfo.caliCurveSource, type);

    if (!type.isEmpty() && reagentInfo.caliCurveSource != tf::CurveSource::CURVE_SOURCE_ASSAY)
    {
        QString caliTime = QString::fromStdString(carveResult.lstCaliCurves[0].caliTime);
        //GetCaliTime(reagentInfo, caliTime);

        if (caliTime.isEmpty())
        {
            cellInfo.m_text = type;
        }
        else
        {
            // 判空操作很有必要（根据设备sn获取设备的详细信息）
            auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(reagentInfo.deviceSN);
            if (device == nullptr)
            {
                return expirateData;
            }

            // 判空操作很有必要（获取生化项目的配置详细信息）
            auto spChAssay = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(reagentInfo.assayCode, device->deviceType);
            if (spChAssay == nullptr)
            {
                return expirateData;
            }

            // 如果没有设置曲线校准，就只显示曲线类型
            if (spChAssay->caliSuggest.timeoutLot.timeout <= 0 &&
                spChAssay->caliSuggest.timeoutBottle.timeout <= 0)
            {
                // 设置一个用户数据，用于排序
                // 先补充至10位数值，不足的部分补0
                cellInfo.m_dataExt = (type + QString("9999999999").rightJustified(10, '0'));
                cellInfo.m_text = type;
                return expirateData;
            }

            int32_t timeout = 0;
            ::tf::TimeUnit::type timeUnitType;

            // 获取过期时间（优先获取对应类型的曲线，否则取另外一种）和时间单位
            if (reagentInfo.caliCurveSource == tf::CurveSource::CURVE_SOURCE_LOT && spChAssay->caliSuggest.timeoutLot.timeout > 0)
            {
                timeout = spChAssay->caliSuggest.timeoutLot.timeout;
                timeUnitType = spChAssay->caliSuggest.timeoutLot.timeUnit;
            }
            else if (reagentInfo.caliCurveSource == tf::CurveSource::CURVE_SOURCE_BOTTLE && spChAssay->caliSuggest.timeoutBottle.timeout > 0)
            {
                timeout = spChAssay->caliSuggest.timeoutBottle.timeout;
                timeUnitType = spChAssay->caliSuggest.timeoutBottle.timeUnit;
            }

            if (timeout == 0)
            {
                // 设置一个用户数据，用于排序
                // 先补充至10位数值，不足的部分补0
                cellInfo.m_dataExt = (type + QString("9999999999").rightJustified(10, '0'));
                cellInfo.m_text = type;
                return expirateData;
            }

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
                boost::posix_time::time_from_string(caliTime.toStdString())) + boost::posix_time::hours(hours);
            boost::posix_time::ptime  curTime = boost::posix_time::microsec_clock::local_time();

            // 设置日期大于0的才认为是正常值
            if (endTime > curTime)
            {
                boost::posix_time::time_duration duration = endTime - curTime;
                expirateData = QString::fromStdString(PosixTimeToTimeString(endTime));

                // 过期时间按照24小时一天，并且向上取整（参照bug1315)
                int daysValid = ceil((1.0 *  duration.total_seconds()) / (3600 * 24));

                // 过期时间超过最大期限，不显示
                /*if ((duration.total_seconds() / 3600) > hours)
                {
                    cellInfo.m_text = type;
                }
                else*/
                {
                    cellInfo.m_text = type + ":" + QString::number(daysValid <= 0 ? 0 : daysValid);
                }
            }
            else
            {
                cellInfo.m_text = type + ":0";
                // 备注:2024/06/27 根据免疫Mantis29845,和周晓锋确认，只是不显示红色颜色，不是不显示过期天数
                if (DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
                {
                    cellInfo.m_backgroudColor = UI_REAGENT_NOTECOLOR;
                    cellInfo.m_fontColor = UI_REAGENT_NOTEFONT;
                }
                cellInfo.m_dataExt = "0";
            }
        }
    }
    // 项目曲线不计算有效期，直接此处显示
    else
    {
        cellInfo.m_text = type;
    }

    // 设置一个用户数据，用于排序
    auto&& list = cellInfo.m_text.split(":");
    if (list.size() == 2)
    {
        // 先补充至10位数值，不足的部分补0
        QString sortData = list[0] + list[1].rightJustified(10, '0');
        cellInfo.m_dataExt = sortData;
    }
    else
    {
        cellInfo.m_dataExt = cellInfo.m_text;
    }

    return expirateData;
}

///
/// @brief 根据位置号获取位置控件
///
/// @param[in]  deviceSn		设备编号
/// @param[in]  postionNumber   位置编号
///
/// @return 位置控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月13日，新建函数
///
QStandardItem* GetItemFromPostionAndDevice(const std::string& deviceSn, int postionNumber, QStandardItem* item)
{
    if (item == nullptr)
        item = new QStandardItem;
    item->setText(MakePositionString(deviceSn, postionNumber));

	return item;
}

QString MakePositionString(const std::string& deviceSn, int postionNumber)
{
    return QString::fromStdString(CommonInformationManager::GetDeviceName(deviceSn)) + QStringLiteral("-") + QString::number(postionNumber);
}

///
/// @brief
///     通过设备和位置号获取文本
///
/// @param[in]  deviceSn        设备序列号
/// @param[in]  postionNumber   位置号
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
QString GetTextFromPostionAndDevice(const std::string& deviceSn, int postionNumber)
{
    QString spliterSymb = /*postionNumber < 10 ? QStringLiteral("-0") :*/ QStringLiteral("-");
    QString postion = QString::fromStdString(CommonInformationManager::GetDeviceName(deviceSn)) + spliterSymb + QString::number(postionNumber);
    return postion;
}

QStandardItem* GetSupplyShowStatus(tf::PlaceStatus::type placeStat, tf::UsageStatus::type useStat, int supCode,
	int backupTotal, int backNum, bool iseResidualZero)
{
    QStandardItem* item = new QStandardItem();
    RowCellInfo cellInfo;
    MakeSupplyShowStatus(cellInfo, placeStat, useStat, supCode, backupTotal, backNum, iseResidualZero);

    item->setText(cellInfo.m_text);
    if (!cellInfo.m_backgroudColor.isEmpty())
    {
        SetItemColor(item, cellInfo.m_backgroudColor);
    }
    if (!cellInfo.m_fontColor.isEmpty())
    {
        item->setData(QColor(cellInfo.m_fontColor), Qt::TextColorRole);
    }

    return item;
}

void MakeSupplyShowStatus(RowCellInfo& cellInfo, tf::PlaceStatus::type placeStat, tf::UsageStatus::type useStat, int supCode, 
	int backupTotal /*= -1*/, int backNum /*= -1*/, bool iseResidualZero/* = false*/)
{
	// 无效试剂
	if (supCode <= 0)
	{
		cellInfo.m_text = QObject::tr("无效试剂");
		cellInfo.m_backgroudColor = UI_REAGENT_WARNCOLOR; //设置颜色为红色
		cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
	}
	// 注册但未放置
	else if (placeStat == tf::PlaceStatus::PLACE_STATUS_REGISTER)
	{
		cellInfo.m_text = QObject::tr("未放置");
		cellInfo.m_backgroudColor = UI_REAGENT_WARNCOLOR;
		cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
	}
	// ise余量为0 不可用
	else if (iseResidualZero)
	{
		cellInfo.m_text = "";
	}
    else
    { 
        // 空显示不标色
        switch (useStat)
        {
        case tf::UsageStatus::USAGE_STATUS_CURRENT:
            cellInfo.m_text = QObject::tr("在用");
            break;
        case tf::UsageStatus::USAGE_STATUS_BACKUP:
            cellInfo.m_text = (backupTotal == -1) ? QObject::tr("备用") : QString(QObject::tr("备用%1-%2")).arg(backupTotal).arg(backNum);
            break;
        case tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE: // 不可用（空）
            cellInfo.m_text = "";
            break;
        default:
            ULOG(LOG_WARN, "Unknown supplyInfo.usageStatus.");
            break;
        }
    }
}

///
/// @brief
///     根据耗材和试剂组状态给出显示试剂状态
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂显示状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
QStandardItem* GetReagentShowStatus(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagentInfo, QStandardItem* item)
{
    if (item == nullptr)
    {
        item = new QStandardItem();
    }
    RowCellInfo cellInfo;
    MakeReagentStatusInfo(supplyInfo, reagentInfo, cellInfo);

    item->setText(cellInfo.m_text);
    //SetItemColor(item, cellInfo.m_backgroudColor.isEmpty() ? QVariant() : cellInfo.m_backgroudColor);
    //item->setData(cellInfo.m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo.m_fontColor), Qt::TextColorRole);
    item->setData(cellInfo.m_fontColor.isEmpty() ? QVariant() : QColor(cellInfo.m_fontColor), Qt::UserRole + 1);

    return item;
}

void MakeReagentStatusInfo(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo)
{
	cellInfo.m_backgroudColor.clear();

    // 注册但未放置/已卸载
    if (supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_REGISTER ||
        supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_UNLOAD)
    {
        cellInfo.m_text = QObject::tr("未放置");
        cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
    }
    // 试剂屏蔽
    else if (reagentInfo.reagentMask)
    {
        cellInfo.m_text = QObject::tr("试剂屏蔽");
    }
    // 测试要求余量为0的时候，显示状态为空
    else if (reagentInfo.remainCount <= 0)
    {
        cellInfo.m_text = "";
    }
    // 校准屏蔽
    else if (reagentInfo.caliMask)
    {
        cellInfo.m_text = QObject::tr("校准屏蔽");
    }
    // 当前试剂
    else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_CURRENT)
    {
        cellInfo.m_text = QObject::tr("在用");
    }
    // 备用试剂
    else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_BACKUP)
    {
        cellInfo.m_text = QString(QObject::tr("备用%1-%2")).arg(reagentInfo.backupTotal).arg(reagentInfo.backupNum);
    }
    // 不可用（空）
    else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE)
    {
        //cellInfo.m_text = QObject::tr("不可用");
        //cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
    }
}

///
/// @brief
///     获取试剂有效期的状态控件
//////////////////////////////////////////////////////////////////////////
/// @param[in]  reagentInfo  试剂信息
///
/// @return 状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
QStandardItem* GetReagenExpiredStatus(const ch::tf::ReagentGroup& reagentInfo)
{
    QStandardItem* itemExpired = new QStandardItem();

    RowCellInfo cellInfo;
    MakeReagenExpiredStatus(reagentInfo, cellInfo);

    if (!cellInfo.m_backgroudColor.isEmpty())
    {
        SetItemColor(itemExpired, cellInfo.m_backgroudColor);
    }
    if (!cellInfo.m_fontColor.isEmpty())
    {
        itemExpired->setData(QColor(cellInfo.m_fontColor), Qt::TextColorRole);
    }
    itemExpired->setText(cellInfo.m_text);

    return itemExpired;
}

void MakeReagenExpiredStatus(const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo)
{
	// 失效日期
	if (ChSuppliesExpire(reagentInfo.expiryTime))
	{
		//cellInfo.m_backgroudColor = UI_REAGENT_WARNCOLOR;
		cellInfo.m_fontColor = UI_REAGENT_WARNFONT;
	}

    cellInfo.m_text = QString::fromStdString(ConverStdStringToDateString(reagentInfo.expiryTime));
}

///
/// @brief
///     获取校准曲线类型
///
/// @param[in]  reagentSourceType  校准曲线类型枚举
///
/// @return 试剂校准曲线类型控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
QStandardItem* TranslateCalibrateCurveType(::tf::CurveSource::type reagentSourceType)
{
	QString showMsg;
	QStandardItem* item = new QStandardItem();

    switch (reagentSourceType)
    {
    case tf::CurveSource::CURVE_SOURCE_ASSAY: // 项目继承
        showMsg = QObject::tr("项目曲线");
        break;
    case tf::CurveSource::CURVE_SOURCE_LOT: // 批继承
        showMsg = QObject::tr("批曲线");
        break;
    case tf::CurveSource::CURVE_SOURCE_BOTTLE: // 瓶曲线
        showMsg = QObject::tr("瓶曲线");
        break;
    case tf::CurveSource::CURVE_SOURCE_NONE: // 未校准
        showMsg = QObject::tr("未校准");
        SetItemColor(item, UI_REAGENT_WARNCOLOR);
        item->setData(QColor(UI_REAGENT_WARNFONT), Qt::TextColorRole);
        break;
    default:
        ULOG(LOG_WARN, "Unknown caliCurve type.");
        break;
    }

	item->setText(showMsg);
	return item;
}

///
/// @brief 
///     获取校准曲线类型简写
///
/// @param[in]  reagentCurveType  校准曲线类型枚举
/// @param[out]  type		      曲线简写字母
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月1日，新建函数 caliCurveSource
///
void GetCalibrateCurveTypeBrif(::tf::CurveSource::type reagentCurveType, QString& type)
{
    switch (reagentCurveType)
    {
    case tf::CurveSource::CURVE_SOURCE_ASSAY: // 项目继承
        type = "T";
        break;
    case tf::CurveSource::CURVE_SOURCE_LOT: // 批继承
        type = "L";
        break;
    case tf::CurveSource::CURVE_SOURCE_BOTTLE: // 瓶曲线
        type = "B";
        break;
    default: // 其他情况不显示曲线类型
        type = "";
        break;
    }
}

///
/// @brief 获取ise样本类型名称
///
/// @param[in]  type  样本类型
///
/// @return 样本名称
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月27日，新建函数
///
QString GetIseSampleName(const ::ise::tf::SampleType::type& type)
{
    static QMap<::ise::tf::SampleType::type, QString> sampleNameMap = {
        { ::ise::tf::SampleType::Serum, "S" },
        { ::ise::tf::SampleType::Urine, "U" }
    };

    if (sampleNameMap.contains(type))
    {
        return QString(sampleNameMap[type]);
    }

    return QString();
}

///
/// @brief 将ise模块索引转为名称
///
/// @param[in]  moduleIndex  模块索引
///
/// @return 模块名称
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月27日，新建函数
///
QString GetIseModuleName(int moduleIndex)
{
    // 模块索引(大于等于1)
    if (moduleIndex < 1)
        return QString();

    // 自减（方便进行字母转换计算）
    moduleIndex--;

    QString name;
    char ch = 'A' + moduleIndex;
    int loop = moduleIndex / 26;

    while ( (moduleIndex / 26) > 0)
    {
        name.append('A');
        moduleIndex -= 26;

    }

    name += ('A' + moduleIndex % 26);

    return name;
}

///
/// @brief 获取ISE的名称
///
/// @param[in]  supplyInfo  耗材信息
///
/// @return 名称
///
/// @par History:
/// @li 5774/WuHongTao，2023年5月12日，新建函数
///
QString GetIseName(const ise::tf::SuppliesInfo& supplyInfo)
{
    auto spDeivce = CommonInformationManager::GetInstance()->GetDeviceInfo(supplyInfo.deviceSN);
    if (spDeivce == nullptr)
    {
        return "";
    }

    // 主设备名称
    auto name = spDeivce->groupName + spDeivce->name;
    
    if (spDeivce->moduleCount <= 1)
    {
        return QString::fromStdString(name);
    }
    else
    {
        return (QString::fromStdString(name) + GetIseModuleName(supplyInfo.moduleIndex));
    }
}

///
/// @brief 获取校准原因
///
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂校准原因控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliBrateReason(const tf::CaliReason::type& caliReason, QStandardItem* item)
{
    if (item == nullptr)
	    item = new QStandardItem("");
	item->setText(GetCaliBrateReasonInfo(caliReason));
	return item;
}

QString GetCaliBrateReasonInfo(const tf::CaliReason::type& caliReason)
{
	QString showMsg;
	// 校准失败
	if (caliReason == tf::CaliReason::CALI_REASON_CALI_FAIL)
	{
		showMsg = QObject::tr("自动校准");
	}
	// 手动校准
	else if (caliReason == tf::CaliReason::CALI_REASON_MANUAL)
	{
		showMsg = QObject::tr("手动校准");
	}
	// 无
	else if (caliReason == tf::CaliReason::CALI_REASON_NONE)
	{
		showMsg = QObject::tr("");
	}
	// 质控失败
	else if (caliReason == tf::CaliReason::CALI_REASON_QC_FAIL)
	{
		showMsg = QObject::tr("自动校准");
	}
	// 试剂换瓶
	else if (caliReason == tf::CaliReason::CALI_REASON_REAGENT_CHANGE_BOTTLE)
	{
		showMsg = QObject::tr("自动校准");
	}
	// 试剂换批
	else if (caliReason == tf::CaliReason::CALI_REASON_REAGENT_CHANGE_LOT)
	{
		showMsg = QObject::tr("自动校准");
	}
	// 超时
	else if (caliReason == tf::CaliReason::CALI_REASON_CURVE_TIMEOUT)
	{
		showMsg = QObject::tr("自动校准");
	}

	return showMsg;
}

///
/// @brief 获取校准时间
///
/// @param[in]  reagentInfo  试剂组信息
///
/// @return 校准时间控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliTime(const ch::tf::ReagentGroup& reagentInfo)
{
	QStandardItem* item =  new QStandardItem("");
	if (reagentInfo.caliStatus == tf::CaliStatus::CALI_STATUS_NOT
		||
		reagentInfo.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL
		)
	{
		return item;
	}

	// 校准曲线--是否存在
	if (!reagentInfo.__isset.caliCurveId || reagentInfo.caliCurveId <= 0)
	{
		return item;
	}

	// 获取校准曲线
	::ch::tf::CaliCurveQueryCond queryCurve;
	queryCurve.__set_id(reagentInfo.caliCurveId);
	::ch::tf::CaliCurveQueryResp carveResult;
	ch::c1005::LogicControlProxy::QueryCaliCurve(carveResult, queryCurve);
	if (carveResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || carveResult.lstCaliCurves.empty())
	{
		return item;
	}

	// 设置校准时间
	QString caliTime = QString::fromStdString(carveResult.lstCaliCurves.front().caliTime);
	item->setText(caliTime);
	return item;
}

///
/// @brief 
///     获取校准时间
///
/// @param[in]  reagentInfo  试剂组信息
/// @param[out]  caliTime  校准时间
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月1日，新建函数
///
void GetCaliTime(const ch::tf::ReagentGroup& reagentInfo, QString& caliTime)
{
	// 校准曲线--是否存在
	if (!reagentInfo.__isset.caliCurveId || reagentInfo.caliCurveId <= 0)
	{
		return;
	}

	// 获取校准曲线
	::ch::tf::CaliCurveQueryCond queryCurve;
	queryCurve.__set_id(reagentInfo.caliCurveId);
	::ch::tf::CaliCurveQueryResp carveResult;
	ch::c1005::LogicControlProxy::QueryCaliCurve(carveResult, queryCurve);
	if (carveResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS || carveResult.lstCaliCurves.empty())
	{
		return;
	}

	// 设置校准时间
	caliTime = QString::fromStdString(carveResult.lstCaliCurves.front().caliTime);
}

///
/// @brief 校准方法
///
/// @param[in]  caliMode  校准方法的枚举
///
/// @return 试剂校准方法控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliBrateMethod(tf::CaliMode::type caliMode, QStandardItem* item)
{
    if (item == nullptr)
	    item = new QStandardItem("");
	item->setText(GetCaliBrateMethodInfo(caliMode));
	return item;
}

QString GetCaliBrateMethodInfo(const tf::CaliMode::type caliMode)
{
	QString showMsg;
	// 空白
	if (caliMode == tf::CaliMode::CALI_MODE_BLANK)
	{
		showMsg = QObject::tr("空白");
	}
	// 全点
	else if (caliMode == tf::CaliMode::CALI_MODE_FULLDOT)
	{
		showMsg = QObject::tr("全点");
	}
	// 无
	else if (caliMode == tf::CaliMode::CALI_MODE_NONE)
	{
		showMsg = QObject::tr("无");
	}
	// span
	else if (caliMode == tf::CaliMode::CALI_MODE_SPAN_POINT)
	{
		showMsg = QObject::tr("量程点");
	}
	// 两点
	else if (caliMode == tf::CaliMode::CALI_MODE_TWO_POINT)
	{
		showMsg = QObject::tr("两点");
	}

	return showMsg;
}

///
/// @brief 根据试剂信息获取耗材信息
///
/// @param[in]  reagentGroup  试剂信息
///
/// @return 耗材对象，找不到为空
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月15日，新建函数
///
std::shared_ptr<ch::tf::SuppliesInfo> GetSupplyInfoByReagent(const ch::tf::ReagentGroup& reagentGroup)
{
	std::shared_ptr<ch::tf::SuppliesInfo> supplyInfo = nullptr;
	// 查询不同类型的耗材
	ch::tf::SuppliesInfoQueryCond qryCond;
	ch::tf::SuppliesInfoQueryResp qryResp;

	if (reagentGroup.reagentKeyInfos.empty())
	{
		return supplyInfo;
    }

    // 耗材编号
    qryCond.__set_suppliesCode(reagentGroup.suppliesCode);
	// 批号信息
	qryCond.__set_lot(reagentGroup.reagentKeyInfos[0].lot);
	// 瓶号信息
	qryCond.__set_sn(reagentGroup.reagentKeyInfos[0].sn);
	// 设置设备信息
	qryCond.__set_deviceSN(std::vector<std::string>(1, reagentGroup.deviceSN));

	if (!ch::c1005::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| qryResp.lstSuppliesInfo.empty())
	{
		return supplyInfo;
	}

	// 耗材信息
	supplyInfo = std::make_shared<ch::tf::SuppliesInfo>(qryResp.lstSuppliesInfo[0]);
	return supplyInfo;
}

///
/// @brief 根据项目编号获取所有的试剂信息
///
/// @param[in]  assaycode  
///
/// @return 试剂信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
std::vector<ch::tf::ReagentGroup> GetReagentGroupByAssayCode(int assaycode)
{
	ULOG(LOG_INFO, "%s(assayCode : %d)", __FUNCTION__, assaycode);
	std::vector<ch::tf::ReagentGroup> chReagentGroups;
	// 获取设备信息表
	auto deviceMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
	std::vector<std::string> deviceSns;
	for (const auto& device : deviceMap)
	{
		// 必须排除被屏蔽的设备
		if (device.second->__isset.masked && device.second->masked)
		{
			continue;
		}

		// 只查询生化设备的信息
		if (device.second->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
		{
			deviceSns.push_back(device.first);
		}
	}

	// 如果没有设备，那么肯定也没有试剂
	if (deviceSns.empty())
	{
		ULOG(LOG_INFO, "%s(assayCode have not the reagent : %d)", __FUNCTION__, assaycode);
		return chReagentGroups;
	}

	// 生成查询条件
	::ch::tf::ReagentGroupQueryCond queryCond;
	// 当血清指数的时候需要查询sind
	if (IsSIND(assaycode))
	{
		queryCond.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_SIND);
	}
	else
	{
		queryCond.__set_assayCode(assaycode);
	}

	queryCond.__set_deviceSN(deviceSns);
	queryCond.__set_beDeleted(false);

	::ch::tf::ReagentGroupQueryResp qryReagentResp;
	if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResp, queryCond)
		|| qryReagentResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QueryReagentGroup(%d) failed", assaycode);
		return chReagentGroups;
	}

	chReagentGroups = qryReagentResp.lstReagentGroup;
	return chReagentGroups;
}

///
/// @brief 根据ISE编号，获取ISE的耗材信息（所有）
///
/// @param[in]  assaycode  ISE耗材编号
///
/// @return 耗材信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
std::vector<ise::tf::SuppliesInfo> GetISESupplyByAssayCode(int assaycode)
{
	ULOG(LOG_INFO, "%s(assayCode : %d)", __FUNCTION__, assaycode);
	// 查询不同类型的耗材(ise), 查询在机的耗材
	ise::tf::SuppliesInfoQueryCond qryIseCond;
	ise::tf::SuppliesInfoQueryResp qryIseResp;
	std::vector<::tf::PlaceStatus::type> vecPlaceStatusIse;
	vecPlaceStatusIse.push_back(::tf::PlaceStatus::PLACE_STATUS_LOAD);
	qryIseCond.__set_placeStatus(vecPlaceStatusIse);

	std::vector<std::string> devices;
	// 要求查询耗材的设备没有被屏蔽
	auto deviceMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
	for (const auto & deviceInfo : deviceMap)
	{
		// 设备必须是ise
		if (deviceInfo.second->deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_ISE)
		{
			continue;
		}

		// 若是被屏蔽，则不符合
		if (deviceInfo.second->__isset.masked && deviceInfo.second->masked)
		{
			continue;
		}

		devices.push_back(deviceInfo.second->deviceSN);
	}

	qryIseCond.__set_deviceSN(devices);
	ise::ise1005::LogicControlProxy::QuerySuppliesInfo(qryIseResp, qryIseCond);
	if (qryIseResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Query Ise supply failed %d", assaycode);
		return std::vector<ise::tf::SuppliesInfo>{};
	}

	return qryIseResp.lstSuppliesInfo;
}

///
/// @brief 根据项目编号获取所有的试剂信息(免疫)
///
/// @param[in]  assaycode  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
std::vector<im::tf::ReagentInfoTable> GetImReagentGroupByAssayCode(int assaycode)
{
    // 构造数组
    std::vector<im::tf::ReagentInfoTable> vecInRgntInfo;

    // 获取所有免疫设备
    std::vector<std::string> vecDevSns;
    auto mapDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = mapDev.begin(); it != mapDev.end(); it++)
    {
        // 如果设备类型不为免疫，则跳过
        if (it->second->deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            continue;
        }

        // 记录设备序列号
        vecDevSns.push_back(it->first);
    }

    // 构造查询条件
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;
    qryCond.__set_assayCode(assaycode);
    qryCond.__set_deviceSNs(vecDevSns);

    // 执行查询
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return std::vector<im::tf::ReagentInfoTable>();
    }

    return qryResp.lstReagentInfos;
}

bool IsSIND(int assayCode)
{
	if (assayCode > ::ch::tf::g_ch_constants.ASSAY_CODE_SIND
		&& assayCode <= ::ch::tf::g_ch_constants.ASSAY_CODE_I)
	{
		return true;
	}

	return false;
};

///
/// @brief 是否ISE设备
///
/// @param[in]  assayCode  项目编号
///
/// @return true:表示ISE设备
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsISE(int assayCode)
{
	if (assayCode > ::ise::tf::g_ise_constants.ASSAY_CODE_ISE
		&& assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
	{
		return true;
	}

	return false;
}

///
/// @brief 是否试剂异常
///
/// @param[in]  reagent  试剂信息
///
/// @return true:试剂异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsReagentAbNormal(const ch::tf::ReagentGroup& reagent)
{
	// 试剂异常（1：可用测试数为0，2:试剂屏蔽）
	// 剩余测试数为0 ，可用一定为0
	if (reagent.remainCount <= 0)
	{
		return true;
	}

	// 液位探测失败，则试剂异常（20240911与周晓峰，马彪讨论，液位探测失败则试剂异常）
	if (reagent.liquidStatus == tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		return true;
	}

	// 校准系数不存在，无论剩余测试数多少都是异常
	if (reagent.caliCurveId < 0)
	{
		return true;
	}

	// 试剂被屏蔽以后都是异常
	if (reagent.reagentMask)
	{
		return true;
	}

	return false;
}

///
/// @brief 是否试剂异常
///
/// @param[in]  reagent  试剂信息
///
/// @return true:试剂异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
bool IsReagentAbNormal(const im::tf::ReagentInfoTable& reagent)
{
    // 不为在用，则是异常
    if (reagent.reagStatusUI.status != im::tf::StatusForUI::REAGENT_STATUS_ON_USE)
    {
        return true;
    }

    // 可用测试数为0，则试剂异常
    if (reagent.avaTestNum <= 0)
    {
        return true;
    }

    // 获取开瓶有效期
    QDateTime rgntRegisterDate = QDateTime::fromString(QString::fromStdString(reagent.registerTime), UI_DATE_TIME_FORMAT);
    int showStableRestday = reagent.stableDays - rgntRegisterDate.daysTo(QDateTime::currentDateTime());

    // 获取试剂失效日期
    QString qstrReagentExpriy = QString::fromStdString(reagent.reagentExpiry);
    QDateTime expDateTime = QDateTime::fromString(qstrReagentExpriy, UI_DATE_TIME_FORMAT);

    // 如果开瓶过期或有效期过期，则试剂异常
    if ((showStableRestday <= 0) || (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime())))
    {
        return true;
    }

    return false;
}

///
/// @brief 是否试剂异常(ISE)
///
/// @param[in]  supply		耗材信息
/// @param[in]  moduleInfo	耗材模块信息（用于判断是否校准）
///
/// @return true:耗材异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsReagentAbNormal(const ise::tf::SuppliesInfo& supply, std::shared_ptr<ise::tf::IseModuleInfo> moduleInfo)
{
	// 耗材被卸载了试剂未加载，属于异常
	if (supply.placeStatus == tf::PlaceStatus::type::PLACE_STATUS_UNLOAD)
	{
		return true;
	}

	if (moduleInfo == Q_NULLPTR)
	{
		return true;
	}

	if (supply.residual <= 0)
	{
		return true;
	}

	// 当没有校准参数的时候，校准异常(ise的校准信息存放在模块里面)
	if (moduleInfo->caliCurveId < 0)
	{
		return true;
	}

	return false;
}

///
/// @brief 是否校准异常
///
/// @param[in]  reagent  试剂信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsCaliStatusAbnormal(const ch::tf::ReagentGroup& reagent)
{
	// 无校准系统，异常
	if (reagent.caliCurveId < 0)
	{
		return true;
	}

	// 校准屏蔽，异常
	if (reagent.caliMask)
	{
		return true;
	}

	return false;
}

///
/// @brief 是否校准异常（免疫）
///
/// @param[in]  reagent  试剂信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsCaliStatusAbnormal(const im::tf::ReagentInfoTable& reagent)
{
    // 不为在用，则是异常
    if (reagent.reagStatusUI.status != im::tf::StatusForUI::REAGENT_STATUS_ON_USE)
    {
        return true;
    }

    // 无工作曲线，则异常
    if (reagent.caliCurveExpInfo.empty())
    {
        return true;
    }

    // 获取当前曲线有效期
    QStringList strCalCurveExpLst = QString::fromStdString(reagent.caliCurveExpInfo).split(":");
    if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() <= 0))
    {
        return true;
    }

    // 校准状态为校准失败，则异常
    if (reagent.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL)
    {
        return true;
    }

    return false;
}

///
/// @brief ISE的校准信息是否正常
///
/// @param[in]  supply  ISE的耗材信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsCaliStatusAbnormal(std::shared_ptr<ise::tf::IseModuleInfo> moduleInfo)
{
	if (moduleInfo == Q_NULLPTR)
	{
		return true;
	}

	// 校准参数异常
	if (moduleInfo->caliCurveId < 0)
	{
		return true;
	}

	// 校准屏蔽
	if (moduleInfo->caliMasked)
	{
		return true;
	}

	return false;
}

///
/// @brief 根据耗材查找对应的试剂组
///
/// @param[in]  supplyInfo  耗材信息
///
/// @return 试剂组信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
bool GetReagentGroupsBySupply(const ch::tf::SuppliesInfo& supplyInfo, std::vector<ch::tf::ReagentGroup>& chReagent)
{
    // 查找对应的试剂
    ::ch::tf::ReagentGroupQueryCond qryReagentCond;
    qryReagentCond.__set_suppliesCode(supplyInfo.suppliesCode); // 设置耗材编号
    qryReagentCond.__set_deviceSN({ supplyInfo.deviceSN });
    
    ::ch::tf::SuppliesPosition posInfo; // 位置信息
    posInfo.__set_area(supplyInfo.area);
    posInfo.__set_pos(supplyInfo.pos);
    qryReagentCond.__set_posInfo(posInfo);
    qryReagentCond.__set_beDeleted(false); // 未删除

    ::ch::tf::ReagentGroupQueryResp qryReagentResp;
    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResp, qryReagentCond)
        || qryReagentResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentGroup() failed");
        return false;
    }

    chReagent = qryReagentResp.lstReagentGroup;

    return true;
}

///
/// @brief
///     获取字符串排序值
///
/// @param[in]  strSort  排序字符串
///
/// @return 字符串排序值
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月27日，新建函数
///
unsigned long long GetStringSortVal(const QString& strSort)
{
    // 如果是数字则直接返回数字
    bool bOk = false;
    unsigned long long ulSortVal = strSort.toULongLong(&bOk);
    if (bOk)
    {
        return ulSortVal;
    }

    // 构造排序值
    const int ciStrBaseSortVal = 10000000;
    unsigned long long lSortVal = 0;
    QString strNumber("");
    bool bContainPoint = false;
    for (const auto& charShort : strSort)
    {
        // 判断字符范围
        unsigned long long lAddSortVal = 0;
        if ((charShort.unicode() >= QChar('0').unicode()) && (charShort.unicode() <= QChar('9').unicode()))
        {
            strNumber.push_back(charShort);
            //lSortVal = bContainPoint ? lSortVal : lSortVal * 16;
            continue;
        }
        else if ((charShort.unicode() == QChar('.').unicode()) && !bContainPoint)
        {
            bContainPoint = true;
            strNumber.push_back(charShort);
            continue;
        }
        else if ((charShort.unicode() >= QChar('a').unicode()) && (charShort.unicode() <= QChar('z').unicode()))
        {
            lAddSortVal = charShort.unicode() - QChar('a').unicode() + 10;
        }
        else if ((charShort.unicode() >= QChar('A').unicode()) && (charShort.unicode() <= QChar('Z').unicode()))
        {
            lAddSortVal = charShort.unicode() - QChar('A').unicode() + 36;
        }
        else
        {
            lAddSortVal = 63;
        }

        // 更新排序值
        lSortVal = lSortVal * 64 + lAddSortVal;
    }

    return lSortVal * 100000 + ciStrBaseSortVal + static_cast<unsigned long long>(strNumber.toDouble() * 10000);
}

///
/// @brief 根据耗材位置信息查找对应的在线试剂信息（对应的试剂信息大部分为1个，当有两个的时候为双向同测）
///
/// @param[in]  supplyPos  耗材位置信息
///
/// @return 试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
std::vector<ch::tf::ReagentGroup> GetReagentGroupsBySupply(const ch::tf::SupplyUpdate& supplyPos)
{
	ULOG(LOG_INFO, "%s(deviceSn:%s, area:%d, pos:%d)", __FUNCTION__, supplyPos.deviceSN, int(supplyPos.posInfo.area), supplyPos.posInfo.pos);

	// 位置信息
	::ch::tf::SuppliesPosition posInfo;
	posInfo.__set_area(supplyPos.posInfo.area);
	posInfo.__set_pos(supplyPos.posInfo.pos);

	// 查询条件
	::ch::tf::ReagentGroupQueryCond qryReagentCond;
	qryReagentCond.__set_deviceSN({ supplyPos.deviceSN });
	qryReagentCond.__set_posInfo(posInfo);
	// 查找未卸载的
	// 修改bug0013806 by wuht,考虑当耗材卸载的时候
	//qryReagentCond.__set_beDeleted(false);

	std::vector<std::shared_ptr<ch::tf::ReagentGroup>> reagents;
	::ch::tf::ReagentGroupQueryResp qryReagentResp;
	if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryReagentResp, qryReagentCond)
		|| qryReagentResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_INFO, "%s(failed query deviceSn:%s, area:%d, pos:%d)", __FUNCTION__, supplyPos.deviceSN, int(supplyPos.posInfo.area), supplyPos.posInfo.pos);
		return std::vector<ch::tf::ReagentGroup>{};
	}

	if (qryReagentResp.lstReagentGroup.empty())
	{
		ULOG(LOG_INFO, "%s(no reagent at postion: deviceSn:%s, area:%d, pos:%d)", __FUNCTION__, supplyPos.deviceSN, int(supplyPos.posInfo.area), supplyPos.posInfo.pos);
	}

	return qryReagentResp.lstReagentGroup;

}

int GetSupplyResidual(const ch::tf::SuppliesInfo& supplyInfo)
{
	// 如果液位探测失败，跳过该耗材的统计
	if (supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		return 0.0;
	}

	// 依次计算腔信息，总量
	auto cavitys = supplyInfo.cavityInfos;
	return std::accumulate(cavitys.begin(), cavitys.end(), 0, [](int a, auto& member) {return (a + member.residual);});
}

int GetAvailableSupplyTimes(int supplyCode, const std::string& deviceSn)
{
	auto lstSupplies = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(deviceSn);
	std::vector<ch::tf::SuppliesInfo> vecSupplies;
	for (const auto &si : lstSupplies)
	{
		auto supply = si.second.supplyInfo;

		// 要求液位探测成功、在用和备用
		if (supply.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL || supply.suppliesCode != supplyCode
			|| supply.usageStatus == tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE)
		{
			continue;
		}

		vecSupplies.push_back(supply);
	}

	// 累加次数得到总次数
	return std::accumulate(vecSupplies.begin(), vecSupplies.end(), 0, [=](int first, auto& member) {
        return(first + GetSupplyResidual(member)); });
}

int GetAvailableReagentTimes(int assayCode, const std::string& deviceSn)
{
	auto lstSupplies = CommonInformationManager::GetInstance()->GetChDiskReagentSupplies(deviceSn);
	int useTimes = 0;
	for (const auto& supply : lstSupplies)
	{
		// 耗材跳过
		if (supply.second.reagentInfos.empty())
		{
			continue;
		}

		for (const auto& rgt : supply.second.reagentInfos)
		{
			// 要求试剂必须被校准过、液位探测成功的、可用
			if (rgt.__isset.caliCurveId && rgt.caliCurveId != -1 && rgt.liquidStatus != ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL
				&& rgt.assayCode == assayCode && rgt.usageStatus != tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE)
			{
				useTimes += rgt.remainCount;
			}
		}
	}

	return useTimes;
}

///
/// @brief 根据起点和结束点来生成对应的表达式
///
/// @param[in]  startRange  范围起点
/// @param[in]  endRange    范围结束点
///
/// @return qt正则表达式
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
std::shared_ptr<QRegExp> GenRegexpByText(int startRange, int endRange)
{
	if (startRange > endRange)
	{
		return nullptr;
	}

	// 范围起点的数字列表
	QStringList startRangeParaList = QString::number(startRange).split("", QString::SkipEmptyParts);
	// 范围结束点的数字列表
	QStringList endRangeParaList = QString::number(endRange).split("", QString::SkipEmptyParts);

	if (startRangeParaList.size() < endRangeParaList.size())
	{
		// 字符串开始
		QString RegexString = ("^");
		// 根据数值大小来
        std::for_each(endRangeParaList.begin(), endRangeParaList.end(), [&](auto paraMeber) {
			RegexString += QString("[%1-%2]").arg((RegexString.size() == 1) ? 1 : 0).arg(paraMeber.toInt()); });
		// 字符串结束
		RegexString += "&";

		// 生成智能的正则表达式
		return std::make_shared<QRegExp>(RegexString);
	}

	// 生成表达式的函数
	auto genRegex = [&](auto firstPara, auto secondPara) -> QString
	{
		// 小于的时候
		if (firstPara < secondPara)
		{
			return QString("[%1-%2]").arg(firstPara.toInt()).arg(secondPara.toInt());
		}
		// 等于的时候
		else if (firstPara == secondPara)
		{
			return firstPara;
		}
		// 中间的处理，最后一位需要特殊处理
		else
		{
			return QString("[0-9]");
		}
	};

	QStringList paraSaveList;
	std::transform(startRangeParaList.begin(), startRangeParaList.end(),
		endRangeParaList.begin(), std::back_inserter(paraSaveList),
		genRegex);

	// 修改最后的一位为最后数值的大小
	if (startRangeParaList.back() > endRangeParaList.back())
	{
		paraSaveList[paraSaveList.size() -1] = QString("[0-%1]").arg(endRangeParaList.back().toInt());
	}

	// 头尾
	paraSaveList.push_front("^");
	paraSaveList.push_back("$");
	// 返回正则表达式
	return std::make_shared<QRegExp>(paraSaveList.join(""));
}

std::shared_ptr<const tf::DeviceInfo> FindDeviceInfoBySn(\
    std::vector<std::shared_ptr<const tf::DeviceInfo>>& srcDevs, const std::string& deviceSn)
{
    std::shared_ptr<const tf::DeviceInfo> targetDevice = nullptr;
    for (const auto& device : srcDevs)
    {
        if (device->deviceSN == deviceSn)
        {
            targetDevice = device;
            break;
        }
    }

    return targetDevice;
}

QString CombineReagentPos(const std::string& deviceSn, int reagentPos)
{
    QString spliterSymb = /*reagentPos < 10 ? QStringLiteral("-0") :*/ QStringLiteral("-");
    return QString::fromStdString(CommonInformationManager::GetDeviceName(deviceSn)) + spliterSymb + QString::number(reagentPos);
}

bool IsReagentPriortyTipDlg(const bool isReagent)
{
	QString text = isReagent ? QObject::tr("确定优先使用选中试剂瓶？") : QObject::tr("确定优先使用选中耗材？");

    TipDlg confirmTipDlg(QObject::tr("优先使用"), text, TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel PriorUse!");
        return false;
    }
    return true;
}

bool IsReagentShieldTipDlg(const QString& reagentName, bool isShield)
{
    QString noticeMsg = isShield ? QObject::tr("确定屏蔽选中的试剂瓶？"):
        QObject::tr("确定对选中的试剂瓶解除屏蔽？");

    TipDlg confirmTipDlg(QObject::tr("试剂屏蔽"), noticeMsg, TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "IsReagentShieldTipDlg canceled.");
        return false;
    }

    return true;
}

///
/// @brief
///     试剂卸载的统一弹框提示
///
/// @par History:
/// @li 未知，新建函数
/// @li 4170/TangChuXian，2023年11月10日，删除未使用的参数
///
bool IsReagentUnloadTipDlg()
{
    TipDlg confirmTipDlg(QObject::tr("试剂卸载"), QObject::tr("确定卸载当前选中试剂？"), TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel unload!");
        return false;
    }

    return true;
}

bool UnloadReagentStat(const tf::DeviceInfo &deviceInfo, const ch::tf::SuppliesInfo &spyInfo, const ch::tf::ReagentGroup &regGroupInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 只处理生化设备
	if (deviceInfo.deviceType != ::tf::DeviceType::DEVICE_TYPE_C1000)
	{
		return false;
	}

	// 只有运行和待机能进行卸载
	if(deviceInfo.status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY
		&& deviceInfo.status != tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		ULOG(LOG_INFO, u8"仪器处于非待机、非运行状态.");
		return false;
	}

	// 判断是试剂还是耗材
	auto useStates = (spyInfo.type == ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT) ?
		regGroupInfo.usageStatus : spyInfo.usageStatus;

	// 在用且非待机
	if (useStates == tf::UsageStatus::USAGE_STATUS_CURRENT && deviceInfo.status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
	{
		ULOG(LOG_INFO, u8"在用瓶不能卸载(待机状态下的在用允许卸载).");
		return false;
	}

	// 扫描成功但是非放置状态
	if (spyInfo.suppliesCode > 0 && spyInfo.placeStatus != tf::PlaceStatus::PLACE_STATUS_LOAD)
	{
		ULOG(LOG_INFO, u8"扫描失败或者放置状态=PLACE_STATUS_LOAD的才能卸载.");
		return false;
	}

	// 开放试剂
	if(spyInfo.suppliesCode <= ::ch::tf::g_ch_constants.MAX_REAGENT_CODE
		&& spyInfo.suppliesCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE)
	{
		ULOG(LOG_INFO, u8"开放试剂不能卸载.");
		return false;
	}

	// 生化设备增加未屏蔽自动装卸载试剂判断
	ChDeviceOtherInfo chdoi;
	if (!DecodeJson(chdoi, deviceInfo.otherInfo))
	{
		ULOG(LOG_ERROR, "DecodeJson ChDeviceOtherInfo Failed");
		return false;
	}
	if (!chdoi.loadReagentOnline.enable)
	{
		ULOG(LOG_INFO, u8"已屏蔽自动装卸载试剂.");
		return false;
	}

	// 查询在线试剂加载
	auto reagentLoader = CommonInformationManager::GetInstance()->getReagentLoaderByDevSn(QString::fromStdString(deviceInfo.deviceSN));
	// 试剂加载器状态异常
	if (reagentLoader.loaderStatus != ::ch::tf::LoaderStatus::LOADER_STATUS_NORMAL
		&& reagentLoader.loaderStatus != ::ch::tf::LoaderStatus::LOADER_STATUS_RUNNING)
	{
		ULOG(LOG_INFO, u8"试剂加载器状态异常.");
		return false;
	}

	// 试剂加载位有空位(有试剂位为空或扫描成功时方可进行试剂卸载)
	bool hasEmptyPos = false;
	for (auto &item : reagentLoader.loaderItems)
	{
		if (item.itemStatus == ::ch::tf::LoaderItemStatus::LOADER_ITEM_STATUS_IDLE
			|| item.itemStatus == ::ch::tf::LoaderItemStatus::LOADER_ITEM_STATUS_WAIT_GRAB)
		{
			hasEmptyPos = true;
			break;
		}
	}
	if (!hasEmptyPos)
	{
		ULOG(LOG_INFO, u8"试剂加载器无空位.");
		return false;
	}

	return true;
}

///
/// @brief
///     弹出卸载试剂被禁止的提示框
///
/// @param[in]  deviceInfo   设备信息
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示卸载试剂被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月5日，新建函数
///
bool PopUnloadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo, const im::tf::ReagentInfoTable& stuRgntInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        TipDlg(QObject::tr("当前仪器类型不允许卸载试剂！")).exec();
        return true;
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载试剂！")).exec();
        return true;
    }

    // 只有预热、待机、加样停、运行状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载试剂！")).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("试剂加卸载机构正在运行中，请稍后再试。")).exec();
        return true;
    }

    // 获取试剂信息或耗材信息
    im::tf::StatusForUI::type enUseState = im::tf::StatusForUI::REAGENT_STATUS_INVALID;
    ::tf::CaliStatus::type enCaliState = ::tf::CaliStatus::CALI_STATUS_NOT;
    enUseState = stuRgntInfo.reagStatusUI.status;
    enCaliState = stuRgntInfo.caliStatus;

    // 如果仪器处于加样停/运行且屏蔽的是在用试剂，则弹框提示不允许卸载
    if (((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status)) &&
        (enUseState == im::tf::StatusForUI::REAGENT_STATUS_ON_USE))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载在用瓶试剂！")).exec();
        return true;
    }

    // 正在校准的试剂不能卸载
    if (enCaliState == ::tf::CaliStatus::CALI_STATUS_DOING)
    {
        TipDlg(QObject::tr("该试剂正在校准不允许卸载！")).exec();
        return true;
    }

    // 正在质控的试剂不能卸载 bug18043
    if (im::LogicControlProxy::IsReagentControling(stuRgntInfo))
    {
        TipDlg(QObject::tr("该试剂正在质控不允许卸载！")).exec();
        return true;
    }

	// 正在测试使用的试剂不能卸载
	::tf::ResultLong _return;
	bool bRet = im::i6000::LogicControlProxy::CheckUnLoadReagent(_return, stuRgntInfo.deviceSN, stuRgntInfo.reagentPos);
	if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		TipDlg(QObject::tr("该试剂正在使用不允许卸载！")).exec();
		return true;
	}

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("不允许卸载，当前仪器试剂仓盖打开后未关闭!")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
    bool bHasReagentScan = false;
    if (!::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan))
    {
        TipDlg(QObject::tr("请对当前仪器执行试剂扫描后再尝试卸载!")).exec();
        return true;
    }

    return false;
}

///
/// @brief
///     无位置卸载试剂
///
/// @param[in]  deviceInfo  设备信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月10日，新建函数
///
bool PopUnloadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        TipDlg(QObject::tr("当前仪器类型不允许卸载试剂！")).exec();
        return true;
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载试剂！")).exec();
        return true;
    }

    // 只有预热、待机、加样停、运行状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载试剂！")).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("试剂加卸载机构正在运行中，请稍后再试。")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("不允许卸载，当前仪器试剂仓盖打开后未关闭!")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
    bool bHasReagentScan = false;
    if (!::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan))
    {
        TipDlg(QObject::tr("请对当前仪器执行试剂扫描后再尝试卸载!")).exec();
        return true;
    }

    return false;
}

///
/// @brief
///     是否允许试剂卸载
///
/// @param[in]  deviceInfo   设备信息
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableUnloadRgnt(const tf::DeviceInfo &deviceInfo, const im::tf::ReagentInfoTable& stuRgntInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 不为免疫设备，直接返回false
    if (deviceInfo.deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        return false;
    }

    // 如果正在申请更换试剂，置灰
    auto enApplyStatus = GetApplyChangeRgntStatus(stuTfDevInfo.deviceSN);
    if (enApplyStatus == RGNT_CHANGE_APPLY_STATUS_HANDLE || enApplyStatus == RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY)
    {
        return false;
    }

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo.deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
    }

    // 如果是屏蔽状态，仪器状态为运行或加样停，直接使能按钮，否则禁能
    if (bMask)
    {
        return ((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status));
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        return false;
    }

    // 只有预热、待机、加样停、运行状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        return false;
    }

    // 如果有选中位置，则获取对应设备动作状态、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        // 选中设备正在执行动作，禁能加卸载按钮
        return false;
    }

    // 如果试剂是自动寻找（id小于等于0，则返回true）
    if (stuRgntInfo.id <= 0)
    {
        return true;
    }

    // 获取试剂信息或耗材信息
    im::tf::StatusForUI::type enUseState = im::tf::StatusForUI::REAGENT_STATUS_INVALID;
    ::tf::CaliStatus::type enCaliState = ::tf::CaliStatus::CALI_STATUS_NOT;
    enUseState = stuRgntInfo.reagStatusUI.status;
    enCaliState = stuRgntInfo.caliStatus;

    // 如果仪器处于加样停/运行且卸载的是在用试剂，则弹框提示不允许卸载
    if (((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status)) &&
        (enUseState == im::tf::StatusForUI::REAGENT_STATUS_ON_USE))
    {
        return false;
    }

    // 正在校准的试剂不能卸载
    if (enCaliState == ::tf::CaliStatus::CALI_STATUS_DOING)
    {
        return false;
    }

//     // 正在质控的试剂不能卸载 bug18043
//     if (im::LogicControlProxy::IsReagentControling(stuRgntInfo))
//     {
//         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QObject::tr("该试剂正在质控不允许卸载！")));
//         pTipDlg->exec();
//         return true;
//     }

    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
//     bool bHasReagentScan = false;
//     if (::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan) && !bHasReagentScan)
//     {
//         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QObject::tr("不允许卸载，当前仪器操作试剂仓盖后未执行试剂扫描!")));
//         pTipDlg->exec();
//         return true;
//     }
// 
//     // 判断是否打开试剂仓盖
//     if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
//     {
//         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QObject::tr("不允许卸载，当前仪器试剂仓盖打开后未关闭!")));
//         pTipDlg->exec();
//         return true;
//     }

    return true;
}

///
/// @brief
///     是否使能装载装置复位
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool IsEnableRgntLoaderReset(const tf::DeviceInfo &deviceInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 不为免疫设备，直接返回false
    if (deviceInfo.deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        return false;
    }

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo.deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
    }

    // 如果是屏蔽状态，仪器状态为运行或加样停，直接使能按钮，否则禁能
    if (bMask)
    {
        return false;
    }

    // 只有预热、待机、加样停、运行状态才能加载卸载试剂
    if (::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN == stuTfDevInfo.status)
    {
        return false;
    }

    // 如果有选中位置，则获取对应设备动作状态、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        // 选中设备正在执行动作，禁能加卸载按钮
        return false;
    }

    return true;
}

///
/// @bref
///     是否存在符合扫描条件的设备
///
/// @par History:
/// @li 8276/huchunli, 2023年9月14日，新建函数
///
bool IsExistDeviceCanScan()
{
    // 获取所有设备列表
    auto mapAllDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = mapAllDev.begin(); it != mapAllDev.end(); it++)
    {
        // 轨道则跳过
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK ||
            it->second->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            continue;
        }

        // 免疫设备，不能加卸载、设备不处于待机或者预热，则跳过
        if (it->second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE
            && !RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(it->second->deviceSN)
			&& it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY 
			&& it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP)
        {
            continue;
        }
		// 生化设备，待机和停机态允许试剂扫描（bug14404）
		else if (it->second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
			&& it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY
			&& it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_HALT)
		{
			continue;
		}

        // 如果有选中位置，则获取对应设备动作状态、
        boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(it->first));
        if (!opActStatus.has_value() || !opActStatus->second)
        {
            // 选中设备正在执行动作，禁能加卸载按钮
            return true;
        }
    }

    return false;
}

///
/// @brief
///     是否存在设备可以磁珠混匀
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsExistDeviceCanMix()
{
    // 获取所有设备列表
    auto mapAllDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = mapAllDev.begin(); it != mapAllDev.end(); it++)
    {
        // 轨道则跳过
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 不为i6000则跳过
        if (it->second->deviceType != tf::DeviceType::DEVICE_TYPE_I6000)
        {
            continue;
        }

        // 如果不能加卸载，则禁能
        if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(it->second->deviceSN))
        {
            return false;
        }

        // 待机状态返回true
        if (it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY &&
            it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP)
        {
            continue;
        }

        // 如果有选中位置，则获取对应设备动作状态、
        boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(it->first));
        if (!opActStatus.has_value() || !opActStatus->second)
        {
            // 选中设备正在执行动作，禁能加卸载按钮
            return true;
        }
    }

    return false;
}

///
/// @brief
///     是否存在设备可以装载装置复位
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool IsExistDeviceCanResetLoader()
{
    // 获取所有设备列表
    auto mapAllDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = mapAllDev.begin(); it != mapAllDev.end(); it++)
    {
        // 轨道则跳过
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 不为i6000则跳过
        if (it->second->deviceType != tf::DeviceType::DEVICE_TYPE_I6000)
        {
            continue;
        }

        // 维护状态跳过
        if (it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN ||
            it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT)
        {
            continue;
        }

        // 查询加载功能是否屏蔽
        bool bMask = false;
        if (!ReagentLoadIsMask(bMask, it->first))
        {
            ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        }

        // 如果是屏蔽状态，则跳过
        if (bMask)
        {
            continue;
        }

        // 如果有选中位置，则获取对应设备动作状态、
        boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(it->first));
        if (!opActStatus.has_value() || !opActStatus->second)
        {
            // 选中设备正在执行动作，禁能加卸载按钮
            return true;
        }
    }

    return false;
}

///
/// @brief
///     弹出卸载稀释液被禁止的提示框
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return true表示卸载稀释液被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月5日，新建函数
///
bool PopUnloadDltForbiddenTip(const tf::DeviceInfo &deviceInfo, const im::tf::DiluentInfoTable& stuDltInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        TipDlg(QObject::tr("当前仪器类型不允许卸载稀释液！")).exec();
        return true;
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载稀释液！")).exec();
        return true;
    }

    // 只有预热、待机、加样停、允许状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载稀释液！")).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("试剂加卸载机构正在运行中，请稍后再试。")).exec();
        return true;
    }

    // 获取试剂信息或耗材信息
    im::tf::StatusForUI::type enUseState = im::tf::StatusForUI::REAGENT_STATUS_INVALID;
    enUseState = stuDltInfo.statusForUI.status;

    // 如果仪器处于加样停/运行且屏蔽的是在用试剂，则弹框提示不允许卸载
    if (((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status)) &&
        (enUseState == im::tf::StatusForUI::REAGENT_STATUS_ON_USE))
    {
        TipDlg(QObject::tr("当前仪器状态不允许卸载在用瓶稀释液！")).exec();
        return true;
    }

    // 正在测试使用的稀释液不能卸载
    ::tf::ResultLong _return;
    bool bRet = im::i6000::LogicControlProxy::CheckUnLoadReagent(_return, stuDltInfo.deviceSN, stuDltInfo.pos);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        TipDlg(QObject::tr("该稀释液正在使用不允许卸载！")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("不允许卸载，当前仪器试剂仓盖打开后未关闭!")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
    bool bHasReagentScan = false;
    if (!::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan))
    {
        TipDlg(QObject::tr("请对当前仪器执行试剂扫描后再尝试卸载!")).exec();
        return true;
    }

    return false;
}

///
/// @brief
///     是否使能卸载稀释液
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  stuDltInfo  稀释液
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableUnloadDlt(const tf::DeviceInfo &deviceInfo, const im::tf::DiluentInfoTable& stuDltInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        return false;
    }

    // 如果正在申请更换试剂，置灰
    auto enApplyStatus = GetApplyChangeRgntStatus(stuTfDevInfo.deviceSN);
    if (enApplyStatus == RGNT_CHANGE_APPLY_STATUS_HANDLE || enApplyStatus == RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY)
    {
        return false;
    }

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo.deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
    }

    // 如果是屏蔽状态，仪器状态为运行或加样停，直接使能按钮，否则禁能
    if (bMask)
    {
        return ((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status));
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        return false;
    }

    // 只有预热、待机、加样停、允许状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        return false;
    }

    // 如果有选中位置，则获取对应设备动作状态、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        // 选中设备正在执行动作，禁能加卸载按钮
        return false;
    }

    // 如果试剂是自动寻找（id小于等于0，则返回true）
    if (stuDltInfo.id <= 0)
    {
        return true;
    }

    // 获取试剂信息或耗材信息
    im::tf::StatusForUI::type enUseState = im::tf::StatusForUI::REAGENT_STATUS_INVALID;
    enUseState = stuDltInfo.statusForUI.status;

    // 如果仪器处于加样停/运行且屏蔽的是在用试剂，则弹框提示不允许卸载
    if (((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status)) &&
        (enUseState == im::tf::StatusForUI::REAGENT_STATUS_ON_USE))
    {
        return false;
    }

    return true;
}

///
/// @brief
///     弹出装载试剂被禁止的提示框
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  iPos        试剂位置
///
/// @return true表示装载试剂被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
bool PopLoadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo, int& iPos /*= 0*/)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        TipDlg(QObject::tr("当前仪器类型不允许装载试剂！")).exec();
        return true;
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        TipDlg(QObject::tr("当前仪器状态不允许装载试剂！")).exec();
        return true;
    }

    // 只有预热、待机、加样停、允许状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        TipDlg(QObject::tr("当前仪器状态不允许装载试剂！")).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("试剂加卸载机构正在运行中，请稍后再试。")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("不允许装载，当前仪器试剂仓盖打开后未关闭!")).exec();
        return true;
    }
    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
    bool bHasReagentScan = false;
    if (!::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan))
    {
        TipDlg(QObject::tr("请对当前仪器执行试剂扫描后再尝试装载!")).exec();
        return true;
    }

	// 位置检测需要放最后检测，因为检测通过时即会弹出加载试剂槽 add by wzx - 230829
	// 检查位置是否可以加载试剂,不在1-30表示选择距离1最近的可用位置
	::tf::ResultLong _return;
	bool bRet = ::im::i6000::LogicControlProxy::CheckLoadReagent(_return, stuTfDevInfo.deviceSN, iPos);
	if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "CheckLoadReagent failed.");
		if (iPos <= 0)
		{
			TipDlg(QObject::tr("试剂盘已满，请先卸载试剂！")).exec();
		}
		else
		{
			TipDlg(QObject::tr("试剂盘中位置【%1】已被使用！").arg(iPos)).exec();
		}
		return true;
	}

    // 如果允许加载，记录加载位置
    iPos = _return.value;
    return false;
}

///
/// @brief
///     是否使能加载试剂
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示使能
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableLoadRgnt(const tf::DeviceInfo &deviceInfo)
{
    // 获取选中设备信息
    const tf::DeviceInfo& stuTfDevInfo = deviceInfo;

    // 当前设备类型不为免疫，不允许装载
    if (stuTfDevInfo.deviceClassify != ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        return false;
    }

    // 如果正在申请更换试剂，置灰
    auto enApplyStatus = GetApplyChangeRgntStatus(stuTfDevInfo.deviceSN);
    if (enApplyStatus == RGNT_CHANGE_APPLY_STATUS_HANDLE || enApplyStatus == RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY)
    {
        return false;
    }

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo.deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
    }

    // 如果是屏蔽状态，仪器状态为运行或加样停，直接使能按钮，否则禁能
    if (bMask)
    {
        return ((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == stuTfDevInfo.status) 
            || (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == stuTfDevInfo.status)
            || (::tf::DeviceWorkState::DEVICE_STATUS_WAIT_OPEN_LID == stuTfDevInfo.status));
    }

    // 如果不能加卸载，则禁能
    if (!RgntNoticeDataMng::GetInstance()->IsCanLoadReagent(stuTfDevInfo.deviceSN))
    {
        return false;
    }

    // 只有预热、待机、加样停、允许状态才能加载卸载试剂
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP != stuTfDevInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING != stuTfDevInfo.status))
    {
        return false;
    }

    // 如果有选中位置，则获取对应设备动作状态、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(stuTfDevInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        // 选中设备正在执行动作，禁能加卸载按钮
        return false;
    }

    return true;
}

///
/// @brief
///     弹出磁珠混匀被禁止提示框
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示磁珠混匀被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
bool PopBeadMixForbiddenTip(const tf::DeviceInfo &deviceInfo)
{
    // 获取设备名
    QString strDevName = QString::fromStdString(deviceInfo.name);

    // 如果仪器不处于待机/预热状态，则弹框提示不允许磁珠混匀, TODO:confirm.
    if ((::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != deviceInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != deviceInfo.status))
    {
        TipDlg(QObject::tr("【%1】仪器不处于待机状态,不允许试剂混匀！").arg(strDevName)).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(deviceInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("【%1】试剂加卸载机构正在运行中，请稍后再试。").arg(strDevName)).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("【%1】试剂仓盖未关闭，请关闭后重试！").arg(strDevName)).exec();
        return true;
    }

    // 判断是否打开试剂仓盖后关闭仓盖执行试剂扫描
    bool bHasReagentScan = false;
    if (!::im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(deviceInfo.deviceSN, bHasReagentScan))
    {
        TipDlg(QObject::tr("【%1】操作试剂仓盖后未执行试剂扫描，请先执行试剂扫描再重试!").arg(strDevName)).exec();
        return true;
    }

    return false;
}

///
/// @brief
///     装载装置复位
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示装载装置复位被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool PopRgntLoaderResetForbiddenTip(const tf::DeviceInfo &deviceInfo)
{
    // 获取设备名
    QString strDevName = QString::fromStdString(deviceInfo.name);

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, deviceInfo.deviceSN))
    {
        ULOG(LOG_WARN, "deviceInfo failed!");
    }

    // 如果屏蔽则弹框提示
    if (bMask)
    {
        TipDlg(QObject::tr("【%1】试剂在线装载功能已屏蔽,不允许装载装置复位！").arg(strDevName)).exec();
        return true;
    }

    // 如果仪器不处于待机/预热状态，则弹框提示不允许磁珠混匀, TODO:confirm.
    if (::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN == deviceInfo.status)
    {
        TipDlg(QObject::tr("【%1】仪器处于维护状态,不允许装载装置复位！").arg(strDevName)).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(deviceInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("【%1】试剂加卸载机构正在运行中，请稍后再试。").arg(strDevName)).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("【%1】试剂仓盖未关闭，请关闭后重试！").arg(strDevName)).exec();
        return true;
    }

    return false;
}

///
/// @brief
///     执行单设备磁珠混匀
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月18日，新建函数
///
void ExecuteSingleDevBeadMix(const std::string& strDevSn)
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 查询当前选中项设备状态
    std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
    if (spDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "GetDeviceInfo(%s) failed", strDevSn.c_str());
        return;
    }

    // 弹框禁止磁珠混匀
    if (PopBeadMixForbiddenTip(*spDevInfo))
    {
        return;
    }

    // 弹框提示是否启动磁珠混匀
    TipDlg confirmTipDlg(QObject::tr("试剂混匀"), QObject::tr("确定启动磁珠混匀？"), TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel mix!");
        return;
    }

    // 执行磁珠混匀
    ::tf::ResultLong _return;
    std::vector<int> vecPos;
    bool bRet = im::LogicControlProxy::SingleMaintance(::tf::MaintainItemType::type::MAINTAIN_ITEM_REA_VORTEX, strDevSn, vecPos);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "MAINTAIN_ITEM_REA_VORTEX failed.");
    }
}

///
/// @brief
///     弹出试剂扫描被禁止提示框
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示试剂扫描被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月12日，新建函数
///
bool PopRgntScanForbiddenTip(const tf::DeviceInfo &deviceInfo)
{
    // 获取设备名
    QString strDevName = QString::fromStdString(deviceInfo.name);

    // 如果仪器不处于待机/预热状态，则弹框提示不允许磁珠混匀, TODO:confirm.
    if ((::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != deviceInfo.status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != deviceInfo.status))
    {
        TipDlg(QObject::tr("仪器不处于待机状态,不允许试剂扫描！")).exec();
        return true;
    }

    // 如果对应设备动作状态正在运行，则弹框提示、
    boost::optional<QPair<RgntNoticeDataMng::ActionType, bool>> opActStatus = RgntNoticeDataMng::GetInstance()->GetDevActionStatus(QString::fromStdString(deviceInfo.deviceSN));
    if (opActStatus.has_value() && opActStatus->second)
    {
        TipDlg(QObject::tr("试剂加卸载机构正在运行中，请稍后再试。")).exec();
        return true;
    }

    // 判断是否打开试剂仓盖
    if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
    {
        TipDlg(QObject::tr("【%1】试剂仓盖未关闭，请关闭后重试！").arg(strDevName)).exec();
        return true;
    }

    return false;
}

///
/// @brief 是否允许试剂扫描（只针对免疫单机）
///
/// @param[in]  deviceInfo  
///
/// @return true：允许 false：不允许
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
bool EnableReagentScan(const tf::DeviceInfo &deviceInfo)
{
	// 如果仪器不处于待机/预热状态，则弹框提示不允许磁珠混匀, TODO:confirm.
	if ((::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != deviceInfo.status) &&
		(::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != deviceInfo.status))
	{
		// 仪器不处于待机状态,不允许试剂扫描！";
		return false;
	}
	// 判断是否打开试剂仓盖
	if (im::i6000::LogicControlProxy::QueryMaterialStatus(deviceInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
	{
		// 不允许试剂扫描，仪器【%1】试剂仓盖打开后未关闭!
		return false;
	}
	return true;
}

QStandardItem * CenterAligmentItem(const QString & text)
{
    QStandardItem * item = new QStandardItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}

QStandardItem * SetItemTextAligCenter(QStandardItem * item)
{
    // 判断是否是空指针
    if (item)
    {
        item->setTextAlignment(Qt::AlignCenter);
    }
    
    return item;
}

///
/// @brief
///     是否校准过期
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示校准过期
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月2日，新建函数
///
bool IsCaliExpired(const ::im::tf::ReagentInfoTable& stuRgntInfo)
{
    // 判断是否打开校准过期提示
    bool bCalExpireNotice = DictionaryQueryManager::GetInstance()->IsCaliLineExpire();

    // 如果没有打开校准过期提示，无需判断
    if (!bCalExpireNotice)
    {
        ULOG(LOG_INFO, "ignore, do not need notice.");
        return false;
    }

    // 获取瓶校准周期
    // 获取设备信息
    auto spStuDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(stuRgntInfo.deviceSN);
    if (spStuDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "GetDeviceInfo failed.");
        return false;
    }

    // 查询免疫通用项目信息
    auto imGAI = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(stuRgntInfo.assayCode, spStuDevInfo->deviceType, stuRgntInfo.protocolVersion);
    if (imGAI == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "GetImmuneAssayInfo failed. assayCode=%d", stuRgntInfo.assayCode);
        return false;
    }

    // 获取瓶校准周期
    int iBottleCaliCycle = imGAI->kitCaliPeriod;
    if (iBottleCaliCycle <= 0)
    {
        ULOG(LOG_WARN, "iBottleCaliCycle <= 0.");
        return false;
    }

    // 判断是否存在批校准曲线
    bool bHasLotCurve = QString::fromStdString(stuRgntInfo.caliCurveExpInfo).contains("L");

    // 判断是否校准成功过
    QDateTime caliDateTime = QDateTime::fromString(QString::fromStdString(stuRgntInfo.caliTime), UI_DATE_TIME_FORMAT);
    bool bHasCaliSucc = false;
    if (caliDateTime.isValid() && caliDateTime > QDateTime(QDate(2000, 1, 1)))
    {
        // 备注：校准时间初始值默认为1970-1-1，故校准时间是在2000年之前的，都是未校准成功过
        bHasCaliSucc = true;
    }

    // 根据上机时间判断是否校准过期
    bool bCaliExpired = false;
    QDateTime rgntRegisterDate = QDateTime::fromString(QString::fromStdString(stuRgntInfo.registerTime), UI_DATE_TIME_FORMAT);
    if (bHasLotCurve &&
        !bHasCaliSucc &&
        (rgntRegisterDate.addDays(iBottleCaliCycle) <= QDateTime::currentDateTime()))
    {
        bCaliExpired = true;
    }

    // 获取校准时间
    if (bHasLotCurve &&
        bHasCaliSucc &&
        caliDateTime.isValid() &&
        caliDateTime.addDays(iBottleCaliCycle) <= QDateTime::currentDateTime())
    {
        bCaliExpired = true;
    }

    return bCaliExpired;
}

///
/// @brief
///     是否是特殊清洗液
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return true表示是
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
bool IsSpecialWashing(const im::tf::DiluentInfoTable& stuDltInfo)
{
    // 编号匹配则为特殊清洗液
    if (stuDltInfo.__isset.diluentNumber && (stuDltInfo.diluentNumber == s_ciSpecialWashNum))
    {
        return true;
    }

    return false;
}

///
/// @brief
///     是否是特殊清洗液
///
/// @param[in]  iDltNum  稀释液编号
///
/// @return true表示是
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
bool IsSpecialWashing(int iDltNum)
{
    // 编号匹配则为特殊清洗液
    if (iDltNum == s_ciSpecialWashNum)
    {
        return true;
    }

    return false;
}

///
/// @brief
///     根据耗材需求信息获取耗材名称
///
/// @param[in]  stuSplReqInfo  耗材需求信息
///
/// @return 耗材名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月17日，新建函数
///
QString GetNameBySplReqInfo(const tf::SplRequireInfo& stuSplReqInfo)
{
    // 如果是试剂，直接通过项目编号获取名称
    if (stuSplReqInfo.bIsRgnt)
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuSplReqInfo.code);
        if (spAssayInfo == Q_NULLPTR)
        {
            return "";
        }

        return QString::fromStdString(spAssayInfo->assayName);
    }

    // 如果是非试剂，则分设备处理
    if (stuSplReqInfo.deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
    {
        // 生化设备通过code判断耗材类型
        return ThriftEnumTrans::GetSupplyName(ch::tf::SuppliesType::type(stuSplReqInfo.code), true);
    }

    // 如果是免疫设备
    if (stuSplReqInfo.deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        // 免疫设备只判断稀释液
        if (!IsSpecialWashing(stuSplReqInfo.code))
        {
            return QString("Diluent_") + QString::number(stuSplReqInfo.code);
        }
        else
        {
            return QString("WS Wash Buffer");
        }
    }

    return "";
}

bool ShieldReagentStatIm(const tf::DeviceInfo &deviceInfo, im::tf::StatusForUI::type uiStat)
{
    // 如果仪器处于加样停/运行且屏蔽的是在用试剂，则弹框提示不允许屏蔽, TODO:confirm.
    if (((::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == deviceInfo.status) || \
        (::tf::DeviceWorkState::DEVICE_STATUS_RUNNING == deviceInfo.status)) &&
        (uiStat == im::tf::StatusForUI::REAGENT_STATUS_ON_USE))
    {
        TipDlg(QObject::tr("当前仪器状态不允许屏蔽！")).exec();
        return false;
    }

    return true;
}

bool IsDevicesOnlyIm(const std::vector<std::shared_ptr<const tf::DeviceInfo>>& curDevi)
{
    for (const std::shared_ptr<const tf::DeviceInfo>& dev : curDevi)
    {
        if (dev->deviceType != tf::DeviceType::DEVICE_TYPE_I6000)
        {
            return false;
        }
    }

    return true;
}

void GetTblItemColor(std::shared_ptr<RowCellInfo>& cellInfo, TblItemState enState)
{
    if (cellInfo == nullptr)
    {
        return;
    }
    switch (enState)
    {
    case STATE_NORMAL:
        cellInfo->m_fontColor.clear();
        cellInfo->m_backgroudColor.clear();
        break;
    case STATE_NOTICE:
        cellInfo->m_fontColor = UI_REAGENT_NOTEFONT;
		cellInfo->m_backgroudColor.clear();
        break;
    case STATE_WARNING:
        cellInfo->m_fontColor = UI_REAGENT_WARNFONT;
		cellInfo->m_backgroudColor.clear();
        break;
    case STATE_SCAN_FAILED:
        cellInfo->m_fontColor = UI_REAGENT_WARNFONT;
        cellInfo->m_backgroudColor.clear();
        break;
    default:
        cellInfo->m_fontColor.clear();
        cellInfo->m_backgroudColor.clear();
        break;
    }
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
/// @li 4170/TangChuXian，2024年1月18日，新建函数
///
bool ReagentLoadIsMask(bool& bMask, const std::string& deviceSN)
{
    // 检查试剂在线加载是否被屏蔽
    return RgntNoticeDataMng::GetInstance()->IsReagentLoadIsMask(bMask, deviceSN);
}

///
/// @brief
///     设置是否申请更换试剂
///
/// @param[in]  enApplyStatus  申请状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月18日，新建函数
/// @li 4170/TangChuXian，2024年5月16日，参数修改enApplyStatus
///
void SetApplyChangeRgnt(const std::string& strDevSn, RgntChangeApplyStatus enApplyStatus)
{
    s_mapDevApplyChangeRgnt.insert(QString::fromStdString(strDevSn), enApplyStatus);
}

///
/// @brief
///     获取申请更换试剂状态
///
/// @param[in]  strDevSn  设备序列号
///
/// @return 申请更换试剂状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月16日，新建函数
///
RgntChangeApplyStatus GetApplyChangeRgntStatus(const std::string& strDevSn)
{
    auto it = s_mapDevApplyChangeRgnt.find(QString::fromStdString(strDevSn));
    if (it == s_mapDevApplyChangeRgnt.end())
    {
        return RGNT_CHANGE_APPLY_STATUS_ALLOW_APPLY;
    }

    return it.value();
}

QString GetPreAssayName(int preAssayCode)
{
    QString previousAssayName;
    auto spPreAssay = CommonInformationManager::GetInstance()->GetAssayInfo(preAssayCode);

    if (spPreAssay != nullptr)
        previousAssayName = std::move(QString::fromStdString(spPreAssay->assayName));
    // 项目参数如果被删除了，先显示项目编号（和胡蓉确认）
    else if (preAssayCode > -1)
        previousAssayName = QString::number(preAssayCode);

    return previousAssayName;
}

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::ReagentInfoTable& stuRgntInfo)
{
    // 参数检查
    if (!stuRgntInfo.__isset.id)
    {
        return 0;
    }

    // 计算开瓶有效期
    QDateTime qRegisterTime = QDateTime::fromString(QString::fromStdString(stuRgntInfo.registerTime), UI_DATE_TIME_FORMAT);
    const qint64 clSecsOfday = 3600 * 24;
    int totalSecond = stuRgntInfo.stableDays * clSecsOfday - qRegisterTime.secsTo(QDateTime::currentDateTime());
    int showStableRestday = totalSecond / clSecsOfday;
    if (totalSecond > 0)
    {
        showStableRestday++;
    }
    return showStableRestday;
}

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::DiluentInfoTable& stuDltInfo)
{
    // 参数检查
    if (!stuDltInfo.__isset.id)
    {
        return 0;
    }

    // 计算开瓶有效期
    QDateTime qRegisterTime = QDateTime::fromString(QString::fromStdString(stuDltInfo.registerTime), UI_DATE_TIME_FORMAT);
    const qint64 clSecsOfday = 3600 * 24;
    int showStableRestday = stuDltInfo.stableDays - (qRegisterTime.secsTo(QDateTime::currentDateTime()) / clSecsOfday);
    return showStableRestday;
}

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuSplInfo  耗材信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::SuppliesInfoTable& stuSplInfo)
{
    // 参数检查
    if (!stuSplInfo.__isset.iD)
    {
        return 0;
    }

    // 计算开瓶有效期
    QDateTime splRegisterDate = QDateTime::fromString(QString::fromStdString(stuSplInfo.recordDate), UI_DATE_TIME_FORMAT);
    const qint64 clSecsOfday = 3600 * 24;
    int showStableRestday = stuSplInfo.stableDays - (splRegisterDate.secsTo(QDateTime::currentDateTime()) / clSecsOfday);
    return showStableRestday;
}

///
/// @brief
///     初始化试剂管理器
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void InitRgntManager()
{
    RgntNoticeDataMng::GetInstance();
}

int GetOpenRestdays(const std::string& exprityTime)
{
	// 开瓶有效期减去当前时间，得到剩余时间
	QDateTime datetimeExpired = QDateTime::fromString(QString::fromStdString(exprityTime), UI_DATE_TIME_FORMAT);

	// 计算距当前时间还剩多少天
	qint64 lValidDays = 0l;
	qint64 lSubSecs = QDateTime::currentDateTime().secsTo(datetimeExpired);
	const qint64 clSecsOfday = 3600 * 24;
	lValidDays = (lSubSecs <= 0) ? 0 : (lSubSecs + clSecsOfday - 1) / clSecsOfday;

	// 返回开瓶有效期
	return int(lValidDays);
}

bool ChSuppliesExpire(const std::string& exprityTime)
{
	QStringList strTimeList = QString::fromStdString(exprityTime).split(" ");

	if (strTimeList.size() != 2)
	{
		return false;
	}

	if (strTimeList[0].isEmpty())
	{
		return false;
	}

	QDate expDateTime = QDate::fromString(strTimeList[0], UI_DATE_FORMAT);
	if (expDateTime.isValid() && (expDateTime <= QDate::currentDate()))
	{
		return true;
	}

	return false;
}

bool ParserInputPositions(const QString& inputStringPos, std::vector<int>& vctPositons)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 若位置信息为空，直接返回
	if (inputStringPos.isEmpty())
	{
		ULOG(LOG_WARN, "User input postion string is empty.");
		TipDlg(QObject::tr("位置信息为空，请重新输入！")).exec();
		return false;
	}

	/* 验证输入的位置信息格式是否正确(贪婪匹配) 示例格式(数字不能以0开头，且只能两位):
	12或12,13,60或12,13-15,16,17,78-79或2-3,4-7,8-9,99,98,96,54-57 */
	QRegExp rxValidator(UI_REAGENT_POS);
	inputStringPos.indexOf(rxValidator);
	if (inputStringPos.length() != rxValidator.matchedLength())
	{
		// 如果位置格式不匹配
		ULOG(LOG_WARN, "User input postion string is not meet standard format.(%s)", inputStringPos.toStdString());
		QString noticeMsg = QObject::tr("格式解析错误。\n示例格式(数字不能以0开头，且只能两位):"\
			"12或12,13,60或12,13-15,16,17,78-79或2-3,4-7,8-9,99,98,96,54-57");
		TipDlg(noticeMsg).exec();
		return false; 
	}

	// 若没有数据，直接返回
	QStringList selectPostionList = inputStringPos.split(QRegExp("[,，]"));
	for (const QString& pospara : selectPostionList)
	{
		QStringList postions = pospara.split("-");
		if (postions.empty())
		{
			continue;
		}

		// 只有一个位置数据
		if (postions.size() == 1)
		{
			vctPositons.push_back(postions[0].toInt()); // 放入位置
			continue;
		}

		int startPos = postions[0].toInt(); // 开始位置
		int endPos = postions[1].toInt(); // 结束位置
		while (startPos <= endPos)
		{
			vctPositons.push_back(startPos++);
		}
	}

	// 判断填写的位置是否为空
	if (vctPositons.size() == 0)
	{
		TipDlg(QObject::tr("获取到的位置为空！")).exec();
		return false;
	}
	else
	{
		// 重复检测
		std::set<int> uniq;
		for (int vItem : vctPositons)
		{
			if (uniq.find(vItem) == uniq.end())
			{
				uniq.insert(vItem);
			}
			else
			{
				TipDlg(QObject::tr("输入的位置存在重复，重复的试剂位为：%1！").arg(vItem)).exec();
				return false;
			}
		}
	}

	return true;
}

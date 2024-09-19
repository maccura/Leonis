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
/// @file     uidcsadapter.h
/// @brief    UI-DCS接口适配器
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
#pragma once

#include "adaptertypedef.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include <QMap>
#include <tuple>

// 获取对应设备的适配器实例
#define   gUiAdapterPtr(...)                       UiDcsAdapter::GetInstance(__VA_ARGS__)

// UI-DCS接口适配器
class UiDcsAdapter
{
public:
    ~UiDcsAdapter();

    ///
    /// @brief
    ///     注册所有设备
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月12日，新建函数
    ///
    static bool RegisterDevice();

    ///
    /// @brief
    ///     获取单例对象
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  iModuleNo   模块号，0表示整机(比色和免疫设备没有使用该字段，固定为1)
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    static UiDcsAdapter* GetInstance(const QString strDevName = "", int iModuleNo = 0);

    ///
    /// @brief
    ///     获取单例对象
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  strGrpName  组名
    /// @param[in]  iModuleNo   模块号，0表示整机(比色和免疫设备没有使用该字段，固定为1)
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月25日，新建函数
    ///
    static UiDcsAdapter* GetInstance(const QString& strDevName, const QString& strGrpName, int iModuleNo = 0);

    ///
    /// @brief
    ///     获取设备类别
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  strGrpName  组名
    ///
    /// @return 设备类别
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月6日，新建函数
    ///
    DEVICE_CLASSIFY GetDeviceClasssify(const QString& strDevName, const QString strGrpName = "");

	///
	/// @brief 获取设备类型
	///
	/// @param[in]  strDevName  设备名
	/// @param[in]  strGrpName  组名
	///
	/// @return 设备类型
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月9日，新建函数
	///
	tf::DeviceType::type GetDeviceType(const QString& strDevName, const QString strGrpName = "");

    ///
    /// @brief
    ///     是否包含其他子设备
    ///
    /// @param[in]  strDevName  设备名
    ///
    /// @return true表示是组设备
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月18日，新建函数
    ///
    bool WhetherContainOtherSubDev(const QString& strDevName);

    ///
    /// @brief
    ///     是否是单机版
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月31日，新建函数
    ///
    bool WhetherSingleDevMode();

    ///
    /// @brief
    ///     是否只有免疫设备
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月31日，新建函数
    ///
    bool WhetherOnlyImDev();

	///
	/// @brief
	///     是否只有生化设备
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月26日，新建函数
	///
	bool WhetherOnlyChDev();

	///
	/// @brief 目前的机型是否支持该维护项
	///
	/// @param[in]  maintainItem  
	///
	/// @return true:支持
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年10月17日，新建函数
	///
	bool WhetherSupportTargetMaintainItem(const ::tf::MaintainItem& maintainItem);

    ///
    /// @brief
    ///     是否包含免疫设备
    ///
    /// @return true表示包含免疫设备
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月4日，新建函数
    ///
    bool WhetherContainImDev();

    ///
    /// @brief
    ///     获取免疫设备序列号列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月7日，新建函数
    ///
    QStringList GetImDevSnLst();

	///
	/// @brief
	///     是否包含目标设备(测试项目类型分类)
	///
	/// @param[in]  devClassify  目标设备类型
	///
	/// @return true表示包含
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
	///
	bool WhetherContainTargetDevClassify(const tf::AssayClassify::type &devClassify);

    ///
    /// @brief
    ///     获取设备名列表
    ///
    /// @param[in]  strDevNameList  设备名列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    bool GetDevNameList(QList<QPair<QString, QString>>& strDevNameList);

    ///
    /// @brief
    ///     获取设备名列表（包含轨道）
    ///
    /// @param[in]  strDevNameList  设备名列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月30日，新建函数
    ///
    bool GetDevNameListWithTrack(QList<QPair<QString, QString>>& strDevNameList);

    ///
    /// @brief
    ///     获取组设备和独立设备列表
    ///
    /// @param[in]  strDevNameList  获取组设备和独立设备列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月18日，新建函数
    ///
    bool GetGroupDevNameList(QStringList& strDevNameList);

    ///
    /// @brief
    ///     获取组设备和独立设备列表
    ///
    /// @param[in]  strDevNameList  获取组设备和独立设备列表
    /// @param[in]  enTfDevType     设备类型
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    bool GetGroupDevNameList(QStringList& strDevNameList, tf::DeviceType::type enTfDevType);

    ///
    /// @brief
    ///     获取组设备和独立设备列表
    ///
    /// @param[out]  strDevNameList  获取组设备和独立设备列表
	/// @param[in]  needSnSort  需要排序(true:按照sn升序排列, false:按照名称排序)
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月25日，新建函数
    ///
    bool GetGroupDevNameListWithTrack(QStringList& strDevNameList, bool needSnSort = false);

	///
	/// @brief 获取设备序列号
	///
	/// @param[out]  vecDevSn  获取的设备序列号，按照sn升序排列
	///
	/// @return true:获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月13日，新建函数
	///
	bool GetDevSnVectorWithTrack(QVector<QString>& vecDevSn);

    ///
    /// @brief
    ///     获取组设备的子设备列表
    ///
    /// @param[in]  strGrpDevName   组设备名
    /// @param[in]  strDevNameList  子设备列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月18日，新建函数
    ///
    bool GetSubDevNameList(const QString& strGrpDevName, QStringList& strDevNameList);

	///
	/// @brief
	///     获取组设备的子设备列表，例如CH1,CH3A,CH3B
	///
	/// @param[in]  strDevNameList  子设备列表
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月30日，新建函数
	///
	bool GetSubDevCombineNameList(QStringList& strDevNameList);

    ///
    /// @brief
    ///     通过设备序列号获取设备名
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return (设备名，组名)
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月20日，新建函数
    ///
    QPair<QString, QString> GetDevNameBySn(const QString& strDevSn);

    ///
    /// @brief
    ///     根据设备名获取设备序列号
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  strGrpName  组名
    ///
    /// @return 设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    QString GetDevSnByName(const QString& strDevName, const QString strGrpName = "");

	///
	/// @brief
	///     通过设备序列号获取设备类型
	///
	/// @param[in]  strDevSn  设备序列号
	///
	/// @return 设备名
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
	///
	tf::DeviceType::type GetDevTypeBySn(const QString& strDevSn);

    ///
    /// @brief
    ///     获取ISE以外的设备名列表
    ///
    /// @param[in]  strDevNameList  ISE以外设备名列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年2月3日，新建函数
    ///
    bool GetDevNameListButIse(QList<QPair<QString, QString>>& strDevNameList);

    ///
    /// @brief
    ///     获取组设备列表（ISE设备除外）
    ///
    /// @param[in]  strDevNameList  获取组设备和独立设备列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月18日，新建函数
    ///
    bool GetGroupDevNameListButIse(QStringList& strDevNameList);

    ///
    /// @brief
    ///     获取质控品名称列表
    ///
    /// @param[out]  strQcDocList  质控品名称列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool GetQcDocList(QStringList& strQcDocList);

    ///
    /// @brief
    ///     获取ISE以外的质控品名称列表
    ///
    /// @param[out]  strQcDocList  ISE以外的质控品名称列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年2月3日，新建函数
    ///
    bool GetQcDocListButIse(QStringList& strQcDocList);

    ///
    /// @brief
    ///     查询质控申请信息
    ///
    /// @param[in]  strQcName        质控名称
    /// @param[out] stuQcApplyInfo   质控申请信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool QueryQcApplyTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo);

    ///
    /// @brief
    ///     更新质控申请信息是否选择
    ///
    /// @param[in&out]  stuQcApplyItem  质控申请信息(传入原质控申请信息，选择更新后传回新的质控申请信息)
    /// @param[in]      bSelected       是否选择
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool UpdateQcApplyItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected = true);

    ///
    /// @brief
    ///     查询默认质控信息
    ///
    /// @param[in]  strQcName        质控名称
    /// @param[out] stuQcApplyInfo   质控申请信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool QueryDefaultQcTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo);

    ///
    /// @brief
    ///     更新默认质控信息是否选择
    ///
    /// @param[in]  stuQcApplyItem  质控申请信息
    /// @param[in]  bSelected       是否选择
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool UpdateDefaultQcItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected = true);

    ///
    /// @brief
    ///     查询备用瓶质控信息
    ///
    /// @param[in]  strQcName        质控名称
    /// @param[out] stuQcApplyInfo   质控申请信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool QueryBackupRgntQcTblInfo(const QString& strQcName, QList<QC_APPLY_INFO>& stuQcApplyInfo);

    ///
    /// @brief
    ///     更新备用瓶质控信息是否选择
    ///
    /// @param[in]  stuQcApplyItem  质控申请信息
    /// @param[in]  bSelected       是否选择
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    bool UpdateBackupRgntItemSelected(QC_APPLY_INFO& stuQcApplyItem, bool bSelected = true);

    ///
    /// @bref
    ///		获取设备的试剂槽数量
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月28日，新建函数
    ///
    int  GetReagentSlotNumber();
	
    /// @brief
    ///     获取项目列表
    ///
    /// @param[in]  strAssayList  项目列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool GetAssayList(QStringList& strAssayList);

    /// @brief
    ///     获取质控LJ项目列表
    ///
    /// @param[in]  strAssayList  项目列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool GetQcLjAssayList(QStringList& strAssayList);

    /// @brief
    ///     获取质控TP项目列表
    ///
    /// @param[in]  strAssayList  项目列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool GetQcTpAssayList(QStringList& strAssayList);

    ///
    /// @brief
    ///     查询质控靶值信息
    ///
    /// @param[in]   qryCond            查询条件
    /// @param[out]  stuQcConcInfoList  查询结果列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool QueryQcDocConcInfo(const QC_CONC_INFO_QUERY_COND& qryCond, QList<QC_DOC_CONC_INFO>& stuQcConcInfoList);

    ///
    /// @brief
    ///     查询质控结果信息
    ///
    /// @param[in]   qryCond            查询条件
    /// @param[out]  stuQcRltList       查询结果列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool QueryQcRltInfo(const QC_RESULT_QUERY_COND& qryCond, QList<QC_RESULT_INFO>& stuQcRltList);

    ///
    /// @brief
    ///     更新靶值SD
    ///
    /// @param[in]  stuDocInfo      质控品信息
    /// @param[in]  strAssayName    项目名
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月27日，新建函数
    ///
    bool UpdateQcTargetValSD(::tf::ResultLong& _return, const QC_DOC_CONC_INFO& stuDocInfo, const QString& strAssayName);

    ///
    /// @brief
    ///     更新质控失控原因和处理措施
    ///
    /// @param[in]  stuQcRlt        质控结果
    /// @param[in]  strReason       失控原因
    /// @param[in]  strSolution     处理措施
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool UpdateQcOutCtrlReasonAndSolution(QC_RESULT_INFO& stuQcRlt, const QString& strReason, const QString& strSolution);

    ///
    /// @brief
    ///     更新质控失控原因和处理措施
    ///
    /// @param[in]  stuQcRlt        质控结果
    /// @param[in]  strReason       失控原因
    /// @param[in]  strSolution     处理措施
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月3日，新建函数
    ///
    bool UpdateQcOutCtrlReasonAndSolution(TWIN_QC_RESULT_INFO& stuQcRlt, const QString& strReason, const QString& strSolution);

    ///
    /// @brief
    ///     更新单质控计算点
    ///
    /// @param[in]  stuQcRlt    质控结果
    /// @param[in]  bCalculate  是否计算
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool UpdateQcCalcPoint(QC_RESULT_INFO& stuQcRlt, bool bCalculate);

    ///
    /// @brief
    ///     更新联合质控计算点
    ///
    /// @param[in]  stuQcRlt    质控结果
    /// @param[in]  bCalculate  是否计算
    /// @param[in]  bTwinQc     是否是联合质控
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月29日，新建函数
    ///
    bool UpdateQcCalcPoint(TWIN_QC_RESULT_INFO& stuQcRlt, bool bCalculate);

    ///
    /// @brief
    ///     查询联合质控信息
    ///
    /// @param[in]  qryCond             查询条件
    /// @param[in]  stuTwinQcRltList    结果列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月30日，新建函数
    ///
    bool QueryQcYoudenRltInfo(const QC_RESULT_QUERY_COND& qryCond, QList<TWIN_QC_RESULT_INFO>& stuTwinQcRltList);

    ///
    /// @brief
    ///     获取耗材需求信息
    ///
    /// @param[out] stuSplReqInfoList   耗材需求信息列表
    /// @param[in]  iDevClassify        设备类别
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月25日，新建函数
    ///
    bool GetSplReqInfo(QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList, int iDevClassify);

    ///
    /// @brief
    ///     保存耗材需求信息
    ///
    /// @param[in]  stuSplReqInfoList  耗材需求信息列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    bool SaveSplReqInfo(const QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList);

private:
    // 单例模式
    UiDcsAdapter();

    ///
    /// @brief
    ///     加载免疫试剂信息到耗材需求映射表
    ///
    /// @param[out]  mapSplReqInfo  耗材需求映射表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月27日，新建函数
    ///
    bool LoadImRgntInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo);

    ///
    /// @brief
    ///     加载免疫稀释液信息到耗材需求映射表
    ///
    /// @param[out]  mapSplReqInfo  耗材需求映射表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月27日，新建函数
    ///
    bool LoadImDltInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo);

    ///
    /// @brief    加载生化仓内试剂和耗材信息到耗材需求映射表
    ///
    /// @param[out]  mapSplReqInfo  耗材需求映射表
    ///
    /// @return true表示成功
    ///
    /// @par History:
	/// @li 7997/XuXiaoLong，2024年8月20日，新建函数
    ///
    bool LoadChRgntSpyInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo);

	///
	/// @brief  加载生化仓内试剂信息
	///
	/// @param[in]  spDevInfo 设备指针
	/// @param[in]  rgtInfos  试剂信息
	/// @param[out]  mapSplReqInfo  耗材需求映射表
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月20日，新建函数
	///
	void LoadChRgtInfo(std::shared_ptr<const tf::DeviceInfo> spDevInfo, const std::vector<::ch::tf::ReagentGroup>& rgtInfos,
		QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo);

	///
	/// @brief  加载生化仓内耗材信息
	///
	/// @param[in]  spDevInfo   设备指针
	/// @param[in]  supplyInfo  耗材信息
	/// @param[out]  mapSplReqInfo  耗材需求映射表
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月20日，新建函数
	///
	void LoadChSpyInfo(std::shared_ptr<const tf::DeviceInfo> spDevInfo, const ::ch::tf::SuppliesInfo& supplyInfo, 
		QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo);

    ///
    /// @brief
    ///     加载耗材需求映射表
    ///
    /// @param[out]  mapSplReqInfo  耗材需求映射表
    /// @param[out]  iDevClassify   设备类别
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月27日，新建函数
    ///
    bool LoadSplReqInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, int iDevClassify);

    ///
    /// @brief
    ///     加载耗材动态计算值
    ///
    /// @param[out]  mapSplReqInfo  耗材需求映射表
    /// @param[out]  iDevClassify   设备类别
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月5日，新建函数
    ///
    bool LoadDynCalcInfoToMap(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, int iDevClassify);

    ///
    /// @brief
    ///     耗材需求信息构造字符串并压入列表
    ///
    /// @param[out]  mapSplReqInfo       耗材需求映射表
    /// @param[out]  stuSplReqInfoList   耗材需求信息列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月27日，新建函数
    ///
    bool MakeStrForSplReqInfo(QMap<std::tuple<int, bool, int>, SPL_REQ_VOL_INFO>& mapSplReqInfo, QList<SPL_REQ_VOL_INFO>& stuSplReqInfoList);

private:
    // 设备名
    QString                     m_strDevName;

    // 组名
    QString                     m_strGrpName;

    // 组合设备映射其子设备列表
    QMap<QString, QStringList>  m_mapDevGroup;

    // 模块号
    int                         m_iModuleNo;
};

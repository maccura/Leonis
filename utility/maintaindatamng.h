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
/// @file     maintainwidget.h
/// @brief    应用->维护保养界面
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QObject>
#include <QMap>
#include <QSet>
#include <QList>
#include "src/thrift/gen-cpp/defs_types.h"

class TipDlg;


class MaintainDataMng : public QObject
{
    Q_OBJECT

public:
    // 维护模式
    enum MaintainMode
    {
        MAINTAIN_MODE_NORMAL = 0,       // 普通维护
        MAINTAIN_MODE_REMAINTAIN        // 重新维护
    };

public:
    ~MaintainDataMng();

    ///
    /// @brief
    ///     获取单例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    static MaintainDataMng* GetInstance();

    ///
    /// @brief
    ///     获取维护组设备类型列表
    ///
    /// @param[in]  strMaintainGrpName  维护组名称
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月19日，新建函数
    ///
    QSet<tf::DeviceType::type> GetDeviceTypesForMaintainGrp(const QString& strMaintainGrpName);

    ///
    /// @brief
    ///     设备模块是否正在执行
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  iModuleIdx  模块索引
    ///
    /// @return true表示正在执行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月26日，新建函数
    ///
    bool IsDevModuleMaintaining(const QString& strDevSn, int iModuleIdx);

    ///
    /// @brief
    ///     获取所有维护组
    ///
    /// @param[in]  grpList  维护组列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月15日，新建函数
    ///
    bool GetAllMaintainGrp(QList<tf::MaintainGroup>& grpList);

    ///
    /// @brief
    ///     获取所有维护详情
    ///
    /// @param[in]  grpList  维护组列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    bool GetAllMaintainDetail(QList<tf::LatestMaintainInfo>& detailList);

    ///
    /// @brief
    ///     通过维护名获取维护组信息
    ///
    /// @param[in]  strName  维护名
    ///
    /// @return 维护组信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月15日，新建函数
    ///
    tf::MaintainGroup GetMaintainGrpByName(QString strName);

	
	///
	/// @brief 根据维护单项类型获取单项维护组（只能查找单项维护组）
	///
	/// @param[in]  itemType  维护项类型
	///
	/// @return 单项维护组信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	tf::MaintainGroup GetSingleMaintainGrpByType(::tf::MaintainItemType::type itemType);

	///
	/// @brief 根据维护组类型获取维护组（只能查找组合维护组）
	///
	/// @param[in]  groupType  维护组类型
	///
	/// @return 维护组信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	tf::MaintainGroup GetCombinMaintainGrpByType(::tf::MaintainGroupType::type groupType);

    ///
    /// @brief
    ///     通过维护组ID获取维护组信息
    ///
    /// @param[in]  lMtGrpId  维护组ID
    ///
    /// @return 维护组信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    tf::MaintainGroup GetMaintainGrpById(long long lMtGrpId);

    ///
    /// @brief
    ///     通过设备序列号获取维护详情信息
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return 维护详情信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    tf::LatestMaintainInfo GetMaintainDetailByDevSn(QString strDevSn);

    ///
    /// @brief
    ///     通过设备组名获取维护详情
    ///
    /// @param[in]  strDevGrpName  设备组名
    ///
    /// @return 设备组维护详情数组
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月18日，新建函数
    ///
    QVector<tf::LatestMaintainInfo> GetMaintainDetailByDevGrpName(QString strDevGrpName);

    ///
    /// @brief
    ///     根据维护项类型获取维护名
    ///
    /// @param[in]  enItemType  维护项类型
    ///
    /// @return 维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月15日，新建函数
    ///
    QString GetMaintainItemNameByType(tf::MaintainItemType::type enItemType);

    ///
    /// @brief
    ///     根据维护项类型获取维护项
    ///
    /// @param[in]  enItemType  维护项类型
    ///
    /// @return 维护项信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月12日，新建函数
    ///
    tf::MaintainItem GetMaintainItemByType(tf::MaintainItemType::type enItemType);

    ///
    /// @brief
    ///     根据维护项名称获取维护项
    ///
    /// @param[in]  strItemName  维护项名称
    ///
    /// @return 维护项信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月12日，新建函数
    ///
    tf::MaintainItem GetMaintainItemByName(const QString& strItemName);

    ///
    /// @brief
    ///     准备开启重新维护
    ///
    /// @param[in]  enMaintainMode  维护模式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月21日，新建函数
    ///
    inline void PrepareStartRemaintain()
    {
        m_enMaintainMode = MAINTAIN_MODE_REMAINTAIN;
    }

	///
	/// @brief 获取比色管路填充参数
	///
	/// @param[in]  bEACS   酸性清洗液是否启用
	/// @param[in]  bEALCS  碱性清洗液是否启用
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetChFluidPerfusionParam(const bool& bEACS, const bool& bEALCS);

	///
	/// @brief 获取光度计检查参数
	///
	/// @param[in]  bECC   修正校验是否启用
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetChPhotometerCheckParam(const bool& bECC);

	///
	/// @brief 获取比色反应杯更换参数
	///
	/// @param[in]  uniteNum  需要更换的联排
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetChChangeReactionCupParam(const std::vector<int>& uniteNum);

	///
	/// @brief 获取针清洗参数
	///
	/// @param[in]  bESPC  样本针清洗是否启用
	/// @param[in]  bERPC  试剂针清洗是否启用
	/// @param[in]  iRPCT  试剂针清洗次数
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetProbeCleanParam(const bool& bESPC, const bool& bERPC);

	///
	/// @brief 获取ISE电极更换参数
	///
	/// @param[in]  bNa  Na电极是否更换
	/// @param[in]  bK  K电极是否更换
	/// @param[in]  bCl  Cl电极是否更换
	/// @param[in]  bRef  Ref电极是否更换
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetIseElecReplaceParam(const bool& bNa, const bool& bK, const bool& bCl, const bool& bRef);

	///
	/// @brief 获取ISE管路填充参数
	///
	/// @param[in]  bESS  内部标准液是否启用
	/// @param[in]  bEB   缓冲液是否启用
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetIseFluidPerfusionParam(const bool& bESS, const bool& bEB);

	///
	/// @brief 获取ISE清洗液余量探测参数
	///
	/// @param[in]  vecPos  探测位置信息
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年8月13日，新建函数
	///
	std::string GetIseSurplusDetectionParam(const std::vector<int>& vecPos);

	///
	/// @brief 获取反应槽水更换参数
	///
	/// @param[in]  iMode  更换模式
	///
	/// @return 失败返回空字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月5日，新建函数
	///
	std::string GetReacTankWaterReplaceParam(const int& iMode);

	///
	/// @brief 检查用户输入的维护参数数据，次数：（1-99），时间：（1-60min）
	///
	/// @param[in]  frequencyVec    次数参数
	/// @param[in]  timeVec			时间参数
	///
	/// @return true:检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月12日，新建函数
	///
	bool CheckMaintainItemInputParams(const QVector<int>& frequencyVec, const QVector<int>& timeVec);

	///
	///  @brief 过滤ui不显示的维护组
	///
	///
	///  @param[in]   groups  维护组列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void FilterGroups(std::vector<tf::MaintainGroup> &groups);

    ///
    /// @brief
    ///     当前用户是否可以执行维护单项
    ///
    /// @param[in]  strCurMtItemName  维护单项名称
    ///
    /// @return true表示允许执行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年5月11日，新建函数
    ///
    bool WhetherItemCanExeByCurUser(const QString& strCurMtItemName);

    ///
    /// @brief
    ///     加载数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月15日，新建函数
    ///
    void LoadData();

	///
	/// @brief
	///     更新对应设备的维护详情
	///
	/// @param[in]  strDevSn  设备序列号
	///
	/// @par History:
	/// @li 4170/TangChuXian，2023年5月29日，新建函数
	///
	void UpdateMaintainDetail(const QString& strDevSn);

    // 更新m_mapMaintainDetail记录中的最近维护， 此处入参maintGroups是同一设备的维护记录
    void UpdateLatestMaintainGroup(const std::string& devSN);

	///
	/// @brief 检查设备是否可执行维护
	///
	/// @param[out] noticeInfo  检查后的提示信息
	/// @param[in]  dev  需要执行的设备
	/// @param[in]  mtGroup  需要执行的维护组
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool CheckDevIsReadyForMaintain(QString& noticeInfo, const ::tf::DevicekeyInfo& dev, const tf::MaintainGroup& mtGroup);

protected:
    MaintainDataMng(QObject *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     设备是否可执行某个维护项
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  enItemType  维护项类型
    ///
    /// @return true表示可执行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月12日，新建函数
    ///
    bool WhetherDevColudExeItem(const QPair<QString, QString>& strDevName, tf::MaintainItemType::type enItemType);

	///
	/// @brief 比色设备的绑定设备状态判断
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool ChBindDevStatusCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 比色设备故障判断
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool ChFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief ISE绑定设备状态检查
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool IseBindDevStatusCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief ISE设备故障信息检查
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool IseFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 轨道故障信息检查
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool TrackFaultCheck(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 检查比色设备是否能够执行维护
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool CheckChDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 检查ISE设备是否能够执行维护
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool CheckIseDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 检查免疫设备是否能够执行维护
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool CheckImDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

	///
	/// @brief 检查轨道设备是否能够执行维护
	///
	/// @param[out]  noticeInfo  提示信息
	/// @param[in]  devInfo  设备信息
	/// @param[in]  mtGroup  执行的维护组
	///
	/// @return true表示检查通过
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月12日，新建函数
	///
	bool CheckTrackDevIsReadyForMaintain(QString& noticeInfo, std::shared_ptr<const tf::DeviceInfo> devInfo, const tf::MaintainGroup& mtGroup);

protected Q_SLOTS:

	///
	///  @brief 更新维护组
	///
	///
	///  @param[in]   lmi  最近执行情况
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月2日，新建函数
	///
	void OnUpdateMaintainGroup(tf::LatestMaintainInfo lmi);

    ///
    /// @brief
    ///     维护数据初始化
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月15日，新建函数
    ///
    void OnMaintainDataInit(QString strDevSn);
	
	///
	///  @brief 更新维护项
	///
	///
	///  @param[in]   devSN		设备序列号
	///  @param[in]   groupId	维护组id
	///  @param[in]   mier		执行结果
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月2日，新建函数
	///
	void OnUpdatedMaintainItem(QString devSN, long long groupId, tf::MaintainItemExeResult mier);

    ///
    /// @brief
    ///     更新维护组数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void UpdateMaintainGrpData();

    ///
    /// @brief
    ///     维护组阶段更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enPhaseType     阶段类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月24日，新建函数
    ///
    void OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType);

	///
	/// @brief
	///     维护组阶段更新
	///
	/// @param[in]  lmi     正在执行的维护组信息
	///
	/// @par History:
	/// @li 4170/TangChuXian，2023年2月24日，新建函数
	///
	void OnMaintainGrpProgressUpdate(tf::LatestMaintainInfo lmi);

    ///
    /// @brief
    ///     维护组执行时间更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  strMaintainTime 维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainGrpTimeUpdate(QString strDevSn, long long lGrpId, QString strMaintainTime);

    ///
    /// @brief
    ///     维护项状态更新
    ///
    /// @param[in]  strDevSn        设备序列号
	/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
	/// @param[in]  lGrpId			维护组ID
    /// @param[in]  enItemType      维护项类型
    /// @param[in]  enPhaseType     阶段类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType);

    ///
    /// @brief
    ///     维护项时间更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enItemType      维护项类型
    /// @param[in]  strTime         时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainItemTimeUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, QString strTime);

    ///
    /// @brief
    ///     维护项进度更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enItemType      维护项类型
    /// @param[in]  iProgress       进度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainItemProgressUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, int iProgress);

    ///
    /// @brief
    ///     维护失败原因已经被阅读
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月4日，新建函数
    ///
    void OnMaintainFailedCauseRead();

	///
	///  @brief 维护失败原因
	///
	///
	///  @param[in]   strDevSn		设备序列号
	///  @param[in]   lGrpId		维护组ID
	///  @param[in]   failCause		失败原因，参见 tf::MaintainFailCause::type
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月29日，新建函数
	///
	void OnMaintainFailCauseUpdate(QString strDevSn, long long lGrpId, int failCause);
private:
	
	///
	///  @brief 查询所有维护组
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void QueryMaintainGroup();
	
	///
	///  @brief 查询所有维护项
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void QueryMaintainItem();
	
	///
	///  @brief 查询所有最近维护信息
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void QueryLatestMaintainInfo();

	///
	///  @brief 更新缓存(设备状态与维护状态不匹配，则更新缓存)
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void UpdateCache();

private:
    MaintainMode                            m_enMaintainMode;           // 维护模式

    QList<tf::MaintainGroup>                m_stuMtGrpList;             // 维护组列表
    QMap<QString, tf::MaintainGroup>        m_mapMaintainGrp;           // 维护组【名称-信息】
    QMap<QString, tf::MaintainGroup>        m_mapMaintainItemGrp;       // 维护单项【名称-信息】
    QMap<QString, tf::MaintainGroup>        m_mapMaintainFilterGrp;     // 维护单项【名称-信息】
    QMap<QString, tf::MaintainItem>         m_mapMaintainItem;          // 维护项【名称-信息】
    QMap<QString, tf::MaintainItem>         m_mapWithPermissionItem;    // 当前用户拥有权限维护项【名称-信息】
    QMap<QString, tf::LatestMaintainInfo>   m_mapMaintainDetail;        // 维护详情【设备序列号-信息】
    QMap<QPair<QString, tf::MaintainItemType::type>, int>   m_mapMaintainItemExeStep;   // 维护项执行步数【<设备序列号-维护单项>-执行步数】

    TipDlg*                                 m_pTipDlg;                  // 提示对话框
    QString                                 m_strMaintainFailedCause;   // 维护失败原因
};

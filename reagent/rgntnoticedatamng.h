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
/// @file     rgntnoticedatamng.h
/// @brief    试剂提醒数据管理器
///
/// @author   4170/TangChuXian
/// @date     2023年8月8日
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

#include "src/thrift/im/i6000/gen-cpp/i6000UiControl.h"
#include "src/public/im/ImConfigDefine.h"
#include <boost/optional/optional.hpp>
#include <QObject>
#include <QMap>
#include <QSet>

// 耗材使能信息
struct  ImSuppliesEnableInfo
{
    bool bBaseLiquidGrp1;
    bool bBaseLiquidGrp2;
    bool bCleanBuffer1;
    bool bCleanBuffer2;

    ImSuppliesEnableInfo()
    {
        bBaseLiquidGrp1 = true;
        bBaseLiquidGrp2 = true;
        bCleanBuffer1 = true;
        bCleanBuffer2 = true;
    }
};

class RgntNoticeDataMng : public QObject
{
    Q_OBJECT

public:
    enum DltType
    {
        DLT_TYPE_DILUENT = 0,       // 稀释液
        DLT_TYPE_SPEC_WASHING       // 特殊清洗液
    };

    enum ActionType
    {
        ACTION_TYPE_NONE = 0,       // 无
        ACTION_TYPE_LOAD_RGNT,      // 加载试剂
        ACTION_TYPE_UNLOAD_RGNT     // 卸载试剂
    };

public:
    ~RgntNoticeDataMng();

    ///
    /// @brief
    ///     获取单例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    static RgntNoticeDataMng* GetInstance();

    ///
    /// @brief
    ///     试剂剩余测试数是否提醒
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  iResidetest 剩余测试数
    ///
    /// @return true表示需要提醒
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月10日，新建函数
    ///
    bool IsRgntResidualTestNotice(int iAssayCode, int iResidetest);

    ///
    /// @brief
    ///     试剂盘上是否存在空试剂或空稀释液
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年5月9日，新建函数
    ///
    bool IsExistEmptyRgntOrDlt();

    ///
    /// @brief
    ///     试剂可用测试数是否提醒
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  iAvatest    可用测试数
    ///
    /// @return true表示提醒
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月3日，新建函数
    ///
    bool IsRgntAvaTestNotice(int iAssayCode, int iAvatest);

    ///
    /// @brief
    ///     稀释液剩余量是否需要提醒
    ///
    /// @param[in]  iDltNum  稀释液编号
    /// @param[in]  dReVol   剩余量
    ///
    /// @return true表示需要提醒
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月10日，新建函数
    ///
    bool IsDltCurVolNotice(int iDltNum, double dReVol);

    ///
    /// @brief
    ///     稀释液可用量是否需要提醒
    ///
    /// @param[in]  iDltNum  稀释液编号
    /// @param[in]  dAvaVol  可用量
    ///
    /// @return true表示需要提醒
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月10日，新建函数
    ///
    bool IsDltAvaVolNotice(int iDltNum, double dAvaVol);

    ///
    /// @brief
    ///     耗材余量是否需要提醒
    ///
    /// @param[in]  iSplType  耗材类型
    /// @param[in]  dReVol    耗材余量
    ///
    /// @return true表示需要提醒
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月17日，新建函数
    ///
    bool IsSplReVolNotice(int iSplType, double dReVol);

    ///
    /// @brief 试剂加载机构试剂槽是否存在试剂盒
    ///
    /// @param[oyut]	bExist  true-存在，false-不存在
    /// @param[in]		deviceSN  设备编号
    ///
    /// @return true-获取成功，false-获取失败
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年9月4日，新建函数
    ///
    bool ReagentLoadExistReag(bool& bExist, std::string& deviceSN);

    ///
    /// @brief
    ///     查询试剂信息
    ///
    /// @param[in]  qryResp  返回结果
    /// @param[in]  qryCond  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    bool QueryReagentInfoForUI(::im::tf::ReagentInfoTableQueryResp& qryResp, const  ::im::tf::ReagTableUIQueryCond& qryCond);

    ///
    /// @brief
    ///     查询稀释液信息
    ///
    /// @param[in]  qryResp  返回结果
    /// @param[in]  qryCond  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    bool QueryDiluentInfoForUI(::im::tf::DiluentInfoTableQueryResp& dltr, const  ::im::tf::ReagTableUIQueryCond& dltc);

    ///
    /// @brief
    ///     查询耗材信息
    ///
    /// @param[in]  qryResp  查询结果
    /// @param[in]  qryCond  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    bool QuerySuppliesInfo(::im::tf::SuppliesInfoTableQueryResp& qryResp, const  ::im::tf::SuppliesInfoTableQueryCond& qryCond);

    ///
    /// @brief
    ///     是否可以加卸载试剂
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return true表示允许
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    bool IsCanLoadReagent(const QString& strDevSn);

    ///
    /// @brief
    ///     是否可以加卸载试剂
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return true表示允许
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    bool IsCanLoadReagent(const std::string& strDevSn);

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
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    bool IsReagentLoadIsMask(bool& bMask, const std::string& deviceSN);

    ///
    /// @brief
    ///     设置装载装置复位正在执行
    ///
    /// @param[in]  strDevSn  设备序列号
    /// @param[in]  bExecute  是否正在执行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月22日，新建函数
    ///
    void SetDevExcuteRgntLoaderReset(const std::string& strDevSn, bool bExecute);

    ///
    /// @brief
    ///     设置设备装载装置执行失败
    ///
    /// @param[in]  strDevSn  设备序列号
    /// @param[in]  bFaulst   是否加卸载失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月22日，新建函数
    ///
    void SetDevRgntLoaderFault(const std::string& strDevSn, bool bFaulst);

    ///
    /// @brief
    ///     试剂装载装置是否正在复位
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return true正在复位
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月22日，新建函数
    ///
    bool IsRgntLoaderResetting(const std::string& strDevSn);

    ///
    /// @brief
    ///     是否上一次试剂加卸载失败
    ///
    /// @param[in]  strDevSn  设备序列还
    ///
    /// @return true表示上一次加卸载失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月22日，新建函数
    ///
    bool IsRgntLoaderFault(const std::string& strDevSn);

    ///
    /// @brief
    ///     设置上一个动作
    ///
    /// @param[in]  enActType  动作类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    inline void SetLastAction(ActionType enActType)
    {
        m_enLastAction = enActType;
    }

    ///
    /// @brief
    ///     获取上一个动作
    ///
    /// @return 动作类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    inline ActionType GetLastAction()
    {
        return m_enLastAction;
    }

    ///
    /// @brief
    ///     设置设备动作状态
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  enActType   动作类型
    /// @param[in]  bRunning    是否正在执行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    inline void SetDevActionStatus(const QString& strDevSn, ActionType enActType, bool bRunning)
    {
        m_mapDevActionStatus.insert(strDevSn, qMakePair(enActType, bRunning));
    }

    ///
    /// @brief
    ///     获取设备动作状态
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return 设备动作状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    inline boost::optional<QPair<ActionType, bool>> GetDevActionStatus(const QString& strDevSn)
    {
        auto it = m_mapDevActionStatus.find(strDevSn);
        if (it == m_mapDevActionStatus.end())
        {
            return boost::none;
        }

        return it.value();
    }

    ///
    /// @brief
    ///     是否开启卸载
    ///
    /// @return true表示已开启
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月23日，新建函数
    ///
    inline bool IsStartUnload(const QString& strDevSn)
    {
        return m_setStartLoadOrUnload.contains(strDevSn);
    }

    ///
    /// @brief
    ///     设置是否开启测试
    ///
    /// @param[in]  bStart  是否开启测试
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月23日，新建函数
    ///
    inline void SetStartUnload(const QString& strDevSn, bool bStart)
    {
        if (bStart)
        {
            m_setStartLoadOrUnload.insert(strDevSn);
        }
        else
        {
            m_setStartLoadOrUnload.remove(strDevSn);
        }
    }

    ///
    /// @brief
    ///     获取耗材停用信息
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年5月27日，新建函数
    ///
    ImSuppliesEnableInfo GetSplEnableInfo(const std::string& strDevSn);

protected:
    RgntNoticeDataMng(QObject *parent = Q_NULLPTR);

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
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     弹框提示请取走试剂盒
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月23日，新建函数
    ///
    void PopTakeRgntTip(const QString& strDevSn);

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新稀释液报警值
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月10日，新建函数
    ///
    void UpdateDltAlarmVol();

    ///
    /// @brief
    ///     更新项目报建值
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月15日，新建函数
    ///
    void UpdateAssayAlarmVol();

    ///
    /// @brief
    ///     试剂仓加卸载是否正在运行
    ///
    /// @param[in]  strDevSn  设备序列号
    /// @param[in]  bRunning  是否正在运行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    void OnRgntLoadStatusChanged(QString strDevSn, bool bRunning);

    ///
    /// @brief
    ///     拿走试剂盒消息
    ///
    /// @param[in]  strDevSn  设备编号
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年9月4日，新建函数
    /// @li 4170/TangChuXian，2023年9月8日，逻辑完善
    ///
    void OnTakeReagentMessage(QString strDevSn);

    ///
    /// @brief 响应申请试剂结果消息
    ///
    /// @param[in]  deviceSN  设备编号
    /// @param[in]  result    是否接受
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2024年1月11日，新建函数
    /// @li 4170/TangChuXian，2024年1月18日，逻辑完善
    ///
    void OnManualHandleReagResult(const QString& deviceSN, const int result);

    ///
    /// @brief
    ///     更新试剂信息
    ///
    /// @param[in]  srr   试剂信息
    /// @param[in]  type  试剂更新类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    void UpdateReagentInfo(const im::tf::ReagentInfoTable& srr, const im::tf::ChangeType::type type);

    ///
    /// @brief
    ///     免疫稀释液信息更新
    ///
    /// @param[in]  stuDltInfo   试剂信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    void UpdateImDltChanged(const im::tf::DiluentInfoTable& stuDltInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     免疫耗材信息更新
    ///
    /// @param[in]  stuSplInfo   耗材信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月24日，新建函数
    ///
    void UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     设备状态改变
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     功能管理更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    void OnFunctionMngUpdate();

    ///
    /// @brief
    ///     检测模式更新处理
    ///
    /// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月4日，新建函数
    ///
    void OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo);

    ///
    /// @brief	设备信息更新
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月5日，新建函数
    /// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
    ///
    void UpdateDeviceStatus(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     耗材管理设置改变
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  stuSplMng   耗材管理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月27日，新建函数
    ///
    void OnSplMngCfgChanged(QString strDevSn, ImDeviceOtherInfo stuSplMng);

    ///
    /// @brief
    ///     试剂装载装置复位技术
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  rlt         复位结果
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月19日，新建函数
    ///
    void OnRgntLoaderResetFinished(QString strDevSn, im::i6000::tf::ReagentLoaderResetRlt::type rlt);

private:
    bool                                    m_bInit;                    // 是否初始化
    QMap<int, int>                          m_mapRgntBottleVol;         // 试剂瓶报警值【项目编号-瓶报警值】
    QMap<int, int>                          m_mapAssayAlarmVol;         // 项目报警值【项目编号-项目报警值】
    QMap<int, double>                       m_mapDltBottleAlarmVol;     // 稀释液报警值【稀释液类型-瓶报警值】
    QMap<int, double>                       m_mapDltTypeAlarmVol;       // 稀释液报警值【稀释液类型-项目报警值】
    QMap<int, double>                       m_mapSplAlarmVol;           // 耗材报警值【耗材类型-报警值】

    // 设备加卸载状态
    ActionType                              m_enLastAction;             // 上一个设备动作
    QMap<QString, QPair<ActionType, bool>>  m_mapDevActionStatus;       // 设备动作状态
    QSet<QString>                           m_setStartLoadOrUnload;     // 启动加卸载设备集合

    // 缓存试剂和稀释液、耗材，提高界面刷新效率
    bool                                    m_bRgntInfoUpdateReq;       // 试剂信息需要更新
    bool                                    m_bDltInfoUpdateReq;        // 稀释液信息需要更新
    bool                                    m_bSplInfoUpdateReq;        // 耗材信息需要更新
    std::vector<std::string>                m_vStrAllImDevSns;          // 全部免疫设备序列号列表
    QMap<int, im::tf::ReagentInfoTable>     m_mapRgntInfo;              // 试剂信息【ID-试剂信息】
    QMap<int, im::tf::DiluentInfoTable>     m_mapDltInfo;               // 稀释液信息【ID-稀释液信息】
    QMap<int, im::tf::SuppliesInfoTable>    m_mapSplInfo;               // 耗材信息【ID-耗材信息】

    // 记录影响试剂界面按钮使能的状态
    QStringList                             m_strCanLoadRgntDevSnLst;   // 可加卸载试剂设备序列号列表
    QMap<std::string, bool>                 m_mapDevRgntLoadMask;       // 设备是否屏蔽了在线加载试剂功能【设备序列号-是否屏蔽】

    // 缓存耗材停用状态
    QMap<std::string, ImSuppliesEnableInfo> m_mapSplEnable;             // 缓存耗材停用信息

    // 正在装载装置复位的设备集合
    QSet<QString>                           m_setDevRgntLoaderReset;    // 装载装置复位

    // 上一次加卸载复位失败的设备集合
    QSet<QString>                           m_setDevRgntLoaderFault;    // 上一次加卸载失败的设备
};

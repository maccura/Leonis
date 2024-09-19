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
/// @file     rgntplatewidget.h
/// @brief    试剂盘界面
///
/// @author   4170/TangChuXian
/// @date     2022年8月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QMap>
#include <memory>

#include "shared/checkableframe.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/public/im/ImConfigDefine.h"

namespace Ui { class ImRgntPlateWidget; };

// 前置声明
class   ImMcReagentPlate;
class   QReagentScanDialog;
enum    ImReagentState;
struct  ImReagentInfo;
class   QLabel;


// 耗材使能信息
struct  SuppliesEnableInfo
{
    bool bBaseLiquidGrp1;
    bool bBaseLiquidGrp2;
    bool bCleanBuffer1;
    bool bCleanBuffer2;
    bool bWasteLiquid;

    SuppliesEnableInfo()
    {
        bBaseLiquidGrp1 = true;
        bBaseLiquidGrp2 = true;
        bCleanBuffer1 = true;
        bCleanBuffer2 = true;
        bWasteLiquid = true;
    }
};

class ImRgntPlateWidget : public QWidget
{
    Q_OBJECT

public:
    // 耗材状态
    enum SuppliesStatus
    {
        SPL_STATUS_USING = 0,               // 在用
        SPL_STATUS_BACKUP,                  // 备用
        SPL_STATUS_REMAIND,                 // 提醒
        SPL_STATUS_WARN,                    // 警告
        SPL_STATUS_INVALID,                 // 失效
        SPL_STATUS_SCAN_FAILED,             // 扫描失败
        SPL_STATUS_MASK,                    // 屏蔽
        SPL_STATUS_EMPTY                    // 未放置
    };

    // 耗材类型
    enum SuppliesType
    {
        SPL_TYPE_BASE_LIQUID_A = 0,         // 底物液A
        SPL_TYPE_BASE_LIQUID_B,             // 底物液B
        SPL_TYPE_WASH_BUFFER,               // 清洗缓冲液
        SPL_TYPE_REACTION_CUP,              // 反应杯
        SPL_TYPE_WASTE_SOLID,               // 废料桶
        SPL_TYPE_WASTE_LIQUID               // 废液桶
    };

public:
    ImRgntPlateWidget(QWidget *parent = Q_NULLPTR);
    ~ImRgntPlateWidget();

    ///
    /// @brief
    ///     测试
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月22日，新建函数
    ///
    void Test();

    ///
    /// @brief
    ///     更新当前显示是被
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月1日，新建函数
    ///
    void UpdateCurDev(std::shared_ptr<const tf::DeviceInfo>& spDev);
	
	///
	/// @brief 设备状态变更
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月2日，新建函数
	///
	void DevStateChange();

protected:
    ///
    /// @brief
    ///     事件过滤器（主要用于监听场景事件）
    ///
    /// @param[in] obj    事件的目标对象
    /// @param[in] event  事件对象
    ///
    /// @return true表示事件已处理，false表示事件未处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月20日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月20日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月20日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月20日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月20日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

    ///
    /// @brief
    ///     清空选中试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void ClearSelRgntInfo();

    ///
    /// @brief
    ///     注册瓶子类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void RegisterBottleType();

    ///
    /// @brief
    ///     获取储液瓶类型
    ///
    /// @param[in]  enType    耗材类型
    /// @param[in]  enStatus  耗材状态
    ///
    /// @return 储液瓶类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月30日，新建函数
    ///
    int GetBottleType(SuppliesType enType, SuppliesStatus enStatus);

    ///
    /// @brief
    ///     获取选中耗材的耗材信息
    ///
    /// @return 储液瓶类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月30日，新建函数
    ///
    im::tf::SuppliesInfoTable GetSelSplInfo();

    ///
    /// @brief
    ///     重置耗材信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月30日，新建函数
    ///
    void ResetSplInfo();

    ///
    /// @brief
    ///     更新耗材显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月25日，新建函数
    ///
    void UpdateSplUiDisplay();

    ///
    /// @brief
    ///     清空耗材选中索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void ClearSplSelIndex();

    ///
    /// @brief
    ///     从试剂盘中移除试剂
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月7日，新建函数
    ///
    void RemoveRgntFromPlateUi(const im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     从试剂盘中移除稀释液
    ///
    /// @param[in]  stuDltInfo  稀释液信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月7日，新建函数
    ///
    void RemoveRgntFromPlateUi(const im::tf::DiluentInfoTable& stuDltInfo);

	///
	/// @brief 设置试剂扫描按钮是否允许状态，只针对免疫单机
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月2日，新建函数
	///
	void SetEnableOfReagSacnBtn();

    ///
    /// @brief
    ///     加载耗材使能信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月12日，新建函数
    ///
    void LoadSplEnableInfo();

    ///
    /// @brief
    ///     根据耗材使能映射更新UI显示
    ///
    /// @param[in]  mapSplWgts      启用耗材控件映射
    /// @param[in]  mapSplRowScale  启用行比例映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月12日，新建函数
    ///
    void UpdateSplUiByEnableMap(const QMap<int, QList<QWidget*>>& mapSplWgts, const QMap<int, int>& mapSplRowScale);

    ///
    /// @brief
    ///     无位置选择卸载试剂
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月10日，新建函数
    ///
    void UnloadRgntInNoSelPos();

protected Q_SLOTS:
    ///
    /// @brief
    ///     试剂屏蔽按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnRgntMaskBtnClicked();

    ///
    /// @brief
    ///     优先使用按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnPriorUseBtnClicked();

    ///
    /// @brief
    ///     试剂装载按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnLoadRgntBtnClicked();

    ///
    /// @brief
    ///     试剂卸载按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnUnloadRgntBtnClicked();

    ///
    /// @brief
    ///     磁珠混匀按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnMixBtnClicked();

    ///
    /// @brief
    ///     装载装置复位按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月4日，新建函数
    ///
    void OnLoadPartResetBtnClicked();

    ///
    /// @brief
    ///     试剂扫描按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月26日，新建函数
    ///
    void OnRgntScanBtnClicked();

    ///
    /// @brief
    ///     试剂盘选中索引改变
    ///
    /// @param[in]  iSelIndex  选中索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void OnRgntPlateSelIndexChanged(int iSelIndex);

    ///
    /// @brief
    ///     更新试剂盘
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月18日，新建函数
    ///
    void UpdateRgntPlate();

    ///
    /// @brief
    ///     更新试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void UpdateRgntInfo();

    ///
    /// @brief
    ///     更新稀释液信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月18日，新建函数
    ///
    void UpdateDltInfo();

    ///
    /// @brief
    ///     更新耗材信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void UpdateSplInfo();

    ///
    /// @brief
    ///     更新选中试剂详情
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月18日，新建函数
    ///
    void UpdateSelRgntDetail();

    ///
    /// @brief
    ///     更新选中试剂详情
    ///
    /// @param[in]  iSelPos    选中位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月18日，新建函数
    ///
    void UpdateSelRgntDetail(int iSelPos);

    ///
    /// @brief
    ///     更新试剂信息
    ///
    /// @param[in]  srr   试剂信息
    /// @param[in]  type  试剂更新类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月19日，新建函数
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
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
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
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
    ///
    void UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     更新废液桶显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月27日，新建函数
    ///
    void UpdateBucketVisible();

    ///
    /// @brief
    ///     清空试剂信息
    ///
    /// @param[in]  strDevSn		设备序列号
	/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
	/// @param[in]  lGrpId			维护组ID
    /// @param[in]  mit				维护项目类型
    /// @param[in]  enPhase			阶段
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月19日，新建函数
    ///
    void OnClearReagentInfo(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type mit, tf::MaintainResult::type enPhase);

    ///
    /// @brief
    ///     更新按钮状态（如禁能使能，按钮文本等）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月5日，新建函数
    ///
    void UpdateBtnStatus();

    ///
    /// @brief
    ///     试剂扫描对话框确认按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void OnRgntScanDlgOkBtnClicked();

    ///
    /// @brief
    ///     添加反应杯按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void OnAddReactBtnClicked();

    ///
    /// @brief
    ///     反应杯扫描结束
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void OnReactCupScanFinished();

    ///
    /// @brief
    ///     试剂指定位置扫描开始
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  vecPos      指定位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月22日，新建函数
    ///
    void OnRgntScanPosStart(QString strDevName, std::vector<int, std::allocator<int>> vecPos);

    ///
    /// @brief
    ///     耗材被选中
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void OnSplChecked();

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
    ///     设备状态改变
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月14日，新建函数
    ///
    void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     设备温度异常状态改变
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  bErr        是否温度异常
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月30日，新建函数
    ///
    void OnDevTemperatureErrChanged(QString strDevSn, bool bErr);

    ///
    /// @brief
    ///     更新耗材使能状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月12日，新建函数
    ///
    void UpdateSplEnabled();

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
    /// @brief	设备信息更新
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月5日，新建函数
    /// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
    ///
    void UpdateDeviceStatus(tf::DeviceInfo deviceInfo);

private:
    void GetRelatedReagentIndexs(const im::tf::ReagentInfoTable& reag, QSet<int> &relatedIndex);

    // 获取试剂状态，用于试剂盘颜色
    ImReagentState GetImReagentState(const im::tf::ReagentInfoTable& reag);

    ///
    /// @brief
    ///     获取试剂状态字符串
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    ///
    /// @return 试剂状态字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月27日，新建函数
    ///
    QString GetReagentStateStr(const im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     获取试剂继承其配对试剂的状态
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月5日，新建函数
    ///
    ImReagentState GetInheritMantchRgntState(const im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     获取稀释液状态
    ///
    /// @param[in]  stuDlt  稀释液信息
    ///
    /// @return 稀释液状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月27日，新建函数
    ///
    ImReagentState GetImDltState(const im::tf::DiluentInfoTable& stuDlt);

    ///
    /// @brief
    ///     获取稀释液状态字符串
    ///
    /// @param[in]  stuDltInfo  稀释液信息
    ///
    /// @return 稀释液状态字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月27日，新建函数
    ///
    QString GetDltStateStr(const im::tf::DiluentInfoTable& stuDltInfo);

    ///
    /// @brief
    ///     获取耗材状态
    ///
    /// @param[in]  stuDlt  耗材信息
    ///
    /// @return 耗材状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月21日，新建函数
    ///
    SuppliesStatus GetImSplState(const im::tf::SuppliesInfoTable& stuSpl);

    // 赋值界面试剂结构信息
    void AssignImReagentInfo(const im::tf::ReagentInfoTable& reag, ImReagentInfo &stuInfo);

    ///
    /// @brief
    ///     更新关联试剂状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月9日，新建函数
    ///
    void UpdateRelativeRgntState();

	///
	/// @brief 试剂加载机构试剂槽是否存在试剂盒
	///
	/// @param[out]  bExist  true-存在，false-不存在
	///
	/// @return true-获取成功，false-获取失败
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月4日，新建函数
	///
	bool ReagentLoadExistReag(bool& bExist);
    void AssayLabelWithTip(QLabel* qLable, const QString& strText);

private:
    Ui::ImRgntPlateWidget                           *ui;                // ui对象指针
    bool                                             m_bInit;           // 是否初始化

    ImMcReagentPlate*                                m_pRgntPlate;      // 试剂盘
    QMap<QPair<SuppliesType, SuppliesStatus>, int>   m_mapBottleType;   // 储液瓶类型映射

    // 业务逻辑相关
    QString                                          m_strDevName;      // 设备名
    QString                                          m_strDevSn;        // 设备序列号

    // 耗材选择器映射表
    QMap<CheckableFrame*, im::tf::SuppliesInfoTable> m_mapSplChecker;   // 耗材选择器映射表
    QReagentScanDialog*                              m_scanDialog;      // 试剂扫描对话框

    // 耗材使能状态（是否停用）
    SuppliesEnableInfo                               m_stuSplEnableInfo;// 耗材使能信息
};

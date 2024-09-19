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
/// @file     ReagentChemistryWidgetPlate.h
/// @brief    生化试剂盘头文件
///
/// @author   5774/WuHongTao
/// @date     2021年8月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年8月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include <QWidget>
#include <boost/optional.hpp>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/public/ConfigDefine.h"
#include "QReagentItem.h"

class QLabel;
class QRadioButton;
class QProgressBarShell;
class QRegisterSupplyInfo;
class QReagentScanDialog;
struct ChReagentInfo;
class toolTipLabel;
class ChReagentUnLoadDlg;

namespace Ui {
	class ReagentChemistryWidgetPlate; 
};

class ReagentChemistryWidgetPlate : public QWidget
{
    Q_OBJECT

public:

    ReagentChemistryWidgetPlate(QWidget *parent = Q_NULLPTR);
    ~ReagentChemistryWidgetPlate();

    ///
    /// @brief 更新设备组信息
    ///
    /// @param[in]  deviceGroups    设备组  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月9日，新建函数
    ///
    void UpdateGroupDevice(std::vector<std::shared_ptr<const tf::DeviceInfo>>& deviceGroups);

protected:
    ///
    /// @brief 页面显示时，处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:
    ///
    /// @brief 界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月5日，新建函数
    ///
    void InitBeforeShow();

	///
	/// @brief     更新试剂盘
	///
	/// @param[in]  spModule  设备模块信息
	/// @param[in]  setPos	  更新的位置，空表示全部更新
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void UpdateReagentPlate(std::shared_ptr<const tf::DeviceInfo>& spModule, const std::set<int> &setPos = {});

	///
	/// @brief   更新仓外耗材的信息
	///
	/// @param[in]  spModule  设备模块信息
	/// @param[in]  setPos	  更新的位置，空表示全部更新
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void UpdateCabinetSupplies(std::shared_ptr<const tf::DeviceInfo>& spModule, const std::set<int> &setPos = {});

	///
	/// @brief  更新指定的仓外耗材
	///
	/// @param[in]  si  仓外耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void UpdateCabinetSupply(const ch::tf::SuppliesInfo& si);

	///
	/// @brief   更新试剂加载位信息
	///
	/// @param[in]  spModule  设备信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月24日
	///
	void UpdateReagentLoaderStatus(std::shared_ptr<const tf::DeviceInfo>& spModule);

    ///
    /// @brief     更新试剂盘选中槽位信息
    ///
    /// @param[in]  iIndex  试剂盘中选中槽位的索引
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月1日，新建函数
    ///
    void UpdateSelSlotInfo(int iIndex);

	///
	/// @brief  获取试剂显示名称
	///
	/// @param[in]  assayCode  
	/// @param[in]  supplyCode  
	/// @param[in]  supplyName  
	///
	/// @return 试剂显示名称
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月29日，新建函数
	///
	QString GetReagentName(const int assayCode, const int supplyCode, const std::string& supplyName);

    ///
    /// @brief    显示试剂相信信息
    ///
    /// @param[in]  reagentGroup  试剂详细信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月25日，新建函数
    ///
    void ShowReagentGroupDetail(const std::vector<ch::tf::ReagentGroup>& reagentGroups, ch::tf::SuppliesInfo& supply);

    ///
    /// @brief    显示耗材（非试剂的）详细信息
    ///
    /// @param[in]  suppliesInfo  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月21日，新建函数
    ///
    void ShowSuppliesDetail(const ch::tf::SuppliesInfo& suppliesInfo);

	///
	/// @brief    此试剂是否是耗材对应的
	///
	/// @param[in]  supply   耗材信息
	/// @param[in]  reagent  试剂信息
	///
	/// @return true表示对应
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年1月25日，新建函数
	///
	bool IsRightReagentOfSupply(const ch::tf::SuppliesInfo& supply, const ch::tf::ReagentGroup& reagent);

    ///
    /// @brief  清除详细信息为空
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    void ClearReagentStatus();

    ///
    /// @brief   设置试剂的状态
    ///
    /// @param[in]  supplyInfo 耗材信息
	/// @param[in]  rgtGrops   试剂组信息
    /// @param[iout]  stuReagentInfo  用于绘制的信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    void SetReagentStatus(const ch::tf::SuppliesInfo& supplyInfo, const std::vector<ch::tf::ReagentGroup>& rgtGrops, QReagentItem::ReagentInfo& stuReagentInfo);

    ///
    /// @brief   禁止所有的功能键使能
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月20日，新建函数
    ///
    void DisableFunctionBtn();

    ///
    /// @brief  获取耗材最大值
    ///
    /// @param[in]  supplyInfo  耗材信息
    ///
    /// @return 最大信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月18日，新建函数
    ///
    int GetSupplyMax(const ch::tf::SuppliesInfo& supplyInfo);

    ///
    /// @brief   显示制定位置的备用编号
    ///
    /// @param[in]  pos  指定位置
    /// @param[in]  IsShow  是否显示
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月25日，新建函数
    ///
    bool ShowPostionSpareNumber(int pos, bool IsShow);

	///
	/// @brief 优先使用试剂
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月20日，新建函数
	///
	void PriorityReagent();

	///
	/// @brief 优先使用耗材
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月20日，新建函数
	///
	void PrioritySupply();

	///
	/// @brief  清除试剂盘数据
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月22日，新建函数
	///
	void ClearReagentDiskDatas();

	///
	/// @brief  判断试剂item状态
	///
	/// @param[in]  reGroupList  试剂组列表
	///
	/// @return 必须包含6个值: 校准失败、项目报警、瓶报警、屏蔽、项目屏蔽、液位探测失败对应的状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	std::vector<bool> JudgeItemStatus(const std::vector<ch::tf::ReagentGroup>& reGroupList);

	///
	/// @brief  获取加载器状态字符串
	///
	/// @param[in]  status  加载器状态
	/// @param[in]  isReagent  true表示是试剂 false表示耗材
	///
	/// @return 加载器状态字符串
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月2日，新建函数
	///
	QString GetLoaderStatus(const int status, const bool isReagent);

	///
	/// @brief  获取名称和余量
	///
	/// @param[in]  loaderItem  加载器信息
	///
	/// @return （名称，余量）
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月2日，新建函数
	///
	std::pair<QString, QString> GetLoaderNameRemain(const ch::tf::ReagentLoaderItem &loaderItem);

protected Q_SLOTS:

    ///
    /// @brief   试剂盘选中槽位改变
    ///
    /// @param[in]  iIndex  槽位索引
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月1日，新建函数
    ///
    void OnSelectedReagentSlotChanged(int iIndex);

    ///
    /// @brief   刷新试剂盘
    ///
    /// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
    ///
    void OnRefreshReagentPlate();

	///
	/// @brief   刷新仓外耗材
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void OnRefreshCabinetSupply();

	///
	/// @brief   设备OtherInfo信息更新
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void OnDeviceOtherInfoUpdate(const QString &devSn);

    ///
    /// @brief  登记按钮的槽函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年10月19日，新建函数
    ///
    void onRegisterClicked();

    ///
    /// @brief  试剂屏蔽
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnReagentShield();

    ///
    /// @brief  优先使用
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年11月20日，新建函数
    ///
    void OnPriority();

    ///
    /// @brief  试剂卸载
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnReagentUnload();

    ///
    /// @brief   试剂扫描
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnScanReagent();

    ///
    /// @brief    生化试剂&耗材更新
    ///
    /// @param[in]  supplyUpdates  更新的信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月19日，新建函数
    ///
    void OnUpdateSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief 设备状态更新
	///
	/// @param[in]  deviceInfo  设备信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月11日，新建函数
	///
	void OnDeviceStatusUpdate(class tf::DeviceInfo deviceInfo);

    ///
    /// @brief 更新加载器状态
    ///
    /// @param[in]  deviceSN      设备序列号
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年11月24日，新建函数
    ///
    void OnLoaderStatusUpdate(QString deviceSN);

    ///
    /// @brief 耗材关注取消
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void OnFocusOut();

    ///
    /// @brief 选中耗材
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void OnSelectSupply();

    ///
    /// @brief 显示废液桶
    ///
    /// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
    ///
    void OnShowBucket();

    ///
    /// @brief  更新废液桶
    ///
    /// @param[in]  devSN  设备序列号
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2024年3月19日，新建函数
    ///
    void OnUpdateBucketStatus(const QString& devSN);

private:
    Ui::ReagentChemistryWidgetPlate*         ui;                ///< ui对象指针
    QRegisterSupplyInfo*            m_registerDialog;           ///< 登记对话框
    QReagentView*                   m_pReagentPlate;            ///< 试剂盘对象
    std::vector< std::pair < QRadioButton*, std::shared_ptr<const tf::DeviceInfo>> > m_subDeviceVec;
    std::shared_ptr<const tf::DeviceInfo> m_currentDevModule;   ///< 当前选中模块
    int                             m_currentPostion;           ///< 当前试剂盘选择位置
	bool							m_focusState;				///< 当前焦点位于仓内还是仓外
	std::shared_ptr<ChReagentInfo>	m_currSelectInfo;			///< 当前选中的仓内试剂&耗材信息
    std::set<int>					m_backUpShowPos;            ///< 备用试剂的位置显示列表
    bool                            m_IsReagent;                ///< true代表是试剂，反之是耗材
    std::vector<QLabel*>            m_ReagentLoader;            ///< 试剂加载位
    std::vector<toolTipLabel*>      m_ReagentTips;              ///< 试剂加载位提示信息
	QReagentScanDialog*				m_scanDialog;				///< 试剂扫描对话框
    QProgressBarShell*              m_pProgressBar;             ///< 当前选中的耗材句柄
    bool                            m_needUpdate;               ///< 需要更新与否
	ChReagentUnLoadDlg*				m_unloadDlg;				///< 试剂卸载对话框

	// 反应杯清洗液相关控件
	struct WashLiquidWgt
	{
		QProgressBarShell* progressBar = nullptr;
		QLabel*			   tips = nullptr;
		QLabel*			   name = nullptr;

		WashLiquidWgt() {}
		WashLiquidWgt(QProgressBarShell* p, QLabel* t, QLabel* n)
		{
			progressBar = p;
			tips = t;
			name = n;
		}
	};

	std::map<int, WashLiquidWgt>	m_cupWashLiquidMap;			///< 反应杯清洗液列表
};

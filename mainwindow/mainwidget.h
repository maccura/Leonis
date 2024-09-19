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
/// @file     mainwidget.h
/// @brief    主窗口界面
///
/// @author   4170/TangChuXian
/// @date     2020年4月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/gen-cpp/defs_types.h"
#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QTimer;
class QAlarmDialog;
class MenuWidget;
class AnalysisDlg;
class McProgressDlg;
class UtilityWidget;
class SystemOverviewWgt;

struct BaseSet;
namespace Ui { class MainWidget; };


class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(std::function<void()> cbExit, QWidget *parent = Q_NULLPTR);
    ~MainWidget();

	///
	/// @brief  自动弹出系统总览页面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void showSystemOverviewWidget();

    ///
    /// @brief  更新打印机状态
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月22日，新建函数
    ///
    void UpdatePrintDevStatus(bool isConnected);

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 初始化成员变量
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void InitMemberVar();

	///
	/// @brief 更新日期时间格式字符串
	///
	/// @param[in]  ds  格式信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月15日，新建函数
	///
	void UpdateDateTimeFormat(const BaseSet& ds);

    ///
    /// @brief 初始化堆栈窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void InitStackWidget();

    ///
    /// @brief 初始化设备状态标签
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月6日，新建函数
    ///
    void InitDeviceStateLabel();

    ///
    /// @brief 初始化单机版设备状态标签
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月6日，新建函数
    ///
    void InitSingalDeviceStateLabel(std::map<std::string, std::shared_ptr<const tf::DeviceInfo>>& devMap);

    ///
    /// @brief 初始化联机版设备状态标签
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月6日，新建函数
    ///
    void InitPipeDeviceStateLabel(std::map<int, std::shared_ptr<const tf::DeviceInfo>>& devMap);

    ///
    /// @brief 初始化信号槽连接
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief 界面显示之前注册消息处理槽函数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月29日，新建函数
    ///
    void RegsterMsgHandlerBeforeShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief  重写鼠标按下事件
	///
	/// @param[in]  event  鼠标事件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月11日，新建函数
	///
	void mousePressEvent(QMouseEvent *event) override;

    ///
    /// @brief  更新暂停和停机按钮的状态
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月2日，新建函数
    ///
    void UpdateSampleStopAndStopBtn();

	///
	/// @brief  关闭顶层窗口
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月1日，新建函数
	///
	void CloseTopLevelWindow();

    ///
    /// @brief  设备复位后的报警等级改变的相应函数
    ///
    ///
    /// @param[in]  devSn  设备序列号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月31日，新建函数
    ///
    void AlarmLevelChangedAfterDevReset(QString devSn);

signals:
    void PrintStatusChanged(bool isConnected);

protected Q_SLOTS:
    ///
    /// @brief 切换tab页
    ///     
    /// @param[in]  iTabIndex  tab页索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月29日，新建函数
    ///
    void OnSwitchTab(int iTabIndex);

	///
	/// @brief 退出回调函数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月5日，新建函数
	///
	void OnExitBtnClicked();

    ///
    /// @brief 分析按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void OnAnalysisBtnClicked();

	///
	/// @brief 自动进架按钮按下消息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年2月6日，新建函数
	///
	void OnAnalysisBtnClicked_continueRun();

    ///
    /// @brief 加样停按钮被点击
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void OnStopSamplingBtnClicked();

    ///
    /// @brief 急停按钮被点击
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void OnStopBtnClicked();

    ///
    /// @brief 告警按钮被点击
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月27日，新建函数
    ///
    void OnAlarmBtnClicked();

    ///
    /// @brief 定时更新当前时间
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void OnUpdateCurTime();

    ///
    /// @brief 当前菜单位置发生改变
    ///     
    /// @param[in]  strCurMenuLocate  当前菜单位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void OnCurMenuLocChange(QString strCurMenuLocate);

    ///
    /// @brief
    ///     急诊对话框项目选择按钮被点击
    ///
    /// @param[in]  iBtnIndex 被点击按钮位置索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月22日，新建函数
    ///
    void OnStatDlgAssayBtnClicked(int iBtnIndex);

    ///
    /// @brief
    ///     急诊对话框样本量下拉框被点击
    ///
    /// @param[in]  iIndex  下拉框当前选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月22日，新建函数
    ///
    void OnStatDlgSampSizeComboChanged(int iIndex);

    ///
    /// @brief
    ///     当P300急诊菜单项触发
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月22日，新建函数
    ///
    void OnP300StatMenuItemActive();

    ///
    /// @brief
    ///     p300急诊对话框确认按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月23日，新建函数
    ///
    void OnP300StatOkBtnClicked();

    ///
    /// @brief
    ///     当U2000急诊菜单项触发
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月22日，新建函数
    ///
    void OnU2000StatMenuItemActive();

    ///
    /// @brief
    ///     更新设备状态
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月23日，新建函数
    ///
    void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     更新设备当前状态的倒计时
    ///
    /// @param[in]  totalSeconds  总的倒计时时间
    /// @param[in]  sn2seconds	其他设备的倒计时时间
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月25日，新建函数
    ///
    void OnDevStateTimeChange(int totalSeconds, const QMap<QString, int> &sn2seconds);

	///
	/// @brief	更新时间显示格式
	///
	/// @param[in]  ds  显示设置信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月29日，新建函数
	///
	void OnDateTimeFormatUpDate(const BaseSet& ds);

    ///
    /// @brief
    ///     打开进度条对话框
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月27日，新建函数
    ///
    void OpenProgressDlg(QString strTitle, bool bIndicator = false);

    ///
    /// @brief
    ///     关闭进度条对话框
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月27日，新建函数
    ///
    void CloseProgressDlg(bool bfalg = true);

    ///
    /// @brief
    ///     更新进度条
    ///
    /// @param[in]  ProgressInfo  进度信息
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月27日，新建函数
    ///
    void UpdateProgressDlg(int iPro);

    ///
    /// @brief  新报警相应操函数
    ///
    /// @param[in]  alarmDesc  报警信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月10日，新建函数
    ///
    void OnAlarmLevelChanged(int level);

    ///
    /// @brief  更新设备屏蔽状态
    ///
    /// @param[in]  groupName	设备组母
    /// @param[in]  devSn		设备序列号
    /// @param[in]  masked		true表示屏蔽
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月3日，新建函数
    ///
    void OnUpadteDeviceMaskStatus(const QString& groupName, const QString& devSn, bool masked);

    ///
    /// @brief  更新开始测试按钮的状态
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月10日，新建函数
    ///
    void OnUpdateAnalysisBtnStatus();

    ///
    /// @brief  更新Lis的展示状态
    ///
    /// @param[in]  status 是否为连接状态
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月30日，新建函数
    ///
    void OnUpdateLisStatus(const bool status);

	///
	/// @brief  显示数据浏览界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月13日，新建函数
	///
	void OnShowDataBrowsePage();

	///
	/// @brief  显示样本架监视界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月11日，新建函数
	///
	void OnShowSampleRackMonitor();

	///
	/// @brief 登录用户切换
	///     
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
	///
	void UpdateLoginUser();

	///
	/// @brief  显示试剂盘界面
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月10日，新建函数
	///
	void OnShowShowReagentPage(const QString &devSn);

	///
	/// @brief  显示维护界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月30日，新建函数
	///
	void OnShowMaintainPage();

    ///
    /// @brief  显示错误提示
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年7月24日，新建函数
    ///
    void OnShowErrText(QString text);

private:
    Ui::MainWidget*             ui;                             // ui对象指针

    bool                        m_bIsInit;                      // 是否已经初始化
    QTimer*                     m_pGetCurdateTimeTimer;         // 实时刷新当前日期时间
    QVector<QPushButton*>       m_vTabBtns;                     // tab按钮数组
	MenuWidget*                 m_menuWidget;
    UtilityWidget*              m_pUtilityWidget;               // 应用页面
	QAlarmDialog*			    m_pAlarmDialog;				    // 告警对话框指针
	AnalysisDlg*			    m_pAnalysisDlg;				    // 开始测试弹窗指针
	QMovie *				    m_pStopMovie;					// 告警图标停机闪烁(红色)
    QMovie *				    m_pSampleStopMovie;				// 告警图标暂停闪烁(黄色)
    QMovie *				    m_pAttentionMovie;				// 告警图标注意闪烁(蓝色)
	QString					    m_strDateTimeFormat;		    // 时间显示格式
    std::function<void()>       m_cbExit;                       // 退出前需要执行的函数

    std::shared_ptr<McProgressDlg> m_pProgressDlg;              // 进度条对话框
	SystemOverviewWgt*			m_pSystemOverviewWgt;			// 系统总览弹出框
};

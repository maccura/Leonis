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
/// @file     utilityWidget.h
/// @brief    应用界面
///
/// @author   7951/LuoXin
/// @date     2022年12月02日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月02日，新建函数
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <set>
#include "QWidget"

class ConsumablesManageDlg;
class ClearSetDlg;
class DetectionSetDlg;
class SoftwareVersionDlg;
class DevManageDlg;
class CommParamSetDlg;
class SampleRackAllocationDlg;
class PromptSetWidget;
class QDisplayShow;
class QAssayShowPostion;
class SampleReceiveModeDlg;
class PrintSetWidget;
class functionManageDlg;
class AssayAllocationDlg;
class UserManualWidget;
class NodeDbgDlg;
class ProcessCheckDlg;

namespace Ui {
    class UtilityWidget;
};

class UtilityWidget : public QWidget
{
    Q_OBJECT

public:
    UtilityWidget(QWidget *parent = Q_NULLPTR);
    ~UtilityWidget();

    ///
    /// @brief 应用页面切换为主界面
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月15日，新建函数
    ///
    void SetCurrentWidgetIsMain();

    ///
    /// @brief 当前页面是否存在未保存的数据
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年5月9日，新建函数
    ///
    bool CurrentWidgetHasUnSaveData();

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月02日，新建函数
	///
	void Init();

protected Q_SLOTS:

    void OnAssayOrder();

    ///
    /// @brief
    ///     用户管理按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
    ///
    void OnUserMngBtnClicked();

	///
	/// @brief	通讯设置被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月05日，新建函数
	///
	void OnCommParamSetClicked();

	///
	/// @brief	自动开机设置被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月05日，新建函数
	///
	void OnStartupSetClicked();

    ///
    /// @brief 显示设置的被点击
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void OnDisplaySetClicked();

	///
	/// @brief	样本架分配被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月05日，新建函数
	///
	void OnSampleRackClicked();

	///
	/// @brief	设备管理被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月05日，新建函数
	///
	void OnDevManageClicked();

	///
	/// @brief	软件版本被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月05日，新建函数
	///
	void OnSoftwareVersionClicked();

	///
	/// @brief	进样模式被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月08日，新建函数
	///
	void OnInjectionModelClicked();

	///
	/// @brief	清空设置被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月20日，新建函数
	///
	void OnClearSetClicked();

    ///
    /// @brief  耗材管理被点击
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月2日，新建函数
    ///
    void OnConsumablesManageClicked();

	///
	/// @brief  样本接收模式被点击
	///
	/// @param[in]    void
	///
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月22日，新建函数
	///
	void OnSampleReceiveModeClicked();

	///
	/// @brief  帮助文档被点击
	///
	/// @param[in]    void
	///
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月10日，新建函数
	///
	void OnHelpFileClicked();

    ///
    /// @brief  打印设置被点击
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月20日，新建函数
    ///
    void OnPrintSetClicked();

	///
	/// @brief  导出设置被点击
	///
	/// @param[in]    void
	///
	/// @return void
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月12日，新建函数
	///
	void OnExportSetClicked();

    ///
    /// @brief  功能管理弹窗
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void OnFunctionManageClicked();

    ///
    /// @brief  项目分配弹窗
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void OnAssayAllocationClicked();

    ///
    /// @bref
    ///		项目设置按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnAssaySettings();

    ///
    /// @brief
    ///     节点调试
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void OnNodeDebugBtnClicked();

    ///
    /// @brief
    ///     流程检查
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void OnProcessCheckBtnClicked();

    ///
    /// @brief
    ///     诊断按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void OnDiagnoseBtnCliked();

    ///
    /// @brief
    ///     传感器状态按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void OnSensorStateBtnCliked();

    ///
    /// @brief
    ///     参数按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void OnParamBtnClicked();

    ///
    /// @bref
    ///		项目位置按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnAssayPositionSettings();

    ///
    /// @bref
    ///		备用订单
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnStandbyOrder();

    ///
    /// @bref
    ///		交叉污染按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnSpecialWash();

    ///
    /// @bref
    ///		日志按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnLog();

    ///
    /// @bref
    ///		状态按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnStatus();

    ///
    /// @bref
    ///		显示按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnDisplay();

    ///
    /// @bref
    ///		提示按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnPrompt();

    ///
    /// @bref
    ///		计算项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnCalculateAssay();

    ///
    /// @bref
    ///		统计分析
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnStatistic();

    ///
    /// @brief
    ///     AI识别设置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月30日，新建函数
    ///
    void OnAiAnalysisCfgClicked();

    ///
    /// @bref
    ///		权限变更响应函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnPermisionChanged();

public Q_SLOTS:

	///
	/// @bref
	///		维护保养
	///
	/// @par History:
	/// @li 8276/huchunli, 2023年7月22日，新建函数
	///
	void OnMaintain();

private:

    typedef void (UtilityWidget::*MemberFunc)();
    struct ButtonInfo
    {
        QString m_showName;
        QString m_objectName;
        QString m_iconName;
        MemberFunc m_memberSlot;

        ButtonInfo(const QString& showName, const QString& objName, const QString& iconName, MemberFunc memberFunc)
        {
            m_showName = showName;
            m_objectName = objName;
            m_iconName = iconName;
            m_memberSlot = memberFunc;
        }
    };

    ///
    /// @bref
    ///		往一个Widget框中添加按钮列表
    ///
    /// @param[in] pRootWidget 目标框
    /// @param[in] title 框标题
    /// @param[in] rowCnt 框的行限制
    /// @param[in] colCnt 框的列限制
    /// @param[in] btnInfo 按钮信息列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void InitButtonArea(QWidget* pRootWidget, const QString& title, int rowCnt, int colCnt, const std::vector<ButtonInfo>& btnInfo);

    ///
    /// @bref
    ///		隐藏一个Widget中的按钮，后续按钮往前移动
    ///
    /// @param[in] pRootWidget 目标框
    /// @param[in] btnObjName 按钮类名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void SetButtonHide(QWidget* pRootWidget, const QString& btnObjName);

    ///
    /// @bref
    ///		显示一个Widget中的按钮，后续按钮往后移动
    ///
    /// @param[in] pRootWidget 目标框
    /// @param[in] btnObjName 按钮类名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void SetButtonShow(QWidget* pRootWidget, const QString& btnObjName);

    ///
    /// @bref
    ///		设置按钮的显示与隐藏状态，并调整按钮的布局中的位置
    ///
    /// @param[in] pRootWidget 目标框
    /// @param[in] btnObjName 目标按钮对象名字
    /// @param[in] isShow 显示或隐藏
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void SetButtonVisiable(QWidget* pRootWidget, const QString& btnObjName, bool isShow);

	///
	/// @brief 初始化应用模块在主界面左下角导航菜单过滤
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月19日，新建函数
	///
	void InitFilterMenuLocLab();

private:
	Ui::UtilityWidget*				ui;
    QAssayShowPostion*              m_assayOrder;                   // 项目顺序
    QDisplayShow*                   m_pDisplayShowDlg;              // 显示设置对话框
	DevManageDlg*					m_pDevManageDlg;				// 设备管理弹窗
	SampleRackAllocationDlg*		m_pSampleRackAllocationDlg;		// 样本架分配
	CommParamSetDlg*				m_pCommParamSetDlg;				// 通讯设置弹窗
	SoftwareVersionDlg*				m_pSoftwareVersionDlg;			// 普通权限软件版本弹窗
	DetectionSetDlg*				m_pDetectionSetDlg;			    // 检测设置弹窗
	ClearSetDlg*					m_pClearSetDlg;					// 清空设置弹窗
    ConsumablesManageDlg*           m_pConsumablesManageDlg;        // 耗材管理弹窗
	SampleReceiveModeDlg*			m_pSampleReceiveModeDlg;		// 样本接收模式弹窗
    functionManageDlg*              m_pfunctionManageDlg;           // 功能屏蔽弹窗
    AssayAllocationDlg*             m_pAssayAllocationDlg;          // 项目分配弹窗
	std::set<QString>				m_setFilterMenuLocLab;			// 主界面左下角导航菜单过滤 add by Chenjianlin 20240319
	NodeDbgDlg*						m_pNodeDbgDlg;					// 节点调试弹窗
    ProcessCheckDlg*                m_pProcessCheckDlg;             // 流程检查对话框
};

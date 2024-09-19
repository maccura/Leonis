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
/// @file     analysisdlg.h
/// @brief    主界面->开始分析对话框
///
/// @author   4170/TangChuXian
/// @date     2020年8月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include "shared/basedlg.h"

#define         ASSAY_TEST_ENABLE       ("canStart")        // 能否开始测试的动态属性名称

namespace Ui { class AnalysisDlg; };

class CommParamSet;
class AssayShiledDlg;

class AnalysisDlg : public BaseDlg
{
    Q_OBJECT

public:
    AnalysisDlg(QWidget *parent = Q_NULLPTR);
    ~AnalysisDlg();

    ///
    /// @brief 获取不能开始测试的原因
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月10日，新建函数
    ///
    QString GeterrText() { return m_errText; };

    ///
    /// @brief 更新启动测试按钮状态 
    ///
    /// @return true:可以开始测试
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月10日，新建函数
    ///
    bool UpdateOkBtnStatus();

	///
	/// @brief 执行自动进架消息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年2月6日，新建函数
	///
	void ContinueRun();
protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月17日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月17日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief 设置序号模式下默认的开始样本号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月20日，新建函数
    ///
    void SetSeqDefaultNumber();

	///
	/// @brief  检测所有免疫设备是否有试剂仓盖打开的情况
	///
	/// @param[in]    
	///
	/// @return bool
	///
	/// @par History:
	/// @li 7915/LeiDingXiang，2023年1月3日，新建函数
	///
	bool QueryImReagentCoverIsClose();

	///
	/// @brief  检测i 6000设备是否有废液桶溢出情况
	///
	/// @param[in]    
	///
	/// @return 
	///
	/// @par History:
	/// @li 7915/LeiDingXiang，2023年1月12日，新建函数
	///
	bool QueryI6000WasteLiquidFull();

    ///
    /// @brief  更新维护组下拉框
    ///     
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月19日，新建函数
    ///
    void UpdateMaintainGroupCommbox();

    ///
    /// @brief	主机通信勾选状态改变
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年2月23日，新建函数
    ///
    bool SaveCommParamConfig();

    ///
    /// @brief	检查当前模式下的试剂可以次数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年4月8日，新建函数
    ///
    bool CheckReagentRemainCount();

    ///
    /// @brief	自动复查勾选状态改变
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年2月23日，新建函数
    ///
    bool SaveAutoRecheckConfig();

    ///
    /// @brief	保存维护配置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年10月27日，新建函数
    ///
    bool SaveMaintainConfig();

protected Q_SLOTS:
	///
	/// @brief	项目遮蔽按钮被点击
	///     
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月2日，新建函数
	///
	void OnAssayShieldClicked();

	///
	/// @brief	开始测试按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年2月23日，新建函数
	///
	void OnStartBtnClicked();

    ///
    /// @brief	关闭按钮槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年10月27日，新建函数
    ///
    void OnCloseBtnClicked();

	///
	/// @brief	项目屏蔽改变
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年2月24日，新建函数
	///
	void OnAssayShiledChanged();

    ///
    /// @brief  主机通讯按钮限制
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年11月7日，新建函数
    ///
    void OnCommunicationChanged();

private:
    Ui::AnalysisDlg			*ui;                      // UI指针
	AssayShiledDlg			*m_pAssayShiledDlg;		  // 项目遮蔽弹窗
    bool					m_bInit;                  // 是否已经初始化
	bool					m_bIsPipeLine;			  // 当前是否为流水线模式
    QString                 m_errText;                // 不能开始的测试的错误信息
    QString                 m_lastSeqNo;              // 上一次开始的样本号
};

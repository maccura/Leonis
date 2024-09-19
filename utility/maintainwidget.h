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
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "src/thrift/gen-cpp/DcsControl.h"
#include "src/thrift/im/i6000/gen-cpp/i6000UiControl.h"
#include <QWidget>
namespace Ui { class MaintainWidget; };
class  QDetectModuleDebugDlg;
class QAutoMaintainCfgDlg;

class MaintainWidget : public QWidget
{
    Q_OBJECT

public:
    MaintainWidget(QWidget *parent = Q_NULLPTR);
    ~MaintainWidget();

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitChildCtrl();

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
    ///     根据设备类型获取设备类别
    ///
    /// @param[in]  strMaintainGrpName  维护组名称
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月19日，新建函数
    ///
    QSet<tf::AssayClassify::type> GetDevClassify(const QString& strMaintainGrpName);

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

protected Q_SLOTS:
    ///
    /// @brief
    ///     维护组tab按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void OnGrpMaintainBtnClicked();

    ///
    /// @brief
    ///     维护单项tab按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void OnItemMaintainBtnClicked();

    ///
    /// @brief
    ///     维护状态按钮被点击
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void OnMaintainStatusBtnClicked(QString strMaintainName);

    ///
    /// @brief
    ///     设备控件被点击
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void OnDevWgtClicked(QString strDevSn);

    ///
    /// @brief
    ///     组合设置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月21日，新建函数
    ///
    void OnProfileCfgBtnClicked();

    ///
    /// @brief
    ///     维护日志按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月21日，新建函数
    ///
    void OnMaintainLogBtnClicked();

    ///
    /// @brief
    ///     自动维护设置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月21日，新建函数
    ///
    void OnAutoMaintainCfgBtnClicked();

    ///
    /// @brief
    ///     停止维护按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void OnStopMaintainBtnClicked();

    ///
    /// @brief
    ///     显示信息设置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void OnDisplayInfoCfgClicked();
	
	///
	/// @brief 电极清洗设置
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void OnElectrodeCleanSetBtnClicked();

	///
	/// @brief
	///     发光剂流程按钮被点击
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年2月27日，新建函数
	///
	void OnDetectDebugBtnClicked();

    ///
    /// @brief
    ///     更新维护组按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void UpdateMaintainGrpBtn();

    ///
    /// @brief
    ///     更新设备维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月24日，新建函数
    ///
    void UpdateDevMaintainStatus();

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
	/// @bref
	///		权限变化响应
	///
	/// @par History:
	/// @li 8276/huchunli, 2023年7月6日，新建函数
	///
    void OnPermisionChanged();

	///
	/// @brief  显示仪器复位弹窗
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月10日，新建函数
	///
	void OnShowInstrumentReset();

private:
    Ui::MaintainWidget*             ui;                     // UI对象指针
    bool                            m_bInit;                // 是否已经初始化

    QStringList                     m_strDevNameList;       // 当前设备名列表
    std::shared_ptr<QAutoMaintainCfgDlg> m_spMaintainLogDlg;
	std::shared_ptr<QDetectModuleDebugDlg> m_spDetectModuleDebugDlg;  // 发光剂流程对话框（因为界面关闭后再打开，需要保留关闭前的状态，固需要保存） 
};

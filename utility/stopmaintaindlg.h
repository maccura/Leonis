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
/// @file     stopmaintaindlg.h
/// @brief    停止维护对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <QList>
#include <QMap>
#include "src/thrift/gen-cpp/defs_types.h"
#include "shared/basedlg.h"
namespace Ui { class StopMaintainDlg; };

class QPushButton;


class StopMaintainDlg : public BaseDlg
{
    Q_OBJECT

	// 参数设置控件的页码
	enum ParamPageNum
	{
		page_Normal = 0,				///< 常规页
		page_ChPipeFill,				///< 比色管路填充
		page_ProbeClean,				///< 针清洗
		page_CuvetteReplace,			///< 比色杯更换
		page_CuvetteBlank,				///< 杯空白测定
		page_PhotometerCheck,			///< 光度计检查
		page_IseDrainage,				///< ISE排液
		page_IsePipeFill				///< ISE管路填充

	};

public:
    StopMaintainDlg(QWidget *parent = Q_NULLPTR);
    ~StopMaintainDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月14日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     停止按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void OnStopBtnClicked();

    ///
    /// @brief
    ///     更新维护组按钮显示状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void UpdateMtGrpBtnStatus();

    ///
    /// @brief
    ///     设备按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void OnDevBtnClicked();

    ///
    /// @brief
    ///     维护组阶段更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enPhaseType     阶段类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType);

    ///
    /// @brief
    ///     更新设备维护组状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void UpdateDevMaintainGrpStatus();

private:

	///
	/// @brief 获取被选中的设备信息
	///		如果同一个sn的多个模块同时被选中，说明是整机，则modelIndex为0
	///
	/// @param[in]  checkedDev  被选中的设备信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月9日，新建函数
	///
	void TakeCheckedDevkeyInfo(std::vector<::tf::DevicekeyInfo>& checkedDev);

    ///
    /// @brief
    ///     选中正在维护按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月25日，新建函数
    ///
    void SelectMaintainingBtn();

private:
    Ui::StopMaintainDlg                        *ui;                         // ui指针
    bool                                        m_bInit;                    // 是否已经初始化

    QStringList                                 m_strDevNameList;           // 设备名列表
    QList<QPushButton*>                         m_pDevBtnList;              // 设备按钮列表
    QMap<QPushButton*, ::tf::DevicekeyInfo>     m_mapBtnDevInfo;            // 按钮设备信息映射
};

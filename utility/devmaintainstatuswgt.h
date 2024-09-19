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
/// @file     devmaintainstatuswgt.h
/// @brief    设备维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年2月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
namespace Ui { class DevMaintainStatusWgt; };

// 前置声明
class QMovie;   // 动画

class DevMaintainStatusWgt : public QWidget
{
    Q_OBJECT

public:
    enum DevMaintainStatus
    {
        DEV_MAINTAIN_STATUS_EMPTY = 0,                  // 空
        DEV_MAINTAIN_STATUS_NOT_MAINTAIN = 1,           // 未维护
        DEV_MAINTAIN_STATUS_MAINTAINING = 2,            // 正在维护
        DEV_MAINTAIN_STATUS_SUCC = 3,                   // 完成
        DEV_MAINTAIN_STATUS_FAILED = 4,                 // 维护失败
    };

	// 设备维护信息显示模式
	enum DevMaintainInfoShowModel
	{
		MODEL_EMPTY = 0,				     // 空
		MODEL_PERCENT  = 1,				     // 百分比显示模式
		MODEL_ITEM_CNT = 2					 // 项目数显示模式
	};

	// 设备显示模式
	enum DevDisplayModel
	{
		MODEL_NORMAL = 0,				     // 正常模式
		MODEL_LEFT   = 1,				     // 左边模式（c2000a使用）
		MODEL_RIGHT  = 2					 // 右边模式（c2000b使用）
	};

public:
    DevMaintainStatusWgt(QWidget *parent = Q_NULLPTR);
    ~DevMaintainStatusWgt();

    ///
    /// @brief
    ///     获取控件的设备名
    ///
    /// @return 设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
	QPair<QString, QString> GetDeviceName();

    ///
    /// @brief
    ///     设置设备名
    ///
    /// @param[in]  strDevName  设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetDeviceName(const QString& strDevName);

	///
	/// @brief 获取设备序列号
	///
	///
	/// @return 设备序列号
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年8月28日，新建函数
	///
	QString GetDeviceSn();

	///
	/// @brief 设置设备序列号
	///
	/// @param[in]  strDevSn  设备序列号
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年8月28日，新建函数
	///
	void SetDeviceSn(const QString& strDevSn);

    ///
    /// @brief
    ///     获取进度
    ///
    /// @return 进度百分百
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    QString GetProgress();

    ///
    /// @brief
    ///     设置进度
    ///
    /// @param[in]  strProgress  进度百分百
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetProgress(const QString& strProgress, const QString& strItemCount, const QString& strItemName);

    ///
    /// @brief
    ///     获取维护时间
    ///
    /// @return 维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    QString GetMaintainTime();

    ///
    /// @brief
    ///     设置维护时间
    ///
    /// @param[in]  strMaintainTime  维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void SetMaintainTime(const QString& strMaintainTime);

    ///
    /// @brief
    ///     获取设备维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    DevMaintainStatus GetStatus();

    ///
    /// @brief
    ///     设置设备维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetStatus(DevMaintainStatus enStatus);

    ///
    /// @brief
    ///     更新状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void UpdateStatus();

    ///
    /// @brief
    ///     重置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void Reset();

	///
	/// @brief
	///     设置维护信息显示模式
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月19日，新建函数
	///
	void SetMaintainInfoShowModel(const DevMaintainInfoShowModel &showModel);

	///
	/// @brief
	///     设置维护信息显示模式
	///
	/// @param[in] 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月19日，新建函数
	///
	void UpdateMaintainDetailToUi();

Q_SIGNALS:
    ///
    /// @brief
    ///     设备控件被点击
    ///
    /// @param[in]  strDevName  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SigDevDetailWgtClicked(QString strDevSn);

protected:
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
    ///     详情按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void OnDetailBtnClicked();

private:
    Ui::DevMaintainStatusWgt        *ui;                // UI对象指针

	QString							 m_strDevSn;		// 设备序列号
	QPair<QString, QString>			 m_pairDevName;		// (设备名称, 组名称)

    QString                          m_strProgress;     // 维护进度(百分比)
	QString                          m_strItemCount;    // 维护进度(项目数)
	QString                          m_strItemName;     // 当前维护项名称
    QString                          m_strMaintainTime; // 维护时间

    DevMaintainStatus                m_enStatus;        // 设备维护状态
    QMovie*                          m_pMovie;          // 动画

	DevMaintainInfoShowModel		 m_enShowModel;		// 显示信息设置

	DevDisplayModel					 m_enDisplayModel;  // 控件设备显示模式
};

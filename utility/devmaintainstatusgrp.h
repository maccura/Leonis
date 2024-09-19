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
/// @file     devmaintainstatusgrp.h
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

#include "devmaintainstatuswgt.h"
#include <QWidget>
#include <QStringList>
#include <QMap>
#include "src/thrift/gen-cpp/defs_types.h"

// 前置声明
class DevMaintainStatusWgt;              // 设备维护状态控件
class QPushButton;

class DevMaintainStatusGrp : public QWidget
{
    Q_OBJECT

public:
    DevMaintainStatusGrp(QWidget *parent = Q_NULLPTR);
    ~DevMaintainStatusGrp();

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
    ///     设置当前页
    ///
    /// @param[in]  iPage  页码编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void SetCurrentPage(int iPage);

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
    ///     重置所有设备状态控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void ResetAllDevStatusWgt();

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

Q_SIGNALS:
    ///
    /// @brief
    ///     设备控件被点击
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SigDevWgtClicked(QString strDevSn);

protected Q_SLOTS:
    ///
    /// @brief
    ///     上一页按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void OnPrevPageClicked();

    ///
    /// @brief
    ///     下一页按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月25日，新建函数
    ///
    void OnNextPageClicked();

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
    ///     维护组进度更新
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
    /// @li 4170/TangChuXian，2023年3月16日，新建函数
    ///
    void OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType);

	///
	/// @brief
	///     维护显示信息设置更新
	///
	/// @param[in]  isShowByItemCnt      是否通过项目数显示（true:项目数，false:百分比）
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月18日，新建函数
	///
	void OnMaintainShowSetUpdate(bool isShowByItemCnt);

private:
	///
	/// @brief
	///     获取维护显示模式
	///
	/// @return 维护显示模式
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月18日，新建函数
	///
	DevMaintainStatusWgt::DevMaintainInfoShowModel GetDevsMaintianShowModel();

	///
	/// @brief 通过最后一次维护信息获取维护进度
	///
	/// @param[in]  stuDetailInfo 维护细节 
	/// @param[out]  strItemName   当前执行到的维护项名称
	/// @param[out]  strProgress   整体维护进度
	/// @param[out]  strItemCount  维护计数
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月22日，新建函数
	///
	void GetMaintainProcessByLatestMaintainInfo(const tf::LatestMaintainInfo& stuDetailInfo, QString& strItemName, QString& strProgress, QString& strItemCount);

private:
    bool                                    m_bInit;            // 是否已经初始化
	QVector<QString>						m_vecDevSn;			// 设备序列号数组

    int                                     m_iCurPage;         // 当前页
    int                                     m_iTotalPage;       // 总页数
    QPushButton*                            m_pPrevPageBtn;     // 上一页按钮
    QPushButton*                            m_pNextPageBtn;     // 下一页按钮

    QVector<DevMaintainStatusWgt*>          m_vDevWgt;          // 设备状态控件数组
    QMap<QString, DevMaintainStatusWgt*>    m_mapDevToWgt;      // 设备序列号到设备维护状态控件的映射
};

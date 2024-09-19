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
/// @file     devmaintaindetail.h
/// @brief    设备维护详情对话框
///
/// @author   4170/TangChuXian
/// @date     2023年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "src/thrift/gen-cpp/defs_types.h"
#include "shared/basedlg.h"
#include <QList>
#include <QMap>
namespace Ui { class DevMaintainDetail; };
class QRadioButton;


class DevMaintainDetail : public BaseDlg
{
    Q_OBJECT

public:
    DevMaintainDetail(const QString strDevSn, QWidget *parent = Q_NULLPTR);
    ~DevMaintainDetail();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void showEvent(QShowEvent *event);

Q_SIGNALS:
    ///
    /// @brief
    ///     数据更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月4日，新建函数
    ///
    void SigDataUpdate();

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新维护详情信息
    ///
    /// @param[in]  bInit   是否是初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月9日，新建函数
    ///
    void UpdateMtDetailInfo(bool bInit = false);

    ///
    /// @brief
    ///     重新维护
    ///
    /// @param[in]  strDevName   设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月21日，新建函数
    ///
    void OnReMaintain(QString strDevName);

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
    ///     维护项时间更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enItemType      维护项类型
    /// @param[in]  strTime         时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月16日，新建函数
    ///
    void OnMaintainItemTimeUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, QString strTime);

    ///
    /// @brief
    ///     维护组阶段更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enPhaseType     阶段类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月20日，新建函数
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

private:
    Ui::DevMaintainDetail      *ui;                       // ui指针
    bool                        m_bInit;                  // 是否已经初始化

    QString                     m_strCurDevSn;            // 当前设备序列号
	QMap<QString, int>			m_mapIndexName;			  // 模块名称与索引的映射(重新维护使用)

    // 维护项计数
    int                         m_iWaitCnt;               // 等待数
    int                         m_iSuccCnt;               // 成功数
    int                         m_iFailedCnt;             // 失败数
};

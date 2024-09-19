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
/// @file     maintainitemdetailgrp.h
/// @brief    维护按钮组
///
/// @author   4170/TangChuXian
/// @date     2023年3月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "devmaintaindetailitemwgt.h"
#include <QScrollArea>
#include <QStringList>
#include <QMap>

class MaintainItemDetailGrp : public QScrollArea
{
    Q_OBJECT

public:
    MaintainItemDetailGrp(QWidget *parent = Q_NULLPTR);
    ~MaintainItemDetailGrp();

    ///
    /// @brief
    ///     获取维护名列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    const QMap<QString, QStringList>& GetMaintainNameList();

    ///
    /// @brief
    ///     设置维护列表
    ///
    /// @param[in]  strMaintainList  维护列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SetMaintainNameList(const QMap<QString, QStringList>& mapMaintainList);

    ///
    /// @brief
    ///     设置维护状态
    ///
    /// @param[in]  iRow            行号
    /// @param[in]  enStatus        维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SetMaintainStatus(int iRow, DevMaintainDetailItemWgt::DevMaintainItemStatus enStatus);

    ///
    /// @brief
    ///     获取维护状态
    ///
    /// @param[in]  iRow  行号
    ///
    /// @return 维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    DevMaintainDetailItemWgt::DevMaintainItemStatus GetMaintainStatus(int iRow);

    ///
    /// @brief
    ///     设置维护时间
    ///
    /// @param[in]  iRow  行号
    /// @param[in]  strMaintainTime  维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SetMaintainTime(int iRow, const QString& strMaintainTime);

    ///
    /// @brief
    ///     获取维护进度
    ///
    /// @param[in]  iRow  行号
    ///
    /// @return 维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    QString GetMaintainTime(int iRow);

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     重置所有维护状态按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void ResetAllMaintainStatusBtn();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     重新维护
    ///
    /// @param[in]  strDevName  设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月14日，新建函数
    ///
    void SigReMaintain(QString strDevName);

private:
    QFrame*                                 m_pContentWgt;          // 滚动区域内容窗口

    bool                                    m_bInit;                // 是否已经初始化
    QMap<QString, QStringList>              m_mapMaintainList;      // 设备名和维护列表映射

    QMap<int, DevMaintainDetailItemWgt*>    m_mapMaintainToWgt;     // 行号与维护项控件的映射
};

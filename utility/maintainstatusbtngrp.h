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
/// @file     maintainstatusbtngrp.h
/// @brief    维护按钮组
///
/// @author   4170/TangChuXian
/// @date     2023年2月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "maintainstatusbtn.h"
#include <QScrollArea>
#include <QStringList>
#include <QMap>

class MaintainStatusBtnGrp : public QScrollArea
{
    Q_OBJECT

public:
    MaintainStatusBtnGrp(QWidget *parent = Q_NULLPTR);
    ~MaintainStatusBtnGrp();

    ///
    /// @brief
    ///     获取维护名列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    const QStringList& GetMaintainNameList();

    ///
    /// @brief
    ///     设置维护列表
    ///
    /// @param[in]  strMaintainList     维护列表
    /// @param[in]  bIsItem             是否是单项维护
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void SetMaintainNameList(const QStringList& strMaintainList, bool bIsItem = false);

    ///
    /// @brief
    ///     设置维护类型
    ///
    /// @param[in]  strMaintainName  维护名
    /// @param[in]  enType           类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void SetMaintainType(const QString& strMaintainName, MaintainStatusBtn::BtnMaintainType enType);

    ///
    /// @brief
    ///     获取维护类型
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    MaintainStatusBtn::BtnMaintainType GetMaintainType(const QString& strMaintainName);

    ///
    /// @brief
    ///     设置维护状态
    ///
    /// @param[in]  strMaintainName  维护名
    /// @param[in]  enStatus         维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void SetMaintainStatus(const QString& strMaintainName, MaintainStatusBtn::BtnMaintainStatus enStatus);

    ///
    /// @brief
    ///     获取维护状态
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @return 维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    MaintainStatusBtn::BtnMaintainStatus GetMaintainStatus(const QString& strMaintainName);

    ///
    /// @brief
    ///     设置设备类别
    ///
    /// @param[in]  strMaintainName  维护名
    /// @param[in]  enStatus         维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月19日，新建函数
    ///
    void SetDevClassify(const QString& strMaintainName, MaintainStatusBtn::BtnDevClassify enDevClassify);

    ///
    /// @brief
    ///     获取设备类别
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @return 维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月19日，新建函数
    ///
    MaintainStatusBtn::BtnDevClassify GetDevClassify(const QString& strMaintainName);

    ///
    /// @brief
    ///     设置维护进度
    ///
    /// @param[in]  strMaintainName  维护名
    /// @param[in]  dProgress        进度百分百，1表示百分之百，0表示百分之0
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void SetMaintainProgress(const QString& strMaintainName, double dProgress);

    ///
    /// @brief
    ///     获取维护进度
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @return 维护进度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    double GetMaintainProgress(const QString& strMaintainName);

    ///
    /// @brief
    ///     设置维护进度是否可见
    ///
    /// @param[in]  strMaintainName     进度名
    /// @param[in]  bVisible            是否可见
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void SetMaintainProgressVisible(const QString& strMaintainName, bool bVisible);

    ///
    /// @brief
    ///     获取维护进度是否可见
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @return true表示可见
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    bool IsMaintainProgressVisible(const QString& strMaintainName);

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     重置所有维护状态按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    void ResetAllMaintainStatusBtn();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月16日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     维护状态按钮点击信号
    ///
    /// @param[in]    strMaintainName 维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void SigMtStatusBtnClicked(QString strMaintainName);

protected Q_SLOTS:
    ///
    /// @brief
    ///     维护状态按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void OnMtStatusBtnClicked();

private:
    QFrame*                                 m_pContentWgt;          // 滚动区域内容窗口

    bool                                    m_bInit;                // 是否已经初始化
    bool                                    m_bMtItem;              // 是否是单项维护
    int                                     m_iColCnt;              // 列数
    QStringList                             m_strMaintainList;      // 维护列表

    QMap<QString, MaintainStatusBtn*>       m_mapMaintainToWgt;     // 设备名到设备维护状态控件的映射
};

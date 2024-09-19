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
/// @file     maintainstatusbtn.h
/// @brief    维护状态按钮
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

#include <QPushButton>
#include <QIcon>
#include <QMap>
namespace Ui { class MaintainStatusBtn; };

class MaintainStatusBtn : public QPushButton
{
    Q_OBJECT

public:
    // 按钮维护状态
    enum BtnMaintainStatus
    {
        BTN_MAINTAIN_STATUS_NORMAL = 0,                 // 正常（正在维护或完成）
        BTN_MAINTAIN_STATUS_FAILED,                     // 维护失败
    };

    // 按钮维护类型
    enum BtnMaintainType
    {
        BTN_MAINTAIN_TYPE_MAINTAIN_GROUP = 0,           // 维护组
        BTN_MAINTAIN_TYPE_MAINTAIN_ITEM,                // 维护项
    };

    // 按钮设备类别
    enum BtnDevClassify
    {
        BTN_DEV_CLASSIFY_NONE = 0,                      // 无
        BTN_DEV_CLASSIFY_IM,                            // 免疫
        BTN_DEV_CLASSIFY_CH,                            // 生化
    };

public:
    MaintainStatusBtn(QWidget *parent = Q_NULLPTR);
    ~MaintainStatusBtn();

    ///
    /// @brief
    ///     获取控件的维护
    ///
    /// @return 维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    QString GetMaintainName();

    ///
    /// @brief
    ///     设置维护名
    ///
    /// @param[in]  strMaintainName  维护名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetMaintainName(const QString& strMaintainName);

    ///
    /// @brief
    ///     获取进度
    ///
    /// @return 进度百分比：1表示100%，0表示0%
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    double GetProgress();

    ///
    /// @brief
    ///     设置进度
    ///
    /// @param[in]  dProgress  进度百分比：1表示100%，0表示0%
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetProgress(double dProgress);

    ///
    /// @brief
    ///     获取按钮维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    BtnMaintainStatus GetStatus();

    ///
    /// @brief
    ///     设置按钮维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetStatus(BtnMaintainStatus enStatus);

    ///
    /// @brief
    ///     获取按钮维护类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    BtnMaintainType GetMaintainType();

    ///
    /// @brief
    ///     设置按钮维护类型
    ///
    /// @param[in]  enType  维护类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void SetMaintainType(BtnMaintainType enType);

    ///
    /// @brief
    ///     获取设备类别
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月18日，新建函数
    ///
    BtnDevClassify GetDevClassify();

    ///
    /// @brief
    ///     设置设备类别
    ///
    /// @param[in]  enClassify  设备类别
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月18日，新建函数
    ///
    void SetDevClassify(BtnDevClassify enClassify);

    ///
    /// @brief
    ///     进度是否显示
    ///
    /// @return true表示显示进度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    bool IsProgressVisible();

    ///
    /// @brief
    ///     设置进度是否显示
    ///
    /// @param[in]  bVisible  是否显示进度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void SetProgressVisible(bool bVisible);

    ///
    /// @brief
    ///     更新状态显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月13日，新建函数
    ///
    void UpdateStatusDisplay();

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
    ///     设置图标
    ///
    /// @param[in]  btnIcon  按钮图标
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月26日，新建函数
    ///
    void SetIcon(const QIcon& btnIcon);

    ///
    /// @brief
    ///     初始化维护图标映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月26日，新建函数
    ///
    static void InitMaintainIconMap();

private:
    Ui::MaintainStatusBtn        *ui;                // UI对象指针

    BtnMaintainStatus             m_enStatus;        // 按钮维护状态
    BtnMaintainType               m_enType;          // 维护类型
    BtnDevClassify                m_enDevClassify;   // 设备类别

    double                        m_dProgress;       // 进度
    const int                     m_ciTextAreaWidth; // 文本区域宽度
    const int                     m_ciTextFontSize;  // 文本字体大小
    const QSize                   m_csizeIcon;       // 图标尺寸

    static QMap<QString, QIcon>   sm_mapMaintainIcon;// 维护图标映射【维护名-图标】
};

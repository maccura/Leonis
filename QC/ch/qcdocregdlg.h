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
/// @file     qcdocregdlg.h
/// @brief    质控登记对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
namespace Ui { class QcDocRegDlg; };

class QcDocRegDlg : public BaseDlg
{
    Q_OBJECT

public:
    QcDocRegDlg(QWidget *parent = Q_NULLPTR);
    ~QcDocRegDlg();

    ///
    /// @brief
    ///     设置内容页列表
    ///
    /// @param[in]  wgtList  窗口列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SetContentPageList(const QList<QWidget*>& wgtList);

    ///
    /// @brief
    ///     设置当前页
    ///
    /// @param[in]  pWidget  界面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SetCurrentPage(QWidget* pWidget);

    ///
    /// @brief
    ///     设置修改质控模式
    ///
    /// @param[in]  bModifyQcMode  修改质控模式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SetModifyQcMode(bool bModifyQcMode);

    ///
    /// @brief
    ///     设置忽略试剂图标是否显示
    ///
    /// @param[in]  bVisible  是否显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月31日，新建函数
    ///
    void SetIgnRgntIconVisible(bool bVisible);

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
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
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void showEvent(QShowEvent *event);

Q_SIGNALS:
    ///
    /// @brief
    ///     切换到扫描登记页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SigSwitchScanRegPage();

    ///
    /// @brief
    ///     切换手工登记页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SigSwitchManualRegPage();

    ///
    /// @brief
    ///     切换质控下载页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SigSwitchQcDownloadPage();

private:
    Ui::QcDocRegDlg        *ui;                                 // ui对象指针
    bool                    m_bInit;                            // 是否已经初始化
    bool                    m_bModifyQcMode;                    // 修改质控模式
    const int               m_ciModifyStrechHeight;             // 修改模式伸缩高度
};

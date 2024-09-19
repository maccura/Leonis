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
/// @file     imaddreactcupdlg.h
/// @brief    免疫添加反应杯对话框
///
/// @author   4170/TangChuXian
/// @date     2022年9月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/BaseDlg.h"
#include <QVariant>
namespace Ui { class ImAddReactCupDlg; };

class ImAddReactCupDlg : public BaseDlg
{
    Q_OBJECT

public:
    ImAddReactCupDlg(QWidget *parent = Q_NULLPTR);
    ~ImAddReactCupDlg();

    ///
    /// @brief
    ///     设置扫描到的耗材数据
    ///
    /// @param[in]  varSplData  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void SetScanSplData(const QVariant& varSplData);

    ///
    /// @brief
    ///     获取扫描到的耗材数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    const QVariant& GetScanSplData();

protected:
    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

protected Q_SLOTS:
    ///
    /// @brief
    ///     扫码定时器触发
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void OnScanTimer();

    ///
    /// @brief
    ///     条码改变（表明正在输入条码）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void OnBarcodeChanged();

Q_SIGNALS:
    ///
    /// @brief
    ///     条码扫描完成
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年7月7日，新建函数
    ///
    void SigBarcodeScanFinished();

private:
    Ui::ImAddReactCupDlg               *ui;                     // UI对象指针
    bool                                m_bInit;                // 是否初始化

    // 条码扫描
    QTimer*                             m_pScanTimer;           // 扫码定时器
    int                                 m_iOverTimeCnt;         // 最大超时次数
    QVariant                            m_varSplData;           // 耗材数据

    // 友元类
    friend class                        ImRgntPlateWidget;      // 免疫试剂盘界面
    friend class                        QSupplyList;            // 耗材信息界面
};

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
/// @file     dbgprocessdlg.h
/// @brief    调试流程对话框
///
/// @author   4170/TangChuXian
/// @date     2024年2月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <QMap>
namespace Ui { class DbgProcessDlg; };

// 前置声明
class QTableWidgetItem;                     // 表格单元项

class DbgProcessDlg : public BaseDlg
{
    Q_OBJECT

public:
    DbgProcessDlg(QWidget *parent = Q_NULLPTR);
    ~DbgProcessDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
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
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     执行按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月23日，新建函数
    ///
    void OnExcuteBtnClicked();

    ///
    /// @brief
    ///     删除按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月15日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief
    ///     导入按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月15日，新建函数
    ///
    void OnInportBtnClicked();

    ///
    /// @brief
    ///     更新调试流程表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月18日，新建函数
    ///
    void UpdateDbgProcessTbl();

    ///
    /// @brief
    ///     表格选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月18日，新建函数
    ///
    void OnTableSelectChanged();

    ///
    /// @brief
    ///     设备单选框被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    void OnDevRBtnClicked();

    ///
    /// @brief
    ///     收到调试流程
    ///
    /// @param[in]  stuTfDbgProc  调试流程
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    void OnRecvExeRlt(im::tf::DebugProcess stuTfDbgProc);

private:
    // 测试流程，表头枚举
    enum DbgProcessHeader {
        Tph_Name = 0,                   // 名称
        Tph_RepeatTime,                 // 重复次数
        Tph_Result,                     // 结果
    };

private:
    Ui::DbgProcessDlg                  *ui;                     // ui指针
    bool                                m_bInit;                // 是否已经初始化

    // 表格
    int                                 m_ciDefaultRowCnt;      // 默认行数
    int                                 m_ciDefaultColCnt;      // 默认列数

    // 设备列表
    QString                             m_strCurDevName;        // 当前设备名称
    QStringList                         m_strDevNameList;       // 设备列表

    // 缓存执行情况
    QMap<QPair<QString, QString>, QPair<int, QString>> mapExeBufer;  // 执行状态缓存（设备sn，模块 - 重复次数，状态）

    // 资源文件目录
    const QString                       m_cstrFileDirPath;      // 资源文件路径
};

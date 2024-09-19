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
/// @file     QQcLeveyJennPage.h
/// @brief    L-J质控图界面
///
/// @author   8090/YeHuaNing
/// @date     2022年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui { class QQcLeveyJennPage; };
class QQcLJResultModel;
class QQcProjectModel;


class QQcLeveyJennPage : public QWidget
{
    Q_OBJECT

public:
    using DeviceModules = std::vector<std::shared_ptr<const tf::DeviceInfo>>;
    QQcLeveyJennPage(QWidget *parent = Q_NULLPTR);
    ~QQcLeveyJennPage();

    ///
    /// @brief 启动Levey Jennings页面的列表显示
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @return 成功，返回true
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    bool StartLeveyJennAppPage(DeviceModules& devices);

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief 初始化表格内容
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitTableView();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

protected Q_SLOTS:

    ///
    /// @brief 项目编号或者设备切换
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月17日，新建函数
    ///
    void OnAssayOrDeviceChanged();

signals:
    ///
    /// @brief 初始化界面视图数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void InitViewData();

    ///
    /// @brief 初始化界面视图数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void AssayCodeChangde(const std::string& device, int assay, const QString& startDate, const QString& endDate);

    ///
    /// @brief 选择的设备发生了变化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月14日，新建函数
    ///
    void DeviceChanged();

private:
    Ui::QQcLeveyJennPage            *ui;                        ///<  UI对象
    DeviceModules                   m_devInfos;
    bool                            m_bInit;                    ///< 是否已经初始化
    QQcProjectModel*                m_qcProList;                ///< 质控品列表
    QQcLJResultModel*               m_pTbl1Model;               ///< 图表1质控结果模型
    QQcLJResultModel*               m_pTbl2Model;               ///< 图表2质控结果模型
    QQcLJResultModel*               m_pTbl3Model;               ///< 图表3质控结果模型
};

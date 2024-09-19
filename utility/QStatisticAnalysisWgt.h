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
/// @file     QStatisticAnalysisWgt.h
/// @brief 	  统计分析界面
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <mutex>
#include <condition_variable>
namespace Ui { class QStatisticAnalysisWgt; };

class QStatisticAnalysisWgt : public QWidget
{
	Q_OBJECT

public:
	QStatisticAnalysisWgt(QWidget *parent = Q_NULLPTR);
	~QStatisticAnalysisWgt();

    ///
    ///  @brief 用于记录页签句柄等信息
    ///
    struct TabInfo
    {
        int m_tbId;
        QWidget* m_widget;
        QString m_Name;

        TabInfo(int tbid, QWidget* wdg, const QString& name)
        {
            m_tbId = tbid;
            m_widget = wdg;
            m_Name = name;
        }
    };

	///
	///  @brief 初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
	///
	void Init();

protected Q_SLOTS:

	///
	///  @brief 响应打印按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
	///
	void OnPrintBtnClicked();

	///
	///  @brief 响应导出按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
	///
	void OnExportBtnClicked();

    ///
    /// @bref
    ///		权限变更响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月26日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		响应提示信息信号
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年1月29日，新建函数
    ///
    void OnHint(QString strInfo);

    ///
    /// @bref
    ///		响应提示信息信号
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年1月29日，新建函数
    ///
    void OnImportFileName();

Q_SIGNALS:
    void hint(QString strInfo);         // 提示信息
    void importFileName();

private:
	Ui::QStatisticAnalysisWgt *ui;

    std::vector<TabInfo> m_tabs;     ///< tab页签缓存，用于寻找页签，因为remove后index可能会变化

    std::mutex                  m_mtx;
    std::condition_variable     m_CV;           // 条件变量，用于控制导出文件名的输入
    QString                     m_strFileName;  // 文件名，用于导出文件
};

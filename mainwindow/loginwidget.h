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
/// @file     loginwidget.h
/// @brief    登录界面
///
/// @author   4170/TangChuXian
/// @date     2020年4月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include "src/thrift/gen-cpp/DcsControl.h"
#include <QWidget>
#include <QPair>
#include <QMap>

namespace Ui {
    class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = Q_NULLPTR);
    ~LoginWidget();

protected:
    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月8日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief 通过界面按钮退出软件，过滤Alt+F4关闭窗口事件
	///
	/// @param[in]  eventType  
	/// @param[in]  message  
	/// @param[in]  result  
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月9日，新建函数
	///
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

protected slots:
    ///
    /// @brief 登录按钮被点击
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月29日，新建函数
    ///
    void OnLoginBtnClicked();

    ///
    /// @brief 关机按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月29日，新建函数
    ///
    void OnShutdownBtnClicked();

	///
	/// @brief  用户名称回车
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年3月20日，新建函数
	///
	void OnUserNameEntern();

    ///
    /// @brief
    ///     用户输入内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月8日，新建函数
    ///
    void OnInputContent();

private:
    Ui::LoginWidget *ui;                    // ui对象

};
#endif //LOGINWIDGET_H

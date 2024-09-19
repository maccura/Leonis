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
/// @file     SoftwareVersionDlg.h
/// @brief    软件版本弹窗
///
/// @author   7951/LuoXin
/// @date     2022年9月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "shared/basedlg.h"

#define SOFTWARE_VERSION		"1.0.00.240917"

namespace Ui {
    class SoftwareVersionDlg;
    class SoftwareFontPermitDlg;
};

class SoftwareVersionManageDlg;

class SoftwareVersionDlg : public BaseDlg
{
	Q_OBJECT

public:
	SoftwareVersionDlg(QWidget *parent = Q_NULLPTR);
	~SoftwareVersionDlg();

    ///
    /// @brief	加载显示数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年9月16日，新建函数
    ///
    void LoadDataToDlg();

	///
	/// @brief  保存版本号信息到数据库
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 3558/ZhouGuangMing，2024年7月8日，新建函数
	///
	static bool SaveVersion2Db();

private:

    ///
    /// @brief	初始化表格
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年7月23日，新建函数
    ///
    void InitTableView();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年7月30日，新建函数
    ///
    void showEvent(QShowEvent *event);

signals:
    void ShowVersionManageDlg();

public  Q_SLOTS:

    ///
    /// @brief	服务帮助按钮响应槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月04日，新建函数
    ///
    void OnServiceHelpClicked();

    ///
    /// @brief	关闭按钮响应槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月04日，新建函数
    ///
    void OnCloseBtnClicked();

    ///
    /// @brief	详情按钮响应槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年07月30日，新建函数
    ///
    void OnDetailBtnClicked();

    ///
    /// @brief 更新按钮响应槽函数
    /// 
    ///     
    /// @li 4058/WangZhiNang，2024年1月15日，新建
    ///
    void OnUpdateBtnClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年07月30日，新建函数
    ///
    void OnPermisionChanged();

private:
    QString CurrentTimeString();
    QString CurrentSoftwareVersion();

private:
	Ui::SoftwareVersionDlg*								ui;
    SoftwareVersionManageDlg*                           m_pSoftwareVersionManageDlg;    // 工程师权限软件版本弹窗
};

// 软件字体著作权申明
class SoftwareFontPermitDlg : public BaseDlg
{
    Q_OBJECT
public:
enum widgetType 
    {
        FRONT_WIDGET,   // 字体声明窗口
        LEONIS_WIDGET,  // 软件声明窗口
        QT_WIDGET       // QT声明窗口
    };

    SoftwareFontPermitDlg(QWidget *parent = Q_NULLPTR);
    ~SoftwareFontPermitDlg() {}

    void showWidgetByType(widgetType type);

private:
    QString readFileToQString(const QString& filePath);

private:
    Ui::SoftwareFontPermitDlg*						    ui;
};

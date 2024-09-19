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
/// @file     ExportSetDlg.h
/// @brief    导出设置对话框
///
/// @author   8580/GongZhiQiang
/// @date     2023年11月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/public/ConfigSerialize.h"

#include <tuple>
#include <QButtonGroup>
#include <QMap>

namespace Ui { class ExportSetDlg; };

class ExportSetDlg : public BaseDlg
{
	Q_OBJECT

public:
	ExportSetDlg(QWidget *parent = Q_NULLPTR);
	~ExportSetDlg();

protected:
    // 网格标题信息
    struct GridTitleInfo 
    {
        GridTitleInfo() : iColumnCnt(0), iItemCnt(0), strTitle(""){}
        GridTitleInfo(int columnCnt, int itemCnt, const QString& title) : iColumnCnt(columnCnt), iItemCnt(itemCnt), strTitle(title) {}

        int     iColumnCnt;         // 列数
        int     iItemCnt;           // 项目数
        QString strTitle;           // 标题名称
    };

protected:

	///
	/// @brief 显示前初始化
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void InitBeforeShow();

	///
	/// @brief 显示后初始化
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void InitAfterShow();
	
	///
	/// @brief 初始化信号槽
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void InitConnect();

	///
	/// @brief 初始化子控件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void InitChildCtrl();

	///
	/// @brief 初始化数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void InitData();

	///
	/// @brief 初始化ISE样本&质控界面
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void InitIseSamAndQcPage();

	///
	/// @brief 初始化ISE校准界面
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void InitIseCaliPage();

	///
	/// @brief 初始化常规&质控界面
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void InitNormalAndQcPage();

	///
	/// @brief 初始化校准界面
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void InitCaliPage();

    ///
    /// @brief
    ///     初始化免疫页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月1日，新建函数
    ///
    void InitImmunePage();
	
	///
	/// @brief 往目标界面添加checkBox
	///
	/// @param[in]  spWidget  目标界面
	/// @param[in]  checkBoxList  需要添加的checkBox
    /// @param[in]  mapTitle  标题映射（行号->标题）
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
    /// @li 4170/TangChuXian，2023年12月18日，增加默认参数
	///
	void AddCheckBoxsToWidget(QWidget* spWidget, const QVector<QPair<QString, ExportInfoEn>>& checkBoxList, 
        const QMap<int, GridTitleInfo>& mapTitle = QMap<int, GridTitleInfo>());

	///
	/// @brief 更新目标界面的数据
	///
	/// @param[in]  spWidget  目标界面
	/// @param[in]  dataInfo  数据信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void UpdateWidgetCheckBoxStatus(const QWidget* spWidget, const std::set<ExportInfoEn>& dataInfo);

	///
	/// @brief 保存目标界面的信息
	///
	/// @param[in]  spWidget  目标界面
	/// @param[in]  dataInfo  保存的信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月28日，新建函数
	///
	void SaveWidgetCheckBoxStatus(const QWidget* spWidget, std::set<ExportInfoEn>& dataInfo);

	void showEvent(QShowEvent *event);

protected slots:

	///
	/// @brief 保存按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月24日，新建函数
	///
	void OnSaveBtnClicked();

private:
	Ui::ExportSetDlg *ui;

	bool                     m_bInit;                               // 是否已经初始化
	QButtonGroup			 m_btnGroup;							// 生化界面选择按钮
};

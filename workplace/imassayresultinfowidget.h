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
/// @file     imassayresultinfowidget.h
/// @brief    免疫项目结果详情信息界面
///
/// @author   4170/TangChuXian
/// @date     2022年9月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui { class ImAssayResultInfoWidget; };

class ImAssayResultInfoWidget : public QWidget
{
    Q_OBJECT

public:
	ImAssayResultInfoWidget(QWidget *parent = Q_NULLPTR);
    ~ImAssayResultInfoWidget();

    ///
    /// @brief
    ///     清空界面信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月21日，新建函数
    ///
    void Clear();

	///
	/// @brief 更新结果详情显示
	///
	/// @param[in]  data  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月3日，新建函数
	///
	void UpdateDetailData(const ::im::tf::AssayTestResult& data, bool bVolEditEnable, bool bIsFirst = true, bool bIsLastReTest = false);

	///
	/// @brief 设置当前显示的testItem（数据浏览）
	///
	/// @param[in]  testItem  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月11日，新建函数
	///
	void SetCurrentTestItem(const tf::TestItem& testItem);

	///
	/// @brief 获取当前显示的testItem（数据浏览）
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月14日，新建函数
	///
	const tf::TestItem& GetCurrentTestItem();

	///
	/// @brief 保存修改后的检测结果
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月7日，新建函数
	///
	bool Save();

	///
	/// @brief 将当前页面复查结果设置为复查结果（数据浏览）
	///
	///
	/// @return true:设置成功，false:设置失败
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月11日，新建函数
	///
	bool SetCurrentResult();

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月25日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月25日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月25日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

    protected slots:

    ///
    /// @bref
    ///		权限限制
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月11日，新建函数
    ///
    void OnPermisionChanged();

Q_SIGNALS:
    void dataChanged();

private:
    Ui::ImAssayResultInfoWidget     *ui;                   // ui对象指针
    bool                             m_bInit;              // 是否初始化

	::im::tf::AssayTestResult		m_assayResult;			///< 项目测试结果
	tf::TestItem                    m_currentTestItem;

	bool m_bIsFirst;				///< 是否是第一次测试
	bool m_bIsLastReTest;			///< 是否是最后一次测试

    // 设置友元类
    friend class                     ItemResultDetailDlg;  // 项目结果详情
};

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
/// @file     QDialogBatchRegister.h
/// @brief    批量输入对话框
///
/// @author   5774/WuHongTao
/// @date     2022年5月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/public/ConfigSerialize.h"
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

namespace Ui {
    class QDialogBatchRegister;
};


class QDialogBatchRegister : public BaseDlg
{
	Q_OBJECT

public:

	// 批量输入模式
	enum BATCHMODE
	{
		STOPSAMPLENO = 0,      ///< 终止样本号/条码模式
		SAMPLECOUNT            ///< 样本数量模式
	};

	QDialogBatchRegister(QWidget *parent = Q_NULLPTR);
	~QDialogBatchRegister();

signals:
	///
	/// @brief 批量输入参数已选择信号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月12日，新建函数
	///
	void batchParameterInputed();

public:
	///
	/// @brief 获取终止样本号和样本数量
	///
	/// @param[in]  stop    样本号
	/// @param[in]  count   样本数量
    /// @param[in]  times   每个项目测试数量
	///
	/// @return 批量输入模式类型
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	BATCHMODE GetBatchInputPatater(cpp_int& stop, int& count, int& times);

    ///
    /// @brief 设置默认结束样本
    ///
    /// @param[in]  seq  样本号
    ///
    /// @return true表示设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月8日，新建函数
    ///
    bool SetSefaultStopNo(cpp_int seq, bool mode);

protected slots:
	///
	/// @brief 确定按钮槽函数
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月12日，新建函数
	///
	void OnOkBtnClicked();

	///
	/// @brief 批量输入的改变
	///
	/// @param[in]  text  改变字符
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	void OnSampleEditChanged(const QString& text);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

private:
	Ui::QDialogBatchRegister*		ui;
	AddSampleAction					m_batchAction;				///< 批量添加设置属性
    cpp_int							m_stop;					    ///< 终止样本号/条码
    cpp_int							m_start;			        ///< 开始样本号/条码
	int							    m_count;					///< 样本数量
    int                             m_testTimes;                ///< 每个项目测试个数
    bool                            m_barMode;                  ///< 条码模式（true:条码模式，否在序号模式）
};

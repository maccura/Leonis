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

#pragma once

#include <QWidget>

namespace Ui 
{
    class QIseCard;
};
class QProgressIseBarShell;

namespace ise
{
namespace tf
{
    class SuppliesInfo;
}
}
namespace ch
{
namespace tf
{
    class SuppliesInfo;
}
}

class QIseCard : public QWidget
{
    Q_OBJECT

public:
    QIseCard(QWidget *parent = Q_NULLPTR);
    ~QIseCard() {};

    ///
    /// @brief 设置ISE仓内耗材信息
    ///
    /// @param[in]  supplyInfo  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月8日，新建函数
    ///
    void SetIseSupply(ise::tf::SuppliesInfo supplyInfo);

    ///
    /// @brief  设置ISE台面耗材信息
    ///
    /// @param[in]  supplyInfo  耗材信息
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年10月27日，新建函数
    ///
    void SetSmpClnSupply(ise::tf::SuppliesInfo supplyInfo);

	///
	/// @brief  设置耗材的选中与取消
	///
	/// @param[in]  ck  true表示选中 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void SetChecked(bool ck);

	///
	/// @brief  获取图标指针 主要用于点击图标的选中事件
	///
	/// @return 图标指针
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
    QProgressIseBarShell* getIcon() const;

	///
	/// @brief  获取设备序列号
	///
	/// @return 设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	std::string getDevSN() const { return m_devSn; };

	QString getReamainCountColor(const int supplyCode, const double residual);

signals:

	///
	/// @brief  余量探测
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void signalRemainDetect();

private:
    Ui::QIseCard*                   ui;
	std::string						m_devSn;
};


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
#include <QMap>
#include "src/thrift/ise/gen-cpp/ise_types.h"

class QRecordISESupply;
class QPushButton;
class QIseCard;
class QIsePerfusionDlg;
struct IseDeviceOtherInfo;

namespace Ui {
    class QPageIseSupply;
};


class QPageIseSupply : public QWidget
{
    Q_OBJECT

public:
    QPageIseSupply(QWidget *parent = Q_NULLPTR);
    ~QPageIseSupply();

    ///
    /// @brief 更新ISE耗材信息
    ///
    /// @param[in]  deviceSn  ISE设备序列号
	/// @param[in]  setPos	  需要更新的位置信息，为空则全部更新
    ///
    /// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
    ///
	void UpdateIseSupplies(const std::string& deviceSn, const std::set< std::pair<int, int>> &setPos = {});

	///
	/// @brief  更新指定位置的耗材信息
	///
	/// @param[in]  si			耗材信息
	/// @param[in]  otherInfo	设备其他信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void UpdateIseSupply(const ise::tf::SuppliesInfo &si, const IseDeviceOtherInfo& otherInfo);

	///
	/// @brief  隐藏指定位置的耗材
	///
	/// @param[in]  posInfo  位置信息（模块，位置）
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void hideSupply(const std::pair<int, int>& posInfo);

    ///
    /// @brief 注册ISE耗材
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月7日，新建函数
    ///
    void OnApplyIseSupply();

    ///
    /// @brief  更新ISE液路灌注按钮使能
    ///
    /// @param[in]  devSn  设备序列号
    /// @param[in]  status  设备状态
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年11月16日，新建函数
    ///
    void updatePerfusionBtnEnable(const std::string &devSn, const int &status);

private:

	///
	/// @brief  清除所有的选中状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void clearSelect();

	///
	/// @brief  设置选中耗材
	///
	/// @param[in]  sender  选中耗材指针
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void OnSelect(QObject * sender);

protected:
    ///
    /// @brief 页面显示时，处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief  重写事件过滤器 -> 点击选中状态
	///
	/// @param[in]  obj     产生事件的对象
	/// @param[in]  event   事件
	///
	/// @return bool    true表示接收该事件
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月26日，新建函数
	///
	bool eventFilter(QObject *obj, QEvent *event) override;

protected slots:
	///
	/// @brief ISE项目更新耗材的消息槽函数
	///
	/// @param[in]  supplyUpdates  更新耗材的位置编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月4日，新建函数
	///
	void OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief  点击液路灌注按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月16日，新建函数
	///
	void OnISEPerfusion();

	///
	/// @brief   设备OtherInfo信息更新
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void OnDeviceOtherInfoUpdate(const QString &devSn);

private:
    Ui::QPageIseSupply*             ui;
    QRecordISESupply*				m_regIseDialog;				///< 登记ISE的对话框
    std::string						m_devSn;					///< 当前设备sn
	std::shared_ptr<const tf::DeviceInfo> m_device;				///< 当前设备指针

	// 选中耗材项
	enum EnumSelSupply
	{
		EM_IS_1 = 0,
		EM_BUF_1,
		EM_IS_2,
		EM_BUF_2
	};

	QMap<EnumSelSupply, QIseCard* >	m_mapIseCards;				///< IS和BUF耗材映射
	int								m_selectSupply = -1;		///< 当前选中耗材
	QIsePerfusionDlg*				m_isePerfusionDlg;			///< ISE液路灌注
};

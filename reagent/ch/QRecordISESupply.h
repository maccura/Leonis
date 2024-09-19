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
/// @file     QRecordISESupply.h
/// @brief    ISE耗材的登记
///
/// @author   5774/WuHongTao
/// @date     2022年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/baseDlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QKeyEvent>

namespace Ui {
    class QRecordISESupply;
};


class QRecordISESupply : public BaseDlg
{
	Q_OBJECT

public:
	QRecordISESupply();
	~QRecordISESupply();

	///
	/// @brief  启动对话框
	///
	/// @param[in]  device		设备指针
	/// @param[in]  moduleIdx   模块索引
	/// @param[in]  type		耗材类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月26日，新建函数
	///
	void StartDialog(const std::shared_ptr<const tf::DeviceInfo> &device, const int moduleIdx, const int type);

	///
	/// @brief 获得耗材的加载句柄
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月11日，新建函数
	///
	static QRecordISESupply& GetInstance();

private:

	///
	/// @brief  初始化UI
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月12日，新建函数
	///
	void InitUi();
	
	///
	/// @brief  初始化连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月12日，新建函数
	///
	void InitConnect();

	///
	/// @brief  填充参数
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月26日，新建函数
	///
	void FillParamter();

	///
	/// @brief  保存耗材登记消息
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月26日，新建函数
	///
	bool SaveSupply();

	///
	/// @brief  取消异步任务
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月25日，新建函数
	///
	void CancelTask();

	///
	/// @brief  解析条码 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月9日，新建函数
	///
	void OnParseBarcode();

protected:
	
	///
	/// @brief  键盘按下事件
	///
	/// @param[in]  event  键盘事件
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月12日，新建函数
	///
	void keyPressEvent(QKeyEvent *event) override;

private slots:

	///
	/// @brief  点击RadioButton
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月12日，新建函数
	///
	void OnRadioButtonClicked();

private:
	Ui::QRecordISESupply*				ui;

	std::shared_ptr<const tf::DeviceInfo>		m_device;				///< 缓存当前设备
	int									m_moduleIndex;			///< 缓存模块索引
	ise::tf::SuppliesType::type			m_iseSupplyType;		///< 耗材类型
	bool								m_isRunning;			///< 记录耗材加载前的状态是否是运行态
	int									m_id = -1;				///< 当前耗材数据库主键
	ise::tf::SupplyPosInfo				m_supplyPos;			///< 位置信息
	QFuture<bool>						m_future ;				///< 耗材加载异步任务返回
	QFutureWatcher<bool>				m_watcher;				///< 监听耗材加载异步任务
	QTimer*								m_timer = nullptr;		///< 耗材加载定时器
	int									m_countdown = 0;		///< 耗材加载倒计时
	bool								m_isCancel = false;		///< 是否取消异步任务
	QIntValidator*						m_validator = nullptr;	///< 输入限制
};

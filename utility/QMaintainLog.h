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
/// @file     QMaintainLog.h
/// @brief 	 应用->日志->维护日志界面
///
/// @author   7656/zhang.changjiang
/// @date      2022年8月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年8月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "PrintExportDefine.h"
#include <QToolTip>

class MaintainLogModel;
namespace Ui {
    class QMaintainLog;
};


class QMaintainLog : public BaseDlg
{
	Q_OBJECT

public:
	QMaintainLog(QWidget *parent = Q_NULLPTR);
	~QMaintainLog();

	///
	/// @brief
	///     获取维护日志页面关闭按钮指针
	///
	/// @return		故障日志页面关闭按钮指针
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月7日，新建函数
	///
    QPushButton* GetCloseBtnPtr();
	
	///
	///  @brief
	///		初始化
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月2日，新建函数
	///
	void Init();

	///
	///  @brief 连接信号槽
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void ConnectSigAndSlots();

	///
	///  @brief	查询维护一月之内日志
	///
	///
	///  @param[in]   year		年
	///  @param[in]   mon		月
	///  @param[in]   deviceSN  设备编号
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void QueryMaintainLog(int year, int mon, std::string deviceSN);

    ///
    ///  @brief	查询维护一天之内的日志
    ///
    ///
    ///  @param[in]   year		年
    ///  @param[in]   mon		月
    ///  @param[in]   day		日
    ///  @param[in]   deviceSN  设备编号
    ///  @param[in]   group     维护组
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
    ///
	void QueryMaintainLog(int year, int mon, int day, std::string deviceSN, ::tf::MaintainGroup& group);
	
	///
	///  @brief 显示维护详情
	///
	///
	///  @param[in]   lstMaintainLogs  维护组日志列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月23日，新建函数
	///
	void ShowMaintainDetail(const std::vector<::tf::MaintainLog> & lstMaintainLogs);

	///
	///  @brief	设置垂直表头
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void SetVerHeader();
	
	///
	///  @brief	设置水平表头
	///
	///
	///  @param[in]   year  年
	///  @param[in]   mon	月
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void SetHorHeader(int year, int mon);
public slots:
	
	///
	///  @brief
	///
	///
	///  @param[in]   ml  更新维护日志
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月18日，新建函数
	///
	void UpdateMaintianLog(tf::MaintainLog ml);

	///
	/// @brief
	///     维护组阶段更新
	///
	/// @param[in]  strDevSn        设备序列号
	/// @param[in]  lGrpId          维护组ID
	/// @param[in]  enPhaseType     阶段类型
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2024年2月23日，新建函数
	///
	void OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType);

private slots:

	///
	///  @brief 设备单选框被点击
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月13日，新建函数
	///
	void OnDevRBtnClicked();

	///
	///  @brief	选择年份下拉列表
	///
	///
	///  @param[in]   value  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnComboBoxYearCurrentIndexChanged(QString value);
	
	///
	///  @brief	选择月份下拉列表
	///
	///
	///  @param[in]   value  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnComboBoxMonCurrentIndexChanged(int value);
	
	///
	///  @brief	上一月按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnLastMonBtnClicked();
	
	///
	///  @brief 下一月按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnNextMonBtnClicked();
	
	///
	///  @brief 打印按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnPrintBtnClicked();
	
	///
	///  @brief	导出按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
	///
	void OnOutputBtnClicked();
	
	///
	///  @brief 关闭按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年12月6日，新建函数
	///
	void OnCloseBtnClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月7日，新建函数
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
Q_SIGNALS:
    void hint(QString strInfo);         // 提示信息

private:
	
	///
	///  @brief 创建打印导出数据
	///
	///
	///  @param[in]   vecInfo  数据容器
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
	///
	void CreatePrintData(MaintenanceLog& vecInfo);

	///
	///  @brief 删除系统维护日志
	///
	///
	///  @param[in]   vecLogs  日志列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void RemoveSystemLog(std::vector<tf::MaintainLog> &vecLogs);

	///
	/// @brief 初始化界面数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
	///
	void InitUIData();

	///
	/// @brief 通过界面日期更新UI
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
	///
	void UpdateUIByDate();

	///
	/// @brief 通过年份更新日期
	///
	/// @param[in]  year  年份
	/// @param[in]  oldSelectedMon  之前选择的日期
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
	///
	void UpdateMonthCombxByYear(const int& year, const int& oldSelectedMon =1);

	///
	/// @brief 通过日期更新按钮状态
	///
	/// @param[in]  year  年份
	/// @param[in]  month  日期
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
	///
	void UpdateBtnStatusByDate(const int& year, const int& month);

private:
	Ui::QMaintainLog* ui;
	MaintainLogModel*						m_pMaintainLogModel;		// 维护日志模型
	std::map<std::string, std::string>		m_devNameSNMap;
	std::string                             m_curDeviceSN;
};

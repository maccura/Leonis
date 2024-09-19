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
/// @file     DetectionSetDlg.h
/// @brief    检测设置弹窗
///
/// @author   7951/LuoXin
/// @date     2022年12月08日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月08日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/public/ConfigDefine.h"

namespace Ui {
    class DetectionSetDlg;
};

class DetectionSetDlg : public BaseDlg
{
	Q_OBJECT

public:
	DetectionSetDlg(QWidget *parent = Q_NULLPTR);
	~DetectionSetDlg();

	///
	/// @brief	加载数据到控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月08日，新建函数
	///
	void LoadDataToCtrls();

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月08日，新建函数
	///
	void InitCtrls();

protected Q_SLOTS:
	///
	/// @brief	保存按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年12月08日，新建函数
	///
	void OnSaveBtnClicked();

    ///
    /// @brief
    ///     表格单元格被点击
    ///
    /// @param[in]  index  点击单元格索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月26日，新建函数
    ///
    void OnDetectTableItemClicked(const QModelIndex& index);

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月27日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:
	///
	/// @brief 是否允许修改参数
	///
	/// @param[in]   
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月14日，新建函数
	///
	bool EnableModifyConfig();

    ///
    /// @brief
    ///     是否允许改变检测模式
    ///
    /// @param[in]   index  检测模式表格索引
    ///
    /// @return true表示允许
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月3日，新建函数
    ///
    bool EnableChangeDetectMode(const QModelIndex& index);

	///
	/// @brief 添加进样模式操作日志，日志需要组装，所以提函数
	///
	/// @param[in]  oldSet       之前的设置
	/// @param[in]  uiSet		 现在的设置
	/// @param[in]  strModeName  模式的名称
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年4月1日，新建函数
	///
	void AddEnterSampleModeOptLog(const DetectionSetting& oldSet, const DetectionSetting& uiSet, const QString& strModeName);
	
private:
    // 需求计算列表，表头枚举
    enum DetectModeHeader {
        Dmh_Device = 0,             // 仪器
        Dmh_RoutineMode,            // 常规模式
        Dmh_QuickMode,              // 快速模式
        Dmh_StatMode,               // STAT模式
    };

private:
	Ui::DetectionSetDlg*           ui;
	DetectionSetting               m_detectionSetting;	    // 当前设置信息，用于比较用户的修改
    std::map<std::string, int>     m_mapDevDetectMode;      // 设备检测模式（设备序列号-检测模式）, 参见im::tf::DetectMode::type
    const int                      m_ciTblDefaultRowCnt;    // 表格行数
    const int                      m_ciTblDefaultColCnt;    // 表格列数

	std::map<int, QString>		   m_mapImDetectModeName;		// 检测模式的文本信息(免疫) key是im::tf::DetectMode::type类型  Add by Chenjianlin 20240401
};

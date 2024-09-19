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
/// @file     SpecialWashDlg.h
/// @brief    交叉污染配置
///
/// @author   7951/LuoXin
/// @date     2022年9月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "QWidget"
#include <set>

class QItemSelection;
class QStandardItemModel;
class AddWashCupDlg;
class AddWashReagentProbeDlg;
class AddWashSampleProbeDlg;
class AddWashImSampleProbeDlg;

namespace Ui {
    class SpecialWashDlg;
};


class SpecialWashDlg : public QWidget
{
	Q_OBJECT

public:
	SpecialWashDlg(QWidget *parent = Q_NULLPTR);
	~SpecialWashDlg();

	///
	/// @brief	加载显示数据到界面
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	void LoadDataToDlg();

protected:
	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	void showEvent(QShowEvent *event);
	bool eventFilter(QObject *obj, QEvent *event) override;
private:
	///
	/// @brief
	///     初始化控件
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	void InitCtrls();

	///
	/// @brief
	///     检查用户输入的数据是否合法
	///
	/// @param[in]  AddNewFalg  是否为新增项目
	/// @return		数据合法返回true;
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	bool CheckUserData(bool AddNewFalg);

	///
	/// @brief	加载试剂针清洗配置信息到表格
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月12日，新建函数
	///
	void LoadReagentDataToTableview();

	///
	/// @brief	加载样本针清洗配置信息
	/// @par History:
	/// @li 7951/LuoXin，2023年3月21日，新建函数
	///
	void LoadSampleProbeWashConfig();

	///
	/// @brief	加载样本针清洗配置信息
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void LoadImSampleProbeWashConfig();

	///
	/// @brief	加载样本针清洗配置信息到表格
    ///       
	/// @par History:
	/// @li 7951/LuoXin，2022年9月12日，新建函数
	///
	void LoadSampleDataToTableview(bool pageChanged = true);

	///
	/// @brief	加载样本针清洗配置信息到表格(免疫)
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	void LoadImSampleDataToTableview(const bool pageChanged = true);

    ///
    /// @brief
    ///     获取免疫的通道号-吸样量 map
    ///
    /// @param[in]   vecAsssyCode,通道号集合
    /// @param[out]   mapAssaycodeSuckVol,通道号-吸样量 map
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年10月27日，新建函数
    bool GetImSuckVolMap(std::set<int> vecAsssyCode, std::map<int, int> & mapAssaycodeSuckVol);

	///
	/// @brief	加载反应杯清洗配置信息到表格
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月12日，新建函数
	///
	void LoadCupDataToTableview();

	///
	/// @brief	保存生化样本针清洗配置信息
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void SaveChSampleProbeWashConfig();

	///
	/// @brief	保存免疫样本针清洗配置信息
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void SaveImSampleProbeWashConfig();

	///
	/// @brief 检查是否允许保存免疫样本针清洗配置信息
	///
	///
	/// @return ture:允许保存 false:不允许保存
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年10月26日，新建函数
	///
	bool EnableSaveImSampleProbeWashConfig();

protected Q_SLOTS:
	///
	/// @brief	新增按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月13日，新建函数
	///
	void OnAddBtnClicked();

	///
	/// @brief	编辑按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月13日，新建函数
	///
	void OnModifyBtnClicked();

	///
	/// @brief	删除按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月13日，新建函数
	///
	void OnDelBtnClicked();

	///
	/// @brief	表格的选中行改变
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年2月17日，新建函数
	///
	void OnTableViewRowChanged(const QItemSelection& selected, const QItemSelection& deselected);

	///
	/// @brief	保存表格列的选择状态
	///     
	/// @param[in]  index  索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月17日，新建函数
	///
	void OnSaveTableViewSelect(const QModelIndex& index);
	
	///
	///  @brief 样本针界面切换到生化界面
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	void OnShowByCh();
	
	///
	///  @brief 样本针界面切换到免疫界面
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	void OnShowByIm();

	///
	/// @brief 保存样本针清洗配置信息
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年3月21日，新建函数
	///
	void OnSaveBtnClicked();

    ///
    /// @brief  更新控间状态
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月11日，新建函数
    ///
    void OnUpdateCtrls();

private:
	Ui::SpecialWashDlg*								ui;
	AddWashCupDlg*									m_addWashCupDlg;			// 新增/编辑反应杯特殊清洗配置弹窗
	AddWashReagentProbeDlg*							m_addWashReagentProbeDlg;	// 新增/编辑试剂针特殊清洗配置弹窗
	AddWashSampleProbeDlg*							m_addWashSampleProbeDlg;	// 新增/编辑样本针特殊清洗配置弹窗
	AddWashImSampleProbeDlg*						m_addWashImSampleProbeDlg;	// 新增/编辑样本针特殊清洗配置弹窗(免疫)
	QStandardItemModel*								m_reagentProbeMode;			// 试剂针清洗配置列表的model
	QStandardItemModel*								m_sampleProbeMode;			// 样本针清洗配置列表的model
	QStandardItemModel*								m_imSampleProbeMode;		// 样本针清洗配置列表的(免疫)model
	QStandardItemModel*								m_cupMode;					// 反应杯清洗配置列表的model
	std::vector<QString>							m_vecReagents;				// 反应杯清洗所用的试剂针名称列表
	QWidget*										m_cornerItem;				// 右上角小控件（生化、免疫 RadioButton）
};

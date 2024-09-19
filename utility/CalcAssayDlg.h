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
/// @file     CalcAssayDlg.h
/// @brief    应用--系统--计算项目
///
/// @author   7951/LuoXin
/// @date     2022年10月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年10月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "src/thrift/gen-cpp/defs_types.h"

class QStandardItemModel;
class RangParaSetWidget;
class CRangParaSetWidget;
class CommonInformationManager;
namespace Ui {
    class CalcAssayDlg;
};

class QItemSelection;


class CalcAssayDlg : public QWidget
{
    Q_OBJECT

public:
    CalcAssayDlg(QWidget *parent = Q_NULLPTR);
    ~CalcAssayDlg();

    ///
    /// @brief	加载数据到控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月25日，新建函数
    ///
    void LoadDataToCtrls();

    protected Q_SLOTS:
    ///
    /// @brief	保存按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月25日，新建函数
    ///
    void OnSaveBtnClicked();

    ///
    /// @brief	计算器按钮槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月25日，新建函数
    ///
    void OnCalculatorBtnClicked();

    ///
    /// @brief	新增按钮槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    void OnAddBtnClicked();

    ///
    /// @brief	删除按钮槽函数
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月4日，新建函数
    ///
    void OnDeleteBtnClicked();

    ///
    /// @brief	新增参考区间按钮槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年12月28日，新建函数
    ///
    void OnAddRefBtnClicked();

    ///
    /// @brief	修改参考区间按钮槽函数
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月28日，新建函数
    ///
    void OnModifyRefBtnClicked();

    ///
    /// @brief	删除参考区间按钮槽函数
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月12日，新建函数
    ///
    void OnDeleteRefBtnClicked();

    ///
    /// @brief	计算项目表格选中行改变槽函数
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    void OnSelectRowChanged(const QItemSelection &selected, const QItemSelection &deselected);

    ///
    /// @brief	项目按钮当前页改变
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年12月27日，新建函数
    ///
    void OnCurrentPageChange();

    ///
    /// @brief	更新参考范围表
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年12月29日，新建函数
    ///
    void OnUpdateRefRange();

    ///
    /// @bref
    ///		项目列表的选择变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月12日，新建函数
    ///
    void OnTabelSelectChanged(const QItemSelection &selected, const QItemSelection &deselected);

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月27日，新建函数
    ///
    void UpdateCtrlsEnabled();

    ///
    /// @bref
    ///		列宽变化槽函数
    ///
    /// @param[in] column 列索引
    /// @param[in] oldWidth 旧宽度
    /// @param[in] newWidth 新宽度
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月22日，新建函数
    ///
    void OnColumnResized(int column, int oldWidth, int newWidth);

signals:
    void optCompleted(int code);

private:

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月11日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief	初始化控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月25日，新建函数
    ///
    void InitCtrls();

    ///
    /// @brief	清理控件中的数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    void ClearCtrls();
    void DisableAllCtrls(bool bDisable);

    ///
    /// @brief	加载普通项目信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年12月27日，新建函数
    ///
    void LoadAssayData();
    void LoadAssayDataChim(std::vector<std::pair<int, QString>>& assayData);
    void LoadAssayDataImmy(std::vector<std::pair<int, QString>>& assayData);

    ///
    /// @brief	显示项目选择按钮文本
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年12月27日，新建函数
    ///
    void ShowAssayNameToCtrls();

    ///
    /// @brief	加载计算项目信息到控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    void LoadCalcAssayToTableview();

    ///
    /// @brief	显示公式
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月2日，新建函数
    ///
    void ShowFormula();

    ///
    /// @brief	解析公式
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    void DecodeFormula(QString formula);

    ///
    /// @brief	检查用户输入数据是否合法
    /// 
    ///@return 数据合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月29日，新建函数
    ///
    bool CheckUserInputData();

    ///
    /// @brief	获取参考区间
    ///  
    /// @param[in]   row		 行号  
    /// @param[out]  referrence	 返回的界面值
    ///
    /// @return		referrence  参考区间
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月29日，新建函数
    ///
    void GetReferrenceByRow(int row, ::tf::AssayReferenceItem& referrence);

    ///
    /// @brief	显示参考区间
    ///     
    /// @param[in]  row   行号
    /// @param[in]  item  参考区间
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月29日，新建函数
    ///
    void ShowReferrence(int row, const ::tf::AssayReferenceItem& item);

    ///
    /// @bref
    ///		从界面获取计算项目参数设置
    ///
    /// @param[out] cai 参数设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月12日，新建函数
    ///
    bool TakeCalcAssayInfo(::tf::CalcAssayInfo& cai);

    ///
    /// @bref
    ///		保存计算项目设置（成功返回assayCode，else -1
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月12日，新建函数
    ///
    void ProcSaveRef(bool bRefChange);

    ///
    /// @brief  删除指定的就算项目
    ///     
    ///
    /// @param[in]  calcIdAndCode 待删除的计算项目的id和项目编号 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年7月26日，新建函数
    ///
    void deleteCalcAssay(const std::map<int, int>& calcIdAndCode);

	///
	/// @brief 计算项目是否发生变化（当前只判断计算项目信息，不包含参考区间）
	///
	/// @param[in]  calci 计算项目信息
	///
	/// @return true存在变化，false无变化
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年9月6日，新建函数
	///
	bool IsChangedOfCalcAssay(const ::tf::CalcAssayInfo& calci);
signals:
    void saveCompleted(int code);
    void deleteCompleted();

private:
    Ui::CalcAssayDlg*									ui;
    std::vector<QString>								m_vecFormula;			// 计算器输入信息列表
    std::vector<std::pair<int, QString>>				m_vecAssayInfo;			// 项目编号--项目名称映射
    QStandardItemModel*									m_calcMode;				// 计算项目列表的model
    QStandardItemModel*									m_refMode;				// 参考区间列表的model
    CRangParaSetWidget*								    m_rangParaSetWidget;	// 参考区间修改弹窗
    std::shared_ptr<CommonInformationManager>           m_assayMgr;
    int                                                 m_currentSampleSource;  // 当前的样本源
    std::string                                         m_lastAssayName;        // 上次选中的项目名称
};

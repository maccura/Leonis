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
/// @file     QCaliBrateResult.cpp
/// @brief    校准结果
///
/// @author   5774/WuHongTao
/// @date     2022年7月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "curveDraw/common.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QCaliBrateResult;
};
class QStandardItemModel;
class QTableView;
class CaliResultInfoCh;

class QCaliBrateResult : public BaseDlg
{
	Q_OBJECT

public:
	QCaliBrateResult(QWidget *parent = Q_NULLPTR);
	~QCaliBrateResult();

	///
	/// @brief 启动校准详情对话框
	///
	/// @param[in]  reagent  实际详细信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月15日，新建函数
	///
	void StartDialog(ch::tf::ReagentGroup& reagent);

	///
	/// @brief 获取单例
	///
	///
	/// @return 单例句柄
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	static QCaliBrateResult& GetInstance();

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void Init();

	///
	/// @brief 获取试剂的当前校准曲线
	///
	/// @param[in]  reagent  试剂信息
	///
	/// @return 校准曲线
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月15日，新建函数
	///
	std::shared_ptr<ch::tf::CaliCurve> GetCurrentCurve(ch::tf::ReagentGroup& reagent);

	///
	/// @brief 获取试剂的历史校准曲线
	///
	/// @param[in]  reagent  试剂信息
	///
	/// @return 校准曲线列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月15日，新建函数
	///
	std::vector<std::shared_ptr<ch::tf::CaliCurve>> GetHistoryCurves(ch::tf::ReagentGroup& reagent);

	///
	/// @brief 设置历史曲线
	///
	/// @param[in]  caliCurves  历史曲线组
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void SetHistoryCurve(std::vector<std::shared_ptr<ch::tf::CaliCurve>>& caliCurves);

	///
	/// @brief 根据参数获取显示曲线
	///
	/// @param[in]  curveData  曲线参数
	/// @param[in]  curveAlg   算法曲线相关参数
	/// @param[in]  curves     曲线可绘制参数
	///
	/// @return		成功则true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	bool ShowCurve(std::shared_ptr<ch::tf::CaliCurve>& curveData, std::shared_ptr<ch::ICalibrateCurve>& curveAlg, std::vector<curveSingleDataStore>& curves);

	///
	/// @brief 设置校准状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void InitCaliStatus();

	///
	/// @brief 根据试剂和校准曲线信息获取校准状态
	///
	/// @param[in]  reagent  试剂信息
	/// @param[in]  curve    曲线状态
	///
	/// @return 校准状态信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	boost::optional<QStringList> GetCaliStatusShow(ch::tf::ReagentGroup& reagent, std::shared_ptr<ch::tf::CaliCurve>& curve);

	///
	/// @brief 显示当前曲线
	///
	/// @param[in]  reagent		试剂信息
	/// @param[in]  curveStore  曲线的绘制信息
	///
	/// @return 曲线的数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	std::shared_ptr<ch::tf::CaliCurve> SetCurrentCurve(ch::tf::ReagentGroup& reagent, std::vector<curveSingleDataStore>& curveStore);

    ///
    /// @brief 设置表格文字大小
    ///
    /// @param[in]  table  表格
    /// @param[in]  fontSize  文字大小
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年11月29日，新建函数
    ///
    void SetTableViewFontSize(QTableView* table, int fontSize = 14);

private slots:
	///
	/// @brief 历史曲线的选择槽函数
	///
	/// @param[in]    历史曲线index
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void OnHistoryCurve(int);

	///
	/// @brief 更新工作曲线
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void OnUpdateWorkCurve();

	///
	/// @brief 打印按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	void OnPrintBtnClicked();

	///
	/// @brief 导出按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	void OnExportBtnClicked();

private:

	///
	/// @brief 设置校准信息（导出使用）
	///
	/// @param[in] strExportTextList   导出的数据
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	bool SetCaliInfo(QStringList& strExportTextList);

    ///
    /// @brief 设置打印导出是否可用
    ///
    /// @param[in]  hasCurve  是否存在可用曲线
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月22日，新建函数
    ///
    void SetPrintAndExportEnabled(bool hasCurve);

    ///
    /// @brief 获取打印数据
    ///
    /// @param[in]  info  校准记录
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月12日，新建函数
    ///
    bool GetPrintExportInfo(CaliResultInfoCh& info);

private:
	Ui::QCaliBrateResult*			ui;
	QStandardItemModel*             m_calibrate;				///< 校准品浓度mode
	QStandardItemModel*             m_parameter;				///< 校准曲线参数vmod
    std::vector<std::shared_ptr<ch::tf::CaliCurve>> m_historyCurve;	///< 历史曲线列表
    std::shared_ptr<ch::tf::CaliCurve> m_curCurve;                  ///< 当前校准曲线
	std::vector<curveSingleDataStore> m_currentCurves;
	std::vector<curveSingleDataStore> m_historyCurvesStore;
	std::map<int, QLabel*>			m_labelMap;					///< 显示校准详细信息的映射关系
	ch::tf::ReagentGroup			m_reagent;					///< 当前试剂
	int								m_currentIndex;				///< 历史曲线的当前选中号
};

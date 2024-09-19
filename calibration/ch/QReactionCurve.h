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
/// @file     QReactionCurve.h
/// @brief    曲线
///
/// @author   5774/WuHongTao
/// @date     2022年3月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include "shared/basedlg.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class QStandardItemModel;
class QComboBox;
class QReactionCurveCtrl;
class ReactCurveData;

namespace Ui {
    class QReactionCurve;
};

class QReactionCurve : public BaseDlg
{
    Q_OBJECT

    using AssayRetmap = std::map<std::int64_t, std::vector<ch::tf::AssayTestResult>>;
public:
	QReactionCurve(QWidget *parent = Q_NULLPTR);
    ~QReactionCurve();

    ///
    /// @brief
    ///     显示反应曲线信息
    ///
    /// @param[in]  caliCurve  反应曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月28日，新建函数
    ///
    void ShowReactionCurve(const ch::tf::CaliCurve& caliCurve);

    ///
    /// @brief
    ///     显示曲线列表
    ///
    /// @param[in]  curves  曲线列表
    /// @param[in]  type    页面类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月4日，新建函数
    ///
    void ShowCurveList(std::shared_ptr<ch::tf::CaliCurve> curve);

private:

    ///
    /// @brief
    ///     根据测试结果设置选择框
    ///
    /// @param[in]  CalibrateResult  测试结果
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月3日，新建函数
    ///
    void SetComBoxContent(AssayRetmap& CalibrateResult);

	///
	/// @brief 初始化页面内容
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月8日，新建函数
	///
	void InitPageInfos();

	///
	/// @brief 初始化顶部标签
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月8日，新建函数
	///
	void InitLabels();

	///
	/// @brief 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月8日，新建函数
	///
	void InitPlot();

private slots:

	///
	/// @brief 清空测光点的表格内容
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void ClearViewContent();

	///
	/// @brief
	///     显示曲线的检测点
	///
	/// @param[in]  result  曲线检测点
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月2日，新建函数
	///
	void ShowDetectPointView(ch::tf::AssayTestResult& result);

	///
	/// @brief 设置曲线对应的校准品信息
	///
	/// @param[in]  result  项目测试结果
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月12日，新建函数
	///
	void SetCalibrateInfos(const ch::tf::CaliCurve & curve);

	///
	/// @brief 校准品曲线变化
	///
	/// @param[in]  index  发生变化项目的索引
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月13日，新建函数
	///
	void SelectNewCaliCurve(int index);

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
	/// @brief 设置反应曲线（导出使用）
	///
	/// @param[in] strExportTextList   导出的数据
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	bool SetReactionCurveInfo(QStringList& strExportTextList);

	///
	/// @brief 设置曲线校准信息（导出使用）
	///
	/// @param[in] strExportTextList   导出的数据
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	bool SetCurveCaliInfo(QStringList& strExportTextList);

	///
	/// @brief 获取反应曲线打印数据（打印使用）
	///
	/// @param[in] rceactionCurveInfo   反应曲线打印数据
	///
	/// @return true:获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
    bool GetReactionCurvePrintData(ReactCurveData& rceactionCurveInfo);

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

private:
    Ui::QReactionCurve*             ui;
	AssayRetmap						m_CalibrateResult;          ///< 校准测试结果（水平--校准结果（顺序代表初次或者复查））
    int                             m_PrimCurveId;              ///< 主曲线
    int                             m_SecCurveId;               ///< 次曲线
    int                             m_PrimSubSecSecCurveId;     ///< 主次相减曲线
    QStandardItemModel*             m_CurvePointMode;           ///< 曲线的主次波长点
    ch::tf::CaliCurve               m_CurveData;                ///< 曲线数据
    QReactionCurveCtrl*             m_curvePlot;                ///< 曲线的画板
};

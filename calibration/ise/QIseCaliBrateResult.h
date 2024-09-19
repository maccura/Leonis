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
/// @file     QIseCaliBrateResult.h
/// @brief    校准结果
///
/// @author   8090/YeHuaNing
/// @date     2023年4月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "shared/basedlg.h"

class QStandardItemModel;
class CaliResultInfoIse;
namespace Ui {
    class QIseCaliBrateResult;
};


namespace ise
{
    namespace tf
    {
        class IseModuleInfo;
        class CaliCurve;
    };
};

class QIseCaliBrateResult : public BaseDlg
{
	Q_OBJECT

public:
	QIseCaliBrateResult(QWidget *parent = Q_NULLPTR);
	~QIseCaliBrateResult();

	///
	/// @brief 启动校准详情对话框
	///
	/// @param[in]  module  模块信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
	///
	void StartDialog(ise::tf::IseModuleInfo& module);

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
	///
	void Init();

	///
	/// @brief 获取当前校准曲线
	///
	/// @param[in]  module 模块信息 
	///
	/// @return 当前校准曲线
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
	///
	std::shared_ptr<ise::tf::CaliCurve> GetCurrentCurve();

    ///
    /// @brief 初始化当前结果模型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void InitCurCurveModel();

    ///
    /// @brief 初始化历史结果模型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void InitHisCurveModel();

    ///
    /// @brief 初始化校准时间模型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void InitCaliTimesModel();

	///
	/// @brief 获取历史校准曲线信息
	///
	/// @param[in]  module  模块信息
	///
	/// @return 历史校准曲线
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
	///
	void GetHistoryCurves();

	///
	/// @brief 设置校准状态
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
	///
	void InitCaliStatus();

    ///
    /// @brief 初始化表格数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void InitTableModel();

    ///
    /// @brief 根据模块信息和校准曲线信息获取校准状态
    ///
    /// @param[in]  module  模块信息
    /// @param[in]  curve  曲线信息
    ///
    /// @return 校准状态信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    QStringList GetCaliStatusShow(ise::tf::CaliCurve& curve);

    ///
    /// @brief 获取曲线的主要数据（表格内容）
    ///
    /// @param[in]  curve  曲线信息
    ///
    /// @return 返回结果
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    QStringList GetCurveData(ise::tf::CaliCurve& curve);

    ///
    /// @brief 获取核心数据
    ///
    /// @param[in]  core  核心数据来源
    /// @param[in]  assayCode  项目编号
    /// @param[in]  curve  曲线数据
    ///
    /// @return 核心数据记录
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    QStringList GetCoreData(ise::tf::CaliDataCore core, int assayCode, const ise::tf::CaliCurve& curve);

    ///
    /// @brief 获取项目对应的报警
    ///
    /// @param[in]  assayCode  项目编号
    /// @param[in]  curve  曲线数据
    ///
    /// @return 报警信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月22日，新建函数
    ///
    QString GetAlarmInfo(int assayCode, const ise::tf::CaliCurve& curve);

    ///
    /// @brief 显示校准曲线的状态信息
    ///
    /// @param[in]  data  状态信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void ShowCaliCurveStatus(const QStringList& data);

    ///
    /// @brief 显示校准曲线核心数据
    ///
    /// @param[in]  model  表格模型
    /// @param[in]  data  数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void ShowCurveCoreData(QStandardItemModel* model, const QStringList& data);

    ///
    /// @brief 设置校准时间模型数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月11日，新建函数
    ///
    void SetCaliTimesModelData();

private slots:

	///
	/// @brief 更新工作曲线
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年4月11日，新建函数
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

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

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
    bool GetPrintExportInfo(CaliResultInfoIse& Info);

private:
	Ui::QIseCaliBrateResult*		ui;
	QStandardItemModel*             m_curModel;				    ///< 当前曲线mode
	QStandardItemModel*             m_hisModel;				    ///< 历史曲线参mod
	std::vector<ise::tf::CaliCurve> m_historyCurve;	            ///< 历史曲线列表
	std::map<int, QLabel*>			m_labelMap;					///< 显示校准详细信息的映射关系
	ise::tf::IseModuleInfo			m_module;					///< 当前电解质模块
	int								m_currentIndex;				///< 历史曲线的当前选中号
    std::shared_ptr<ise::tf::CaliCurve> m_curCurve;             ///< 当前曲线数据
};

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
/// @file     analysisparamwidget.h
/// @brief    应用界面->项目设置界面->分析参数界面
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "model/AssayListModel.h"
#include <QWidget>
#include <QMap>
#include <memory>
#include <set>

class QLineEdit;
class QComboBox;

namespace Ui
{
    class AnalysisParamWidget;
};

namespace tf
{
    class GeneralAssayInfo;
};

namespace ch
{
    namespace tf
    {
        class SpecialAssayInfo;
        class GeneralAssayInfo;
        class AnalyCheckParam;
        class SampleAspirateVol;
    }
};

using namespace std;

class AnalysisParamWidget : public QWidget
{
    Q_OBJECT

public:
    AnalysisParamWidget(QWidget *parent = Q_NULLPTR);
    ~AnalysisParamWidget();

    ///
    /// @brief
    ///     加载指定项目的分析参数
    ///
    /// @param[in]  item  数据
    /// @param[in]  updateSampleType  是否刷新样本类型下拉框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item, bool updateSampleType = true);

    ///
    /// @brief  获取编辑的项目分析参数
    ///     
    ///
    /// @param[in]  GAI         通用项目参数
    /// @param[in]  chGAI       生化项目参数
    /// @param[in]  chSAI       生化特殊项目参数
    /// @param[in]  assayCode   项目编号
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool GetAnalysisParam(tf::GeneralAssayInfo& GAI, ch::tf::GeneralAssayInfo& chGAI, ch::tf::SpecialAssayInfo& chSAI, int assayCode);

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月11日，新建函数
    ///
    bool HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief 获取当前项目的样本类型列表
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月23日，新建函数
    ///
    QVector<int> GetCurAssaySampleSource();

    ///
    /// @brief 更新优先样本类型
    ///
    /// @param[in]  assayCode       项目编号
    /// @param[in]  deviceType      设备类型
    /// @param[in]  version         版本号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月27日，新建函数
    ///
    void UpdateFirstSampleTypeCommbox(int assayCode, int deviceType, const string& version);

    ///
    /// @brief 更新优先样本类型
    ///
    /// @param[in]  oldVersion      修改前的版本号
    /// @param[in]  newVersion      修改后的版本号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月29日，新建函数
    ///
    void UpdateVersionCommbox(int oldVersion, int newVersion);

signals:
    // 样本量改变
    void SampleVolChanged(double vol);

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitChildCtrl();

	///
	/// @brief	为界面的输入框设置正则表达式
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年10月24日，新建函数
	///
	void SetCtrlsRegExp();

    ///
    /// @brief	检查用户输入参数是否合法
    ///
    /// @return 合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年08月23日，新建函数
    ///
    bool CheckUserInputData(int code);

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief
	///     复位控件
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月18日，新建函数
	///
    void Reset(bool updateSampleType = true);

	///
	/// @brief 加载双向同测配置
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年7月20日，新建函数
	///
	void LoadCombTwinsTest(std::shared_ptr<::ch::tf::SpecialAssayInfo> chSAI);

	///
	/// @brief	更新控件的显示和编辑权限
	///    
    /// @param[in]  isOpenAssay 是否为开放项目  
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月20日，新建函数
	///
	void UpdateCtrlsEnabled(bool isOpenAssay);

    ///
    /// @brief  检查四个测光点设置的值是否合法
    ///     
    ///
    /// @param[in]  m  
    /// @param[in]  n  
    /// @param[in]  p  
    /// @param[in]  q  
    ///
    /// @return 合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月10日，新建函数
    ///
    bool CheckMeteringPointIsValid(int m, int n, int p, int q);

    ///
    /// @brief
    ///     检查样本量设置是否合法
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月07日，新建函数
    ///
    bool CheckSampleVolIsValid(QLineEdit* edit);

    ///
    /// @brief
    ///     获取界面的结果检查范围
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月07日，新建函数
    ///
    ch::tf::AnalyCheckParam GetAnalyChkParam();

    ///
    /// @brief
    ///     获取界面的样本量
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月07日，新建函数
    ///
    std::vector<ch::tf::SampleAspirateVol> GetSampleAspirateVols();

	protected Q_SLOTS:
	///
	/// @brief
	///     当前选中样本源改变
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月08日，新建函数
	///
	void OnSampleSourceChanged(const QString &text);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2023年10月31日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		新增项目
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年2月22日，新建函数
    ///
    void OnAddAssay(QString name, QString unit, int code, QString version, QSet<int> sampleSources);

    ///
    /// @bref
    ///		刷新样本类型到下来框
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年2月27日，新建函数
    ///
    void UpdateSampleTypeToCommbox(QComboBox* box, const std::set<int>& sampleTypes);

private:
    Ui::AnalysisParamWidget			*ui;                        /// UI指针
    bool							m_bInit;                    /// 是否已经初始化
	bool							m_bSampleTypeFlag;			/// 样本源类型下拉框是否可用
    bool							m_bVersionFlag;			    /// 参数版本下拉框是否可用

    // 设置友元类
    friend class					AssayConfigWidget;          /// 将仪器设置窗口设置为友元类

    vector<tuple<QLineEdit *, QLineEdit *, QLineEdit *, QComboBox *>>               m_sampCtrlsp;      //样本量控件列表
    vector<tuple<QLineEdit *, QLineEdit *, QComboBox *, QComboBox *>>               m_reagentCtrls;    // 试剂量控件列表
    vector<tuple<vector<QComboBox *>, vector<QLineEdit *>>>                         m_checkParamsCtrls;// 吸光度检查控件列表

};

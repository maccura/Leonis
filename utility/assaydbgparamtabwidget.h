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
/// @file     assaydbgparamtabwidget.h
/// @brief    应用界面->项目设置界面->项目参数tab页面
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

#include <QTabWidget>
#include <memory>
#include "src/thrift/im/gen-cpp/im_types.h"

class DbgParamPrehandleWidget;
class DbgParamAnalysisWidget;
class DbgParamFittingWidget;
class DbgParamCaliWidget;

namespace utilcomm {
    class CaliCurvParam;
    class CaliCurvCache;
};

class AssayDbgParamTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    AssayDbgParamTabWidget(QWidget *parent);
    ~AssayDbgParamTabWidget();

    ///
    /// @brief
    ///     页签类型
    ///
    enum TabType{ TabPreHandle, TabAnalysis, TabFitting, TabCali}; 

    ///
    /// @brief
    ///     页签信息
    ///
    struct TabItem
    {
        TabItem(TabType tabType, QWidget* widget)
            : m_tbType(tabType), m_tabWidget(widget)
        {}
        TabType m_tbType;
        QWidget* m_tabWidget;

        inline QString GetName() const;
    };

    ///
    /// @bref
    ///	    切换项目，各个界面加载当前点击的项目信息
    ///
    /// @param[in] assayCode 项目ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void LoadAssayInfo(int assayCode);

    ///
    /// @bref
    ///		保存各个界面的项目信息变更
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void SaveAssayInfo();

protected:

    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

	protected Q_SLOTS:
	///
	/// @brief	页面切换的槽函数
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年3月16日，新建函数
	///
	void OnCurrentChanged(int index);

    ///
    /// @bref
    ///		目标校准曲线的批号调整
    ///
    /// @param[in] strLot 批号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月8日，新建函数
    ///
    void CaliLotChanged(const QString& strLot);

    ///
    /// @bref
    ///		检查界面参数是否存在修改
    ///
    /// @param[in] uiParam 界面参数
    /// @param[in] origCaliCurv 原始参数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月11日，新建函数
    ///
    bool IsCaliCurvChanged(const utilcomm::CaliCurvParam& uiParam, const utilcomm::CaliCurvParam& origCaliCurv);

private:
    bool							m_bInit;					/// 是否已经初始化
    DbgParamPrehandleWidget         *m_pPrehandleWidget;		/// 预处理
    DbgParamAnalysisWidget			*m_pAnalysisWidget;	        /// 参数
    DbgParamFittingWidget			*m_pFittingWidget;	        /// 拟合
    DbgParamCaliWidget				*m_pCaliWidget;		        /// 校准


    bool m_existPrehandleAssay;                                 /// 是否存在预处理试剂
    std::shared_ptr<utilcomm::CaliCurvParam> m_origCurvParam;   /// 保存当前解析过的参数
    std::shared_ptr<::im::tf::CaliCurve> m_origCurv;            /// 保存当前曲线

    std::shared_ptr<utilcomm::CaliCurvCache> m_caliCurvCache;   /// 校准曲线缓存

    std::vector<TabItem>            m_tabInfo;                  /// 暂存Tab页签信息
};

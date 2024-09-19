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
/// @file     QHistoryFilterSample.h
/// @brief    筛选对话框
///
/// @author   5774/WuHongTao
/// @date     2022年5月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigDefine.h"

class QHistoryFilterSampleImp;
class CustomTabWidget;
namespace Ui {
    class QHistoryFilterSample;
};


class QHistoryFilterSample : public BaseDlg
{
	Q_OBJECT

public:
	QHistoryFilterSample(QWidget *parent = Q_NULLPTR);
	~QHistoryFilterSample();


	/// @brief 
	///
	/// @param[in]  qryCond  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月14日，新建函数
	///
	bool GetQueryConditon(::tf::HistoryBaseDataQueryCond& qryCond);

	///
	/// @brief 获取快捷筛选条件
	///
	/// @param[in]  vecQryCond  快捷筛选条件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月14日，新建函数
	///
	bool GetFastFilterQueryConditon(std::vector<::tf::HistoryBaseDataQueryCond>& vecQryCond);

	///
	/// @brief 更新工作页面的按钮状态
	///
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月5日，新建函数
	///
	void UpdateButtonStatus(const PageSet& workSet);
signals:
	void finished();
	void fastFilterFinished(const ::tf::HistoryBaseDataQueryCond& qryCond);

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void Init();

	///
	/// @brief 初始化查询条件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月19日，新建函数
	///
	void InitCond();

private slots:
	///
	/// @brief 保存筛选参数
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void OnSaveParameter();

	///
	/// @brief 保存快捷筛选条件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月5日，新建函数
	///
	bool OnSaveFastFilter();

	///
	/// @brief 重置按钮槽函数
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月14日，新建函数
	///
	void OnReset();

    ///
    /// @bref
    ///		切换页签事件
    ///
    /// @param[in] index 当前页签编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月22日，新建函数
    ///
    void OnCurrentChanged(int index);

    ///
    /// @bref
    ///		保存特定页签的参数
    ///
    /// @param[in] targetPage 被指定的页签
    /// @param[out] qryCond 被保存的结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月27日，新建函数
    ///
    bool SavePageParameter(QHistoryFilterSampleImp* targetPage, ::tf::HistoryBaseDataQueryCond& qryCond);

protected:
	void showEvent(QShowEvent *event) override;


    void keyPressEvent(QKeyEvent* event);

private:
	Ui::QHistoryFilterSample*				ui;
	std::map<int, std::vector<::tf::DeviceInfo>>	m_mapDevices;///< key-设备类型
	QHistoryFilterSampleImp*		m_filterImp;			///< 筛选内部窗口
	CustomTabWidget*						m_fastFilterTab;		///< 快捷查询Tab窗口
	std::vector<::tf::HistoryBaseDataQueryCond> m_vecFastFilterCond;	///< 快捷筛选条件
	::tf::HistoryBaseDataQueryCond			m_lastFilterCond;			///< 上一次的筛选条件

    int m_prePageIdx;   ///< 前一个选择的页签
    std::vector<QHistoryFilterSampleImp*> m_totalPages; ///< 缓存的所有页签
};

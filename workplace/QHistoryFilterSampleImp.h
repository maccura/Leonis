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
/// @file     QHistoryFilterSampleImp.h
/// @brief    历史数据查询内部窗口
///
/// @author   7702/WangZhongXin
/// @date     2023年4月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#ifndef QHISTORYFILTERSAMPLEIMP_H
#define QHISTORYFILTERSAMPLEIMP_H

#include <QWidget>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigDefine.h"

class QGroupAssayPositionSettings;

namespace Ui {
    class QHistoryFilterSampleImp;
};

class QHistoryFilterSampleImp : public QWidget
{
    Q_OBJECT

public:

    ///
    /// @brief 
    ////    当pageIdx时不显示名称编辑框，其余在名称编辑框中默认填写“快捷筛选n”
    ///
    explicit QHistoryFilterSampleImp(QWidget *parent = nullptr, int pageIdx = 0);
    ~QHistoryFilterSampleImp();

	/// @brief 
	///
	/// @param[in]  qryCond  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月14日，新建函数
	///
	bool GetQueryConditon(::tf::HistoryBaseDataQueryCond& qryCond) const;

	///
	///
	/// @brief 设置当前查询条件
	///
	/// @param[in]  qryCond  查询条件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月13日，新建函数
	///
	void SetQueryConditon(const ::tf::HistoryBaseDataQueryCond& qryCond);

	///
	/// @brief 更新工作页面的按钮状态
	///
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月5日，新建函数
	///
	void UpdateButtonStatus(const PageSet& workSet);

    ///
    /// @bref
    ///		当切换页签时，检测当前页签的设置变化，并进行提醒; 返回是否需要保存
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月22日，新建函数
    ///
    bool IsWannaSaveChanged() const;

    ///
    /// @brief 验证保存筛选参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月31日，新建函数
    ///
    bool VerifyParameter();

    ///
    /// @bref
    ///		获取快捷筛选名称
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月27日，新建函数
    ///
    QString GetFilterName() const;

    ///
    /// @bref
    ///		获取当前页签所在的tab索引
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月27日，新建函数
    ///
    inline int GetCurrentIndex() { return m_pageIdx; };

private slots:
	///
	/// @brief 设备类型改变的槽函数
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月16日，新建函数
	///
	void OnDeviceChanged();

signals:
	void finished();

	///
	/// @brief 保存快捷筛选信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月5日，新建函数
	///
	void SignalSaveFastFilter();

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void Init();

	public slots:

	///
	/// @brief 项目选择对应的槽函数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void OnAssaySelect();

	///
	/// @brief 处理项目选择信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月1日，新建函数
	///
	void OnSelectAssay();

	///
	/// @brief 重置按钮槽函数(清空所有条件，但是显示名称)
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月14日，新建函数
	///
	void OnReset();
    inline void ReloadCondition() { SetQueryConditon(m_cond); };

	///
	/// @brief 根据应用设置更新按钮状态
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月31日，新建函数
	///
	void OnUpdateButtonStatus();

private:
    ///
    /// @bref
    ///		设置‘项目名’编辑框
    ///
    /// @param[in] strAssays 项目名字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void SetAssayEdit(const QString& strAssays);

    ///
    /// @bref
    ///		比较两个筛选条件是否相等，忽略日期的时间变化
    ///
    /// @param[in] src1 筛选条件1
    /// @param[in] src2 筛选条件2
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月22日，新建函数
    ///
    bool CompearFilterCondition(tf::HistoryBaseDataQueryCond src1, tf::HistoryBaseDataQueryCond src2) const;

    ///
    /// @bref
    ///		样本列表按钮列表中，隐藏包膜腔积液按钮
    ///
    /// @param[in] showBmqjy 隐藏或显示，真表示显示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月23日，新建函数
    ///
    void SetSampleTypeBtnVisble(bool showBmqjy);

protected:
	void showEvent(QShowEvent *event) override;

    ///
    /// @bref
    ///		用于判断是否弹框提示保存
    ///
    void hideEvent(QHideEvent *event) override;

private:
    Ui::QHistoryFilterSampleImp* ui;
    QGroupAssayPositionSettings*								m_assaySelectDialog;	///< 项目选择对话框
	std::vector<int>											m_vecAssayCode;			///< 项目选择编号
	std::map<int, std::vector<::tf::DeviceInfo>>				m_mapDevices;			///< key-设备类型
	int														    m_pageIdx;		        ///< 当默认为0时是快捷查询，其它为存储查询条件的快捷查询

	::tf::HistoryBaseDataQueryCond								m_cond;					///< 查询条件
	PageSet														m_workSet;              ///< 工作页面设置
};

#endif // QHISTORYFILTERSAMPLEIMP_H

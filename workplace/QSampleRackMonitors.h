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
/// @file     QSampleRackMonitors.h
/// @brief    样本架监控页面（单机）
///
/// @author   5774/WuHongTao
/// @date     2023年7月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年7月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "ui_QSampleRackMonitors.h"
#include "ui_QSampleRackMutilMonitors.h"
#include "src/thrift/track/gen-cpp/track_types.h"
#include "src/public/ConfigSerialize.h"
#include "QWorkShellPage.h"

class QSampleModel :public QAbstractTableModel
{
    Q_OBJECT

public:
    QSampleModel(QObject *parent = Q_NULLPTR, int rowCount = 5);
    virtual ~QSampleModel() {};

    ///
    /// @brief 设置当前样本数据
    ///
    /// @param[in]  sampleData   样本数据
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月24日，新建函数
    ///
    bool SetSampleData(const std::vector<SampleData>& sampleData)
    {
        this->beginResetModel();
        m_sampleData = sampleData;
        this->endResetModel();

		m_sampleMap.clear();
		int column = 0;
		for (const auto& data : sampleData)
		{
			if (data == Q_NULLPTR)
			{
				column++;
				continue;
			}

			m_sampleMap[data->id] = column++;
		}

        return true;
    };

    ///
    /// @brief 
    ///
    /// @param[in]    
    /// @param[in]  column  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月24日，新建函数
    ///
    QVariant GetDispaly(SampleData& sampleInfo, int column) const;

	///
	/// @brief 获取当前Index所有对于的样本
	///
	/// @param[in]  index  当前索引
	///
	/// @return 样本数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月4日，新建函数
	///
	SampleData GetSampelDateByIndex(const QModelIndex& index);

protected slots:
	///
	/// @brief 更新显示样本的槽函数
	///  
	/// @param[in]  vSIs  样本改变列表的信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月11日，新建函数
	///
	void OnUpdateSample(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs);

protected:
    int rowCount(const QModelIndex &parent) const override { return m_rowCount; };
    int columnCount(const QModelIndex &parent) const override { return m_headrData.size(); };
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    std::vector<SampleData>         m_sampleData;
    int                             m_rowCount;
	std::map<int64_t, int>			m_sampleMap;
    QStringList                     m_headrData;
};

class QSampleRackMonitorsUi
{
public:
	QSampleRackMonitorsUi();

	QSampleRackMonitorShell *sample_rack_monitor;
	QTableView *sample_detail_view;
	QLabel *reason_label;
	QPushButton *addSample_btn;
	QLabel *reason_text_label;
	QLabel *bk_left_bottom_label;
	QWidget *widget;
	QGridLayout *gridLayout_2;
	QHBoxLayout *horizontalLayout_6;
	QLabel *legend_sample_empty;
	QLabel *label_6;
	QSpacerItem *horizontalSpacer_5;
	QSpacerItem *verticalSpacer;
	QHBoxLayout *horizontalLayout_8;
	QLabel *legend_sample_testing;
	QLabel *label_10;
	QSpacerItem *horizontalSpacer_7;
	QHBoxLayout *horizontalLayout_10;
	QLabel *legend_rack_abnormal;
	QLabel *label_14;
	QSpacerItem *horizontalSpacer_9;
	QHBoxLayout *horizontalLayout_9;
	QLabel *legend_sample_abnormal;
	QLabel *label_12;
	QSpacerItem *horizontalSpacer_8;
	QHBoxLayout *horizontalLayout_7;
	QLabel *legend_sample_finished;
	QLabel *label_8;
	QSpacerItem *horizontalSpacer_6;
	QHBoxLayout *horizontalLayout_11;
	QLabel *legend_sample_to_be_tested;
	QLabel *label_4;
	QSpacerItem *horizontalSpacer_4;
	QWidget *widget1;
	QWidget *widget11;
	QHBoxLayout *horizontalLayout;
	QCheckBox *autoclear_btn;
	QSpacerItem *horizontalSpacer;
	QPushButton *clear_btn;
	QPushButton *clear_btn_2;
	QPushButton *clear_btn_3;
	QWidget *layoutWidget;
	QGridLayout *gridLayout;
	QLineEdit *seq_edit;
	QLabel *label_2;
	QLabel *label_3;
	QHBoxLayout *horizontalLayout_2;
	QLineEdit *bar_edit;
	QPushButton *find_btn;
	QSpacerItem *horizontalSpacer_2;
	QLabel *label;
	QLineEdit *rack_edit;
	QSpacerItem *verticalSpacer_2;
	QWidget *widget2;
	QHBoxLayout *horizontalLayout_5;
	QHBoxLayout *horizontalLayout_3;
	QLabel *plate_text_label;
	QLabel *platelabel;
	QHBoxLayout *horizontalLayout_4;
	QLabel *rack_number_text_label;
	QLabel *rack_number_label;
	QSpacerItem *horizontalSpacer_3;
	QFrame *line_h;
	QFrame *line_v;
	QFrame *line_v_2;
	QFrame *line_h_2;
	QLabel *bk_border;
	QLabel *bk_border_2;
	QFrame *line_v_3;
	QFrame *line_v_4;

	void setupUi(QWidget *QSampleRackMonitors, bool isPipeLine = false);
	Ui::QSampleRackMonitors          ui;
	Ui::QSampleRackMutilMonitors	 ui_onLine;
	bool							 m_isPipeLine;
};

class QSampleRackMonitors : public QWorkShellPage
{
    Q_OBJECT

public:
    QSampleRackMonitors(QWidget *parent = Q_NULLPTR);
    ~QSampleRackMonitors() {};

    ///
    /// @brief 新的样本架来临
    ///
    /// @param[in]  sampleRackInfo  （first:样本架的编号，样本信息（1-5））
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月25日，新建函数
    ///
    void AddNewSampleRack(const int32_t trayNo, std::pair<std::string, std::vector<SampleData>>& sampleRackInfo);

	///
	/// @brief 获取当前的盘数
	///
	///
	/// @return 盘数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月25日，新建函数
	///
	std::vector<int32_t> GetTrays() { return m_trays; };

	///
	/// @brief 
	///
	/// @param[in]  event  
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年5月24日，新建函数
	///
	void showEvent(QShowEvent *event);

protected:
    void Init();
	void ClearData(int trayId);
protected slots:
	///
	/// @brief 样本架回收信息
	///
	/// @param[in]  rack			样本架号   
	/// @param[in]  recycleRackInfo 样本架详细信息  
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月24日，新建函数
	///
	void OnUpdateRecyCleRackInfo(const int trayNo, std::vector<tf::RecycleRackInfo, std::allocator<tf::RecycleRackInfo>> recycleRackInfos);

	///
	/// @brief 托盘被清空的消息
	///
	/// @param[in]  statusInfo  消息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月8日，新建函数
	///
	void OnClearRecyCleRackInfo(class track::tf::LowerStatusInfo statusInfo);

private:
	QSampleRackMonitorsUi            ui;
    QSampleModel*                    m_model;
	std::vector<int32_t>			 m_trays;
	PageSet                          m_data;                     ///< 保存数据(是否开启自动清空功能)
    int                              m_testRow;
	bool							 m_isPipeLine;
};

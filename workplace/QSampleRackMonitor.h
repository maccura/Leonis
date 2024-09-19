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
/// @file     QSampleRackMonitor.h
/// @brief    样本架监控基础控件
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
#include <QAbstractTableModel>
#include "QWorkShellPage.h"
#include "boost/optional.hpp"
#include "thrift/DcsControlProxy.h"
#include <QPixmap>
#include <QTableView>
#include <QLabel>
#include <QHBoxLayout>
#include <deque>
#define  COLUMN 6

namespace Ui {
    class QSampleRackMonitor;
	class QSampleOneRackMonitor;
};


template<typename T>
class FixedVector
{
public:
    FixedVector(size_t maxSize, const std::function<T()>& initFunc)
        : maxSize(maxSize)
        , initFunction(initFunc)
    {
        data.assign(maxSize, initFunction());
    }

    // 重置
    bool Reset()
    {
        data.clear();
        data.assign(maxSize, initFunction());
        return true;
    }

    // 添加一个，删除一个，维持大小
    T addCoreData(T value)
    {
        data.emplace(data.begin(), value);
		T endValue = data.back();
        data.pop_back();
		return endValue;
    }

    // 拷贝构造函数
    FixedVector(const FixedVector& other)
        : maxSize(other.maxSize)
        , initFunction(other.initFunction)
    {
        data = other.data;
    }

    // 默认构造函数
    FixedVector() :FixedVector(10, []() {return T(); }) {}

    FixedVector& operator=(const FixedVector& other)
    {
        if (this == &other)
        {
            return *this;
        }

        maxSize = other.maxSize;
        initFunction = other.initFunction;
        data = other.data;
        return *this;
    }

	bool SetValue(int row, const T& dataElement)
	{
		if (row >= data.size() || row < 0)
		{
			return false;
		}

		data[row] = dataElement;
		return true;
	}

    // 获取变量
    const T operator[](size_t index) const 
    {
        if (index >= data.size())
        {
            return std::move(initFunction());
        }

        return data[index];
    }

private:
    std::vector<T> data;
    size_t         maxSize;
    std::function<T()> initFunction;
};

class QRackMonitorModel;
enum class ModelType
{
    standAlone,             ///< 单机
    pipeLine                ///< 联机
};

using SampleData = std::shared_ptr<tf::SampleInfo>;
using CoreData = std::pair<std::string, std::vector<SampleData>>;
class QSampleRackMonitor : public QWidget
{
    Q_OBJECT

public:
    QSampleRackMonitor(QWidget *parent = Q_NULLPTR,
        ModelType modelType = ModelType::standAlone,
		int32_t tray = 0);
    ~QSampleRackMonitor();

	struct RackUi
	{
		QTableView *tableRackView;
		QLabel *label;
		QLabel *bk_tableRackView;

		RackUi()
		{
			tableRackView = Q_NULLPTR;
			label = Q_NULLPTR;
			bk_tableRackView = Q_NULLPTR;
		};

		void setupUi(QWidget *QSampleRackMonitor, ModelType type = ModelType::standAlone);

	private:
		Ui::QSampleRackMonitor*          ui_onLine;
		Ui::QSampleOneRackMonitor*		 ui;
		ModelType						 m_model;
	};

    bool Clear();
    const CoreData GetCurrentRackData();
	const int32_t GetTrayID() { return m_trayId; };
    bool AddNewRackInfo(CoreData& coreData);
    void SetPageCode(int number);
	void SetSelected(int selectRow);
	void DeSelected();
	SampleData FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition);

Q_SIGNALS:
    void rackChanged(int row = 0, int column = 0);

protected:
    void Init();

private:
	RackUi*							ui;
    QRackMonitorModel*              m_model;
    ModelType                       m_modelType;
	int32_t							m_trayId;	
};

class QRackMonitorModel :public QAbstractTableModel
{
    Q_OBJECT

public:
    QRackMonitorModel(QObject *parent = Q_NULLPTR, int rowCount = 5);
    virtual ~QRackMonitorModel();

    ///
    /// @brief 添加样本架数据
    ///
    /// @param[in]  rackInfo  样本架编号
    ///
    /// @return true:添加成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月24日，新建函数
    ///
    bool AddNewRackInfo(CoreData& coreData);
    const CoreData GetCurrentRackData(int row);
    bool Clear();
	boost::optional<QModelIndex> FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition);
	void SetCurrentIndex(boost::optional<QModelIndex>& index)
	{
		m_selectedIndex = index;
		auto indexstart = this->index(0, 0);
		auto indexend = this->index(this->rowCount(QModelIndex()) - 1, this->columnCount(QModelIndex()) - 1);
		emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
	};

	///
	/// @brief 获取样本状态的pixmap标识
	///
	/// @param[in]  sampleInfo  样本信息
	/// @param[in]  isSelected  是否选中
	///
	/// @return 状态标识pixmap
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月24日，新建函数
	///
	QPixmap GetStatusPixmap(SampleData& sampleInfo, bool isSelected) const;

protected:
    int rowCount(const QModelIndex &parent) const override { return m_rowCount; };
    int columnCount(const QModelIndex &parent) const override { return COLUMN; };
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	bool IsPipeLine() const;

protected slots:
	///
	/// @brief 更新样本信息
	///
	/// @param[in]  enUpdateType  更新类型（删除，增加）-只处理修改  
	/// @param[in]  vSIs  样本
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月28日，新建函数
	///
	void OnSampleInfoUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs);

private:
    FixedVector<CoreData>           m_rackData;
    int                             m_rowCount;
    QStringList                     m_headrData;
	boost::optional<QModelIndex>	m_selectedIndex;
	std::map<int64_t, std::shared_ptr<tf::SampleInfo>> m_sampleMap;
	std::map<int64_t, std::pair<int, int>> m_samplePostion;
};

class QSampleRackMonitorShell : public QWidget
{
	Q_OBJECT

public:
	QSampleRackMonitorShell(QWidget *parent = Q_NULLPTR);
	~QSampleRackMonitorShell() {};

	///
	/// @brief 初始化托盘数目
	///
	/// @param[in]  trays  托盘编号列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月18日，新建函数
	///
	bool SetMonitorTrays(std::vector<int32_t> trays, bool isPipeLine = false);

	///
	/// @brief 
	///
	/// @param[in]  trayNo			托盘编号   
	/// @param[in]  sampleRackInfo  样本架  
	///
	/// @return true:添加成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	bool AddNewSampleRack(const int32_t trayNo, std::pair<std::string, std::vector<SampleData>>& sampleRackInfo);

	///
	/// @brief 清除trayId对应的托盘数据
	///
	/// @param[in]  trayNo  托盘Id
	///
	/// @return true:表示清除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	bool Clear(const int32_t trayNo);

	///
	/// @brief 获取当前的选中数据
	///
	///
	/// @return 当前选中数据（架号-样本列表）
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	const CoreData GetCurrentRackData();

	///
	/// @brief 获取当前的托盘Id
	///
	///
	/// @return托盘Id
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	boost::optional<int32_t> GetCurrentTrayId() { return m_currentTray; };

	///
	/// @brief 根据条件查询样本信息
	/// 
	/// @param[in]  searchCondition  查询条件
	///
	/// @return 样本信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	SampleData FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition);

	///
	/// @brief 选中样本管
	///
	/// @param[in]  select  列号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月19日，新建函数
	///
	void SetSelected(int select);

Q_SIGNALS:
	void rackChanged(int row = 0, int column = 0);

protected:
	void Init();

private:
	std::map<int, std::shared_ptr<QSampleRackMonitor>> m_sampleRackMonitors;
	QHBoxLayout*					m_layout;
	boost::optional<int32_t>		m_currentTray;				///< 当前选中的托盘
};

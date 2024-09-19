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

#include "QSampleRackMonitor.h"
#include "ui_QSampleRackMonitor.h"
#include "ui_QSampleOneRackMonitor.h"
#include "shared/CReadOnlyDelegate.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "src/common/Mlog/mlog.h"
#define	 FAILEDID		-99

QSampleRackMonitor::QSampleRackMonitor(QWidget *parent, ModelType modelType, int32_t tray)
    : QWidget(parent)
    , m_model(Q_NULLPTR)
    , m_modelType(modelType)
	, m_trayId(tray)
{
    ui = new RackUi();
    ui->setupUi(this, m_modelType);
    Init();
    connect(ui->tableRackView, &QTableView::clicked, this, [&](const QModelIndex &index)
    {
		SetSelected(-1);
		auto row = index.row();
		auto column = index.column();
		SetSelected(column - 1);
        emit this->rackChanged(row, column);
    });

	// modify bug 0012058 by wuht
	connect(ui->tableRackView->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex& index, const QModelIndex& preIndex)
	{
		SetSelected(-1);
		auto row = index.row();
		auto column = index.column();
		SetSelected(column - 1);
		emit this->rackChanged(row, column);
	});
}

QSampleRackMonitor::~QSampleRackMonitor()
{
}

bool QSampleRackMonitor::Clear()
{
    if (m_model == Q_NULLPTR)
    {
        return false;
    }

    m_model->Clear();
    return true;
}

const CoreData QSampleRackMonitor::GetCurrentRackData()
{
    auto index = ui->tableRackView->currentIndex();
    if (!index.isValid())
    {
        std::vector<SampleData> innerData(5, Q_NULLPTR);
        return std::move(std::make_pair("", innerData));
    }

    return m_model->GetCurrentRackData(index.row());
}

bool QSampleRackMonitor::AddNewRackInfo(CoreData& coreData)
{
    if (m_model == Q_NULLPTR)
    {
        return false;
    }

	SetSelected(-1);
	ui->tableRackView->clearSelection();
    m_model->AddNewRackInfo(coreData);
    return true;
}

void QSampleRackMonitor::SetPageCode(int number)
{
    ui->label->setText(QString::number(number));
	ui->label->show();
};

void QSampleRackMonitor::SetSelected(int selectRow)
{
	auto index = ui->tableRackView->currentIndex();
	if (!index.isValid())
	{
		auto index = boost::make_optional(QModelIndex());
		m_model->SetCurrentIndex(index);
		return;
	}

	if (m_model != Q_NULLPTR)
	{
		int row = index.row();
		auto currentindex = m_model->index(row, selectRow + 1);
		auto currentIndexWarp = boost::make_optional(currentindex);
		m_model->SetCurrentIndex(currentIndexWarp);
	}
}

void QSampleRackMonitor::DeSelected()
{
	ui->tableRackView->clearSelection();
	auto index = boost::make_optional(QModelIndex());
	m_model->SetCurrentIndex(index);
}

SampleData QSampleRackMonitor::FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition)
{
	if (m_model == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	SetSelected(-1);
	ui->tableRackView->clearSelection();
	auto sampleIndex = m_model->FindSample(searchCondition);
	if (!sampleIndex)
	{
		return Q_NULLPTR;
	}

	int row = sampleIndex.value().row();
	int column = sampleIndex.value().column();

	ui->tableRackView->setCurrentIndex(sampleIndex.value());
	emit this->rackChanged(row, column);
	auto coreData = m_model->GetCurrentRackData(row);
	if (coreData.first.empty())
	{
		return Q_NULLPTR;
	}

	// modify bug0011884 by wuht 
	if (column <= 0)
	{
		return std::make_shared<tf::SampleInfo>();
	}

	if (coreData.second.size() < 5)
	{
		return Q_NULLPTR;
	}

	return coreData.second[5 - column];
}

void QSampleRackMonitor::RackUi::setupUi(QWidget *QSampleRackMonitor, ModelType type /*= ModelType::standAlone*/)
{
	if (type == ModelType::pipeLine)
	{
		ui_onLine = new Ui::QSampleRackMonitor();
		ui_onLine->setupUi(QSampleRackMonitor);
		tableRackView = ui_onLine->tableRackView;
		label = ui_onLine->label;
		bk_tableRackView = ui_onLine->bk_tableRackView;
	}
	else
	{
		ui = new Ui::QSampleOneRackMonitor();
		ui->setupUi(QSampleRackMonitor);
		tableRackView = ui->tableRackView;
		label = ui->label;
		bk_tableRackView = ui->bk_tableRackView;
	}
}

void QSampleRackMonitor::Init()
{
	ui->label->setText("");
	ui->label->hide();
	if (m_modelType == ModelType::standAlone)
	{
		m_model = new QRackMonitorModel(this, 10);
	}
	else
	{
		m_model = new QRackMonitorModel(this, 20);
	}

	ui->tableRackView->setModel(m_model);
	ui->tableRackView->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
	ui->tableRackView->setItemDelegateForColumn(1, new QCircleButton(this));
	ui->tableRackView->setItemDelegateForColumn(2, new QCircleButton(this));
	ui->tableRackView->setItemDelegateForColumn(3, new QCircleButton(this));
	ui->tableRackView->setItemDelegateForColumn(4, new QCircleButton(this));
	ui->tableRackView->setItemDelegateForColumn(5, new QCircleButton(this));

	ui->tableRackView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableRackView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableRackView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableRackView->setShowGrid(false);

	if (m_modelType == ModelType::standAlone)
	{
		setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-single-outer.png) center no-repeat;");
		ui->tableRackView->setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-single.png) center no-repeat;");
		ui->tableRackView->setProperty("headers_h", ("tableRackView_single_h"));
		ui->tableRackView->setProperty("headers_v", ("tableRackView_single_v"));
		ui->tableRackView->setProperty("items", ("tableRackView_single"));
		//ui->tableRackView->setColumnWidth(0, 78);

		ui->tableRackView->setColumnWidth(0, 90);
	}
	else
	{
		this->resize(365, 797);
		ui->tableRackView->setGeometry(QRect(44, 30, 312, 760));
		ui->bk_tableRackView->setGeometry(QRect(0, 0, 365, 797));

		setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-online-outer.png) center no-repeat;");
		ui->tableRackView->setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-online.png) center no-repeat;");
		ui->tableRackView->setProperty("headers_h", ("tableRackView_online_h"));
		ui->tableRackView->setProperty("headers_v", ("tableRackView_online_v"));
		ui->tableRackView->setProperty("items", ("tableRackView_online"));
		ui->tableRackView->setColumnWidth(0, 68);
	}

// 	ui->tableRackView->setStyleSheet(
// 
// 		"QTableView:item:selected {"
// 		"background-color: lightblue;"
// 		"border: none;"
// 		"border-radius: 10px"
// 		"}"
// 	);

    //ui->tableRackView->verticalHeader()->hide();
    //ui->tableRackView->horizontalHeader()->hide();
}

QRackMonitorModel::QRackMonitorModel(QObject *parent, int rowCount)
    : QAbstractTableModel(parent)
    , m_rowCount(rowCount)
	, m_selectedIndex(boost::none)
{
    // 初始化样本架状态
    std::vector<SampleData> innerData(5, Q_NULLPTR);
    auto initFunc = [=]() {
        return std::make_pair(" ", innerData);
    };

    m_rackData = FixedVector<CoreData>(m_rowCount, initFunc);
    m_headrData << "" << "5" << "4" << "3" << "2" << "1";
	REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnSampleInfoUpdate);
}

QRackMonitorModel::~QRackMonitorModel()
{

}

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
bool QRackMonitorModel::AddNewRackInfo(CoreData& coreData)
{
	// 删除样本的旧位置数据
	for (const auto& sampleInfo : coreData.second)
	{
		if (sampleInfo == Q_NULLPTR)
		{
			continue;
		}

		// 更新位置信息（主要是删除之前样本的位置）
		auto iterPos = m_samplePostion.find(sampleInfo->id);
		if (iterPos != m_samplePostion.end())
		{
			int row = iterPos->second.first;
			int column = iterPos->second.second;
			if (row < m_rowCount 
				&& row >= 0
				&& column < (COLUMN-1))
			{
				auto rack = m_rackData[row].first;
				auto sampleVec = m_rackData[row].second;
				auto currentSample = sampleVec[column];

				ULOG(LOG_INFO, "%s(want to delete sample id: %d, row: %d, column: %d)", __FUNCTION__, sampleInfo->id, row, column);
				if (currentSample != nullptr)
				{
					ULOG(LOG_INFO, "%s(real delete sample id: %d, row: %d, column: %d)", __FUNCTION__, currentSample->id, row, column);
				}
				else
				{
					ULOG(LOG_INFO, "%s(the postion is empty)", __FUNCTION__);
				}

				sampleVec[column] = Q_NULLPTR;
				// 回写值
				m_rackData.SetValue(row, std::make_pair(rack, sampleVec));
				auto index = this->index(row, column);
				emit dataChanged(index, index, { Qt::DisplayRole });
			}

			m_samplePostion.erase(iterPos);
		}

		ULOG(LOG_INFO, "%s(add sample id: %d : status: %d)", __FUNCTION__, sampleInfo->id, sampleInfo->status);
		// 执行添加
		m_sampleMap[sampleInfo->id] = sampleInfo;
	}

	// 刷新样本位置信息
	for (auto& postionInfo : m_samplePostion)
	{
		auto& postion = postionInfo.second;
		//ULOG(LOG_INFO, " sample id: %d(row: %d : column: %d)", __FUNCTION__, postionInfo.first, postion.first, postion.second);
		postion.first = postion.first + 1;
	}

	// 更新新增样本位置信息
	int newColumn = 0;
	for (const auto& sampleInfo : coreData.second)
	{
		if (sampleInfo == Q_NULLPTR)
		{
			newColumn++;
			continue;
		}

		m_samplePostion[sampleInfo->id] = std::make_pair(0, newColumn++);
	}

	this->beginResetModel();
	auto popupData = m_rackData.addCoreData(coreData);
	this->endResetModel();

	// 从样本地图中删除已经pop_up的样本
	for (const auto& sampleInfo : popupData.second)
	{
		if (sampleInfo == Q_NULLPTR)
		{
			continue;
		}

		ULOG(LOG_INFO, "%s(popUp sample id: %d : status: %d)", __FUNCTION__, sampleInfo->id, sampleInfo->status);
		// 执行删除
		auto iter = m_sampleMap.find(sampleInfo->id);
		if (iter != m_sampleMap.end())
		{
			m_sampleMap.erase(iter);
		}

		// 删除位置信息
		auto iterPos = m_samplePostion.find(sampleInfo->id);
		if (iterPos != m_samplePostion.end())
		{
			m_samplePostion.erase(iterPos);
		}
	}

    return true;
}

const CoreData QRackMonitorModel::GetCurrentRackData(int row)
{
    if (row < 0 || row >= m_rowCount)
    {
        std::vector<SampleData> innerData(5, Q_NULLPTR);
        return std::move(std::make_pair("", innerData));
    }

    return m_rackData[row];
}

bool QRackMonitorModel::Clear()
{
    this->beginResetModel();
    m_rackData.Reset();
	m_sampleMap.clear();
	m_samplePostion.clear();
    this->endResetModel();
    return true;
}

boost::optional<QModelIndex> QRackMonitorModel::FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition)
{
	if (m_rowCount <= 0)
	{
		return boost::none;
	}

	// 第一条件，样本条码
	auto firstCond = std::get<0>(searchCondition);
	// 第二条件，样本号
	auto secondCond = std::get<1>(searchCondition);
	// 第三条件 ，样本架
	auto thirdCond = std::get<2>(searchCondition);

	ULOG(LOG_INFO, "%s(find sampleInfo rack:%s, barCode:%s, seqNO:%s)", __FUNCTION__, firstCond, secondCond, thirdCond);

	int i = 0;
	do 
	{
		// 获取每行的样本
		auto datas = GetCurrentRackData(i);
		// 当first为空的时候，代表此行为空
		if (datas.first.empty())
		{
			i++;
			continue;
		}

		// 若有架号，必须首先判断架
		if (!thirdCond.empty() && datas.first != thirdCond)
		{
			i++;
			continue;
		}

		// 若架号相同，并且其他两个条件都为空，则直接返回当前架号
		if (firstCond.empty() && secondCond.empty())
		{
			m_selectedIndex = this->index(i, 0);
			return m_selectedIndex;
		}

		int column = 5;
		// 依次处理每个样本数据
		for (auto& sampleData : datas.second)
		{
			if (column <= 0)
			{
				continue;
			}

			if (!sampleData)
			{
				column--;
				continue;
			}

			// 样本条码
			if (!firstCond.empty() && sampleData->barcode != firstCond)
			{
				column--;
				continue;
			}

			// 样本号
			if (!secondCond.empty() && sampleData->seqNo != secondCond)
			{
				column--;
				continue;
			}

			// 所有条件都满足，则返回
			m_selectedIndex = this->index(i, column);
			return m_selectedIndex;
		}

		i++;
	} while (i < m_rowCount);

	return boost::none;
}

///
/// @brief 获取样本状态的pixmap标识
///
/// @param[in]  sampleInfo  样本信息
///
/// @return 状态标识pixmap
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月24日，新建函数
///
QPixmap QRackMonitorModel::GetStatusPixmap(SampleData& sampleInfo, bool isSelected) const
{
	// 若当前样本未为空
	if (!sampleInfo)
	{
		if (IsPipeLine())
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-empty.png");
		}
		else
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-empty.png");
		}
	}

	// 特殊处理扫描失败的情况
	if (sampleInfo->id == FAILEDID)
	{
		if (isSelected)
		{
			if (IsPipeLine())
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-abnormal-selected.png");
			}
			else
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-abnormal-selected.png");
			}
		}

		return QPixmap(":/Leonis/resource/image/rack-sample-abnormal.png");
	}
	// 测试完成状态
	else if (sampleInfo->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTED)
	{
		if (isSelected)
		{
			if (IsPipeLine())
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-finished-selected.png");
			}
			else
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-finished-selected.png");
			}
		}

		if (IsPipeLine())
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-finished.png");
		}
		else
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-finished.png");
		}
	}
	// 测试状态
	else if (sampleInfo->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
	{
		if (isSelected)
		{
			if (IsPipeLine())
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-testing-selected.png");
			}
			else
			{
				return QPixmap(":/Leonis/resource/image/rack-sample-testing-selected.png");
			}
		}

		return QPixmap(":/Leonis/resource/image/rack-sample-testing.png");
	}
	// 待测
	else if (sampleInfo->status == tf::SampleStatus::type::SAMPLE_STATUS_PENDING)
	{
		if (isSelected)
		{
			if (IsPipeLine())
			{
				return QPixmap(":/Leonis/resource/image/rack-sample--to-be-tested-selected.png");
			}
			else
			{
				return QPixmap(":/Leonis/resource/image/rack-sample--to-be-tested-selected.png");
			}
		}

		if (IsPipeLine())
		{
			return QPixmap(":/Leonis/resource/image/rack-sample--to-be-tested.png");
		}
		else
		{
			return QPixmap(":/Leonis/resource/image/rack-sample--to-be-tested.png");
		}
	}
	
	// 若找不到则是异常状态
	if (isSelected)
	{
		if (IsPipeLine())
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-abnormal-selected.png");
		}
		else
		{
			return QPixmap(":/Leonis/resource/image/rack-sample-abnormal-selected.png");
		}
	}

	if (IsPipeLine())
	{
		return QPixmap(":/Leonis/resource/image/rack-sample-abnormal.png");
	}
	else
	{
		return QPixmap(":/Leonis/resource/image/rack-sample-abnormal.png");
	}
}

QVariant QRackMonitorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto column = index.column();
    if (row < 0 || row >= m_rowCount)
    {
        return QVariant();
    }

    // 获取数据
    if (role == Qt::DisplayRole) {

        auto data = m_rackData[row].first;
		if(column == 0)
        {
            return QString::fromStdString(data);
        }
        else
        {
			// 位置编号是0，1，2，3，4
			if (column > 0 && column <= 5)
			{
				if (m_rackData[row].second.size() < 5)
				{
					return QVariant();
				}
				auto sampleData = m_rackData[row].second[5 - column];

				// 判断当前Item是否被选中
				bool isSelected = false;
				if (m_selectedIndex 
					&& m_selectedIndex.value() == index)
				{
					isSelected = true;
				}
				return GetStatusPixmap(sampleData, isSelected);
			}
        }
    }
	else if (role == Qt::UserRole)
	{
		// 当样本架号为空的时候，表示架异常
		auto data = m_rackData[row].first;
		bool isAbnormalRack = data.empty();

		// 第一列的边框
		if (column == 0)
		{
			// 若找不到则是异常状态
			if (IsPipeLine())
			{
				if (isAbnormalRack)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-online-left-abnormal.png");
				}

				return QPixmap(":/Leonis/resource/image/bk-monitor-online-left.png");
			}
			else
			{
				if (isAbnormalRack)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-single-left-abnormal.png");
				}

				return QPixmap(":/Leonis/resource/image/bk-monitor-single-left.png");
			}
		}

		// 最后一列的边框
		if (column == 5)
		{
			// 若找不到则是异常状态
			if (IsPipeLine())
			{
				if (isAbnormalRack)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-online-right-abnormal.png");
				}

				return QPixmap(":/Leonis/resource/image/bk-monitor-online-right.png");
			}
			else
			{
				if (isAbnormalRack)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-single-right-abnormal.png");
				}

				return QPixmap(":/Leonis/resource/image/bk-monitor-single-right.png");
			}
		}
	}
	else if (role == Qt::UserRole + 2)
	{
		auto data = m_rackData[row].first;
		bool isAbnormalRack = data.empty();
		// 联机版
		if (IsPipeLine())
		{
			if (isAbnormalRack)
			{
				if (column == 0)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-online-left-abnormal.png");
				}
				else if (column == 5)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-online-right-abnormal.png");
				}
				else
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-online-middle-abnormal.png");
				}
			}
		}
		// 单机版
		else
		{
			if (isAbnormalRack)
			{
				if (column == 0)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-single-left-abnormal.png");
				}
				else if (column == 5)
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-single-right-abnormal.png");
				}
				else
				{
					return QPixmap(":/Leonis/resource/image/bk-monitor-single-middle-abnormal.png");
				}
			}
		}
	}
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        /*return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);*/
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    return QVariant();
}

QVariant QRackMonitorModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {

            if (section < 0 || section >= COLUMN)
            {
                return QVariant();
            }

            return m_headrData[section];
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

bool QRackMonitorModel::IsPipeLine() const
{
	if (m_rowCount == 20)
	{
		return true;
	}

	return false;
}

///
/// @brief 更新样本信息
///
/// @param[in]  enUpdateType  更新类型（删除，增加）-只处理修改  
/// @param[in]  vSIs  样本
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月28日，新建函数
///
void QRackMonitorModel::OnSampleInfoUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs)
{
	ULOG(LOG_INFO, "%s(operation type: %d content: %s)", __FUNCTION__, enUpdateType, ToString(vSIs));
	if (enUpdateType != tf::UpdateType::type::UPDATE_TYPE_MODIFY)
	{
		return;
	}

	for (const auto& sample : vSIs)
	{
		auto iter = m_sampleMap.find(sample.id);
		if (iter == m_sampleMap.end())
		{
			continue;
		}

		// 状态改变
		if (!sample.__isset.status)
		{
			continue;
		}

		// 若状态有变化才更新
		auto saveSample = iter->second;
		ULOG(LOG_INFO, "%s(sample id: (old status: %d -- new status: %d))", __FUNCTION__, saveSample->id, saveSample->status, sample.status);
		if (saveSample->status != sample.status)
		{
			iter->second->__set_status(sample.status);
		}
	}

	for (int i = 0; i < m_rowCount; i++)
	{
		auto indexstart = this->index(i, 0);
		auto indexend = this->index(i, this->columnCount(QModelIndex()) - 1);
		emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
	}
}


QSampleRackMonitorShell::QSampleRackMonitorShell(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
{
	this->setObjectName(QStringLiteral("QSampleRackMonitorShell"));
	m_layout = Q_NULLPTR;
	m_currentTray = boost::none;
	Init();
}

///
/// @brief 初始化托盘数目
///
/// @param[in]  trays  托盘编号列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月18日，新建函数
///
bool QSampleRackMonitorShell::SetMonitorTrays(std::vector<int32_t> trays, bool isPipeLine)
{
	if (m_layout == Q_NULLPTR)
	{
		return false;
	}

	for (auto tray : trays)
	{
		ModelType modeltype = ModelType::standAlone;
		if (isPipeLine)
		{
			modeltype = ModelType::pipeLine;
		}
		auto rackMonitor =  new QSampleRackMonitor(this, modeltype, tray);
		/*
		if (modeltype == ModelType::standAlone)
		{
			rackMonitor->setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-single-outer.png) center no-repeat;");
		}
		else
		{
			rackMonitor->setStyleSheet("background: url(:/Leonis/resource/image/bk-monitor-online-outer.png) center no-repeat;");
		}*/

		connect(rackMonitor, &QSampleRackMonitor::rackChanged, this, [&](int row, int column)
		{
			QSampleRackMonitor* sendWidget = qobject_cast<QSampleRackMonitor*>(sender());
			if (sendWidget == Q_NULLPTR)
			{
				ULOG(LOG_INFO, "%s(can not get the sender() row: %d, column: %d)", __FUNCTION__, row, column);
				return;
			}

			if (isPipeLine)
			{
				sendWidget->setProperty("bks", ("bk_tableRackView_online"));
			}
			else
			{
				sendWidget->setProperty("bks", ("tableRackView_single"));
			}
			auto trayId = sendWidget->GetTrayID();
			if (m_currentTray.has_value() && trayId != m_currentTray.value())
			{
				if (m_sampleRackMonitors.count(m_currentTray.value()) <= 0)
				{
					return;
				}

				auto rackMonitor = m_sampleRackMonitors[m_currentTray.value()];
				if (rackMonitor == Q_NULLPTR)
				{
					return;
				}

				rackMonitor->DeSelected();
			}

			m_currentTray = sendWidget->GetTrayID();
			emit this->rackChanged(row, column);
		});

		if (rackMonitor != Q_NULLPTR)
		{
			m_sampleRackMonitors[tray] = std::shared_ptr<QSampleRackMonitor>(rackMonitor);
			m_layout->addWidget(rackMonitor);
		}
	}

	return true;
}

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
bool QSampleRackMonitorShell::AddNewSampleRack(const int32_t trayNo, std::pair<std::string, std::vector<SampleData>>& sampleRackInfo)
{
	if (m_sampleRackMonitors.count(trayNo) <= 0)
	{
		return false;
	}

	auto rackMonitor = m_sampleRackMonitors[trayNo];
	if (rackMonitor == Q_NULLPTR)
	{
		return false;
	}

	// 对应的托盘添加样本信息
	rackMonitor->AddNewRackInfo(sampleRackInfo);
	return true;
}

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
bool QSampleRackMonitorShell::Clear(const int32_t trayNo)
{
	if (m_sampleRackMonitors.count(trayNo) <= 0)
	{
		return false;
	}

	auto rackMonitor = m_sampleRackMonitors[trayNo];
	if (rackMonitor == Q_NULLPTR)
	{
		return false;
	}

	rackMonitor->Clear();
	return true;
}

///
/// @brief 获取当前的选中数据
///
///
/// @return 当前选中数据（架号-样本列表）
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月19日，新建函数
///
const CoreData QSampleRackMonitorShell::GetCurrentRackData()
{
	std::vector<SampleData> innerData(5, Q_NULLPTR);
	if (!m_currentTray.has_value())
	{
		return std::move(std::make_pair("", innerData));
	}

	if (m_sampleRackMonitors.count(m_currentTray.value()) <= 0)
	{
		return std::move(std::make_pair("", innerData));
	}

	auto rackMonitor = m_sampleRackMonitors[m_currentTray.value()];
	if (!rackMonitor)
	{
		return std::move(std::make_pair("", innerData));
	}

	return rackMonitor->GetCurrentRackData();
}

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
SampleData QSampleRackMonitorShell::FindSample(const std::tuple<std::string, std::string, std::string>& searchCondition)
{
	for (auto rackMonitor : m_sampleRackMonitors)
	{
		auto monitor = rackMonitor.second;
		if (monitor == Q_NULLPTR)
		{
			continue;
		}

		auto sampleValue = monitor->FindSample(searchCondition);
		if (sampleValue != Q_NULLPTR)
		{
			return sampleValue;
		}
	}

	return Q_NULLPTR;
}

///
/// @brief 选中样本管
///
/// @param[in]  select  列号
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月19日，新建函数
///
void QSampleRackMonitorShell::SetSelected(int select)
{
	if (!m_currentTray.has_value())
	{
		return;
	}

	if (m_sampleRackMonitors.count(m_currentTray.value()) <= 0)
	{
		return;
	}

	auto rackMonitor = m_sampleRackMonitors[m_currentTray.value()];
	if (!rackMonitor)
	{
		return;
	}

	return rackMonitor->SetSelected(select);
}

void QSampleRackMonitorShell::Init()
{
	m_sampleRackMonitors.clear();
	if (m_layout == Q_NULLPTR)
	{
		m_layout = new QHBoxLayout(this);
		m_layout->setObjectName("test33");
		m_layout->setSpacing(4);
		m_layout->setContentsMargins(0, 4, 0, 0);
	}
}

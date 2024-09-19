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

#include "QSampleRackMonitors.h"
#include "QSampleAssayModel.h"
#include "shared/msgiddef.h"
#include "shared/basedlg.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "manager/SystemConfigManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#define MAXPOSNUMER		 5
#define MAXCOLUMN		 5
#define	 FAILEDID		-99

QSampleRackMonitors::QSampleRackMonitors(QWidget *parent)
    : QWorkShellPage(parent)
	, m_isPipeLine(false)
    , m_model(Q_NULLPTR)
{
	// 是否联机模式（待完善）
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		m_isPipeLine = true;
	}

	// 测试使用（置为true为联机，false为单机）
	// m_isPipeLine = true;
	ui.setupUi(this, m_isPipeLine);
    Init();
	// 测试的时候使用的按钮去掉，modify bug0011313 by wuht
	ui.addSample_btn->setVisible(false);
	// 0021469: [工作] 样本架监视界面，未选择任何样本架时异常原因默认不为空，界面初始化不显示异常原因 modify by chenjianlin 20230814
	//ui.reason_label->setText("");
	ui.rack_number_label->setText("");
    m_testRow = 0;
    connect(ui.sample_rack_monitor, &QSampleRackMonitorShell::rackChanged, this, [&](int row, int column)
    {
        if (m_model == Q_NULLPTR)
        {
            return;
        }

        auto coreData = ui.sample_rack_monitor->GetCurrentRackData();
        m_model->SetSampleData(coreData.second);
		auto trayId = ui.sample_rack_monitor->GetCurrentTrayId();
		if (trayId)
		{
			ui.platelabel->setText(QString::number(trayId.value()));
		}
		else
		{
			ui.platelabel->setText("");
		}

        ui.rack_number_label->setText(QString::fromStdString(coreData.first));

		if (column > MAXPOSNUMER || column <= 0)
		{
			if (column > MAXPOSNUMER)
			{
				ui.bar_edit->clear();
				ui.seq_edit->clear();
				ui.rack_edit->clear();
			}
		}
		else
		{

			auto sampleInfoValue = coreData.second[MAXCOLUMN - column];
			if (sampleInfoValue)
			{
				auto currentIndex = m_model->index(MAXCOLUMN - column, 0);
				ui.sample_detail_view->setCurrentIndex(currentIndex);
				/// 取消映射关系bug0011170
// 				auto barCode = QString::fromStdString(sampleInfoValue->barcode);
// 				auto seqCode = QString::fromStdString(sampleInfoValue->seqNo);
// 				auto rackCode = QString::fromStdString(sampleInfoValue->rack);
// 				ui.bar_edit->setText(barCode);
// 				ui.seq_edit->setText(seqCode);
// 				ui.rack_edit->setText(rackCode);
			}
		}

    });

	std::vector<QPushButton*> clearButtons;
	if (m_isPipeLine)
	{
		clearButtons = { ui.clear_btn , ui.clear_btn_2, ui.clear_btn_3 };
	}
	else
	{
		clearButtons = { ui.clear_btn};
	}

    // 清空
	for (auto& button : clearButtons)
	{
		connect(button, &QPushButton::clicked, this, [=]()
		{
			int trayid = button->property("tray").toInt();
			ClearData(trayid);
		});
	}

	// modify bug0011798 by wuht 
	ui.find_btn->setEnabled(false);
	ui.bar_edit->setMaxLength(25);
	ui.seq_edit->setMaxLength(12);
	ui.rack_edit->setMaxLength(4);
	// modify bug0011799 by wuht 
	for (const auto& edit : { ui.bar_edit ,ui.seq_edit, ui.rack_edit })
	{
		connect(edit, &QLineEdit::textChanged, this, [&]()
		{
			auto barCode = ui.bar_edit->text().toStdString();
			auto seqCode = ui.seq_edit->text().toStdString();
			auto rackCode = ui.rack_edit->text().toStdString();
			if (!barCode.empty() || !seqCode.empty() || !rackCode.empty())
			{
				ui.find_btn->setEnabled(true);
			}
			else
			{
				ui.find_btn->setEnabled(false);
			}
		});
	}

	// 查询
	connect(ui.find_btn, &QPushButton::clicked, this, [&]()
	{
		auto barCode = ui.bar_edit->text().toStdString();
		auto seqCode = ui.seq_edit->text().toStdString();
		auto rackCode = ui.rack_edit->text().toStdString();
		auto sampleData = ui.sample_rack_monitor->FindSample(std::make_tuple(barCode, seqCode, rackCode));
		if (Q_NULLPTR == sampleData)
		{
			ui.sample_detail_view->clearSelection();
			// modify for bug 0011618 by wuht
			ULOG(LOG_ERROR, "Seacher SampleInfo() failed barCode: %s seqCode: %s rackCode: %s", barCode, seqCode, rackCode);
			auto spDiglog = std::make_shared<QCustomDialog>(this, 1000, tr("未查询到符合条件的样本架！"), false);
			if (spDiglog != Q_NULLPTR)
			{
				spDiglog->SetCenter();
				spDiglog->exec();
			}
		}
	});

	// 自动清空功能
	connect(ui.autoclear_btn, &QPushButton::clicked, this, [&]()
	{
		// 重新取一次设置
		if (!DictionaryQueryManager::GetPageset(m_data))
		{
			return;
		}

		m_data.rackAutoClear = ui.autoclear_btn->isChecked();
		if (!DictionaryQueryManager::SavePageset(m_data))
		{
			ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
		}
	});

    connect(ui.addSample_btn, &QPushButton::clicked, this, [&]()
    {
        CoreData data;
        for (int i = 0; i < 5; i++)
        {
            if(qrand() % 3 == 0)
            {
                if (data.first.empty())
                {
                    data.first = to_string(qrand()%1000 + 5000);
                }
                data.second.push_back(Q_NULLPTR);
            }
            else
            {
                auto sampleInfo = DataPrivate::Instance().GetSampleByRow(m_testRow++);
				if (data.first.empty())
				{
					if (sampleInfo.has_value())
					{
						auto rack = sampleInfo.value().rack;
						data.first = to_string(qrand() % 1000 + 5000);
					}
					else
					{
						data.first = to_string(qrand() % 1000 + 5000);
					}
				}

				// 生成扫描失败的样本信息
				std::shared_ptr<tf::SampleInfo> spSample = nullptr;
				if (qrand() % 5 > 2 || !sampleInfo)
				{
					spSample = std::make_shared<tf::SampleInfo>();
					// FAILEDID的代表扫描失败
					spSample->__set_id(FAILEDID);
				}
				else
				{
					spSample = std::make_shared<tf::SampleInfo>(sampleInfo.value());
				}

                data.second.push_back(spSample);
            }
        }

		// 生成架异常的
		if (qrand() % 5 > 1)
		{
			data.first.clear();
		}

		if (m_isPipeLine)
		{
			auto tray = qrand() % 4;
			if (tray == 0)
			{
				tray += 1;
			}
			this->AddNewSampleRack(tray, data);
		}
		else
		{
			this->AddNewSampleRack(1, data);
		}

    });

	// 选中当前样本管
	connect(ui.sample_detail_view, &QTableView::clicked, this, [&](const QModelIndex& current)
	{
		if (!current.isValid() || m_model == Q_NULLPTR)
		{
			ULOG(LOG_INFO, "%s(index is invalid)", __FUNCTION__);
			return;
		}

		int row = current.row();
		int column = current.column();
		ULOG(LOG_INFO, "%s(currentIndex row:%d - column:%d)", __FUNCTION__, row, column);
		if (row >= MAXPOSNUMER || column >= MAXCOLUMN
			|| row < 0 || column < 0)
		{
			return;
		}

		auto sampleData = m_model->GetSampelDateByIndex(current);
		if (sampleData == Q_NULLPTR)
		{
			ui.bar_edit->clear();
			ui.seq_edit->clear();
			ui.rack_edit->clear();
		}
		else
		{
			/// 取消映射关系bug0011172
// 			auto barCode =  QString::fromStdString(sampleData->barcode);
// 			auto seqCode =  QString::fromStdString(sampleData->seqNo);
// 			auto rackCode = QString::fromStdString(sampleData->rack);
// 			ui.bar_edit->setText(barCode);
// 			ui.seq_edit->setText(seqCode);
// 			ui.rack_edit->setText(rackCode);
		}

		ui.sample_rack_monitor->SetSelected(4 - row);
	});

	// 样本架回收消息
	REGISTER_HANDLER(MSG_ID_RACK_RECYCLE_UPDATE, this, OnUpdateRecyCleRackInfo);
	// 自动清空
	REGISTER_HANDLER(MSG_ID_TRACT_STATE_UPDATE, this, OnClearRecyCleRackInfo);
}

void QSampleRackMonitors::ClearData(int trayId)
{
	ui.sample_rack_monitor->Clear(trayId);
	m_testRow = 0;
	auto currentTrayId = ui.sample_rack_monitor->GetCurrentTrayId();
	if (trayId != currentTrayId)
	{
		return;
	}

	if (m_model != Q_NULLPTR)
	{
		m_model->SetSampleData(std::vector<SampleData>{});
	}

	ui.bar_edit->clear();
	ui.seq_edit->clear();
	ui.rack_edit->clear();
	ui.rack_number_label->setText("");
}

void QSampleRackMonitors::AddNewSampleRack(const int32_t trayNo, std::pair<std::string, std::vector<SampleData>>& sampleRackInfo)
{
	if (m_model != Q_NULLPTR)
	{
		m_model->SetSampleData(std::vector<SampleData>{});
	}
	ui.rack_number_label->setText("");
    ui.sample_rack_monitor->AddNewSampleRack(trayNo, sampleRackInfo);
}

void QSampleRackMonitors::showEvent(QShowEvent *event)
{
	// modify bug0013921 by wuht
	POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 工作 > ") + tr("样本架监视"));
}

void QSampleRackMonitors::Init()
{
	if (m_isPipeLine)
	{
		std::set<int32_t> trays = { };
		auto trackConfig = DictionaryQueryManager::GetInstance()->GetTrackConfig();
		for (const auto& info : trackConfig.m_iomSet)
		{
			trays.insert(info.m_id);
		}

		for (const auto tray : trays)
		{
			m_trays.push_back(tray);
		}

		if (trays.empty())
		{
			m_trays = { 1 };
		}

		ui.clear_btn->hide();
		ui.clear_btn_2->hide();
		ui.clear_btn_3->hide();
		if (m_trays.size() >= 1)
		{
			ui.clear_btn->show();
			ui.clear_btn->setProperty("tray", m_trays[0]);
		}

		if (m_trays.size() >= 2)
		{
			ui.clear_btn_2->setProperty("tray", m_trays[1]);
			ui.clear_btn_2->show();
		}

		if (m_trays.size() >= 3)
		{
			ui.clear_btn_3->setProperty("tray", m_trays[2]);
			ui.clear_btn_3->show();
		}

		ui.sample_rack_monitor->SetMonitorTrays(m_trays, m_isPipeLine);
		int size = (m_trays.size() == 0) ? 1 : m_trays.size();
		if (size == 1)
		{
			// 新需求，当只有一个托盘的时候，不需要显示托盘号(0012554)
			ui.plate_text_label->hide();
			ui.platelabel->hide();
			ui.clear_btn->setText(tr("清空"));
		}

		//int width = 373 * size;
		//ui.widget1->setGeometry(QRect(-8, 1, width, 865));
		//ui.widget2->setGeometry(QRect(width - 11, 1, 1914 - width, 865));

// 		int width = 377 * size;
// 		ui.widget1->setGeometry(QRect(-8, 1, width, 865));
// 		ui.widget2->setGeometry(QRect(width - 23, 1, 1926 - width, 865));

		int width = (365 + 4) * size;
		ui.widget1->setGeometry(QRect(1, 1, width, 797));
		ui.widget2->setGeometry(QRect(width+4, 1, 1903 - width-4, 897));
	}
	else
	{		
		// 新需求，当只有一个托盘的时候，不需要显示托盘号(0012554)
		ui.plate_text_label->hide();
		ui.platelabel->hide();
		ui.clear_btn->setText(tr("清空"));
		m_trays = { 1 };
		ui.clear_btn->setProperty("tray", m_trays[0]);
		ui.sample_rack_monitor->SetMonitorTrays(m_trays, m_isPipeLine);
	}

    m_model = new QSampleModel(this);
    ui.sample_detail_view->setModel(m_model);
    ui.sample_detail_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.sample_detail_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.sample_detail_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.sample_detail_view->verticalHeader()->hide();

	ui.sample_detail_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.sample_detail_view->setColumnWidth(0, 80);
	ui.sample_detail_view->setColumnWidth(2, 120);
	ui.sample_detail_view->setColumnWidth(3, 120);
	ui.sample_detail_view->horizontalHeader()->setStretchLastSection(true);
	ui.sample_detail_view->horizontalHeader()->setMinimumSectionSize(80);

	if (!DictionaryQueryManager::GetPageset(m_data))
	{
		ULOG(LOG_ERROR, "Failed to get pageset.");
		return;
	}

	// modify bug0012045 by wuht
	if (m_data.rackAutoClear)
	{
		ui.autoclear_btn->setChecked(true);
	}
	else
	{
		ui.autoclear_btn->setChecked(false);
	}
}

///
/// @brief 样本架回收信息
///
/// @param[in]  rack			样本架号   
/// @param[in]  recycleRackInfo 样本架详细信息  
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月24日，新建函数
///
void QSampleRackMonitors::OnUpdateRecyCleRackInfo(const int trayNo, std::vector<tf::RecycleRackInfo, std::allocator<tf::RecycleRackInfo>> recycleRackInfos)
{
	ULOG(LOG_INFO, "%s(Update tray:%d - content:%s)", __FUNCTION__, trayNo, ToString(recycleRackInfos));
	for (const auto& recycleRackInfo : recycleRackInfos)
	{
		CoreData data;
		// 更新架号
		data.first = recycleRackInfo.rackCode;
		std::vector<SampleData> sampleDataVec;
		sampleDataVec.assign(MAXPOSNUMER, Q_NULLPTR);

		// 生成样本架回收信息
		for (auto& rackInfo : recycleRackInfo.mapPos2Sample)
		{
			if (rackInfo.first < 1 || rackInfo.first > MAXPOSNUMER)
			{
				ULOG(LOG_WARN, "%s(Update Rack:%s - pos:%d)", __FUNCTION__, recycleRackInfo.rackCode, rackInfo.first);
				continue;
			}

			std::shared_ptr<tf::SampleInfo> spSample = Q_NULLPTR;
			auto recycleSampleInfo = rackInfo.second;
			// 当数据库主键>0的时候表明扫描成功，显示样本信息
			if (recycleSampleInfo.id > 0)
			{
				// 获取样本信息
				spSample = DataPrivate::Instance().GetSampleInfoByDb(recycleSampleInfo.id);
				if (spSample == Q_NULLPTR)
				{
					ULOG(LOG_INFO, "%s(Get sample id:%d  is empty)", __FUNCTION__, recycleSampleInfo.id);
					continue;
				}

			}
			// 当数据库主键<=0的时候，查看是否
			else if(recycleSampleInfo.scanFailed)
			{
				spSample = std::make_shared<tf::SampleInfo>();
				// FAILEDID的代表扫描失败
				spSample->__set_id(FAILEDID);
			}
			else
			{
				continue;
			}

			ULOG(LOG_INFO, "%s(%d - %d)", __FUNCTION__, rackInfo.first, recycleSampleInfo.id);
			// 更新架中样本数据
			sampleDataVec[rackInfo.first - 1] = std::make_shared<tf::SampleInfo>(*spSample);
		}

		data.second = sampleDataVec;
		this->AddNewSampleRack(trayNo, data);
	}
}

void QSampleRackMonitors::OnClearRecyCleRackInfo(class track::tf::LowerStatusInfo statusInfo)
{
	ULOG(LOG_INFO, "%s(content: %s)", __FUNCTION__, ToString(statusInfo));
	if (statusInfo.__isset.sampleOutTrayPlaced && !statusInfo.sampleOutTrayPlaced)
	{
		bool isChecked = ui.autoclear_btn->isChecked();
		if (isChecked)
		{
			ui.sample_rack_monitor->Clear(1);
			m_testRow = 0;
			if (m_model != Q_NULLPTR)
			{
				m_model->SetSampleData(std::vector<SampleData>{});
			}

			ui.bar_edit->clear();
			ui.seq_edit->clear();
			ui.rack_edit->clear();
		}
	}
}


QSampleModel::QSampleModel(QObject *parent /*= Q_NULLPTR*/, int rowCount /*= 5*/)
    :QAbstractTableModel(parent)
    , m_rowCount(rowCount)
{
    m_headrData << tr("位置") << tr("样本条码") << tr("样本号") << tr("样本类型") << tr("状态") /*<< tr("异常原因")*/;
    m_sampleData.assign(MAXPOSNUMER, Q_NULLPTR);
	REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnUpdateSample);
}

QVariant QSampleModel::GetDispaly(SampleData& sampleInfo, int column) const
{
	if (sampleInfo == Q_NULLPTR)
	{
		return QVariant();
	}

	// 获取样本的状态类型
	auto GetSampleStatus = [&](SampleData& sampleInfo)->QString
	{
		if (sampleInfo->id == FAILEDID)
		{
			return QString(tr("扫码失败"));
		}

		switch (sampleInfo->status)
		{
			case tf::SampleStatus::SAMPLE_STATUS_PENDING :
			{
				return QString(tr("待测"));
			}

			case tf::SampleStatus::SAMPLE_STATUS_TESTING:
			{
				return QString(tr("测试中"));
			}

			case tf::SampleStatus::SAMPLE_STATUS_TESTED:
			{
				return QString(tr("已完成"));
			}

			default:
				return QString(tr("未知状态"));
				break;
		}
	};

    switch (column)
    {
        case 1:
            return QString::fromStdString(sampleInfo->barcode);
        case 2:
            return QString::fromStdString(sampleInfo->seqNo);
        case 3:
            return ThriftEnumTrans::GetSourceTypeName(sampleInfo->sampleSourceType, QString(tr("未知类型")));
        case 4:
            return GetSampleStatus(sampleInfo);
        default:
            return QVariant();
            break;
    }
}

SampleData QSampleModel::GetSampelDateByIndex(const QModelIndex& index)
{
	if (!index.isValid())
	{
		return Q_NULLPTR;
	}

	int row	   = index.row();
	int column = index.column();
	ULOG(LOG_INFO, "%s(CurrentIndex row:%d - column:%d)", __FUNCTION__, row, column);
	int size = m_sampleData.size();
	if (row >= size || column >= MAXCOLUMN
		|| row < 0 || column < 0)
	{
		ULOG(LOG_INFO, "%s(Out of range)", __FUNCTION__);
		return Q_NULLPTR;
	}

	return m_sampleData[row];
}

QVariant QSampleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    auto row = index.row();
    auto column = index.column();
    if (row < 0 || row >= m_sampleData.size())
    {
        return QVariant();
    }

    // 获取数据
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {

        // 位置
        if (column == 0)
        {
            return QVariant(row+1);
        }

        auto data = m_sampleData[row];
        if (data == Q_NULLPTR)
        {
            return QVariant();
        }
        else
        {
            return GetDispaly(data, column);
        }
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    return QVariant();
}

void QSampleModel::OnUpdateSample(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs)
{
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

		auto row = iter->second;
		if (row < 0 || row >= m_sampleData.size())
		{
			continue;
		}

		m_sampleData[row] = std::make_shared<tf::SampleInfo>(sample);
		auto indexstart = this->index(row, 0);
		auto indexend = this->index(row, this->columnCount(QModelIndex()) - 1);
		emit dataChanged(indexstart, indexend, { Qt::DisplayRole });
	}
}

QVariant QSampleModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {

            if (section < 0 || section >= m_headrData.size())
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

QSampleRackMonitorsUi::QSampleRackMonitorsUi()
{
	m_isPipeLine = false;
	// 是否联机模式（待完善）
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		m_isPipeLine = true;
	}
	// m_isPipeLine = true;
	//m_isPipeLine = true;
}

void QSampleRackMonitorsUi::setupUi(QWidget *QSampleRackMonitors, bool isPipeLine)
{
	if (!isPipeLine)
	{
		ui.setupUi(QSampleRackMonitors);
		sample_rack_monitor = ui.sample_rack_monitor;
		sample_detail_view = ui.sample_detail_view;
		//reason_label = ui.reason_label;
		addSample_btn = ui.addSample_btn;
		widget = ui.widget;
		gridLayout_2 = ui.gridLayout_2;
		horizontalLayout_6 = ui.horizontalLayout_6;
		legend_sample_empty = ui.legend_sample_empty;
		label_6 = ui.label_6;
		horizontalSpacer_5 = ui.horizontalSpacer_5;
		verticalSpacer = ui.verticalSpacer;
		horizontalLayout_8 = ui.horizontalLayout_8;
		legend_sample_testing = ui.legend_sample_testing;
		label_10 = ui.label_10;
		horizontalSpacer_7 = ui.horizontalSpacer_7;
		horizontalLayout_10 = ui.horizontalLayout_10;
		legend_rack_abnormal = ui.legend_rack_abnormal;
		label_14 = ui.label_14;
		horizontalSpacer_9 = ui.horizontalSpacer_9;
		horizontalLayout_9 = ui.horizontalLayout_9;
		legend_sample_abnormal = ui.legend_sample_abnormal;
		label_12 = ui.label_12;
		horizontalSpacer_8 = ui.horizontalSpacer_8;
		horizontalLayout_7 = ui.horizontalLayout_7;
		legend_sample_finished = ui.legend_sample_finished;
		label_8 = ui.label_8;
		horizontalSpacer_6 = ui.horizontalSpacer_6;
		horizontalLayout_11 = ui.horizontalLayout_11;
		legend_sample_to_be_tested = ui.legend_sample_to_be_tested;
		label_4 = ui.label_4;
		horizontalSpacer_4 = ui.horizontalSpacer_4;
		widget1 = ui.widget1;
		widget11 = Q_NULLPTR;
		horizontalLayout = ui.horizontalLayout;
		autoclear_btn = ui.autoclear_btn;
		horizontalSpacer = ui.horizontalSpacer;
		clear_btn = ui.clear_btn;
		layoutWidget = ui.layoutWidget;
		gridLayout = ui.gridLayout;
		seq_edit = ui.seq_edit;
		label_2 = ui.label_2;
		label_3 = ui.label_3;
		horizontalLayout_2 = ui.horizontalLayout_2;
		bar_edit = ui.bar_edit;
		find_btn = ui.find_btn;
		horizontalSpacer_2 = ui.horizontalSpacer_2;
		label = ui.label;
		rack_edit = ui.rack_edit;
		verticalSpacer_2 = ui.verticalSpacer_2;
		widget2 = ui.widget2;
		horizontalLayout_5 = ui.horizontalLayout_5;
		horizontalLayout_3 = ui.horizontalLayout_3;
		plate_text_label = ui.plate_text_label;
		platelabel = ui.platelabel;
		horizontalLayout_4 = ui.horizontalLayout_4;
		rack_number_text_label = ui.rack_number_text_label;
		rack_number_label = ui.rack_number_label;
		horizontalSpacer_3 = ui.horizontalSpacer_3;
	}
	else
	{
		ui_onLine.setupUi(QSampleRackMonitors);
		sample_rack_monitor = ui_onLine.sample_rack_monitor;
		sample_detail_view = ui_onLine.sample_detail_view;
		//reason_label = ui_onLine.reason_label;
		addSample_btn = ui_onLine.addSample_btn;
		//bk_left_bottom_label = ui_onLine.bk_left_bottom_label;
		widget = ui_onLine.widget;
		gridLayout_2 = ui_onLine.gridLayout_2;
		horizontalLayout_6 = ui_onLine.horizontalLayout_6;
		legend_sample_empty = ui_onLine.legend_sample_empty;
		label_6 = ui_onLine.label_6;
		//horizontalSpacer_5 = ui_onLine.horizontalSpacer_5;
		//verticalSpacer		= ui_onLine.verticalSpacer;
		horizontalLayout_8 = ui_onLine.horizontalLayout_8;
		legend_sample_testing = ui_onLine.legend_sample_testing;
		label_10 = ui_onLine.label_10;
		/*horizontalSpacer_7 = ui_onLine.horizontalSpacer_7;*/
		horizontalLayout_10 = ui_onLine.horizontalLayout_10;
		legend_rack_abnormal = ui_onLine.legend_rack_abnormal;
		label_14 = ui_onLine.label_14;
		/*horizontalSpacer_9 = ui_onLine.horizontalSpacer_9;*/
		horizontalLayout_9 = ui_onLine.horizontalLayout_9;
		legend_sample_abnormal = ui_onLine.legend_sample_abnormal;
		label_12 = ui_onLine.label_12;
		//horizontalSpacer_8 = ui_onLine.horizontalSpacer_8;
		horizontalLayout_7 = ui_onLine.horizontalLayout_7;
		legend_sample_finished = ui_onLine.legend_sample_finished;
		label_8 = ui_onLine.label_8;
		/*horizontalSpacer_6 = ui_onLine.horizontalSpacer_6;*/
		horizontalLayout_11 = ui_onLine.horizontalLayout_11;
		legend_sample_to_be_tested = ui_onLine.legend_sample_to_be_tested;
		label_4 = ui_onLine.label_4;
		//horizontalSpacer_4 = ui_onLine.horizontalSpacer_4;
		widget1 = ui_onLine.widget1;
		widget11 = ui_onLine.widget11;
		horizontalLayout = ui_onLine.horizontalLayout;
		autoclear_btn = ui_onLine.autoclear_btn;
		horizontalSpacer = ui_onLine.horizontalSpacer;
		clear_btn = ui_onLine.clear_btn;
		clear_btn_2 = ui_onLine.clear_btn_2;
		clear_btn_3 = ui_onLine.clear_btn_3;
		gridLayout = ui_onLine.gridLayout;
		seq_edit = ui_onLine.seq_edit;
		label_2 = ui_onLine.label_2;
		label_3 = ui_onLine.label_3;
		horizontalLayout_2 = ui_onLine.horizontalLayout_2;
		bar_edit = ui_onLine.bar_edit;
		find_btn = ui_onLine.find_btn;
		horizontalSpacer_2 = ui_onLine.horizontalSpacer_2;
		label = ui_onLine.label;
		rack_edit = ui_onLine.rack_edit;
		verticalSpacer_2 = ui_onLine.verticalSpacer_2;
		widget2 = ui_onLine.widget2;
		horizontalLayout_5 = ui_onLine.horizontalLayout_5;
		horizontalLayout_3 = ui_onLine.horizontalLayout_3;
		plate_text_label = ui_onLine.plate_text_label;
		platelabel = ui_onLine.platelabel;
		horizontalLayout_4 = ui_onLine.horizontalLayout_4;
		rack_number_text_label = ui_onLine.rack_number_text_label;
		rack_number_label = ui_onLine.rack_number_label;
		horizontalSpacer_3 = ui_onLine.horizontalSpacer_3;
	}

	m_isPipeLine = isPipeLine;
}

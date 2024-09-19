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

#include "QReCheckDialog.h"
#include "ui_QReCheckDialog.h"
#include "ui_QReCheckSimpleDialog.h"
#include "QSampleAssayModel.h"

#include <QComboBox>
#include <QStandardItemModel>
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/QComDelegate.h"
#include "manager/DictionaryQueryManager.h"
#include "workplace/WorkpageCommon.h"

#include "utility/UtilityCommonFunctions.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"
#include "thrift/DcsControlProxy.h"

QReCheckDialog::QReCheckDialog(QWidget *parent)
    : BaseDlg(parent)
    , m_sampleInfo(boost::none)
    , m_mode(Mode::dilu_mode)
    , m_recheckMode(nullptr)
{
    ui = new Ui::QReCheckDialog();
    ui_simple = new Ui::QReCheckSimpleDialog();

    setFocusPolicy(Qt::NoFocus);
}

QReCheckDialog::~QReCheckDialog()
{
}

///
/// @brief 设置样本Id
///
/// @param[in]  sampleId  样本Id
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月22日，新建函数
///
void QReCheckDialog::SetSample(tf::SampleInfo& sampleInput)
{
    m_sampleInfo = boost::none;
    ClearStatus();

    // 根据样本ID查询样本
    ::tf::SampleInfoQueryCond sampleCond;
    ::tf::SampleInfoQueryResp sampleResult;
    sampleCond.__set_id(sampleInput.id);
    sampleCond.__set_containTestItems(true);

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampleCond, sampleResult)
        || sampleResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || sampleResult.lstSampleInfos.empty())
    {
        ULOG(LOG_ERROR, "%s(), QuerySampleInfo failed.", __FUNCTION__);
        return;
    }

    // 获取样本信息
    auto sampleInfo = sampleResult.lstSampleInfos[0];
    m_sampleInfo = sampleInfo;

    // 单项目
    if (sampleInput.__isset.testItems)
    {
        m_sampleInfo->__set_testItems(sampleInput.testItems);
    }

    // 多项目
    if (sampleInput.__isset.profiles)
    {
        m_sampleInfo->__set_profiles(sampleInput.profiles);
    }

    // 简单模式（根据Bug1644,样本申请页面不显示架号，位置号）
    if (m_mode == Mode::simple_mode)
    {
        return;
    }

    // 架子号
	/*
    if (sampleInfo.__isset.rack)
    {
        if (m_mode == Mode::simple_mode)
        {
            ui_simple->rack_Edit->setText(QString::fromStdString(sampleInfo.rack));
        }
        else
        {
            ui->rack_Edit->setText(QString::fromStdString(sampleInfo.rack));
        }
    }
    // 位置号
    if (sampleInfo.__isset.pos && sampleInfo.pos >= 1 && sampleInfo.pos <= 5)
    {
        auto posList = findChildren<QPushButton*>();
        for (const auto& pos_btn : posList)
        {
            if (pos_btn->objectName().contains("_" + QString::number(sampleInfo.pos)))
            {
                emit pos_btn->setChecked(true);
            }
        }
    }*/

    // 获取对应的模式
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->assay_view->model());
    if (model == Q_NULLPTR)
    {
        return;
    }

	// 获取已经选中的项目的
	m_selectedItems = QSampleDetailModel::Instance().GetTestSelectedItems();
    model->clear();
    model->setRowCount(m_selectedItems.size());
    model->setHorizontalHeaderLabels({ tr("项目"),tr("样本量/稀释倍数") });

    int iRow = 0;
    for (auto item : m_selectedItems)
    {
		auto spTestItem = item.second;
		if (spTestItem == Q_NULLPTR)
		{
			continue;
		}

        if (!WorkpageCommon::IsItemReCheckStatus(*spTestItem))
        {
            continue;
        }

        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(spTestItem->assayCode);
        if (spAssay == Q_NULLPTR)
        {
            continue;
        }

		// 项目名称
		auto nameItem = new QStandardItem(QString::fromStdString(spAssay->assayName));
		nameItem->setTextAlignment(Qt::AlignCenter);
		// 存储项目的数据库主键信息
		nameItem->setData(spTestItem->id);
        model->setItem(iRow, 0, nameItem);

		auto spCombox = GetComBoxFromTestItem(spTestItem, m_dataMap);
		if (spCombox == Q_NULLPTR)
		{
			continue;
		}

        // 稀释倍数
        ui->assay_view->setFocusPolicy(Qt::NoFocus);
        ui->assay_view->setIndexWidget(model->index(iRow,1), spCombox);
        ++iRow;
    }
}

bool QReCheckDialog::IsSpecialAssay(int assayCode)
{
	// 是否ISE
	if (CommonInformationManager::GetInstance()->IsAssayISE(assayCode))
	{
		return true;
	}

	// 是否血清指数
	if (CommonInformationManager::GetInstance()->IsAssaySIND(assayCode))
	{
		return true;
	}

	// 是否糖化血红蛋白
	if (assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_HBA1c)
	{
		return true;
	}

	return false;
}

QComboBox* QReCheckDialog::GetComBoxFromTestItem(std::shared_ptr<tf::TestItem> spTestItem, const ValueType& dataMap)
{
	if (spTestItem == Q_NULLPTR)
	{
		ULOG(LOG_WARN, "%s( the testItme is NULL)", __FUNCTION__);
		return Q_NULLPTR;
	}

	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(spTestItem->assayCode);
	if (spAssay == Q_NULLPTR)
	{
		ULOG(LOG_WARN, "%s( can not find the gerneral info assaycode : %d)", __FUNCTION__, spTestItem->assayCode);
		return Q_NULLPTR;
	}

	QComboBox* comBox;
	comBox = new QComboBox(this);
	auto lineEdit = new QLineEdit(comBox);
	lineEdit->setFrame(false);
	comBox->setLineEdit(lineEdit);
	comBox->lineEdit()->setAlignment(Qt::AlignCenter);
	comBox->setFocusPolicy(Qt::NoFocus);

	// 特殊项目（ISE,SIND,糖化）
	if (IsSpecialAssay(spTestItem->assayCode))
	{
		if (dataMap.count(ASSAYTYPE::SPECIAL) <= 0)
		{
			return Q_NULLPTR;
		}

		auto mapType = dataMap[ASSAYTYPE::SPECIAL].toStdMap();
		int index = 0;
		for (const auto& data : mapType)
		{
			comBox->addItem(data.second, data.first);
			static_cast<QStandardItemModel*>(comBox->model())->item(index++)->setTextAlignment(Qt::AlignCenter);
		}

		comBox->setCurrentIndex(0);
	}
	// 生化
	else if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
	{
		if (dataMap.count(ASSAYTYPE::CHEMISTRY) <= 0)
		{
			return Q_NULLPTR;
		}

		auto mapType = dataMap[ASSAYTYPE::CHEMISTRY].toStdMap();
		int index = 0;
		for (const auto& data : mapType)
		{
			comBox->addItem(data.second, data.first);
			static_cast<QStandardItemModel*>(comBox->model())->item(index++)->setTextAlignment(Qt::AlignCenter);
		}

		// 此处根据当前项目的情况设置稀释倍数
		// 数据浏览中的稀释倍数设置只能设置机内稀释，不能设置手工稀释
		if (spTestItem->dilutionFactor <= 1 && spTestItem->__isset.suckVolType)
		{
			comBox->setCurrentIndex(spTestItem->suckVolType);
		}
		else
		{
			int index = 0;
			for (auto& data : mapType)
			{
				// 设置稀释比例
				if (index > 2 && data.first == spTestItem->dilutionFactor)
				{
					comBox->setCurrentIndex(index);
				}

				index++;
			}
		}

	}
	// 免疫项目
	else
	{
		if (dataMap.count(ASSAYTYPE::IMMUNE) <= 0)
		{
			return Q_NULLPTR;
		}

		auto mapType = dataMap[ASSAYTYPE::IMMUNE].toStdMap();
		for (const auto& data : mapType)
		{
			comBox->addItem(data.second, data.first);
		}

		if (spTestItem->dilutionFactor <= 1 && spTestItem->__isset.suckVolType)
		{
			// 免疫只有常量
			comBox->setCurrentIndex(0);
		}
		else
		{
			int index = 0;
			for (auto& data : mapType)
			{
				// 设置稀释比例
				if (index > 0 && data.first == spTestItem->dilutionFactor)
				{
					comBox->setCurrentIndex(index);
				}

				index++;
			}
		}
	}

	return comBox;
}

///
/// @brief 设置模式
///
/// @param[in]  mode  模式
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月23日，新建函数
///
void QReCheckDialog::SetMode(Mode mode)
{
    m_mode = mode;
    if (m_mode == Mode::dilu_mode)
    {
        ui->setupUi(this);
    }
    else
    {
        ui_simple->setupUi(this);
    }

    Init();
}

void QReCheckDialog::SetTestItemAttribute(tf::TestItem& testItem, const QComboBox* duliCombox)
{
	if (duliCombox == Q_NULLPTR)
	{
		return;
	}

	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(testItem.assayCode);
	if (spAssay == Q_NULLPTR)
	{
		return;
	}

	// 先设置为常量-减量-增量
	// 先设置手工稀释失效，因为复查对话框中无法设置手工稀释（bug0012408）
	testItem.__set_preDilutionFactor(1);
	testItem.__set_dilutionFactor(1);
	testItem.__set_rerun(true);
	testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
	// 生化项目
	int value = duliCombox->currentData().toInt();
	if (spAssay->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
	{
		switch (value)
		{
			// 常量
			case 0:
				testItem.__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_STD);
				break;
				// 减量
			case 1:
				testItem.__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_DEC);
				break;
				// 增量
			case 2:
				testItem.__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_INC);
				break;
			default:
				// 用户自定义
				testItem.__set_dilutionFactor(value);
				break;
		}
	}
	// 免疫项目
	else if (spAssay->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
	{
		switch (value)
		{
			// 常量
			case 0:
				testItem.__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_STD);
				break;
			default:
				// 用户自定义
				testItem.__set_dilutionFactor(value);
				break;
		}
	}
}

///
/// @brief 保存当前数据
///
///
/// @return true 保存成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月13日，新建函数
///
bool QReCheckDialog::OnSaveData()
{
    QString rackText;
    if (m_mode == Mode::dilu_mode)
    {
        rackText = ui->rack_Edit->text();
    }
    else
    {
        rackText = ui_simple->rack_Edit->text();
    }

    if (rackText.isEmpty())
    {
        TipDlg(tr("提示"), tr("架号不能为空")).exec();
        return false;
    }

	// 首先确定架号是否在复查架范围内
	bool IsRackOutOfrange = CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_RERUN, rackText.toInt());
	if (IsRackOutOfrange && m_sampleInfo)
	{
		int rackData;
		if (stringutil::IsInteger(m_sampleInfo->rack, rackData))
		{
			// 急诊，可以使用急诊的范围内架号
			if (m_sampleInfo->stat)
			{
				IsRackOutOfrange = CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_STAT, rackText.toInt());
			}
			// 常规
			else
			{
				IsRackOutOfrange = CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_ROUTINE, rackText.toInt());
			}
		}
	}

    // 判断架号是否超出范围
    if (IsRackOutOfrange)
    {
        TipDlg(tr("提示"), tr("架号超出范围")).exec();
        return false;
    }

	// 是否需要判断位置被占用（当不改变位置的时候不需要）
	bool isJudgeOccupy = true;
    int postion = GetSamplePostion();
	if (m_sampleInfo->__isset.rack && m_sampleInfo->__isset.pos)
	{
		if (m_sampleInfo->rack == rackText.toStdString() && m_sampleInfo->pos == postion)
		{
			isJudgeOccupy = false;
		}
	}

	// 需要判断位置是否被占用
	// if (isJudgeOccupy)
	{
		auto spSample = DataPrivate::Instance().GetSampleByRackAndPsotion(m_sampleInfo->testMode, rackText.toStdString(), postion);
		if (spSample != Q_NULLPTR
			&& spSample->id != m_sampleInfo->id
			// bug0013497
			&& (spSample->status == tf::SampleStatus::type::SAMPLE_STATUS_PENDING
			|| spSample->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING))
		{
			TipDlg(tr("提示"), tr("存在相同架号位置号的样本")).exec();
			return false;
		}
	}

	// 设置架号位置号，无论是简洁模式还是稀释模式
	m_sampleInfo->__set_rack(rackText.toStdString());
	if (postion > 0)
	{
		m_sampleInfo->__set_pos(postion);
	}
	else
	{
		TipDlg(tr("提示"), tr("请正确设置位置参数")).exec();
		return false;
	}

	// 稀释模式
	if (m_mode == Mode::dilu_mode)
	{
		QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->assay_view->model());
		if (model == Q_NULLPTR)
		{
			return false;
		}

		auto& testItems = m_sampleInfo.value().testItems;
		for (int i = 0; i < model->rowCount(); i++)
		{
			// 获取对应的数据库主键
			auto id = model->item(i, 0)->data().toInt();
			// 获取对应的稀释combox
			QComboBox* comBox = qobject_cast<QComboBox*>(ui->assay_view->indexWidget(model->index(i, 1)));
			if (comBox == Q_NULLPTR)
			{
				continue;
			}

			// 找到对应的项目信息，然后更新
			for (auto& testItem : testItems)
			{
				if (testItem.id != id)
				{
					continue;
				}

				// 设置项目稀释倍数
				SetTestItemAttribute(testItem, comBox);
				break;
			}
		}
	}


    // 稀释模式
	/*
    if (m_mode == Mode::dilu_mode)
    {
        // 设置架号
        m_sampleInfo->__set_rack(ui->rack_Edit->text().toStdString());
        int postion = GetSamplePostion();
        if (postion > 0)
        {
            m_sampleInfo->__set_pos(postion);
        }

        QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->assay_view->model());
        if (model == Q_NULLPTR)
        {
            return false;
        }

        std::vector<tf::TestItem> items;
        for (int i = 0; i < m_sampleInfo->testItems.size(); i++)
        {
            auto item = model->item(i, 0);
            if (item == Q_NULLPTR)
            {
                continue;
            }

            auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(item->text().toStdString());
            if (spAssay == Q_NULLPTR)
            {
                continue;
            }

            auto iter = std::find_if(m_sampleInfo.value().testItems.begin(), m_sampleInfo.value().testItems.end(),
                [&](auto item)->bool {return item.assayCode == spAssay->assayCode; });

            if (iter != m_sampleInfo.value().testItems.end())
            {
                QComboBox* comBox = qobject_cast<QComboBox*>(ui->assay_view->indexWidget(model->index(i, 1)));
                if (comBox == Q_NULLPTR)
                {
                    continue;
                }

                // 先设置为常量-减量-增量
                iter->__set_dilutionFactor(1);
                // 生化项目
                int value = comBox->currentData().toInt();
                if (spAssay->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
                {
                    switch (value)
                    {
                        // 常量
                        case 0:
                            iter->__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_STD);
                            break;
                            // 减量
                        case 1:
                            iter->__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_DEC);
                            break;
                            // 增量
                        case 2:
                            iter->__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_INC);
                            break;
                        default:
                            // 用户自定义
                            iter->__set_dilutionFactor(value);
                            break;
                    }
                }

                // 免疫项目
                if (spAssay->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
                {
                    switch (value)
                    {
                        // 常量
                        case 0:
                            iter->__set_suckVolType(tf::SuckVolType::SUCK_VOL_TYPE_STD);
                            break;
                        default:
                            // 用户自定义
                            iter->__set_dilutionFactor(value);
                            break;
                    }
                }

            }
        }
    }
    else
    {
        // 设置架号
        m_sampleInfo->__set_rack(ui_simple->rack_Edit->text().toStdString());
        int postion = GetSamplePostion();
        if (postion > 0)
        {
            m_sampleInfo->__set_pos(postion);
        }
    }*/

    // 修改样本
    if (!DcsControlProxy::GetInstance()->ModifySampleInfo(m_sampleInfo.value()))
    {
        ULOG(LOG_ERROR, "%s(), ModifySampleInfo failed.", __FUNCTION__);
        return false;
    }

    return true;
}

///
/// @brief 初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月22日，新建函数
///
void QReCheckDialog::Init()
{
    SetTitleName(tr("复查"));

    if (m_mode == Mode::dilu_mode)
    {
        ui->assay_view->setModel(new QStandardItemModel(this));
        ValueType dataMap;
        // 生化
        {
            QMap<int, QString> map;
            map.insert(0, tr("常量"));
            map.insert(1, tr("减量"));
            map.insert(2, tr("增量"));
            map.insert(3, "3");
            map.insert(5, "5");
            map.insert(10, "10");
            map.insert(20, "20");
            map.insert(50, "50");
            m_dataMap.insert(ASSAYTYPE::CHEMISTRY, map);
        }

        // 免疫
        {
            QMap<int, QString> map;
            map.insert(0, tr("原倍"));

            QStringList duliList = utilcomm::GetDuliRatioListIm();
            for (const QString& duli : duliList)
            {
                map.insert(duli.toInt(), duli);
            }
            m_dataMap.insert(ASSAYTYPE::IMMUNE, map);
        }

		// 特殊项目（SIND,ISE,糖化）
		{
			QMap<int, QString> map;
			map.insert(0, tr("常量"));
			m_dataMap.insert(ASSAYTYPE::SPECIAL, map);
		}

        ui->assay_view->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->assay_view->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->assay_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->assay_view->verticalHeader()->hide();
        ui->assay_view->setFocusPolicy(Qt::NoFocus);
        //ui->rack_Edit->setFocusPolicy(Qt::NoFocus);
        connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() {this->close(); });
        connect(ui->ok_btn, &QPushButton::clicked, this, [&]() 
        {
            if(this->OnSaveData())
            this->close(); 
        });

        ui->ok_btn->setFocus();
        ui->cancel_btn->setFocusPolicy(Qt::NoFocus);

		m_postionButton.clear();
		m_postionButton.push_back(ui->pos_1);
		m_postionButton.push_back(ui->pos_2);
		m_postionButton.push_back(ui->pos_3);
		m_postionButton.push_back(ui->pos_4);
		m_postionButton.push_back(ui->pos_5);
		for (auto& button : m_postionButton)
		{
			connect(button, &QPushButton::clicked, this, [&]() 
			{
				for (auto& Otherbutton : m_postionButton)
				{
					if (Otherbutton != button)
					{
						Otherbutton->setChecked(false);
						Otherbutton->clearFocus();
					}
				}
			});
		}
    }
    else
    {
        //ui_simple->rack_Edit->setFocusPolicy(Qt::NoFocus);
        connect(ui_simple->cancel_btn, &QPushButton::clicked, this, [&]() {this->close(); });
        connect(ui_simple->ok_btn, &QPushButton::clicked, this, [&]() 
        {
            if (this->OnSaveData())
            this->close(); 
        });

        ui_simple->ok_btn->setFocus();
        ui_simple->cancel_btn->setFocusPolicy(Qt::NoFocus);

		m_postionButton.clear();
		m_postionButton.push_back(ui_simple->pos_1);
		m_postionButton.push_back(ui_simple->pos_2);
		m_postionButton.push_back(ui_simple->pos_3);
		m_postionButton.push_back(ui_simple->pos_4);
		m_postionButton.push_back(ui_simple->pos_5);
		for (auto& button : m_postionButton)
		{
			button->clearFocus();
			button->setFocusPolicy(Qt::NoFocus);
			connect(button, &QPushButton::clicked, this, [&]()
			{
				for (auto& Otherbutton : m_postionButton)
				{
					if (Otherbutton != button)
					{
						Otherbutton->setChecked(false);
						Otherbutton->clearFocus();
					}
				}
			});
		}
    }
}

///
/// @brief 获取样本位置
///
///
/// @return 样本位置
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月27日，新建函数
///
int QReCheckDialog::GetSamplePostion()
{
    int postion = -1;

    if (m_mode == Mode::dilu_mode)
    {
        if (ui->pos_1->isChecked())
        {
            postion = 1;
        }
        else if (ui->pos_2->isChecked())
        {
            postion = 2;
        }
        else if (ui->pos_3->isChecked())
        {
            postion = 3;
        }
        else if (ui->pos_4->isChecked())
        {
            postion = 4;
        }
        else if (ui->pos_5->isChecked())
        {
            postion = 5;
        }
    }
    else
    {
        if (ui_simple->pos_1->isChecked())
        {
            postion = 1;
        }
        else if (ui_simple->pos_2->isChecked())
        {
            postion = 2;
        }
        else if (ui_simple->pos_3->isChecked())
        {
            postion = 3;
        }
        else if (ui_simple->pos_4->isChecked())
        {
            postion = 4;
        }
        else if (ui_simple->pos_5->isChecked())
        {
            postion = 5;
        }
    }

    return postion;
}

///
/// @brief 清除状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月9日，新建函数
///
void QReCheckDialog::ClearStatus()
{
	// 位置号
	auto posList = findChildren<QPushButton*>();
	int btnStart = 1;
	for (auto& pos_btn : posList)
	{
		pos_btn->setChecked(false);
		pos_btn->clearFocus();
	}

    // 架号
    if (m_mode == Mode::simple_mode)
    {
        ui_simple->rack_Edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_RACK_NUM), this));
        ui_simple->rack_Edit->clear();
    }
    else
    {
        ui->rack_Edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_RACK_NUM), this));
        ui->rack_Edit->clear();
    }
}

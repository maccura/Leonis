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
/// @file     functionConfigDlg.cpp
/// @brief    应用--仪器--功能管理-功能配置
///
/// @author   7951/LuoXin
/// @date     2023年8月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年8月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "functionConfigDlg.h"
#include "ui_functionConfigDlg.h"
#include <QStandardItemModel>
#include "QCheckBox"
#include "QHeadSortDisplay.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ConfigSerialize.h"

#define CH_TQBWIDGET_INDEX         (0)             // 生化页面索引
#define IM_TQBWIDGET_INDEX         (1)             // 免疫页面索引
#define INDEX                      "index"         // 功能管理单项索引

functionConfigDlg::functionConfigDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::functionConfigDlg)
    , m_leftModel(Q_NULLPTR)
    , m_rightModel(Q_NULLPTR)
{
    ui->setupUi(this);
    Init();
}

functionConfigDlg::~functionConfigDlg()
{

}

QString functionConfigDlg::GetStringByIndex(int index)
{
    QString name;
    switch (index)
    {
        case ChFunctionItemId::CH_CLOT_DETECTION:
            name = tr("凝块检查");
            break;
        case ChFunctionItemId::LOAD_REAGENT_ONLINE:
            name = tr("试剂在线加载");
            break;
        case ChFunctionItemId::START_CUP_NO_CHANGED:
            name = tr("反应杯起始杯号变更");
            break;
        case ChFunctionItemId::INCUBATION_TP:
            name = tr("孵育温度检查");
            break;
        case ChFunctionItemId::ULTRASONIC_MIXED:
            name = tr("超声混匀");
            break;
        case ChFunctionItemId::REAGENT_PROBE_RUN:
            name = tr("试剂针运行");
            break;
        case ChFunctionItemId::SAMPLE_PROBE_RUN:
            name = tr("加样针运行");
            break;
        case ChFunctionItemId::PIPELINE_BUBBLES:
            name = tr("管路气泡检查");
            break;
        case ChFunctionItemId::ELECTRONIC_COVER:
            name = tr("前后防护罩电子锁");
            break;
        case ChFunctionItemId::DEGAS_CHECK:
            name = tr("脱气检查");
            break;
        case ChFunctionItemId::DEVICE_SLEEP:
            name = tr("休眠");
            break;
        case ChFunctionItemId::CLEAN_STATION:
            name = tr("清洗站");
            break;
        case ChFunctionItemId::MODULE_ENABLE:
            name = tr("模块启用");
            break;
		case ImFunctionItemId::IM_CLOT_DETECTION:
			name = tr("样本凝块检测");
			break;
		case ImFunctionItemId::IM_BACK_CLAMSHELL_FAN_DETECTION:
			name = tr("风扇检测-后翻盖风扇");
			break;
		case ImFunctionItemId::IM_BACK_COVER_FAN_DETECTION:
			name = tr("风扇检测-后盖板风扇");
			break;
		case ImFunctionItemId::IM_CONTROL_BOX_FAN_DETECTION:
			name = tr("风扇检测-控制箱风扇");
			break;
		case ImFunctionItemId::IM_WATER_FAN_DETECTION:
			name = tr("风扇检测-水冷风扇");
			break;
		case ImFunctionItemId::IM_POWER_BOX_FAN_DETECTION:
			name = tr("风扇检测-电源箱风扇");
			break;
        case ImFunctionItemId::IM_SUB_BULLER_SENSOR:
            name = tr("气泡检测-底物液管路");
            break;
		case ImFunctionItemId::IM_CONC_BULLER_SENSOR:
			name = tr("气泡检测-清洗缓冲液管路");
			break;
		case ImFunctionItemId::IM_REAG_WASH_PIPETTOR_BULLER_SENSOR:
			name = tr("气泡检测-试剂管路");
			break;
		case ImFunctionItemId::IM_SUCK_SAMPLE_WASH_POOL_LEAKAGE_DETECTION:
			name = tr("加样洗针池漏液检测");
			break;
		case ImFunctionItemId::IM_REAG_ARM_2ND_WASH_POOL_LEAKAGE_DETECTION:
			name = tr("第二试剂臂洗针池漏液检测");
			break;
		case ImFunctionItemId::IM_MIDDLE_BORD_LEAKAGE_DETECTION:
			name = tr("试剂储存系统漏液检测");
			break;
		case ImFunctionItemId::IM_FLUID_COMPONENT_LEAKAGE_DETECTION:
			name = tr("液路组件漏液检测");
			break;
		case ImFunctionItemId::IM_CONFECT_LEAKAGE_DETECTION:
			name = tr("清洗液在线配制漏液检测");
			break;
        case ImFunctionItemId::IM_SAMPLE_TUPE_DETECTION:
            name = tr("样本管类型判断");
            break;
        case ImFunctionItemId::IM_PRESSURE_DETECTION:
            name = tr("样本洗针压力检测");
            break;
		case ImFunctionItemId::IM_BACK_COVER_SWITCH_DETECTION:
			name = tr("后翻盖开关检测");
			break;
        case ImFunctionItemId::IM_AUTO_LOAD_REAGENT:
            name = tr("试剂在线加载");
            break;
		case ImFunctionItemId::IM_BUZZER_VOICE:
			name = tr("蜂鸣器报警音");
			break;
		case ImFunctionItemId::IM_ULTRASONIC_WASH_PIPETTOR:
			name = tr("超声洗针");
			break;
        default:
            break;
    }

    return name;
}

std::vector<FunctionManageItem> functionConfigDlg::GetData()
{
    std::vector<FunctionManageItem> data;

    for (int i=0; i<m_rightModel->rowCount(); i++)
    {
        FunctionManageItem item;
        item.index = m_rightModel->item(i)->data(Qt::UserRole + 1).toInt();
        item.postion = i;
        item.enable = true;

        data.push_back(std::move(item));
    }

    return std::move(data);
}

void functionConfigDlg::OnCheckBoxClicked(bool checked)
{
    // 如不能获取对应的控件，则返回
    auto sendSrc = qobject_cast<QCheckBox*>(sender());
    if (sendSrc == Q_NULLPTR)
    {
        return;
    }

    int index = sendSrc->property(INDEX).toInt();
    QString name = GetStringByIndex(index);

    if (checked)
    {
        auto si = new QStandardItem(name);
        si->setData(index, Qt::UserRole + 1);
        m_rightModel->appendRow(si);     
    }
    else
    {
        for (auto item : m_rightModel->findItems(name))
        {
            m_rightModel->removeRow(item->row());
        }
    }

    ui->select_num_label->setText(QString(tr("已选(%1)")).arg(m_rightModel->rowCount()));

    UpdateBtnState();
}

void functionConfigDlg::OnCloseDelegate(QString text)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto items = m_rightModel->findItems(text);
    for (auto item : items)
    {
        m_rightModel->removeRow(item->row());
    }

    for (int i=0; i<m_leftModel->rowCount(); i++)
    {
        if (m_leftModel->item(i, 1)->text() == text)
        {
            auto widget = qobject_cast<QWidget*>(ui->tableView_left->indexWidget(m_leftModel->index(i, 0)));
            if (widget != Q_NULLPTR)
            {
                widget->findChild<QCheckBox*>()->setChecked(false);
            }
        }
    }

    ui->select_num_label->setText(QString(tr("已选(%1)")).arg(m_rightModel->rowCount()));

    UpdateBtnState();
}

void functionConfigDlg::Init()
{
    SetTitleName(tr("功能配置"));

    m_leftModel = new QStandardItemModel(this);
    m_rightModel = new QStandardItemModel(this);

    if (m_leftModel == Q_NULLPTR || m_rightModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "new QStandardItemModel failed");
        return;
    }

    m_leftModel->setHorizontalHeaderLabels({ tr(" "), tr("") });
    m_rightModel->setHorizontalHeaderLabels({ "" });

    ui->tableView_left->setModel(m_leftModel);
    ui->tableView_left->setMouseTracking(true);
    ui->tableView_right->setModel(m_rightModel);
    ui->tableView_right->setMouseTracking(true);

    ui->tableView_left->setColumnWidth(0, 44);

	// 设置滚动条距顶部位置
	ui->tableView_left->setStyleSheet("QScrollBar::handle:vertical{margin: 51px 3px 2px 2px; border : 0;}"
									  "QScrollBar::sub-line:vertical{height: 48px; border-bottom: 1px solid #a4a4a4;}");
	ui->tableView_right->setStyleSheet("QScrollBar::handle:vertical{margin: 51px 3px 2px 2px; border : 0;}"
									   "QScrollBar::sub-line:vertical{height: 48px; border-bottom: 1px solid #a4a4a4;}");

    // 选中行改变
    connect(ui->tableView_right->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &functionConfigDlg::UpdateBtnState);

    // 右侧表格,删除代理
    auto deleGate = new QSelectNewDelegate(this);
    ui->tableView_right->setItemDelegateForColumn(0, deleGate);
    connect(deleGate, &QSelectNewDelegate::closeDelegate, this, &functionConfigDlg::OnCloseDelegate);

    auto showGate = new QShowTextDelegate(this);
    //左侧表格显示悬浮
    ui->tableView_left->setItemDelegateForColumn(1, showGate);

    // 清空全部
    connect(ui->clear_btn, &QPushButton::clicked, this, [&]()
    {
        m_rightModel->removeRows(0, m_rightModel->rowCount());

        for (int i=0; i<m_leftModel->rowCount(); i++)
        {
            auto widget = qobject_cast<QWidget*>(ui->tableView_left->indexWidget(m_leftModel->index(i,0)));
            if (widget != Q_NULLPTR)
            {
                widget->findChild<QCheckBox*>()->setChecked(false);
            }  
        }
    });

    // 向上
    connect(ui->up_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->tableView_right->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_rightModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        auto currentPos = currentIndex.row() - 1;
        m_rightModel->insertRow(currentPos, items);
        ui->tableView_right->setCurrentIndex(m_rightModel->index(currentPos, 0));
        UpdateBtnState();
    });

    // 向下
    connect(ui->down_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->tableView_right->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_rightModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        m_rightModel->insertRow(currentIndex.row() + 1, items);
        ui->tableView_right->setCurrentIndex(m_rightModel->index(currentIndex.row() + 1, 0));
        UpdateBtnState();
    });

    // 置顶
    connect(ui->top_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->tableView_right->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_rightModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        m_rightModel->insertRow(0, items);
        ui->tableView_right->setCurrentIndex(m_rightModel->index(0, 0));

        UpdateBtnState();
    });

    // 置底
    connect(ui->bottom_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->tableView_right->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_rightModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        m_rightModel->insertRow(m_rightModel->rowCount(), items);
        ui->tableView_right->setCurrentIndex(m_rightModel->index(m_rightModel->rowCount()-1, 0));
        UpdateBtnState();
    });
}

void functionConfigDlg::LoadDataToView(std::vector<FunctionManageItem> data)
{
    // 删除原来的行
    m_leftModel->removeRows(0,m_leftModel->rowCount());
    m_rightModel->removeRows(0, m_rightModel->rowCount());

    // 排序
    std::sort(data.begin(), data.end(), [](const FunctionManageItem& i, const FunctionManageItem& j) 
    {
        return i.postion < j.postion;
    });

    int iRow = 0;
    for (auto item : data)
    {
        // 左侧表格
        QString name = GetStringByIndex(item.index);
        auto si = new QStandardItem(name);
        si->setData(Qt::AlignVCenter, Qt::TextAlignmentRole);
        m_leftModel->setItem(iRow, 1, si);

        // 加入勾选框
        QWidget* wid = new QWidget(ui->tableView_left);
        QCheckBox* box = new QCheckBox(wid);
        box->setMinimumSize(20, 20);
        box->move(6, 2);
        ui->tableView_left->setIndexWidget(m_leftModel->index(iRow, 0), wid);
        connect(box, &QCheckBox::toggled, this, &functionConfigDlg::OnCheckBoxClicked);

        box->setProperty(INDEX, item.index);
        box->setChecked(item.enable);

        iRow++;
    }

    ui->wait_select_num_label->setText(QString(tr("备选(%1)")).arg(m_leftModel->rowCount()));

    UpdateBtnState();
}

QPushButton* functionConfigDlg::GetSaveBtnPtr()
{
    return ui->save_btn;
}

void functionConfigDlg::UpdateBtnState()
{
    ui->up_btn->setEnabled(false);
    ui->down_btn->setEnabled(false);
    ui->top_btn->setEnabled(false);
    ui->bottom_btn->setEnabled(false);

    int rowCount = m_rightModel->rowCount();
    auto currentIndex = ui->tableView_right->currentIndex();
    if (!currentIndex.isValid() || rowCount <= 1)
    {
        return;
    }

    int currentRow = currentIndex.row();

    // 第一行
    if (currentRow == 0)
    {
        ui->down_btn->setEnabled(true);
        ui->bottom_btn->setEnabled(true);
    }
    // 最后一行
    else if (currentRow == (rowCount - 1))
    {
        ui->up_btn->setEnabled(true);
        ui->top_btn->setEnabled(true);
    }
    else
    {
        ui->up_btn->setEnabled(true);
        ui->top_btn->setEnabled(true);
        ui->down_btn->setEnabled(true);
        ui->bottom_btn->setEnabled(true);
    }
}

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
/// @file     QCustomCard.cpp
/// @brief 	  自定义下拉列表
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCustomCard.h"
#include "ui_QCustomCard.h"
#include "src/common/Mlog/mlog.h"
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QSpacerItem>
#include <QTableView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>


#define		LAB_DISTANCE					(15)					// 标签文字的左右边距
#define		LAB_RIGHT_DISTANCE				(-16)					// 标签到右边按钮的距离
#define		LAB_HIGHT						(26)					// 标签高度
#define		LAB_WIDTH						(49)					// 标签宽度
#define		LAB_WIDTH_MIN					(49)					// 标签最小宽度
#define		BTN_HIGHT						(16)					// 关闭按钮高度
#define		BTN_RIGHT_MARGIN				(6)						// 关闭按钮右边距

QCustomCard::QCustomCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QCustomCard)
{
    m_hLayout = nullptr;

    ui->setupUi(this);
    connect(this, SIGNAL(OnClearSignal()), this, SLOT(OnClear()));
	connect(ui->timeEdit, &QTimeEdit::timeChanged, this, [=](QTime time) {emit  InputChanged(); });
    m_weekDays = QStringList{ "1", "2", "3", "4", "5", "6", "7" };
    InitWeekComb();
}

QCustomCard::~QCustomCard()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @bref
///		初始化时间控件数据
///
/// @param[in] strTime 时间
/// @param[in] selectedWeek 周
///
/// @par History:
/// @li 8276/huchunli, 2023年9月5日，新建函数
///
void QCustomCard::UpdateData(const std::string& strTime, const std::vector<int>& selectedWeek)
{
    std::set<int> choicedd;
    for (int d : selectedWeek)
    {
        choicedd.insert(d == 0 ? 7 : d);
    }
    ui->timeEdit->setTime(QTime::fromString(QString::fromStdString(strTime), "HH:mm:ss"));
    
    // 更新周选则
    for (int i=0; i<m_weekDays.length(); ++i)
    {
        std::map<int, CardCtrls>::iterator it = m_labels.find(i + 1);
        if (it != m_labels.end())
        {
            if (choicedd.find(i + 1) != choicedd.end())
            {
                it->second.label->show();
                it->second.checkBox->setChecked(true);
            }
            else
            {
                it->second.label->hide();
                it->second.checkBox->setChecked(false);
            }
        }
    }

    this->update();
}

///
/// @bref
///		清空内容的槽函数
///
/// @par History:
/// @li 8276/huchunli, 2023年8月1日，新建函数
///
void QCustomCard::OnClear()
{
    // 清空标签按钮
    auto lstWidget = this->findChildren<QCustomLabel *>();
    for (auto it : lstWidget)
    {
        it->hide();
    }

    // 清空复选框的选中状态
    auto lstBox = this->findChildren<QCheckBox *>();
    for (auto it : lstBox)
    {
        it->setChecked(false);
    }

	//初始化时间设置
	ui->timeEdit->setTime(QTime::fromString(QString::fromStdString("00:00:00"), "HH:mm:ss"));
}

///
/// @bref
///		获取选择的天（0-6）
///
/// @param[out] choisedDays 输出的选择
///
/// @par History:
/// @li 8276/huchunli, 2023年8月1日，新建函数
///
void QCustomCard::GetSelectedDays(std::vector<int>& choisedDays)
{
    choisedDays.clear();

    std::map<int, CardCtrls>::iterator it = m_labels.begin();
    for (; it != m_labels.end(); ++it)
    {
        if (it->second.label == nullptr)
        {
            continue;
        }
        if (!it->second.label->isHidden())
        {
            it->first == 7 ? choisedDays.push_back(0) : choisedDays.push_back(it->first);
        }
    }
}

///
/// @bref
///		获取时间字符串 hh:MM:ss
///
/// @par History:
/// @li 8276/huchunli, 2023年8月1日，新建函数
///
QString QCustomCard::GetTimeString()
{
    return ui->timeEdit->time().toString("HH:mm:ss");
}

bool QCustomCard::IsEmpty()
{
	// 时间判断
	if (ui->timeEdit->time().toString("HH:mm:ss") != QString("00:00:00"))
	{
		return false;
	}

	// 复选框判断
	auto lstBox = this->findChildren<QCheckBox *>();
	for (auto it : lstBox)
	{
		if (it->isChecked())	return false;
	}

	return true;
		
}

///
///  @brief 初始化
///
///  @param[in]   selectedWeek 选中的天  
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
///
void QCustomCard::InitWeekComb()
{
    m_hLayout = new QHBoxLayout(ui->widget);
    m_hLayout->setMargin(4);
    m_hLayout->setSpacing(3);
    QStandardItemModel *model = new QStandardItemModel(ui->comboBox);
    QTableView *view = new QTableView();
    //view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    view->setObjectName(QStringLiteral("comboView"));
    ui->comboBox->setModel(model);
    ui->comboBox->setView(view);

    // 设置下拉列表表头
    auto headerItem = new QStandardItem(tr("   星期:"));
    headerItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	model->setHorizontalHeaderItem(0, headerItem);

	view->horizontalHeader()->setStyleSheet("border: 0;");
    view->verticalHeader()->setVisible(false);
    view->horizontalHeader()->setVisible(true);
    view->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->setDefaultSectionSize(39);
    // view->setStyleSheet("");
    for (int i = 0; i < m_weekDays.size(); ++i)
    {
        CardCtrls cacheCtrl;

        // 初始化下拉框
        model->insertRow(i, new QStandardItem(""));
        cacheCtrl.checkBox = new QCheckBox(m_weekDays.at(i));
        connect(cacheCtrl.checkBox, &QCheckBox::stateChanged, this, &QCustomCard::OnChecked);
        view->setIndexWidget(model->index(i, 0), cacheCtrl.checkBox);

        // 初始化下拉框上的按钮标签
        cacheCtrl.label = new QCustomLabel(i + 1, ui->widget);
        m_labels.insert(std::pair<int, CardCtrls>(i + 1, cacheCtrl));
        m_hLayout->addWidget(cacheCtrl.label);
        cacheCtrl.label->hide();

        connect(cacheCtrl.label, &QCustomLabel::checked, this, &QCustomCard::OnLabelDelBtnClicked);
    }
    m_hLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
}

///
///  @brief 复选框状态改变
///
///
///  @param[in]   state  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
///
void QCustomCard::OnChecked(int state)
{
    auto checkBox = dynamic_cast<QCheckBox*>(sender());
    if (checkBox == nullptr)
    {
        return;
    }
    auto lstWidget = this->findChildren<QCustomLabel *>();
    for (auto it : lstWidget)
    {
        if (it->text() == checkBox->text())
        {
            it->setVisible(state == Qt::Checked);
			emit  InputChanged();
            break;
        }
    }
}

///
///  @brief 点击标签删除按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
///
void QCustomCard::OnLabelDelBtnClicked()
{
    auto label = dynamic_cast<QCustomLabel*>(sender());
    if (label == nullptr)
    {
        return;
    }
    label->hide();
    auto lstBox = this->findChildren<QCheckBox *>();
    for (auto it : lstBox)
    {
        if (it->text() == label->text())
        {
            it->setChecked(false);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
/// @file     QCustomCard.h
/// @brief    自定义标签
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QCustomLabel::QCustomLabel(int day, QWidget *parent)
    : QLabel(parent)
    , m_btn(new QPushButton(this))
    , m_day(day)
{
    Init(day);

    connect(m_btn, &QPushButton::clicked, this, [&]() {emit checked(); });
}

///
///  @brief 设置按钮大小
///
///
///  @param[in]   size  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
///
void QCustomLabel::setBtnSize(int size)
{

    // 设置底层标签
    setFixedWidth(width() + LAB_RIGHT_DISTANCE + size);

    // 设置按钮
    m_btn->setGeometry(width() - size, 0, size, size);
    m_btn->setGeometry(width() - size, (height() - size) / 2, size, size);
}

///
///  @brief 初始化
///
///
///  @param[in]   day  周几
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月10日，新建函数
///
void QCustomLabel::Init(int day)
{
    setObjectName("week_" + QString::number(day));

    // 设置顶层标签
    setAlignment(Qt::AlignLeft);
    setText(QString::number(day));

    // 设置宽度
    adjustSize();

    setGeometry(0, 0, LAB_WIDTH, LAB_HIGHT);

    // 设置底层标签
    setFixedHeight(LAB_HIGHT);
    setFixedWidth(LAB_WIDTH);

    // 设置按钮
    m_btn->setGeometry(width() - BTN_HIGHT - BTN_RIGHT_MARGIN, (height() - BTN_HIGHT) / 2, BTN_HIGHT, BTN_HIGHT);
    m_btn->raise();
    m_btn->setIcon(QIcon(":/Leonis/resource/image/icon-label-close.png"));
}

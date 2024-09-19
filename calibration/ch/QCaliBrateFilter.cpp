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
/// @file     QCaliBrateFilter.h
/// @brief    校准的筛选
///
/// @author   5774/WuHongTao
/// @date     2022年3月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliBrateFilter.h"
#include "ui_QCaliBrateFilter.h"
#include "CaliBrateCommom.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include <QDate>

QCaliBrateFilter::QCaliBrateFilter(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QCaliBrateFilter();
    ui->setupUi(this);
    Init();
}

QCaliBrateFilter::~QCaliBrateFilter()
{
}

///
/// @brief
///     设置校准条件
///
/// @param[in]  cond  校准条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
void QCaliBrateFilter::SetCaliBrateCond(::ch::tf::CaliCurveQueryCond& cond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_QueryCaliCurveCond = cond;
}

///
/// @brief
///     获取查询条件
///
///
/// @return 查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
ch::tf::CaliCurveQueryCond& QCaliBrateFilter::GetQueryFilterCond()
{
    return m_QueryCaliCurveCond;
}

///
/// @brief
///     初始化选中框
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
void QCaliBrateFilter::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 时间段
    ui->Time_ComboBox->addItem(tr("lastOneDay"), CaliBrateCommom::TimeType::LAST_ONE_DAY);
    ui->Time_ComboBox->addItem(tr("lastThreeDay"), CaliBrateCommom::TimeType::LAST_THREE_DAY);
    ui->Time_ComboBox->addItem(tr("lastOneWeek"), CaliBrateCommom::TimeType::LAST_ONE_WEEK);
    ui->Time_ComboBox->addItem(tr("lastOneMonth"), CaliBrateCommom::TimeType::LAST_ONE_MONTH);
    ui->Time_ComboBox->addItem(tr("lasthalfYear"), CaliBrateCommom::TimeType::LAST_HALF_YEAR);

    // 获取项目列表
    ChAssayIndexUniqueCodeMaps assayMap;
	CommonInformationManager::GetInstance()->GetChAssayMaps(::tf::DeviceType::DEVICE_TYPE_C1000, assayMap);
    for (const auto& assay : assayMap)
    {
        // 添加项目
        ui->Assay_ComboBox->addItem(QString::fromStdString(assay.first), assay.second->assayCode);
    }
    // 查询所有项目
    ui->Assay_ComboBox->addItem(tr("all_project"), -1);

    connect(ui->ok_Button, SIGNAL(clicked()), this, SLOT(OnSaveQueryCond()));
    // 对话框关闭
    connect(ui->Cancel_button, SIGNAL(clicked()), this, SLOT(close()));
}

///
/// @brief
///     保存查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建函数
///
void QCaliBrateFilter::OnSaveQueryCond()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    QDate currentDate = QDate::currentDate();
    QString endStart = currentDate.toString("yyyy-MM-dd");
    endStart = endStart + " 00:00:00";
    QString timeStart;

    // 设置时间
    switch (static_cast<CaliBrateCommom::TimeType>(ui->Time_ComboBox->currentData().toInt()))
    {
    // 最近一天
    case CaliBrateCommom::TimeType::LAST_ONE_DAY:
    {
        timeStart = (currentDate.addDays(-1)).toString("yyyy-MM-dd");
    }
    break;
    // 最近三天
    case CaliBrateCommom::TimeType::LAST_THREE_DAY:
    {
        timeStart = (currentDate.addDays(-3)).toString("yyyy-MM-dd");
    }
    break;
    // 最近一周
    case CaliBrateCommom::TimeType::LAST_ONE_WEEK:
    {
        timeStart = (currentDate.addDays(-7)).toString("yyyy-MM-dd");
    }
    break;
    // 最近一月
    case CaliBrateCommom::TimeType::LAST_ONE_MONTH:
    {
        timeStart = (currentDate.addMonths(-1)).toString("yyyy-MM-dd");
    }
    break;
    // 最近半年
    case CaliBrateCommom::TimeType::LAST_HALF_YEAR:
    {
        timeStart = (currentDate.addMonths(-6)).toString("yyyy-MM-dd");
    }
    break;
    }

    timeStart = timeStart + " 23:59:59";

    tf::TimeRangeCond timeCond;
    timeCond.__set_startTime(timeStart.toStdString());
    timeCond.__set_endTime(endStart.toStdString());
    // 设置校准时间
    m_QueryCaliCurveCond.__set_caliTime(timeCond);

    int assayCode = ui->Assay_ComboBox->currentData().toInt();
    if (assayCode != -1)
    {
        m_QueryCaliCurveCond.__set_assayCode(assayCode);
    }

    emit HasFinshedSelect();
    close();
}

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

#include "QDefaultQcApp.h"
#include "ui_QDefaultQcApp.h"

#include "QQcSampleAppModel.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/QComDelegate.h"
#include "thrift/DcsControlProxy.h"

QDefaultQcApp::QDefaultQcApp(QWidget *parent)
    : BaseDlg(parent)
    , m_module(nullptr)
{
    ui = new Ui::QDefaultQcApp();
    ui->setupUi(this);

    if (m_module == nullptr)
    {
        m_module = new QQcSampleAppModel(QQcSampleAppModel::MODE::DEFAULT);
        ui->tableView->setModel(m_module);
    }

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setItemDelegateForColumn(8, new CheckBoxDelegate(this));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnShowCurrentQc(int)));
    connect(ui->pushButton_2, &QPushButton::clicked, this, [&]() {this->close(); emit closeWindows(); });
    // 更新质控申请信息
    REGISTER_HANDLER(MSG_ID_QCAPP_INFO_UPDATE, this, [&]() {OnShowCurrentQc(m_currentQcIndex); });
}

QDefaultQcApp::~QDefaultQcApp()
{
}

///
/// @brief 启动申请页面的列表显示
///
/// @param[in]  devices  设备列表
///
/// @return 成功，返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月05日，新建函数
///
bool QDefaultQcApp::StartQcAppPage(DeviceModules& devices)
{
    ::tf::QcDocQueryResp _return;
    ::tf::QcDocQueryCond cdqc;
    if (!DcsControlProxy::GetInstance()->QueryQcDoc(_return, cdqc)
        || _return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || m_module == nullptr)
    {
        return false;
    }

    m_docs.clear();
    ui->comboBox->clear();
    for (const auto& qcDoc : _return.lstQcDocs)
    {
        m_docs.push_back(qcDoc);
        ui->comboBox->addItem(QString::fromStdString(qcDoc.name));
    }

    m_devInfos = devices;
    if (!m_docs.empty())
    {
        m_module->SetData(QQcSampleAppModel::MODE::DEFAULT, m_devInfos, m_docs[0]);
    }

    return true;
}

void QDefaultQcApp::OnShowCurrentQc(int index)
{
    if (index >= m_docs.size() || index < 0 
        || m_module == nullptr || m_devInfos.size() == 0)
    {
        return;
    }

    m_currentQcIndex = index;
    // 刷新页面
    m_module->SetData(QQcSampleAppModel::MODE::DEFAULT, m_devInfos, m_docs[index]);
}

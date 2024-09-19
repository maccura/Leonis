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
/// @file     AlarmMusicDlg.cpp
/// @brief    应用--系统--提示设置--音乐管理
///
/// @author   7951/LuoXin
/// @date     2023年5月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年5月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AlarmMusicDlg.h"
#include "ui_AlarmMusicDlg.h" 
#include "shared/tipdlg.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"
#include <QFileDialog>
#include <QStandardItemModel>

AlarmMusicDlg::AlarmMusicDlg(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::AlarmMusicDlg)
    , m_pItemModel(new QStandardItemModel)
{
    ui->setupUi(this);
    Init();
}

AlarmMusicDlg::~AlarmMusicDlg()
{
}

void AlarmMusicDlg::UpdateMusicName(const QStringList& musiclist)
{
    // 移除所有行
    m_pItemModel->removeRows(0, m_pItemModel->rowCount());

    // 重新添加
    int row = 0;
    for (auto str : musiclist)
    {
        m_pItemModel->setItem(row, 0, new QStandardItem(str));
        row++;
    }
}

void AlarmMusicDlg::Init()
{
    SetTitleName(tr("音乐管理"));

    // 设置表头
    m_pItemModel->setHorizontalHeaderLabels({ tr("报警音乐") });
    ui->tableView->setModel(m_pItemModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->load_btn, &QPushButton::clicked, this, &AlarmMusicDlg::OnLoadBtnClicked);
    connect(ui->del_btn, &QPushButton::clicked, this, &AlarmMusicDlg::OnDelBtnClicked);
}

void AlarmMusicDlg::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    QDir dir(QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR);
    if (!dir.exists())
    {
        TipDlg(tr("删除失败，报警音乐配置文件目录不存在！")).exec();
        return;
    }

    // 删除选中行
    QStringList nameList;
    for (auto index : ui->tableView->selectionModel()->selectedRows())
    {
        // 删除音频文件
        QString name = m_pItemModel->data(m_pItemModel->index(index.row(), 0)).toString();
        nameList.push_back(name);
    }

    AlarmMusic am;
    if (!DictionaryQueryManager::GetAlarmMusicConfig(am))
    {
        TipDlg(tr("删除报警音乐文件失败！")).exec();
        return;
    }

    if (nameList.contains(QString::fromStdString(am.strAttentionSoundName))
        || nameList.contains(QString::fromStdString(am.strSampleStopSoundName))
        || nameList.contains(QString::fromStdString(am.strStopSoundName)))
    {
        TipDlg(tr("当前选中的报警音乐文件中存在正在使用的文件，不可删除！")).exec();
        return;
    }

    if (TipDlg(tr("即将删除选中的报警音乐文件，请确认"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 删除文件
    for (const QString& name : nameList)
    {
        QFile file(QString::fromStdString(GetCurrentDir() + ALARM_MUSIC_CONFIG_DIR) + name);
        file.remove();
    }

    // 清空表格
    m_pItemModel->removeRows(0, m_pItemModel->rowCount());

    // 获取报警音文件列表
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    if (infoList.empty())
    {
        return;
    }
    
    // 遍历
    int row = 0;
    for (auto info : infoList)
    {
        m_pItemModel->setItem(row, 0, new QStandardItem(info.fileName()));
        row++;
    }
}

void AlarmMusicDlg::OnLoadBtnClicked()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("音乐导入"), ".", tr("音频文件(*.mp3 *.wav)"));

    QStringList AlarmFileName;

    //遍历获取每一行的文件名
    for (int row = 0; row < m_pItemModel->rowCount(); row++)
    {
        for (int column = 0; column < m_pItemModel->columnCount(); column++)
        {
            QModelIndex index = m_pItemModel->index(row, column);
            QVariant data = m_pItemModel->data(index);
            AlarmFileName << data.toString();
        }
    }

    for (auto name : fileNames)
    {
        QFile file(name);
        QFileInfo fileInfo(file);
        //若界面包含则给出提示
        if (AlarmFileName.contains(fileInfo.fileName()))
        {
           //若存在同名文件给出提示bug 0019256
            TipDlg(tr("读取失败"), tr("所选音频文件%1和已有文件重复，请更改后重新选择文件。").arg(name)).exec();
            continue;
        }
        // 若文件没有访问权限给出提示 bug 0019261
        if (!(fileInfo.permissions() & QFile::ReadUser))
        {
            TipDlg(tr("读取失败"), tr("所选音频文件%1用户权限不足。").arg(name)).exec();
            continue;
        }
        //若音频文件大于100MB则给出提示不能保存
        if ((fileInfo.size() / (1024 * 1024)) > 100)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("音频文件不能超过100Mb，请核对后导入。")));
            pTipDlg->exec();
            return;
        }
        file.copy(QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR + fileInfo.fileName());
        m_pItemModel->setItem(m_pItemModel->rowCount(), 0, new QStandardItem(fileInfo.fileName()));
    }
}

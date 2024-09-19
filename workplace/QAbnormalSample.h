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

#pragma once

#include "QWorkShellPage.h"
#include <QAbstractTableModel>

class QAbnormalSampleModel;
class QAbnormalAssayModel;

namespace Ui {
    class QAbnormalSample;
};


class QAbnormalSample : public QWorkShellPage
{
    Q_OBJECT

public:
    QAbnormalSample(QWidget *parent = Q_NULLPTR);
    ~QAbnormalSample();
    void RefreshCornerWidgets(int index)override;

public slots:
    void ShowBySample();
    void ShowByResult();

private:
    Ui::QAbnormalSample*            ui;
    QAbnormalSampleModel*           m_sampleModel;
    QAbnormalAssayModel*            m_assayModel;
    QWidget*						m_cornerItem;
};

class QAbnormalSampleModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    QAbnormalSampleModel();

protected:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    QStringList                     m_headerNames;              /// 表头名称集合
};

class QAbnormalAssayModel : public QAbnormalSampleModel
{
    Q_OBJECT
public:
    QAbnormalAssayModel();

protected:
    virtual QVariant data(const QModelIndex &index, int role) const override;
};

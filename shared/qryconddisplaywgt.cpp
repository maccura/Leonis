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
/// @file     qryconddisplaywgt.cpp
/// @brief    查询条件显示控件
///
/// @author   4170/TangChuXian
/// @date     2023年8月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qryconddisplaywgt.h"
#include "ui_qryconddisplaywgt.h"
#include "shared/uidef.h"
#include "src/common/Mlog/mlog.h"
#include <QTimer>

QryCondDisplayWgt::QryCondDisplayWgt(QWidget *parent)
    : QWidget(parent),
      m_enElideMode(Qt::ElideRight)
{
    ui = new Ui::QryCondDisplayWgt();
    ui->setupUi(this);

    // 连接信号
    connect(ui->icon_small_close, SIGNAL(clicked()), this, SIGNAL(SigResetQry()));
    connect(ui->QryCondLab, SIGNAL(textChanged(const QString&)), this, SLOT(OnTextChanged(const QString&)));

    // 安装事件过滤器
    ui->QryCondLab->installEventFilter(this);

    // 默认不显示
    this->setVisible(false);
}

QryCondDisplayWgt::~QryCondDisplayWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     查询文本
///
/// @param[in]  strQryText  查询文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
void QryCondDisplayWgt::SetText(const QString& strQryText)
{
    ui->QryCondLab->setText(strQryText);
    this->setVisible(!strQryText.isEmpty());
}

///
/// @brief
///     重置查询
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
void QryCondDisplayWgt::ResetQry()
{
    ui->QryCondLab->clear();
    m_strText.clear();
    m_strToolTip.clear();
    m_enElideMode = Qt::ElideRight;
    this->setVisible(false);
    emit SigResetQry();
}

///
/// @brief
///     设置文本提示
///
/// @param[in]  strToolTip  文本提示
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
void QryCondDisplayWgt::SetToolTip(const QString& strToolTip)
{
    m_strToolTip = strToolTip;
    setToolTip(strToolTip);
}

///
/// @brief
///     设置文本省略模式
///
/// @param[in]  elideMode  省略模式
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
void QryCondDisplayWgt::SetElideMode(Qt::TextElideMode elideMode)
{
    m_enElideMode = elideMode;
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj    目标对象
/// @param[in]  event  事件
///
/// @return true表示已处理
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
bool QryCondDisplayWgt::eventFilter(QObject *obj, QEvent *event)
{
    // 判断对象
    // 质控结果
    if ((obj == ui->QryCondLab) && (event->type() == QEvent::Resize))
    {
        QTimer::singleShot(0, this, SLOT(UpdateElidedText()));
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     文本内容改变
///
/// @param[in]  strText  改变后的文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
void QryCondDisplayWgt::OnTextChanged(const QString& strText)
{
    // 记录原始文本
    m_strText = strText;

    // 设置提示
    if (m_strToolTip.isEmpty())
    {
        setToolTip(m_strText);
    }
}

///
/// @brief
///     更新可省略文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月16日，新建函数
///
void QryCondDisplayWgt::UpdateElidedText()
{
    if (ui->horizontalSpacer->geometry().width() <= 2)
    {
        QString newText(m_strText);
        QFont defFont = ui->QryCondLab->font();
        defFont.setFamily(FONT_FAMILY);
        defFont.setPixelSize(FONT_SIZE_GENERAL);
        QFontMetrics fontMtcs(defFont);
        if (fontMtcs.width(m_strText) > ui->QryCondLab->width())
        {
            newText = fontMtcs.elidedText(m_strText, m_enElideMode, ui->QryCondLab->width(), Qt::TextSingleLine);
        }
        ui->QryCondLab->blockSignals(true);
        ui->QryCondLab->setText(newText);
        ui->QryCondLab->blockSignals(false);
    }
    else
    {
        // 如果是多行文本
        QString strFirstLine = m_strText.split("\n").front();
        if (strFirstLine.size() < m_strText.size())
        {
            strFirstLine = strFirstLine + "...";
        }

        ui->QryCondLab->blockSignals(true);
        ui->QryCondLab->setText(strFirstLine);
        ui->QryCondLab->blockSignals(false);
    }
}

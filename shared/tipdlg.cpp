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
/// @file     tipdlg.cpp
/// @brief    提示对话框
///
/// @author   4170/TangChuXian
/// @date     2020年6月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "tipdlg.h"
#include <QFontMetrics>
#include "ui_tipdlg.h"


TipDlg::TipDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_fristBtnText(tr("确定")),
      m_secondBtnText(tr("取消"))
{
    ui = new Ui::TipDlg();
    ui->setupUi(this);

    SetTitleName(tr("提醒"));
}

///
/// @brief
///     构造时初始化提示文本和按钮模式
///
/// @param[in]  strText     提示文本
/// @param[in]  enBtnModel  按钮模式
/// @param[in]  parent      父窗口指针
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建函数
///
TipDlg::TipDlg(const QString& strText, TipDlgType::ButtonModel enBtnModel, QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent),
      m_bInit(false),
      m_strTipText(strText),
      m_enBtnModel(enBtnModel),
      m_fristBtnText(tr("确定")),
      m_secondBtnText(tr("取消"))
{
    ui = new Ui::TipDlg();
    ui->setupUi(this);

    SetTitleName(tr("提示"));
}

///
/// @brief
///     构造时初始化提示文本和按钮模式
///
/// @param[in]  strTitle    标题文本
/// @param[in]  strText     提示文本
/// @param[in]  enBtnModel  按钮模式
/// @param[in]  parent      父窗口指针
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月11日，新建函数
///
TipDlg::TipDlg(const QString& strTitle, const QString& strText, TipDlgType::ButtonModel enBtnModel /*= TipDlgType::SINGLE_BUTTON*/, QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent),
      m_bInit(false),
      m_strTipText(strText),
      m_enBtnModel(enBtnModel),
      m_fristBtnText(tr("确定")),
      m_secondBtnText(tr("取消"))
{
    ui = new Ui::TipDlg();
    ui->setupUi(this);

    SetTitleName(strTitle);
}

TipDlg::~TipDlg()
{
}

///
/// @brief
///     设置提示对话框文本
///
/// @param[in]  strText  提示对话框
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建函数
///
void TipDlg::SetText(const QString& strText)
{
    m_strTipText = strText;
    ui->TipLabel->setText(m_strTipText);

    // 对话框长宽自适应文本
    ResizeByText();
}

///
/// @brief
///     获取当前显示文本
///
/// @return 当前显示文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月29日，新建函数
///
const QString& TipDlg::Text()
{
    return m_strTipText;
}

///
/// @brief
///     设置按钮模式
///
/// @param[in]  enBtnModel  按钮模式
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建函数
///
void TipDlg::SetButtonModel(TipDlgType::ButtonModel enBtnModel)
{
    m_enBtnModel = enBtnModel;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建函数
///
void TipDlg::InitAfterShow()
{
    // 对话框长宽自适应文本
    ResizeByText();

    // 初始化按钮模式
    if (m_enBtnModel == TipDlgType::SINGLE_BUTTON)
    {
        ui->OKBtn->setVisible(false);
        ui->CancelBtn->setVisible(true);
        ui->CancelBtn->setText(m_fristBtnText);
        connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    }
    else if (m_enBtnModel == TipDlgType::TWO_BUTTON)
    {
        ui->OKBtn->setVisible(true);
		ui->OKBtn->setText(m_fristBtnText);
        ui->CancelBtn->setVisible(true);
		ui->CancelBtn->setText(m_secondBtnText);
        connect(ui->OKBtn, SIGNAL(clicked()), this, SLOT(accept()));
        connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    }
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建函数
///
void TipDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     根据文本调整长款自适应
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月20日，新建函数
///
void TipDlg::ResizeByText()
{
    QFontMetrics fm(ui->TipLabel->font());

    // 设置文字
    ui->TipLabel->setText(m_strTipText);

    // 计算文本行数，重新设置标签高度
    // 仅统计/n不对，
    // 如
    // xxxxxxxx
    // xx/n
    // xxxxxx
    // xxx
    // 没有统计自动换行的行数，这里暂时添加多余行数进行处理
    // 考虑包含多个换行符的情况，计算行数——mod-tcx——bug26757
    int numLines = 0;
    QStringList strSegList = m_strTipText.split("\n");
    for (const QString& strSegText : strSegList)
    {
        //QFontMetrics fm(this->fontMetrics());
        numLines += ((fm.width(strSegText) / ui->TipLabel->width()) + 1);
    }
    numLines = numLines > 18 ? 18 : numLines; // 超过18行，会把框中的操作按钮挤出屏幕外，造成无法关闭对话框。

    int newHeight = qBound(ui->TipLabel->minimumHeight(), fm.lineSpacing() * numLines, ui->TipLabel->maximumHeight());
    ui->TipLabel->resize(ui->TipLabel->width(), newHeight);

    // 重新设置位置
    setFixedHeight(newHeight + 204);
    move(x(), y() - ((newHeight - 100) / 2));
    ui->TipLabel->move(50, 90);
    ui->OKBtn->move(392, height() - 97);
    ui->CancelBtn->move(527, height() - 97);
}

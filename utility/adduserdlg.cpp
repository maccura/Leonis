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
/// @file     adduserdlg.cpp
/// @brief    质控登记对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "adduserdlg.h"
#include "ui_adduserdlg.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include <memory>

AddUserDlg::AddUserDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_ciUserNameMaxCharCnt(20),
      m_ciPasswordMaxCharCnt(10)
{
    // 初始化ui对象
    ui = new Ui::AddUserDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

AddUserDlg::~AddUserDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     清空界面
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月13日，新建函数
///
void AddUserDlg::Clear()
{
    // 清空编辑框
    ui->AddPageLoginNameEdit->clear();
    ui->AddPageUserNameEdit->clear();
    ui->AddPagePasswordEdit->clear();
    ui->AddPagePasswordSureEdit->clear();
    ui->AddPagePermissionCombo->setCurrentIndex(0);
    ui->ErrTipLab->clear();
    ui->AddPageLoginNameEdit->setFocus();
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void AddUserDlg::InitBeforeShow()
{
    // 设置标题
    SetTitleName(tr("新增用户"));
    ui->ErrTipLab->clear();
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void AddUserDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void AddUserDlg::InitStrResource()
{

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void AddUserDlg::InitConnect()
{
    // 连接信号槽
    // 取消按钮被点击
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 用户编辑时清空错误提示
    connect(ui->AddPageLoginNameEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));
    connect(ui->AddPageUserNameEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));
    connect(ui->AddPagePasswordEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));
    connect(ui->AddPagePasswordSureEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));

    // 更新确定按钮使能状态
    connect(ui->AddPageLoginNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
    connect(ui->AddPageUserNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
    connect(ui->AddPagePasswordEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
    connect(ui->AddPagePasswordSureEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));

    // 编辑用户名时计算长度限制输入
    connect(ui->AddPageUserNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnUserNameTextEditted(const QString&)));
    connect(ui->AddPagePasswordEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnPasswordTextEditted(const QString&)));
    connect(ui->AddPagePasswordSureEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnPasswordTextEditted(const QString&)));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void AddUserDlg::InitChildCtrl()
{
}

///
/// @brief
///     获取文本的字符数
///
/// @param[in]  strText  文本
///
/// @return 文本字符数
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月31日，新建函数
///
int AddUserDlg::GetCharCnt(const QString& strText)
{
    int iRetLen = 0;
    for (const auto& qChar : strText)
    {
        if (qChar.unicode() >= 0x4e00 && qChar.unicode() <= 0x9fa5)
        {
            iRetLen += 2;
        }
        else
        {
            ++iRetLen;
        }
    }

    return iRetLen;
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void AddUserDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    // 显示时清空
    Clear();

    // 更新确定按钮使能状态
    UpdateOkBtnEnabled();
}

///
/// @brief
///     更新确定按钮是否使能
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月8日，新建函数
///
void AddUserDlg::UpdateOkBtnEnabled()
{
    // 如果有编辑框为空，则确定按钮置灰
    if (ui->AddPageLoginNameEdit->text().isEmpty() ||
        ui->AddPageUserNameEdit->text().isEmpty() ||
        ui->AddPagePasswordEdit->text().isEmpty() ||
        ui->AddPagePasswordSureEdit->text().isEmpty())
    {
        ui->OkBtn->setEnabled(false);
        return;
    }

    // 否则按钮使能
    ui->OkBtn->setEnabled(true);
}

///
/// @brief
///     用户名文本改变
///
/// @param[in]  strText  新的文本
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月31日，新建函数
///
void AddUserDlg::OnUserNameTextEditted(const QString& strText)
{
    if (GetCharCnt(strText) > m_ciUserNameMaxCharCnt)
    {
        ui->AddPageUserNameEdit->backspace();
    }
}

///
/// @brief
///     密码文本被编辑
///
/// @param[in]  strText  新的文本
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建函数
///
void AddUserDlg::OnPasswordTextEditted(const QString& strText)
{
    // 获取信号发送者
    QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
    if (pEdit == Q_NULLPTR)
    {
        return;
    }

    // 如果文本长度合理则忽略
    if (strText.length() <= m_ciPasswordMaxCharCnt)
    {
        return;
    }

    // 文本过长,弹框提示
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("密码应为【1-10】位除中文外的任意字符。")));
    pTipDlg->exec();

    // 回退
    pEdit->backspace();
}

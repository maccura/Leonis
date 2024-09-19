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
/// @file     usermngdlg.cpp
/// @brief    默认质控品对话框
///
/// @author   4170/TangChuXian
/// @date     2023年3月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "usermngdlg.h"
#include "ui_usermngdlg.h"
#include "adduserdlg.h"
#include "ui_adduserdlg.h"
#include "modifypassworddlg.h"
#include "ui_modifypassworddlg.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/basedlg.h"
#include "shared/uidef.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include <QTimer>

#define  DEFAULT_ROW_CNT_OF_USER_TABLE                   (44)           // 用户表表默认行数
#define  DEFAULT_COL_CNT_OF_USER_TABLE                   (6)            // 用户表表默认列数

#define  USER_TBL_COL_IDX_ROW_NUM                        (0)            // 行号列索引
#define  USER_TBL_COL_IDX_AUTO_LOGIN                     (4)            // 自动登录列索引
#define  USER_TBL_COL_IDX_DB_NO                          (5)            // 数据库主键列索引

#define  USER_NAME_ADMIN                                 ("admin")      // 内置管理员

Q_DECLARE_METATYPE(tf::UserInfo)

UserMngDlg::UserMngDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_strAutoLoginUser(""),
      m_pAddUserDlg(new AddUserDlg(this)),
      m_pModifyPasswordDlg(new ModifyPasswordDlg(this))
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::UserMngDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

UserMngDlg::~UserMngDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_pAddUserDlg->deleteLater();
    m_pModifyPasswordDlg->deleteLater();
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UserMngDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化成员变量（当前登录用户）
    m_spLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if ((m_spLoginUserInfo == Q_NULLPTR) || (m_spLoginUserInfo->type == tf::UserType::USER_TYPE_GENERAL))
    {
        ui->AddBtn->setEnabled(false);
        ui->AutoLoginBtn->setEnabled(false);
    }

    // 密码输入框设置正则表达式
    m_pAddUserDlg->ui->AddPageLoginNameEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_LOGIN_NAME)));
    m_pAddUserDlg->ui->AddPageUserNameEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_NAME)));
    m_pAddUserDlg->ui->AddPagePasswordEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_PASSWORD_ANY_LENGTH)));
    m_pAddUserDlg->ui->AddPagePasswordSureEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_PASSWORD_ANY_LENGTH)));
    m_pModifyPasswordDlg->ui->ModPageOldkeyEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_PASSWORD)));
    m_pModifyPasswordDlg->ui->ModPageNewkeyEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_PASSWORD)));
    m_pModifyPasswordDlg->ui->ModPageNewkeySureEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_USER_PASSWORD)));

    // 密码输入框禁用输入法
    m_pAddUserDlg->ui->AddPagePasswordEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pAddUserDlg->ui->AddPagePasswordSureEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pModifyPasswordDlg->ui->ModPageOldkeyEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pModifyPasswordDlg->ui->ModPageNewkeyEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pModifyPasswordDlg->ui->ModPageNewkeySureEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    // 初始化项目表
    ui->UserTbl->setRowCount(DEFAULT_ROW_CNT_OF_USER_TABLE);
    ui->UserTbl->setColumnCount(DEFAULT_COL_CNT_OF_USER_TABLE);

    // 隐藏数据库主键列
    ui->UserTbl->hideColumn(USER_TBL_COL_IDX_DB_NO);

    // 初始化下拉框
    if (m_spLoginUserInfo != Q_NULLPTR)
    {
        if (m_spLoginUserInfo->type > tf::UserType::USER_TYPE_GENERAL)
        {
            AddTfEnumItemToComBoBox(m_pAddUserDlg->ui->AddPagePermissionCombo, tf::UserType::USER_TYPE_GENERAL);
        }

        if ((m_spLoginUserInfo->type > tf::UserType::USER_TYPE_ADMIN) ||
            (m_spLoginUserInfo->username == USER_NAME_ADMIN))
        {
            AddTfEnumItemToComBoBox(m_pAddUserDlg->ui->AddPagePermissionCombo, tf::UserType::USER_TYPE_ADMIN);
        }

        // 普通用户不允许设置自动登录，重置密码按钮不显示，不允许新增用户
        if (m_spLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)
        {
            ui->AutoLoginBtn->setEnabled(false);
            ui->AddBtn->setEnabled(false);
            ui->ResetPasswordBtn->setVisible(false);
        }
    }

    // 获取自动登录用户
    std::string strUserName;
    if (!DictionaryQueryManager::GetAutoLoginUser(strUserName))
    {
        ULOG(LOG_WARN, "Failed to get autologin user.");
        return;
    }
    m_strAutoLoginUser = QString::fromStdString(strUserName);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UserMngDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void UserMngDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UserMngDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 添加按钮被点击
    connect(ui->AddBtn, SIGNAL(clicked()), this, SLOT(OnAddBtnClicked()));

    // 修改按钮被点击
    connect(ui->ModifyBtn, SIGNAL(clicked()), this, SLOT(OnModifyBtnClicked()));

    // 删除按钮被点击
    connect(ui->DelBtn, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));

    // 重置密码按钮被点击
    connect(ui->ResetPasswordBtn, SIGNAL(clicked()), this, SLOT(OnResetPasswordBtnClicked()));

    // 新增用户对话框确定按钮被点击
    connect(m_pAddUserDlg->ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnAddUserDlgOkBtnClicked()));

    // 修改密码对话框确定按钮被点击
    connect(m_pModifyPasswordDlg->ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnModifyPasswordDlgOkBtnClicked()));

    // 自动登录按钮被点击
    connect(ui->AutoLoginBtn, SIGNAL(clicked()), this, SLOT(OnAutoLoginBtnClicked()));

    // 用户表选项改变
    QItemSelectionModel* selItemModel = ui->UserTbl->selectionModel();
    if (selItemModel != Q_NULLPTR)
    {
        // 项目信息更新
        connect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnSelUserChanged(const QModelIndex&)));
    }
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UserMngDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("用户管理"));

    QStringList strHeaderList;
    strHeaderList << tr("序号") << tr("登录名") << tr("用户名") << tr("用户权限") << tr("自动登录");
    ui->UserTbl->setHorizontalHeaderLabels(strHeaderList);
    ui->UserTbl->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->UserTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->UserTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->UserTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->UserTbl->setColumnWidth(0, 177);
	ui->UserTbl->setColumnWidth(1, 272);
	ui->UserTbl->setColumnWidth(2, 272);
	ui->UserTbl->setColumnWidth(3, 272);
	ui->UserTbl->horizontalHeader()->setStretchLastSection(true);
	ui->UserTbl->horizontalHeader()->setMinimumSectionSize(100);

    // 列宽设置
    //ui->UserTbl->setColumnWidth(0, 200);

    // 设置表格列宽比
//     QVector<double> vScale;
//     vScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->DefaultQcTbl, vScale);


    // 设置绝对列宽
//     QVector<double> vColWidth;
//     vColWidth << 162.0 << 160.0 << 161.0 << 160.0 << 161.0 << 131.0;
//     SetTblColAbsWidthScale(ui->DefaultQcTbl, vColWidth);

    // 更新用户信息表
    UpdateUserInfoTbl();
}

///
/// @brief
///     获取下一个空白行
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
int UserMngDlg::GetNextBlamkRow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int iOldRowCnt = ui->UserTbl->rowCount();
    for (int iRow = 0; iRow < iOldRowCnt; iRow++)
    {
        // 获取Item
        QTableWidgetItem* pItem = ui->UserTbl->item(iRow, USER_TBL_COL_IDX_DB_NO);
        if (pItem == Q_NULLPTR)
        {
            return iRow;
        }
    }

    // 如果没有找到空行，则增加一行
    ui->UserTbl->setRowCount(iOldRowCnt + 1);
    return iOldRowCnt;
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UserMngDlg::showEvent(QShowEvent *event)
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
///     是否允许去操作用户
///
/// @param[in]  stuOpUser   被操作的用户
/// @param[in]  enOpMd      操作方式
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月8日，新建函数
///
bool UserMngDlg::IsAllowedToOperateUser(const tf::UserInfo& stuOpUser, OpUserMethod enOpMd)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        return false;
    }

    // 权限判断
    if (enOpMd == OP_USER_METHOD_DELETE)
    {
        // 不允许删除自身
        if (m_spLoginUserInfo->username == stuOpUser.username)
        {
            return false;
        }

        // 只能删除比自己权限低的用户
        if (m_spLoginUserInfo->type < stuOpUser.type ||
            ((m_spLoginUserInfo->type == stuOpUser.type) && (m_spLoginUserInfo->username != USER_NAME_ADMIN)))
        {
            return false;
        }

        // 不允许删除内置管理员
        if (stuOpUser.username == USER_NAME_ADMIN)
        {
            return false;
        }
    }
    else if (enOpMd == OP_USER_METHOD_AUTO_LOGIN)
    {
        // 管理员以上级别才能设置自动登录
        if (m_spLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)
        {
            return false;
        }

        // 只允许设置自己和比自己权限低的用户
        if (m_spLoginUserInfo->type < stuOpUser.type ||
            ((m_spLoginUserInfo->type == stuOpUser.type) &&
             (m_spLoginUserInfo->username != stuOpUser.username) &&
             (m_spLoginUserInfo->username != USER_NAME_ADMIN)))
        {
            return false;
        }
    }
    else if (enOpMd == OP_USER_METHOD_RESET_PASSWORD)
    {
        // 只能重置比自己权限低的用户
        if (m_spLoginUserInfo->type < stuOpUser.type)
        {
            return false;
        }

        // 内置管理员可以重置管理员的密码
        if ((m_spLoginUserInfo->type == stuOpUser.type) &&
            (m_spLoginUserInfo->username != USER_NAME_ADMIN || (stuOpUser.username == USER_NAME_ADMIN)))
        {
            return false;
        }
    }
    else
    {
        // 未知的操作方式
        return false;
    }

    return true;
}

///
/// @brief
///     更新表格行号显示
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月14日，新建函数
///
void UserMngDlg::UpdateTblRowNum()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int iRow = 0;
    for (int iRow = 0; iRow < ui->UserTbl->rowCount(); iRow++)
    {
        // 逐一获取单元项
        QTableWidgetItem* pItem = ui->UserTbl->item(iRow, USER_TBL_COL_IDX_ROW_NUM);
        if (pItem == Q_NULLPTR)
        {
            break;
        }

        // 设置行号
        pItem->setText(QString::number(iRow + 1));
    }
}

///
/// @brief
///     更新用户信息表
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
void UserMngDlg::UpdateUserInfoTbl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空表格和缓存
    ui->UserTbl->clearContents();
    m_mapUserInfo.clear();

    // 查询用户信息
    tf::UserInfoQueryResp qryResp;
    tf::UserInfoQueryCond qryCond;

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryAllUserInfo(qryResp, qryCond) || (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "%s(), QueryAllUserInfo failed", __FUNCTION__);
        UpdateBtnEnableStatus();
        return;
    }

    // 将用户信息添加到表格中
    int iRow = 0;
    for (const auto& stuUserInfo : qryResp.lstUserInfos)
    {
        // 记录缓存
        m_mapUserInfo.insert(QString::fromStdString(stuUserInfo.username), stuUserInfo);

        // 工程师和最高权限不显示在用户列表里
        if ((stuUserInfo.type == tf::UserType::USER_TYPE_ENGINEER) ||
            (stuUserInfo.type == tf::UserType::USER_TYPE_TOP))
        {
            continue;
        }

        // 如果行号大于等于行数，则行数自动增加（始终预留一行空白行）
        if (iRow >= (ui->UserTbl->rowCount() - 1))
        {
            ui->UserTbl->setRowCount(iRow + 2);
        }

        // 自动登录列显示
        QString strAutoLogin("");
        if (m_strAutoLoginUser == QString::fromStdString(stuUserInfo.username))
        {
            strAutoLogin = tr("自动登录");
        }

        // 添加数据
        ui->UserTbl->setItem(iRow, 0, new QTableWidgetItem(QString::number(iRow + 1)));                         // 序号
        ui->UserTbl->setItem(iRow, 1, new QTableWidgetItem(QString::fromStdString(stuUserInfo.username)));      // 登录名
        ui->UserTbl->setItem(iRow, 2, new QTableWidgetItem(QString::fromStdString(stuUserInfo.nickname)));      // 用户名
        ui->UserTbl->setItem(iRow, 3, new QTableWidgetItem(ConvertTfEnumToQString(stuUserInfo.type)));          // 权限
        ui->UserTbl->setItem(iRow, 4, new QTableWidgetItem(strAutoLogin));                                      // 自动登录
        ui->UserTbl->setItem(iRow, 5, new QTableWidgetItem(QString::number(stuUserInfo.id)));                   // 数据库主键

        // 缓存用户信息数据
        ui->UserTbl->item(iRow, USER_TBL_COL_IDX_DB_NO)->setData(Qt::UserRole, QVariant::fromValue<tf::UserInfo>(stuUserInfo));

        // 行号自增
        ++iRow;
    }

    // 选中第一行
    ui->UserTbl->selectRow(0);

    // 更新按钮使能
    UpdateBtnEnableStatus();

    // 表格列宽自适应
    //ResizeTblColToContent(ui->UserTbl);

    // 表格文本居中
    SetTblTextAlign(ui->UserTbl, Qt::AlignCenter);
}

///
/// @brief
///     选中用户信息改变
///
/// @param[in]  selIndex  选中索引
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
void UserMngDlg::OnSelUserChanged(const QModelIndex& selIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新按钮使能状态
    UpdateBtnEnableStatus();
}

///
/// @brief
///     新增按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
void UserMngDlg::OnAddBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 操作员没有权限添加用户
    if (m_spLoginUserInfo->type == tf::UserType::USER_TYPE_GENERAL)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("【操作员】不能添加用户，请先切换【管理员】账号登录。")));
        pTipDlg->exec();
        return;
    }

    // 获取下一个空行
    int iNextBlamkRow = GetNextBlamkRow();

    // 用户表选项改变
    QItemSelectionModel* selItemModel = ui->UserTbl->selectionModel();
    if (selItemModel != Q_NULLPTR)
    {
        // 项目信息更新
        disconnect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnSelUserChanged(const QModelIndex&)));
        ui->UserTbl->selectRow(iNextBlamkRow);
        ui->DelBtn->setEnabled(false);
        ui->ResetPasswordBtn->setEnabled(false);
        ui->AutoLoginBtn->setEnabled(false);
        connect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnSelUserChanged(const QModelIndex&)));
    }

    // 弹出新增用户对话框
    m_pAddUserDlg->exec();
}

///
/// @brief
///     修改按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
void UserMngDlg::OnModifyBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 更新用户名显示
    m_pModifyPasswordDlg->ui->ModPageUserNameEdit->setText(QString::fromStdString(m_spLoginUserInfo->nickname));
    m_pModifyPasswordDlg->ui->ModPageUserNameEdit->setEnabled(false);

    // 弹出修改密码对话框
    m_pModifyPasswordDlg->exec();
}

///
/// @brief
///     删除按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月29日，新建函数
///
void UserMngDlg::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 获取当前选中项索引
    QModelIndex selIndex = ui->UserTbl->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_WARN, "%s(), !selIndex.isValid()", __FUNCTION__);
        return;
    }

    // 获取当前选中项用户信息
    int iSelRow = selIndex.row();
    QTableWidgetItem* pSelDbItem = ui->UserTbl->item(iSelRow, USER_TBL_COL_IDX_DB_NO);
    if (pSelDbItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pSelDbItem == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中用户信息
    tf::UserInfo stuUserInfo = pSelDbItem->data(Qt::UserRole).value<tf::UserInfo>();

    // 不允许删除自身
    if (m_spLoginUserInfo->username == stuUserInfo.username)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("用户不允许删除自己！")));
        pTipDlg->exec();
        return;
    }

    // 只能删除比自己权限低的用户
    if (m_spLoginUserInfo->type < stuUserInfo.type ||
        ((m_spLoginUserInfo->type == stuUserInfo.type) && (m_spLoginUserInfo->username != USER_NAME_ADMIN)))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("【%1】不允许删除选中用户！").arg(ConvertTfEnumToQString(m_spLoginUserInfo->type))));
        pTipDlg->exec();
        return;
    }

    // 只能删除比自己权限低的用户
    if (stuUserInfo.username == USER_NAME_ADMIN)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("不允许删除系统内置管理员！")));
        pTipDlg->exec();
        return;
    }

    // 弹框提示是否删除
    {
        QString strTip = tr("确定删除用户") + "【" + QString::fromStdString(stuUserInfo.username) + "】" + tr("？");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), strTip, TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "%s(), Canceled", __FUNCTION__);
            return;
        }
    }

    // 执行删除
    if (!DcsControlProxy::GetInstance()->DeleteUserInfo(stuUserInfo.username))
    {
        ULOG(LOG_ERROR, "%s(), DeleteUserInfo() failed.", __FUNCTION__);
        return;
    }

    // 更新map
    m_mapUserInfo.remove(QString::fromStdString(stuUserInfo.username));

    // 删除成功，清空当前选中行
    ui->UserTbl->removeRow(iSelRow);
    ui->UserTbl->insertRow(ui->UserTbl->rowCount());

    // 更新行号
    UpdateTblRowNum();

    // 判断是否为最后一行
    if (GetNextBlamkRow() <= iSelRow)
    {
        // 选中上一行
        int iLastRow = (iSelRow >= 1) ? (iSelRow - 1) : 0;
        ui->UserTbl->selectRow(iLastRow);
    }
    else
    {
        // 选中下一行
        int iNextRow = (iSelRow < ui->UserTbl->rowCount()) ? (iSelRow) : (ui->UserTbl->rowCount() - 1);
        ui->UserTbl->selectRow(iNextRow);
    }
}

///
/// @brief
///     重置密码按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月2日，新建函数
///
void UserMngDlg::OnResetPasswordBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 获取当前选中项索引
    QModelIndex selIndex = ui->UserTbl->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_WARN, "%s(), !selIndex.isValid()", __FUNCTION__);
        return;
    }

    // 获取当前选中项用户信息
    int iSelRow = selIndex.row();
    QTableWidgetItem* pSelDbItem = ui->UserTbl->item(iSelRow, USER_TBL_COL_IDX_DB_NO);
    if (pSelDbItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pSelDbItem == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中用户信息
    tf::UserInfo stuUserInfo = pSelDbItem->data(Qt::UserRole).value<tf::UserInfo>();

    // 只能重置比自己权限低的用户
    if (m_spLoginUserInfo->type < stuUserInfo.type)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("【%1】不允许重置选中用户密码！").arg(ConvertTfEnumToQString(m_spLoginUserInfo->type))));
        pTipDlg->exec();
        return;
    }

    // 内置管理员可以重置管理员的密码
    if ((m_spLoginUserInfo->type == stuUserInfo.type) && 
        (m_spLoginUserInfo->username != USER_NAME_ADMIN || (stuUserInfo.username == USER_NAME_ADMIN)))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("【%1】不允许重置选中用户密码！").arg(ConvertTfEnumToQString(m_spLoginUserInfo->type))));
        pTipDlg->exec();
        return;
    }

    // 弹框提示是否重置
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("重置密码"), tr("确定重置此用户密码？"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "%s(), Canceled", __FUNCTION__);
            return;
        }
    }

    // 修改密码为123
    // 执行修改
    tf::UserInfo modUserInfo;
    QString strstrNewKey("123");
    modUserInfo.__set_id(stuUserInfo.id);
    modUserInfo.__set_password(strstrNewKey.toStdString());
    if (!DcsControlProxy::GetInstance()->ModefyUserInfo(modUserInfo))
    {
        ULOG(LOG_ERROR, "%s(), ModefyUserInfo() failed.", __FUNCTION__);
        return;
    }

    // 弹框提示重置成功
    QCustomDialog* popTip = new QCustomDialog(this, 2000, tr("重置成功！"));
    popTip->setAttribute(Qt::WA_DeleteOnClose);
    popTip->resize(188, 98);
    //popTip.setProperty("bks", "flashDiglog_success");
    popTip->setStyleSheet("QLabel{\
                            border-width:13px;\
                            width:188px;\
                            height:98px;\
                            border-image:url(:/Leonis/resource/image/bk-flashDiglog-success.png) 13 fill no-repeat;\
                            padding-left:40px;\
                            color: #11b93e;\
                            }");
    //popTip.style()->unpolish(&popTip);
    //popTip.style()->polish(&popTip);
    //popTip.update();
    popTip->show();
}

///
/// @brief
///     更新按钮使能状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月10日，新建函数
///
void UserMngDlg::UpdateBtnEnableStatus()
{
    // 重置密码按钮和删除按钮默认禁能
    ui->DelBtn->setEnabled(false);
    ui->ResetPasswordBtn->setEnabled(false);
    ui->AutoLoginBtn->setEnabled(false);

    // 获取当前选中项索引
    QModelIndex selIndex = ui->UserTbl->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_WARN, "%s(), !selIndex.isValid()", __FUNCTION__);
        return;
    }

    // 获取当前选中项用户信息
    int iSelRow = selIndex.row();
    QTableWidgetItem* pSelDbItem = ui->UserTbl->item(iSelRow, USER_TBL_COL_IDX_DB_NO);
    if (pSelDbItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pSelDbItem == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中用户信息
    tf::UserInfo stuUserInfo = pSelDbItem->data(Qt::UserRole).value<tf::UserInfo>();

    // 更新按钮使能状态
    ui->AutoLoginBtn->setEnabled(IsAllowedToOperateUser(stuUserInfo, OP_USER_METHOD_AUTO_LOGIN));
    ui->DelBtn->setEnabled(IsAllowedToOperateUser(stuUserInfo, OP_USER_METHOD_DELETE));
    ui->ResetPasswordBtn->setEnabled(IsAllowedToOperateUser(stuUserInfo, OP_USER_METHOD_RESET_PASSWORD));

    // 更新自动登录按钮文本
    QString strAutoLoginBtnText;
    if (m_strAutoLoginUser == QString::fromStdString(stuUserInfo.username))
    {
        strAutoLoginBtnText = tr("取消自动登录");
    }
    else
    {
        strAutoLoginBtnText = tr("设为自动登录");
    }
    ui->AutoLoginBtn->setText(strAutoLoginBtnText);
}

///
/// @brief
///     添加用户对话框确定按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月13日，新建函数
///
void UserMngDlg::OnAddUserDlgOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 新增页面
    // 参数检查
    if (m_pAddUserDlg->ui->AddPageLoginNameEdit->text().isEmpty())
    {
        m_pAddUserDlg->ui->ErrTipLab->setText(tr("登录名不能为空！"));
        return;
    }

    // 参数检查
    if (m_pAddUserDlg->ui->AddPageUserNameEdit->text().isEmpty())
    {
        m_pAddUserDlg->ui->ErrTipLab->setText(tr("用户名不能为空！"));
        return;
    }

    // 参数检查
    if (m_pAddUserDlg->ui->AddPagePasswordEdit->text().isEmpty())
    {
        m_pAddUserDlg->ui->ErrTipLab->setText(tr("密码不能为空！"));
        return;
    }

    // 参数检查
    if (m_pAddUserDlg->ui->AddPagePasswordSureEdit->text().isEmpty())
    {
        m_pAddUserDlg->ui->ErrTipLab->setText(tr("确认密码不能为空！"));
        return;
    }

    // 密码确认
    if (m_pAddUserDlg->ui->AddPagePasswordEdit->text() != m_pAddUserDlg->ui->AddPagePasswordSureEdit->text())
    {
        m_pAddUserDlg->ui->ErrTipLab->setText(tr("两次密码不一致，请重新输入。"));
        return;
    }

    // 检查用户是否已存在
    QString strNameRepeatTip = QString("%1") + "【" + QString("%2") + "】" + tr("已存在，请重新输入。");
    for (auto it = m_mapUserInfo.begin(); it != m_mapUserInfo.end(); it++)
    {
        // 登录名相同
        if (m_pAddUserDlg->ui->AddPageLoginNameEdit->text() == QString::fromStdString(it.value().username))
        {
            m_pAddUserDlg->ui->ErrTipLab->setText(strNameRepeatTip.arg(tr("登录名")).arg(QString::fromStdString(it.value().username)));
            return;
        }

        // 用户名相同
        if (m_pAddUserDlg->ui->AddPageUserNameEdit->text() == QString::fromStdString(it.value().nickname))
        {
            m_pAddUserDlg->ui->ErrTipLab->setText(strNameRepeatTip.arg(tr("用户名")).arg(QString::fromStdString(it.value().nickname)));
            return;
        }
    }

    // 构造用户信息(待完善，新需求需要数据库增加字段)
    tf::UserInfo stuUserInfo;
    stuUserInfo.__set_username(m_pAddUserDlg->ui->AddPageLoginNameEdit->text().toStdString());
    stuUserInfo.__set_nickname(m_pAddUserDlg->ui->AddPageUserNameEdit->text().toStdString());
    stuUserInfo.__set_type(tf::UserType::type(m_pAddUserDlg->ui->AddPagePermissionCombo->currentData().toInt()));
    stuUserInfo.__set_password(m_pAddUserDlg->ui->AddPagePasswordEdit->text().toStdString());

    // 添加用户
    tf::ResultLong retLong;
    if (!DcsControlProxy::GetInstance()->AddUserInfo(retLong, stuUserInfo))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败，请检查后重新输入!")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "%s(), AddUserInfo() failed.", __FUNCTION__);
        return;
    }
    stuUserInfo.__set_id(retLong.value);

    // 更新map
    m_mapUserInfo.insert(QString::fromStdString(stuUserInfo.username), stuUserInfo);

    // 同步更新到表格
    int iNextBlankRow = GetNextBlamkRow();

    // 添加数据
    ui->UserTbl->setItem(iNextBlankRow, 0, new QTableWidgetItem(QString::number(iNextBlankRow + 1)));                // 序号
    ui->UserTbl->setItem(iNextBlankRow, 1, new QTableWidgetItem(QString::fromStdString(stuUserInfo.username)));      // 登录名
    ui->UserTbl->setItem(iNextBlankRow, 2, new QTableWidgetItem(QString::fromStdString(stuUserInfo.nickname)));      // 用户名
    ui->UserTbl->setItem(iNextBlankRow, 3, new QTableWidgetItem(ConvertTfEnumToQString(stuUserInfo.type)));          // 权限
    ui->UserTbl->setItem(iNextBlankRow, 4, new QTableWidgetItem(""));                                                // 自动登录
    ui->UserTbl->setItem(iNextBlankRow, 5, new QTableWidgetItem(stuUserInfo.id));                                    // 数据库主键

    // 缓存用户信息数据
    ui->UserTbl->item(iNextBlankRow, USER_TBL_COL_IDX_DB_NO)->setData(Qt::UserRole, QVariant::fromValue<tf::UserInfo>(stuUserInfo));

    // 表格文本居中
    SetTblTextAlign(ui->UserTbl, Qt::AlignCenter);

    // 选中下一个空白行
    ui->UserTbl->selectRow(GetNextBlamkRow());

    // 关闭对话框
    m_pAddUserDlg->accept();
    UpdateBtnEnableStatus();
}

///
/// @brief
///     修改密码对话框确定按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月13日，新建函数
///
void UserMngDlg::OnModifyPasswordDlgOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 修改密码
    // 参数检查
    if (m_pModifyPasswordDlg->ui->ModPageUserNameEdit->text().isEmpty())
    {
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("用户名不能为空！"));
        return;
    }

    // 参数检查
    if (m_pModifyPasswordDlg->ui->ModPageOldkeyEdit->text().isEmpty())
    {
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("原密码不能为空！"));
        return;
    }

    // 参数检查
    if (m_pModifyPasswordDlg->ui->ModPageNewkeyEdit->text().isEmpty())
    {
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("新密码不能为空！"));
        return;
    }

    // 参数检查
    if (m_pModifyPasswordDlg->ui->ModPageNewkeySureEdit->text().isEmpty())
    {
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("确认密码不能为空！"));
        return;
    }

    // 密码确认
    if (m_pModifyPasswordDlg->ui->ModPageNewkeyEdit->text() != m_pModifyPasswordDlg->ui->ModPageNewkeySureEdit->text())
    {
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("两次密码不一致，请重新输入。"));
        return;
    }

    // 验证原密码
    std::string strUserName = m_spLoginUserInfo->username;
    std::string strOldKey = m_pModifyPasswordDlg->ui->ModPageOldkeyEdit->text().toStdString();
    std::string strNewKey = m_pModifyPasswordDlg->ui->ModPageNewkeyEdit->text().toStdString();
    if (!DcsControlProxy::GetInstance()->UserIdentityVerify(strUserName, strOldKey))
    {
        ULOG(LOG_INFO, "%s(), UserIdentityVerify() failed.", __FUNCTION__);
        m_pModifyPasswordDlg->ui->ErrTipLab->setText(tr("原密码错误，请重新输入。"));
        return;
    }

    // 执行修改
    tf::UserInfo modUserInfo;
    modUserInfo.__set_id(m_spLoginUserInfo->id);
    modUserInfo.__set_password(strNewKey);
    if (!DcsControlProxy::GetInstance()->ModefyUserInfo(modUserInfo))
    {
        ULOG(LOG_ERROR, "%s(), ModefyUserInfo() failed.", __FUNCTION__);
        return;
    }

    // 关闭对话框
    m_pModifyPasswordDlg->accept();
    UpdateBtnEnableStatus();
}

///
/// @brief
///     自动登录按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月8日，新建函数
///
void UserMngDlg::OnAutoLoginBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 当前登录用户不能为空
    if (m_spLoginUserInfo == Q_NULLPTR)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请先登录！")));
        pTipDlg->exec();
        return;
    }

    // 获取当前选中项索引
    QModelIndex selIndex = ui->UserTbl->currentIndex();
    if (!selIndex.isValid())
    {
        ULOG(LOG_WARN, "%s(), !selIndex.isValid()", __FUNCTION__);
        return;
    }

    // 获取当前选中项用户信息
    int iSelRow = selIndex.row();
    QTableWidgetItem* pSelDbItem = ui->UserTbl->item(iSelRow, USER_TBL_COL_IDX_DB_NO);
    if (pSelDbItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pSelDbItem == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中用户信息
    tf::UserInfo stuUserInfo = pSelDbItem->data(Qt::UserRole).value<tf::UserInfo>();

    // 管理员以上级别才能设置自动登录
    if (m_spLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("操作员不允许设置自动登录！")));
        pTipDlg->exec();
        return;
    }

    // 只允许设置自己和比自己权限低的用户
    if (m_spLoginUserInfo->type < stuUserInfo.type ||
        ((m_spLoginUserInfo->type == stuUserInfo.type) &&
         (m_spLoginUserInfo->username != stuUserInfo.username) &&
         (m_spLoginUserInfo->username != USER_NAME_ADMIN)))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("【%1】不允许对选中用户设置自动登录！").arg(ConvertTfEnumToQString(m_spLoginUserInfo->type))));
        pTipDlg->exec();
        return;
    }

    // 确认提示
    QString strSureTip("");
    if (m_strAutoLoginUser == QString::fromStdString(stuUserInfo.username))
    {
        strSureTip = tr("确定取消设置该用户为自动登录用户？");
    }
    else
    {
        strSureTip = tr("确定设置该用户为自动登录用户？");
    }

    // 确认设置
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("自动登录"), strSureTip, TipDlgType::TWO_BUTTON));
    if (pTipDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Canceled", __FUNCTION__);
        return;
    }

    // 判断当前用户是否为自动登录用户，如果是则取消自动登录
    std::string strName;
    if (m_strAutoLoginUser != QString::fromStdString(stuUserInfo.username))
    {
        strName = stuUserInfo.username;
    }
    if (!DictionaryQueryManager::SaveAutoLoginUser(strName))
    {
        ULOG(LOG_ERROR, "Failed to save autologin user.");
        return;
    }

    // 更新界面
    m_strAutoLoginUser = QString::fromStdString(strName);
    for (int iRow = 0; iRow < ui->UserTbl->rowCount(); iRow++)
    {
        // 获取自动登录项
        QTableWidgetItem* pDbItem = ui->UserTbl->item(iRow, USER_TBL_COL_IDX_DB_NO);
        if (pDbItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取选中用户信息
        tf::UserInfo stuUserInfo = pDbItem->data(Qt::UserRole).value<tf::UserInfo>();

        // 自动登录项
        QTableWidgetItem* pAutoLoginItem = ui->UserTbl->item(iRow, USER_TBL_COL_IDX_AUTO_LOGIN);
        if (pAutoLoginItem == Q_NULLPTR)
        {
            continue;
        }

        // 判断是否为自动登录用户
        if (m_strAutoLoginUser == QString::fromStdString(stuUserInfo.username))
        {
            pAutoLoginItem->setText(tr("自动登录"));
        }
        else
        {
            pAutoLoginItem->setText("");
        }
    }

    // 更新按钮使能状态
    UpdateBtnEnableStatus();
}

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
/// @file     dbgprocessdlg.cpp
/// @brief    调试流程对话框
///
/// @author   4170/TangChuXian
/// @date     2024年2月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "dbgprocessdlg.h"
#include "ui_dbgprocessdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/basedlg.h"
#include "shared/uidef.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "uidcsadapter/uidcsadapter.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "shared/CommonInformationManager.h"
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QRadioButton>

DbgProcessDlg::DbgProcessDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_ciDefaultRowCnt(20),
      m_ciDefaultColCnt(3),
      m_cstrFileDirPath(QCoreApplication::applicationDirPath() + "/dbg_process")
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::DbgProcessDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

DbgProcessDlg::~DbgProcessDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建函数
///
void DbgProcessDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表格行列数
    ui->TestProcessTbl->setRowCount(m_ciDefaultRowCnt);
    ui->TestProcessTbl->setColumnCount(m_ciDefaultColCnt);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<DbgProcessHeader, QString>> ssStateHeader = {
        { Tph_Name, tr("名称") },
        { Tph_RepeatTime, tr("重复次数") },
        { Tph_Result, tr("结果") }
    };
    QStringList strHeaderList;
    for (auto it = ssStateHeader.begin(); it != ssStateHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    ui->TestProcessTbl->setHorizontalHeaderLabels(strHeaderList);

    // 隐藏垂直表头
    ui->TestProcessTbl->verticalHeader()->setVisible(false);

    // 表格列宽自适应
    ResizeTblColToContent(ui->TestProcessTbl);

    // 根据设备列表创建单选框
    QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevListFrame);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(10);
    pHlayout->addStretch(1);

    // 获取设备列表
    if (!gUiAdapterPtr()->GetGroupDevNameList(m_strDevNameList, tf::DeviceType::DEVICE_TYPE_I6000))
    {
        ULOG(LOG_ERROR, "%s(), GetDevNameList() failed", __FUNCTION__);
        return;
    }

    // 依次添加具体设备
    QRadioButton* pFirstRBtn = Q_NULLPTR;
    for (const auto& strDevName : m_strDevNameList)
    {
        QRadioButton* pRBtn = new QRadioButton(strDevName);
        if (strDevName == m_strDevNameList.front())
        {
            m_strCurDevName = strDevName;
            pRBtn->setChecked(true);
            pFirstRBtn = pRBtn;
        }

        connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
        pHlayout->addWidget(pRBtn);
    }

    // 第一个按钮检查
    if (pFirstRBtn == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), device Empty", __FUNCTION__);
        return;
    }

    // 如果仅一个设备，不允许点击
    if (m_strDevNameList.size() <= 1)
    {
        pFirstRBtn->setEnabled(false);
    }
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建函数
///
void DbgProcessDlg::InitAfterShow()
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
void DbgProcessDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建函数
///
void DbgProcessDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 导入按钮被点击
    connect(ui->InportBtn, SIGNAL(clicked()), this, SLOT(OnInportBtnClicked()));

    // 执行按钮被点击
    connect(ui->ExeBtn, SIGNAL(clicked()), this, SLOT(OnExcuteBtnClicked()));

    // 删除按钮被点击
    connect(ui->DelBtn, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));

    // 表格选中项改变
    connect(ui->TestProcessTbl->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(OnTableSelectChanged()));

    // 监听执行结果
    REGISTER_HANDLER(MSG_ID_DEBUGPRO_RESULT, this, OnRecvExeRlt);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月23日，新建函数
///
void DbgProcessDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("调试流程"));

//     QStringList strHeaderList;
//     strHeaderList << tr("序号") << tr("登录名") << tr("用户名") << tr("用户权限") << tr("自动登录");
//     ui->UserTbl->setHorizontalHeaderLabels(strHeaderList);
//     ui->UserTbl->verticalHeader()->setVisible(false);
// 
//     // 设置表格选中模式为行选中，不可多选
    ui->TestProcessTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->TestProcessTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->TestProcessTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 更新调试流程表
    UpdateDbgProcessTbl();

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
    //UpdateUserInfoTbl();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void DbgProcessDlg::showEvent(QShowEvent *event)
{
    // 调用基类接口
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
///     执行按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void DbgProcessDlg::OnExcuteBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取当前选中项
    QModelIndex selIndex = ui->TestProcessTbl->currentIndex();
    if (!selIndex.isValid() || ui->TestProcessTbl->item(selIndex.row(), Tph_Name) == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), sel invalid item", __FUNCTION__);
        return;
    }

    // 设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);

    // 获取选中设备信息
    auto spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn.toStdString());
    if (spDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), spDevInfo == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 如果不是待机或预热状态，则返回
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != spDevInfo->status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != spDevInfo->status))
    {
        ULOG(LOG_INFO, "%s(), device status do not suport!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前仪器状态不允许执行!")));
        pTipDlg->exec();
        return;
    }

    // 获取选中项名称
    QString strDbgModuleName = ui->TestProcessTbl->item(selIndex.row(), Tph_Name)->text();
    if (strDbgModuleName.isEmpty() || ui->lineEdit->text().toInt() <= 0)
    {
        ULOG(LOG_INFO, "%s(), invalid param", __FUNCTION__);
        return;
    }

    // 构造文件名
    QString strSrcFIleName = strDbgModuleName + ".src";

    // 组合新路径
    QString strFilePath = m_cstrFileDirPath + "/" + strSrcFIleName;

    // 执行
    im::tf::DebugProcess Process;
    Process.__set_FileName(strFilePath.toLocal8Bit().data());
    Process.__set_deviceSN(strDevSn.toStdString());
    Process.__set_ModuleName(strDbgModuleName.toLocal8Bit().data());
    Process.__set_RepetitonTimes(ui->lineEdit->text().toInt());
    if (!im::LogicControlProxy::ExecuteDebugProcess(Process))
    {
        ULOG(LOG_ERROR, "%s(), ExecuteDebugProcess() failed", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        return;
    }

    // 正在执行关闭按钮置灰
    ui->ExeBtn->setEnabled(false);
    ui->CloseBtn->setEnabled(false);
    ui->DevListFrame->setEnabled(false);

    // 更新界面显示
    QTableWidgetItem* pRepeatTimeItem = ui->TestProcessTbl->item(selIndex.row(), Tph_RepeatTime);
    QTableWidgetItem* pExeRltItem = ui->TestProcessTbl->item(selIndex.row(), Tph_Result);
    if (pRepeatTimeItem == Q_NULLPTR)
    {
        ui->TestProcessTbl->setItem(selIndex.row(), Tph_RepeatTime, new QTableWidgetItem(QString::number(Process.RepetitonTimes)));
    }
    else
    {
        pRepeatTimeItem->setText(QString::number(Process.RepetitonTimes));
    }
    pExeRltItem->setText("");

    // 缓存数据
    mapExeBufer.insert(qMakePair(strDevSn, strDbgModuleName), qMakePair(Process.RepetitonTimes, QString("")));
}

///
/// @brief
///     删除按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月15日，新建函数
///
void DbgProcessDlg::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取当前选中项
    QModelIndex selIndex = ui->TestProcessTbl->currentIndex();
    if (!selIndex.isValid() || ui->TestProcessTbl->item(selIndex.row(), Tph_Name) == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), sel invalid item", __FUNCTION__);
        return;
    }

    // 获取选中项名称
    QString strDbgModuleName = ui->TestProcessTbl->item(selIndex.row(), Tph_Name)->text();

    // 构造文件名
    QString strSrcFIleName = strDbgModuleName + ".src";
    QString strTextFIleName = strDbgModuleName + ".txt";

    // 组合新路径
    QFile srcCopyFile(m_cstrFileDirPath + "/" + strSrcFIleName);
    QFile textCopyFile(m_cstrFileDirPath + "/" + strTextFIleName);

    // 删除对应文件
    if (srcCopyFile.exists())
    {
        srcCopyFile.remove();
    }

    if (textCopyFile.exists())
    {
        textCopyFile.remove();
    }

    // 表格删除对应行
    ui->TestProcessTbl->removeRow(selIndex.row());
    ui->TestProcessTbl->setRowCount(m_ciDefaultRowCnt);
}

///
/// @brief
///     导入按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月15日，新建函数
///
void DbgProcessDlg::OnInportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("调试流程导入"), ".", tr("调试流程文件(*.src)"));
    if (fileNames.isEmpty())
    {
        ULOG(LOG_INFO, "%s(), not sel file", __FUNCTION__);
        return;
    }

    // 拷贝文件
    // 获取执行程序目录
    QDir dir(m_cstrFileDirPath);
    if (!dir.exists())
    {
        dir.mkpath(m_cstrFileDirPath);
    }

    QStringList strDbgModuleNameLst;
    for (const QString& strSrcFilePath : fileNames)
    {
        // src文件名
        QString strSrcFIleName = strSrcFilePath.section("/", -1, -1);

        // text文本文件名
        QString strTextFIleName = strSrcFIleName;
        strTextFIleName.replace(".src", ".txt");

        // text文本文件路径
        QString strTextFilePath = strSrcFilePath;
        strTextFilePath.replace(".src", ".txt");

        QFile srcFile(strSrcFilePath);
        QFile textFile(strTextFilePath);

        // 组合新路径
        QFile srcCopyFile(m_cstrFileDirPath + "/" + strSrcFIleName);
        QFile textCopyFile(m_cstrFileDirPath + "/" + strTextFIleName);
        bool Delret1 = false;
        bool Delret2 = false;
        if (srcCopyFile.exists())
        {
            Delret1 = QFile::remove(m_cstrFileDirPath + "/" + strSrcFIleName);
        }
        if (textCopyFile.exists())
        {
            Delret2 = QFile::remove(m_cstrFileDirPath + "/" + strTextFIleName);
        }

        // 拷贝文件
        Delret1 = srcFile.copy(m_cstrFileDirPath + "/" + strSrcFIleName);
        Delret2 = textFile.copy(m_cstrFileDirPath + "/" + strTextFIleName);

        // 获取调试模块名称
        QString strDbgModuleName = strSrcFIleName.remove(".src");
        strDbgModuleNameLst.push_back(strDbgModuleName);
    }

    // 如果表格中没有对应项，则将导入项插入表格中
    for (int iRow = 0; iRow < ui->TestProcessTbl->rowCount(); iRow++)
    {
        QTableWidgetItem* pItem = ui->TestProcessTbl->item(iRow, Tph_Name);
        if (pItem != Q_NULLPTR)
        {
            // 如果已经存在则移除
            strDbgModuleNameLst.removeOne(pItem->text());
            continue;;
        }

        // 插入新的名称
        for (const QString& strDbgModuleName : strDbgModuleNameLst)
        {
            ui->TestProcessTbl->setItem(iRow, Tph_Name, new QTableWidgetItem(strDbgModuleName));
            ++iRow;
        }
        break;
    }

    // 刷新表格
    UpdateDbgProcessTbl();
}

///
/// @brief
///     更新调试流程表
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月18日，新建函数
///
void DbgProcessDlg::UpdateDbgProcessTbl()
{
    // 记录之前选中的文本
    QModelIndex selIndex = ui->TestProcessTbl->currentIndex();
    QString strSelModule("");
    if (selIndex.isValid() && ui->TestProcessTbl->item(selIndex.row(), 0) != Q_NULLPTR)
    {
        strSelModule = ui->TestProcessTbl->item(selIndex.row(), 0)->text();
    }

    // 清空表格内容
    ui->TestProcessTbl->clearContents();

    // 获取调试流程文件目录
    QDir dir(m_cstrFileDirPath);
    if (!dir.exists())
    {
        ULOG(LOG_INFO, "%s(), !dir.exists()", __FUNCTION__);
        return;
    }

    // 文件过滤
    QStringList filterList;
//     filterList.push_back(".src");
//     filterList.push_back(".txt");

    // 获取所有调试模块
    QStringList fileNameList;
    QSet<QString> setStrModuleName;
    fileNameList = dir.entryList(filterList, QDir::Files);
    for (const QString& strFIleName : fileNameList)
    {
        // 如果不是src文件，则跳过
        if (!strFIleName.contains(".src"))
        {
            continue;
        }

        QString strModeleName = strFIleName.section(".", 0, 0);
        setStrModuleName.insert(strModeleName);
    }

    // 设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);

    // 插入表格
    int iRow = 0;
    for (const QString& strModeleName : setStrModuleName)
    {
        // 行自增
        if (iRow >= ui->TestProcessTbl->rowCount())
        {
            ui->TestProcessTbl->setRowCount(iRow + 1);
        }

        QString strRepeatTime("");
        QString strExeRlt("");
        auto it = mapExeBufer.find(qMakePair(strDevSn, strModeleName));
        if (it != mapExeBufer.end())
        {
            strRepeatTime = QString::number(it.value().first);
            strExeRlt = it.value().second;
        }

        ui->TestProcessTbl->setItem(iRow, 0, new QTableWidgetItem(strModeleName));
        ui->TestProcessTbl->setItem(iRow, 1, new QTableWidgetItem(strRepeatTime));
        ui->TestProcessTbl->setItem(iRow, 2, new QTableWidgetItem(strExeRlt));

        // 尝试选中之前的模块
        if (!strSelModule.isEmpty() && strSelModule == strModeleName)
        {
            ui->TestProcessTbl->selectRow(iRow);
        }

        ++iRow;
    }
}

///
/// @brief
///     表格选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月18日，新建函数
///
void DbgProcessDlg::OnTableSelectChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空操作步骤内容
    ui->OpStepContentLab->clear();

    // 获取当前选中项
    QModelIndex selIndex = ui->TestProcessTbl->currentIndex();
    if (!selIndex.isValid() || ui->TestProcessTbl->item(selIndex.row(), Tph_Name) == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), sel invalid item", __FUNCTION__);
        return;
    }

    // 获取选中项名称
    QString strDbgModuleName = ui->TestProcessTbl->item(selIndex.row(), Tph_Name)->text();

    // 构造文件名
    QString strTextFIleName = strDbgModuleName + ".txt";

    // 组合新路径
    QFile textCopyFile(m_cstrFileDirPath + "/" + strTextFIleName);

    // 文件不存在则返回
    if (!textCopyFile.exists())
    {
        return;
    }

    // 读取文本
    if (textCopyFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&textCopyFile);
        ui->OpStepContentLab->setText(in.readAll());
        textCopyFile.close();
    }

    // 设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);

    // 如果该模块正在执行，删除按钮置灰
    auto it = mapExeBufer.find(qMakePair(strDevSn, strDbgModuleName));
    if (it == mapExeBufer.end())
    {
        // 没执行过
        ui->DelBtn->setEnabled(true);
        return;
    }

    // 结果非空则使能
    ui->DelBtn->setEnabled(!it.value().second.isEmpty());
}

///
/// @brief
///     设备单选框被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void DbgProcessDlg::OnDevRBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
    if (pRBtn == Q_NULLPTR)
    {
        return;
    }

    // 如果更新当前设备名
    m_strCurDevName = pRBtn->text();

    // 刷新表
    UpdateDbgProcessTbl();
}

///
/// @brief
///     收到调试流程
///
/// @param[in]  stuTfDbgProc  调试流程
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void DbgProcessDlg::OnRecvExeRlt(im::tf::DebugProcess stuTfDbgProc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 执行完成关闭按钮使能
    ui->ExeBtn->setEnabled(true);
    ui->CloseBtn->setEnabled(true);
    ui->DevListFrame->setEnabled(true);

    // 插入缓存
    auto it = mapExeBufer.find(qMakePair(QString::fromStdString(stuTfDbgProc.deviceSN), QString::fromLocal8Bit(stuTfDbgProc.ModuleName.c_str())));
    if (it == mapExeBufer.end())
    {
        ULOG(LOG_INFO, "%s(),ignore, no data match", __FUNCTION__);
        return;
    }

    it.value().first = stuTfDbgProc.RepetitonTimes;
    it.value().second = stuTfDbgProc.bSucceed ? tr("成功") : tr("失败");

    // 如果设备不匹配，则返回
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);
    if (strDevSn != QString::fromStdString(stuTfDbgProc.deviceSN))
    {
        ULOG(LOG_INFO, "%s(),ignore, strDevSn not match", __FUNCTION__);
        return;
    }

    // 刷新表格
    for (int iRow = 0; iRow < ui->TestProcessTbl->rowCount(); iRow++)
    {
        QTableWidgetItem* pItem = ui->TestProcessTbl->item(iRow, Tph_Name);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        QString strModeleName = pItem->text();
        if (strModeleName != QString::fromLocal8Bit(stuTfDbgProc.ModuleName.c_str()))
        {
            continue;
        }

        // 刷新状态
        pItem = ui->TestProcessTbl->item(iRow, Tph_RepeatTime);
        if (pItem == Q_NULLPTR)
        {
            ui->TestProcessTbl->setItem(iRow, Tph_RepeatTime, new QTableWidgetItem(QString::number(stuTfDbgProc.RepetitonTimes)));
        }
        else
        {
            pItem->setText(QString::number(stuTfDbgProc.RepetitonTimes));
        }

        // 刷新状态
        pItem = ui->TestProcessTbl->item(iRow, Tph_Result);
        if (pItem == Q_NULLPTR)
        {
            ui->TestProcessTbl->setItem(iRow, Tph_Result, new QTableWidgetItem(it.value().second));
        }
        else
        {
            pItem->setText(it.value().second);
        }

        break;
    }
}

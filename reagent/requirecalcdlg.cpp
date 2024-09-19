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
/// @file     requirecalcdlg.cpp
/// @brief    需求计算对话框
///
/// @author   4170/TangChuXian
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "requirecalcdlg.h"
#include "ui_requirecalcdlg.h"
#include <QRadioButton>
#include "QHBoxLayout"

#include <memory>
#include "requirecalcwidget.h"
#include "requirecfgdlg.h"
#include "uidcsadapter/uidcsadapter.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "shared/tipdlg.h"

#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(SPL_REQ_VOL_INFO)

RequireCalcDlg::RequireCalcDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化ui对象
    ui = new Ui::RequireCalcDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

RequireCalcDlg::~RequireCalcDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("需求计算"));

    // 添加Tab项
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Monday), tr("星期一"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Tuesday), tr("星期二"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Wednesday), tr("星期三"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Thursday), tr("星期四"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Friday), tr("星期五"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Saturday), tr("星期六"));
    ui->WeekReqCalcTabWgt->addTab(new RequireCalcWidget(Qt::Sunday), tr("星期日"));

    // 选中当前日期
    int iDayOfWeek = QDate::currentDate().dayOfWeek();
    if (iDayOfWeek > 0)
    {
        ui->WeekReqCalcTabWgt->setCurrentIndex(iDayOfWeek - 1);
    }

    // 角落窗口
    m_pAllDevRBtn = new QRadioButton(tr("全部"));
    m_pAllChRBtn = new QRadioButton(tr("生化"));
    m_pAllImRBtn = new QRadioButton(tr("免疫"));
    QWidget* pCornerWidget = new QWidget();
    QHBoxLayout* pHLayout = new QHBoxLayout(pCornerWidget);
    pHLayout->setSpacing(10);
    pHLayout->setContentsMargins(10, 0, 10, 10);
    pHLayout->addWidget(m_pAllDevRBtn);
    pHLayout->addWidget(m_pAllChRBtn);
    pHLayout->addWidget(m_pAllImRBtn);
    ui->WeekReqCalcTabWgt->setCornerWidget(pCornerWidget);

    // 判断模式
    if (gUiAdapterPtr()->WhetherOnlyImDev())
    {
        // 免疫单机
        m_pAllImRBtn->setChecked(true);
        m_pAllImRBtn->setVisible(false);
        m_pAllDevRBtn->setVisible(false);
        m_pAllChRBtn->setVisible(false);
    }
    else if(gUiAdapterPtr()->WhetherContainImDev())
    {
        // 生免联机
        m_pAllDevRBtn->setChecked(true);
    }
    else
    {
        // 生化单机
        m_pAllChRBtn->setChecked(true);
        m_pAllImRBtn->setVisible(false);
        m_pAllDevRBtn->setVisible(false);
        m_pAllChRBtn->setVisible(false);
    }

    // 连接信号槽，监听设备类型改变
    connect(m_pAllDevRBtn, SIGNAL(clicked()), this, SLOT(LoadData()));
    connect(m_pAllChRBtn, SIGNAL(clicked()), this, SLOT(LoadData()));
    connect(m_pAllImRBtn, SIGNAL(clicked()), this, SLOT(LoadData()));

    // 加载数据
    LoadData();
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
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
void RequireCalcDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(close()));

    // 确定按钮被点击
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));

    // 需求设置按钮被点击
    connect(ui->ReqCalcCfgBtn, SIGNAL(clicked()), this, SLOT(OnReqCalcCfgBtnClicked()));

    // 打印按钮被点击
    connect(ui->PrintDataBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCalcDlg::showEvent(QShowEvent *event)
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
///     加载数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
void RequireCalcDlg::LoadData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 构造设备类型
    int iDevClassify = 0;
    if (m_pAllDevRBtn->isChecked())
    {
        iDevClassify = DEVICE_CLASSIFY_CHEMISTRY | DEVICE_CLASSIFY_IMMUNE;
    }
    else if (m_pAllChRBtn->isChecked())
    {
        iDevClassify = DEVICE_CLASSIFY_CHEMISTRY;
    }
    else if (m_pAllImRBtn->isChecked())
    {
        iDevClassify = DEVICE_CLASSIFY_IMMUNE;
    }
    else
    {
        ULOG(LOG_ERROR, "%s(), unknown devclassify", __FUNCTION__);
        return;
    }

    // 加载耗材需求信息
    RequireCalcWidget::LoadData(iDevClassify);
}

///
/// @brief
///     需求计算设置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月27日，新建函数
///
void RequireCalcDlg::OnReqCalcCfgBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出希求计算对话框
    std::shared_ptr<RequireCfgDlg> spImBeadMixDlg(new RequireCfgDlg(this));
    if (spImBeadMixDlg->exec() == QDialog::Rejected)
    {
        return;
    }

    // 刷新界面
    LoadData();
}

///
/// @brief
///     确定按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
void RequireCalcDlg::OnOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 保存数据
    RequireCalcWidget::SaveData();

    accept();
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年3月25日，新建函数
///
void RequireCalcDlg::OnPrintBtnClicked()
{
    RequireCalcInfo Info;
    RequireCalcWidget* pW = qobject_cast<RequireCalcWidget*>(ui->WeekReqCalcTabWgt->currentWidget());
    pW->GetPrintInfo(Info);
    if (Info.ItemVector.empty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可打印的数据!")));
        pTipDlg->exec();
        return;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
}




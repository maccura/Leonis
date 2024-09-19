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
/// @file     SoftwareVersionDlg.cpp
/// @brief    软件版本弹窗
///
/// @author   7951/LuoXin
/// @date     2022年9月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include "ui_SoftwareVersionDlg.h"
#include "ui_SoftwareFontPermitDlg.h"
#include "SoftwareVersionDlg.h"
#include "SoftwareVersionManageDlg.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "src/public/DictionaryKeyName.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

// 表格列
enum TABLEWIDGET_COL
{
    NAME_COL,               // 名称
    VERSION_COL,            // 版本号
    PERMIT_COL,             // 许可证
    SRC_COL                 // 源码地址
};

SoftwareVersionDlg::SoftwareVersionDlg(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::SoftwareVersionDlg)
    , m_pSoftwareVersionManageDlg(nullptr)
{
    ui->setupUi(this);
    BaseDlg::SetTitleName(tr("版本信息"));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    InitTableView();

    connect(ui->help_btn, &QPushButton::clicked, this, &SoftwareVersionDlg::OnServiceHelpClicked);
    connect(ui->close_btn, &QPushButton::clicked, this, &SoftwareVersionDlg::OnCloseBtnClicked);
    connect(ui->detail_btn, &QPushButton::clicked, this, &SoftwareVersionDlg::OnDetailBtnClicked);
    connect(ui->update_btn, &QPushButton::clicked, this, &SoftwareVersionDlg::OnUpdateBtnClicked);

    QString text = ui->label_2->text();
    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type == IMMUNE)
    {
        text = text.arg(tr("全自动化学发光免疫分析仪"));
    }
    else if (type == CHEMISTRY)
    {
        text = text.arg(tr("全自动生化分析仪"));
    }
    else
    {
        text = text.arg(tr("全自动生化分析仪、全自动化学发光免疫分析仪"));
    }

    text = text.replace("YEAR",QString::number(QDate::currentDate().year()));

    ui->label_2->setText(text);
    ui->stackedWidget->setCurrentIndex(0);
	ui->tableWidget->setColumnWidth(0, 230);
	ui->tableWidget->setColumnWidth(1, 160);
	ui->tableWidget->setColumnWidth(2, 160);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->horizontalHeader()->setMinimumSectionSize(100);
}

SoftwareVersionDlg::~SoftwareVersionDlg()
{

}

void SoftwareVersionDlg::LoadDataToDlg()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QString vstr = CurrentSoftwareVersion();
    QString showText = tr("软件发布版本号：") + vstr[0] + "\n";
    showText += tr("软件完整版本号：") + vstr;
	ui->label->setText(showText);
    ui->stackedWidget->setCurrentIndex(0);
}

bool SoftwareVersionDlg::SaveVersion2Db()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	::tf::DictionaryInfo di;
	di.__set_keyName(DKN_SYSTEM_SOFTWARE_VERSION);
	di.__set_value(SOFTWARE_VERSION);

	// 因为仪器云会从数据库中读取软件版本号，所以更新数据库中的软件版本号
	std::shared_ptr<DcsControlProxy> dcp = DcsControlProxy::GetInstance();
	if (!dcp->ModifyDictionaryInfo(di))
	{
		ULOG(LOG_ERROR, "Failed to execute ModifyDictionaryInfo()");
		return false;
	}

	return true;
}

void SoftwareVersionDlg::InitTableView()
{
    // 字体
    {
        // 初始化服务帮助表格内容
        auto tempSi = new QTableWidgetItem(tr("字体著作权声明"));
        tempSi->setToolTip(tr("字体著作权声明"));
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, NAME_COL, tempSi);

        // 字体版本
        tempSi = new QTableWidgetItem("2021");
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(0, VERSION_COL, tempSi);

        // 许可证
        QPushButton* permitBtn = new QPushButton("", ui->tableWidget);
        permitBtn->setObjectName("permitBtn");
        ui->tableWidget->setItem(0, PERMIT_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(0, PERMIT_COL, permitBtn);

        SoftwareFontPermitDlg* sfp = new SoftwareFontPermitDlg(this);
        connect(permitBtn, &QPushButton::clicked, this, [sfp] 
        { sfp->showWidgetByType(SoftwareFontPermitDlg::widgetType::FRONT_WIDGET); });

        // 源码地址
        QPushButton* frontSrcBtn = new QPushButton("", ui->tableWidget);
        frontSrcBtn->setObjectName("frontSrcBtn");
        ui->tableWidget->setItem(0, SRC_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(0, SRC_COL, frontSrcBtn);

        TipDlg* tipDlg = new TipDlg(tr("源代码地址"), FONT_SRC_PATH, TipDlgType::SINGLE_BUTTON, this);
        tipDlg->SetButtonText(tr("确定"), tr("关闭"));
        connect(frontSrcBtn, &QPushButton::clicked, this, [tipDlg] { tipDlg->exec(); });
    }

    // 软件
    {
        // 初始化服务帮助表格内容
        auto tempSi = new QTableWidgetItem(tr("软件版权声明"));
        tempSi->setToolTip(tr("软件版权声明"));
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(1, NAME_COL, tempSi);

        // 版本
        tempSi = new QTableWidgetItem("1");
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(1, VERSION_COL, tempSi);

        // 许可证
        QPushButton* leonisBtn = new QPushButton("", ui->tableWidget);
        leonisBtn->setObjectName("leonisBtn");
        ui->tableWidget->setItem(1, PERMIT_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(1, PERMIT_COL, leonisBtn);

        SoftwareFontPermitDlg* sfp = new SoftwareFontPermitDlg(this);
        connect(leonisBtn, &QPushButton::clicked, this, [sfp] 
        { sfp->showWidgetByType(SoftwareFontPermitDlg::widgetType::LEONIS_WIDGET); });

        // 源码地址
        QPushButton* leonisSrcBtn = new QPushButton("", ui->tableWidget);
        leonisSrcBtn->setObjectName("leonisSrcBtn");
        ui->tableWidget->setItem(1, SRC_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(1, SRC_COL, leonisSrcBtn);

        TipDlg* tipDlg = new TipDlg(tr("源代码地址"), LEONIS_SRC_PATH, TipDlgType::SINGLE_BUTTON, this);
        tipDlg->SetButtonText(tr("确定"), tr("关闭"));
        connect(leonisSrcBtn, &QPushButton::clicked, this, [tipDlg] { tipDlg->exec(); });
    }

    // Qt
    {
        // 初始化服务帮助表格内容
        auto tempSi = new QTableWidgetItem(tr("Qt版权声明"));
        tempSi->setToolTip(tr("Qt版权声明"));
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(2, NAME_COL, tempSi);

        // 版本
        tempSi = new QTableWidgetItem("3");
        tempSi->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(2, VERSION_COL, tempSi);

        // 许可证
        QPushButton* qtBtn = new QPushButton("", ui->tableWidget);
        qtBtn->setObjectName("qtBtn");
        ui->tableWidget->setItem(2, PERMIT_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(2, PERMIT_COL, qtBtn);

        SoftwareFontPermitDlg* sfp = new SoftwareFontPermitDlg(this);
        connect(qtBtn, &QPushButton::clicked, this, [sfp] 
        { sfp->showWidgetByType(SoftwareFontPermitDlg::widgetType::QT_WIDGET); });

        // 源码地址
        QPushButton* qtSrcBtn = new QPushButton("", ui->tableWidget);
        qtSrcBtn->setObjectName("qtSrcBtn");
        ui->tableWidget->setItem(2, SRC_COL, new QTableWidgetItem());
        ui->tableWidget->setCellWidget(2, SRC_COL, qtSrcBtn);

        TipDlg* tipDlg = new TipDlg(tr("源代码地址"), QT_SRC_PATH, TipDlgType::SINGLE_BUTTON, this);
        tipDlg->SetButtonText(tr("确定"), tr("关闭"));
        connect(qtSrcBtn, &QPushButton::clicked, this, [tipDlg] { tipDlg->exec(); });
    }
}

void SoftwareVersionDlg::showEvent(QShowEvent *event)
{
    BaseDlg::showEvent(event);
    LoadDataToDlg();
}

void SoftwareVersionDlg::OnServiceHelpClicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    SetTitleName(tr("服务与帮助"));
    show();
}

void SoftwareVersionDlg::OnCloseBtnClicked()
{
    if (ui->stackedWidget->currentIndex() == 0)
    {
        close();
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(0);
        SetTitleName(tr("版本信息"));
    }
}

void SoftwareVersionDlg::OnDetailBtnClicked()
{
    if (nullptr == m_pSoftwareVersionManageDlg)
    {
        m_pSoftwareVersionManageDlg = new SoftwareVersionManageDlg(this);
    }

    hide();
    m_pSoftwareVersionManageDlg->exec();
    show();
}

void SoftwareVersionDlg::OnUpdateBtnClicked()
{

}

void SoftwareVersionDlg::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto& userPms = UserInfoManager::GetInstance();

    // 管理员以上显示更新
    ui->update_btn->setVisible(userPms->IsPermisson(PSM_SOFTWARE_UPDETE));

    // 工程师以上显示详情
    ui->detail_btn->setVisible(userPms->IsPermisson(PSM_SOFTWARE_DETAIL));
}

QString SoftwareVersionDlg::CurrentTimeString()
{
    QString dataTime;
    dataTime += __DATE__;
    dataTime += __TIME__;
    dataTime.replace("  ", " ");
    QDateTime dt = QLocale(QLocale::English).toDateTime(dataTime, "MMM d yyyyhh:mm:ss");
    return dt.toString("yyMMdd");
}

QString SoftwareVersionDlg::CurrentSoftwareVersion()
{
    ULOG(LOG_INFO, __FUNCTION__);

	// 查询数据字典中记录的版本
	std::string strVer = DictionaryQueryManager::GetInstance()->GetSoftwareVersion();
	if (strVer.empty())
	{
		strVer = SOFTWARE_VERSION;
	}
	
    return QString::fromStdString(strVer);
}

SoftwareFontPermitDlg::SoftwareFontPermitDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::SoftwareFontPermitDlg)
{
    ui->setupUi(this);
    BaseDlg::SetTitleName(tr("许可证"));
}

void SoftwareFontPermitDlg::showWidgetByType(widgetType type)
{
    QString path = qApp->applicationDirPath();
    switch (type)
    {
        case SoftwareFontPermitDlg::FRONT_WIDGET:
            path += "/ui_cfg/FRONT-LICENSE.txt";
            break;
        case SoftwareFontPermitDlg::LEONIS_WIDGET:
            path += "/ui_cfg/LEONIS-LICENSE.txt";
            break;
        case SoftwareFontPermitDlg::QT_WIDGET:
            path += "/COPYING.COPYING";
            break;
        default:
            return;
    }

    ui->textBrowser->clear();
    ui->textBrowser->setPlainText(readFileToQString(path));
    exec();
}

QString SoftwareFontPermitDlg::readFileToQString(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ULOG(LOG_ERROR, "open file failed path[%s]", filePath);
        return "";
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}

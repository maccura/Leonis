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
/// @file     maintainstatusbtn.cpp
/// @brief    维护状态按钮
///
/// @author   4170/TangChuXian
/// @date     2023年2月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainstatusbtn.h"
#include "ui_maintainstatusbtn.h"
#include "uidcsadapter/uidcsadapter.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/uidef.h"
#include "src/common/Mlog/mlog.h"
#include <QTextLayout>
#include <QTextOption>

#define MAINTAIN_STATUS_PROPERTY_NAME                   ("status")              // 维护状态属性名

#define MAINTAIN_STATUS_NOT_MAINTAIN                    ("not_maintain")        // 未维护
#define MAINTAIN_STATUS_NORMAL                          ("maintain_normal")     // 正常
#define MAINTAIN_STATUS_MAINTAIN_FAILED                 ("maintain_failed")     // 失败

#define MAINTAIN_DEV_CLASSIFY_PROPERTY_NAME             ("classify")            // 维护设备类别

#define MAINTAIN_DEV_CLASSIFY_NONE                      ("classify_none")       // 无类别
#define MAINTAIN_DEV_CLASSIFY_IM                        ("classify_im")         // 免疫设备
#define MAINTAIN_DEV_CLASSIFY_CH                        ("classify_ch")         // 生化设备

// 初始化静态成员变量
QMap<QString, QIcon>   MaintainStatusBtn::sm_mapMaintainIcon;                   // 维护图标映射【维护名-图标】

MaintainStatusBtn::MaintainStatusBtn(QWidget *parent)
    : QPushButton(parent),
      m_dProgress(1.0),
      m_enStatus(BTN_MAINTAIN_STATUS_NORMAL),
      m_enType(BTN_MAINTAIN_TYPE_MAINTAIN_GROUP),
      m_enDevClassify(BTN_DEV_CLASSIFY_NONE),
      m_ciTextAreaWidth(158),
      m_ciTextFontSize(16),
      m_csizeIcon(QSize(36, 36))
{
    ui = new Ui::MaintainStatusBtn();
    ui->setupUi(this);

    // 显示之前初始化
    InitBeforeShow();
}

MaintainStatusBtn::~MaintainStatusBtn()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     获取省略文本
///
/// @param[in]  text        原文本
/// @param[in]  font        字体
/// @param[in]  lineWidth   行宽
///
/// @return 省略文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月13日，新建函数
///
QString getElidedText(const QString& text, const QFont& font, quint32 lineWidth)
{
    QTextLayout textLayout(text, font);
    QTextOption option = textLayout.textOption();
    option.setWrapMode(QTextOption::WrapMode::WrapAtWordBoundaryOrAnywhere);
    textLayout.setTextOption(option);

    textLayout.beginLayout();
    while (true)
    {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(lineWidth);
    }
    textLayout.endLayout();

    // 这里只需要显示前两行，可根据需要修改，甚至可以通过参数控制行数
    if (textLayout.lineCount() > 2)
    {
        QTextLine line1 = textLayout.lineAt(0);
        QTextLine line2 = textLayout.lineAt(1);

        int length = line1.textLength() + line2.textLength();
        // 这里length - 1是因为需要留一个字符的空间给省略号，
        // 但是之前大佬说不同的语种和字体，字符的宽度也会不一样，这里可能需要-2来保证显示完全
        return text.mid(0, length - 3) + "...";
    }

    return text;
}

///
/// @brief
///     获取控件的维护
///
/// @return 维护名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
QString MaintainStatusBtn::GetMaintainName()
{
    QString strToolTip = toolTip();
    if (!strToolTip.isEmpty())
    {
        return strToolTip;
    }

    return ui->MaintainNameLab->text();
}

///
/// @brief
///     设置维护名
///
/// @param[in]  strMaintainName  维护名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void MaintainStatusBtn::SetMaintainName(const QString& strMaintainName)
{
    QFont defFont = ui->MaintainNameLab->font();
    defFont.setFamily(FONT_FAMILY);
    defFont.setPixelSize(m_ciTextFontSize);
    QString strElideText = getElidedText(strMaintainName, defFont, m_ciTextAreaWidth);
    ui->MaintainNameLab->setText(strElideText);
    if (strElideText != strMaintainName)
    {
        setToolTip(strMaintainName);
    }

    // 根据维护名设置图标
    auto it = sm_mapMaintainIcon.find(strMaintainName);
    if (it != sm_mapMaintainIcon.end())
    {
        SetIcon(QIcon(it.value()));
    }
}

///
/// @brief
///     获取进度
///
/// @return 进度百分比：1表示100%，0表示0%
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
double MaintainStatusBtn::GetProgress()
{
    return m_dProgress;
}

///
/// @brief
///     设置进度
///
/// @param[in]  dProgress  进度百分比：1表示100%，0表示0%
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void MaintainStatusBtn::SetProgress(double dProgress)
{
    m_dProgress = dProgress;
    int iPercentVal = 100 * dProgress;
    //ui->MaintainProgressBar->setValue(iPercentVal);
    //ui->MaintainProgressLab->setText(QString::number(iPercentVal) + "%");
}

///
/// @brief
///     获取按钮维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
MaintainStatusBtn::BtnMaintainStatus MaintainStatusBtn::GetStatus()
{
    return m_enStatus;
}

///
/// @brief
///     设置按钮维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void MaintainStatusBtn::SetStatus(BtnMaintainStatus enStatus)
{
    // 设置状态
    m_enStatus = enStatus;

    // 根据状态构造对应字符串（用于qss动态属性
    QString strStatus("");
    switch (enStatus)
    {
    case MaintainStatusBtn::BTN_MAINTAIN_STATUS_NORMAL:
        strStatus = MAINTAIN_STATUS_NORMAL;
        break;
    case MaintainStatusBtn::BTN_MAINTAIN_STATUS_FAILED:
        strStatus = MAINTAIN_STATUS_MAINTAIN_FAILED;
        break;
    default:
        break;
    }

    // 更新动态属性
    ui->BkFrame->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    //ui->MaintainProgressLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    //ui->MaintainProgressBar->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);

    // 刷新界面显示
    UpdateStatusDisplay();
}

///
/// @brief
///     获取按钮维护类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
MaintainStatusBtn::BtnMaintainType MaintainStatusBtn::GetMaintainType()
{
    return m_enType;
}

///
/// @brief
///     设置按钮维护类型
///
/// @param[in]  enType  维护类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void MaintainStatusBtn::SetMaintainType(BtnMaintainType enType)
{
    m_enType = enType;
}

///
/// @brief
///     获取设备类别
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月18日，新建函数
///
MaintainStatusBtn::BtnDevClassify MaintainStatusBtn::GetDevClassify()
{
    return m_enDevClassify;
}

///
/// @brief
///     设置设备类别
///
/// @param[in]  enClassify  设备类别
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月18日，新建函数
///
void MaintainStatusBtn::SetDevClassify(BtnDevClassify enClassify)
{
    // 设置设备类别
    m_enDevClassify = enClassify;

    // 根据状态构造对应字符串（用于qss动态属性
    QString strDevClassify("");
    switch (enClassify)
    {
    case MaintainStatusBtn::BTN_DEV_CLASSIFY_NONE:
        ui->DevClassLab->clear();
        break;
    case MaintainStatusBtn::BTN_DEV_CLASSIFY_IM:
        ui->DevClassLab->setText(tr("免疫"));
        strDevClassify = MAINTAIN_DEV_CLASSIFY_IM;
        break;
    case MaintainStatusBtn::BTN_DEV_CLASSIFY_CH:
        ui->DevClassLab->setText(tr("生化"));
        strDevClassify = MAINTAIN_DEV_CLASSIFY_CH;
        break;
    default:
        ui->DevClassLab->clear();
        break;
    }

    // 更新动态属性
    ui->DevClassLab->setProperty(MAINTAIN_DEV_CLASSIFY_PROPERTY_NAME, strDevClassify);
    //ui->MaintainProgressLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    //ui->MaintainProgressBar->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);

    // 如果是单机模式，则不显示设备类别
    if (gUiAdapterPtr()->WhetherSingleDevMode())
    {
        ui->DevClassLab->clear();
        ui->DevClassLab->setProperty(MAINTAIN_DEV_CLASSIFY_PROPERTY_NAME, "");
    }

    // 刷新界面显示
    UpdateStatusDisplay();
}

///
/// @brief
///     进度是否显示
///
/// @return true表示显示进度
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
bool MaintainStatusBtn::IsProgressVisible()
{
    //return ui->MaintainProgressBar->isVisible();
    return true;
}

///
/// @brief
///     设置进度是否显示
///
/// @param[in]  bVisible  是否显示进度
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainStatusBtn::SetProgressVisible(bool bVisible)
{
    //ui->ProgressFrame->setVisible(bVisible);
}

///
/// @brief
///     更新状态显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void MaintainStatusBtn::UpdateStatusDisplay()
{
    // 更新状态(背景)
    ui->BkFrame->style()->unpolish(ui->BkFrame);
    ui->BkFrame->style()->polish(ui->BkFrame);
    ui->BkFrame->update();

    // 更新设备类别
    ui->DevClassLab->style()->unpolish(ui->DevClassLab);
    ui->DevClassLab->style()->polish(ui->DevClassLab);
    ui->DevClassLab->update();

    // 更新状态(进度)
    //ui->MaintainProgressLab->style()->unpolish(ui->MaintainProgressLab);
    //ui->MaintainProgressLab->style()->polish(ui->MaintainProgressLab);
    //ui->MaintainProgressLab->update();

    // 更新状态(项目名)
    //ui->MaintainProgressBar->style()->unpolish(ui->MaintainProgressBar);
    //ui->MaintainProgressBar->style()->polish(ui->MaintainProgressBar);
    //ui->MaintainProgressBar->update();
}

///
/// @brief
///     设置图标
///
/// @param[in]  btnIcon  按钮图标
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月26日，新建函数
///
void MaintainStatusBtn::SetIcon(const QIcon& btnIcon)
{
    ui->IconLab->setPixmap(btnIcon.pixmap(m_csizeIcon));
}

///
/// @brief
///     初始化维护图标映射
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月26日，新建函数
///
void MaintainStatusBtn::InitMaintainIconMap()
{
    // 如果已经有内容直接返回
    if (!sm_mapMaintainIcon.isEmpty())
    {
        return;
    }

	// 单机与联机特殊处理项(可根据该标识动态设置)
	bool isPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();

    // 初始化维护图标
    sm_mapMaintainIcon =
    {
        { QObject::tr("月维护"), QIcon(":/Leonis/resource/image/icon-maintain-ywh.png") },
        { QObject::tr("针清洗"), QIcon(":/Leonis/resource/image/icon-maintain-zqx.png") },
        { QObject::tr("周维护"), QIcon(":/Leonis/resource/image/icon-maintain-zwh.png") },
        { QObject::tr("仪器关机"), QIcon(":/Leonis/resource/image/icon-maintain-zdgj.png") },
        { QObject::tr("杯空白测定"), QIcon(":/Leonis/resource/image/icon-maintain-bkbcd.png") },
        { QObject::tr("测试结束维护"), QIcon(":/Leonis/resource/image/icon-maintain-csjswh.png") },
        { QObject::tr("磁珠混匀"), QIcon(":/Leonis/resource/image/icon-maintain-czhy.png") },
        { QObject::tr("底物泵维护"), QIcon(":/Leonis/resource/image/icon-maintain-dwbwh.png") },
        { QObject::tr("底物管路清洗"), QIcon(":/Leonis/resource/image/icon-maintain-duglqx.png") },
        { QObject::tr("底物停用"), QIcon(":/Leonis/resource/image/icon-maintain-dwty.png") },
        { QObject::tr("电极更换"), QIcon(":/Leonis/resource/image/icon-maintain-djgh.png") },
		{ QObject::tr("电极清洗"), QIcon(":/Leonis/resource/image/icon-maintain-djgh.png") },
        { QObject::tr("反应杯更换"), QIcon(":/Leonis/resource/image/icon-maintain-fybgh.png") },
        { QObject::tr("反应杯清洗"), QIcon(":/Leonis/resource/image/icon-maintain-fybqx.png") },
        { QObject::tr("清洗反应槽"), QIcon(":/Leonis/resource/image/icon-maintain-fycqx.png") },
		{ QObject::tr("清洗供水桶"), QIcon(":/Leonis/resource/image/icon-maintain-qxgst.png") },
		{ QObject::tr("排空供水桶"), QIcon(":/Leonis/resource/image/icon-maintain-pkgst.png") },
		{ QObject::tr("比色管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-bsgltc.png") },
		{ QObject::tr("清洗液管路清洗"), QIcon(":/Leonis/resource/image/icon-maintain-qxyglqx.png") },
		{ QObject::tr("ISE管路清洗"), QIcon(":/Leonis/resource/image/icon-maintain-iseglqx.png") },
		{ QObject::tr("ISE管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-isegltc.png") },
		{ QObject::tr("ISE排液"), QIcon(":/Leonis/resource/image/icon-maintain-isepy.png") },
		{ QObject::tr("ISE清洗液余量检测"), QIcon(":/Leonis/resource/image/icon-maintain-iseqxyyljc.png") },
		{ QObject::tr("超声清洗"), QIcon(":/Leonis/resource/image/icon-maintain-chqx.png") },
        { QObject::tr("反应槽水更换"), QIcon(":/Leonis/resource/image/icon-maintain-fycsgh.png") },
        { QObject::tr("负压泵维护"), QIcon(":/Leonis/resource/image/icon-maintain-fybwh.png") },
        { QObject::tr("关机维护"), QIcon(":/Leonis/resource/image/icon-maintain-gjwh.png") },
        { QObject::tr("管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-gltc.png") },
        { QObject::tr("光度计检查"), QIcon(":/Leonis/resource/image/icon-maintain-gdjjc.png") },
        { QObject::tr("开机维护"), QIcon(":/Leonis/resource/image/icon-maintain-kjwh.png") },
        { QObject::tr("排气"), QIcon(":/Leonis/resource/image/icon-maintain-pq.png") },
        { QObject::tr("清除反应杯"), QIcon(":/Leonis/resource/image/icon-maintain-qcfyb.png") },
        { QObject::tr("清除样本架"), QIcon(":/Leonis/resource/image/icon-maintain-qcybj.png") },
        { QObject::tr("免疫试剂扫描"), QIcon(":/Leonis/resource/image/icon-maintain-sjsm.png") },
        { (isPipeLine ? QObject::tr("生化试剂扫描") : QObject::tr("试剂扫描")), QIcon(":/Leonis/resource/image/icon-maintain-sjsm.png") },
        { QObject::tr("样本针浸泡"), QIcon(":/Leonis/resource/image/icon-maintain-ybzjp.png") },
        { QObject::tr("仪器复位"), QIcon(":/Leonis/resource/image/icon-maintain-yqfw.png") },
        { QObject::tr("抽液针特殊清洗"), QIcon(":/Leonis/resource/image/icon-maintain-cyztsqx.png") },
        { QObject::tr("磁分离管路清洗"), QIcon(":/Leonis/resource/image/icon-maintain-cflglqx.png") },
        { QObject::tr("磁分离管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-cflgltc.png") },
        { QObject::tr("底物A注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-dwazylzj.png") },
        { QObject::tr("底物B注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-dwbzylzj.png") },
        { QObject::tr("底物管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-dwgltc.png") },
        { QObject::tr("底物停用管路维护"), QIcon(":/Leonis/resource/image/icon-maintain-dwtyglwh.png") },
        { QObject::tr("第二磁分离堵针自检"), QIcon(":/Leonis/resource/image/icon-maintain-decfldzzj.png") },
        { QObject::tr("第二磁分离注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-decflzylzj.png") },
        { QObject::tr("第一磁分离堵针自检"), QIcon(":/Leonis/resource/image/icon-maintain-dycfldzzj.png") },
        { QObject::tr("第一磁分离注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-dycflzylzj.png") },
        { QObject::tr("添加管路填充管"), QIcon(":/Leonis/resource/image/icon-maintain-ghtcg.png") },
        { QObject::tr("故障自动修复"), QIcon(":/Leonis/resource/image/icon-maintain-gzzdxf.png") },
        { QObject::tr("进样系统复位"), QIcon(":/Leonis/resource/image/icon-maintain-jyxtfw.png") },
        { QObject::tr("清除管路填充管"), QIcon(":/Leonis/resource/image/icon-maintain-qctcg.png") },
        { QObject::tr("试剂混匀"), QIcon(":/Leonis/resource/image/icon-maintain-sjhy.png") },
        { QObject::tr("试剂在线加载复位"), QIcon(":/Leonis/resource/image/icon-maintain-sjzxjzfw.png") },
        { QObject::tr("试剂针洗针"), QIcon(":/Leonis/resource/image/icon-maintain-sjzxz.png") },
        { QObject::tr("洗针池灌注"), QIcon(":/Leonis/resource/image/icon-maintain-xzcgz.png") },
        { QObject::tr("样本针堵针自检"), QIcon(":/Leonis/resource/image/icon-maintain-ybzdzzj.png") },
        { QObject::tr("样本针特殊清洗"), QIcon(":/Leonis/resource/image/icon-maintain-ybztsqx.png") },
        { QObject::tr("样本针洗针"), QIcon(":/Leonis/resource/image/icon-maintain-ybzxz.png") },
        { QObject::tr("仪器关机"), QIcon(":/Leonis/resource/image/icon-maintain-yqgj.png") },
        { QObject::tr("在线配液排空"), QIcon(":/Leonis/resource/image/icon-maintain-zxpypk.png") },
        { QObject::tr("在线配液自检"), QIcon(":/Leonis/resource/image/icon-maintain-zxpyzj.png") },
        { QObject::tr("整机管路排空"), QIcon(":/Leonis/resource/image/icon-maintain-zjglpk.png") },
        { QObject::tr("整机管路清洗"), QIcon(":/Leonis/resource/image/icon-maintain-zjglqx.png") },
        { QObject::tr("整机管路填充"), QIcon(":/Leonis/resource/image/icon-maintain-zjgltc.png") },
        { QObject::tr("整机针浸泡"), QIcon(":/Leonis/resource/image/icon-maintain-zjzjp.png") },
        { QObject::tr("整机针特殊清洗"), QIcon(":/Leonis/resource/image/icon-maintain-zjztsqx.png") },
        { QObject::tr("第一试剂针注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-sjzzylzj.png") },
        { QObject::tr("第二试剂针注液量自检"), QIcon(":/Leonis/resource/image/icon-maintain-sjzzylzj.png") },
    };
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainStatusBtn::InitBeforeShow()
{
    // 初始化状态为正常
    SetStatus(BTN_MAINTAIN_STATUS_NORMAL);
    SetDevClassify(BTN_DEV_CLASSIFY_NONE);
    //ui->MaintainProgressBar->setMinimum(0);
    //ui->MaintainProgressBar->setMaximum(100);

    // 初始化维护图标映射
    InitMaintainIconMap();

    // 设置默认图表
    SetIcon(QIcon(":/Leonis/resource/image/icon-maintain-custom.png"));
}

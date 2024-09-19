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
/// @file     maintainwidget.cpp
/// @brief    应用->维护保养界面
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainwidget.h"
#include "ui_maintainwidget.h"
#include "maintainopdlg.h"
#include "devmaintaindetail.h"
#include "maintaindatamng.h"
#include "stopmaintaindlg.h"
#include "maintainprogressfmtcfgdlg.h"
#include "QMaintainSetPage.h"
#include "QElectrodeCleanSetDlg.h"
#include "QMaintainLog.h"
#include "QDetectModuleDebugDlg.h"
#include "QAutoMaintainCfgDlg.h"
#include "uidcsadapter/uidcsadapter.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "manager/UserInfoManager.h"
#include "thrift/DcsControlProxy.h"
#include "model/MaintainGroupModel.h"
#include "src/common/Mlog/mlog.h"
#include <QButtonGroup>

#define TAB_INDEX_MAINTAIN_GROUP                          (0)                 // 维护组Tab页索引
#define TAB_INDEX_MAINTAIN_ITEM                           (1)                 // 维护单项Tab页索引

MaintainWidget::MaintainWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_spDetectModuleDebugDlg(nullptr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui = new Ui::MaintainWidget();
    ui->setupUi(this);

    m_spMaintainLogDlg = nullptr;

    // 显示之前初始化
    InitBeforeShow();
}

MaintainWidget::~MaintainWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置Tab按钮为互斥
    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    pButtonGroup->addButton(ui->GrpMaintainBtn);
    pButtonGroup->addButton(ui->ItemMaintainBtn);
    pButtonGroup->setExclusive(true);

    // 栈窗口默认选中第一个
    if (ui->MaintainStkWgt->count() > 0)
    {
        ui->MaintainStkWgt->setCurrentIndex(0);
    }

	// 注册系统总览点击按钮“复位”处理函数（需要在构造中注册，不能在显示后初始化）
	REGISTER_HANDLER(MSG_ID_SYSTEM_OVERVIEW_CLICK_RESET, this, OnShowInstrumentReset);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString ItemMaintainStr = tr("单项维护");
    QString GrpMaintainStr = tr("组合维护");
    ui->ItemMaintainBtn->setText(ItemMaintainStr);
    ui->GrpMaintainBtn->setText(GrpMaintainStr);

    //按钮多语言显示
    QString tmpStr = ui->AutoMaintainCfgBtn->fontMetrics().elidedText(ui->AutoMaintainCfgBtn->text(), Qt::ElideRight, 220);
    ui->AutoMaintainCfgBtn->setToolTip(ui->AutoMaintainCfgBtn->text());
    ui->AutoMaintainCfgBtn->setText(tmpStr);
	// 电极清洗是否显示
	ui->ElectrodeCleanSetBtn->setVisible(gUiAdapterPtr()->WhetherContainTargetDevClassify(tf::AssayClassify::ASSAY_CLASSIFY_ISE));


    // 更新维护组按钮
    UpdateMaintainGrpBtn();

    // 更新设备维护状态
    UpdateDevMaintainStatus();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 维护组tab按钮点击
    connect(ui->GrpMaintainBtn, SIGNAL(clicked()), this, SLOT(OnGrpMaintainBtnClicked()));

    // 维护单项tab按钮点击
    connect(ui->ItemMaintainBtn, SIGNAL(clicked()), this, SLOT(OnItemMaintainBtnClicked()));

    // 维护组状态按钮被点击
    connect(ui->MtGrpSubWgt, SIGNAL(SigMtStatusBtnClicked(QString)), this, SLOT(OnMaintainStatusBtnClicked(QString)));

    // 维护项状态按钮被点击
    connect(ui->MtItemSubWgt, SIGNAL(SigMtStatusBtnClicked(QString)), this, SLOT(OnMaintainStatusBtnClicked(QString)));

    // 组合设置按钮被点击
    connect(ui->ProfileCfgBtn, SIGNAL(clicked()), this, SLOT(OnProfileCfgBtnClicked()));

    // 维护日志按钮被点击
    connect(ui->MaintainLogBtn, SIGNAL(clicked()), this, SLOT(OnMaintainLogBtnClicked()));

    // 自动维护按钮被点击
    connect(ui->AutoMaintainCfgBtn, SIGNAL(clicked()), this, SLOT(OnAutoMaintainCfgBtnClicked()));

    // 停止维护按钮被点击
    connect(ui->StopMaintainBtn, SIGNAL(clicked()), this, SLOT(OnStopMaintainBtnClicked()));

    // 显示信息设置按钮被点击
    connect(ui->DisplayInfoCfg, SIGNAL(clicked()), this, SLOT(OnDisplayInfoCfgClicked()));

	// 电极清洗设置按钮被点击
	connect(ui->ElectrodeCleanSetBtn, SIGNAL(clicked()), this, SLOT(OnElectrodeCleanSetBtnClicked()));

    // 发光剂流程按钮被点击
    connect(ui->DetectDebugBtn, SIGNAL(clicked()), this, SLOT(OnDetectDebugBtnClicked()));
	ui->DetectDebugBtn->hide();// 0021412: [应用] 维护保养界面“发光剂流程”按钮多余显示 added by zhang.changjiang 20240603

    // 设置状态控件被点击
    connect(ui->DevMtStatusGrpWgt, SIGNAL(SigDevWgtClicked(QString)), this, SLOT(OnDevWgtClicked(QString)));

    // 监听维护组组合设置更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_CFG_UPDATE, this, UpdateMaintainGrpBtn);

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

    // 监听维护组进度
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE, this, OnMaintainGrpProgressUpdate);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
/// @brief
///     根据设备类型获取设备类别
///
/// @param[in]  strMaintainGrpName  维护组名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月19日，新建函数
///
QSet<tf::AssayClassify::type> MaintainWidget::GetDevClassify(const QString& strMaintainGrpName)
{
    MaintainDataMng* mtIns = MaintainDataMng::GetInstance();

    // 获取对应的设备类型
    QSet<tf::DeviceType::type> setDeviceType = mtIns->GetDeviceTypesForMaintainGrp(strMaintainGrpName);

    // 构造返回值
    QSet<tf::AssayClassify::type> setRet;
    // 遍历设备类型集合
    for (const auto& enDevType : setDeviceType)
    {
        // 生化
        if ((enDevType == tf::DeviceType::DEVICE_TYPE_C1000) ||
            (enDevType == tf::DeviceType::DEVICE_TYPE_C200) ||
            (enDevType == tf::DeviceType::DEVICE_TYPE_ISE1005))
        {
            setRet.insert(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
            continue;
        }

        // 免疫
        if (enDevType == tf::DeviceType::DEVICE_TYPE_I6000)
        {
            setRet.insert(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
            continue;
        }

        // 其他
        setRet.insert(tf::AssayClassify::ASSAY_CLASSIFY_OTHER);
    }

    // 返回结果
    return setRet;
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void MaintainWidget::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化, 显示后初始化
    if (!m_bInit)
    {
        // 显示之后初始化
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     维护组tab按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainWidget::OnGrpMaintainBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 按钮选中
    ui->GrpMaintainBtn->setChecked(true);

    // 切换页面
    ui->MaintainStkWgt->setCurrentIndex(TAB_INDEX_MAINTAIN_GROUP);
}

///
/// @brief
///     维护单项tab按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月16日，新建函数
///
void MaintainWidget::OnItemMaintainBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 按钮选中
    ui->ItemMaintainBtn->setChecked(true);

    // 切换页面
    ui->MaintainStkWgt->setCurrentIndex(TAB_INDEX_MAINTAIN_ITEM);
}

///
/// @brief
///     维护状态按钮被点击
///
/// @param[in]  strMaintainName  维护名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainWidget::OnMaintainStatusBtnClicked(QString strMaintainName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出维护操作对话框
    std::shared_ptr<MaintainOpDlg> spMaintainOpDlg(new MaintainOpDlg(strMaintainName, this));
    spMaintainOpDlg->exec();
}

///
/// @brief
///     设备控件被点击
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月14日，新建函数
///
void MaintainWidget::OnDevWgtClicked(QString strDevSn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出设备维护详情对话框
    std::shared_ptr<DevMaintainDetail> spDevMaintainDetailDlg(new DevMaintainDetail(strDevSn, this));
    spDevMaintainDetailDlg->exec();
}

///
/// @brief
///     组合设置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月21日，新建函数
///
void MaintainWidget::OnProfileCfgBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出组合设置对话框
    std::shared_ptr<QMaintainSetPage> spMaintainSetDlg(new QMaintainSetPage(this));
    spMaintainSetDlg->exec();
}

///
/// @brief
///     维护日志按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月21日，新建函数
///
void MaintainWidget::OnMaintainLogBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出维护日志对话框
    std::shared_ptr<QMaintainLog> spMaintainLogDlg(new QMaintainLog(this));
    spMaintainLogDlg->exec();
}

///
/// @brief
///     自动维护设置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月21日，新建函数
///
void MaintainWidget::OnAutoMaintainCfgBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出自动维护设置对话框
    if (m_spMaintainLogDlg == nullptr)
    {
        m_spMaintainLogDlg = std::shared_ptr<QAutoMaintainCfgDlg>(new QAutoMaintainCfgDlg(this));
    }
    m_spMaintainLogDlg->exec();
}

///
/// @brief
///     停止维护按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void MaintainWidget::OnStopMaintainBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出停止维护对话框
    std::shared_ptr<StopMaintainDlg> spStopMaintainDlg(new StopMaintainDlg(this));
    spStopMaintainDlg->exec();
}

///
/// @brief
///     显示信息设置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void MaintainWidget::OnDisplayInfoCfgClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出显示信息设置对话框
    std::shared_ptr<MaintainProgressFmtCfgDlg> spMaintainProgressFmtCfgDlg(new MaintainProgressFmtCfgDlg(this));
    spMaintainProgressFmtCfgDlg->exec();
}

///
/// @brief 电极清洗设置
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
///
void MaintainWidget::OnElectrodeCleanSetBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 弹出电极清洗设置对话框
	std::shared_ptr<QElectrodeCleanSetDlg> spQElectrodeCleanSetDlg(new QElectrodeCleanSetDlg(this));
	spQElectrodeCleanSetDlg->exec();
}

void MaintainWidget::OnDetectDebugBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出发光剂流程对话框
    if (m_spDetectModuleDebugDlg == nullptr)
    {
        m_spDetectModuleDebugDlg.reset(new QDetectModuleDebugDlg(this));
    }
    // 使用非模态显示
    m_spDetectModuleDebugDlg->show();
}

///
/// @brief
///     更新维护组按钮
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月23日，新建函数
///
void MaintainWidget::UpdateMaintainGrpBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取所有维护组
    QList<tf::MaintainGroup> grpList;
    if (!MaintainDataMng::GetInstance()->GetAllMaintainGrp(grpList))
    {
        ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetAllMaintainGrp failed.", __FUNCTION__);
        return;
    }

    // 设置维护组按钮组
    QStringList strMaintainGrpNameLst;
    QStringList strMaintainItemNameLst;
    for (const auto& stuMaintainGrp : grpList)
    {
        // 判断维护组类型
        if (stuMaintainGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
        {
            strMaintainGrpNameLst.push_back(QString::fromStdString(stuMaintainGrp.groupName));
            continue;
        }

        // 维护单项不允许当前用户执行则跳过
        if (!MaintainDataMng::GetInstance()->WhetherItemCanExeByCurUser(QString::fromStdString(stuMaintainGrp.groupName)))
        {
            continue;
        }

        // 添加当前用户可执行的维护单项
        strMaintainItemNameLst.push_back(QString::fromStdString(stuMaintainGrp.groupName));
    }
    ui->MtGrpSubWgt->SetMaintainNameList(strMaintainGrpNameLst);
    ui->MtItemSubWgt->SetMaintainNameList(strMaintainItemNameLst, gUiAdapterPtr()->WhetherOnlyImDev());

    // 设置维护进度和状态
    for (const tf::MaintainGroup& stuMaintainGrp : grpList)
    {
        // 获取维护组名
        QString strMaintainGrpName = QString::fromStdString(stuMaintainGrp.groupName);

        // 判断维护组类型
        MaintainStatusBtnGrp* pMtBtnGrp = Q_NULLPTR;
        if (stuMaintainGrp.groupType == tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
        {
            pMtBtnGrp = ui->MtItemSubWgt;
            pMtBtnGrp->SetMaintainType(strMaintainGrpName, MaintainStatusBtn::BTN_MAINTAIN_TYPE_MAINTAIN_ITEM);
        }
        else
        {
            pMtBtnGrp = ui->MtGrpSubWgt;
            pMtBtnGrp->SetMaintainType(strMaintainGrpName, MaintainStatusBtn::BTN_MAINTAIN_TYPE_MAINTAIN_GROUP);
        }

        // 获取维护组类别
        QSet<tf::AssayClassify::type> setDevClassify = GetDevClassify(strMaintainGrpName);
        SOFTWARE_TYPE type = CommonInformationManager::GetInstance()->GetSoftWareType();
        if (type != CHEMISTRY_AND_IMMUNE)
        {
            pMtBtnGrp->SetDevClassify(strMaintainGrpName, MaintainStatusBtn::BTN_DEV_CLASSIFY_NONE);
        }
        else if (setDevClassify.find(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) != setDevClassify.end() &&
                 setDevClassify.find(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE) != setDevClassify.end())
        {
            // 既支持生化。也支持免疫，不显示标签
            pMtBtnGrp->SetDevClassify(strMaintainGrpName, MaintainStatusBtn::BTN_DEV_CLASSIFY_NONE);
        }
        else if ((*setDevClassify.begin()) == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
        {
            pMtBtnGrp->SetDevClassify(strMaintainGrpName, MaintainStatusBtn::BTN_DEV_CLASSIFY_CH);
        }
        else if ((*setDevClassify.begin()) == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            pMtBtnGrp->SetDevClassify(strMaintainGrpName, MaintainStatusBtn::BTN_DEV_CLASSIFY_IM);
        }
        else
        {
            pMtBtnGrp->SetDevClassify(strMaintainGrpName, MaintainStatusBtn::BTN_DEV_CLASSIFY_NONE);
        }

        // 获取所有维护详情
        QList<tf::LatestMaintainInfo> detailLst;
        if (!MaintainDataMng::GetInstance()->GetAllMaintainDetail(detailLst))
        {
            ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetAllMaintainDetail failed.", __FUNCTION__);
            return;
        }

        // 判断状态
        bool bFailed = false;
        bool bExcute = false;
        for (const auto& stuDevDetail : detailLst)
        {
            // 维护组不匹配则跳过
            if (stuDevDetail.groupId != stuMaintainGrp.id)
            {
                continue;
            }

            // 遍历维护项执行状态
            for (const auto& stuItemDetail : stuDevDetail.resultDetail)
            {
                if (stuItemDetail.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
                {
                    bExcute = true;
                }
                else if (stuItemDetail.result == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
                {
                    bFailed = true;
                }
            }
        }

        // 判断状态
        pMtBtnGrp->SetMaintainProgressVisible(strMaintainGrpName, false);
        pMtBtnGrp->SetMaintainStatus(strMaintainGrpName, bFailed ? MaintainStatusBtn::BTN_MAINTAIN_STATUS_FAILED : MaintainStatusBtn::BTN_MAINTAIN_STATUS_NORMAL);
    }
}

///
/// @brief
///     更新设备维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void MaintainWidget::UpdateDevMaintainStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}


///
/// @brief
///     维护组阶段更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void MaintainWidget::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(strDevSn);
// 
//     // 如果更新阶段为未知，则弹框提示当前仪器状态不能执行维护
//     if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
//     {
//         // 获取维护名
//         tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpById(lGrpId);
// 
//         // 弹框提示
//         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("仪器当前状态不能执行【%1】！").arg(QString::fromStdString(stuMtGrp.groupName))));
//         pTipDlg->exec();
//         return;
//     }

    // 更新设备维护状态
    UpdateDevMaintainStatus();
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  lmi     正在执行的维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void MaintainWidget::OnMaintainGrpProgressUpdate(tf::LatestMaintainInfo lmi)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	Q_UNUSED(lmi);
	// 更新设备维护进度（因为一个设备只能执行一个维护组）
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年7月6日，新建函数
///
void MaintainWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 发光剂流程(最高权限+免疫设备)
	if (userPms->IsPermisson(PMS_ENGDEBUG_LIGHTREAGENT_PROC) && 
		gUiAdapterPtr()->WhetherContainTargetDevClassify(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
	{
		// 0021412: [应用] 维护保养界面“发光剂流程”按钮多余显示 added by zhang.changjiang 20240603
		//ui->DetectDebugBtn->show();
	}
	else
	{
		ui->DetectDebugBtn->hide();
	}

    // 自动维护
    userPms->IsPermissionShow(PSM_IM_MANT_AUTOMAINTSET) ? ui->AutoMaintainCfgBtn->show() : ui->AutoMaintainCfgBtn->hide();

    // 组合维护
    userPms->IsPermissionShow(PSM_IM_MANT_GROUP) ? ui->ProfileCfgBtn->show() : ui->ProfileCfgBtn->hide();

    // 显示信息设置
    userPms->IsPermissionShow(PSM_IM_MANT_SHOWINFOSET) ? ui->DisplayInfoCfg->show() : ui->DisplayInfoCfg->hide();

    // 权限更新后重新过滤维护项（解决部分页面在权限变更后没有触发过滤而显示超出权限的维护项的问题）
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent != nullptr)
    {
        ::tf::MaintainGroupQueryResp retGroup;
        ::tf::MaintainGroupQueryCond miqcGroup;
        if (spCilent->QueryMaintainGroup(retGroup, miqcGroup))
        {
            MaintainDataMng::GetInstance()->FilterGroups(retGroup.lstMaintainGroups);
            MaintainGroupModel::Instance().SetData(retGroup.lstMaintainGroups);
        }
    }

    // 重新加载维护项数据
    MaintainDataMng::GetInstance()->LoadData();
    UpdateMaintainGrpBtn();
}

void MaintainWidget::OnShowInstrumentReset()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 获取单项维护-仪器复位名字
	static QString insResetName  = ConvertTfEnumToQString(::tf::MaintainItemType::MAINTAIN_ITEM_RESET);

	if (!insResetName.isEmpty())
	{
		// 显示“仪器复位”弹窗
		OnMaintainStatusBtnClicked(insResetName);
	}
}

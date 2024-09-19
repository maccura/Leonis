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
/// @file     rgntplatewidget.cpp
/// @brief    试剂盘界面
///
/// @author   4170/TangChuXian
/// @date     2022年8月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imrgntplatewidget.h"
#include "immcreagentplate.h"
#include "imrgntscandlg.h"
#include "imaddreactcupdlg.h"
#include "immctubeslot.h"
#include "reagent/rgntnoticedatamng.h"
#include <QFontMetrics>

#include "ui_imrgntplatewidget.h"
#include "ui_imrgntscandlg.h"
#include "ui_imaddreactcupdlg.h"

#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/ReagentCommon.h"
#include "shared/datetimefmttool.h"

#include "manager/DictionaryQueryManager.h"

#include "src/leonis/thrift/im/ImLogicControlProxy.h"
#include "src/leonis/thrift/im/i6000/I6000LogicControlProxy.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/im/ImConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "thrift/DcsControlProxy.h"
#include "reagent/ch/QReagentScanDialog.h"
#include "reagent/im/imbeadmixdlg.h"
#include "reagent/im/imloaderresetdlg.h"
#include "uidcsadapter/uidcsadapter.h"


// 储液瓶类型
#define        BOTTLE_TYPE_INVALID                          (0)             // 无效
#define        BOTTLE_TYPE_A1_BACKUP                        (1)             // 底物液A1备用
#define        BOTTLE_TYPE_A1_USING                         (2)             // 底物液A1在用
#define        BOTTLE_TYPE_A1_WARN                          (3)             // 底物液A1提醒
#define        BOTTLE_TYPE_A1_ERROR                         (4)             // 底物液A1警告
#define        BOTTLE_TYPE_A1_EMPTY                         (5)             // 底物液A1空位置
#define        BOTTLE_TYPE_B1_BACKUP                        (6)             // 底物液B1备用
#define        BOTTLE_TYPE_B1_USING                         (7)             // 底物液B1在用
#define        BOTTLE_TYPE_B1_WARN                          (8)             // 底物液B1提醒
#define        BOTTLE_TYPE_B1_ERROR                         (9)             // 底物液B1警告
#define        BOTTLE_TYPE_B1_EMPTY                         (10)            // 底物液B1空位置
#define        BOTTLE_TYPE_CLEAN_BACKUP                     (11)            // 清洗缓冲液备用
#define        BOTTLE_TYPE_CLEAN_USING                      (12)            // 清洗缓冲液在用
#define        BOTTLE_TYPE_CLEAN_WARN                       (13)            // 液清洗缓冲液提醒
#define        BOTTLE_TYPE_CLEAN_ERROR                      (14)            // 液清洗缓冲液警告
#define        BOTTLE_TYPE_CLEAN_EMPTY                      (15)            // 液清洗缓冲液空位置
#define        BOTTLE_TYPE_REACTION_USING                   (16)            // 反应杯在用
#define        BOTTLE_TYPE_REACTION_WARN                    (17)            // 反应杯提醒
#define        BOTTLE_TYPE_REACTION_ERROR                   (18)            // 反应杯警告
#define        BOTTLE_TYPE_WASTE_S_USING                    (19)            // 废料桶在用
#define        BOTTLE_TYPE_WASTE_S_WARN                     (20)            // 废料桶提醒
#define        BOTTLE_TYPE_WASTE_S_ERROR                    (21)            // 废料桶警告
#define        BOTTLE_TYPE_WASTE_S_EMPTY                    (22)            // 废料桶空
#define        BOTTLE_TYPE_WASTE_L_USING                    (23)            // 废液桶在用
#define        BOTTLE_TYPE_WASTE_L_WARN                     (24)            // 废液桶提醒
#define        BOTTLE_TYPE_WASTE_L_ERROR                    (25)            // 废液桶警告

// 试剂盘
#define        SLOT_CNT_OF_RGNT_PLATE                       (30)            // 试剂盘槽位数

Q_DECLARE_METATYPE(im::tf::ReagentInfoTable)
Q_DECLARE_METATYPE(im::tf::DiluentInfoTable)
Q_DECLARE_METATYPE(im::tf::SuppliesInfoTable)

ImRgntPlateWidget::ImRgntPlateWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_scanDialog(Q_NULLPTR)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::ImRgntPlateWidget();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

ImRgntPlateWidget::~ImRgntPlateWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     测试
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月22日，新建函数
///
void ImRgntPlateWidget::Test()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    //QGridLayout * playOut = new QGridLayout(ui->RgntPlateWgt);
    m_pRgntPlate = new ImMcReagentPlate(30, 0, ui->RgntPlateWgt);
    ImReagentInfo stuInfo;
    for (int i = 1; i <= m_pRgntPlate->GetTubeSlotCount(); i++)
    {
        stuInfo.enState = ImReagentState(i % 5);
        stuInfo.bMask = (i % 3 == 1 && stuInfo.enState != EMPTY);
        if (i < 6 && stuInfo.enState != EMPTY)
        {
            stuInfo.iBackupNo = i;
        }
        else
        {
            stuInfo.iBackupNo = 0;
        }

        if (i > 25)
        {
            stuInfo.bDillute = true;
        }
        m_pRgntPlate->LoadReagent(i, stuInfo);
    }
    m_pRgntPlate->setGeometry(6, 6, 701, 701);
    //     playOut->addWidget(pRgnt);
    //     playOut->setMargin(5);
    //     w.show();
    // 
    //     测试储液瓶
    ui->Grp1A1Bottle->SetBottleType(BOTTLE_TYPE_A1_USING);
    ui->Grp1A1Bottle->SetLiquidRemainRatio(0.75);
    ui->Grp1A1Bottle->SetRemainVolText("450ml");
    ui->Grp2A2Bottle->SetBottleType(BOTTLE_TYPE_A1_WARN);
    ui->Grp2A2Bottle->SetLiquidRemainRatio(0.35);
    ui->Grp1B1Bottle->SetBottleType(BOTTLE_TYPE_B1_USING);
    ui->Grp1B1Bottle->SetLiquidRemainRatio(0.75);
    ui->Grp2B2Bottle->SetBottleType(BOTTLE_TYPE_B1_WARN);
    ui->Grp2B2Bottle->SetLiquidRemainRatio(0.35);

    ui->CleanLiquid1Bottle->SetBottleType(BOTTLE_TYPE_CLEAN_BACKUP);
    ui->CleanLiquid1Bottle->SetLiquidRemainRatio(1.0);
    ui->CleanLiquid2Bottle->SetBottleType(BOTTLE_TYPE_CLEAN_USING);
    ui->CleanLiquid2Bottle->SetLiquidRemainRatio(0.35);

    ui->ReactionBottle->SetBottleType(BOTTLE_TYPE_REACTION_WARN);
    ui->ReactionBottle->SetLiquidRemainRatio(0.5);
    ui->WasteSolidBottle->SetBottleType(BOTTLE_TYPE_WASTE_S_USING);
    ui->WasteSolidBottle->SetLiquidRemainRatio(0.1);
    ui->WasteLiquidBottle->SetBottleType(BOTTLE_TYPE_WASTE_L_ERROR);
    ui->WasteLiquidBottle->SetLiquidRemainRatio(0.85);

    ui->PriorUseBtn->setIcon(QIcon(":/Leonis/resource/image/im/prior_use_normal_icon.png"));
    ui->RgntMaskBtn->setIcon(QIcon(":/Leonis/resource/image/rgnt_mask_icon.png"));

    //     ui->PriorUseBtn->setStyleSheet("QPushButton{background-image:url(:/Leonis/resource/image/im/prior_use_normal_icon.png) left center no-repeat;}"
    //                                    "QPushButton:hover{background-image:url(:/Leonis/resource/image/im/prior_use_hover_icon.png) left center no-repeat;}");
}

///
/// @brief
///     事件过滤器（主要用于监听场景事件）
///
/// @param[in] obj    事件的目标对象
/// @param[in] event  事件对象
///
/// @return true表示事件已处理，false表示事件未处理
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
bool ImRgntPlateWidget::eventFilter(QObject *obj, QEvent *event)
{
    QEvent::Type currEvent = event->type();

    if (obj == ui->RgntPlateWgt && currEvent == QEvent::Paint)
    {
        QPainter paint(ui->RgntPlateWgt);
        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.setPen(Qt::NoPen);
        paint.setBrush(Qt::white);
        paint.drawEllipse(ui->RgntPlateWgt->rect());
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月20日，新建函数
///
void ImRgntPlateWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化余量提醒管理器
    RgntNoticeDataMng::GetInstance();

    // 安装事件过滤器(画试剂盘圆环)
    ui->RgntPlateWgt->installEventFilter(this);

    // 初始化试剂盘
    m_pRgntPlate = new ImMcReagentPlate(SLOT_CNT_OF_RGNT_PLATE, 0, ui->RgntPlateWgt);
    m_pRgntPlate->ReseReagent();
    m_pRgntPlate->setGeometry(6, 6, 701, 701);

    // 注册储液瓶类型
    RegisterBottleType();

    // 测试
    //Test();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月20日，新建函数
///
void ImRgntPlateWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月20日，新建函数
///
void ImRgntPlateWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新试剂盘
    UpdateRgntPlate();

    // 更新耗材信息
    UpdateSplInfo();

    // 更新耗材停用
    UpdateSplEnabled();

    // 更新选中试剂详情
    UpdateSelRgntDetail();

    // 更新按钮状态
    UpdateBtnStatus();
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月20日，新建函数
///
void ImRgntPlateWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 试剂盘选中项改变
    connect(m_pRgntPlate, SIGNAL(selectedItemChange(int)), this, SLOT(OnRgntPlateSelIndexChanged(int)));

    // 试剂屏蔽按钮被点击
    connect(ui->RgntMaskBtn, SIGNAL(clicked()), this, SLOT(OnRgntMaskBtnClicked()));

    // 优先使用按钮被点击
    connect(ui->PriorUseBtn, SIGNAL(clicked()), this, SLOT(OnPriorUseBtnClicked()));

    // 试剂装载按钮被点击
    connect(ui->LoadRgntBtn, SIGNAL(clicked()), this, SLOT(OnLoadRgntBtnClicked()));

    // 试剂卸载按钮被点击
    connect(ui->UnloadRgntBtn, SIGNAL(clicked()), this, SLOT(OnUnloadRgntBtnClicked()));

    // 磁珠混匀按钮被点击
    connect(ui->MixBtn, SIGNAL(clicked()), this, SLOT(OnMixBtnClicked()));

    // 装载装置复位按钮被点击
    connect(ui->LoadPartResetBtn, SIGNAL(clicked()), this, SLOT(OnLoadPartResetBtnClicked()));

    // 试剂扫描按钮被点击
    connect(ui->RgntScanBtn, SIGNAL(clicked()), this, SLOT(OnRgntScanBtnClicked()));

    // 添加反应杯按钮被点击
    connect(ui->AddReactBtn, SIGNAL(clicked()), this, SLOT(OnAddReactBtnClicked()));

    // 耗材选择改变
    connect(ui->BaseLiquidGrp1CheckFrame, SIGNAL(SigMousePressed()), this, SLOT(OnSplChecked()));
    connect(ui->BaseLiquidGrp2CheckFrame, SIGNAL(SigMousePressed()), this, SLOT(OnSplChecked()));
    connect(ui->CleanLiquid1CheckFrame, SIGNAL(SigMousePressed()), this, SLOT(OnSplChecked()));
    connect(ui->CleanLiquid2CheckFrame, SIGNAL(SigMousePressed()), this, SLOT(OnSplChecked()));

    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateReagentInfo);

    // 监听试剂扫描开始消息
    //REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnClearReagentInfo);

    // 监听稀释信息更新
    REGISTER_HANDLER(MSG_ID_IM_DILUENT_INFO_UPDATE, this, UpdateImDltChanged);

    // 监听耗材信息更新
    REGISTER_HANDLER(MSG_ID_IM_SUPPLY_INFO_UPDATE, this, UpdateImSplChanged);

    // 废液桶是否显示设置更新
    REGISTER_HANDLER(MSG_ID_BUCKET_SET_UPDATE, this, UpdateBucketVisible);

    // 当前日期改变
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, UpdateRgntPlate);

    // 当前日期改变
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, UpdateSplInfo);

    // 当前日期改变
    REGISTER_HANDLER(MSG_ID_NEW_DAY_ARRIVED, this, UpdateSelRgntDetail);

    // 指定位置试剂扫描
    REGISTER_HANDLER(MSG_ID_REAGENT_SCAN_POS_START, this, OnRgntScanPosStart);

    // 试剂报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_REAGENT, this, UpdateRgntPlate);

    // 耗材报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, UpdateSplInfo);

    // 耗材报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, UpdateRgntPlate);

    // 试剂加卸载状态更新
    REGISTER_HANDLER(MSG_ID_REAGENT_LOAD_STATUS_UPDATE, this, OnRgntLoadStatusChanged);

    // 监听耗材管理设置更新
    REGISTER_HANDLER(MSG_ID_CONSUMABLES_MANAGE_UPDATE, this, OnSplMngCfgChanged);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);

    // 监听设备温度异常状态改变
    //REGISTER_HANDLER(MSG_ID_TEMPERATURE_ERR, this, OnDevTemperatureErrChanged);

    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);

    // 申请更换试剂结果消息
    REGISTER_HANDLER(MSG_ID_MANUAL_HANDL_REAG, this, OnManualHandleReagResult);

    // 监听设备状态改变
    REGISTER_HANDLER(MSG_ID_DCS_REPORT_STATUS_CHANGED, this, UpdateDeviceStatus);
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月20日，新建函数
///
void ImRgntPlateWidget::showEvent(QShowEvent* event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        m_bInit = true;
    }
    else
    {
        // 更新试剂盘
        UpdateRgntPlate();

        // 更新耗材信息
        UpdateSplInfo();

        // 更新选中试剂详情
        UpdateSelRgntDetail();

        // 更新按钮状态
        UpdateBtnStatus();
    }
}

///
/// @brief
///     清空选中试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void ImRgntPlateWidget::ClearSelRgntInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->AssayNameValLab->clear();
    ui->PosValLab->clear();
    ui->RgntLotValLab->clear();
    ui->ExpireDateValLab->clear();
    ui->RemainTestCntValLab->clear();
    ui->StateValLab->clear();
}

///
/// @brief
///     注册瓶子类型
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void ImRgntPlateWidget::RegisterBottleType()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 注册储液瓶类型
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_A1_BACKUP, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/A1_backup_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/A1_backup_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_A1_USING, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/A1_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/A1_using_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_A1_WARN, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/A1_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/A1_warn_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_A1_ERROR, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/A1_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/A1_error_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_A1_EMPTY, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/A1_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/A1_error_liquid.png"), false));

    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_B1_BACKUP, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/B1_backup_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/B1_backup_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_B1_USING, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/B1_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/B1_using_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_B1_WARN, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/B1_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/B1_warn_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_B1_ERROR, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/B1_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/B1_error_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_B1_EMPTY, QPoint(5, 17),
        QPixmap(":/Leonis/resource/image/im/B1_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/B1_error_liquid.png"), false));

    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_CLEAN_BACKUP, QPoint(22, 17),
        QPixmap(":/Leonis/resource/image/im/clean_backup_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/clean_backup_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_CLEAN_USING, QPoint(22, 17),
        QPixmap(":/Leonis/resource/image/im/clean_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/clean_using_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_CLEAN_WARN, QPoint(22, 17),
        QPixmap(":/Leonis/resource/image/im/clean_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/clean_warn_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_CLEAN_ERROR, QPoint(22, 17),
        QPixmap(":/Leonis/resource/image/im/clean_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/clean_error_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_CLEAN_EMPTY, QPoint(22, 17),
        QPixmap(":/Leonis/resource/image/im/clean_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/clean_error_liquid.png"), false));

    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_REACTION_USING, QPoint(5, 20),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_using_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_REACTION_WARN, QPoint(5, 20),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_warn_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_REACTION_ERROR, QPoint(5, 20),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/reaction_cup_error_liquid.png")));

    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_S_USING, QPoint(10, 13),
        QPixmap(":/Leonis/resource/image/im/waste_solid_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_solid_using_solid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_S_WARN, QPoint(10, 13),
        QPixmap(":/Leonis/resource/image/im/waste_solid_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_solid_warn_solid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_S_ERROR, QPoint(10, 13),
        QPixmap(":/Leonis/resource/image/im/waste_solid_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_solid_error_solid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_S_EMPTY, QPoint(10, 13),
        QPixmap(":/Leonis/resource/image/im/waste_solid_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_solid_error_solid.png"), false));

    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_L_USING, QPoint(13, 16),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_using_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_using_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_L_WARN, QPoint(13, 16),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_warn_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_warn_liquid.png")));
    ImLiquidBottleWidget::RegisterBottleType(ImLiquidBottleWidget::BottleTypeInfo(BOTTLE_TYPE_WASTE_L_ERROR, QPoint(13, 16),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_error_bottle.png"),
        QPixmap(":/Leonis/resource/image/im/waste_liquid_error_liquid.png")));

    // 构造储液瓶类型映射
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_A, SPL_STATUS_USING), BOTTLE_TYPE_A1_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_A, SPL_STATUS_BACKUP), BOTTLE_TYPE_A1_BACKUP);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_A, SPL_STATUS_REMAIND), BOTTLE_TYPE_A1_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_A, SPL_STATUS_WARN), BOTTLE_TYPE_A1_ERROR);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_A, SPL_STATUS_EMPTY), BOTTLE_TYPE_A1_EMPTY);

    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_B, SPL_STATUS_USING), BOTTLE_TYPE_B1_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_B, SPL_STATUS_BACKUP), BOTTLE_TYPE_B1_BACKUP);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_B, SPL_STATUS_REMAIND), BOTTLE_TYPE_B1_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_B, SPL_STATUS_WARN), BOTTLE_TYPE_B1_ERROR);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_BASE_LIQUID_B, SPL_STATUS_EMPTY), BOTTLE_TYPE_B1_EMPTY);

    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASH_BUFFER, SPL_STATUS_USING), BOTTLE_TYPE_CLEAN_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASH_BUFFER, SPL_STATUS_BACKUP), BOTTLE_TYPE_CLEAN_BACKUP);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASH_BUFFER, SPL_STATUS_REMAIND), BOTTLE_TYPE_CLEAN_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASH_BUFFER, SPL_STATUS_WARN), BOTTLE_TYPE_CLEAN_ERROR);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASH_BUFFER, SPL_STATUS_EMPTY), BOTTLE_TYPE_CLEAN_EMPTY);

    m_mapBottleType.insert(qMakePair(SPL_TYPE_REACTION_CUP, SPL_STATUS_USING), BOTTLE_TYPE_REACTION_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_REACTION_CUP, SPL_STATUS_REMAIND), BOTTLE_TYPE_REACTION_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_REACTION_CUP, SPL_STATUS_WARN), BOTTLE_TYPE_REACTION_ERROR);

    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_SOLID, SPL_STATUS_USING), BOTTLE_TYPE_WASTE_S_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_SOLID, SPL_STATUS_REMAIND), BOTTLE_TYPE_WASTE_S_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_SOLID, SPL_STATUS_WARN), BOTTLE_TYPE_WASTE_S_ERROR);

    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_LIQUID, SPL_STATUS_USING), BOTTLE_TYPE_WASTE_L_USING);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_LIQUID, SPL_STATUS_REMAIND), BOTTLE_TYPE_WASTE_L_WARN);
    m_mapBottleType.insert(qMakePair(SPL_TYPE_WASTE_LIQUID, SPL_STATUS_WARN), BOTTLE_TYPE_WASTE_L_ERROR);
}

///
/// @brief
///     获取储液瓶类型
///
/// @param[in]  enType    耗材类型
/// @param[in]  enStatus  耗材状态
///
/// @return 储液瓶类型
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月30日，新建函数
///
int ImRgntPlateWidget::GetBottleType(SuppliesType enType, SuppliesStatus enStatus)
{
    // ULOG(LOG_INFO, "%s()", __FUNCTION__); // too frequency.

    auto it = m_mapBottleType.find(qMakePair(enType, enStatus));
    if (it == m_mapBottleType.end())
    {
        return BOTTLE_TYPE_INVALID;
    }

    return it.value();
}

///
/// @brief
///     获取选中耗材的耗材信息
///
/// @return 储液瓶类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
im::tf::SuppliesInfoTable ImRgntPlateWidget::GetSelSplInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历选择器
    for (auto it = m_mapSplChecker.begin(); it != m_mapSplChecker.end(); it++)
    {
        if (it.key()->IsChecked())
        {
            return it.value();
        }
    }

    return im::tf::SuppliesInfoTable();
}

///
/// @brief
///     重置耗材信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月30日，新建函数
///
void ImRgntPlateWidget::ResetSplInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置耗材信息
    ui->Grp1A1Bottle->SetBottleType(BOTTLE_TYPE_A1_EMPTY);
    ui->Grp1A1Bottle->SetLiquidRemainRatio(0.0);
    ui->Grp1A1Bottle->SetRemainVolText("");
    ui->Grp2A2Bottle->SetBottleType(BOTTLE_TYPE_A1_EMPTY);
    ui->Grp2A2Bottle->SetLiquidRemainRatio(0.0);
    ui->Grp2A2Bottle->SetRemainVolText("");
    ui->Grp1B1Bottle->SetBottleType(BOTTLE_TYPE_B1_EMPTY);
    ui->Grp1B1Bottle->SetLiquidRemainRatio(0.0);
    ui->Grp1B1Bottle->SetRemainVolText("");
    ui->Grp2B2Bottle->SetBottleType(BOTTLE_TYPE_B1_EMPTY);
    ui->Grp2B2Bottle->SetLiquidRemainRatio(0.0);
    ui->Grp2B2Bottle->SetRemainVolText("");

    ui->CleanLiquid1Bottle->SetBottleType(BOTTLE_TYPE_CLEAN_EMPTY);
    ui->CleanLiquid1Bottle->SetLiquidRemainRatio(0.0);
    ui->CleanLiquid1Bottle->SetRemainVolText("");
    ui->CleanLiquid2Bottle->SetBottleType(BOTTLE_TYPE_CLEAN_EMPTY);
    ui->CleanLiquid2Bottle->SetLiquidRemainRatio(0.0);
    ui->CleanLiquid2Bottle->SetRemainVolText("");

    ui->ReactionBottle->SetBottleType(BOTTLE_TYPE_REACTION_ERROR);
    ui->ReactionBottle->SetLiquidRemainRatio(0.0);
    ui->ReactionBottle->SetRemainVolText("");
    ui->WasteSolidBottle->SetBottleType(BOTTLE_TYPE_WASTE_S_ERROR);
    ui->WasteSolidBottle->SetLiquidRemainRatio(0.0);
    ui->WasteSolidBottle->SetRemainVolText("");
    ui->WasteLiquidBottle->SetBottleType(BOTTLE_TYPE_WASTE_L_ERROR);
    ui->WasteLiquidBottle->SetLiquidRemainRatio(0.0);
    ui->WasteLiquidBottle->SetRemainVolText("");

    // 更新耗材界面显示
    UpdateSplUiDisplay();

    // 耗材选择器配对
    m_mapSplChecker.insert(ui->BaseLiquidGrp1CheckFrame, im::tf::SuppliesInfoTable());
    m_mapSplChecker.insert(ui->BaseLiquidGrp2CheckFrame, im::tf::SuppliesInfoTable());
    m_mapSplChecker.insert(ui->CleanLiquid1CheckFrame, im::tf::SuppliesInfoTable());
    m_mapSplChecker.insert(ui->CleanLiquid2CheckFrame, im::tf::SuppliesInfoTable());
}

///
/// @brief
///     更新耗材显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月25日，新建函数
///
void ImRgntPlateWidget::UpdateSplUiDisplay()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->Grp1A1Bottle->update();
    ui->Grp2A2Bottle->update();
    ui->Grp1B1Bottle->update();
    ui->Grp2B2Bottle->update();
    ui->CleanLiquid1Bottle->update();
    ui->CleanLiquid2Bottle->update();
    ui->ReactionBottle->update();
    ui->WasteSolidBottle->update();
    ui->WasteLiquidBottle->update();
}

///
/// @brief
///     清空耗材选中索引
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void ImRgntPlateWidget::ClearSplSelIndex()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历选择器
    for (auto it = m_mapSplChecker.begin(); it != m_mapSplChecker.end(); it++)
    {
        it.key()->SetChecked(false);
    }
}

///
/// @brief
///     从试剂盘中移除试剂
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月7日，新建函数
///
void ImRgntPlateWidget::RemoveRgntFromPlateUi(const im::tf::ReagentInfoTable& stuRgntInfo)
{
    ULOG(LOG_INFO, "%s(ReagentInfoTable)", __FUNCTION__);
    // 设备序列号不匹配则跳过
    if (m_strDevSn != QString::fromStdString(stuRgntInfo.deviceSN))
    {
        ULOG(LOG_INFO, "%s(), device sn is not matched.", __FUNCTION__);
        return;
    }

    // 遍历试剂盘
    for (int iPos = 1; iPos <= m_pRgntPlate->GetTubeSlotCount(); iPos++)
    {
        // 获取选中试剂信息
        boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iPos);
        if (!stuInfo.has_value() || !stuInfo->userData.isValid())
        {
            ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
            continue;
        }

        // 如果试剂为稀释液则跳过
        if (stuInfo->bDillute)
        {
            continue;
        }

        // 获取对应的试剂信息
        im::tf::ReagentInfoTable stuBufRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();

        // 如果位置匹配直接卸载
        if (stuRgntInfo.reagentPos > 0 && stuRgntInfo.reagentPos == iPos)
        {
            // 找到匹配项则卸载
            m_pRgntPlate->UnloadReagent(iPos);
            break;
        }

        // 项目编号不同则不匹配
        if ((stuRgntInfo.assayCode != stuBufRgntInfo.assayCode) && 
            (stuRgntInfo.assayCode > 0 || stuBufRgntInfo.assayCode > 0))
        {
            continue;
        }

        // id不同则不匹配
        if (stuRgntInfo.id != stuBufRgntInfo.id)
        {
            continue;
        }

        // 找到匹配项则卸载
        m_pRgntPlate->UnloadReagent(iPos);
        break;
    }
}

///
/// @brief
///     从试剂盘中移除稀释液
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月7日，新建函数
///
void ImRgntPlateWidget::RemoveRgntFromPlateUi(const im::tf::DiluentInfoTable& stuDltInfo)
{
    ULOG(LOG_INFO, "%s(DiluentInfoTable)", __FUNCTION__);
    // 设备序列号不匹配则跳过
    if (m_strDevSn != QString::fromStdString(stuDltInfo.deviceSN))
    {
        ULOG(LOG_INFO, "%s(), device sn is not matched.", __FUNCTION__);
        return;
    }

    // 遍历试剂盘
    for (int iPos = 1; iPos <= m_pRgntPlate->GetTubeSlotCount(); iPos++)
    {
        // 获取选中试剂信息
        boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iPos);
        if (!stuInfo.has_value() || !stuInfo->userData.isValid())
        {
            ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
            continue;
        }

        // 不是稀释液则跳过
        if (!stuInfo->bDillute)
        {
            continue;
        }

        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuBufDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();

        // 如果位置匹配直接卸载
        if (stuDltInfo.pos > 0 && stuDltInfo.pos == iPos)
        {
            // 找到匹配项则卸载
            m_pRgntPlate->UnloadReagent(iPos);
            break;
        }

        // 匹配名称
        if (stuBufDltInfo.diluentName != stuDltInfo.diluentName)
        {
            continue;
        }

        // Id不匹配则跳过
        if (stuBufDltInfo.id != stuDltInfo.id)
        {
            continue;
        }

        // 找到匹配项则卸载
        m_pRgntPlate->UnloadReagent(iPos);
        break;
    }
}

///
/// @brief 设置试剂扫描按钮是否允许状态，只针对免疫单机
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
void ImRgntPlateWidget::SetEnableOfReagSacnBtn()
{
	// 如果界面未初始化完成
	if (!m_bInit)
	{
		return;
	}
	// 如果不是是免疫单机
	std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetImSingleDevice();
	if (nullptr == spDevInfo)
	{
		return;
	}
	// 试剂扫描按钮是否允许
	ui->RgntScanBtn->setEnabled(IsExistDeviceCanScan());
}

///
/// @brief
///     加载耗材使能信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月12日，新建函数
///
void ImRgntPlateWidget::LoadSplEnableInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 直接查询设备
    tf::DeviceInfoQueryCond qryCond;
    tf::DeviceInfoQueryResp qryResp;
    qryCond.__set_deviceSN(m_strDevSn.toStdString());
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(qryResp, qryCond))
    {
        ULOG(LOG_ERROR, "%s(), QueryDeviceInfo faild", __FUNCTION__);
        return;
    }

    // 如果结果为空，则返回
    if (qryResp.lstDeviceInfos.empty())
    {
        ULOG(LOG_INFO, "%s(), qryResp.lstDeviceInfos.empty()", __FUNCTION__);
        return;
    }

    // 更新设备信息
    auto& curDevIndo = qryResp.lstDeviceInfos[0];

    // 解析otherinfo
    ImDeviceOtherInfo imdoi;
    if (!DecodeJson(imdoi, curDevIndo.otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
        return;
    }

    // 更新耗材使能信息
    m_stuSplEnableInfo.bBaseLiquidGrp1 = imdoi.bSubstrate1;
    m_stuSplEnableInfo.bBaseLiquidGrp2 = imdoi.bSubstrate2;
    m_stuSplEnableInfo.bCleanBuffer1 = imdoi.bConfectLiquid1;
    m_stuSplEnableInfo.bCleanBuffer2 = imdoi.bConfectLiquid2;
    m_stuSplEnableInfo.bWasteLiquid = curDevIndo.enableWasterContainer;
}

///
/// @brief
///     根据耗材使能映射更新UI显示
///
/// @param[in]  mapSplWgts      启用耗材控件映射
/// @param[in]  mapSplRowScale  启用行比例映射
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月12日，新建函数
///
void ImRgntPlateWidget::UpdateSplUiByEnableMap(const QMap<int, QList<QWidget*>>& mapSplWgts, const QMap<int, int>& mapSplRowScale)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    const int ciRowCnt = 3;
    const int ciMaxColCnt = 3;
    QMap<int, QPair<int, int>> mapRowYHeight;
    QMap<QPair<int, int>, int> mapRowScaleHeight;
    mapRowScaleHeight.insert(qMakePair(1, 1), 228);
    mapRowScaleHeight.insert(qMakePair(1, 2), 475);
    mapRowScaleHeight.insert(qMakePair(1, 3), 727);
    mapRowScaleHeight.insert(qMakePair(2, 1), 243);
    mapRowScaleHeight.insert(qMakePair(2, 2), 495);
    mapRowScaleHeight.insert(qMakePair(3, 1), 248);
    int iStartY = 55;
    const int ciSpace = 4;
    for (auto it = mapSplRowScale.begin(); it != mapSplRowScale.end(); it++)
    {
        // 参数检查
        if (it.key() < 1 || it.key() > 3)
        {
            ULOG(LOG_ERROR, "%s(), it.key() < 1 || it.key() > 3", __FUNCTION__);
            return;
        }

        // 计算高度
        auto itRowHeight = mapRowScaleHeight.find(qMakePair(it.key(), it.value()));
        if (itRowHeight == mapRowScaleHeight.end())
        {
            ULOG(LOG_ERROR, "%s(), itRowHeight == mapRowScaleHeight.end()", __FUNCTION__);
            return;
        }

        // 记录行高
        mapRowYHeight.insert(it.key(), qMakePair(iStartY, itRowHeight.value()));

        // 纵坐标起始点改变
        iStartY += itRowHeight.value() + ciSpace;
    }

    // 调整界面位置尺寸
    auto funcAdjustWidgetWidth = [ciMaxColCnt](const QList<QWidget*>& lstRowWgts, const QPair<int, int>& pairYHeight)
    {
        // 获取参数个数
        int iLstSize = lstRowWgts.size();

        // 参数检查
        if ((iLstSize <= 0) || (iLstSize > ciMaxColCnt))
        {
            return false;
        }

        // 不同参数个数分别处理
        if (iLstSize == 1)
        {
            lstRowWgts.at(0)->setGeometry(1140, pairYHeight.first, 762, pairYHeight.second);
        }
        else if (iLstSize == 2)
        {
            lstRowWgts.at(0)->setGeometry(1140, pairYHeight.first, 380, pairYHeight.second);
            lstRowWgts.at(1)->setGeometry(1522, pairYHeight.first, 380, pairYHeight.second);
        }
        else if (iLstSize == 3)
        {
            lstRowWgts.at(0)->setGeometry(1140, pairYHeight.first, 248, pairYHeight.second);
            lstRowWgts.at(1)->setGeometry(1392, pairYHeight.first, 258, pairYHeight.second);
            lstRowWgts.at(2)->setGeometry(1652, pairYHeight.first, 248, pairYHeight.second);
        }
        else
        {
            return false;
        }

        return true;
    };

    // 遍历耗材控件表，调整显示位置和尺寸
    for (auto it = mapSplWgts.begin(); it != mapSplWgts.end(); it++)
    {
        // 参数检查
        if (it.key() < 1 || it.key() > 3)
        {
            ULOG(LOG_ERROR, "%s(), it.key() < 1 || it.key() > 3 mapSplWgts", __FUNCTION__);
            return;
        }

        // 获取Y和行高
        auto itYHeight = mapRowYHeight.find(it.key());
        if (itYHeight == mapRowYHeight.end())
        {
            ULOG(LOG_ERROR, "%s(), itYHeight == mapRowYHeight.end()", __FUNCTION__);
            return;
        }

        // 设置耗材控件位置和尺寸
        if (!funcAdjustWidgetWidth(it.value(), itYHeight.value()))
        {
            ULOG(LOG_ERROR, "%s(), funcAdjustWidgetWidth failed", __FUNCTION__);
            return;
        }
    }
}

///
/// @brief
///     无位置选择卸载试剂
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月10日，新建函数
///
void ImRgntPlateWidget::UnloadRgntInNoSelPos()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取选中设备信息
    tf::DeviceInfo stuTfDevInfo = *CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevSn.toStdString());

    // 是否禁止卸载
    bool bForbidden = PopUnloadRgntForbiddenTip(stuTfDevInfo);

    // 禁止卸载则返回
    if (bForbidden)
    {
        return;
    }

    // 卸载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
    bool bExist = false;
    if (!ReagentLoadExistReag(bExist))
    {
        ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
        return;
    }
    // 存在试剂盒，调用取消加载试剂指令（包含检测是否有试剂盒以及复位回收流程）
    if (bExist)
    {
		// 设置已启动卸载
		RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);

        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, m_strDevSn.toStdString());
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return;
        }
        return;
    }

    // 判断试剂盘是否存在空瓶
    if (!RgntNoticeDataMng::GetInstance()->IsExistEmptyRgntOrDlt())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前试剂盘不存在可卸载的空瓶！")));
        pTipDlg->exec();
        return;
    }

    // 弹框提示是否确认卸载试剂
    TipDlg confirmTipDlg(QObject::tr("试剂卸载"), QObject::tr("确定卸载试剂瓶？"), TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel unload!");
        return;
    }

    // 执行卸载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::UnLoadReagent(_return, m_strDevSn.toStdString(), -1);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "UnLoadReagent failed.");
    }

    // 如果结果返回1，表示找不到要卸载的试剂
    if (_return.value == 1)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选择需要卸载的试剂，请重新选择！")));
        pTipDlg->exec();
        return;
    }

    // 设置已启动卸载
    RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);
}

///
/// @brief
///     试剂屏蔽按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnRgntMaskBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取试剂盘选中索引，无效则返回
    int iSelIndex = m_pRgntPlate->GetSelIndex();
    if (iSelIndex <= 0 || iSelIndex > m_pRgntPlate->GetTubeSlotCount())
    {
        ULOG(LOG_INFO, "%s(), iSelIndex <= 0", __FUNCTION__);
        return;
    }

    // 获取选中试剂信息
    boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iSelIndex);
    if (!stuInfo.has_value() || !stuInfo->userData.isValid())
    {
        ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
        return;
    }

    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;
    devInfoCond.__set_deviceSN(m_strDevSn.toStdString());

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
        return;
    }

    // 结果为空则返回
    if (devInfoResp.lstDeviceInfos.empty())
    {
        ULOG(LOG_INFO, "devInfoResp.lstDeviceInfos.empty()!");
        return;
    }

    // 获取选中设备信息
    tf::DeviceInfo& stuTfDevInfo = devInfoResp.lstDeviceInfos[0];
    im::tf::StatusForUI::type enUseStatus = im::tf::StatusForUI::REAGENT_STATUS_INVALID;

    // 判断是否为稀释液
    if (!stuInfo->bDillute)
    {
        // 获取对应的试剂信息
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();

        // 弹出禁止屏蔽提示框
//         if (!ShieldReagentStatIm(stuTfDevInfo, stuRgntInfo.reagStatusUI.status))
//         {
//             return;
//         }

        // 弹框提示是否确认试剂屏蔽
        if (!IsReagentShieldTipDlg(ui->AssayNameValLab->text(), !stuInfo->bMask))
        {
            return;
        }

        // 执行试剂屏蔽
        im::i6000::tf::ResultReagent _return;
        bool bRet = ::im::i6000::LogicControlProxy::SetReagentMask(_return, stuRgntInfo);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetReagentMask failed.");
            return;
        }
    }
    else
    {
        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();

        // 弹出禁止屏蔽提示框
//         if (!ShieldReagentStatIm(stuTfDevInfo, stuDltInfo.statusForUI.status))
//         {
//             return;
//         }

        // 弹框提示是否确认试剂屏蔽
        if (!IsReagentShieldTipDlg(ui->AssayNameValLab->text(), !stuInfo->bMask))
        {
            return;
        }

        // 执行稀释液屏蔽
        tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::SetDiluentMask(_return, stuDltInfo);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetDiluentMask failed.");
            return;
        }
    }

    // 更新试剂屏蔽文本
    ui->RgntMaskBtn->setText(!stuInfo->bMask ? tr("解除屏蔽") : tr("试剂屏蔽"));
}

///
/// @brief
///     优先使用按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnPriorUseBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取试剂盘选中索引，无效则返回
    int iSelIndex = m_pRgntPlate->GetSelIndex();
    if (iSelIndex <= 0 || iSelIndex > m_pRgntPlate->GetTubeSlotCount())
    {
        ULOG(LOG_INFO, "%s(), iSelIndex <= 0", __FUNCTION__);
        im::tf::SuppliesInfoTable stuSelSplInfo = GetSelSplInfo();
        if (stuSelSplInfo.iD <= 0)
        {
            ULOG(LOG_INFO, "%s(), not sel spl", __FUNCTION__);
            return;
        }

        // 弹框提示是否确认优先使用
        std::shared_ptr<TipDlg> spTipIsPrio = std::make_shared<TipDlg>(tr("优先使用"), tr("确定优先使用选中的耗材？"), TipDlgType::TWO_BUTTON);
        if (spTipIsPrio->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel priority!");
            return;
        }

        // 执行优先使用
        tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::SetSupplyPreUse(_return, stuSelSplInfo);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetSupplyPreUse failed.");
            return;
        }

        return;
    }

    // 弹框提示是否确认优先使用
//     QString rgn = QString("%1-%2").arg(m_strDevName).arg(iSelIndex);
//     if (!IsReagentPriortyTipDlg(rgn))
//     {
//         ULOG(LOG_INFO, "Call isreagentprioritytipdlg return negetive!");
//         return;
//     }

    // 获取选中试剂信息
    const boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iSelIndex);
    if (!stuInfo.has_value() || !stuInfo->userData.isValid())
    {
        ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
        return;
    }

    // 判断是否为稀释液
    if (!stuInfo->bDillute)
    {
        // 获取对应的试剂信息
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();

        // 弹框提示是否确认优先使用
        std::shared_ptr<TipDlg> spTipIsPrio = std::make_shared<TipDlg>(tr("优先使用"), tr("确定优先使用选中的试剂瓶？"), TipDlgType::TWO_BUTTON);
        if (spTipIsPrio->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel priority!");
            return;
        }

        // 执行优先使用
        tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::SetReagentPreUse(_return, stuRgntInfo);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetReagentPreUse failed.");
            return;
        }
    }
    else
    {
        // 获取对应的试剂信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();

        // 弹框提示是否确认优先使用
        std::shared_ptr<TipDlg> spTipIsPrio = std::make_shared<TipDlg>(tr("优先使用"), tr("确定优先使用选中的稀释液？"), TipDlgType::TWO_BUTTON);
        if (spTipIsPrio->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel priority!");
            return;
        }

        // 执行优先使用
        tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::SetDiluentPreUse(_return, stuDltInfo);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetDiluentPreUse failed.");
            return;
        }
    }
}

///
/// @brief
///     试剂装载按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnLoadRgntBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查是否屏蔽试剂在线加载功能，如果屏蔽，则卸载按钮功能为试剂申请 TODO
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, m_strDevSn.toStdString()))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        return;
    }

    // 屏蔽后进入申请更换试剂流程
    if (bMask)
    {
        // 判断当前申请更换试剂状态
        auto RgntApplyStatus = GetApplyChangeRgntStatus(m_strDevSn.toStdString());

        // 如果已申请，则取消
        if (RgntApplyStatus == RGNT_CHANGE_APPLY_STATUS_APPLYING)
        {
            TipDlg tipDlg(tr("取消申请更换试剂"), tr("是否取消申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, m_strDevSn.toStdString(), false);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("取消申请更换试剂"), tr("取消申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(m_strDevSn.toStdString(), RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnStatus();
        }
        else
        {
            TipDlg tipDlg(tr("申请更换试剂"), tr("是否申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, m_strDevSn.toStdString());
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("申请更换试剂"), tr("申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(m_strDevSn.toStdString(), RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnStatus();
        }

        return;
    }

    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;
    devInfoCond.__set_deviceSN(m_strDevSn.toStdString());

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
        return;
    }

    // 获取选中设备信息
    tf::DeviceInfo& stuTfDevInfo = devInfoResp.lstDeviceInfos[0];

    // 弹出禁止装载提示框
    int iPos = m_pRgntPlate->GetSelIndex();
    if (PopLoadRgntForbiddenTip(stuTfDevInfo, iPos))
    {
        return;
    }

    // 检查是否存在试剂盒，如果不存在，则提醒用户——陈建林老师、郭静老师、甘鹏飞老师讨论后决定加载前检查
    bool bExist = false;
    do 
    {
        // 弹框提示是否确认加载试剂
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("装载试剂"), tr("请将试剂盒放入自动加载处，完成后点击“确定”。"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel PriorUse!");
			// 设置已启动卸载
			RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);

            // 取消加载试剂
            ::tf::ResultLong _return;
            bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, m_strDevSn.toStdString());
            if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "CancelLoadReagent failed.");
            }
            return;
        }

        // 检查是否存在试剂
        if (!ReagentLoadExistReag(bExist))
        {
            ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
            return;
        }
    } while (!bExist);

    // 加载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::LoadReagent(_return, m_strDevSn.toStdString(), iPos);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "LoadReagent failed.");
        return;
    }

	// 设置已启动卸载
	RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);
}

///
/// @brief
///     试剂卸载按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnUnloadRgntBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查是否屏蔽试剂在线加载功能，如果屏蔽，则卸载按钮功能为试剂申请 TODO
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, m_strDevSn.toStdString()))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
        return;
    }

    // 屏蔽后进入申请更换试剂流程
    if (bMask)
    {
        // 判断当前申请更换试剂状态
        auto RgntApplyStatus = GetApplyChangeRgntStatus(m_strDevSn.toStdString());

        // 如果已申请，则取消
        if (RgntApplyStatus == RGNT_CHANGE_APPLY_STATUS_APPLYING)
        {
            TipDlg tipDlg(tr("取消申请更换试剂"), tr("是否取消申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, m_strDevSn.toStdString(), false);
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("取消申请更换试剂"), tr("取消申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(m_strDevSn.toStdString(), RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnStatus();
        }
        else
        {
            TipDlg tipDlg(tr("申请更换试剂"), tr("是否申请更换试剂？"), TipDlgType::TWO_BUTTON);
            if (tipDlg.exec() == QDialog::Rejected)
            {
                ULOG(LOG_WARN, "Cancel apply!");
                return;
            }

            ::tf::ResultBool _return;
            ::im::i6000::LogicControlProxy::ManualHandlReag(_return, m_strDevSn.toStdString());
            if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                TipDlg tipDlg(tr("申请更换试剂"), tr("申请更换试剂失败！"), TipDlgType::SINGLE_BUTTON);
                return;
            }

            // 申请之后需要把试剂加卸载按钮置灰，等待申请成功消息
            // TODO
            SetApplyChangeRgnt(m_strDevSn.toStdString(), RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY);
            UpdateBtnStatus();
        }
    }

    // 获取试剂盘选中索引，无效则返回
    int iSelIndex = m_pRgntPlate->GetSelIndex();
    if (iSelIndex <= 0 || iSelIndex > m_pRgntPlate->GetTubeSlotCount())
    {
        ULOG(LOG_INFO, "%s(), iSelIndex <= 0", __FUNCTION__);
        UnloadRgntInNoSelPos();
        return;
    }

    // 获取选中试剂信息
    const boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iSelIndex);
    if (!stuInfo.has_value() || !stuInfo->userData.isValid())
    {
        ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
        UnloadRgntInNoSelPos();
        return;
    }

    // 获取选中设备信息
    tf::DeviceInfo stuTfDevInfo = *CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevSn.toStdString());

    // 是否禁止卸载
    bool bForbidden = false;

    // 获取试剂信息或耗材信息
    if (!stuInfo->bDillute)
    {
        // 获取对应的稀释液信息
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();
        bForbidden = PopUnloadRgntForbiddenTip(stuTfDevInfo, stuRgntInfo);
    }
    else
    {
        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();
        bForbidden = PopUnloadDltForbiddenTip(stuTfDevInfo, stuDltInfo);
    }

    // 禁止卸载则返回
    if (bForbidden)
    {
        return;
    }

	// 卸载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
	bool bExist = false;
	if (!ReagentLoadExistReag(bExist))
	{
		ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
		return;
	}
	// 存在试剂盒，调用取消加载试剂指令（包含检测是否有试剂盒以及复位回收流程）
	if (bExist)
	{
		// 设置已启动卸载
		RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);

		::tf::ResultLong _return;
		bool bRet = ::im::i6000::LogicControlProxy::CancelLoadReagent(_return, m_strDevSn.toStdString());
		if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			return;
		}
		return;
	}

    // 弹框提示是否确认卸载试剂
    if (!IsReagentUnloadTipDlg())
    {
        return;
    }

    // 执行卸载试剂
    ::tf::ResultLong _return;
    bool bRet = ::im::i6000::LogicControlProxy::UnLoadReagent(_return, m_strDevSn.toStdString(), iSelIndex);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "UnLoadReagent failed.");
        return;
    }

    // 设置已启动卸载
    RgntNoticeDataMng::GetInstance()->SetStartUnload(m_strDevSn, true);
}

///
/// @brief
///     磁珠混匀按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnMixBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单机模式直接询问是否执行
//     if (gUiAdapterPtr()->WhetherSingleDevMode())
//     {
//         // 执行单机磁珠混匀
//         ExecuteSingleDevBeadMix(m_strDevSn.toStdString());
//         return;
//     }

    // 弹出磁珠混匀对话框
    std::shared_ptr<ImBeadMixDlg> spImBeadMixDlg(new ImBeadMixDlg(this));
    if (spImBeadMixDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Mix Canceled.", __FUNCTION__);
        return;
    }

    // 获取选择的设备SN
    std::vector<std::string> deviceSns;
    spImBeadMixDlg->GetSelectDeviceSn(deviceSns);
    int iMixCnt = spImBeadMixDlg->GetMixTimes();

    // 获取选中设备信息
    for (const auto& strDevSn : deviceSns)
    {
        // 查询当前选中项设备状态
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
        if (spDevInfo == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "GetDeviceInfo(%s) failed", strDevSn.c_str());
            continue;
        }

        // 判断是否允许执行
        if (PopBeadMixForbiddenTip(*spDevInfo))
        {
            return;
        }
    }

    // 根据选中设备执行磁珠混匀
    for (auto strSn : deviceSns)
    {
        // 执行磁珠混匀
        ::tf::ResultLong _return;
        std::vector<int> vecPos;
        vecPos.push_back(iMixCnt);
        bool bRet = im::LogicControlProxy::SingleMaintance(::tf::MaintainItemType::type::MAINTAIN_ITEM_REA_VORTEX, strSn, vecPos);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "MAINTAIN_ITEM_REA_VORTEX failed.");
        }
    }
}

///
/// @brief
///     装载装置复位按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
void ImRgntPlateWidget::OnLoadPartResetBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单机模式直接询问是否执行
    if (gUiAdapterPtr()->WhetherSingleDevMode())
    {
        // 弹框提示是否确认优先使用
        std::shared_ptr<TipDlg> spTipIsPrio = std::make_shared<TipDlg>(tr("装载装置复位"), tr("确定执行装载装置复位？"), TipDlgType::TWO_BUTTON);
        if (spTipIsPrio->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel priority!");
            return;
        }

        // 执行单机装载装置复位
        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::ResetReagentLoader(_return, m_strDevSn.toStdString());
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ResetReagentLoader(%s) failed", m_strDevSn.toStdString().c_str());
            return;
        }
        return;
    }

    // 弹出装载装置复位对话框
    std::shared_ptr<ImLoaderResetDlg> spImLoaderResetDlg(new ImLoaderResetDlg(this));
    if (spImLoaderResetDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), reset Canceled.", __FUNCTION__);
        return;
    }

    // 获取选择的设备SN
    std::vector<std::string> deviceSns;
    spImLoaderResetDlg->GetSelectDeviceSn(deviceSns);

    // 获取选中设备信息
    QVector<std::string> maskDevSns;
    for (const auto& strDevSn : deviceSns)
    {
        // 查询当前选中项设备状态
        std::shared_ptr<const tf::DeviceInfo> spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
        if (spDevInfo == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "GetDeviceInfo(%s) failed", strDevSn.c_str());
            continue;
        }

        // 判断是否允许执行
        if (PopRgntLoaderResetForbiddenTip(*spDevInfo))
        {
            maskDevSns.push_back(strDevSn);
            continue;
        }
    }

    // 根据选中设备执行磁珠混匀
    for (auto strSn : deviceSns)
    {
        // 如果不能执行则跳过
        if (maskDevSns.contains(strSn))
        {
            continue;
        }

        // 执行装载装置复位
        ::tf::ResultLong _return;
        bool bRet = ::im::i6000::LogicControlProxy::ResetReagentLoader(_return, strSn);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "ResetReagentLoader(%s) failed", strSn.c_str());
        }
    }
}

///
/// @brief
///     试剂扫描按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月26日，新建函数
///
void ImRgntPlateWidget::OnRgntScanBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_scanDialog == nullptr)
    {
        m_scanDialog = new QReagentScanDialog(this);
    }
    m_scanDialog->show();

    // 查询当前选中项设备状态
//     tf::DeviceInfoQueryResp devInfoResp;
//     tf::DeviceInfoQueryCond devInfoCond;
//     devInfoCond.__set_deviceSN(m_strDevSn.toStdString());
// 
//     // 执行查询
//     if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
//     {
//         ULOG(LOG_WARN, "QueryDeviceInfo failed!");
//         return;
//     }
// 
//     // 获取选中设备信息
//     tf::DeviceInfo& stuTfDevInfo = devInfoResp.lstDeviceInfos[0];
// 
//     // 判断当前仪器状态是否允许扫描
//     // 弹框禁止试剂扫描
//     if (PopRgntScanForbiddenTip(stuTfDevInfo))
//     {
//         return;
//     }
// 
//     // 弹出扫描对话框
//     std::shared_ptr<ImRgntScanDlg> spImRgntScanDlg(new ImRgntScanDlg(this));
//     connect(spImRgntScanDlg.get(), SIGNAL(SigOkBtnClicked()), this, SLOT(OnRgntScanDlgOkBtnClicked()));
//     spImRgntScanDlg->exec();
}

///
/// @brief
///     试剂盘选中索引改变
///
/// @param[in]  iSelIndex  选中索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void ImRgntPlateWidget::OnRgntPlateSelIndexChanged(int iSelIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空耗材选中索引
    if (iSelIndex > 0 && iSelIndex <= m_pRgntPlate->GetTubeSlotCount())
    {
        ClearSplSelIndex();
    }

    // 更新按钮状态
    UpdateBtnStatus();

    // 更新选中试剂详情
    UpdateSelRgntDetail(iSelIndex);
}

///
/// @brief
///     更新试剂盘
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void ImRgntPlateWidget::UpdateRgntPlate()
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置试剂盘设备名
    m_pRgntPlate->SetSelDetail(m_strDevName);

    // 记录原选中位置
    int iSelIndex = m_pRgntPlate->GetSelIndex();

    // 清空试剂盘
    m_pRgntPlate->ReseReagent();

    // 更新试剂信息
    UpdateRgntInfo();

    // 更新稀释液信息
    UpdateDltInfo();

    // 恢复选中位置
    m_pRgntPlate->SetSelIndex(iSelIndex);
}

ImReagentState ImRgntPlateWidget::GetImReagentState(const im::tf::ReagentInfoTable& reag)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 首先判断是否为扫描失败
    if (reag.reagentLot.empty() && reag.reagentSN.empty())
    {
        return SCAN_FAILED;
    }

    // 首先判断是否为空瓶
    if (reag.residualTestNum <= 0 || reag.lLDError > 0)
    {
        return EMPTY_BOTTLE;
    }

    ImReagentState reagentStat;
    switch (reag.reagStatusUI.status)
    {
    case im::tf::StatusForUI::REAGENT_STATUS_ON_USE: // 蓝色
        reagentStat = USING;
        break;
    case im::tf::StatusForUI::REAGENT_STATUS_BACKUP:
    case im::tf::StatusForUI::REAGENT_STATUS_MASK:
        reagentStat = BACKUP;
        break;
    default:
        reagentStat = WARNING; // 红色
        break;
    }

    // 获取当前曲线有效期
    QStringList strCalCurveExpLst = QString::fromStdString(reag.caliCurveExpInfo).split(":");

    // 获取开瓶有效期
    int showStableRestday = GetOpenBottleValidDays(reag);

    // 获取试剂失效日期
    QString qstrReagentExpriy = QString::fromStdString(reag.reagentExpiry);
    QDateTime expDateTime = QDateTime::fromString(qstrReagentExpriy, UI_DATE_TIME_FORMAT);

    // 根据上机时间判断是否校准过期
    bool bCaliExpired = IsCaliExpired(reag);
    if (!bCaliExpired)
    {
        // 曲线是否过期
        QStringList strCalCurveExpLst = QString::fromStdString(reag.caliCurveExpInfo).split(":");
        bCaliExpired = ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0) && DictionaryQueryManager::GetInstance()->IsCaliLineExpire());
    }

    // 判断试剂状态
    if ((reagentStat == WARNING) ||
        (reag.lLDError > 0) ||
        (reag.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT && reag.caliCurveExpInfo.empty()) ||
        (reag.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL) ||
        (reag.residualTestNum <= 0) || 
        (reag.avaTestNum <= 0) ||
        (showStableRestday <= 0) || 
        (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime())) ||
        bool(reag.qCOut))
    {
        reagentStat = WARNING;
    }
    else if (RgntNoticeDataMng::GetInstance()->IsRgntResidualTestNotice(reag.assayCode, reag.residualTestNum) || 
             ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0)) ||
             bCaliExpired)
    {
        reagentStat = NOTICE;
    }

    return reagentStat;
}

///
/// @brief
///     获取试剂状态字符串
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return 试剂状态字符串
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月27日，新建函数
///
QString ImRgntPlateWidget::GetReagentStateStr(const im::tf::ReagentInfoTable& stuRgntInfo)
{
    // 文字颜色格式
    QString strTextNoticeColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_NOTEFONT));
    QString strTextWarnColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_WARNFONT));

    // 构造状态列表
    QStringList strStateList;

    // 获取使用状态
    QString strUseStatus = ConvertTfEnumToQString(stuRgntInfo.reagStatusUI.status);
    if (stuRgntInfo.reagStatusUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
    {
        strUseStatus += QString::number(stuRgntInfo.reagStatusUI.backupCnt) + "-" + QString::number(stuRgntInfo.reagStatusUI.backupIndx);
    }

    // 获取状态
    ImReagentState enRgntState = GetImReagentState(stuRgntInfo);

    // 使用状态不为空，则压入状态列表
    strUseStatus.remove(QChar(' '));
    if (!strUseStatus.isEmpty() && (enRgntState == NOTICE || enRgntState == WARNING))
    {
        strStateList.push_back(strUseStatus);
    }

    // 校准状态(未校准有曲线为空)
    QString strCaliStatus("");
	strCaliStatus = ConvertTfEnumToQString(stuRgntInfo.caliStatus);

    // 校准状态不为空，则压入
    if (!strCaliStatus.isEmpty())
    {
        if (stuRgntInfo.caliStatus == tf::CaliStatus::CALI_STATUS_NOT || stuRgntInfo.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL)
        {
            strCaliStatus = strTextWarnColorFmt.arg(strCaliStatus);
        }
        strStateList.push_back(strCaliStatus);
    }

    // 判断是否失控
    if (bool(stuRgntInfo.qCOut))
    {
        strStateList.push_back(strTextWarnColorFmt.arg(tr("试剂失控")));
    }

    // 判断是否失效
    // 获取试剂失效日期
    QString qstrReagentExpriy = QString::fromStdString(stuRgntInfo.reagentExpiry);
    QDateTime expDateTime = QDateTime::fromString(qstrReagentExpriy, UI_DATE_TIME_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
    {
        strStateList.push_back(strTextWarnColorFmt.arg(tr("试剂失效")));
    }

    // 获取开瓶有效期
    int showStableRestday = GetOpenBottleValidDays(stuRgntInfo);
    if (showStableRestday <= 0)
    {
        strStateList.push_back(strTextWarnColorFmt.arg(tr("开瓶过期")));
    }

    return strStateList.join(tr("，"));
}

///
/// @brief
///     获取试剂继承其配对试剂的状态
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月5日，新建函数
///
ImReagentState ImRgntPlateWidget::GetInheritMantchRgntState(const im::tf::ReagentInfoTable& stuRgntInfo)
{
    ULOG(LOG_INFO, "%s(%s,)", __FUNCTION__, ToString(stuRgntInfo));
    // 首先判断是否为空瓶
    if (stuRgntInfo.residualTestNum <= 0 || stuRgntInfo.lLDError > 0)
    {
        return EMPTY_BOTTLE;
    }

    ImReagentState reagentStat;
    switch (stuRgntInfo.reagStatusUI.status)
    {
    case im::tf::StatusForUI::REAGENT_STATUS_ON_USE: // 蓝色
        reagentStat = USING;
        break;
    case im::tf::StatusForUI::REAGENT_STATUS_BACKUP:
    case im::tf::StatusForUI::REAGENT_STATUS_MASK:
        reagentStat = BACKUP;
        break;
    default:
        reagentStat = WARNING; // 红色
        break;
    }

    // 判断试剂状态
    if ((reagentStat == WARNING) ||
        (stuRgntInfo.caliStatus == ::tf::CaliStatus::CALI_STATUS_NOT && stuRgntInfo.caliCurveExpInfo.empty()) ||
        (stuRgntInfo.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL) ||
        (stuRgntInfo.residualTestNum <= 0))
    {
        reagentStat = WARNING;
    }
    else if (RgntNoticeDataMng::GetInstance()->IsRgntResidualTestNotice(stuRgntInfo.assayCode, stuRgntInfo.residualTestNum) ||
             RgntNoticeDataMng::GetInstance()->IsRgntAvaTestNotice(stuRgntInfo.assayCode, stuRgntInfo.avaTestNum))
    {
        reagentStat = NOTICE;
    }

    return reagentStat;
}

///
/// @brief
///     获取稀释液状态
///
/// @param[in]  stuDlt  稀释液信息
///
/// @return 稀释液状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月27日，新建函数
///
ImReagentState ImRgntPlateWidget::GetImDltState(const im::tf::DiluentInfoTable& stuDlt)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 首先判断是否为空瓶
    if (stuDlt.curVol <= DBL_EPSILON || stuDlt.LLDErr > 0)
    {
        return EMPTY_BOTTLE;
    }

    ImReagentState dltStat;
    switch (stuDlt.statusForUI.status)
    {
    case im::tf::StatusForUI::REAGENT_STATUS_ON_USE: // 蓝色
        dltStat = USING;
        break;
    case im::tf::StatusForUI::REAGENT_STATUS_BACKUP:
    case im::tf::StatusForUI::REAGENT_STATUS_MASK:
        dltStat = BACKUP;
        break;
    default:
        dltStat = WARNING; // 红色
        break;
    }

    // 获取开瓶有效期
    int showStableRestday = GetOpenBottleValidDays(stuDlt);

    // 获取稀释液失效日期
    QString strDltExpireDate = QString::fromStdString(stuDlt.diluentExpiry);
    QDateTime expDateTime = QDateTime::fromString(strDltExpireDate, UI_DATE_TIME_FORMAT);

    // 判断试剂状态
    if ((dltStat == WARNING) ||
        (stuDlt.LLDErr > 0) ||
        (stuDlt.curVol <= DBL_EPSILON) ||
        (stuDlt.avaVol <= DBL_EPSILON) ||
        (showStableRestday <= 0) ||
        (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime())))
    {
        dltStat = WARNING;
    }
    else if (RgntNoticeDataMng::GetInstance()->IsDltCurVolNotice(stuDlt.diluentNumber, stuDlt.curVol / 1000.0))
    {
        dltStat = NOTICE;
    }

    return dltStat;
}

///
/// @brief
///     获取稀释液状态字符串
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return 稀释液状态字符串
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月27日，新建函数
///
QString ImRgntPlateWidget::GetDltStateStr(const im::tf::DiluentInfoTable& stuDltInfo)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 文字颜色格式
    QString strTextNoticeColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_NOTEFONT));
    QString strTextWarnColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_WARNFONT));

    // 构造状态列表
    QStringList strStateList;

    // 获取使用状态
    QString strUseStatus = ConvertTfEnumToQString(stuDltInfo.statusForUI.status);
    if (stuDltInfo.statusForUI.status == ::im::tf::StatusForUI::REAGENT_STATUS_BACKUP)
    {
        strUseStatus += QString::number(stuDltInfo.statusForUI.backupCnt) + "-" + QString::number(stuDltInfo.statusForUI.backupIndx);
    }

    // 获取状态
    ImReagentState enDltState = GetImDltState(stuDltInfo);

    // 使用状态不为空，则压入状态列表
    strUseStatus.remove(QChar(' '));
    if (!strUseStatus.isEmpty() && (enDltState == NOTICE || enDltState == WARNING))
    {
        strStateList.push_back(strUseStatus);
    }

    // 判断是否失效
    // 获取稀释液失效日期
    QString strDltExpireDate = QString::fromStdString(stuDltInfo.diluentExpiry);
    QDateTime expDateTime = QDateTime::fromString(strDltExpireDate, UI_DATE_TIME_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
    {
        strStateList.push_back(strTextWarnColorFmt.arg(tr("稀释液失效")));
    }

    // 获取开瓶有效期
    int showStableRestday = GetOpenBottleValidDays(stuDltInfo);
    if (showStableRestday <= 0)
    {
        strStateList.push_back(strTextWarnColorFmt.arg(tr("开瓶过期")));
    }

    return strStateList.join(tr("，"));
}

///
/// @brief
///     获取耗材状态
///
/// @param[in]  stuSpl  耗材信息
///
/// @return 耗材状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月21日，新建函数
///
ImRgntPlateWidget::SuppliesStatus ImRgntPlateWidget::GetImSplState(const im::tf::SuppliesInfoTable& stuSpl)
{
    //ULOG(LOG_INFO, "%s", __FUNCTION__); // too frequency.
    SuppliesStatus splState;
    switch (stuSpl.usingStatus)
    {
    case 0:                                             // 无效
        splState = SPL_STATUS_WARN;
        break;
    case 1:                                             // 在用
        splState = SPL_STATUS_USING;
        break;
    case 2:                                             // 备用
        splState = SPL_STATUS_BACKUP;
        break;
    case 3:                                             // 停用
        splState = SPL_STATUS_WARN;
        break;
    default:
        splState = SPL_STATUS_WARN;                     // 红色警告
        break;
    }

    // 余量警告和余量提醒标志
    bool bRemainVolAlarm = false;
    bool bRemainVolNotice = false;
    if ((stuSpl.supType == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID) || (stuSpl.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID))
    {
        // 废液和废料以大于提醒值为提醒
        if (RgntNoticeDataMng::GetInstance()->IsSplReVolNotice(stuSpl.supType, stuSpl.remainVol))
        {
            bRemainVolNotice = true;
        }

        // 废液桶或者废料桶满，则警告 ——mod-tcx由于触发光电时后台存在缺陷可能会将规格改为0，此处不再判断规格为0的情况
        if (/*(stuSpl.volumeAmount > 0) && */(stuSpl.remainVol >= stuSpl.volumeAmount))
        {
            bRemainVolAlarm = true;
        }
    }
    else
    {
        // 获取开瓶有效期
        int showStableRestday = GetOpenBottleValidDays(stuSpl);

        // 获取耗材失效日期
        QString strSplExpriy = QString::fromStdString(stuSpl.expiryDate);
        QDateTime expDateTime = QDateTime::fromString(strSplExpriy, UI_DATE_TIME_FORMAT);

        int iResidual = stuSpl.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS ? int(stuSpl.remainVol) : int(stuSpl.remainVol + 1.0 - 0.0001);
        if (stuSpl.available == 0 &&
            (stuSpl.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A || stuSpl.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B))
        {
            iResidual = 0;
        }

        // 普通耗材以空为警告
        if ((iResidual <= 0) ||
            (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime())))
        {
            bRemainVolAlarm = true;
        }

        // 反应杯没有开瓶有效期
        if ((stuSpl.supType != im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS) && (showStableRestday <= 0))
        {
            bRemainVolAlarm = true;
        }

        // 使用状态为警告，则警告
        if (splState == WARNING)
        {
            bRemainVolAlarm = true;
        }

        // 普通耗材以小于提醒值为提醒
        if (RgntNoticeDataMng::GetInstance()->IsSplReVolNotice(stuSpl.supType, stuSpl.remainVol))
        {
            bRemainVolNotice = true;
        }
    }

    // 判断试剂状态
    if (bRemainVolAlarm)
    {
        splState = SPL_STATUS_WARN;
    }
    else if (bRemainVolNotice)
    {
        splState = SPL_STATUS_REMAIND;
    }

    return splState;
}

void ImRgntPlateWidget::AssignImReagentInfo(const im::tf::ReagentInfoTable& reag, ImReagentInfo &stuInfo)
{
    ULOG(LOG_INFO, "%s(%s,)", __FUNCTION__, ToString(reag));
    // 缓存试剂信息
    stuInfo.userData = QVariant::fromValue<im::tf::ReagentInfoTable>(reag);

    // 颜色状态
    stuInfo.enState = GetImReagentState(reag);

    // 设置关联试剂的试剂盘指示器
//     QSet<int> matchedIdx;
//     GetRelatedReagentIndexs(reag, matchedIdx);
//     for (auto it = matchedIdx.begin(); it != matchedIdx.end(); it++)
//     {
//         // 获取对应位置的试剂信息
//         const boost::optional<ImReagentInfo> stuUiInfo = m_pRgntPlate->GetReagentInfo(*it);
//         if (!stuUiInfo.has_value() || !stuUiInfo->userData.isValid() || stuUiInfo->bDillute)
//         {
//             continue;
//         }
// 
//         // 预处理试剂， 批号相同，瓶号substep互同
//         im::tf::ReagentInfoTable stuRgntInfo = stuUiInfo->userData.value<im::tf::ReagentInfoTable>();
//         ImReagentState enMatchedState = GetInheritMantchRgntState(stuRgntInfo);
// 
//         // 如果状态为空则跳过
//         if (enMatchedState == EMPTY)
//         {
//             continue;
//         }
// 
//         // 如果状态级别更高，则更新当前试剂的状态为更高状态
//         if (int(enMatchedState) > int(stuInfo.enState))
//         {
//             stuInfo.enState = enMatchedState;
//         }
//     }

    // 备用瓶索引
    stuInfo.iBackupNo = (im::tf::StatusForUI::REAGENT_STATUS_ON_USE == reag.reagStatusUI.status) ? 0 : \
        (im::tf::StatusForUI::REAGENT_STATUS_BACKUP == reag.reagStatusUI.status ? reag.reagStatusUI.backupIndx : \
            stuInfo.iBackupNo);

    // 掩码
    stuInfo.bMask = im::tf::StatusForUI::REAGENT_STATUS_MASK == reag.reagStatusUI.status ? true : stuInfo.bMask;

    // 是否是稀释液
    stuInfo.bDillute = false;
}

///
/// @brief
///     更新关联试剂状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月9日，新建函数
///
void ImRgntPlateWidget::UpdateRelativeRgntState()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 试剂槽位号从1开始
    for (int i = 1; i <= SLOT_CNT_OF_RGNT_PLATE; ++i)
    {
        boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(i);
        if (!stuInfo.has_value() || !stuInfo->userData.isValid() || stuInfo->bDillute)
        {
            continue;
        }

        // 获取试剂信息
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();

        // 设置关联试剂的试剂盘指示器
        QSet<int> matchedIdx;
        GetRelatedReagentIndexs(stuRgntInfo, matchedIdx);
        for (auto it = matchedIdx.begin(); it != matchedIdx.end(); it++)
        {
            // 获取对应位置的试剂信息
            const boost::optional<ImReagentInfo> stuUiInfo = m_pRgntPlate->GetReagentInfo(*it);
            if (!stuUiInfo.has_value() || !stuUiInfo->userData.isValid() || stuUiInfo->bDillute)
            {
                continue;
            }

            // 预处理试剂， 批号相同，瓶号substep互同
            im::tf::ReagentInfoTable stuMatchRgntInfo = stuUiInfo->userData.value<im::tf::ReagentInfoTable>();
            ImReagentState enMatchedState = GetInheritMantchRgntState(stuMatchRgntInfo);

            // 如果状态为空则跳过
            if (enMatchedState == EMPTY)
            {
                continue;
            }

            // 如果状态级别更高，则更新当前试剂的状态为更高状态
            if (int(enMatchedState) > int(stuInfo->enState))
            {
                stuInfo->enState = enMatchedState;
            }
        }

        // 更新试剂状态
        m_pRgntPlate->LoadReagent(i, stuInfo.value());
    }
}

///
/// @brief 试剂加载机构试剂槽是否存在试剂盒
///
/// @param[out]  bExist  true-存在，false-不存在
///
/// @return true-获取成功，false-获取失败
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月4日，新建函数
///
bool ImRgntPlateWidget::ReagentLoadExistReag(bool& bExist)
{
	// 加载前试剂加载机构检测试剂槽上是否有试剂盒，如果存在，则弹窗提示用户拿走试剂盒
	::tf::ResultBool _return_existReag;
	im::LogicControlProxy::QueryReagentLoadUnloadExistReagent(_return_existReag, m_strDevSn.toStdString());
	if (_return_existReag.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "ReagentLoadExistReag failed!");
		return false;
	}
	bExist = _return_existReag.value;

	return true;
}

void ImRgntPlateWidget::AssayLabelWithTip(QLabel* qLable, const QString& strText)
{
    if (qLable == Q_NULLPTR)
    {
        return;
    }
    qLable->setText(strText);

    // 文字显示不完，添加tooltip
    QFontMetrics fm(qLable->fontMetrics());
    if (fm.width(strText) > 220)
    {
        qLable->setToolTip(strText);
    }
    else
    {
        qLable->setToolTip("");
    }
}

///
/// @brief
///     更新试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void ImRgntPlateWidget::UpdateRgntInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 构造查询条件和查询结果
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 筛选条件
    std::vector<std::string> vtDv;
    vtDv.push_back(m_strDevSn.toStdString());
    qryCond.__set_deviceSNs(vtDv);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::ReagentInfoTable& stuRgntInfo : qryResp.lstReagentInfos)
    {
        // 构造试剂信息(后台结构未完善)
        ImReagentInfo stuInfo;
        AssignImReagentInfo(stuRgntInfo, stuInfo);

        m_pRgntPlate->LoadReagent(stuRgntInfo.reagentPos, stuInfo);
    }

    // 更新关联试剂状态
    UpdateRelativeRgntState();
}

///
/// @brief
///     更新稀释液信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void ImRgntPlateWidget::UpdateDltInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 构造查询条件和查询结果
    ::im::tf::DiluentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;
    // 筛选条件
    std::vector<std::string> vtDv;
    vtDv.push_back(m_strDevSn.toStdString());
    qryCond.__set_deviceSNs(vtDv);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryDiluentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDiluentInfoForUI failed.");
        return;
    }

    // 将试剂信息加载到试剂盘中
    for (const im::tf::DiluentInfoTable& stuDltInfo : qryResp.lstDiluentInfos)
    {
        // 构造试剂信息(后台结构未完善)
        ImReagentInfo stuInfo;

        // 缓存稀释液信息
        stuInfo.userData = QVariant::fromValue<im::tf::DiluentInfoTable>(stuDltInfo);

        // 使用状态(后台无使用状态字段，功能暂无法实现)
        if (im::tf::StatusForUI::REAGENT_STATUS_ON_USE == stuDltInfo.statusForUI.status)
        {
            stuInfo.enState = USING;
            stuInfo.iBackupNo = 0;
        }
        else if (im::tf::StatusForUI::REAGENT_STATUS_BACKUP == stuDltInfo.statusForUI.status)
        {
            stuInfo.enState = BACKUP;
            stuInfo.iBackupNo = stuDltInfo.statusForUI.backupIndx;
        }
        else if (im::tf::StatusForUI::REAGENT_STATUS_MASK == stuDltInfo.statusForUI.status)
        {
            stuInfo.enState = BACKUP;
            stuInfo.bMask = true;
        }
        else
        {
            stuInfo.enState = WARNING;
        }

        // 获取稀释液状态
        stuInfo.enState = GetImDltState(stuDltInfo);

        // 是否是稀释液
        stuInfo.bDillute = true;

        m_pRgntPlate->LoadReagent(stuDltInfo.pos, stuInfo);
    }
}

///
/// @brief
///     更新耗材信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void ImRgntPlateWidget::UpdateSplInfo()
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置耗材信息
    ResetSplInfo();

    // 构造查询条件和查询结果
    ::im::tf::SuppliesInfoTableQueryResp qryResp;
    ::im::tf::SuppliesInfoTableQueryCond qryCond;
    qryCond.__set_isLoad(1);
    if (!m_strDevSn.isEmpty())
    {
        qryCond.__set_deviceSN(m_strDevSn.toStdString());
    }

    // 查询所有耗材信息
    bool bRet = ::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfo failed.");
        return;
    }

    // 将耗材信息加载到图例中(待完善)
    auto LoadSupplyFunc = [this](const im::tf::SuppliesInfoTable& stuSplInfo)
    {
        // 判断耗材类型
        ImLiquidBottleWidget* pTargetBottleWgt = Q_NULLPTR;
        CheckableFrame* pCheckableFrame = Q_NULLPTR;
        SuppliesType enType;
        SuppliesStatus enStatus;
        if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_WASH_WATER)
        {
            ///< 清洗液1（缓存桶）
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A)
        {
            ///< 底物A
            if (stuSplInfo.groupNum == 1)
            {
                pTargetBottleWgt = ui->Grp1A1Bottle;
                pCheckableFrame = ui->BaseLiquidGrp1CheckFrame;
            }
            else if (stuSplInfo.groupNum == 2)
            {
                pTargetBottleWgt = ui->Grp2A2Bottle;
                pCheckableFrame = ui->BaseLiquidGrp2CheckFrame;
            }
            enType = SPL_TYPE_BASE_LIQUID_A;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B)
        {
            ///< 底物B
            if (stuSplInfo.groupNum == 1)
            {
                pTargetBottleWgt = ui->Grp1B1Bottle;
                pCheckableFrame = ui->BaseLiquidGrp1CheckFrame;
            }
            else if (stuSplInfo.groupNum == 2)
            {
                pTargetBottleWgt = ui->Grp2B2Bottle;
                pCheckableFrame = ui->BaseLiquidGrp2CheckFrame;
            }
            enType = SPL_TYPE_BASE_LIQUID_B;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_PURE_WATER)
        {
            ///< 纯水
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
        {
            ///< 新反应杯
            pTargetBottleWgt = ui->ReactionBottle;
            enType = SPL_TYPE_REACTION_CUP;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID)
        {
            ///< 固体废物
            pTargetBottleWgt = ui->WasteSolidBottle;
            enType = SPL_TYPE_WASTE_SOLID;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID)
        {
            ///< 清洗缓冲液
            if (stuSplInfo.groupNum == 1)
            {
                pTargetBottleWgt = ui->CleanLiquid1Bottle;
                pCheckableFrame = ui->CleanLiquid1CheckFrame;
            }
            else if (stuSplInfo.groupNum == 2)
            {
                pTargetBottleWgt = ui->CleanLiquid2Bottle;
                pCheckableFrame = ui->CleanLiquid2CheckFrame;
            }
            enType = SPL_TYPE_WASH_BUFFER;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID)
        {
            ///< 废液
            pTargetBottleWgt = ui->WasteLiquidBottle;
            enType = SPL_TYPE_WASTE_LIQUID;
        }
        else if (stuSplInfo.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONFECT_LIQUID)
        {
            ///< 配液桶
        }

        // 如果储液瓶没有对应项，则跳过
        if (pTargetBottleWgt == Q_NULLPTR)
        {
            return;
        }

        // 耗材选择器配对
        if (pCheckableFrame != Q_NULLPTR)
        {
            m_mapSplChecker.insert(pCheckableFrame, stuSplInfo);
        }

        // 默认显示：待完善
        //pTargetBottleWgt->setVisible(true);

        // 设置余量文本
        double dResidual = stuSplInfo.remainVol;
        if (enType == SPL_TYPE_BASE_LIQUID_A || enType == SPL_TYPE_BASE_LIQUID_B)
        {
            // 设置余量文本(向上取整)
            int iResidual = stuSplInfo.available == 0 ? 0 : int(stuSplInfo.remainVol + 1.0 - 0.0001);
            dResidual = stuSplInfo.available == 0 ? 0.0 : stuSplInfo.remainVol;
            pTargetBottleWgt->SetRemainVolText(QString::number(iResidual) + "ml");
        }
        else if (enType == SPL_TYPE_WASH_BUFFER)
        {
            // 设置余量文本(向上取整)
            int iResidual = stuSplInfo.available == 0 ? 0 : int(stuSplInfo.remainVol + 1.0 - 0.0001);
            dResidual = stuSplInfo.available == 0 ? 0.0 : stuSplInfo.remainVol;
            pTargetBottleWgt->SetRemainVolText(QString::number(iResidual) + "ml");
        }
        else if (enType == SPL_TYPE_REACTION_CUP)
        {
            // 设置余量文本
            pTargetBottleWgt->SetRemainVolText(QString::number(stuSplInfo.remainVol) + tr("个"));
        }

        // 判断总容量
        double dVolCnt = stuSplInfo.volumeAmount;
//         if ((enType == SPL_TYPE_WASTE_LIQUID) || (enType == SPL_TYPE_WASTE_SOLID))
//         {
//             dVolCnt = stuSplInfo.alarmVol;
//         }
//         else
//         {
//             dVolCnt = stuSplInfo.volumeAmount;
//         }

        // 判断使用状态(待完善)
        if (stuSplInfo.usingStatus == 0)
        {
            // 无效
            enStatus = SPL_STATUS_INVALID;
        }
        else if (stuSplInfo.usingStatus == 1)
        {
            // 在用
            enStatus = SPL_STATUS_USING;
        }
        else if (stuSplInfo.usingStatus == 2)
        {
            // 备用
            enStatus = SPL_STATUS_BACKUP;
        }
        else if (stuSplInfo.usingStatus == 3)
        {
            // 停用(待完善)
            //pTargetBottleWgt->setVisible(false);
        }

        // 获取稀释液状态
        enStatus = GetImSplState(stuSplInfo);

        // 获取储液瓶类型
        pTargetBottleWgt->SetBottleType(GetBottleType(enType, enStatus));
        pTargetBottleWgt->SetLiquidRemainRatio(dVolCnt > DBL_EPSILON ? dResidual / dVolCnt : 1.0);
    };

    for (const im::tf::SuppliesInfoTable& stuSplInfo : qryResp.lstSuppliesInfos)
    {
        // 加载耗材信息
        LoadSupplyFunc(stuSplInfo);
    }

    // 更新耗材界面显示
    UpdateSplUiDisplay();
}

///
/// @brief
///     更新当前显示是被
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月1日，新建函数
///
void ImRgntPlateWidget::UpdateCurDev(std::shared_ptr<const tf::DeviceInfo>& spDev)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (spDev == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), invalid param", __FUNCTION__);
        return;
    }

    // 记录设备名和序列号
    m_strDevName = QString::fromStdString(spDev->name);
    m_strDevSn = QString::fromStdString(spDev->deviceSN);

    // 更新试剂盘
    UpdateRgntPlate();

    // 更新耗材信息
    UpdateSplInfo();

    // 更新耗材使能状态
    UpdateSplEnabled();

    // 更新选中试剂详情
    UpdateSelRgntDetail();

    // 更新按钮状态
    UpdateBtnStatus();
}

///
/// @brief 设备状态变更
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
void ImRgntPlateWidget::DevStateChange()
{
	// 设置试剂扫描按钮是否允许状态，只针对免疫单机
	SetEnableOfReagSacnBtn();
}

///
/// @brief
///     更新选中试剂详情
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void ImRgntPlateWidget::UpdateSelRgntDetail()
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取试剂盘选中索引，无效则返回
    int iSelIndex = m_pRgntPlate->GetSelIndex();

    // 更新选中试剂详情
    UpdateSelRgntDetail(iSelIndex);
}

///
/// @brief
///     更新选中试剂详情
///
/// @param[in]  iSelPos    选中位置
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void ImRgntPlateWidget::UpdateSelRgntDetail(int iSelPos)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iSelPos);
    // 清空选中试剂信息
    ClearSelRgntInfo();

    // 参数检查
    if (iSelPos <= 0 || iSelPos > m_pRgntPlate->GetTubeSlotCount())
    {
        ULOG(LOG_INFO, "%s(), iSelIndex <= 0", __FUNCTION__);
        return;
    }

    // 判断是否是空位
    boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iSelPos);
    if (!stuInfo.has_value() || !stuInfo->userData.isValid())
    {
        ULOG(LOG_INFO, "%s(%d), !stuInfo.has_value()", __FUNCTION__, iSelPos);
        return;
    }

    // 构造一个QLabel富文本，用于设置行高
    //QString strLineHeightFmt("<p style='line-height:48px'>%1</p>");
    QString strLineHeightFmt("%1");
    QString strTextRedColorFmt = QString("<font style='color: %1;'>%2</font>").arg(QString(UI_REAGENT_WARNFONT));

    // 判断是否是稀释液
    if (stuInfo->bDillute)
    {
        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();

        // 批号序列号同时为空判定扫描失败
        if (stuDltInfo.dluentLot.empty() && stuDltInfo.diluentSN.empty())
        {
            AssayLabelWithTip(ui->AssayNameValLab, tr("扫描失败"));
            ui->PosValLab->setText(m_strDevName + "-" + QString::number(stuDltInfo.pos));
            return;
        }

        // 构造沟渠日期
        QStringList strDateTimeList = QString::fromStdString(stuDltInfo.diluentExpiry).split(" ");
        QString strDate("");
        if (strDateTimeList.size() >= 1)
        {
            strDate = strDateTimeList.front();
        }

        // 余量文本
        QString strRemainText = (stuDltInfo.LLDErr > 0) ? ("0ml") : QString::number(QString::number(stuDltInfo.curVol / 1000.0, 'f', 1).toDouble()) + "ml";
        if (stuDltInfo.LLDErr > 0 || stuDltInfo.curVol <= DBL_EPSILON)
        {
            strRemainText = strTextRedColorFmt.arg(strRemainText);
        }

        // 失效日期
        QString strExpireDate = ToCfgFmtDateTime(strDate);
        QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(stuDltInfo.diluentExpiry), UI_DATE_TIME_FORMAT);
        if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
        {
            strExpireDate = strTextRedColorFmt.arg(strExpireDate);
        }

        // 将选中试剂信息显示在界面上
        AssayLabelWithTip(ui->AssayNameValLab, QString::fromStdString(stuDltInfo.diluentName));
        ui->PosValLab->setText(m_strDevName + "-" + QString::number(stuDltInfo.pos));
        ui->RgntLotValLab->setText(QString::fromStdString(stuDltInfo.dluentLot));
        ui->ExpireDateValLab->setText(strExpireDate);
        ui->RemainTestCntValLab->setText(strRemainText);
        ui->StateValLab->setText(strLineHeightFmt.arg(GetDltStateStr(stuDltInfo)));
        return;
    }

    // 获取对应的试剂信息
    im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();

    // 批号序列号同时为空判定扫描失败
    if (stuRgntInfo.reagentLot.empty() && stuRgntInfo.reagentSN.empty())
    {
        AssayLabelWithTip(ui->AssayNameValLab, tr("扫描失败"));
        ui->PosValLab->setText(m_strDevName + "-" + QString::number(stuRgntInfo.reagentPos));
        return;
    }

    // 转换项目编号为项目名
    QString strAssayName = QString::fromStdString(stuRgntInfo.name);
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuRgntInfo.assayCode);
    if (Q_NULLPTR != spAssayInfo)
    {
        ULOG(LOG_WARN, "Invalid assay code,");
        strAssayName = QString::fromStdString(spAssayInfo->assayName);
    }

    // 构造过期日期
    QStringList strDateTimeList = QString::fromStdString(stuRgntInfo.reagentExpiry).split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    // 设置关联试剂的试剂盘指示器
    QSet<int> relatedIndexs = { iSelPos };
    GetRelatedReagentIndexs(stuRgntInfo, relatedIndexs);
    m_pRgntPlate->SetIndicator(relatedIndexs);

    // 失效日期
    QString strExpireDate = ToCfgFmtDateTime(strDate);
    QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(stuRgntInfo.reagentExpiry), UI_DATE_TIME_FORMAT);
    if (expDateTime.isValid() && (expDateTime <= QDateTime::currentDateTime()))
    {
        strExpireDate = strTextRedColorFmt.arg(strExpireDate);
    }

    // 将选中试剂信息显示在界面上
    AssayLabelWithTip(ui->AssayNameValLab, strAssayName);
    ui->PosValLab->setText(m_strDevName + "-" + QString::number(stuRgntInfo.reagentPos));
    ui->RgntLotValLab->setText(QString::fromStdString(stuRgntInfo.reagentLot));
    ui->ExpireDateValLab->setText(strExpireDate);
	if (stuRgntInfo.residualTestNum < 0)
	{
		ui->RemainTestCntValLab->setText("0");
	}
	else
	{
		ui->RemainTestCntValLab->setText((stuRgntInfo.lLDError > 0) ? ("0") : QString::number(stuRgntInfo.residualTestNum));
	}    

    // 余量文本
    QString strRemainText = ui->RemainTestCntValLab->text();
    if (strRemainText == "0")
    {
        strRemainText = strTextRedColorFmt.arg(strRemainText);
    }
    ui->RemainTestCntValLab->setText(strRemainText);

    ui->StateValLab->setText(strLineHeightFmt.arg(GetReagentStateStr(stuRgntInfo)));
}

///
/// @brief
///     更新试剂信息
///
/// @param[in]  srr   试剂信息
/// @param[in]  type  试剂更新类型
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月19日，新建函数
///
void ImRgntPlateWidget::UpdateReagentInfo(const im::tf::ReagentInfoTable& srr, const im::tf::ChangeType::type type)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果设备不匹配则忽略
    if (m_strDevSn != QString::fromStdString(srr.deviceSN))
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    // 从界面移除对应试剂
    RemoveRgntFromPlateUi(srr);

    // 构造试剂查询结构
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 构造查询条件
    if (srr.__isset.assayCode && (srr.assayCode > 0))
    {
        std::vector<std::string> vecDevSns;
        vecDevSns.push_back(srr.deviceSN);
        qryCond.__set_deviceSNs(vecDevSns);
        qryCond.__set_assayCode(srr.assayCode);
    }
    else
    {
        qryCond.__set_reagentId(srr.id);
    }

    // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfo failed.");
        UpdateSelRgntDetail();
        UpdateBtnStatus();
        return;
    }

    // 更新该设备所有同项目试剂
    for (const auto& stuRgntInfo : qryResp.lstReagentInfos)
    {
        // 构造试剂信息
        ImReagentInfo stuInfo;
        AssignImReagentInfo(stuRgntInfo, stuInfo);

        m_pRgntPlate->LoadReagent(stuRgntInfo.reagentPos, stuInfo);

        // 如果试剂位置与当前选中位置相同，则刷新当前选中试剂信息
        // 获取试剂盘选中索引
        int iSelIndex = m_pRgntPlate->GetSelIndex();
        if (iSelIndex == stuRgntInfo.reagentPos)
        {
            UpdateSelRgntDetail(iSelIndex);
            UpdateBtnStatus();
        }
    }

    // 更新关联试剂状态
    UpdateRelativeRgntState();
}

///
/// @brief
///     免疫稀释液信息更新
///
/// @param[in]  stuDltInfo   试剂信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月5日，新建函数
///
void ImRgntPlateWidget::UpdateImDltChanged(const im::tf::DiluentInfoTable& stuDltInfo, im::tf::ChangeType::type changeType)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果设备不匹配则忽略
    if (m_strDevSn != QString::fromStdString(stuDltInfo.deviceSN))
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    // 从界面移除对应稀释液
    RemoveRgntFromPlateUi(stuDltInfo);

    // 更新稀释液信息
    UpdateDltInfo();

    // 更新试剂盘选中试剂详情
    UpdateSelRgntDetail();

    // 更新按钮状态
    UpdateBtnStatus();
}

///
/// @brief
///     免疫耗材信息更新
///
/// @param[in]  stuSplInfo   耗材信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月5日，新建函数
///
void ImRgntPlateWidget::UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(changeType);
    // 刷新耗材信息
    UpdateSplInfo();
    UpdateBtnStatus();
}

///
/// @brief
///     更新废液桶显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月27日，新建函数
///
void ImRgntPlateWidget::UpdateBucketVisible()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新耗材使能
    UpdateSplEnabled();
}

///
/// @brief
///     清空试剂信息
///
/// @param[in]  strDevSn		设备序列号
/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
/// @param[in]  lGrpId			维护组ID
/// @param[in]  mit				维护项目类型
/// @param[in]  enPhase			阶段
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月19日，新建函数
///
void ImRgntPlateWidget::OnClearReagentInfo(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type mit, tf::MaintainResult::type enPhase)
{
    ULOG(LOG_INFO, "%s(, %d, %d)", __FUNCTION__, int(mit), int(enPhase));
	Q_UNUSED(devModuleIndex);
	Q_UNUSED(lGrpId);
    if (m_strDevSn != strDevSn)
    {
        return;
    }

    if ((mit != ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN) || (enPhase != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING))
    {
        return;
    }

    // 设备不匹配则忽略
    if (m_strDevSn != strDevSn)
    {
        return;
    }

    // 重置试剂信息
    m_pRgntPlate->ReseReagent();

    // 清空选中试剂信息
    ClearSelRgntInfo();
}

///
/// @brief
///     更新按钮状态（如禁能使能，按钮文本等）
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月5日，新建函数
///
void ImRgntPlateWidget::UpdateBtnStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 页面处于隐藏状态时不处理
    if (!isVisible())
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 获取当前设备信息
    auto stuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevSn.toStdString());
    if (stuTfDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), stuTfDevInfo == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 查询加载功能是否屏蔽
    bool bMask = false;
    if (!ReagentLoadIsMask(bMask, stuTfDevInfo->deviceSN))
    {
        ULOG(LOG_WARN, "ReagentLoadIsMask failed!");
    }

    // 如果在线加载功能屏蔽，则合并加载和卸载按钮
    if (bMask)
    {
        ui->UnloadRgntBtn->setVisible(false);
        ui->LoadRgntBtn->setText(GetApplyChangeRgntStatus(stuTfDevInfo->deviceSN) == RGNT_CHANGE_APPLY_STATUS_APPLYING ? tr("取消申请") : tr("申请更换"));
    }
    else
    {
        ui->UnloadRgntBtn->setVisible(true);
        ui->LoadRgntBtn->setText(tr("试剂装载"));
    }

    // 禁能相关按钮
    ui->RgntMaskBtn->setEnabled(false);
    ui->PriorUseBtn->setEnabled(false);
    ui->UnloadRgntBtn->setEnabled(false);
    ui->LoadRgntBtn->setEnabled(bMask ? IsEnableLoadRgnt(*stuTfDevInfo) : false);

    // 如果是免疫单机版，则试剂扫描和试剂混匀也受使能控制
    ui->MixBtn->setEnabled(IsExistDeviceCanMix());
    ui->RgntScanBtn->setEnabled(IsExistDeviceCanScan());
    ui->LoadPartResetBtn->setEnabled(IsExistDeviceCanResetLoader());

    // 默认文本
    ui->RgntMaskBtn->setText(tr("试剂屏蔽"));

    // 获取试剂盘选中索引，无效则返回
    int iSelIndex = m_pRgntPlate->GetSelIndex();
    if (iSelIndex <= 0 || iSelIndex > m_pRgntPlate->GetTubeSlotCount())
    {
        ULOG(LOG_INFO, "%s(), iSelIndex <= 0", __FUNCTION__);
        ui->LoadRgntBtn->setEnabled(IsEnableLoadRgnt(*stuTfDevInfo));
        im::tf::SuppliesInfoTable stuSelSplInfo = GetSelSplInfo();
        ui->PriorUseBtn->setEnabled(stuSelSplInfo.usingStatus == 2);
        ui->UnloadRgntBtn->setEnabled(IsEnableUnloadRgnt(*stuTfDevInfo));
        return;
    }

    // 获取选中试剂信息
    boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(iSelIndex);
    if (!stuInfo.has_value() || !stuInfo->userData.isValid())
    {
        ULOG(LOG_INFO, "%s(), stuRgntInfo == boost::none", __FUNCTION__);
        ui->LoadRgntBtn->setEnabled(IsEnableLoadRgnt(*stuTfDevInfo));
        ui->UnloadRgntBtn->setEnabled(IsEnableUnloadRgnt(*stuTfDevInfo));
        return;
    }

    // 扫描失败时直接返回
    if (!stuInfo->bDillute)
    {
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();
        ui->UnloadRgntBtn->setEnabled(IsEnableUnloadRgnt(*stuTfDevInfo, stuRgntInfo));
        if (stuRgntInfo.assayCode <= 0)
        {
            ULOG(LOG_INFO, "%s(), select scan failed reagent.", __FUNCTION__);
            return;
        }

        // 剩余测试数为0直接返回
        if (stuRgntInfo.residualTestNum <= 0 || stuRgntInfo.lLDError > 0)
        {
            ULOG(LOG_INFO, "%s(), select empty reagent.", __FUNCTION__);
            return;
        }
    }
    else
    {
        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();
        ui->UnloadRgntBtn->setEnabled(IsEnableUnloadDlt(*stuTfDevInfo, stuDltInfo));

        // 剩余测试数为0直接返回
        if (stuDltInfo.curVol <= 0 || stuDltInfo.LLDErr > 0)
        {
            ULOG(LOG_INFO, "%s(), select empty reagent.", __FUNCTION__);
            return;
        }
    }

    // 是识别到的试剂或稀释液
    ui->RgntMaskBtn->setEnabled(true);

    // 试剂和稀释液分开判断
    if (!stuInfo->bDillute)
    {
        // 获取对应的稀释液信息
        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();
        ui->PriorUseBtn->setEnabled((stuRgntInfo.reagStatusUI.status == im::tf::StatusForUI::REAGENT_STATUS_BACKUP) && !stuRgntInfo.caliCurveExpInfo.empty());
    }
    else
    {
        // 获取对应的稀释液信息
        im::tf::DiluentInfoTable stuDltInfo = stuInfo->userData.value<im::tf::DiluentInfoTable>();
        ui->PriorUseBtn->setEnabled(stuDltInfo.statusForUI.status == im::tf::StatusForUI::REAGENT_STATUS_BACKUP);
    }

    // 更新试剂屏蔽文本
    ui->RgntMaskBtn->setText(stuInfo->bMask ? tr("解除屏蔽") : tr("试剂屏蔽"));
}

///
/// @brief
///     试剂扫描对话框确认按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntPlateWidget::OnRgntScanDlgOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    ImRgntScanDlg* pImRgntScanDlg = qobject_cast<ImRgntScanDlg*>(sender());

    // 获取失败则返回
    if (pImRgntScanDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pImRgntScanDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;
    devInfoCond.__set_deviceSN(m_strDevSn.toStdString());

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
        return;
    }

    // 获取选中设备信息
    tf::DeviceInfo& stuTfDevInfo = devInfoResp.lstDeviceInfos[0];

    // 判断当前仪器状态是否允许扫描
    // 弹框禁止试剂扫描
    if (PopRgntScanForbiddenTip(stuTfDevInfo))
    {
        return;
    }

    // 判断是否选择扫描全部
    if (pImRgntScanDlg->ui->ScanAllRBtn->isChecked())
    {
        // 扫描所有位置
        // 空的位置代表扫描全部
        ::tf::ResultLong _return;
        std::vector<int> vecPos;
        bool bRet = im::LogicControlProxy::SingleMaintance(::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN, m_strDevSn.toStdString(), vecPos);
        if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "SetReagentScan failed.");
        }

        // 关闭对话框
        pImRgntScanDlg->accept();
        return;
    }

    // 解析位置文本
    auto ParsePosText = [](QString strPoxText, std::vector<int>& vecPos)
    {
        // 清空容器
        vecPos.clear();

        // 用集合去重并排序
        std::set<int> setPos;

        // 先移除空格
        strPoxText.remove(" ");

        // 根据逗号分隔
        QStringList strPosSectionList = strPoxText.split(",");
        for (const auto& strPosSection : strPosSectionList)
        {
            // 如果为空，直接跳过
            if (strPosSection.isEmpty())
            {
                continue;
            }

            // 如果为数字，直接转化为数字
            bool ok;
            int iPos = strPosSection.toInt(&ok);
            if (ok)
            {
                // 是数字并转换成功
                setPos.insert(iPos);
                continue;
            }

            // 不是数字，尝试解析数字段
            QStringList strPosList = strPosSection.split("-");
            if (strPosList.size() != 2)
            {
                return false;
            }

            // 解析数字段
            bool ok1, ok2;
            const QString& strStartPos = strPosList.first();
            const QString& strEndPos = strPosList.last();
            int iStartPos = strStartPos.toInt(&ok1);
            int iEndPos = strEndPos.toInt(&ok2);
            if (!ok1 || !ok2)
            {
                return false;
            }

            // 将数字段加入数组
            for (int secPos = qMin(iStartPos, iEndPos); secPos <= qMax(iStartPos, iEndPos); secPos++)
            {
                // 加入数组中
                setPos.insert(secPos);
            }
        }

        // 将集合拷贝到数组
        vecPos.insert(vecPos.end(), setPos.begin(), setPos.end());

        return true;
    };

    // 获取扫描特定位置
    std::vector<int> vecPos;
    if (!ParsePosText(pImRgntScanDlg->ui->SpecPosEdit->text(), vecPos))
    {
        // 弹框提示
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("无效的位置输入！")));
        pTipDlg->exec();
        return;
    }

    // 位置检查
    for (int iPos : vecPos)
    {
        if (iPos <= 0 || iPos > SLOT_CNT_OF_RGNT_PLATE)
        {
            // 弹框提示
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("无效的位置输入！")));
            pTipDlg->exec();
            return;
        }
    }

    // 通知试剂指定位置扫描开始
    SEND_MESSAGE(MSG_ID_REAGENT_SCAN_POS_START, m_strDevName, vecPos);

    // 扫描指定位置
    ::tf::ResultLong _return;
    bool bRet = im::LogicControlProxy::SingleMaintance(::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN_POS, m_strDevSn.toStdString(), vecPos);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "SetReagentScan failed.");
    }

    // 关闭对话框
    pImRgntScanDlg->accept();
}

///
/// @brief
///     添加反应杯按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntPlateWidget::OnAddReactBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 弹出添加反应杯对话框
    std::shared_ptr<ImAddReactCupDlg> spImAddReactCupDlg(new ImAddReactCupDlg(this));

    // 连接信号槽
    connect(spImAddReactCupDlg.get(), SIGNAL(SigBarcodeScanFinished()), this, SLOT(OnReactCupScanFinished()));

    // 弹出对话框
    if (spImAddReactCupDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "%s(), Canceled.", __FUNCTION__);
        return;
    }

    // 添加反应杯
    im::tf::SuppliesInfoTable stuSplInfo = spImAddReactCupDlg->GetScanSplData().value<im::tf::SuppliesInfoTable>();

    // 执行添加
    tf::ResultLong _return;
    bool bRet = ::im::LogicControlProxy::AddSupplyCup(_return, stuSplInfo);
    if (!bRet || _return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "AddSupplyCup failed.");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("反应杯信息存储失败，请检查反应杯条码！")));
        pTipDlg->exec();
        return;
    }

    // 更新耗材信息
    UpdateSplInfo();
}

///
/// @brief
///     反应杯扫描结束
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImRgntPlateWidget::OnReactCupScanFinished()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    ImAddReactCupDlg* pImAddReactCupDlg = qobject_cast<ImAddReactCupDlg*>(sender());
    if (pImAddReactCupDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), qobject_cast failed.", __FUNCTION__);
        return;
    }

    // 默认将提示置为扫描失败
    pImAddReactCupDlg->ui->ScanRltTipLab->setText(tr("信息读取失败！"));
    pImAddReactCupDlg->ui->ScanRltTipLab->setStyleSheet("color:red;font-size:16;");

    // 禁能确认按钮，清空耗材信息
    pImAddReactCupDlg->ui->OkBtn->setEnabled(false);
    pImAddReactCupDlg->ui->SplNameValLab->clear();
    pImAddReactCupDlg->ui->ResidualValLab->clear();
    pImAddReactCupDlg->ui->SplLotValLab->clear();
    pImAddReactCupDlg->ui->SplSnValLab->clear();
    pImAddReactCupDlg->ui->ExpDateValLab->clear();
    pImAddReactCupDlg->ui->RegisterTimeValLab->clear();
    pImAddReactCupDlg->SetScanSplData(QVariant());

    // 获取条码
    QString strSplBarcode = pImAddReactCupDlg->ui->BarcodeInputEdit->text();

    // 解析条码
    ::im::tf::ParseSupplyeCupResp CupRlt;
    bool bRet = ::im::LogicControlProxy::ParseSupplyCupInfo(CupRlt, strSplBarcode.toStdString(), m_strDevSn.toStdString());
    if (!bRet || CupRlt.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "ParseSupplyCupInfo failed.");
        return;
    }

    // 获取返回结果
    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::HasAdded)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("反应杯信息重复登记，请更换反应杯条码！")));
        pTipDlg->exec();
        return;
    }

    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::IsNotCup)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("不是反应杯条码。")));
        pTipDlg->exec();
        return;
    }

    if (CupRlt.scanResult == im::tf::ScanSupplyCupType::ParseBarcodeErr)
    {
        //         std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("解析反应杯条码失败。")));
        //         pTipDlg->exec();
        //         return;
    }

    if (CupRlt.scanResult != im::tf::ScanSupplyCupType::CanAdd)
    {
        ULOG(LOG_WARN, "can not add.");
        return;
    }

    // 允许添加
    // 更新提示
    pImAddReactCupDlg->ui->ScanRltTipLab->setText(tr("扫描结果成功！"));
    pImAddReactCupDlg->ui->ScanRltTipLab->setStyleSheet("color:green;font-size:16;");

    // 使能确认按钮，加载耗材信息
    pImAddReactCupDlg->ui->OkBtn->setEnabled(true);
    pImAddReactCupDlg->ui->SplNameValLab->setText(tr("反应杯"));
    pImAddReactCupDlg->ui->ResidualValLab->setText(QString::number(CupRlt.cup.remainVol) + tr("个"));
    pImAddReactCupDlg->ui->SplLotValLab->setText(QString::fromStdString(CupRlt.cup.supLot));
    pImAddReactCupDlg->ui->SplSnValLab->setText(QString::fromStdString(CupRlt.cup.supSerial));
    pImAddReactCupDlg->ui->ExpDateValLab->setText(ToCfgFmtDateTime(QString::fromStdString(CupRlt.cup.expiryDate)));
    pImAddReactCupDlg->ui->RegisterTimeValLab->setText(ToCfgFmtDateTime(QString::fromStdString(CupRlt.cup.recordDate)));

    // 缓存数据
    pImAddReactCupDlg->SetScanSplData(QVariant::fromValue<im::tf::SuppliesInfoTable>(CupRlt.cup));
}

///
/// @brief
///     试剂指定位置扫描开始
///
/// @param[in]  strDevName  设备名
/// @param[in]  vecPos      指定位置
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月22日，新建函数
///
void ImRgntPlateWidget::OnRgntScanPosStart(QString strDevName, std::vector<int, std::allocator<int>> vecPos)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    if (m_strDevName != strDevName)
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 扫面前卸载指定位置试剂
    for (int iPos : vecPos)
    {
        // 卸载指定位置试剂
        m_pRgntPlate->UnloadReagent(iPos);
    }
}

///
/// @brief
///     耗材被选中
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void ImRgntPlateWidget::OnSplChecked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    CheckableFrame* pCheckableFrame = qobject_cast<CheckableFrame*>(sender());
    if (pCheckableFrame == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pCheckableFrame == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 遍历选择器
    for (auto it = m_mapSplChecker.begin(); it != m_mapSplChecker.end(); it++)
    {
        if (it.key() == pCheckableFrame)
        {
            it.key()->SetChecked(true);
            continue;
        }

        // 其余耗材全部置为未选中
        it.key()->SetChecked(false);
    }

    // 试剂盘置为未选中
    m_pRgntPlate->SetSelIndex(0);

    // 更新按钮使能状态
    UpdateBtnStatus();
}

///
/// @brief
///     试剂仓加卸载是否正在运行
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  bRunning  是否正在运行
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月30日，新建函数
///
void ImRgntPlateWidget::OnRgntLoadStatusChanged(QString strDevSn, bool bRunning)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s(%s,%d)", __FUNCTION__, strDevSn.toStdString().c_str(), int(bRunning));
    // 如果设备不匹配则忽略
    if (m_strDevSn != strDevSn)
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    UpdateBtnStatus();
}

///
/// @brief
///     耗材管理设置改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  stuSplMng   耗材管理
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月27日，新建函数
///
void ImRgntPlateWidget::OnSplMngCfgChanged(QString strDevSn, ImDeviceOtherInfo stuSplMng)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(stuSplMng);
    // 如果设备不匹配则忽略
    if (m_strDevSn != strDevSn)
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    // 更新耗材使能
    UpdateSplEnabled();
}

///
/// @brief
///     设备状态改变
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
void ImRgntPlateWidget::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    // 如果设备不匹配则忽略
    if (m_strDevSn != QString::fromStdString(deviceInfo.deviceSN))
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    // 更新按钮状态
    UpdateBtnStatus();
}

///
/// @brief
///     设备温度异常状态改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  bErr        是否温度异常
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月30日，新建函数
///
void ImRgntPlateWidget::OnDevTemperatureErrChanged(QString strDevSn, bool bErr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(bErr);
    // 如果设备不匹配则忽略
    if (m_strDevSn != strDevSn)
    {
        ULOG(LOG_INFO, "%s(), ignore, device sn not match", __FUNCTION__);
        return;
    }

    // 更新按钮状态
    UpdateBtnStatus();
}

///
/// @brief
///     更新耗材使能状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月12日，新建函数
///
void ImRgntPlateWidget::UpdateSplEnabled()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 加载耗材使能信息
    LoadSplEnableInfo();

    // 隐藏停用的耗材，显示启用的耗材
    ui->BaseLiquidGrp1Frame->setVisible(m_stuSplEnableInfo.bBaseLiquidGrp1);
    ui->BaseLiquidGrp2Frame->setVisible(m_stuSplEnableInfo.bBaseLiquidGrp2);
    ui->CleanLiquid1Frame->setVisible(m_stuSplEnableInfo.bCleanBuffer1);
    ui->CleanLiquid2Frame->setVisible(m_stuSplEnableInfo.bCleanBuffer2);
    ui->WasteLiquidFrame->setVisible(m_stuSplEnableInfo.bWasteLiquid);

    // 更新显示
    // 分情况显示
    int iRow = 1;
    QMap<int, QList<QWidget*>>  mapRowVisibleWgts;
    QMap<int, int>              mapRowScale;
    if (m_stuSplEnableInfo.bBaseLiquidGrp1 || m_stuSplEnableInfo.bBaseLiquidGrp2)
    {
        QList<QWidget*> pBaseLiquidWgtLst;
        if (m_stuSplEnableInfo.bBaseLiquidGrp1)
        {
            pBaseLiquidWgtLst.push_back(ui->BaseLiquidGrp1Frame);
        }
        if (m_stuSplEnableInfo.bBaseLiquidGrp2)
        {
            pBaseLiquidWgtLst.push_back(ui->BaseLiquidGrp2Frame);
        }
        mapRowVisibleWgts.insert(iRow, pBaseLiquidWgtLst);
        mapRowScale.insert(iRow, 1);
        iRow++;
    }

    // 清洗缓冲液
    if (m_stuSplEnableInfo.bCleanBuffer1 || m_stuSplEnableInfo.bCleanBuffer2)
    {
        QList<QWidget*> pCleanLiquidWgtLst;
        if (m_stuSplEnableInfo.bCleanBuffer1)
        {
            pCleanLiquidWgtLst.push_back(ui->CleanLiquid1Frame);
        }
        if (m_stuSplEnableInfo.bCleanBuffer2)
        {
            pCleanLiquidWgtLst.push_back(ui->CleanLiquid2Frame);
        }
        mapRowVisibleWgts.insert(iRow, pCleanLiquidWgtLst);
        mapRowScale.insert(iRow, 1);
        iRow++;
    }

    // 如果已经只剩第三行了，反应杯、废料桶、废液桶只能显示在第三行
    if (iRow >= 3)
    {
        QList<QWidget*> pWgtLst;
        pWgtLst.push_back(ui->ReactionFrame);
        pWgtLst.push_back(ui->WasteSolidFrame);
        if (m_stuSplEnableInfo.bWasteLiquid)
        {
            pWgtLst.push_back(ui->WasteLiquidFrame);
        }
        mapRowVisibleWgts.insert(iRow, pWgtLst);
        mapRowScale.insert(iRow, 1);
    }
    else if (iRow >= 1)
    {
        QList<QWidget*> pWgtLst;
        pWgtLst.push_back(ui->ReactionFrame);
        pWgtLst.push_back(ui->WasteSolidFrame);
        mapRowVisibleWgts.insert(iRow, pWgtLst);
        if (m_stuSplEnableInfo.bWasteLiquid)
        {
            mapRowScale.insert(iRow, 3 - iRow);
            ++iRow;
            QList<QWidget*> pWasteLiquidWgtLst;
            pWasteLiquidWgtLst.push_back(ui->WasteLiquidFrame);
            mapRowVisibleWgts.insert(iRow, pWasteLiquidWgtLst);
            mapRowScale.insert(iRow, 1);
        }
        else
        {
            mapRowScale.insert(iRow, 4 - iRow);
        }
    }
    else
    {
        ULOG(LOG_ERROR, "%s(), iRow=%d", __FUNCTION__, iRow);
        return;
    }

    // 更新耗材控件显示
    UpdateSplUiByEnableMap(mapRowVisibleWgts, mapRowScale);
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月4日，新建函数
///
void ImRgntPlateWidget::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    // 界面不显示则忽略
    if (!isVisible())
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果设备序列号不匹配，则跳过
    auto it = mapUpdateInfo.find(m_strDevSn);
    if (it == mapUpdateInfo.end())
    {
        ULOG(LOG_INFO, "%s(), device sn is not matched, ignore", __FUNCTION__);
        return;
    }

    UpdateRgntPlate();
    UpdateSelRgntDetail();
}

///
/// @brief 响应申请试剂结果消息
///
/// @param[in]  deviceSN  设备编号
/// @param[in]  result    是否接受
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月11日，新建函数
/// @li 4170/TangChuXian，2024年1月18日，逻辑完善
///
void ImRgntPlateWidget::OnManualHandleReagResult(const QString& deviceSN, const int result)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 显示则刷新按钮使能
    if (this->isVisible() && (result != RGNT_CHANGE_RLT_ACCEPT))
    {
        UpdateBtnStatus();
    }
}

///
/// @brief	设备信息更新
///     
/// @par History:
/// @li 7951/LuoXin，2023年6月5日，新建函数
/// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
///
void ImRgntPlateWidget::UpdateDeviceStatus(tf::DeviceInfo deviceInfo)
{
    Q_UNUSED(deviceInfo);
    UpdateBtnStatus();
}

void ImRgntPlateWidget::GetRelatedReagentIndexs(const im::tf::ReagentInfoTable& reag, QSet<int> &relatedIndex)
{
    // 试剂槽位号从1开始
    for (int i = 1; i <= SLOT_CNT_OF_RGNT_PLATE; ++i)
    {
        const boost::optional<ImReagentInfo> stuInfo = m_pRgntPlate->GetReagentInfo(i);
        if (!stuInfo.has_value() || !stuInfo->userData.isValid() || stuInfo->bDillute)
        {
            continue;
        }

        im::tf::ReagentInfoTable stuRgntInfo = stuInfo->userData.value<im::tf::ReagentInfoTable>();
        if (stuRgntInfo.id == reag.id)
        {
            continue;
        }
        // 预处理试剂
        if (IsRelativeReagentIm(reag, stuRgntInfo))
        {
            relatedIndex.insert(i);
        }
    }
}

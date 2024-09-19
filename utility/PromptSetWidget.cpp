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
/// @file     PromptSetWidget.cpp
/// @brief    应用--系统--提示设置
///
/// @author   7951/LuoXin
/// @date     2023年5月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年5月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "PromptSetWidget.h"
#include "ui_PromptSetWidget.h" 
#include "AlarmMusicDlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "shared/QComDelegate.h"

#include "model/QAlarmReagentSetModel.h"
#include "model/QAlarmSupplySetModel.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/common/defs.h"
#include "src/thrift/track/gen-cpp/track_types.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/ResultStatusCodeManager.h"
#include "manager/DictionaryQueryManager.h"
#include <QMouseEvent>
#include <QFileDialog>
#include <QStandardItemModel>

#define  ALARM_BTN_PROPERTY_NAME        ("playing") //  当前音频是否正在播放

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return;\
}

// 报警提示表格各信息所在列
enum AlarmPromptColunm
{
    AP_COLUNM_CLASSIFY,		        // 分类
    AP_COLUNM_TYPE,                 // 设备型号
    AP_COLUNM_CODE,			        // 报警码
    AP_COLUNM_NAME,				    // 名称
    AP_COLUNM_SOFTWARE_ENABLE		// 软件报警
};

// 数据报警表格各信息所在列
enum ResultStatusColunm
{
    SP_COLUNM_ALARM_CODE = 0,		// 结果状态报警码
    SP_COLUNM_DESCRIBTION,			// 描述
    SP_COLUNM_SELECT,				// 勾选框
    SP_COLUNM_ID					// 数据库主键
};

PromptSetWidget::PromptSetWidget(QWidget *parent)
    : QWidget(parent)
    , m_pResultStatusModel(new QStandardItemModel())
    , m_pAlarmCodeModel(new QColorHeaderModel())
    , m_pAlarmMusicDlg(nullptr)
    , m_bInit(false)
{
    ui = new Ui::PromptSetWidget();
    ui->setupUi(this);

    // 非生化时，隐藏生化特有配置
    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type != CHEMISTRY && type != CHEMISTRY_AND_IMMUNE)
    {
        ui->im_rabtn->setChecked(true);
    }

    ui->rabtn_group->setVisible(false);

    ui->stop_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
    ui->sample_stop_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
    ui->attention_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);

    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [&](QMediaPlayer::MediaStatus status)
    {
        if (status != QMediaPlayer::EndOfMedia)
        {
            return;
        }

        ui->stop_sound_btn->setText(tr("试听"));
        ui->sample_stop_sound_btn->setText(tr("试听"));
        ui->attention_sound_btn->setText(tr("试听"));

        ui->stop_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
        ui->sample_stop_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
        ui->attention_sound_btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
    });

    ui->tabWidget->tabBar()->installEventFilter(this);
}

PromptSetWidget::~PromptSetWidget()
{
}

bool PromptSetWidget::isExistChangedData()
{
    bool hasChange = false;
    QString pageName = ui->tabWidget->currentWidget()->objectName();

    // 判断当前页面是否由未保存的信息
    if (pageName == "tab_result_prompt")
    {
        hasChange = ExistUnSaveResultPromptConfig();
    }
    else if (pageName == "tab_alarm")
    {
        // 待删除列表
        std::vector<::tf::DisableAlarmCode> deleteCodes;
        // 待添加列表
        std::vector<::tf::DisableAlarmCode> addCodes;
        GetModifyAlarmPromptConfig(deleteCodes, addCodes);

        hasChange = !deleteCodes.empty() || !addCodes.empty();
    }
    else if (pageName == "reagent_set")
    {
        hasChange = QAlarmReagentSetModel::Instance().isExistChangedData();
    }
    else if (pageName == "supply_set")
    {
        hasChange = QAlarmSupplySetModel::Instance().isExistChangedData();
    }
    else if (pageName == "waste_set")
    {
        hasChange = QAlarmWasteSetModel::Instance().isExistChangedData();
    }
    else if (pageName == "vol_set")
    {
        hasChange = ExistUnSaveAlarmMusicConfig();
    }
    else if (pageName == "result_status")
    {
        hasChange = ExistUnSaveResultStatusConfig();
    }
    else if (pageName == "cali_prompt")
    {
        hasChange = ExistUnSaveCaliLineExpire();
    }
    
    return hasChange;
}

bool PromptSetWidget::eventFilter(QObject *obj, QEvent *event)
{
    // 切换当前页或者返回主页面
    if (obj == ui->tabWidget->tabBar() && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *>(event);
        int clickedTabIndex = ui->tabWidget->tabBar()->tabAt(mouseEvent->pos());

        // 点击当前页不提示
        if (clickedTabIndex == ui->tabWidget->currentIndex())
        {
            return true;
        }
        
        if (isExistChangedData()
            && TipDlg(tr("当前页面存在未保存的信息，切换页面后将丢失这些信息，是否切换页面"),
                TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
        {
            event->ignore();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void PromptSetWidget::OnSaveParameter()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    QString pageName = ui->tabWidget->currentWidget()->objectName();

    if (pageName == "tab_result_prompt")
    {
        // 保存结果提示设置
        SaveResultPromptConfig();
    } 
    else if(pageName == "tab_alarm")
    {
        // 保存报警提示设置
        SaveAlarmPromptConfig();
    }
    else if (pageName == "reagent_set")
    {
		bool hasChange = false, hasSupply = false;
        // 保存试剂参数
        QAlarmReagentSetModel::Instance().SaveAlarmParameter(hasChange,hasSupply);

		if (hasChange)
		{
			// 试剂提醒更新
			POST_MESSAGE(MSG_ID_RGNT_ALARM_VOL_UPDATE);
		}

		// 由于有些耗材是在试剂提示中，当改变的是耗材时需通知耗材提醒值更新
		if (hasSupply)
		{
			POST_MESSAGE(MSG_ID_SPL_ALARM_VAL_UPDATE);
		}

        TipDlg(tr("提示"), tr("保存成功")).exec();
    }
    else if (pageName == "supply_set")
    {
		bool hasChange = false;
        // 保存耗材参数
        QAlarmSupplySetModel::Instance().SaveAlarmParameter(hasChange);
		if (hasChange)
		{
			// 耗材提醒更新
			POST_MESSAGE(MSG_ID_SPL_ALARM_VAL_UPDATE);
		}

        TipDlg(tr("提示"), tr("保存成功")).exec();
    }
    else if (pageName == "waste_set")
    {
        // 保存废液桶参数
        QAlarmWasteSetModel::Instance().SaveAlarmParameter();
        POST_MESSAGE(MSG_ID_SPL_ALARM_VAL_UPDATE);
        TipDlg(tr("提示"), tr("保存成功")).exec();
    }
    else if (pageName == "vol_set")
    {
        // 保存报警提示音配置信息
        SaveAlarmMusicConfig();
    }
    else if (pageName == "result_status")
    {
        // 保存结果状态配置信息
        SaveResultStatusConfig();
    }
    else if (pageName == "cali_prompt")
    {
        // 保存校准提示配置信息
        SaveCaliLineExpire();
    }
}

void PromptSetWidget::OnMusicManageBtnClicked()
{
    if (m_pAlarmMusicDlg == nullptr)
    {
        m_pAlarmMusicDlg = new AlarmMusicDlg(this);
    }

    m_pAlarmMusicDlg->UpdateMusicName(m_listAlarmMusic);
    m_pAlarmMusicDlg->exec();
    
    RefreshAlarmMusicFiles();
}

void PromptSetWidget::OnSoundClicked()
{
    // 被点击的按钮
    QPushButton* btn = qobject_cast<QPushButton *>(sender());

    if (btn->property(ALARM_BTN_PROPERTY_NAME).toBool())
    {
        m_player.stop();
        btn->setText(tr("试听"));
        btn->setProperty(ALARM_BTN_PROPERTY_NAME, false);
        return;
    }

    // 获取对应的勾选框和滑动条
    QComboBox*	coBox;
    QSlider*	slider;
    if (btn->objectName() == "stop_sound_btn")
    {
        coBox = ui->stop_sound_cbox;
        slider = ui->stop_sound_hslider;
    }
    else if(btn->objectName() == "sample_stop_sound_btn")
    {
        coBox = ui->sample_stop_sound_cbox;
        slider = ui->sample_stop_sound_hslider;
    }
    else
    {
        coBox = ui->attention_sound_cbox;
        slider = ui->attention_sound_hslider;
    }

    // 判断是否选择音频
    if (coBox->currentIndex() == -1)
    {
        TipDlg(tr("提示"), tr("请选择试听文件")).exec();
        return;
    }

    QFile file(QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR + coBox->currentText());
    if (!file.exists())
    {
        TipDlg(tr("提示"), tr("当前选择的文件不存在")).exec();
        RefreshAlarmMusicFiles();
        return;
    }

    // 播放音频
    m_player.stop();
    m_player.setMedia(QUrl::fromLocalFile(QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR + coBox->currentText()));
    m_player.setVolume(slider->value());
    m_player.play();

    for (auto pb : ui->vol_set->findChildren<QPushButton*>())
    {
        pb->setText(tr("试听"));
        pb->setProperty(ALARM_BTN_PROPERTY_NAME, false);
    }

    btn->setText(tr("停止"));
    btn->setProperty(ALARM_BTN_PROPERTY_NAME, true);

}

void PromptSetWidget::OnValueChanged(int value)
{
    QString btnName = qobject_cast<QSlider*>(sender())->objectName().replace("hslider", "btn");
    QPushButton* btn = ui->vol_set->findChild<QPushButton*>(btnName);

    if (!btn->property(ALARM_BTN_PROPERTY_NAME).toBool()) 
    {
        return;
    }

    m_player.setVolume(value);
}

void PromptSetWidget::OnTabWidgetChangedPage(int index)
{
    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
    ui->flat_music->setVisible(ui->tabWidget->currentWidget()->objectName() == "vol_set");
    ui->rabtn_group->setVisible(type == CHEMISTRY_AND_IMMUNE && ui->tabWidget->currentWidget()->objectName() == "result_status");

    if (ui->tabWidget->currentWidget()->objectName() == "tab_result_prompt")
    {
        LoadResultPromptConfig();
    }

    if (ui->tabWidget->currentWidget()->objectName() == "tab_alarm")
    {
        LoadAlarmPromptConfig();
    }

    if (ui->tabWidget->currentWidget()->objectName() == "reagent_set")
    {
        QAlarmReagentSetModel::Instance().Update();

        // 还原默认顺序
        ui->reagent_view->sortByColumn(-1, Qt::DescendingOrder);
        m_pReagentSortModel->sort(-1, Qt::DescendingOrder);
    }

    if (ui->tabWidget->currentWidget()->objectName() == "supply_set")
    {
        QAlarmSupplySetModel::Instance().Update();

        // 还原默认顺序
        ui->supply_view->sortByColumn(-1, Qt::DescendingOrder);
        m_pSupplySortModel->sort(-1, Qt::DescendingOrder);
    }

    if (ui->tabWidget->currentWidget()->objectName() == "waste_set")
    {
        QAlarmWasteSetModel::Instance().Update();

        // 还原默认顺序
        ui->waste_view->sortByColumn(-1, Qt::DescendingOrder);
        m_pWasteSortModel->sort(-1, Qt::DescendingOrder);
    }

    if (ui->tabWidget->currentWidget()->objectName() == "vol_set")
    {
        RefreshAlarmMusicFiles();
    }

    if (ui->tabWidget->currentWidget()->objectName() == "result_status")
    {
        LoadResultStatusConfig();

        // 还原默认顺序
        ui->result_view->sortByColumn(-1, Qt::DescendingOrder);
        m_pDataAlarmSortModel->sort(-1, Qt::DescendingOrder);
    }
    if (ui->tabWidget->currentWidget()->objectName() == "cali_prompt")
    {
        bool ce = false;
        if (!DictionaryQueryManager::GetCaliLineExpire(ce))
        {
            ULOG(LOG_ERROR, "Failed to get cali line expire.");
            ReportErr(true, tr("保存失败"));
        }

        ui->cali_line_ckbox->setChecked(ce);
    }
}

void PromptSetWidget::UpdateCtrlsEnabled()
{    
	// 未登录时直接返回
    SEG_NO_LOGIN_RETURN;
	
    std::shared_ptr<UserInfoManager> insUser = UserInfoManager::GetInstance();
    ui->flat_save->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_RESULT));

    // 结果提示
    ui->tab_result_prompt->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_RESULT));
    // 报警提示
    ui->tab_alarm->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_ARLARM));
    // 试剂提示
    ui->reagent_set->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_REAGENT));
    // 耗材提示
    ui->supply_set->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_SUPPLY));
    // 废液废料提示
    ui->waste_set->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_WASTE));
    // 报警音
    ui->vol_set->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_ALARMVOCIE));
    // 数据报警
    ui->result_status->setEnabled(insUser->IsPermisson(PSM_IM_NOTICESET_DATAALARM));
    // 校准提示
    ui->cali_prompt->setEnabled(insUser->IsPermisson(PSM_NOTICESET_CALI));
}

void PromptSetWidget::Init()
{
    // 报警提示
    InitAlarmView();

    // 数据报警
    InitResultView();

    // 隐藏音乐管理按钮
    ui->flat_music->hide();

    // 试剂提示
    m_pReagentSortModel = new QUtilitySortFilterProxyModel(this);
    m_pReagentSortModel->setSourceModel(&QAlarmReagentSetModel::Instance());
    ui->reagent_view->setModel(m_pReagentSortModel);

    m_pReagentSortModel->SetTableViewSort(ui->reagent_view, { (int)QAlarmReagentSetModel::COL::MODEL,
        (int)QAlarmReagentSetModel::COL::NAME,(int)QAlarmReagentSetModel::COL::UNIT });
    //,(int)QAlarmReagentSetModel::COL::BOTTLEWARNING,
        //(int)QAlarmReagentSetModel::COL::ASSAYWARNING,

	ui->reagent_view->setColumnWidth(0, 379);
	ui->reagent_view->setColumnWidth(1, 379);
	ui->reagent_view->setColumnWidth(2, 379);
	ui->reagent_view->setColumnWidth(3, 379);
	ui->reagent_view->horizontalHeader()->setStretchLastSection(true);
	ui->reagent_view->horizontalHeader()->setMinimumSectionSize(100);

    // 设置耗材页面
    m_pSupplySortModel = new QUtilitySortFilterProxyModel(this);
    m_pSupplySortModel->setSourceModel(&QAlarmSupplySetModel::Instance());
    ui->supply_view->setModel(m_pSupplySortModel);

    m_pSupplySortModel->SetTableViewSort(ui->supply_view, { (int)QAlarmSupplySetModel::COL::MODEL, (int)QAlarmSupplySetModel::COL::NAME,
        (int)QAlarmSupplySetModel::COL::UNIT });
    //(int)QAlarmSupplySetModel::COL::ASSAYWARNING, 

    // 设置行选中
    ui->supply_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->supply_view->setColumnWidth(0, 473);
	ui->supply_view->setColumnWidth(1, 473);
	ui->supply_view->setColumnWidth(2, 473);
	ui->supply_view->horizontalHeader()->setStretchLastSection(true);
	ui->supply_view->horizontalHeader()->setMinimumSectionSize(100);

    // 设置废液信息
    m_pWasteSortModel = new QUtilitySortFilterProxyModel(this);
    m_pWasteSortModel->setSourceModel(&QAlarmWasteSetModel::Instance());
    ui->waste_view->setModel(m_pWasteSortModel);

    m_pWasteSortModel->SetTableViewSort(ui->waste_view, { (int)QAlarmWasteSetModel::COL::MODEL, 
        (int)QAlarmWasteSetModel::COL::NAME,(int)QAlarmWasteSetModel::COL::WARNING, (int)QAlarmWasteSetModel::COL::UNIT });
    
    // 设置行选中
    ui->waste_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->waste_view->setColumnWidth(0, 473);
	ui->waste_view->setColumnWidth(1, 473);
	ui->waste_view->setColumnWidth(2, 473);
	ui->waste_view->horizontalHeader()->setStretchLastSection(true);
	ui->waste_view->horizontalHeader()->setMinimumSectionSize(100);

    // 试听停机音乐
    connect(ui->stop_sound_btn, SIGNAL(clicked()), this, SLOT(OnSoundClicked()));
    // 试听暂停音乐
    connect(ui->sample_stop_sound_btn, SIGNAL(clicked()), this, SLOT(OnSoundClicked()));
    // 试听提示音
    connect(ui->attention_sound_btn, SIGNAL(clicked()), this, SLOT(OnSoundClicked()));
    // 音乐管理按钮
    connect(ui->flat_music, SIGNAL(clicked()), this, SLOT(OnMusicManageBtnClicked()));
    // 保存参数按钮
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveParameter()));
    // 报警提示设置筛选按钮
    connect(ui->alarm_query_btn, &QPushButton::clicked, this, [=] 
    {
        LoadAlarmPromptConfig();
    });

    // slider调节音量
    connect(ui->stop_sound_hslider, &QSlider::valueChanged, this, &PromptSetWidget::OnValueChanged);
    connect(ui->sample_stop_sound_hslider, &QSlider::valueChanged, this, &PromptSetWidget::OnValueChanged);
    connect(ui->attention_sound_hslider, &QSlider::valueChanged, this, &PromptSetWidget::OnValueChanged);

    // 刷新
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &PromptSetWidget::OnTabWidgetChangedPage);

    // 重置按钮
    connect(ui->reset_btn, &QPushButton::clicked, this, [&]
    {
        ui->alarm_classifty_combox->setCurrentIndex(0);
        ui->lineEdit->setText("");
        ui->alarm_name_edit->setText("");
        LoadAlarmPromptConfig();
    });

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

void PromptSetWidget::InitAlarmView()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 分类下拉框初始化
    QStringList itemNameList({ tr("全部"), tr("控制单元") });

    auto CIM = CommonInformationManager::GetInstance();
    auto type = CIM->GetSoftWareType();
    if (type == IMMUNE)
    {
        itemNameList.push_back(tr("免疫"));
    } 
    else if (type == CHEMISTRY)
    {
        itemNameList.push_back(tr("生化"));
    }
    else if (type == CHEMISTRY_AND_IMMUNE)
    {
        itemNameList.push_back(tr("免疫"));
        itemNameList.push_back(tr("生化"));
    }

    if (CIM->HasIseDevice())
    {
        itemNameList.push_back(tr("ISE"));
    }

    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        itemNameList.push_back(tr("轨道"));
    }

    ui->alarm_classifty_combox->addItems(itemNameList);

    // 表头
    m_pAlarmCodeModel->setHorizontalHeaderLabels({ tr("分类"),tr("设备型号"), 
            tr("报警代码"), tr("报警名称"), tr("软件报警") });

    // 排序
    m_pAlarmCodeSortModel = new QUtilitySortFilterProxyModel(this);
    m_pAlarmCodeSortModel->setSourceModel(m_pAlarmCodeModel);
    ui->alarm_view->setModel(m_pAlarmCodeSortModel);
    m_pAlarmCodeSortModel->SetTableViewSort(ui->alarm_view, { AP_COLUNM_CLASSIFY,AP_COLUNM_CODE,AP_COLUNM_NAME });

    // 还原默认顺序
    ui->alarm_view->sortByColumn(-1, Qt::DescendingOrder);
    m_pAlarmCodeSortModel->sort(-1, Qt::DescendingOrder);

	ui->alarm_view->setColumnWidth(AP_COLUNM_CLASSIFY, 473);
	ui->alarm_view->setColumnWidth(AP_COLUNM_CODE, 473);
	ui->alarm_view->setColumnWidth(AP_COLUNM_NAME, 473);
	ui->alarm_view->horizontalHeader()->setStretchLastSection(true);
	ui->alarm_view->horizontalHeader()->setMinimumSectionSize(100);

    // 勾选代理
    ui->alarm_view->setItemDelegateForColumn(AP_COLUNM_SOFTWARE_ENABLE, new CheckBoxDelegate(this));
    ui->alarm_view->hideColumn(AP_COLUNM_TYPE);

    // 勾选框
    connect(ui->alarm_view, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
    {
        if (index.column() == AP_COLUNM_SOFTWARE_ENABLE)
        {
            m_pAlarmCodeModel->item(index.row(), index.column())->setData(
                !m_pAlarmCodeModel->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
        }
    });

    // 设置表头颜色
    m_pAlarmCodeModel->SetColunmColor(AP_COLUNM_SOFTWARE_ENABLE, UI_HEADER_FONT_COLOR);

    // 将报警码加载到表格
    UpdateAlarmView();
}

void PromptSetWidget::UpdateAlarmView()
{
    m_pAlarmCodeModel->removeRows(0,m_pAlarmCodeModel->rowCount());

    std::vector<::tf::AlarmPrompt> aps;
    DcsControlProxy::GetInstance()->GetAllAlarmPrompt(aps);
    const auto& CIM = CommonInformationManager::GetInstance();
    auto type = CIM->GetSoftWareType();
    int row = 0;
    for (auto& ap : aps)
    {
        QString DevClassify = GetDevClassifyByDevType(ap.deviceType);

        // 仅生化，显示生化和控制单元
        if (type == CHEMISTRY && 
            ((DevClassify == tr("控制单元") && ap.mainCode == 92)
                    ||(DevClassify != tr("生化") && DevClassify != "ISE" && DevClassify != tr("控制单元"))))   // 92号段的是免疫报警码不显示
        {
            continue;
        }
        //  仅免疫，显示生化和控制单元
        else if (type == IMMUNE
            && ((DevClassify != tr("控制单元") && ap.mainCode == 93)
                    || DevClassify != tr("免疫") && DevClassify != tr("控制单元")))                           // 93号段的是生化报警码不显示
        {
            continue;
        }

        // 存在ISE才显示ISE的报警
        if (!CIM->HasIseDevice() && DevClassify == "ISE")
        {
            continue;
        }

        QString code = UiCommon::AlarmCodeToString(ap.mainCode, ap.middleCode, ap.subCode);
        QString name = QString::fromStdString(ap.name);

        // 单机不显示轨道
        if (!DictionaryQueryManager::GetInstance()->GetPipeLine()
            && ap.deviceType >= ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        AddTextToTableView(m_pAlarmCodeModel, row, AP_COLUNM_CLASSIFY, DevClassify);
        AddTextToTableView(m_pAlarmCodeModel, row, AP_COLUNM_TYPE, QString::number(ap.deviceType));
        AddTextToTableView(m_pAlarmCodeModel, row, AP_COLUNM_CODE, code);
        AddTextToTableView(m_pAlarmCodeModel, row, AP_COLUNM_NAME, name);
        AddTextToTableView(m_pAlarmCodeModel, row, AP_COLUNM_SOFTWARE_ENABLE, "");
        m_pAlarmCodeModel->item(row, AP_COLUNM_SOFTWARE_ENABLE)->setData(true, Qt::UserRole + 1);
        row++;
    }

    // 还原默认顺序
    ui->alarm_view->sortByColumn(-1, Qt::DescendingOrder);
    m_pAlarmCodeSortModel->sort(-1, Qt::DescendingOrder);
}

void PromptSetWidget::InitResultView()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 设置结果状态表格表头
    QStringList headerListString;
    headerListString << tr("数据报警") << tr("描述") << tr("启用") << tr("id");
    m_pResultStatusModel->setHorizontalHeaderLabels(headerListString);
    m_pResultStatusModel->setHeaderData(2, Qt::Horizontal, UI_TEXT_COLOR_HEADER_MARK, Qt::ForegroundRole);

    // 排序
    m_pDataAlarmSortModel = new QUtilitySortFilterProxyModel(this);
    m_pDataAlarmSortModel->setSourceModel(m_pResultStatusModel);
    ui->result_view->setModel(m_pDataAlarmSortModel);
    m_pDataAlarmSortModel->SetTableViewSort(ui->result_view, { SP_COLUNM_ALARM_CODE,SP_COLUNM_DESCRIBTION});

    // 勾选框代理
    ui->result_view->setItemDelegateForColumn(SP_COLUNM_SELECT, new CheckBoxDelegate(this));
    ui->result_view->horizontalHeader()->setStretchLastSection(true);

    ui->result_view->hideColumn(SP_COLUNM_ID);

    // 勾选框
    connect(ui->result_view, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
    {
        if (index.column() == SP_COLUNM_SELECT)
        {
            m_pResultStatusModel->item(index.row(), index.column())->setData(!m_pResultStatusModel->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
        }
    });

    connect(ui->ch_rabtn, &QRadioButton::toggled, this, [&] { LoadResultStatusConfig(); });
}

QString PromptSetWidget::GetDevClassifyByDevType(int devType)
{
    switch (devType)
    {
        case ::tf::DeviceType::DEVICE_TYPE_ISE1005:
            return "ISE";
        case ::tf::DeviceType::DEVICE_TYPE_C1000:
        case ::tf::DeviceType::DEVICE_TYPE_C200:
            return tr("生化");
        case ::tf::DeviceType::DEVICE_TYPE_I6000:
            return tr("免疫");
        case ::tf::DeviceType::DEVICE_TYPE_INVALID:
            return tr("控制单元");
        case ::tf::DeviceType::DEVICE_TYPE_U3000:
            return tr("尿液");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_IOM01:
            return tr("进样回收模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_ISE:
            return tr("ISE运输加样模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_ST04:
            return tr("免疫运输加样模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_ST05:
            return tr("生化运输加样模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_RWC:
            return tr("复查缓存模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_DQI:
            return tr("视觉识别模块");
        case ::tf::DeviceType::DEVICE_TYPE_TRACK_6008_SOFTWARE:
            return tr("轨道控制系统软件");
        default:
            return tr("");
    }
}

void PromptSetWidget::RefreshAlarmMusicFiles()
{
    ui->stop_sound_cbox->clear();
    ui->sample_stop_sound_cbox->clear();
    ui->attention_sound_cbox->clear();

    QDir dir(QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR);
    m_listAlarmMusic.clear();
    if (!dir.exists())
    {
        return;
    }

    // 设置过滤规则
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    if (infoList.empty())
    {
        return;
    }

    // 遍历
    for (auto info : infoList)
    {
        m_listAlarmMusic.push_back(info.fileName());
    }

    // 设置下拉框
    ui->stop_sound_cbox->insertItems(0, m_listAlarmMusic);
    ui->sample_stop_sound_cbox->insertItems(0, m_listAlarmMusic);
    ui->attention_sound_cbox->insertItems(0, m_listAlarmMusic);

    LoadAlarmMusicConfig();
}

void PromptSetWidget::LoadResultPromptConfig()
{
    ResultPrompt rp = DictionaryQueryManager::GetInstance()->GetResultTips();

    ui->out_range_ckbox->setChecked(rp.enabledOutRange);
    ui->mark_first_rabtn->setChecked(rp.outRangeMarkType == 1);
    ui->mark_second_rabtn->setChecked(rp.outRangeMarkType == 2);
    ui->critical_ckbox->setChecked(rp.enabledCritical);
}

void PromptSetWidget::LoadAlarmPromptConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 查询禁用的报警码
    ::tf::DisableAlarmCode dac;
    ::tf::DisableAlarmCodeQueryResp dacqr;
    DcsControlProxy::GetInstance()->QueryDisableAlarmCode(dacqr, dac);
    if (dacqr.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDisableAlarmCode Failed!");
        return;
    }

    m_vecDisableAlarmCode.assign(dacqr.lstDisableAlarmCode.begin(), dacqr.lstDisableAlarmCode.end());

    QString filterType = ui->alarm_classifty_combox->currentText();
    QString filterCode = ui->lineEdit->text();
    QString filterName = ui->alarm_name_edit->text();

    for (int row = 0; row < m_pAlarmCodeModel->rowCount(); row++)
    {
        // 隐藏不符合筛选的设备类型的行
        QString rowDevType = m_pAlarmCodeModel->index(row, AP_COLUNM_CLASSIFY).data().toString();
        if (filterType != tr("全部") && rowDevType != filterType)
        {
            ui->alarm_view->setRowHidden(row, true);
            continue;
        }

        // 隐藏不符合筛选的报警代码的行
        QString rowCodeStr = m_pAlarmCodeModel->index(row, AP_COLUNM_CODE).data().toString();
        if (!filterCode.isEmpty() && !rowCodeStr.contains(filterCode))
        {
            ui->alarm_view->setRowHidden(row, true);
            continue;
        }

        // 隐藏不符合筛选的报警名称的行
        QString rowNameStr = m_pAlarmCodeModel->index(row, AP_COLUNM_NAME).data().toString();
        if (!filterName.isEmpty() && !rowNameStr.contains(filterName))
        {
            ui->alarm_view->setRowHidden(row, true);
            continue;
        }

        // 显示符合筛选条件的行
        ui->alarm_view->setRowHidden(row, false);
        auto codeList = rowCodeStr.split("-");
        m_pAlarmCodeModel->item(row, AP_COLUNM_SOFTWARE_ENABLE)->setData(true, Qt::UserRole + 1);
        int devType = m_pAlarmCodeModel->item(row, AP_COLUNM_TYPE)->data(Qt::DisplayRole).toInt();
        for (auto& item : m_vecDisableAlarmCode)
        {
            if (devType == item.deviceType
                && codeList[0].toInt() == item.mainCode
                && codeList[1].toInt() == item.middleCode
                && codeList[2].toInt() == item.subCode)
            {
                m_pAlarmCodeModel->item(row, AP_COLUNM_SOFTWARE_ENABLE)->setData(false, Qt::UserRole + 1);
                break;
            }
        }
    }
}

void PromptSetWidget::LoadAlarmMusicConfig()
{
    if (m_listAlarmMusic.empty())
    {
        ULOG(LOG_ERROR, "Alarm Music File Not exists !");
        return;
    }

    AlarmMusic am;
    if (!DictionaryQueryManager::GetAlarmMusicConfig(am))
    {
        ULOG(LOG_ERROR, "Failed to get alaram music config.");
        return;
    }

    // 配置的停机音乐文件是否存在
    if (m_listAlarmMusic.count(QString::fromStdString(am.strStopSoundName)) <= 0)
    {
        TipDlg(tr("初始化异常"), tr("默认停止级别告警音乐不存在，请导入停止音乐。"));
        ULOG(LOG_ERROR, "The Music File  Not exists !");
    }
    else
    {
        ui->stop_sound_cbox->setCurrentText(QString::fromStdString(am.strStopSoundName));
    }

    // 配置的停机音乐文件是否存在
    if (m_listAlarmMusic.count(QString::fromStdString(am.strSampleStopSoundName)) <= 0)
    {
        TipDlg(tr("初始化异常"), tr("默认暂停级别告警音乐不存在，请导入暂停音乐。"));
        ULOG(LOG_ERROR, "The Music File  Not exists !");
    }
    else
    {
        ui->sample_stop_sound_cbox->setCurrentText(QString::fromStdString(am.strSampleStopSoundName));
    }

    // 配置的注意音乐文件是否存在
    if (m_listAlarmMusic.count(QString::fromStdString(am.strAttentionSoundName)) <= 0)
    {
        TipDlg(tr("初始化异常"), tr("默认注意级别告警音乐不存在，请导入告警音乐。"));
        ULOG(LOG_ERROR, "The Music File  Not exists !");
    }
    else
    {
        ui->attention_sound_cbox->setCurrentText(QString::fromStdString(am.strAttentionSoundName));
    }

    // 设置音量
    ui->stop_sound_hslider->setValue(am.iStopSoundVolume);
    ui->sample_stop_sound_hslider->setValue(am.iSampleStopSoundVolume);
    ui->attention_sound_hslider->setValue(am.iAttentionSoundVolume);
}

void PromptSetWidget::LoadResultStatusConfig()
{
    m_pResultStatusModel->removeRows(0, m_pResultStatusModel->rowCount());

    if (ui->ch_rabtn->isChecked())
    {
        LoadChResultStatusConfig();
    }
    else
    {
        LoadImResultStatusConfig();
    }
}

void PromptSetWidget::LoadChResultStatusConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    std::vector<::tf::DataAlarmItem> dataAlarmItems;
    DcsControlProxy::GetInstance()->QueryDataAlarm(dataAlarmItems, "ALL");
    auto codes = DictionaryQueryManager::GetChResultStatusCodes();

    int iRow = 0;
    for (auto& item : dataAlarmItems)
    {
        if (!item.enableIse && !item.enableCh)
        {
            continue;
        }

        // 结果状态报警码
        {
            auto si = new QStandardItem(QString::fromStdString(item.code));
            si->setTextAlignment(Qt::AlignCenter);
            m_pResultStatusModel->setItem(iRow, SP_COLUNM_ALARM_CODE, si);
        }

        // 勾选框
        {
            auto si = new QStandardItem("");
            si->setData(!codes.contains(QString::fromStdString(item.code)), Qt::UserRole + 1);
            m_pResultStatusModel->setItem(iRow, SP_COLUNM_SELECT, si);
        }

        // 描述
        m_pResultStatusModel->setItem(iRow, SP_COLUNM_DESCRIBTION, new QStandardItem(QString::fromStdString(item.name)));

        iRow++;
    }

    // 默认选中第一行
    ui->result_view->selectRow(0);

    ui->result_view->setColumnWidth(0, 460);
    ui->result_view->setColumnWidth(1, 1040);
    ui->result_view->setColumnWidth(2, 360);
	ui->result_view->horizontalHeader()->setStretchLastSection(true);
	ui->result_view->horizontalHeader()->setMinimumSectionSize(100);
}

void PromptSetWidget::LoadImResultStatusConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    std::vector<::tf::DataAlarmItem> dataAlarmItems;
    DcsControlProxy::GetInstance()->QueryDataAlarm(dataAlarmItems, "ALL");
    auto codes = DictionaryQueryManager::GetChResultStatusCodes();

    int iRow = 0;
    for (auto& item : dataAlarmItems)
    {
        if (!item.enableIm)
        {
            continue;
        }

        // 结果状态报警码
        {
            auto si = new QStandardItem(QString::fromStdString(item.code));
            si->setTextAlignment(Qt::AlignCenter);
            m_pResultStatusModel->setItem(iRow, SP_COLUNM_ALARM_CODE, si);
        }

        // 勾选框
        {
            auto si = new QStandardItem("");
            im::tf::ResultCodeSet resSet;
            bool display = false;
            if (ResultStatusCodeManager::GetInstance()->GetResStatusCodeSet(item.code, resSet))
            {
                display = resSet.isDisplay;
            }           
            si->setData(display, Qt::UserRole + 1);
            m_pResultStatusModel->setItem(iRow, SP_COLUNM_SELECT, si);
        }

        // 描述
        m_pResultStatusModel->setItem(iRow, SP_COLUNM_DESCRIBTION, new QStandardItem(QString::fromStdString(item.name)));

        iRow++;
    }

    // 默认选中第一行
    ui->result_view->selectRow(0);

    ui->result_view->setColumnWidth(0, 460);
    ui->result_view->setColumnWidth(1, 1040);
    ui->result_view->setColumnWidth(2, 360);
}

bool PromptSetWidget::ExistUnSaveResultPromptConfig()
{
    ResultPrompt rp;
    rp.enabledOutRange = ui->out_range_ckbox->isChecked();
    rp.outRangeMarkType = ui->mark_first_rabtn->isChecked() ? 1 : 2;
    rp.enabledCritical = ui->critical_ckbox->isChecked();

    const auto& oldRp = DictionaryQueryManager::GetInstance()->GetResultTips();
    return rp.enabledOutRange != oldRp.enabledOutRange
        || rp.outRangeMarkType != oldRp.outRangeMarkType
        || rp.enabledCritical != oldRp.enabledCritical;   
}

void PromptSetWidget::SaveResultPromptConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    ResultPrompt rp;
    rp.enabledOutRange = ui->out_range_ckbox->isChecked();
    rp.outRangeMarkType = ui->mark_first_rabtn->isChecked() ? 1 : 2;
    rp.enabledCritical = ui->critical_ckbox->isChecked();

    if (!DictionaryQueryManager::SaveResultTips(rp))
    {
        ULOG(LOG_ERROR, "Failed to save resultip config.");
        ReportErr(true, tr("保存失败"));
    }

	// 数据字典更新
	POST_MESSAGE(MSG_ID_DICTIONARY_UPDATE);

    TipDlg(tr("保存成功")).exec();
}

void PromptSetWidget::GetModifyAlarmPromptConfig(std::vector<::tf::DisableAlarmCode>& deleteCodes, std::vector<::tf::DisableAlarmCode>& addCodes)
{
    for (int row = 0; row < m_pAlarmCodeModel->rowCount(); row++)
    {
        ::tf::DisableAlarmCode tempDac;
        QStringList codeList = m_pAlarmCodeModel->index(row, AP_COLUNM_CODE).data().toString().split("-");
        tempDac.__set_mainCode(codeList[0].toInt());
        tempDac.__set_middleCode(codeList[1].toInt());
        tempDac.__set_subCode(codeList[2].toInt());
        tempDac.__set_deviceType(m_pAlarmCodeModel->item(row, AP_COLUNM_TYPE)->data(Qt::DisplayRole).toInt());

        bool select = m_pAlarmCodeModel->item(row, AP_COLUNM_SOFTWARE_ENABLE)->data().toBool();
        auto iter = std::find_if(m_vecDisableAlarmCode.begin(), m_vecDisableAlarmCode.end(), [&](::tf::DisableAlarmCode& dac)
        {
            return tempDac.deviceType == dac.deviceType
                && tempDac.mainCode == dac.mainCode
                && tempDac.middleCode == dac.middleCode
                && tempDac.subCode == dac.subCode;
        });

        // 勾选且在已屏蔽的列表中找到,删除当前的报警码
        if (select && iter != m_vecDisableAlarmCode.end())
        {
            deleteCodes.push_back(std::move(tempDac));
        }

        // 未勾选且未在已屏蔽的列表中找到,添加当前的报警码
        if (!select && iter == m_vecDisableAlarmCode.end())
        {
            addCodes.push_back(std::move(tempDac));
        }
    }
}

void PromptSetWidget::SaveAlarmPromptConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 待删除列表
    std::vector<::tf::DisableAlarmCode> deleteCodes;
    // 待添加列表
    std::vector<::tf::DisableAlarmCode> addCodes;
    GetModifyAlarmPromptConfig(deleteCodes, addCodes);

    if (!deleteCodes.empty())
    {
        if (!DcsControlProxy::GetInstance()->DeleteDisableAlarmCodes(deleteCodes))
        {
            ULOG(LOG_ERROR, "DeleteDisableAlarmCodes Failed!");
            TipDlg(tr("保存失败"), tr("保存失败"));

            // 保存失败, 刷新页面至未修改前
            LoadAlarmPromptConfig();
            return;
        }
    }

    if (!addCodes.empty())
    {
        if (!DcsControlProxy::GetInstance()->AddDisableAlarmCodes(addCodes))
        {
            ULOG(LOG_ERROR, "AddDisableAlarmCodes Failed!");
            TipDlg(tr("保存失败"), tr("保存失败"));
            return;
        }
    }

    TipDlg(tr("提示"), tr("保存成功")).exec();
    
    // 刷新页面
    LoadAlarmPromptConfig();
}

bool PromptSetWidget::ExistUnSaveAlarmMusicConfig()
{
    AlarmMusic am;
    am.strStopSoundName = ui->stop_sound_cbox->currentText().toUtf8().toStdString();
    am.iStopSoundVolume = ui->stop_sound_hslider->value();
    am.strSampleStopSoundName = ui->sample_stop_sound_cbox->currentText().toUtf8().toStdString();
    am.iSampleStopSoundVolume = ui->sample_stop_sound_hslider->value();
    am.strAttentionSoundName = ui->attention_sound_cbox->currentText().toUtf8().toStdString();
    am.iAttentionSoundVolume = ui->attention_sound_hslider->value();

    AlarmMusic oldAm;
    if (!DictionaryQueryManager::GetAlarmMusicConfig(oldAm))
    {
        ULOG(LOG_ERROR, "Failed to get alarm music config.");
        return false;
    }

    return !(oldAm == am);
}

void PromptSetWidget::SaveAlarmMusicConfig()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    AlarmMusic am;
    // 停机
    if (ui->stop_sound_cbox->currentIndex() != -1)
    {
        am.strStopSoundName = ui->stop_sound_cbox->currentText().toUtf8().toStdString();
    }
    // 停机音乐音量
    am.iStopSoundVolume = ui->stop_sound_hslider->value();

    // 加样停
    if (ui->sample_stop_sound_cbox->currentIndex() != -1)
    {
        am.strSampleStopSoundName = ui->sample_stop_sound_cbox->currentText().toUtf8().toStdString();
    }
    // 加样停音乐音量
    am.iSampleStopSoundVolume = ui->sample_stop_sound_hslider->value();

    // 注意
    if (ui->attention_sound_cbox->currentIndex() != -1)
    {
        am.strAttentionSoundName = ui->attention_sound_cbox->currentText().toUtf8().toStdString();
    }
    //注意音乐音量
    am.iAttentionSoundVolume = ui->attention_sound_hslider->value();

    if (!DictionaryQueryManager::SaveAlarmMusicConfig(am))
    {
        ULOG(LOG_ERROR, "Failed to save alarm music config.");
        ReportErr(true, tr("保存失败"));
    }

    // 发送告警配置更新信息到UI消息总线
    POST_MESSAGE(MSG_ID_ASSAY_SYSTEM_UPDATE);
    TipDlg(tr("提示"), tr("保存成功")).exec();
}

bool PromptSetWidget::ExistUnSaveResultStatusConfig()
{
    if (ui->ch_rabtn->isChecked())
    {
        // 检查生化的结果状态是否编辑
        QStringList codes;
        for (int i = 0; i < m_pResultStatusModel->rowCount(); i++)
        {
            if (!m_pResultStatusModel->item(i, SP_COLUNM_SELECT)->data().toBool())
            {
                codes.push_back(m_pResultStatusModel->index(i, SP_COLUNM_ALARM_CODE).data().toString());
            }
        }

        return DictionaryQueryManager::GetChResultStatusCodes() != codes;
    }
    else
    {
        // 检查免疫的结果状态是否编辑
        return false;
    }

    return false;
}

void PromptSetWidget::SaveResultStatusConfig()
{
    if (ui->ch_rabtn->isChecked())
    {
        SaveChResultStatusConfig();
    }
    else
    {
        SaveImResultStatusConfig();
    }
}

void PromptSetWidget::SaveChResultStatusConfig()
{
    QStringList codes;
    for (int i = 0; i < m_pResultStatusModel->rowCount(); i++)
    {
        if (!m_pResultStatusModel->item(i, SP_COLUNM_SELECT)->data().toBool())
        {
            codes.push_back(m_pResultStatusModel->index(i, SP_COLUNM_ALARM_CODE).data().toString());
        }
    }

    if (!DictionaryQueryManager::SaveChResultStatusCodes(codes))
    {
        ULOG(LOG_ERROR, "SaveResultCode Ch Failed");
        ReportErr(true, tr("保存失败"));
    }
    else
    {
       TipDlg(tr("提示"), tr("保存成功")).exec();
    }
}

void PromptSetWidget::SaveImResultStatusConfig()
{
    std::vector< ::im::tf::ResultCodeSet> rcsVec;
    ::im::tf::ResultCodeSet rcs;
    for (int i = 0; i < m_pResultStatusModel->rowCount(); ++i)
    {
        rcs.__set_id(m_pResultStatusModel->index(i, SP_COLUNM_ID).data().toInt());
        rcs.__set_name(m_pResultStatusModel->index(i, SP_COLUNM_ALARM_CODE).data().toString().toStdString());
        rcs.__set_isDisplay(m_pResultStatusModel->item(i, SP_COLUNM_SELECT)->data().toBool());
        rcs.__set_strDiscribe(m_pResultStatusModel->index(i, SP_COLUNM_DESCRIBTION).data().toString().toStdString());
        rcsVec.push_back(rcs);
        ResultStatusCodeManager::GetInstance()->UpdateResCode(rcs.name, rcs);
    }

    if (!im::LogicControlProxy::SaveResultCode(rcsVec))
    {
        ULOG(LOG_ERROR, "SaveResultCode Im Failed");
        ReportErr(true, tr("保存失败"));
    }
    else
    {
       TipDlg(tr("提示"), tr("保存成功")).exec();
    }
}

bool PromptSetWidget::ExistUnSaveCaliLineExpire()
{
    bool ce;
    if (!DictionaryQueryManager::GetCaliLineExpire(ce))
    {
        ULOG(LOG_ERROR, "Failed to get cali line expire.");
    }

    return ce != ui->cali_line_ckbox->isChecked();
}

void PromptSetWidget::SaveCaliLineExpire()
{
    bool ce;
    if (!DictionaryQueryManager::GetCaliLineExpire(ce))
    {
        ULOG(LOG_ERROR, "Failed to get cali line expire.");
        ReportErr(true, tr("保存失败"));
    }

    // 未修改，不保存
    if (ce == ui->cali_line_ckbox->isChecked())
    {
        return;
    }

    if (!DictionaryQueryManager::SaveCaliLineExpire(!ce))
    {
        ULOG(LOG_ERROR, "Failed to save cali line expire.");
        ReportErr(true, tr("保存失败"));
    }
    // 结果状态码更新
    ResultStatusCodeManager::GetInstance()->SetCaliEShowFlag(!ce);
    DictionaryQueryManager::GetInstance()->SetCaliLineExpire(!ce);
    // 发送告警配置更新信息到UI消息总线
    POST_MESSAGE(MSG_ID_CALI_LINE_EXPIRE_CONFIG, !ce);

    TipDlg(tr("提示"), tr("保存成功")).exec();
}

void PromptSetWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_bInit)
    {
        Init();
        m_bInit = true;
    }

    // 清空报警提示查询条件
    ui->alarm_classifty_combox->setCurrentIndex(0);
    ui->lineEdit->clear();
    ui->alarm_name_edit->clear();

    // 默认当前显示第一页
    if (ui->tabWidget->currentIndex() == 0)
    {
        OnTabWidgetChangedPage(0);
    }

    ui->tabWidget->setCurrentIndex(0);
}

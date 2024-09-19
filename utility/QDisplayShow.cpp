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

#include "QDisplayShow.h"
#include "ui_QDisplayShow.h"

#include "src/common/Mlog/mlog.h"
#include "src/public/SerializeUtil.hpp"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"

#include "QDisplayTableSet.h"
#include "QSampleSetting.h"
#include "QSampleResultSetting.h"
#include "DataDictionaryDlg.h"
#include "QHeadSortDisplay.h"
#include "QWorkPageSet.h"
#include "QAssaySet.h"
#include <QMouseEvent>

// 子页面索引
enum SUB_PAGE_INDEX
{
    PAGE_SHOW_INDEX,            // 页面显示
    HEADER_SHOW_INDEX,          // 表头显示排序
    ASSAY_SHOW_INDEX,           // 项目显示排序
    PATIENT_INFO_INDEX,         // 患者信息
    DATE_FORMAT_INDEX,          // 日期格式
    LANUAGE_SET_INDEX,          // 语言切换
    DATA_DICTIONARY_INDEX,      // 数据字典
    RESULT_SHOW_INDEX,          // 结果显示
    SAMPLE_INFO_INDEX           // 样本信息
};

QDisplayShow::QDisplayShow(QWidget *parent)
    : QWidget(parent)
    , isInit(false)
{
    ui = new Ui::QDisplayShow();
    ui->setupUi(this);
    ui->tabWidget->tabBar()->installEventFilter(this);
}

QDisplayShow::~QDisplayShow()
{
}

bool QDisplayShow::isExistChangedData()
{
    bool existChangedData = false;
    switch (ui->tabWidget->currentIndex())
    {
        case PAGE_SHOW_INDEX:
		{
			auto pWid = qobject_cast<QWorkPageSet*>(ui->tabWidget->currentWidget());
			if (pWid != nullptr)
			{
				existChangedData = pWid->IsModified();
			}
		}
			break;

        case HEADER_SHOW_INDEX:
		{
			auto pWid = qobject_cast<QHeadSortDisplay*>(ui->tabWidget->currentWidget());
			if (pWid != nullptr)
			{
				existChangedData = pWid->IsModified();
			}
		}
			break;

        case ASSAY_SHOW_INDEX:
		{
			auto pWid = qobject_cast<QAssaySet*>(ui->tabWidget->currentWidget());
			if (pWid != nullptr)
			{
				existChangedData = pWid->IsModified();
			}
		}
            break;
        case PATIENT_INFO_INDEX:
		{
			auto pWid = qobject_cast<QDisplayTableSet*>(ui->tabWidget->currentWidget());
			if (pWid != nullptr)
			{
				existChangedData = pWid->IsModified();
			}
		}
            break;

            // 日期格式标签页是否有未保存的数据
        case DATE_FORMAT_INDEX:
            existChangedData = isExistUnSaveLanuageOrTimeFormat(false);
            break;

            // 语言切换标签页是否有未保存的数据
        case LANUAGE_SET_INDEX:
            existChangedData = isExistUnSaveLanuageOrTimeFormat(true);
            break;

            // 数据字典标签页是否有未保存的数据
        case DATA_DICTIONARY_INDEX:
        {
            auto pDataDictionaryDlg = qobject_cast<DataDictionaryDlg*>(ui->tabWidget->currentWidget());
            if (pDataDictionaryDlg != nullptr)
            {
                existChangedData = pDataDictionaryDlg->isExistUnsaveData();
            }
        }
            break;

            // 结果提示标签页是否有未保存的数据
        case RESULT_SHOW_INDEX:
            {
                auto pWid = qobject_cast<QSampleResultSetting*>(ui->tabWidget->currentWidget());
                if (pWid != nullptr)
                {
                    existChangedData = pWid->isExistUnsaveData();
                }
                break;
            }

            // 样本信息页面显示标签页是否有未保存的数据
        case SAMPLE_INFO_INDEX:
            {
                auto pSampleSetting = qobject_cast<QSampleSetting*>(ui->tabWidget->currentWidget());
                if (pSampleSetting != nullptr)
                {
                    existChangedData = pSampleSetting->isExistUnsaveData();
                }
                break;
            }

        default:
            break;
    }

    return existChangedData;
}

void QDisplayShow::Init()
{
    std::vector<DisplaySet> data;
    if (!DictionaryQueryManager::GetUiDispalySet(data))
    {
        ULOG(LOG_ERROR, "Failed to get DisplaySet.");
        return;
    }

    auto pageDisplaySet = new QWorkPageSet(ui->tabWidget);
    ui->tabWidget->insertTab(PAGE_SHOW_INDEX,pageDisplaySet, tr("页面显示"));

    auto pageSet = new QHeadSortDisplay(ui->tabWidget);
    ui->tabWidget->insertTab(HEADER_SHOW_INDEX,pageSet, tr("表头显示排序"));

    auto pageShowOder = new QAssaySet(this);
    ui->tabWidget->insertTab(ASSAY_SHOW_INDEX,pageShowOder, tr("项目显示顺序"));

    auto pagePatient = new QDisplayTableSet(ui->tabWidget);
    ui->tabWidget->insertTab(PATIENT_INFO_INDEX, pagePatient, tr("患者信息"));

    auto pageTableResult = new QSampleResultSetting(ui->tabWidget);
    ui->tabWidget->insertTab(RESULT_SHOW_INDEX,pageTableResult, tr("结果显示"));

    auto pageTable = new QSampleSetting(ui->tabWidget);
    ui->tabWidget->insertTab(SAMPLE_INFO_INDEX, pageTable, tr("样本信息"));
    
    auto dataDict = new DataDictionaryDlg(ui->tabWidget);
    ui->tabWidget->insertTab(DATA_DICTIONARY_INDEX, dataDict, tr("数据字典"));

    // 获取病人信息相关数据;设置数据
    for (auto& dataItem : data)
    {
        if (dataItem.type == PATIENTTYPE)
        {
            pagePatient->SetTableData(dataItem);
            dataDict->SetDictData(dataItem);
        }
    }

    ui->tabWidget->setCurrentIndex(0);

    // 设置commbox
    ui->date_combox->setItemData(0,"yyyy/MM/dd");
    ui->date_combox->setItemData(1, "MM/dd/yyyy");
    ui->date_combox->setItemData(2, "dd/MM/yyyy");

    InitLanguageCombox();

    connect(ui->flat_save, &QPushButton::clicked, this, &QDisplayShow::OnSaveDateAndLanuage);
    connect(ui->save_Button, &QPushButton::clicked, this, &QDisplayShow::OnSaveDateAndLanuage);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&](int index)
    {
        if (index == DATE_FORMAT_INDEX || index == LANUAGE_SET_INDEX)
        {
            UpdateDateAndLanuage();
        }
    });

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void QDisplayShow::InitLanguageCombox()
{
    std::vector<SoftLanguage> softLanguages;
    if (!DictionaryQueryManager::GetSoftLanguageConfig(softLanguages))
    {
        ULOG(LOG_ERROR, "Failed to get softluanguage config.");
        return;
    }

    for (auto& data : softLanguages)
    {
        ui->lanuage_combox->addItem(QString::fromStdString(data.showStr), QString::fromStdString(data.flag));
    }
}

void QDisplayShow::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!isInit)
    {
        Init();
        isInit = true;
    }

    QWidget::showEvent(event);

    ui->tabWidget->setCurrentIndex(0);
}

void QDisplayShow::UpdateDateAndLanuage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    BaseSet ds;
    if (!DictionaryQueryManager::GetUiBaseSet(ds))
    {
        return;
    }

    ui->date_combox->setCurrentIndex(ui->date_combox->findData(QString::fromStdString(ds.strDateForm)));
    ui->lanuage_combox->setCurrentIndex(ui->lanuage_combox->findData(QString::fromStdString(ds.languageType)));
}

bool QDisplayShow::isExistUnSaveLanuageOrTimeFormat(bool isLanuage)
{
    BaseSet ds;
    DictionaryQueryManager::GetUiBaseSet(ds);
    if (isLanuage)
    {
       return ds.languageType != ui->lanuage_combox->currentData().toString().toStdString();
    } 
    else
    {
        return ds.strDateForm != ui->date_combox->currentData().toString().toStdString();;
    }

    return false;
}

bool QDisplayShow::eventFilter(QObject *obj, QEvent *event)
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

void QDisplayShow::OnSaveDateAndLanuage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    BaseSet ds;
    if (!DictionaryQueryManager::GetUiBaseSet(ds))
    {
        return;
    }
    QString btnName = qobject_cast<QPushButton*>(sender())->objectName();
    if (btnName == "flat_save")
    {
        std::string tempData = ui->date_combox->currentData().toString().toStdString();
        if (tempData != ds.strDateForm)
        {
            ds.strDateForm = tempData;
        }
        else
        {
            return;
        }
    }
    else
    {
        std::string tempData = ui->lanuage_combox->currentData().toString().toStdString();
        if (tempData != ds.languageType)
        {
            if (TipDlg(tr("提示"), tr("切换软件显示语言需重启软件后才能生效, 请确认是否切换"),
                TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
            {
                return;
            }

            ds.languageType = tempData;
        }
        else
        {
            return;
        }
    }

    if (!DictionaryQueryManager::SaveUiBaseSet(ds))
    {
        TipDlg(tr("提示"), tr("保存失败")).exec();
        return;
    }

    TipDlg(tr("提示"), tr("保存成功")).exec();
    POST_MESSAGE(MSG_ID_DISPLAY_SET_UPDATE, ds);
}

void QDisplayShow::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    auto funcDispalyWidget = [this](const std::shared_ptr<UserInfoManager>& userPms, SUB_PAGE_INDEX pageIdx, int permitId) {
        QWidget* pWidget = ui->tabWidget->widget(pageIdx);
        if (pWidget != Q_NULLPTR)
        {
            pWidget->setDisabled(!userPms->IsPermisson(permitId));
        }
    };

    // 页面显示
    funcDispalyWidget(userPms, PAGE_SHOW_INDEX, PSM_DISPLAYSET_PAGESHOW);

    // 表头显示排序; 页面内独立限制
    //funcDispalyWidget(userPms, HEADER_SHOW_INDEX, PSM_DISPLAYSET_TABLEHEADERORDER);

    // 项目显示顺序
    funcDispalyWidget(userPms, ASSAY_SHOW_INDEX, PSM_DISPLAYSET_ASSAYORDER);

    // 患者信息
    funcDispalyWidget(userPms, PATIENT_INFO_INDEX, PSM_DISPLAYSET_PATIENT);

    // 结果显示
    funcDispalyWidget(userPms, RESULT_SHOW_INDEX, PSM_DISPLAYSET_RESULT);

    // 样本信息
    funcDispalyWidget(userPms, SAMPLE_INFO_INDEX, PSM_DISPLAYSET_SAMPLEINFO);

    // 数据字典；页面内独立限制
    //funcDispalyWidget(userPms, DATA_DICTIONARY_INDEX, PSM_DISPLAYSET_DICTIONARY);

    // 日期设置 
    funcDispalyWidget(userPms, DATE_FORMAT_INDEX, PSM_DISPLAYSET_DATEFORMAT);

    // 语言设置
    funcDispalyWidget(userPms, LANUAGE_SET_INDEX, PSM_DISPLAYSET_LANGUAGE);
}

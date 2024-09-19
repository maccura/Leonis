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
/// @file     QAssayIndexSetPage.h
/// @brief    项目位置设置
///
/// @author   5774/WuHongTao
/// @date     2022年6月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayIndexSetPage.h"
#include "ui_QAssayIndexSetPage.h"
#include <QScrollArea>
#include <QStandardItemModel>
#include <QFontMetrics>

#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/QAssayTabWidgets.h"

#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/DcsControlProxy.h"
#include "utility/QGroupAssayPositionSettings.h"
#include "src/common/Mlog/mlog.h"
#include "UtilityCommonFunctions.h"

#define ASSAY_MAX_COUNT_ONEROW 4 // 单项目框中每行显示的项目个数
#define COL_AMOUNT_UNIONASSAY_LIST 2 //组合项目列表的列数
#define SINGLE_BUTTON_WIDTH 160 // 项目按钮长度
#define SINGLE_BUTTON_HEIGHT 60 // 项目按钮高度

QAssayIndexSetPage::QAssayIndexSetPage(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QAssayIndexSetPage();
    ui->setupUi(this);
    m_groupAssaySetDlg = nullptr;
    Init();

	KeyDataIn setData;
	setData.assayType = AssayTypeNew::ASSAYTYPE3_POSITION_SET;
	ui->container->SetKeyStatus(setData);
    //ui->container->StartContainer(ASSAYTYPE3_POSITION_SET);

    connect(ui->container, &QAssayCardPage::selectChanged, this, &QAssayIndexSetPage::OnSelectedChagned);
}

QAssayIndexSetPage::~QAssayIndexSetPage()
{
}

///
/// @brief 初始化模块代码
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QAssayIndexSetPage::Init()
{
    ULOG(LOG_INFO, __FUNCTION__);

    ui->compostion_assay_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->compostion_assay_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->compostion_assay_view->verticalHeader()->setVisible(false);

    m_compstionModule = new QStandardItemModel(ui->compostion_assay_view);
    ui->compostion_assay_view->setModel(m_compstionModule);
    ui->compostion_assay_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 搜索框
    connect(ui->lineEditSearch, SIGNAL(textChanged(const QString&)), this, SLOT(OnSearchEditChanged(const QString&)));

    // 单项目、组合项目切换
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnTabChanged(int)));

    // 确认按钮
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveAssaySetInfo()));
    connect(ui->flat_return, SIGNAL(clicked()), this, SLOT(OnQuitBtn()));

    // 项目选择变化
    connect(ui->container, SIGNAL(deviceTypeChanged(int)), this, SLOT(OnAssayTypeSelectChange(int)));

    // 组合项目的增改删按钮事件
    connect(ui->pushButtonAdd, SIGNAL(clicked()), this, SLOT(OnUnionAssayAdd()));
    connect(ui->pushButtonEdit, SIGNAL(clicked()), this, SLOT(OnUnionAssayModify()));
    connect(ui->pushButtonDel, SIGNAL(clicked()), this, SLOT(OnUnionAssayDelete()));

    connect(ui->compostion_assay_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this, SLOT(OnUnionTabelSelectChanged(QItemSelection, QItemSelection)));
    connect(ui->compostion_assay_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnCompostionAssaySelect(const QModelIndex&))); 

    // 项目列表存在更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, UpdateSingleAssaysTable);

    // 设置默认项目选择页面
    SOFTWARE_TYPE stype = CommonInformationManager::GetInstance()->GetSoftWareType();
    OnAssayTypeSelectChange(stype == SOFTWARE_TYPE::CHEMISTRY ? 0 : (stype == SOFTWARE_TYPE::IMMUNE ? 1 : ui->single_assay_stackedWidget->currentIndex()));

    // 初始化单项目选择列表
    InitSingleAssayWidget();

    // 组合项目列表
    QStringList qHoriTitles = { tr("组合项目名称"), tr("子项目名称") };
    m_compstionModule->setHorizontalHeaderLabels(qHoriTitles);
    for (int i = 0; i < qHoriTitles.size(); ++i)
    {
        m_compstionModule->horizontalHeaderItem(i)->setToolTip(qHoriTitles[i]);
    }
    ui->compostion_assay_view->setColumnWidth(0, 180);
    ui->compostion_assay_view->setColumnWidth(1, 460);
	ui->compostion_assay_view->horizontalHeader()->setMinimumSectionSize(120);

    // 搜索框不显示
    DcsControlProxy::GetInstance()->GetAppAssayPositionSearchEditShow() ? ui->lineEditSearch->show() : ui->lineEditSearch->hide();

    // 更新组合列表下的按钮状态
    UpdateUnionDeleteButtonStuta();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void QAssayIndexSetPage::InitSingleAssayWidget()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QStringList qlstSingleAssayIm;
    QStringList qlstSingleAssayCh;
    CommonInformationManager::GetInstance()->GetDepartedAssayNames(qlstSingleAssayIm, qlstSingleAssayCh);

    InitSingleAssayWidgetSpeci(qlstSingleAssayCh, ui->single_assay_stackedpage_ch, m_singleAssayGridCh);
    InitSingleAssayWidgetSpeci(qlstSingleAssayIm, ui->single_assay_stackedpage_im, m_singleAssayGridIm);
}

void QAssayIndexSetPage::AddLetterButtons(QWidget* btnContainer, QVBoxLayout* qvLayout,
    const QString& letter, const QStringList& assayNames, std::map<QString, GridButtonsInfo>& ctrKe)
{
    // 字母标头
    QLabel* literalLabel = new QLabel(btnContainer);
    literalLabel->setText(letter);
    qvLayout->addWidget(literalLabel);
	literalLabel->setStyleSheet("padding-left: 7px;");

    // 项目名表
    GridButtonsInfo gridBtnInfo;
    QGridLayout* qAssayLayout = new QGridLayout(btnContainer);
	qAssayLayout->setHorizontalSpacing(5);
	qAssayLayout->setVerticalSpacing(15);

	int idxRow = 0;
    int idxCol = 0;
    for (const QString& ass : assayNames)
    {
        QPushButton* btn = CreatNewButton(btnContainer, ass);
        qAssayLayout->addWidget(btn, idxRow, idxCol, Qt::AlignLeft);
        gridBtnInfo.m_buttons.push_back(btn);

        // 一行最多放置4个项目按钮
        idxCol++;
        if (idxCol == ASSAY_MAX_COUNT_ONEROW)
        {
            idxRow++;
            idxCol = 0;
        }
    }

    // 如果标签下无项目，则隐藏
    if (assayNames.size() == 0)
    {
        literalLabel->hide();
    }
    else if(assayNames.size() < ASSAY_MAX_COUNT_ONEROW)
    {
        // 补齐填充
        for (int i = 0; i < ASSAY_MAX_COUNT_ONEROW - assayNames.size(); ++i)
        {
            QWidget* tempWidget = new QWidget(btnContainer);
            tempWidget->setMinimumWidth(SINGLE_BUTTON_HEIGHT);
            qAssayLayout->addWidget(tempWidget, idxRow, idxCol, Qt::AlignLeft);
            gridBtnInfo.m_placeHolder.push_back(tempWidget);
            idxCol++;
            if (idxCol == ASSAY_MAX_COUNT_ONEROW)
            {
                idxRow++;
                idxCol = 0;
            }
        }
    }

    qvLayout->addItem(qAssayLayout);
    gridBtnInfo.m_gridLayout = qAssayLayout;
    gridBtnInfo.m_letterLable = literalLabel;
    ctrKe.insert(std::pair<QString, GridButtonsInfo>(letter, gridBtnInfo)); // 缓存布局框，用于后续查找
}

void QAssayIndexSetPage::GetAllLetter(std::vector<QString>& allLetter)
{
    char currentLetter = 'A';
    for (int i = 0; i < 26; i++)
    {
        allLetter.push_back(QString(currentLetter));
        currentLetter++;
    }
    allLetter.push_back("#");
}

void QAssayIndexSetPage::InitSingleAssayWidgetSpeci(const QStringList& assayList, QWidget* pWidget, std::map<QString, GridButtonsInfo>& ctrKe)
{
    // 对项目名称按首字母分类
    std::map<QString, QStringList> classfiedAssay;
    ClassfyAssayByLitteral(assayList, classfiedAssay);

    // 生成26个大写字母, 最后一个#
    std::vector<QString> allLetter;
    GetAllLetter(allLetter);

    // 生成按钮容器
    QScrollArea* vScro = new QScrollArea(pWidget);
    QWidget* btnContainer = new QWidget(pWidget);
    btnContainer->setObjectName("pWgtAssayBtnContainer");
    QVBoxLayout* qvLayout = new QVBoxLayout(btnContainer);

    for (int i = 0; i < allLetter.size(); ++i)
    {
        auto fIt = classfiedAssay.find(allLetter[i]);
        if (fIt != classfiedAssay.end())
        {
            AddLetterButtons(btnContainer, qvLayout, fIt->first, fIt->second, ctrKe);
        }
        else
        {
            AddLetterButtons(btnContainer, qvLayout, allLetter[i], {}, ctrKe);
        }
    }

    qvLayout->addSpacing(20);
    qvLayout->addStretch();
    btnContainer->setGeometry(ui->single_assay_stackedWidget->geometry());
    btnContainer->setLayout(qvLayout);
    btnContainer->adjustSize();
    vScro->setWidgetResizable(true);
    vScro->setGeometry(ui->single_assay_stackedWidget->geometry());
    vScro->setWidget(btnContainer);
}

void QAssayIndexSetPage::UpdateSingleAssaysTableSpeci(const QStringList& assayList, QWidget* pWidget, std::map<QString, GridButtonsInfo>& ctrKe)
{
    ULOG(LOG_INFO, "%s, assays:%s.", __FUNCTION__, assayList.join(',').toStdString().c_str());

    QWidget* btnContainer = pWidget->findChild<QWidget*>("pWgtAssayBtnContainer");
    if (btnContainer == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Null button container.");
        return;
    }

    // 对项目名称按首字母分类
    std::map<QString, QStringList> classfiedAssay;
    ClassfyAssayByLitteral(assayList, classfiedAssay);

    // 生成26个大写字母, 最后一个#
    std::vector<QString> allLetter;
    GetAllLetter(allLetter);

    for (int i = 0; i < allLetter.size(); ++i)
    {
        std::map<QString, GridButtonsInfo>::iterator layoutIt = ctrKe.find(allLetter[i]);
        if (layoutIt == ctrKe.end())
        {
            continue;
        }

        // 清除占位控件
        layoutIt->second.ClearPlaceHolder();

        std::map<QString, QStringList>::iterator dataIter = classfiedAssay.find(allLetter[i]);
        if (dataIter != classfiedAssay.end())
        {
            // 更新现有控件数据
            layoutIt->second.m_letterLable->show();
            int idxRow = 0;
            int idxCol = 0;
            int iCnt = 0;
            int iOrigBtnAmount = layoutIt->second.m_buttons.size();
            for (const QString& ass : dataIter->second)
            {
                if (iCnt < iOrigBtnAmount)
                {
                    QPushButton* curBtn = layoutIt->second.m_buttons[iCnt];
                    curBtn->setText(ass);
                    curBtn->show();
                    layoutIt->second.m_gridLayout->addWidget(curBtn, idxRow, idxCol, Qt::AlignLeft);
                }
                else
                {
                    QPushButton* btn = CreatNewButton(btnContainer, ass);
                    layoutIt->second.m_gridLayout->addWidget(btn, idxRow, idxCol, Qt::AlignLeft);
                    layoutIt->second.m_buttons.push_back(btn);
                }
                idxCol++;
                iCnt++;

                // 一行最多放置4个项目按钮
                if (idxCol == ASSAY_MAX_COUNT_ONEROW)
                {
                    idxRow++;
                    idxCol = 0;
                }
            }

            // 多余的控件则隐藏
            if (dataIter->second.size() < iOrigBtnAmount)
            {
                for (int i = dataIter->second.size(); i < iOrigBtnAmount; ++i)
                {
                    layoutIt->second.m_buttons[i]->hide();
                }
            }

            // 如果不够4个，则补齐填充
            if (dataIter->second.size() < ASSAY_MAX_COUNT_ONEROW)
            {
                for (int i = dataIter->second.size(); i < ASSAY_MAX_COUNT_ONEROW; ++i)
                {
                    QWidget* tempWidget = new QWidget(btnContainer);
                    tempWidget->setMinimumWidth(SINGLE_BUTTON_HEIGHT);
                    tempWidget->show();
                    layoutIt->second.m_gridLayout->addWidget(tempWidget, 0, i, Qt::AlignLeft);
                    layoutIt->second.m_placeHolder.push_back(tempWidget);
                }
            }
        }
        // 原来的字母区域已不存在项目名，则全部隐藏
        else
        {
            layoutIt->second.HideAll();
        }
    }
    btnContainer->adjustSize();
    pWidget->update();
}

///
/// @brief 保存组合项目的位置信息
///
/// @param[in]  assayKeyid  组合项目的关键key= 10000+ 数据库主键
/// @param[in]  assayInfo   项目信息
///
/// @return true返回成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月28日，新建函数
///
bool QAssayIndexSetPage::SaveProfileInfo(int assayKeyid, const int postion)
{
    ULOG(LOG_INFO, "%s, assay keyid:%d, position:%d.", __FUNCTION__, assayKeyid, postion);

    if (assayKeyid < UI_PROFILESTART)
    {
        ULOG(LOG_WARN, "Invalid assaykey, should be more than %d.", UI_PROFILESTART);
        return false;
    }
    // 组合耗材的数据库主键等于 key -10000
    int targetAssayCode = assayKeyid - UI_PROFILESTART;

    // 修改占用位置的组合项目
    if (!CommonInformationManager::GetInstance()->ModifyProfilePosition(targetAssayCode, 0, postion))
    {
        ULOG(LOG_ERROR, "Faield to modify profileinfo.");
        return false;
    }

    return true;
}

///
/// @brief 保存单项目的项目位置信息
///
/// @param[in]  assayKeyid  但项目的项目编号
/// @param[in]  assayInfo   项目信息
///
/// @return true返回成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月28日，新建函数
///
bool QAssayIndexSetPage::SaveAssayInfo(int assayKeyid, const int postion)
{
    ULOG(LOG_INFO, "%s, assyKeyId:%d, position:%d.", __FUNCTION__, assayKeyid, postion);
    return CommonInformationManager::GetInstance()->UpdateAssayPosition(assayKeyid, 0, postion);
}

///
/// @brief 更新项目数据
///
///
/// @return 成功 true
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月3日，新建函数
///
bool QAssayIndexSetPage::UpdateAssayDataUnion(QString strName)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 更新右侧组合项目选择列表
    UpdateUnionAssayTable(GetCurrentUnionData());

    // 刷新项目选择框,以及通知工作界面的刷新处理
    int rowCnt = m_compstionModule->rowCount();
    for (int i=0; i<rowCnt; ++i)
    {
        QStandardItem* cellItem = m_compstionModule->item(i, 0);
        if (cellItem != Q_NULLPTR && cellItem->text() == strName)
        {
            OnCompostionAssaySelect(cellItem->index());
            ui->container->OnUpdateView();
            break;
        }
    }

    // 发送消息通知更新
    //POST_MESSAGE(MSG_ID_ASSAY_CODE_MANAGER_UPDATE);

    return true;
}

void QAssayIndexSetPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    //UpdateAssayDataUnion();
	ui->container->RefreshCard();
	ui->container->Show();
}

///
/// @brief 点击单项目
///
/// @param[in]  index  单项目的位置号
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月2日，新建函数
///
void QAssayIndexSetPage::OnSingleAssaySelect()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QPushButton* curBtn = qobject_cast<QPushButton*>(this->sender());
    if (curBtn == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Invalid object received form sernder.");
        return;
    }
    std::string strAssayName = curBtn->text().toStdString();
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(strAssayName);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null assayinfo, not find by name:%s.", strAssayName.c_str());
        return;
    }

    ui->container->SetAssayCardInfo(spAssayInfo->assayCode);
}

///
/// @brief 处理项目设置模块信息（生化或者免疫类型被选中）
///
/// @param[in]  index 页序号（共3页）
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月2日，新建函数
///
void QAssayIndexSetPage::OnAssayTypeSelectChange(int index)
{
    ui->compostion_assay_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->single_assay_stackedWidget->setCurrentIndex(index);

    std::vector<std::pair<QString, QString>> qlstGroupAssayIm;
    std::vector<std::pair<QString, QString>> qlstGroupAssayCh;
    CommonInformationManager::GetInstance()->GetProfileAssaysList(qlstGroupAssayCh, qlstGroupAssayIm);

    //tf::AssayClassify::type curType = ui->container->GetCurrentAssayType();
    std::vector<std::pair<QString, QString>>* curData = (!ui->container->IsChemistry()) ? &qlstGroupAssayIm : &qlstGroupAssayCh;
    UpdateUnionAssayTable(*curData);
}

///
/// @brief 保存项目位置信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月6日，新建函数
///
void QAssayIndexSetPage::OnSaveAssaySetInfo()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 保存修改后的项目位置信息
    auto assayPostionMap = ui->container->GetAssayPostion();
    for (const auto& assayCard : assayPostionMap)
    {
        // 组合项目的关键key = 10000 + id
        if (assayCard.first >= UI_PROFILESTART)
        {
            SaveProfileInfo(assayCard.first, assayCard.second);
        }
        // 单项目的关键信息时项目编号
        else
        {
            SaveAssayInfo(assayCard.first, assayCard.second);
        }
    }

    // 用于修改QButtonView中 m_position不准引起的第二次新增组合项目后保存，UI显示出错的问题
    ui->container->RefreshCard();
    // 存储修改了的单项目行数
    CommonInformationManager::GetInstance()->SetSingleAssayRowAmount(ui->container->GetSingleCount());

    // 更新对应的备用项目订单设置
    UpdateStandbyOrder();

    // 发送消息通知更新
    POST_MESSAGE(MSG_ID_ASSAY_CODE_MANAGER_UPDATE);
}

void QAssayIndexSetPage::UpdateStandbyOrder()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 获取
    BackupOrderConfig bkOrder;
    if (!DictionaryQueryManager::TakeBackupOrderConfig(bkOrder))
    {
        ULOG(LOG_WARN, "Failed to take backup order config.");
        return;
    }

    auto mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
    if (mapAssayInfo.empty())
    {
        ULOG(LOG_WARN, "Empty general assaycodemap.");
        return;
    }

    // 删除
    std::map<int, std::set<int>>::iterator it = bkOrder.sampleSrcType2TestItems.begin();
    // 样本类型的遍历
    for (; it != bkOrder.sampleSrcType2TestItems.end(); ++it)
    {
        // 包含的项目编号的遍历
        std::set<int>::iterator assayIt = it->second.begin();
        for (; assayIt != it->second.end(); ++assayIt)
        {
            // 如果项目编号已不存在或者没有在项目选择界面上，则移除
            AssayIndexCodeMaps::iterator existIter = mapAssayInfo.find(*assayIt);
            if (existIter == mapAssayInfo.end() || existIter->second->positionIdx == -1)
            {
                assayIt = it->second.erase(assayIt);
                // 移除项目编号后判断迭代器是否合法
                if (assayIt == it->second.end())
                {
                    break;
                }
            }
        }
    }

    // 回写
    if (!UiCommon::SaveBackupOrderConfig(bkOrder))
    {
        ULOG(LOG_WARN, "Failed to take backup order config.");
        return;
    }
}

void QAssayIndexSetPage::UpdateUnionDeleteButtonStuta()
{
    QModelIndexList selectedItems = ui->compostion_assay_view->selectionModel()->selectedRows();
    ui->pushButtonDel->setDisabled(selectedItems.size() == 0);
    ui->pushButtonEdit->setDisabled(selectedItems.size() == 0);
}

QPushButton* QAssayIndexSetPage::CreatNewButton(QWidget* pParent, const QString& strText)
{
    QPushButton* btn = new QPushButton(strText, pParent);
    btn->setMinimumWidth(SINGLE_BUTTON_WIDTH);
    btn->setMaximumWidth(SINGLE_BUTTON_WIDTH);
    btn->setMinimumHeight(SINGLE_BUTTON_HEIGHT);
    btn->setMaximumHeight(SINGLE_BUTTON_HEIGHT);

    btn->setProperty("buttons", "flat_button");
    connect(btn, SIGNAL(clicked()), this, SLOT(OnSingleAssaySelect()));
    btn->show();

    // 文字显示不完，添加tooltip
    QFontMetrics fm(btn->fontMetrics());
    if (fm.width(strText) > btn->width() - 32) // 边框等占用
    {
        btn->setToolTip(strText);
    }

    return btn;
}

void QAssayIndexSetPage::ClassfyAssayByLitteral(const QStringList& srcData, std::map<QString, QStringList>& classfiedAss)
{
    for (const QString& assayItem : srcData)
    {
        // 以第一个字符进行分类
        std::string tempStr = assayItem.toUpper().toStdString();
        QString targStr = (tempStr.size() > 0 && (tempStr[0] >= 'A' && tempStr[0] <= 'Z')) ? QString(tempStr[0]) : "#";

        // 用map按字母分类
        std::map<QString, QStringList>::iterator fIter = classfiedAss.find(targStr);
        if (fIter != classfiedAss.end())
        {
            fIter->second.append(assayItem);
        }
        else
        {
            classfiedAss.insert(std::pair<QString, QStringList>(targStr, { assayItem }));
        }
    }
}

void QAssayIndexSetPage::UpdateUnionAssayTable(const std::vector<std::pair<QString, QString>>& contentList)
{
    ULOG(LOG_INFO, __FUNCTION__);

    QModelIndex selectIdx = ui->compostion_assay_view->currentIndex();
    m_compstionModule->removeRows(0, m_compstionModule->rowCount());
    for (auto it = contentList.begin(); it != contentList.end(); ++it)
    {
        QList<QStandardItem*> rowList;

        QStandardItem* qName = new QStandardItem(it->first);
        qName->setToolTip(it->first);
        rowList.append(qName);

        QStandardItem* qAssays = new QStandardItem(it->second);
        qAssays->setToolTip(InsertSlashN(it->second));
        rowList.append(qAssays);

        m_compstionModule->appendRow(rowList);
    }
    
    SetTblTextAlign(m_compstionModule, Qt::AlignCenter, -1);

    // 更新删除按钮状态
    UpdateUnionDeleteButtonStuta();
    ui->compostion_assay_view->viewport()->update();

    // 选中原来的行或默认选中第一行
    if (selectIdx.isValid())
    {
        ui->compostion_assay_view->selectRow(selectIdx.row());
    }
    else
    {
        if (m_compstionModule->rowCount() > 0)
        {
            ui->compostion_assay_view->selectRow(0);
        }
    }
}

void QAssayIndexSetPage::UpdateSingleAssaysTable()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QStringList qlstSingleAssayIm;
    QStringList qlstSingleAssayCh;
    CommonInformationManager::GetInstance()->GetDepartedAssayNames(qlstSingleAssayIm, qlstSingleAssayCh);

    UpdateSingleAssaysTableSpeci(qlstSingleAssayCh, ui->single_assay_stackedpage_ch, m_singleAssayGridCh);
    UpdateSingleAssaysTableSpeci(qlstSingleAssayIm, ui->single_assay_stackedpage_im, m_singleAssayGridIm);

    // 更新组合项目

}

void QAssayIndexSetPage::FilterSingleAssay(const QString& strFilter)
{
    QStringList qlstSingleAssayIm;
    QStringList qlstSingleAssayCh;
    CommonInformationManager::GetInstance()->GetDepartedAssayNames(qlstSingleAssayIm, qlstSingleAssayCh);

    // 根据搜索添加进行过滤
    QStringList filteredIm;
    QStringList filteredCh;
    if (!strFilter.isEmpty())
    {
        for (const QString& strItem : qlstSingleAssayIm)
        {
            QString strlower = strItem.toLower();
            if (strlower.indexOf(strFilter) != -1)
            {
                filteredIm.append(strItem);
            }

        }
        for (const QString& strItem : qlstSingleAssayCh)
        {
            QString strlower = strItem.toLower();
            if (strlower.indexOf(strFilter) != -1)
            {
                filteredCh.append(strItem);
            }
        }
    }
    else
    {
        filteredCh = qlstSingleAssayCh;
        filteredIm = qlstSingleAssayIm;
    }

    UpdateSingleAssaysTableSpeci(filteredCh, ui->single_assay_stackedpage_ch, m_singleAssayGridCh);
    UpdateSingleAssaysTableSpeci(filteredIm, ui->single_assay_stackedpage_im, m_singleAssayGridIm);
}

void QAssayIndexSetPage::FilterUnionAssay(const QString& strFilter)
{
    std::vector<std::pair<QString, QString>> allUnion = GetCurrentUnionData();

    // 筛选
    std::vector<std::pair<QString, QString>> filtered;
    for (const std::pair<QString, QString>& pr : allUnion)
    {
        if (pr.first.toLower().indexOf(strFilter) != -1)
        {
            filtered.push_back(pr);
        }
    }
    UpdateUnionAssayTable(filtered);
}

std::vector<std::pair<QString, QString>> QAssayIndexSetPage::GetCurrentUnionData()
{
    std::vector<std::pair<QString, QString>> qlstGroupAssayIm;
    std::vector<std::pair<QString, QString>> qlstGroupAssayCh;
    CommonInformationManager::GetInstance()->GetProfileAssaysList(qlstGroupAssayCh, qlstGroupAssayIm);

    // 组合项目列表
    //tf::AssayClassify::type curType = ui->container->GetCurrentAssayType();
    return ui->container->IsChemistry() ? qlstGroupAssayCh:qlstGroupAssayIm  ;
}

void QAssayIndexSetPage::OnSearchEditChanged(const QString& searchText)
{
    QString strSearch = ui->lineEditSearch->text().toLower();
    int curProfIdx = ui->tabWidget->currentIndex();
    if (curProfIdx == 0)
    {
        FilterSingleAssay(strSearch);
    }
    else
    {
        FilterUnionAssay(strSearch);
    }
}

void QAssayIndexSetPage::OnTabChanged(int curSection)
{
    if (ui->lineEditSearch->isHidden())
    {
        return;
    }

    // 缓存搜索框中各自对应的搜索内容
    static QString strIm = "";
    static QString strCh = "";

    if (curSection == 0) // 生化
    {
        strIm = ui->lineEditSearch->text();
        ui->lineEditSearch->setText(strCh);
    }
    else
    {
        strCh = ui->lineEditSearch->text();
        ui->lineEditSearch->setText(strIm);
    }
}

void QAssayIndexSetPage::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;
}

void QAssayIndexSetPage::OnCompostionAssaySelect(const QModelIndex& index)
{
    UpdateUnionDeleteButtonStuta();

    if (index.isValid())
    {
        QModelIndex nameIdx = ui->compostion_assay_view->model()->index(index.row(), 0);
        std::string name = ui->compostion_assay_view->model()->data(nameIdx).toString().toStdString();
        auto profileKeyId = CommonInformationManager::GetInstance()->GetProfileInfoFromName(name);
        if (profileKeyId.has_value())
        {
            // 组合项目传的时Id
            ui->container->SetProfileCardInfo(profileKeyId.value() + UI_PROFILESTART);
        }
    }
}

void QAssayIndexSetPage::OnUnionTabelSelectChanged(const QItemSelection& currIndex, const QItemSelection& prevIndex)
{
    QModelIndexList selectedList = ui->compostion_assay_view->selectionModel()->selectedRows();
    if (selectedList.size() > 0)
    {
        OnCompostionAssaySelect(selectedList[0]);
    }
}

void QAssayIndexSetPage::OnQuitBtn()
{
	ui->container->UpdateTestItemStatus(std::vector<std::shared_ptr<tf::TestItem>>{});
	ui->container->UpdateAssayProfileStatus(std::vector<int64_t>{});
	ui->container->show();
}

QString QAssayIndexSetPage::InsertSlashN(const QString& strSource)
{
    QFontMetrics fm(this->fontMetrics());
    int numLines = (fm.width(strSource) / this->width()) + 1;
    int perCharc = strSource.length() / numLines;

    int curPos = perCharc;
    QString strResult = strSource;
    while (curPos < strSource.length())
    {
        strResult.insert(curPos, "\n");
        curPos += perCharc;
    }

    return strResult;
}

void QAssayIndexSetPage::OnSelectedChagned()
{
    ui->compostion_assay_view->blockSignals(true);
    ui->compostion_assay_view->clearSelection();
    ui->compostion_assay_view->blockSignals(false);
}

void QAssayIndexSetPage::OnUnionAssayAdd()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (m_groupAssaySetDlg == nullptr)
    {
        m_groupAssaySetDlg = new QGroupAssayPositionSettings(this, DlgTypeGroupAssayAdd);
        connect(m_groupAssaySetDlg, SIGNAL(SignalUpdateProfileList(QString)), this, SLOT(UpdateAssayDataUnion(QString)));
    }
    m_groupAssaySetDlg->SetDlgTypeAndUpdate(DlgTypeGroupAssayAdd, "");
    m_groupAssaySetDlg->show();
}

void QAssayIndexSetPage::OnUnionAssayModify()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (m_groupAssaySetDlg == nullptr)
    {
        m_groupAssaySetDlg = new QGroupAssayPositionSettings(this, DlgTypeGroupAssayModify);
        connect(m_groupAssaySetDlg, SIGNAL(SignalUpdateProfileList(QString)), this, SLOT(UpdateAssayDataUnion(QString)));
    }

    QModelIndexList curSelectedRows = ui->compostion_assay_view->selectionModel()->selectedRows();
    if (curSelectedRows.size() == 0)
    {
        TipDlg(tr("选中行为空，不能进行编辑操作！")).exec();
        return;
    }

    QModelIndex curIdx = ui->compostion_assay_view->currentIndex();
    if (!curIdx.isValid())
    {
        ULOG(LOG_WARN, "Invalid model index.");
        return;
    }
    QString qstrProfName = m_compstionModule->item(curIdx.row(), 0)->text();
    m_groupAssaySetDlg->SetDlgTypeAndUpdate(DlgTypeGroupAssayModify, qstrProfName);

    m_groupAssaySetDlg->show();
}

void QAssayIndexSetPage::OnUnionAssayDelete()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 多行删除的支持
    QModelIndexList selectedItems = ui->compostion_assay_view->selectionModel()->selectedRows();
    if (selectedItems.size() == 0)
    {
        TipDlg(tr("选中行为空，不能进行删除操作！")).exec();
        return;
    }
    QStringList targetProfNames;
    for (const QModelIndex& mi : selectedItems)
    {
        if (!mi.isValid())
        {
            continue;
        }
        QString qstrProfName = m_compstionModule->item(mi.row(), 0)->text();
        targetProfNames.append(qstrProfName);
    }
    if (targetProfNames.size() == 0)
    {
        TipDlg(tr("未检出存在合法选中行！")).exec();
        ULOG(LOG_ERROR, "No exist any valid choosed rows.");
        return;
    }

    // 组合项目中的ISE、SIND不能添加
    const auto& CIM = CommonInformationManager::GetInstance();
    QString iseName = QString::fromStdString(CIM->GetAssayNameByCode(::ise::tf::g_ise_constants.ASSAY_CODE_ISE));
    QString sindName = QString::fromStdString(CIM->GetAssayNameByCode(::ch::tf::g_ch_constants.ASSAY_CODE_SIND));
    if (targetProfNames.contains(iseName) || targetProfNames.contains(sindName))
    {
        TipDlg(tr("不能删除组合项目:%1和%2！").arg(iseName).arg(sindName)).exec();
        return;
    }

    TipDlg noticeMsg(tr("删除"), tr("是否确认删除组合项目%1?").arg(targetProfNames.join(',')), TipDlgType::TWO_BUTTON);
    if (noticeMsg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Canceled to delete union assay.");
        return;
    }

    for (const QString& strProf : targetProfNames)
    {
        auto profileCode = CIM->GetProfileInfoFromName(strProf.toStdString());
        if (!CIM->DeleteProfileInfo(strProf.toStdString()))
        {
            TipDlg(tr("执行删除失败！")).exec();
            return;
        }

		if (profileCode.has_value())
		{
			// 删除左侧卡片框中对应的组合项目
			ui->container->DeleteAssayByCode(profileCode.value());
		}
    }

    // 更新右侧组合项目名列表
    QString strName;
    QModelIndex currIdx = ui->compostion_assay_view->currentIndex();
    if (currIdx.isValid())
    {
        QStandardItem* curItem = m_compstionModule->item(currIdx.row(), 0);
        if (curItem != Q_NULLPTR)
        {
            strName = curItem->text();
        }
    }
    UpdateAssayDataUnion(strName);
}

void QAssayIndexSetPage::GridButtonsInfo::ClearPlaceHolder()
{
    for (int i = 0; i < m_placeHolder.size(); ++i)
    {
        if (m_placeHolder[i] == nullptr)
        {
            continue;
        }

        m_placeHolder[i]->hide();
        m_gridLayout->removeWidget(m_placeHolder[i]);
        delete m_placeHolder[i];
        m_placeHolder[i] = nullptr;
    }

    m_placeHolder.clear();
}

void QAssayIndexSetPage::GridButtonsInfo::HideAll()
{
    // 隐藏字母控件
    m_letterLable->hide();

    // 隐藏按钮控件
    for (auto bt : m_buttons)
    {
        if (bt != nullptr)
        {
            bt->hide();
        }
    }

    // 隐藏占位控件
    for (auto wg : m_placeHolder)
    {
        if (wg != nullptr)
        {
            wg->hide();
        }
    }
}

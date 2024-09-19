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

#include "QGroupAssayPositionSettings.h"
#include "ui_QGroupAssayPositionSettings.h"
#include <QGridLayout>
#include <QScrollArea>
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"


// isLetter 不能区分希腊字母，所以直接比较值
#define IS_UPPTER_LETTER(chr) ((chr) >= 'A' && (chr) <= 'Z')

QGroupAssayPositionSettings::QGroupAssayPositionSettings(QWidget *parent, EnumDlgType dlgType)
    : BaseDlg(parent)
{
    ui = new Ui::QGroupAssayPositionSettings();
    ui->setupUi(this);
    m_modifyId = -1;
    m_dlgType = dlgType;

    // 初始化左侧字母框
    InitLetterWidget();

    // 初始化项目名列表
    InitAssayTable();

    QRegExp qExp("[\\S]{35}");
    ui->lineEditProfName->setValidator(new QRegExpValidator(qExp));

    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(OnSaveButtonPress()));
    connect(ui->pushButtonQuit, SIGNAL(clicked()), this, SLOT(OnClearSelectedQuit()));

    connect(ui->tabWidgetAssayBtnCt, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentTabChanged(int)));

    // 如果是选则框
    if (dlgType == DlgTypeAssaySelect)
    {
        SetTitleName(tr("项目选择"));
        ui->labeProfName->hide();
        ui->lineEditProfName->hide();
    }

    // 注册项目设置更新消息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, UpdateItemShowDatas);
}

QGroupAssayPositionSettings::~QGroupAssayPositionSettings()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void QGroupAssayPositionSettings::SetDlgTypeAndUpdate(EnumDlgType dlgType, const QString& groupAssayName)
{
    ULOG(LOG_INFO, "%s(type:%d).", __FUNCTION__, dlgType);

    m_dlgType = dlgType;

    // 如果是选则框
    if (dlgType == DlgTypeAssaySelect)
    {
        SetTitleName(tr("项目选择"));

        // 隐藏组合项目名设置框
        ui->labeProfName->hide();
        ui->lineEditProfName->hide();
        return;
    }

    // 隐藏组合项目名设置框
    ui->labeProfName->show();
    ui->lineEditProfName->show();

    // 字母筛选状态
    bool isAdd = (m_dlgType == DlgTypeGroupAssayAdd);
    QString strTitle = isAdd ? tr("新增组合项目") : tr("编辑组合项目");
    SetTitleName(strTitle);

    ui->lineEditProfName->setText(isAdd ? "" : groupAssayName);
    QStringList assayNames;
    if (!isAdd)
    {
        auto modifyItem = CommonInformationManager::GetInstance()->GetProfileInfoFromName(groupAssayName.toStdString());
        if (!modifyItem)
        {
            ULOG(LOG_ERROR, "Failed to get profile by name:%s.", groupAssayName.toStdString().c_str());
            return;
        }
        m_modifyId = modifyItem.get();
        CommonInformationManager::GetInstance()->GetProfileSubNames(groupAssayName.toStdString(), assayNames);
    }

    // 更新勾选状态缓存与节目按钮状态
    m_checkedButtons.clear();
    for (const QString& assay : assayNames)
    {
        m_checkedButtons.insert(std::pair<QString, int>(assay, GetAssayCodeByName(assay)));
    }
    UpdateButtonCheckStat(assayNames);
}

void QGroupAssayPositionSettings::GetSelectedAssayNames(QStringList& checkedAssays, std::set<int>& assayCodes)
{
    ULOG(LOG_INFO, __FUNCTION__);

    checkedAssays.clear();
    assayCodes.clear();
    AssayIndexNameMaps assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayMap();

    std::vector<std::shared_ptr<tf::CalcAssayInfo>> calcAssays;
    CommonInformationManager::GetInstance()->GetCalcAssayInfo(calcAssays);

    for (const auto& strItem : m_checkedButtons)
    {
        checkedAssays.append(strItem.first);
        assayCodes.insert(strItem.second);
    }
}

void QGroupAssayPositionSettings::ClearSelectedAssays()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 更新计算项目状态按钮
    if (m_dlgType != DlgTypeAssaySelect)
    {
        return;
    }
    m_checkedButtons.clear();
    for (auto btn : m_vecButtonCh)
    {
        btn->setChecked(false);
    }    
    for (auto btn : m_vecButtonIm)
    {
        btn->setChecked(false);
    }
    for (auto btn : m_vecButtonCalc)
    {
        btn->setChecked(false);
    }
}

bool QGroupAssayPositionSettings::SaveGroupAssayInfo()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 检测组合项目名
    QString profName = ui->lineEditProfName->text();
    if (profName.isEmpty())
    {
        TipDlg("组合项目名不可为空！").exec();
        return false;
    }
    // 组合项目名不能和项目名重复
    const auto& CIM = CommonInformationManager::GetInstance();
    if (CIM->GetAssayInfo(profName.toStdString()) != nullptr)
    {
        TipDlg(tr("组合项目名不能和项目名重复！")).exec();
        return false;
    }

    // 组合项目中的ISE、SIND不能添加
    QString iseName = QString::fromStdString(CIM->GetAssayNameByCode(::ise::tf::g_ise_constants.ASSAY_CODE_ISE));
    QString sindName = QString::fromStdString(CIM->GetAssayNameByCode(::ch::tf::g_ch_constants.ASSAY_CODE_SIND));
    if (profName == iseName || profName == sindName)
    {
        TipDlg(tr("组合项目名不能使用%1和%2！").arg(iseName).arg(sindName)).exec();
        return false;
    }

    // 设置组合项目信息
    std::string strProName = profName.toStdString();
    boost::optional<int64_t> curPrNameId = CIM->GetProfileInfoFromName(strProName);
    if (curPrNameId && curPrNameId.get() != m_modifyId)
    {
        TipDlg(tr("组合名已经存在，不可重复！")).exec();
        return false;
    }

    // 获取选择的项目
    bool isImmune = ui->tabWidgetAssayBtnCt->currentWidget() == ui->tabIm;
    if (m_checkedButtons.size() == 0)
    {
        TipDlg(tr("组合项目中需要包含子项目，不可为空！")).exec();
        return false;
    }

    QStringList checkedAssays;
    for (const auto& strItem : m_checkedButtons)
    {
        checkedAssays.append(strItem.first);
    }
    std::vector<int> assayCodes;
    CommonInformationManager::GetInstance()->TranseAssayNameToAssayCode(checkedAssays, assayCodes);

    if (m_dlgType == DlgTypeGroupAssayAdd)
    {
        tf::ProfileInfo prof;
        prof.__set_pageIdx(-1);
        prof.__set_positionIdx(-1);
        prof.__set_profileName(strProName);
        prof.__set_assayClassify(isImmune ? tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE : tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
        prof.__set_subItems(assayCodes);

        // 存储组合项目信息
        if (!CommonInformationManager::GetInstance()->AddProfileInfo(prof))
        {
            TipDlg(tr("保存组合信息失败！")).exec();
            return false;
        }
    }
    else if (m_dlgType == DlgTypeGroupAssayModify)
    {
        auto modifyProf = CommonInformationManager::GetInstance()->GetCommonProfile(m_modifyId);
        if (modifyProf)
        {
            modifyProf->__set_profileName(strProName);
            modifyProf->__set_subItems(assayCodes);
            if (!CommonInformationManager::GetInstance()->ModifyProfileInfo(*modifyProf))
            {
                TipDlg(tr("修改组合信息失败！")).exec();
                return false;
            }
        }
    }

    return true;
}

void QGroupAssayPositionSettings::OnSaveButtonPress()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 组合项目设置的保存
    if (m_dlgType == DlgTypeGroupAssayAdd || m_dlgType == DlgTypeGroupAssayModify)
    {
        if (!SaveGroupAssayInfo())
        {
            return;
        }
        emit SignalUpdateProfileList(ui->lineEditProfName->text());
    }
    else if (m_dlgType == DlgTypeAssaySelect)
    {
        emit selected();
    }

    this->accept();
}

void QGroupAssayPositionSettings::FilterOnButtons(const QString& strFilter, std::map<QString, QPushButton*>* mapButtons, 
    std::vector<QPushButton*>* vecButtons, QStringList* qNm)
{
    if (mapButtons == nullptr || vecButtons == nullptr || qNm == nullptr)
    {
        ULOG(LOG_ERROR, "Null ptr, cause by unknown tab widget.");
        return;
    }

    // 对按钮进行显示与隐藏
    std::vector<QString> showAssays;
    if (!strFilter.isEmpty())
    {
        // 筛选需要显示的项目(首字母筛选，‘#’代表非字母)
        bool isFilterNoneLetter = (strFilter == "#");
        for (auto it = mapButtons->begin(); it != mapButtons->end(); ++it)
        {
            if (it->first.isEmpty())
            {
                continue;
            }

            QChar firstLit = it->first[0].toUpper();
            if (isFilterNoneLetter && !IS_UPPTER_LETTER(firstLit))
            {
                showAssays.push_back(it->first);
                continue;
            }
            if (IS_UPPTER_LETTER(firstLit) && firstLit == strFilter)
            {
                showAssays.push_back(it->first);
                continue;
            }
        }
    }
    else
    {
        for (const QString& strNm : *qNm)
        {
            showAssays.push_back(strNm);
        }
    }

    // 进行显示和隐藏操作
    int i = 0;
    for (auto it = vecButtons->begin(); it != vecButtons->end(); ++it)
    {
        if (i < showAssays.size())
        {
            QString strOrig = (*it)->toolTip();
            (*it)->setToolTip(showAssays[i]);
            (*it)->setText(showAssays[i]);
            (*it)->show();
            (*it)->setChecked(m_checkedButtons.find(showAssays[i]) != m_checkedButtons.end());

            QPushButton* swapBtn = (*mapButtons)[showAssays[i]];
            swapBtn->setText(strOrig);
            swapBtn->setToolTip(strOrig);

            // 交换按钮
            std::map<QString, QPushButton*>::iterator ita = mapButtons->find(showAssays[i]);
            if (ita != mapButtons->end())
            {
                ita->second = *it;
            }
            std::map<QString, QPushButton*>::iterator itb = mapButtons->find(strOrig);
            if (itb != mapButtons->end())
            {
                itb->second = swapBtn;
            }
        }
        else
        {
            (*it)->hide();
        }
        ++i;
    }
}

void QGroupAssayPositionSettings::ProcFilterButtons(const QString& strLetter)
{
    // 找出筛选出来的，需要显示的项目按钮
    std::map<QString, QPushButton*> *curBtnInfo = nullptr;
    std::vector<QPushButton*> *curVecBtns = nullptr;
    QStringList* curNames = nullptr;
    QWidget* pWidget = ui->tabWidgetAssayBtnCt->currentWidget();
    if (pWidget == nullptr)
    {
        ULOG(LOG_WARN, "Current widget is null.");
        return;
    }
    QString widgetName = pWidget->objectName();
    if (widgetName == "tabCh")
    {
        curVecBtns = &m_vecButtonCh;
        curBtnInfo = &m_assayButtonsCh;
        curNames = &m_namesCh;
    }
    else if (widgetName == "tabIm")
    {
        curVecBtns = &m_vecButtonIm;
        curBtnInfo = &m_assayButtonsIm;
        curNames = &m_namesIm;
    }
    else if (widgetName == "widgetCalc")
    {
        curVecBtns = &m_vecButtonCalc;
        curBtnInfo = &m_assayButtonsCalc;
        curNames = &m_namesCalc;
    }

    // 对按钮进行显示和隐藏
    FilterOnButtons(strLetter, curBtnInfo, curVecBtns, curNames);
}

void QGroupAssayPositionSettings::hideEvent(QHideEvent *event)
{
}

int QGroupAssayPositionSettings::GetTableIndex(const QString& qTbObjectName)
{
    for (int i=0; i<ui->tabWidgetAssayBtnCt->count(); ++i)
    {
        QWidget* curWig = ui->tabWidgetAssayBtnCt->widget(i);
        if (curWig == Q_NULLPTR)
        {
            continue;
        }
        if (curWig->objectName() == qTbObjectName)
        {
            return i;
        }
    }

    return -1;
}

int QGroupAssayPositionSettings::GetAssayCodeByName(const QString& assayName)
{
    const AssayIndexNameMaps& assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayMap();

    std::vector<std::shared_ptr<tf::CalcAssayInfo>> calcAssays;
    CommonInformationManager::GetInstance()->GetCalcAssayInfo(calcAssays);

    // 普通项目名->项目编号
    std::string strAssayName = assayName.toStdString();
    AssayIndexNameMaps::const_iterator it = assayMap.find(strAssayName);
    if (it != assayMap.end())
    {
        return it->second->assayCode;
    }
    // 计算项目名->项目编号
    else
    {
        for (const std::shared_ptr<tf::CalcAssayInfo>& calcItem : calcAssays)
        {
            if (calcItem->name == strAssayName)
            {
                return calcItem->assayCode;
            }
        }
    }

    return -1;
}

std::string QGroupAssayPositionSettings::GetAssayNameByCode(int assayCode)
{
    std::shared_ptr<CommonInformationManager> ins = CommonInformationManager::GetInstance();
    std::shared_ptr<::tf::GeneralAssayInfo> pGeneral = ins->GetAssayInfo(assayCode);
    if (pGeneral != nullptr)
    {
        return pGeneral->assayName;
    }

    std::shared_ptr<::tf::CalcAssayInfo> pCalc = ins->GetCalcAssayInfo(assayCode);
    if (pCalc != nullptr)
    {
        return pCalc->name;
    }

    return "";
}

void QGroupAssayPositionSettings::FilterByLitter(QPushButton* litterBtn, bool bStat)
{
    if (litterBtn == Q_NULLPTR)
    {
        return;
    }
    if (bStat)
    {
        // 处理字母按钮选择互斥
        static QString preButtonName("");
        if (!preButtonName.isEmpty() && preButtonName != litterBtn->text())
        {
            m_letterButtons[preButtonName]->blockSignals(true);
            m_letterButtons[preButtonName]->setChecked(false);
            m_letterButtons[preButtonName]->blockSignals(false);
        }
        preButtonName = litterBtn->text();
    }

    ProcFilterButtons(bStat ? litterBtn->text() : "");
}

void QGroupAssayPositionSettings::OnFilerLetter(bool stat)
{
    ULOG(LOG_INFO, "%s(stat:%d)", __FUNCTION__, stat);

    QPushButton* curBtn = qobject_cast<QPushButton*>(sender());
    if (curBtn == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Reveived PushButton is null.");
        return;
    }

    FilterByLitter(curBtn, stat);
}

void QGroupAssayPositionSettings::OnAssayChecked(bool bChecked)
{
    QPushButton* curBtn = qobject_cast<QPushButton*>(sender());
    if (curBtn == Q_NULLPTR)
    {
        return;
    }

    // 维护记录勾选的容器
    if (curBtn->isChecked())
    {
        QString curName = curBtn->toolTip();
        m_checkedButtons.insert(std::pair<QString, int>(curName, GetAssayCodeByName(curName)));
    }
    else
    {
        auto it = m_checkedButtons.find(curBtn->toolTip());
        if (it != m_checkedButtons.end())
        {
            m_checkedButtons.erase(it);
        }
    }
}

void QGroupAssayPositionSettings::OnClearSelectedQuit()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 清除筛选字母
    for (std::map<QString, QPushButton*>::iterator it = m_letterButtons.begin(); it != m_letterButtons.end(); ++it)
    {
        if (it->second->isChecked())
        {
            it->second->setChecked(false);
        }
    }

    // 清除选则状态
    ClearSelectedAssays();

    this->close();
}

void QGroupAssayPositionSettings::UpdateItemShowDatas()
{
    InitAssayTable();
}

void QGroupAssayPositionSettings::OnCurrentTabChanged(int index)
{
    std::map<QString, QPushButton*>::iterator it = m_letterButtons.begin();
    for (; it != m_letterButtons.end(); ++it)
    {
        if (it->second->isChecked())
        {
            FilterByLitter(it->second, true);
            return;
        }
    }

    // 取消筛选
    FilterByLitter(m_letterButtons.begin()->second, false);
}

void QGroupAssayPositionSettings::InitLetterWidget()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QVBoxLayout* vLayout = new QVBoxLayout(ui->widget_siglitteral);

    QGridLayout* gridLayout = new QGridLayout(ui->widget_siglitteral);
    gridLayout->setSpacing(10);
    int idxRow = 0;
    int idxCol = 0;
    char currentLetter = 'A';
    for (int i = 0; i < 27; i++) // 26个大写字母, 最后一个#
    {
        if (i == 26)
        {
            currentLetter = '#';
        }

        QPushButton* btn = new QPushButton(QString(currentLetter), ui->widget_siglitteral);
        btn->setProperty("buttons", "flat_button");
        btn->setProperty("checkable", true);
        btn->setMinimumWidth(70);
        btn->setMaximumWidth(70);
        btn->setMinimumHeight(40);
        btn->setMaximumHeight(40);

        connect(btn, SIGNAL(toggled(bool)), this, SLOT(OnFilerLetter(bool)));
        gridLayout->addWidget(btn, idxRow, idxCol);
        m_letterButtons.insert(std::pair<QString, QPushButton*>(QString(currentLetter), btn));

        currentLetter++;
        idxCol++;
        if (idxCol == 3) // 设定位3列
        {
            idxRow++;
            idxCol = 0;
        }
    }
    vLayout->addItem(gridLayout);
    vLayout->addStretch();
	vLayout->setMargin(0);
    ui->widget_siglitteral->setLayout(vLayout);
}

void QGroupAssayPositionSettings::InitCalcAssayTable()
{
    ULOG(LOG_INFO, __FUNCTION__);

    QWidget* tabCalc = ui->tabWidgetAssayBtnCt->findChild<QWidget*>("widgetCalc");
    if (tabCalc == Q_NULLPTR)
    {
        tabCalc = new QWidget();
        tabCalc->setObjectName(QStringLiteral("widgetCalc"));
        ui->tabWidgetAssayBtnCt->addTab(tabCalc, tr("计算项目"));
    }

    QWidget* widget_sub_calc = tabCalc->findChild<QWidget*>("widget_sub_calc");
    if (widget_sub_calc == Q_NULLPTR)
    {
        widget_sub_calc = new QWidget(tabCalc);
        widget_sub_calc->setObjectName(QStringLiteral("widget_sub_calc"));
        widget_sub_calc->setGeometry(QRect(-1, -1, 1416, 649));
    }
    
    QStringList qCalcNames;
    std::vector<std::shared_ptr<tf::CalcAssayInfo>> calcAssayInfo;
    CommonInformationManager::GetInstance()->GetCalcAssayInfo(calcAssayInfo);
    for (const std::shared_ptr<tf::CalcAssayInfo>& calcItem : calcAssayInfo)
    {
        if (calcItem == nullptr)
        {
            continue;
        }
        qCalcNames.append(QString::fromStdString(calcItem->name));
    }

    // 创建计算项目按钮
    InitAssayTableWidget(widget_sub_calc, qCalcNames, m_assayButtonsCalc, m_vecButtonCalc);
    m_namesCalc = qCalcNames;
}

void QGroupAssayPositionSettings::InitAssayTable()
{
    ULOG(LOG_INFO, __FUNCTION__);

    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();

    // 更新选中的项目名
    std::vector<QString> tempPr;
    for (std::map<QString, int>::iterator itm = m_checkedButtons.begin(); itm != m_checkedButtons.end(); ++itm)
    {
        QString newName = QString::fromStdString(GetAssayNameByCode(itm->second));
        if (itm->first != newName)
        {
            tempPr.push_back(newName);

            itm = m_checkedButtons.erase(itm);
            if (itm == m_checkedButtons.end())
            {
                break;
            }
        }
    }
    for (const QString& strN : tempPr)
    {
        m_checkedButtons.insert(std::pair<QString, int>(strN, GetAssayCodeByName(strN)));
    }

    QStringList qlstSingleAssayIm;
    QStringList qlstSingleAssayCh;
    assayMgr->GetDepartedAssayNames(qlstSingleAssayIm, qlstSingleAssayCh);

    // 如果是生免联机
    bool isHideBar = false;
    SOFTWARE_TYPE curDevs = assayMgr->GetSoftWareType();
    if (curDevs == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
    {
        // 创建生化按钮
        InitAssayTableWidget(ui->widget_sub_ch, qlstSingleAssayCh, m_assayButtonsCh, m_vecButtonCh);
        m_namesCh = qlstSingleAssayCh;

        // 创建免疫按钮
        InitAssayTableWidget(ui->widget_sub_im, qlstSingleAssayIm, m_assayButtonsIm, m_vecButtonIm);
        m_namesIm = qlstSingleAssayIm;
    }
    else
    {
        if (curDevs == SOFTWARE_TYPE::IMMUNE)
        {
            // 创建免疫按钮
            InitAssayTableWidget(ui->widget_sub_im, qlstSingleAssayIm, m_assayButtonsIm, m_vecButtonIm);
            m_namesIm = qlstSingleAssayIm;
            int chIdx = GetTableIndex("tabCh");
            if (chIdx != -1)
            {
                ui->tabWidgetAssayBtnCt->removeTab(chIdx);
            }
        }
        else
        {
            // 创建生化按钮
            InitAssayTableWidget(ui->widget_sub_ch, qlstSingleAssayCh, m_assayButtonsCh, m_vecButtonCh);
            m_namesCh = qlstSingleAssayCh;
            int imIdx = GetTableIndex("tabIm");
            if (imIdx != -1)
            {
                ui->tabWidgetAssayBtnCt->removeTab(imIdx);
            }
        }

        if (m_dlgType != DlgTypeAssaySelect)
        {
            // 当只有生化或者免疫的时候隐藏tabBar
            isHideBar = true;
            ui->tabWidgetAssayBtnCt->tabBar()->hide();
            //ui->line_h_3->move(ui->tabWidgetAssayBtnCt->pos().x(), ui->tabWidgetAssayBtnCt->pos().y());
        }
    }

    //if (!isHideBar)
    //{
    //    const int verticalLineHight = 46;
    //    ui->line_v_4->setFixedHeight(ui->line_v_4->height() - verticalLineHight);
    //    ui->line_v_4->move(ui->line_v_4->geometry().x(), ui->line_v_4->geometry().y() + verticalLineHight);
    //}

    // 如果是数据浏览的项目选择框，则还需增加计算项目选择表
    if (m_dlgType == DlgTypeAssaySelect)
    {
        InitCalcAssayTable();
    }
}

#define BUTTON_ROW_AMOUNT 9 // 9列
void QGroupAssayPositionSettings::InitAssayTableWidget(QWidget* pWidget, \
    const QStringList& assayNames, std::map<QString, QPushButton*>& btns, std::vector<QPushButton*>& vecBtn)
{
    btns.clear();

    // 生成按钮容器
    QScrollArea* vScro = Q_NULLPTR;
    QWidget* btnContainer = Q_NULLPTR;
    QVBoxLayout* qVLayout = Q_NULLPTR;
    QGridLayout* qAssayLayout = Q_NULLPTR;
    std::map<QWidget*, ViewCtrlInfo>::iterator vIter = m_viewCtrls.find(pWidget);
    // 重新添加按钮
    if (vIter != m_viewCtrls.end() && !vIter->second.IsNull())
    {
        vScro = vIter->second.m_vScro;
        btnContainer = vIter->second.m_btnContainer;
        qVLayout = vIter->second.m_qVLayout;
        qAssayLayout = vIter->second.m_qAssayLayout;
        for (auto btn : vecBtn)
        {
            qAssayLayout->removeWidget(btn);
            btn->hide();
            delete btn;
            btn = Q_NULLPTR;
        }
        btns.clear();
        vecBtn.clear();
    }
    else
    {
        vScro = new QScrollArea(pWidget);
        btnContainer = new QWidget(pWidget);
        btnContainer->setObjectName("pWgtAssayBtnContainer");

        qVLayout = new QVBoxLayout(btnContainer);
        qAssayLayout = new QGridLayout(btnContainer);
        qAssayLayout->setSpacing(10);
		qAssayLayout->setContentsMargins(5, 5, 0, 0);
        m_viewCtrls.insert(std::pair<QWidget*, ViewCtrlInfo>(pWidget, ViewCtrlInfo(vScro, btnContainer, qVLayout, qAssayLayout)));
    }

    int idxRow = 0;
    int idxCol = 0;
    for (const QString& aN : assayNames)
    {
        QPushButton* btn = new QPushButton(aN, btnContainer);
        // 防止项目名称过长增加提示，bug26284
        btn->setToolTip(aN);
		btn->setMinimumWidth(140);
		btn->setMaximumWidth(140);
		btn->setMinimumHeight(70);
        btn->setMaximumHeight(70);
        btn->setProperty("buttons", "flat_button");
        btn->setProperty("checkable", QVariant(true));
        connect(btn, SIGNAL(toggled(bool)), this, SLOT(OnAssayChecked(bool)));

        vecBtn.push_back(btn);
        btns.insert(std::pair<QString, QPushButton*>(aN, btn));
        qAssayLayout->addWidget(btn, idxRow, idxCol);

        // 一行最多放置9个项目按钮
        idxCol++;
        if (idxCol == BUTTON_ROW_AMOUNT)
        {
            idxRow++;
            idxCol = 0;
        }
    }

    // 补齐填充
    for (int i = 0; i < BUTTON_ROW_AMOUNT; ++i)
    {
        qAssayLayout->addWidget(new QWidget(btnContainer), idxRow, idxCol);
        idxCol++;
        if (idxCol == BUTTON_ROW_AMOUNT)
        {
            idxRow++;
            idxCol = 0;
        }
    }

    // 添加过就跳过
    if (vIter == m_viewCtrls.end())
    {
        qVLayout->addItem(qAssayLayout);
        qVLayout->addStretch();

        btnContainer->setGeometry(pWidget->geometry());
        btnContainer->setLayout(qVLayout);
        btnContainer->adjustSize();

        vScro->setWidgetResizable(true);
        QRect widgetRect = pWidget->geometry();
        widgetRect.setHeight(widgetRect.height() - 46);
        vScro->setGeometry(widgetRect);
        vScro->setWidget(btnContainer);
    }
}

void QGroupAssayPositionSettings::UpdateButtonCheckStat(const QStringList& assayNames)
{
    ULOG(LOG_INFO, __FUNCTION__);

    std::set<QString> qAssaySet;
    for (const QString& name : assayNames)
    {
        qAssaySet.insert(name);
    }

    // 更新生化按钮状态
    for (auto it = m_assayButtonsCh.begin(); it != m_assayButtonsCh.end(); ++it)
    {
        it->second->setChecked(qAssaySet.find(it->first) != qAssaySet.end());
    }

    // 更新免疫状态按钮
    for (auto it = m_assayButtonsIm.begin(); it != m_assayButtonsIm.end(); ++it)
    {
        it->second->setChecked(qAssaySet.find(it->first) != qAssaySet.end());
    }

    // 更新计算项目状态按钮
    if (m_dlgType == DlgTypeAssaySelect)
    {
        for (auto it = m_assayButtonsCalc.begin(); it != m_assayButtonsCalc.end(); ++it)
        {
            it->second->setChecked(qAssaySet.find(it->first) != qAssaySet.end());
        }
    }
}

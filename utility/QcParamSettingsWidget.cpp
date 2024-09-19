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

#include "QcParamSettingsWidget.h"
#include "ui_QcParamSettingsWidget.h"
#include <vector>
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "QStandardItemModel"
#include "QRegExp"
#include "QRegExpValidator"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/DataManagerQc.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"

#define TABLEEMPTYROW 12 // 质控品表尾部空行数
#define WEEK_DAY_COUNT 7 // 一周有7天
#define COL_AMOUNT 4     // 表格的列数

Q_DECLARE_METATYPE(std::shared_ptr<::tf::QcDoc>);


QcParamSettingsWidget::QcParamSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_bSampleTypeFlag(false)
{
    ui = new Ui::QcParamSettingsWidget();
    ui->setupUi(this);
    m_currentAssayCode = -1;
    m_disableAutoQcChk = false;

    QStringList qcHeader = { tr("质控品编号"), tr("质控品名称"), tr("质控品批号"), tr("质控品水平") };
    m_qcModel = new QStandardItemModel(this);
    m_qcModel->setHorizontalHeaderLabels(qcHeader);

    ui->tableView->setModel(m_qcModel);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);

    ui->lineEditQcbatchLimit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->lineEditQcIntervSamp->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->lineEditQcIntervHours->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(OnTabelSelectChanged(const QModelIndex&, const QModelIndex&)));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    // 质控品存在变化，说明保存了
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocUpdate);
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayInfoUpdate);
}

QcParamSettingsWidget::~QcParamSettingsWidget()
{
}

///
/// @bref
///		把质控时间填写到界面
///
/// @param[in] qcSettings 质控设置
///
/// @par History:
/// @li 8276/huchunli, 2023年11月21日，新建函数
///
void QcParamSettingsWidget::AssignQcTimeToUi(const tf::QcAssaySettings& qcSettings)
{
    // 质控时间
    if (qcSettings.__isset.time && !qcSettings.time.empty())
    {
        ui->timeEditQcTime->setTime(QTime::fromString(QString::fromStdString(qcSettings.time), "HH:mm:ss"));
    }
    else
    {
        ui->timeEditQcTime->setTime(QTime(0, 0, 0));
    }

    // 星期状态
    std::set<int> valueWeek;
    if (qcSettings.__isset.lstWeekIndxs)
    {
        for (unsigned int i = 0; i < qcSettings.lstWeekIndxs.size(); ++i)
        {
            valueWeek.insert(qcSettings.lstWeekIndxs[i]);
        }
    }
    for (unsigned int i = 0; i < WEEK_DAY_COUNT; ++i) // 7代表一周7天
    {
        QString checkboxName = QString("checkBox_week%1").arg(i);
        QCheckBox* curChk = this->findChild<QCheckBox*>(checkboxName);
        if (curChk != nullptr)
        {
            curChk->setChecked(valueWeek.find(i) != valueWeek.end()); // 没有值代表没有勾选
        }
    }
}

void QcParamSettingsWidget::LoadQcParameter(const AssayListModel::StAssayListRowItem &item)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    m_rowItem = item;

    // 更新保存在项目中的相关设置到界面
    OnAssayInfoUpdate();

    // 更新保存在质控品中的设置信息到界面
    OnQcDocUpdate({});
}

///
/// @bref
///		把界面上的自动质控设置保存到行缓存中
///
/// @param[in] rowIdx 对应行号
///
/// @par History:
/// @li 8276/huchunli, 2023年11月13日，新建函数
///
void QcParamSettingsWidget::SaveUiAutoCtrlToCache(int rowIdx)
{
    QStandardItem* preItem = m_qcModel->item(rowIdx, 1);
    if (preItem == nullptr)
    {
        return;
    }
    QVariant preQcValue = preItem->data(Qt::UserRole + 1);
    if (!preQcValue.isNull())
    {
        std::shared_ptr<::tf::QcDoc> qcDoc = preQcValue.value<std::shared_ptr<::tf::QcDoc>>();
        if (qcDoc == nullptr)
        {
            return;
        }
        int compositionIndex = FindAutoCtrlPosition(qcDoc, m_currentAssayCode);
        if (compositionIndex != -1 && compositionIndex < qcDoc->compositions.size())
        {
            AutoQcCtrlSwitch tempAutoCfg;
            tempAutoCfg.timeCheck = (ui->checkBoxQcTime->isChecked());
            tempAutoCfg.countCheck = (ui->checkBoxQcIntervSamp->isChecked());
            tempAutoCfg.hoursCheck = (ui->checkBoxQcIntervHours->isChecked());
            tempAutoCfg.afterCaliCheck = (ui->checkBoxAfterCali->isChecked());

            // 如果都为false且原始值为空，则表示没有变化，不进行修改
            if ((tempAutoCfg.timeCheck + tempAutoCfg.countCheck + tempAutoCfg.hoursCheck + tempAutoCfg.afterCaliCheck) == 0 && 
                qcDoc->compositions[compositionIndex].autoCtrlSet.empty())
            {
                ULOG(LOG_INFO, "Empty autoctrl param to change.");
                return;
            }

            qcDoc->compositions[compositionIndex].__set_autoCtrlSet(tempAutoCfg.EncodeAutoCtrlParam());
        }
    }
}

void QcParamSettingsWidget::OnTabelSelectChanged(const QModelIndex& currIdx, const QModelIndex& prevIdx)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 当前选择行数据
    if (m_currentAssayCode == -1 || !currIdx.isValid())
    {
        ULOG(LOG_INFO, "Invalid index.");
        return;
    }

    // 缓存之前的自动质控信息
    if (prevIdx.isValid())
    {
        SaveUiAutoCtrlToCache(prevIdx.row());
    }

    // 刷新当前的自动质控信息
    ClearQcSettingUi();

    // 刷新当前自动质控信息
    QVariant qcValue = m_qcModel->item(currIdx.row(), 1)->data(Qt::UserRole + 1);
    if (qcValue.isNull())
    {
        ULOG(LOG_INFO, "QcDoc data IsNUll.");
        ui->widget_2->setEnabled(false); // 置灰自动质控设置
        return;
    }
    ui->widget_2->setEnabled(m_disableAutoQcChk);

    // 根据assayCode找到对应的Composition中的autoCtrl设置,并更新对应界面
    std::shared_ptr<::tf::QcDoc> qcDoc = qcValue.value<std::shared_ptr<::tf::QcDoc>>();
    if (qcDoc == nullptr)
    {
        return;
    }
    int compositionIndex = FindAutoCtrlPosition(qcDoc, m_currentAssayCode);
    if (compositionIndex == -1)
    {
        return;
    }
    UpdateQcUi(qcDoc->compositions[compositionIndex].autoCtrlSet);
}

void QcParamSettingsWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    // 联合质控
    ui->groupBox_2->setEnabled(userPms->IsPermisson(PSM_IM_ASSAYSET_UNION_QC_SET));
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_UNION_QC_SET) ? ui->groupBox_2->show() : ui->groupBox_2->hide();

    // 自动质控
    bool autoQc = userPms->IsPermisson(PMS_SETTINGS_AUTO_QC);
    ui->groupBox->setEnabled(autoQc);
    ui->widget_2->setEnabled(autoQc);
    m_disableAutoQcChk = autoQc;
    userPms->IsPermissionShow(PMS_SETTINGS_AUTO_QC) ? ui->groupBox->show() : ui->groupBox->hide();
}

void QcParamSettingsWidget::OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos)
{
    Q_UNUSED(infos);

    m_origAutoCtrl.clear();
    m_bSampleTypeFlag = false;
    // 清空界面
    ClearQcSettingUi();

    // 根据项目ID筛选出质控品
    std::vector<std::shared_ptr<::tf::QcDoc>> targetDoc;

    std::vector<std::shared_ptr<::tf::QcDoc>> allQcDocs;
    DataManagerQc::GetInstance()->GetQcDocs(allQcDocs);
    for (const std::shared_ptr<::tf::QcDoc>& dc : allQcDocs)
    {
        if (dc == nullptr)
        {
            continue;
        }
        for (const ::tf::QcComposition& cp : dc->compositions)
        {
            if (cp.assayCode == m_rowItem.assayCode)
            {
                targetDoc.push_back(std::make_shared<::tf::QcDoc>(*dc));
                m_origAutoCtrl.insert(std::pair<int64_t, std::string>(dc->id, cp.autoCtrlSet));
                break;
            }
        }
    }

    // 质控品的显示
    QModelIndex currSelected = ui->tableView->currentIndex();
    m_qcModel->removeRows(0, m_qcModel->rowCount());
    unsigned int irow = 0;
    for (; irow < targetDoc.size(); ++irow)
    {
        QStandardItem* qitemName = new QStandardItem(QString::fromStdString(targetDoc[irow]->name));
        qitemName->setData(QVariant::fromValue(targetDoc[irow]), Qt::UserRole + 1);
        m_qcModel->setItem(irow, 0, new QStandardItem(QString::fromStdString(targetDoc[irow]->sn)));
        m_qcModel->setItem(irow, 1, qitemName);
        m_qcModel->setItem(irow, 2, new QStandardItem(QString::fromStdString(targetDoc[irow]->lot)));
        m_qcModel->setItem(irow, 3, new QStandardItem(QString::number(targetDoc[irow]->level)));
    }
    // 添加末尾空行
    int colunmCnt = m_qcModel->columnCount();
    for (unsigned int i = 0; i < TABLEEMPTYROW; ++i)
    {
        for (unsigned int col = 0; col < colunmCnt; ++col)
        {
            m_qcModel->setItem(irow, col, new QStandardItem(""));
        }
        irow++;
    }

    SetTblTextAlign(m_qcModel, Qt::AlignCenter, -1);

    // 恢复选中
    if (targetDoc.size() > 0)
    {
        int defaultSelectRow = currSelected.isValid() ? currSelected.row() : 0;
        ui->tableView->selectRow(defaultSelectRow);
    }
    m_bSampleTypeFlag = true;
    ui->widget_2->setEnabled(targetDoc.size() > 0 && m_disableAutoQcChk); // 置灰自动质控设置
}

void QcParamSettingsWidget::OnAssayInfoUpdate()
{
    ui->lineEditQcbatchLimit->clear();
    ui->lineEditQcIntervSamp->clear();
    ui->lineEditQcIntervHours->clear();
    ui->timeEditQcTime->setTime(QTime(0, 0, 0));

    for (auto& ckBox : ui->widget->findChildren<QCheckBox*>())
    {
        ckBox->setChecked(false);
    }

    m_currentAssayCode = m_rowItem.assayCode;

    // 批时间限UI赋值
    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_currentAssayCode);
    if (assayInfo == nullptr)
    {
        return;
    }

    int tempTime = assayInfo->qcSettings.unionTimeLimit;
    ui->lineEditQcbatchLimit->setText(tempTime == 0 ? "" : QString::number(tempTime));

    tempTime = assayInfo->qcSettings.sampleCntLimitQc;
    ui->lineEditQcIntervSamp->setText(tempTime == 0 ? "" : QString::number(tempTime));

    tempTime = assayInfo->qcSettings.timeExpriQc;
    ui->lineEditQcIntervHours->setText(tempTime == 0 ? "" : QString::number(tempTime));
    AssignQcTimeToUi(assayInfo->qcSettings);
}

void QcParamSettingsWidget::UpdateQcUi(const std::string& qcSettings)
{
    AutoQcCtrlSwitch tempCtr;
    tempCtr.DecodeAutoCtrlParamFrom(qcSettings);

    // 是否启用自动质控时间
    ui->checkBoxQcTime->setChecked(tempCtr.timeCheck);

    // 是否使用间隔样本数
    ui->checkBoxQcIntervSamp->setChecked(tempCtr.countCheck);

    // 是否使用质控间隔时间
    ui->checkBoxQcIntervHours->setChecked(tempCtr.hoursCheck);

    // 是否启用校准后质控
    ui->checkBoxAfterCali->setChecked(tempCtr.afterCaliCheck);
}

int  QcParamSettingsWidget::FindAutoCtrlPosition(const std::shared_ptr<tf::QcDoc>& qcDoc, int assayCode)
{
    if (qcDoc == nullptr)
    {
        ULOG(LOG_WARN, "%s null qcDoc parameter.", __FUNCTION__);
        return -1;
    }
    int compositionIdx = -1;
    for (unsigned int i = 0; i < qcDoc->compositions.size(); ++i)
    {
        if (qcDoc->compositions[i].assayCode == assayCode)
        {
            compositionIdx = i;
        }
    }

    return compositionIdx;
}

void QcParamSettingsWidget::ClearQcSettingUi()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    UpdateQcUi("");
}

///
/// @bref
///		把当前行的质控品信息进行入库保存
///
/// @param[in] rowIdx 当前行号
///
/// @par History:
/// @li 8276/huchunli, 2023年11月13日，新建函数
///
void QcParamSettingsWidget::GetChangedQcAutoCtrl(int rowIdx, std::vector<std::shared_ptr<::tf::QcDoc>>& chengedQc)
{
    // 获取当前行中缓存的质控品
    QStandardItem* curQitem = m_qcModel->item(rowIdx, 1);
    if (curQitem == nullptr)
    {
        return;
    }

    QVariant qcValue = curQitem->data(Qt::UserRole + 1);
    if (qcValue.isNull())
    {
        ULOG(LOG_INFO, "Cache qcDoc data is nullptr.");
        return;
    }

    std::shared_ptr<tf::QcDoc> origQcDoc = qcValue.value<std::shared_ptr<tf::QcDoc>>();
    if (origQcDoc == nullptr)
    {
        return;
    }

    int iPositionIdx = FindAutoCtrlPosition(origQcDoc, m_currentAssayCode);
    if (iPositionIdx == -1)
    {
        ULOG(LOG_WARN, "Not exist composition index.");
        return;
    }

    // 判断是否存在修改
    auto qcDocIt = m_origAutoCtrl.find(origQcDoc->id);
    if (qcDocIt != m_origAutoCtrl.end() && origQcDoc->compositions[iPositionIdx].autoCtrlSet == qcDocIt->second)
    {
        ULOG(LOG_INFO, "No changed ret.");
        return;
    }

    // ISE项目的质控设置同步
    if (m_currentAssayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && m_currentAssayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        if (iPositionIdx < origQcDoc->compositions.size())
        {
            std::string curAutoCtrl = origQcDoc->compositions[iPositionIdx].autoCtrlSet;
            for (int i = ::ise::tf::g_ise_constants.ASSAY_CODE_NA; i <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL; i++)
            {
                int iIsePosition = FindAutoCtrlPosition(origQcDoc, i);
                if (iIsePosition != -1)
                {
                    origQcDoc->compositions[iIsePosition].__set_autoCtrlSet(curAutoCtrl);
                }
            }
        }
    }

    chengedQc.push_back(origQcDoc);
}

bool QcParamSettingsWidget::GetQcParameter(tf::GeneralAssayInfo& pGen, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 保存批时间限
    tf::QcAssaySettings qcSettingsObj;
    qcSettingsObj.__set_unionTimeLimit(ui->lineEditQcbatchLimit->text().toInt());
    qcSettingsObj.__set_sampleCntLimitQc(ui->lineEditQcIntervSamp->text().toInt());
    qcSettingsObj.__set_timeExpriQc(ui->lineEditQcIntervHours->text().toInt());
    qcSettingsObj.__set_time(ui->timeEditQcTime->text().toStdString());

    std::vector<int16_t> weeklist;
    for (unsigned int i = 0; i < WEEK_DAY_COUNT; ++i)
    {
        QString checkboxName = QString("checkBox_week%1").arg(i);
        QCheckBox* curChk = this->findChild<QCheckBox*>(checkboxName);
        if (curChk != nullptr && curChk->isChecked())
        {
            weeklist.push_back(i);
        }
    }
    qcSettingsObj.__set_lstWeekIndxs(weeklist);
    pGen.__set_qcSettings(qcSettingsObj);

    // 当前选择行数据
    int selectedRow = ui->tableView->currentIndex().row();
    if (m_currentAssayCode == -1 || selectedRow == -1)
    {
        ULOG(LOG_INFO, "Invalid index.");
        return true;
    }

    // 保存当前界面到对应缓存
    SaveUiAutoCtrlToCache(selectedRow);

    // 对每一行的修改进行保存
    int totolRow = m_qcModel->rowCount();
    for (int i = 0; i < totolRow; ++i)
    {
        GetChangedQcAutoCtrl(i, changedQc);
    }

    return true;
}

///
/// @brief 获取更改操作细节信息
///
/// @param[in]  strOptLog  返回的日志记录信息
/// @param[in]  spAssayInfoUi  界面修改的项目信息
/// @param[in]  vecQcDocUi  界面修改的项目质控信息
/// @param[in]  vecQcDocs  修改前的项目质控信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，新建函数
///
void QcParamSettingsWidget::GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<tf::QcDoc>>& vecQcDocsUi, const std::vector<tf::QcDoc>& vecQcDocs)
{
	auto MakeStr = [](QString& strOptLog, const QString& strName, const QString& strValue)
	{
		strOptLog += " ";
		strOptLog += strName;
		strOptLog += ":";
		strOptLog += strValue;
	};
	// 参数检查
	if (nullptr == spAssayInfoUi)
	{
		ULOG(LOG_WARN, "The input param is nullptr.");
		return;
	}
	// 从CommMgr中获取项目信息
	auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(spAssayInfoUi->assayCode);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_WARN, "Get Assay parameter infor error.");
		return;
	}
	double dFactor = 1;
	for (auto& atUnit : spAssayInfoUi->units)
	{
		if (atUnit.isCurrent)
		{
			dFactor = atUnit.factor;
			break;
		}
	}
	QString strOpen(tr("打开")), strClose(tr("关闭")), strCheck(tr("勾选")), strUncheck(tr("取消勾选"));
	// 如果修改了批时间限
	if (spAssayInfoUi->qcSettings.unionTimeLimit != spAssayInfo->qcSettings.unionTimeLimit)
	{
		MakeStr(strOptLog, ui->label_5->text(), QString::number(spAssayInfoUi->qcSettings.unionTimeLimit)+ui->label_6->text());
	}
	// 如果修改了间隔样本数
	if (spAssayInfoUi->qcSettings.sampleCntLimitQc != spAssayInfo->qcSettings.sampleCntLimitQc)
	{
		MakeStr(strOptLog, ui->label_2->text(), QString::number(spAssayInfoUi->qcSettings.sampleCntLimitQc));
	}
	// 如果修改了质控间隔时间
	if (spAssayInfoUi->qcSettings.timeExpriQc != spAssayInfo->qcSettings.timeExpriQc)
	{
		MakeStr(strOptLog, ui->label_3->text(), QString::number(spAssayInfoUi->qcSettings.timeExpriQc) + ui->label->text());
	}
	// 如果修改了质控时间
	if (spAssayInfoUi->qcSettings.time != spAssayInfo->qcSettings.time)
	{
		MakeStr(strOptLog, ui->labelQcTime->text(), QString::fromStdString(spAssayInfoUi->qcSettings.time));
	}
	// 周时间，0：周天，1-6：周一到周六
	std::vector<QCheckBox *> vecLebles = { ui->checkBox_week0, ui->checkBox_week1, ui->checkBox_week2, ui->checkBox_week3, ui->checkBox_week4, ui->checkBox_week5, ui->checkBox_week6};
	for (int i=0; i<7; ++i)
	{
		bool bFindUi = false, bFindOld = false;
		for (const auto atVal : spAssayInfoUi->qcSettings.lstWeekIndxs)
		{
			if (atVal == i)
			{
				bFindUi = true;
				break;
			}
		}
		for (const auto atVal : spAssayInfo->qcSettings.lstWeekIndxs)
		{
			if (atVal == i)
			{
				bFindOld = true;
				break;
			}
		}
		// 打开
		if (bFindUi && !bFindOld)
		{
			MakeStr(strOptLog, vecLebles[i]->text(), strCheck);
		}
		else if (!bFindUi && bFindOld)	// 关闭
		{
			MakeStr(strOptLog, vecLebles[i]->text(), strUncheck);
		}
		else  // 其他情况
		{
			;
		}
	}
	// 遍历界面自动质控设置的变更
	for (const auto& spQcDocUi : vecQcDocsUi)
	{
		if (nullptr == spQcDocUi)
		{
			continue;
		}
		// 遍历相关质控文档
		for (const auto& atQcDoc : vecQcDocs)
		{
			// 如果不是同一个
			if (spQcDocUi->id != atQcDoc.id)
			{
				continue;
			}
			// 对比差别
			for (const auto& atCompUi : spQcDocUi->compositions)
			{
				for (const auto& atComp : atQcDoc.compositions)
				{
					if (atCompUi.assayCode != atComp.assayCode)
					{
						continue;
					}
					if (atCompUi.assayCode != spAssayInfoUi->assayCode)
					{
						continue;
					}
                    if (atCompUi.autoCtrlSet == atComp.autoCtrlSet)
                    {
                        continue;
                    }

                    AutoQcCtrlSwitch uiCs;
                    uiCs.DecodeAutoCtrlParamFrom(atCompUi.autoCtrlSet);

                    AutoQcCtrlSwitch atCs;
                    atCs.DecodeAutoCtrlParamFrom(atComp.autoCtrlSet);

					// 如果修改了间隔样本数
					if (uiCs.countCheck != atCs.countCheck)
					{
						MakeStr(strOptLog, QString::fromStdString(spQcDocUi->name) + ui->checkBoxQcIntervSamp->text(),
							uiCs.countCheck ? strCheck : strUncheck);
					}
					// 如果修改了质控间隔时间
					if (uiCs.hoursCheck != atCs.hoursCheck)
					{
						MakeStr(strOptLog, QString::fromStdString(spQcDocUi->name) + ui->checkBoxQcIntervHours->text(),
							uiCs.hoursCheck ? strCheck : strUncheck);
					}
					// 如果修改了使用质控时间控制自动质控
					if (uiCs.timeCheck != atCs.timeCheck)
					{
						MakeStr(strOptLog, QString::fromStdString(spQcDocUi->name) + ui->checkBoxQcTime->text(),
							uiCs.timeCheck ? strCheck : strUncheck);
					}
					// 如果修改了完成校准后控制
					if (uiCs.afterCaliCheck != atCs.afterCaliCheck)
					{
						MakeStr(strOptLog, QString::fromStdString(spQcDocUi->name) + ui->checkBoxAfterCali->text(),
							uiCs.afterCaliCheck ? strCheck : strUncheck);
					}
					break;
				}
			}
			break;
		}
	}
}

bool QcParamSettingsWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem & item)
{
    // 当前选择行数据
    int selectedRow = ui->tableView->currentIndex().row();
    if (m_currentAssayCode == -1 || selectedRow == -1)
    {
        ULOG(LOG_INFO, "Invalid index.");
        return false;
    }

    // 保存当前界面到对应缓存
    SaveUiAutoCtrlToCache(selectedRow);

    // 检测每一行的修改情况
    std::vector<std::shared_ptr<tf::QcDoc>> changedQc;
    int totolRow = m_qcModel->rowCount();
    for (int i = 0; i < totolRow; ++i)
    {
        GetChangedQcAutoCtrl(i, changedQc);
        if (changedQc.size() > 0)
        {
            return true;
        }
    }

    return false;
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 1226/zhangjing，2023年4月27日，新建函数
///
void QcParamSettingsWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);
}

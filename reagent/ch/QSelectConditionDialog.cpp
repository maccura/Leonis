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

#include "QSelectConditionDialog.h"
#include "ui_QSelectConditionDialog.h"
#include <QKeyEvent>
#include "shared/ReagentCommon.h"
#include "src/common/Mlog/mlog.h"
#include "uidcsadapter/uidcsadapter.h"


QSelectConditionDialog::QSelectConditionDialog(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::QSelectConditionDialog())
{
    ui->setupUi(this);
    SetTitleName(tr("筛选"));

    Init();
}

QSelectConditionDialog::~QSelectConditionDialog()
{
}

void QSelectConditionDialog::SetDisableForIm(bool disableFlag)
{
    ui->pushButtonNoPleace->setDisabled(disableFlag);
    ui->type_close->setDisabled(disableFlag);
    ui->type_open->setDisabled(disableFlag);
    ui->calistatus_edit->setDisabled(disableFlag);
}

void QSelectConditionDialog::InitConditionCheck(const std::shared_ptr<SelectConditions>& cond, \
    const std::vector<std::shared_ptr<const tf::DeviceInfo>>& curDevi)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 如果只有免疫项目，则disable未放置、开闭试剂
    SetDisableForIm(IsDevicesOnlyIm(curDevi));

    if (cond == nullptr)
    {
        ULOG(LOG_ERROR, "Invlaid condition.");
        return;
    }

    ui->status_current->setChecked(cond->m_inUse);
    ui->status_backup->setChecked(cond->m_backup); // 备用
    ui->pushButtonEmpty->setChecked(cond->m_usageStatusEmpty); // 空
    ui->status_calimask->setChecked(cond->m_caliShield); // 试剂屏蔽
    ui->status_reagentmask->setChecked(cond->m_reagentShield);
    ui->pushButtonNoPleace->setChecked(cond->m_notPlace);

    ui->calistatus_succeed->setChecked(cond->m_caliSuccess);
    ui->calistatus_failed->setChecked(cond->m_caliFailed);
    ui->calistatus_none->setChecked(cond->m_caliStatusEmpty);
    ui->pushButtonCaliing->setChecked(cond->m_caliing);
    ui->pushButtonNotyetCali->setChecked(cond->m_notyetCali);
    ui->calistatus_edit->setChecked(cond->m_caliEdit);

    ui->type_close->setChecked(cond->m_closeType); // 封闭试剂
    ui->type_open->setChecked(cond->m_openType); // 开放试剂
    ui->pushButtonScanFailed->setChecked(cond->m_scanFailed);
}

void QSelectConditionDialog::Init()
{
    // 如果只有免疫设备，则隐藏开放封闭按钮
    if (gUiAdapterPtr()->WhetherOnlyImDev())
    {
        ui->type_open->setVisible(false);
        ui->type_close->setVisible(false);
        ui->pushButtonNoPleace->setVisible(false);
        ui->calistatus_edit->setVisible(false);

        QGridLayout* plyt = qobject_cast<QGridLayout*>(ui->widget->layout());
        if (plyt == Q_NULLPTR)
        {
            return;
        }
        plyt->addWidget(ui->pushButtonEmpty, 1, 1);
        plyt->addWidget(ui->pushButtonScanFailed, 6, 1);
        //ui->pushButtonEmpty->move(152, 166);
        //ui->pushButtonScanFailed->move(152, 384);
    }

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(OnDoSearchReagent()));
    connect(ui->cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->ResetBtn, SIGNAL(clicked()), this, SLOT(OnReset()));

    //处理按钮多语言显示
    QString tmpStr = ui->status_calimask->fontMetrics().elidedText(ui->status_calimask->text(), Qt::ElideRight, 70);
    ui->status_calimask->setToolTip(ui->status_calimask->text());
    ui->status_calimask->setText(tmpStr);

    tmpStr = ui->calistatus_succeed->fontMetrics().elidedText(ui->calistatus_succeed->text(), Qt::ElideRight, 90);
    ui->calistatus_succeed->setToolTip(ui->calistatus_succeed->text());
    ui->calistatus_succeed->setText(tmpStr);
}

void QSelectConditionDialog::OnDoSearchReagent()
{
    std::shared_ptr<SelectConditions> cond(new SelectConditions);
    // 使用中
    cond->m_inUse = ui->status_current->isChecked();
    // 备用
    cond->m_backup = ui->status_backup->isChecked();
    // 无效试剂,使用状态为空
    cond->m_usageStatusEmpty = ui->pushButtonEmpty->isChecked(); // 空
    // 校准屏蔽
    cond->m_caliShield = ui->status_calimask->isChecked();
    // 试剂屏蔽
    cond->m_reagentShield = ui->status_reagentmask->isChecked();
    // 未放置
    cond->m_notPlace = ui->pushButtonNoPleace->isChecked();

    // 校准成功
    cond->m_caliSuccess = ui->calistatus_succeed->isChecked();
    // 校准失败
    cond->m_caliFailed = ui->calistatus_failed->isChecked();
    // 校准空
    cond->m_caliStatusEmpty = ui->calistatus_none->isChecked();
    // 正在校准
    cond->m_caliing = ui->pushButtonCaliing->isChecked();
    // 未校准
    cond->m_notyetCali = ui->pushButtonNotyetCali->isChecked();
    // 编辑
    cond->m_caliEdit = ui->calistatus_edit->isChecked();

    // 封闭试剂
    cond->m_closeType = ui->type_close->isChecked();
    // 开放试剂
    cond->m_openType = ui->type_open->isChecked();
    // 扫描失败
    cond->m_scanFailed = ui->pushButtonScanFailed->isChecked();

    // 没有筛选条件时，不进行筛选操作
    emit SendSearchCondition(cond);
    close();
}

void SelectConditions::CovertTo(::im::tf::ReagTableUIQueryCond& queryCond)
{
    // 使用状态
    m_inUse ? queryCond.__set_onUse(true) : void(0);
    m_backup ? queryCond.__set_backUp(true) : void(0);
    m_reagentShield ? queryCond.__set_reagMask(true) : void(0);
    m_caliShield ? queryCond.__set_caliMask(true) : void(0);
    m_usageStatusEmpty ? queryCond.__set_stateEmpty(true) : void(0);

    // 校准状态
    m_caliSuccess ? queryCond.__set_caliSucc(true) : void(0);
    m_caliFailed ? queryCond.__set_caliFail(true) : void(0);
    m_notyetCali ? queryCond.__set_noCali(true) : void(0);
    m_caliing ? queryCond.__set_beCaling(true) : void(0);
    m_caliStatusEmpty ? queryCond.__set_caliEmpty(true) : void(0);

    // 试剂类型
    m_scanFailed ? queryCond.__set_scanFail(true) : void(0);
    m_closeType ? queryCond.__set_normalReag(true) : void(0); // i6000 正常试剂为封闭试剂
    m_openType ? queryCond.__set_normalReag(false) : void(0);
}

void SelectConditions::CovertTo(::ch::tf::ReagentGroupQueryCond& queryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 使用状态
    {
        std::vector<::tf::UsageStatus::type>  usageStatus;
        m_inUse ? usageStatus.push_back(::tf::UsageStatus::type::USAGE_STATUS_CURRENT) : void(0);
        m_backup ? usageStatus.push_back(::tf::UsageStatus::type::USAGE_STATUS_BACKUP) : void(0);
		m_usageStatusEmpty ? usageStatus.push_back(::tf::UsageStatus::type::USAGE_STATUS_CAN_NOT_USE) : void(0);// 空
        if (usageStatus.size() > 0)
        {
            queryCond.__set_usageStatus(usageStatus);
        }

        m_reagentShield ? queryCond.__set_reagentMask(true) : void(0); // 试剂屏蔽
        m_caliShield ? queryCond.__set_caliMask(true) : void(0); // 校准屏蔽
    }

    // 校准状态
    {
        std::vector<::tf::CaliStatus::type> caliStatus;
        m_caliSuccess ? caliStatus.push_back(::tf::CaliStatus::type::CALI_STATUS_SUCCESS) : void(0); // 校准成功
        m_caliFailed ? caliStatus.push_back(::tf::CaliStatus::type::CALI_STATUS_FAIL) : void(0); // 校准失败
        m_caliing ? caliStatus.push_back(::tf::CaliStatus::type::CALI_STATUS_DOING) : void(0); // 正在校准
        m_caliStatusEmpty ? caliStatus.push_back(::tf::CaliStatus::CALI_STATUS_EMPTY) : void(0); // 校准空
        m_notyetCali ? caliStatus.push_back(::tf::CaliStatus::type::CALI_STATUS_NOT) : void(0); // 未校准
        m_caliEdit ? caliStatus.push_back(::tf::CaliStatus::type::CALI_STATUS_EDIT) : void(0); // 编辑
        if (!caliStatus.empty())
        {
            queryCond.__set_caliStatus(caliStatus);
        }
    }
    {
        std::vector<bool> typeReagent;
        m_closeType ? typeReagent.push_back(false) : void(0); // 封闭试剂
        m_openType ? typeReagent.push_back(true) : void(0); // 开放试剂
        if (!typeReagent.empty())
        {
            queryCond.__set_openReagent(typeReagent);
        }
    }

    // 必须没有删除
    queryCond.__set_beDeleted(false);
}

bool SelectConditions::SelUseStatusIf() const
{
	return m_inUse || m_backup || m_reagentShield || m_caliShield || m_notPlace || m_usageStatusEmpty;
}

bool SelectConditions::SelCaliStatusIf() const
{
	return m_caliSuccess || m_caliFailed || m_notyetCali || m_caliing || m_caliStatusEmpty || m_caliEdit;
}

bool SelectConditions::SelReagentTypeIf() const
{
	return m_closeType || m_openType || m_scanFailed;
}

QString SelectConditions::toString()
{
    // 判断筛选条件
    QStringList strUseStateList;
    if (m_inUse)
    {
        strUseStateList << QObject::tr("在用");
    }
    if (m_backup)
    {
        strUseStateList << QObject::tr("备用");
    }
    if (m_reagentShield)
    {
        strUseStateList << QObject::tr("试剂屏蔽");
    }
    if (m_caliShield)
    {
        strUseStateList << QObject::tr("校准屏蔽");
    }
    if (m_notPlace)
    {
        strUseStateList << QObject::tr("未放置");
    }
    if (m_usageStatusEmpty)
    {
        strUseStateList << QObject::tr("空");
    }

    // 校准条件
    QStringList strCaliStateList;
    if (m_caliSuccess)
    {
        strCaliStateList << QObject::tr("校准成功");
    }
    if (m_caliFailed)
    {
        strCaliStateList << QObject::tr("校准失败");
    }
    if (m_notyetCali)
    {
        strCaliStateList << QObject::tr("未校准");
    }
    if (m_caliing)
    {
        strCaliStateList << QObject::tr("正在校准");
    }
    if (m_caliStatusEmpty)
    {
        strCaliStateList << QObject::tr("空");
    }

    // 开闭与扫描条件
    QStringList strRgntTypeList;
    if (m_openType)
    {
        strRgntTypeList << QObject::tr("开放");
    }
    if (m_closeType)
    {
        strRgntTypeList << QObject::tr("封闭");
    }
    if (m_scanFailed)
    {
        strRgntTypeList << QObject::tr("扫描失败");
    }

    // 构造筛选字符串
    QString strCond("");
    QStringList strCondList;
    if (!strUseStateList.isEmpty())
    {
        strCondList.push_back(QObject::tr("使用状态") + QObject::tr("：") + strUseStateList.join(QObject::tr("，")));
    }
    if (!strCaliStateList.isEmpty())
    {
        strCondList.push_back(QObject::tr("校准状态") + QObject::tr("：") + strCaliStateList.join(QObject::tr("，")));
    }
    if (!strRgntTypeList.isEmpty())
    {
        strCondList.push_back(QObject::tr("试剂类型") + QObject::tr("：") + strRgntTypeList.join(QObject::tr("，")));
    }

    strCond += strCondList.join(QObject::tr("；") + "\n") + "\n";
    return strCond;
}

///
/// @brief  按钮重置
///
/// @par History:
/// @li 1226/zhangjing，2023年8月28日，新建函数
///
void  QSelectConditionDialog::OnReset()
{
    ui->status_current->setChecked(false);
    ui->status_backup->setChecked(false);
    ui->status_reagentmask->setChecked(false);
    ui->status_calimask->setChecked(false);
    ui->pushButtonNoPleace->setChecked(false);
    ui->pushButtonEmpty->setChecked(false);

    ui->calistatus_failed->setChecked(false);
    ui->calistatus_none->setChecked(false);
    ui->calistatus_succeed->setChecked(false);
    ui->pushButtonNotyetCali->setChecked(false);
    ui->pushButtonCaliing->setChecked(false);

    ui->pushButtonScanFailed->setChecked(false);
    ui->type_close->setChecked(false);
    ui->type_open->setChecked(false);
}

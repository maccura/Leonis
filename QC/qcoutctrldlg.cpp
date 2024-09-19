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
/// @file     qcoutctrldlg.cpp
/// @brief    质控失控处理对话框
///
/// @author   4170/TangChuXian
/// @date     2021年9月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qcoutctrldlg.h"
#include "ui_qcoutctrldlg.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/datetimefmttool.h"
#include "src/common/Mlog/mlog.h"
#include <QFontMetrics>
#include <QFont>
#include <QTimer>

QcOutCtrlDlg::QcOutCtrlDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_ciReasonMaxLength(200),
      m_ciSolutinMaxLength(200),
      m_pFocusEdit(Q_NULLPTR)
{
    // 初始化Ui对象
    ui = new Ui::QcOutCtrlDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

QcOutCtrlDlg::~QcOutCtrlDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief LJ初始化失控处理信息
///
/// @param[in]  stuQcResult  单个质控结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月17日，新建函数
///
void QcOutCtrlDlg::InitViewInfo(const QC_RESULT_INFO& stuQcResult)
{
	int pricisionNum = CommonInformationManager::GetInstance()->GetPrecisionNum(stuQcResult.strAssayName.toStdString());
	ui->labelResultShow->setText((QString::number(stuQcResult.dQcResult, 'f', pricisionNum)));
	ui->labelRuleShow->setText(stuQcResult.strOutCtrlRule);
	ui->labelTimeShow->setText(ToCfgFmtDateTime(stuQcResult.strQcTime));

	// 失控原因
	m_strLastValidReason = stuQcResult.strOutCtrlReason;
	ui->OutCtrlReasonEdit->setText(stuQcResult.strOutCtrlReason);

	// 处理措施
	m_strLastValidSolution = stuQcResult.strSolution;
	ui->SolutionEdit->setText(stuQcResult.strSolution);
}

///
/// @brief TP初始化失控处理信息
///
/// @param[in]  assayName  项目名称信息
/// @param[in]  stuQcResult  联合质控结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月17日，新建函数
///
void QcOutCtrlDlg::InitViewInfo(const std::string& assayName, const TWIN_QC_RESULT_INFO& stuQcResult)
{
	int pricisionNum = CommonInformationManager::GetInstance()->GetPrecisionNum(assayName);
	ui->labelResultShow->setText(QString::number(stuQcResult.dQcResult, 'f', pricisionNum));
	ui->labelRuleShow->setText(stuQcResult.strOutCtrlRule);
	ui->labelTimeShow->setText(ToCfgFmtDateTime(stuQcResult.strQcTime));
	
	// 失控原因
	m_strLastValidReason = stuQcResult.strOutCtrlReason;
	ui->OutCtrlReasonEdit->setText(stuQcResult.strOutCtrlReason);

	// 处理措施
	m_strLastValidSolution = stuQcResult.strSolution;
	ui->SolutionEdit->setText(stuQcResult.strSolution);
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcOutCtrlDlg::InitBeforeShow()
{
    // 初始化清空质控信息
    ui->labelResultShow->clear();
    ui->labelRuleShow->clear();
    ui->labelTimeShow->clear();

    // 失控规则文本更新
    connect(ui->labelRuleShow, SIGNAL(textChanged(const QString&)), this, SLOT(OnOutCtrlRuleValLabTextChanged(const QString&)));
    connect(ui->labelResultShow, SIGNAL(textChanged(const QString&)), this, SLOT(OnResultValLabTextChanged(const QString&)));

    // 更新字数
    ui->ReasonLetterCntLab->setText(QString::number(0) + "/" + QString::number(m_ciReasonMaxLength));
    ui->SlvLettertCntLab->setText(QString::number(0) + "/" + QString::number(m_ciSolutinMaxLength));

    // 安装事件过滤器
    ui->labelResultShow->installEventFilter(this);
    ui->labelRuleShow->installEventFilter(this);
    ui->OutCtrlReasonEdit->installEventFilter(this);
    ui->SolutionEdit->installEventFilter(this);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcOutCtrlDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void QcOutCtrlDlg::InitStrResource()
{
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcOutCtrlDlg::InitConnect()
{
    // 确认按钮被点击
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(accept()));

    // 取消按钮被点击
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 监听失控原因文本变化
    connect(ui->OutCtrlReasonEdit, SIGNAL(textChanged()), this, SLOT(OnReasonChanged()));

    // 失控处理措施文本变化
    connect(ui->SolutionEdit, SIGNAL(textChanged()), this, SLOT(OnSolutionChanged()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcOutCtrlDlg::InitChildCtrl()
{
    // 设置标题
    //SetTitleName(LoadStrFromLanguage(CHAR_CODE::IDS_QC_OUT_CTRL_HANDLE));
    // 设置标题
    SetTitleName(tr("失控处理"));

    // 更新字数
    ui->ReasonLetterCntLab->setText(QString::number(ui->OutCtrlReasonEdit->toPlainText().size()) + "/" + QString::number(m_ciReasonMaxLength));
    ui->SlvLettertCntLab->setText(QString::number(ui->SolutionEdit->toPlainText().size()) + "/" + QString::number(m_ciSolutinMaxLength));

    // 失控原因编辑框获得焦点
    ui->OutCtrlReasonEdit->setFocus();

    // 超出内容自动省略
    //QTimer::singleShot(0, this, SLOT(UpdateOutCtrlAndResultContent()));
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月13日，新建函数
///
void QcOutCtrlDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj    目标对象
/// @param[in]  event  事件
///
/// @return true表示已处理
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月4日，新建函数
///
bool QcOutCtrlDlg::eventFilter(QObject *obj, QEvent *event)
{
    // 判断对象
    // 质控结果
    if ((obj == ui->labelResultShow) && (event->type() == QEvent::Resize))
    {
        // 更新文本显示
        QTimer::singleShot(0, this, SLOT(UpdateRltElidedText()));
    }

    // 失控规则
    if ((obj == ui->labelRuleShow) && (event->type() == QEvent::Resize))
    {
        // 更新文本显示
        QTimer::singleShot(0, this, SLOT(UpdateRuleElidedText()));
    }

    // 失控原因文本编辑框获得焦点
    if ((obj == ui->OutCtrlReasonEdit) && (event->type() == QEvent::FocusIn))
    {
        // 更新光标位置
        m_pFocusEdit = qobject_cast<QTextEdit*>(obj);
        QTimer::singleShot(0, this, SLOT(MoveTextCursorToEnd()));
    }

    // 失控处理措施文本编辑框获得焦点
    if ((obj == ui->SolutionEdit) && (event->type() == QEvent::FocusIn))
    {
        // 更新光标位置
        m_pFocusEdit = qobject_cast<QTextEdit*>(obj);
        QTimer::singleShot(0, this, SLOT(MoveTextCursorToEnd()));
    }

    // 文本编辑框失去焦点
    if (!obj->inherits("QTextEdit") && (event->type() == QEvent::FocusIn))
    {
        // 更新光标位置
        m_pFocusEdit = Q_NULLPTR; 
    }

    // 交给基类处理
    return BaseDlg::eventFilter(obj, event);
}

///
/// @brief
///     质控结果文本改变
///
/// @param[in]  curText  当前文本
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月17日，新建函数
///
void QcOutCtrlDlg::OnResultValLabTextChanged(const QString& curText)
{
    // 记录原始文本
    m_strRlt = curText;

    // 设置提示
    ui->labelResultShow->setToolTip(m_strRlt);
}

///
/// @brief
///     失控规则文本改变
///
/// @param[in]  curText  当前文本
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月17日，新建函数
///
void QcOutCtrlDlg::OnOutCtrlRuleValLabTextChanged(const QString& curText)
{
    // 记录原始文本
    m_strRule = curText;

    // 设置提示
    ui->labelRuleShow->setToolTip(m_strRule);
}

///
/// @brief
///     更新失控和结果内容过长自动省略
///
/// @par History:
/// @li 4170/TangChuXian，2022年4月11日，新建函数
///
void QcOutCtrlDlg::UpdateOutCtrlAndResultContent()
{
    // 文本长度自适应
    OnResultValLabTextChanged(ui->labelResultShow->text());
    OnOutCtrlRuleValLabTextChanged(ui->labelRuleShow->text());
}

///
/// @brief
///     更新结果Elide文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
void QcOutCtrlDlg::UpdateRltElidedText()
{
    if (ui->horizontalSpacer->geometry().width() <= 2)
    {
        QString newText(m_strRlt);
        QFont defFont = ui->labelResultShow->font();
        defFont.setFamily(FONT_FAMILY);
        defFont.setPixelSize(FONT_SIZE_GENERAL);
        QFontMetrics fontMtcs(defFont);
        if (fontMtcs.width(m_strRlt) > ui->labelResultShow->width())
        {
            newText = fontMtcs.elidedText(m_strRlt, Qt::ElideRight, ui->labelResultShow->width(), Qt::TextSingleLine);
        }
        ui->labelResultShow->blockSignals(true);
        ui->labelResultShow->setText(newText);
        ui->labelResultShow->blockSignals(false);
    }
}

///
/// @brief
///     更新失控规则Elide文本
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
void QcOutCtrlDlg::UpdateRuleElidedText()
{
    if (ui->horizontalSpacer->geometry().width() <= 2)
    {
        QString newText(m_strRule);
        QFont defFont = ui->labelRuleShow->font();
        defFont.setFamily(FONT_FAMILY);
        defFont.setPixelSize(FONT_SIZE_GENERAL);
        QFontMetrics fontMtcs(defFont);
        if (fontMtcs.width(m_strRule) > ui->labelRuleShow->width())
        {
            newText = fontMtcs.elidedText(m_strRule, Qt::ElideRight, ui->labelRuleShow->width(), Qt::TextSingleLine);
        }
        ui->labelRuleShow->blockSignals(true);
        ui->labelRuleShow->setText(newText);
        ui->labelRuleShow->blockSignals(false);
    }
}

///
/// @brief
///     质控原因改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月25日，新建函数
///
void QcOutCtrlDlg::OnReasonChanged()
{
    // 超过最大字数限制则回退
    int iTextLength = ui->OutCtrlReasonEdit->toPlainText().size();
    if (iTextLength > m_ciReasonMaxLength)
    {
        ui->OutCtrlReasonEdit->blockSignals(true);
        ui->OutCtrlReasonEdit->setText(m_strLastValidReason);
        ui->OutCtrlReasonEdit->blockSignals(false);

		// 光标
		QTextCursor lastCursor = ui->OutCtrlReasonEdit->textCursor();
		m_iLastCursorPosOfReason = ui->OutCtrlReasonEdit->toPlainText().size();
		lastCursor.setPosition(m_iLastCursorPosOfReason);
		ui->OutCtrlReasonEdit->setTextCursor(lastCursor);
        iTextLength = m_strLastValidReason.size();
    }
    else
    {
        m_strLastValidReason = ui->OutCtrlReasonEdit->toPlainText();
        m_iLastCursorPosOfReason = ui->OutCtrlReasonEdit->textCursor().position();
    }

    // 更新字数
    ui->ReasonLetterCntLab->setText(QString::number(iTextLength) + "/" + QString::number(m_ciReasonMaxLength));
}

///
/// @brief
///     处理措施改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月25日，新建函数
///
void QcOutCtrlDlg::OnSolutionChanged()
{
    // 超过最大字数限制则回退
    int iTextLength = ui->SolutionEdit->toPlainText().size();
    if (iTextLength > m_ciSolutinMaxLength)
    {
        ui->SolutionEdit->blockSignals(true);
        ui->SolutionEdit->setText(m_strLastValidSolution);
        ui->SolutionEdit->blockSignals(false);

		// 光标
		QTextCursor lastCursor = ui->SolutionEdit->textCursor();
		m_iLastCursorPosOfSolution = ui->SolutionEdit->toPlainText().size();
		lastCursor.setPosition(m_iLastCursorPosOfSolution);
		ui->SolutionEdit->setTextCursor(lastCursor);
        iTextLength = m_strLastValidSolution.size();
    }
    else
    {
        m_strLastValidSolution = ui->SolutionEdit->toPlainText();
        m_iLastCursorPosOfSolution = ui->SolutionEdit->textCursor().position();
    }

    // 更新字数
    ui->SlvLettertCntLab->setText(QString::number(iTextLength) + "/" + QString::number(m_ciSolutinMaxLength));
}

///
/// @brief
///     将光标移动到末尾
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月19日，新建函数
///
void QcOutCtrlDlg::MoveTextCursorToEnd()
{
    // 检查当前焦点所在编辑框
    if (m_pFocusEdit == Q_NULLPTR)
    {
        return;
    }

    // 将光标设置到末尾
    QTextCursor lastCursor = m_pFocusEdit->textCursor();
    lastCursor.setPosition(m_pFocusEdit->toPlainText().size());
    m_pFocusEdit->setTextCursor(lastCursor);
}

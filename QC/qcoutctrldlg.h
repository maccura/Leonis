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
/// @file     qcoutctrldlg.h
/// @brief    质控失控处理对话框
///
/// @author   4170/TangChuXian
/// @date     2021年11月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年11月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "uidcsadapter/uidcsadapter.h"
#include <QTextEdit>
namespace Ui { class QcOutCtrlDlg; };

class QcOutCtrlDlg : public BaseDlg
{
    Q_OBJECT

public:
    QcOutCtrlDlg(QWidget *parent = Q_NULLPTR);
    ~QcOutCtrlDlg();

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
	void InitViewInfo(const QC_RESULT_INFO& stuQcResult);

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
	void InitViewInfo(const std::string& assayName, const TWIN_QC_RESULT_INFO& stuQcResult);

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月14日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月14日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月14日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月14日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月14日，新建函数
    ///
    void showEvent(QShowEvent *event);

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
    bool eventFilter(QObject *obj, QEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     质控结果文本改变
    ///
    /// @param[in]  curText  当前文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月17日，新建函数
    ///
    void OnResultValLabTextChanged(const QString& curText);

    ///
    /// @brief
    ///     失控规则文本改变
    ///
    /// @param[in]  curText  当前文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月17日，新建函数
    ///
    void OnOutCtrlRuleValLabTextChanged(const QString& curText);

    ///
    /// @brief
    ///     更新失控和结果内容过长自动省略
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年4月11日，新建函数
    ///
    void UpdateOutCtrlAndResultContent();

    ///
    /// @brief
    ///     更新结果Elide文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月25日，新建函数
    ///
    void UpdateRltElidedText();

    ///
    /// @brief
    ///     更新失控规则Elide文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月25日，新建函数
    ///
    void UpdateRuleElidedText();

    ///
    /// @brief
    ///     质控原因改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月25日，新建函数
    ///
    void OnReasonChanged();

    ///
    /// @brief
    ///     处理措施改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月25日，新建函数
    ///
    void OnSolutionChanged();

    ///
    /// @brief
    ///     将光标移动到末尾
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月19日，新建函数
    ///
    void MoveTextCursorToEnd();

private:
    Ui::QcOutCtrlDlg        *ui;                                    // UI对象指针
    bool                     m_bInit;                               // 是否已经初始化

    QString                  m_strRlt;                              // 结果字符串
    QString                  m_strRule;                             // 失控规则字符串
    QString                  m_strQcTime;                           // 质控时间字符串

    const int                m_ciReasonMaxLength;                   // 质控原因最大长度
    const int                m_ciSolutinMaxLength;                  // 解决方案最大长度
    QString                  m_strLastValidReason;                  // 上一次长度有效的失控原因
    QString                  m_strLastValidSolution;                // 上一次长度有效的失控处理措施
    int                      m_iLastCursorPosOfReason;              // 上一次长度有效的失控原因的光标位置
    int                      m_iLastCursorPosOfSolution;            // 上一次长度有效的失控处理措施的光标位置

    QTextEdit*               m_pFocusEdit;                          // 焦点编辑框

    friend class             QcGraphLJWidget;         // 声明友元类
    friend class             QcGraphYoudenWidget;     // 声明友元类
};

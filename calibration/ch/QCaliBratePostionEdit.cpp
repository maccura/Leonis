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
/// @file     QCaliBratePostionEdit.cpp
/// @brief    位置编辑
///
/// @author   5774/WuHongTao
/// @date     2022年3月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliBratePostionEdit.h"
#include "ui_QCaliBratePostionEdit.h"
#include "shared/ReagentCommon.h"
#include "manager/DictionaryQueryManager.h"
#include "src/public/DictionaryDecode.h"
#include "src/common/Mlog/mlog.h"
#include "CaliBrateCommom.h"

QCaliBratePostionEdit::QCaliBratePostionEdit(QWidget *parent)
    : BaseDlg(parent),
    m_Level(-1),
    m_pos(-1)
{
    ui = new Ui::QCaliBratePostionEdit();
    ui->setupUi(this);
    Init();
}

QCaliBratePostionEdit::~QCaliBratePostionEdit()
{
}

///
/// @brief
///     设置校准品的列表和选择的level
///
/// @param[in]  docs  校准品列表
/// @param[in]  level  水平
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QCaliBratePostionEdit::SetCaliDocListAndLevel(QList<ch::tf::CaliDoc>& docs, int level)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_Level = level;
    m_CaliDocs = docs;
}

///
/// @brief
///     获取校准文档列表
///
/// @param[out]  docs  校准文档列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QCaliBratePostionEdit::GetModifyCaliDoc(QList<ch::tf::CaliDoc>& docs)
{
    docs = m_CaliDocs;
}

///
/// @brief
///     初始化位置选中对话框
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QCaliBratePostionEdit::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 限制架号范围
	std::shared_ptr<RackNumRange> rackSharedPointer = DictionaryQueryManager::GetInstance()->GetRackRange();
    if (rackSharedPointer)
    {
        std::shared_ptr<QRegExp> regRxpSp = GenRegexpByText(rackSharedPointer->caliLowerLmt, rackSharedPointer->caliUpperLmt);
        QValidator *validatorRack = nullptr;
        if (regRxpSp == nullptr)
        {
            // 默认的规则
            QRegExp regxRack("^[0-9]*[1-9][0-9]*$");
            validatorRack = new QRegExpValidator(regxRack, ui->rack_edit);
        }
        else
        {
            validatorRack = new QRegExpValidator(*regRxpSp, ui->rack_edit);
        }

        ui->rack_edit->setValidator(validatorRack);
    }

    // 微型杯
    ui->cup_comboBox->addItem(tr("微型"),tf::TubeType::TUBE_TYPE_MICRO);
    // 正常杯
    ui->cup_comboBox->addItem(tr("标准"), tf::TubeType::TUBE_TYPE_NORMAL);
    // 无效
    ui->cup_comboBox->addItem(tr("无效"), tf::TubeType::TUBE_TYPE_INVALID);

    // 添加按钮list
    std::vector<QPushButton*> pushButtons;
    pushButtons.push_back(ui->one_Button);
    pushButtons.push_back(ui->two_Button);
    pushButtons.push_back(ui->three_Button);
    pushButtons.push_back(ui->four_Button);
    pushButtons.push_back(ui->five_Button);

    // 设置按钮属性
    int pos = 1;
    for (auto button : pushButtons)
    {
        connect(button, &QPushButton::clicked, this, [=]() {m_pos = pos; });
        pos++;
    }

	// 确认按钮
	connect(ui->ok_Button, SIGNAL(clicked()), this, SLOT(OnSavePostion()));
	// 关闭按钮
	connect(ui->close_Button, SIGNAL(clicked()), this, SLOT(close()));
	// 设置对话框的名称
	SetTitleName(tr("位置"));
}

///
/// @brief
///     确认按钮按下
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QCaliBratePostionEdit::OnSavePostion()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 若参数错误，直接退出
    if (m_pos < 0 || m_Level < 0 || m_CaliDocs.empty())
    {
        close();
        return;
    }

    // 架号
    int rackTmp = ui->rack_edit->text().toInt();
    int posTmp = m_pos;

	// 判断架构是否超出范围
	std::shared_ptr<RackNumRange> rackSharedPointer = DictionaryQueryManager::GetInstance()->GetRackRange();
	if (rackSharedPointer != nullptr && 
        (rackTmp < rackSharedPointer->caliLowerLmt || rackTmp > rackSharedPointer->caliUpperLmt))
	{
		close();
		return;
	}

	// 判断架号和位置是否重复设置（仅针对还未校准的校准品）
	// 1.避免和生化的架号和位置重复（需要提供给免疫thrift接口）
	// 2.避免和免疫的架号和位置重复（需要免疫提供thrift接口）

    // 依次修改后面水平的校准品的位置了类型
    for (int i = m_Level; i < m_CaliDocs.size(); i++)
    {
        if(posTmp > 5)
        { 
            posTmp = 1;
            rackTmp = rackTmp + 1;
        }

        // 修改位置
        m_CaliDocs[i].__set_rack(to_string(rackTmp));
        m_CaliDocs[i].__set_pos(posTmp);
        // 修改杯类型
        m_CaliDocs[i].__set_tubeType(static_cast<tf::TubeType::type>(ui->cup_comboBox->currentData().toInt()));
        posTmp++;
    }

    emit ModifyCaliPostion();
    close();
}

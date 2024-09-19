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

#include "QPushSampleToRemote.h"
#include "ui_QPushSampleToRemote.h"
#include "src/public/DictionaryKeyName.h"
#include "thrift/DcsControlProxy.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/SerializeUtil.hpp"

QPushSampleToRemote::QPushSampleToRemote(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QPushSampleToRemote();
    ui->setupUi(this);
    ui->comboBox_multi->hide();
    SetTitleName(tr("手工传输"));
    connect(ui->firstresult_btn_multi, &QPushButton::clicked, this, [&](){
		ui->comboBox_multi->setEnabled(false);
		ui->recheck_btn_multi->setChecked(false); // add by chenjianlin 20230906 bug修复 0021867: [工作] 手工传输界面数据类型能同时选择“检测结果”与“复查结果”
	});
    connect(ui->recheck_btn_multi, &QPushButton::clicked, this, [&]() {
		ui->comboBox_multi->setEnabled(true); 
		ui->firstresult_btn_multi->setChecked(false); // add by chenjianlin 20230906 bug修复 0021867: [工作] 手工传输界面数据类型能同时选择“检测结果”与“复查结果”
	});
    //connect(ui->cancel_btn_single, &QPushButton::clicked, this, [&]() {this->close(); });
    connect(ui->cancle_btn_multi, &QPushButton::clicked, this, [&]() {this->close(); });
    //connect(ui->ok_btn_single, &QPushButton::clicked, this, [&]() {this->SaveResult(); });
    connect(ui->ok_btn_multi, &QPushButton::clicked, this, [&]() {this->SaveResult(); });
    //connect(ui->transfer_set, &QCheckBox::clicked, this, [&]() 
    //{
    //    bool firstResult = ui->firstresult_btn_multi->isChecked();
    //    if (ui->transfer_set->isChecked())
    //    {
    //        if (firstResult)
    //        {
    //            m_workSet.transferManual = {true, 0};
    //        }
    //        else
    //        {
    //            m_workSet.transferManual = { true, 1 };
    //        }
    //    }
    //    else
    //    {
    //        if (firstResult)
    //        {
    //            m_workSet.transferManual = { false, 0 };
    //        }
    //        else
    //        {
    //            m_workSet.transferManual = { false, 1 };
    //        }
    //    }

    //    tf::DictionaryInfo di;
    //    std::string xml;
    //    // 设置数据
    //    Encode2Json(xml, m_workSet);
    //    di.__set_keyName(DKN_WORKPAGE_SET);
    //    di.__set_value(xml);

    //    // 添加数据字典信息
    //    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    //    {
    //        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
    //    }
    //    else
    //    {
    //        POST_MESSAGE(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE);
    //    }
    //});
}

QPushSampleToRemote::~QPushSampleToRemote()
{
}

///
/// @brief 设置页面类型
///
/// @param[in]  sampleKeys    选中样本数目
/// @param[in]  reCheckTimes  复查次数(-1表示没有复查)
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
void QPushSampleToRemote::SetPageAttribute(std::vector<int64_t> sampleKeys, int maxTestCnt)
{
    if (sampleKeys.empty())
    {
        return;
    }

    m_sampleSelcts = sampleKeys;
    ui->stackedWidget->setCurrentIndex(1);
}

///
/// @brief 设置当前手工传输设置（包含页面所有设置）
///
/// @param[in]  workSet  工作页面的显示设置
///
/// @return true:设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月20日，新建函数
///
bool QPushSampleToRemote::SetWorkSet(PageSet& workSet)
{
    m_workSet = workSet;

    if (!m_workSet.transferManual.first)
    {
        ui->transfer_set->setChecked(true);
    }
    else
    {
        ui->transfer_set->setChecked(false);
    }
	// 如果是传输首次结果
    if (0 == m_workSet.transferManual.second) 
    {
        ui->firstresult_btn_multi->setChecked(true);
		ui->recheck_btn_multi->setChecked(false);// add by chenjianlin 20230906 bug修复 0021867: [工作] 手工传输界面数据类型能同时选择“检测结果”与“复查结果”
    }
    else // 如果是传输复查结果
    {
        ui->recheck_btn_multi->setChecked(true);
        ui->firstresult_btn_multi->setChecked(false);// add by chenjianlin 20230906 bug修复 0021867: [工作] 手工传输界面数据类型能同时选择“检测结果”与“复查结果”
    }
    return true;
}

///
/// @brief 保存选择结果
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
void QPushSampleToRemote::SaveResult()
{
    // 点击保存按钮后，需要把当前选中设置进行存储。点击取消，则不需要存储。
    // 保存时，检查当前设置和保存的设置是否一致，不一致则需要更新
    bool isSetChecked = ui->transfer_set->isChecked();
    // 上传初查:0  上传复查：1
    int firstResult = !(ui->firstresult_btn_multi->isChecked());

    if ((m_workSet.transferManual.first != !isSetChecked)   ||
        (m_workSet.transferManual.second != firstResult))
    {
        m_workSet.transferManual = { !isSetChecked , firstResult };
        UpdateWorkSet();
    }

    // 单选样本
    //if (m_sampleSelcts.size() == 1)
    //{
        if (ui->firstresult_btn_multi->isEnabled() && ui->firstresult_btn_multi->isChecked())
        {
            emit selectResultIndex(false);
        }
        else if (ui->recheck_btn_multi->isEnabled() && ui->recheck_btn_multi->isChecked())
        {
            emit selectResultIndex(true/*, ui->comboBox_multi->currentData().toInt()*/);
        }
    //}
    //// 多选样本
    //else if (m_sampleSelcts.size() > 1)
    //{
    //    if (ui->firstresult_btn_single->isEnabled() && ui->firstresult_btn_single->isChecked())
    //    {
    //        emit selectResultIndex(false);
    //    }
    //    else if (ui->recheck_btn_single->isEnabled() && ui->recheck_btn_single->isChecked())
    //    {
    //        emit selectResultIndex(true);
    //    }
    //}

    close();
}

void QPushSampleToRemote::UpdateWorkSet()
{
    tf::DictionaryInfo di;
    std::string xml;
    // 设置数据
    Encode2Json(xml, m_workSet);
    di.__set_keyName(DKN_WORKPAGE_SET);
    di.__set_value(xml);

    // 添加数据字典信息
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
    }
    else
    {
        POST_MESSAGE(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE);
    }
}

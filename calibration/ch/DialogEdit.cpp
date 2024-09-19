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
/// @file     DialogEdit.h
/// @brief    位置信息编辑
///
/// @author   5774/WuHongTao
/// @date     2020年6月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DialogEdit.h"
#include "ui_DialogEdit.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/DictionaryKeyName.h"
#include "src/common/DictionaryDecode.h"

///
/// @brief 构造函数
///     
/// @param[in]  parent  父节点
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
DialogEdit::DialogEdit(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::DialogEdit();
    ui->setupUi(this);

    //设置标题
    SetTitleName(tr("PostionEdit"));
    //修改
    connect(ui->yesButton, SIGNAL(clicked()), this, SLOT(modifyDeal()));
    //取消
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    //关闭
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearInfo()));

    // 限制位置范围
    QRegExp regxPostion("[1-9][0]");
    QValidator *validatorPostion = new QRegExpValidator(regxPostion, ui->pos);
    ui->pos->setValidator(validatorPostion);

    // 限制架号范围
    QRegExp regxRack("^[0-9]*[1-9][0-9]*$");
    QValidator *validatorRack = new QRegExpValidator(regxRack, ui->rack);
    ui->rack->setValidator(validatorRack);
}

///
/// @brief 析构函数
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
DialogEdit::~DialogEdit()
{
    delete ui;
}

///
/// @brief 执行具体的修改操作
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void DialogEdit::modifyDeal()
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond qryCond;
    ::tf::DictionaryInfoQueryResp qryResp;
    qryCond.__set_keyName(DKN_RACK_NUM_RANGE);

    // 查询当前架号范围
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo() failed!");
        return;
    }

    //解析架号范围
    RackNumRange rnr;
    if (!DecodeRackNumReange(qryResp.lstDictionaryInfos[0].value, rnr))
    {
        ULOG(LOG_ERROR, "DecodeRackNumReange() failed! %s", qryResp.lstDictionaryInfos[0].value);
        return;
    }

    // 获取架号和位置号
    int pos = ui->pos->text().toInt();
    int rack = ui->rack->text().toInt();

    // 检查范围是否符合要求
    if ((rack >= rnr.caliLowerLmt) && (rack <= rnr.caliUpperLmt))
    {
        emit closeWindowEdit(rack, pos);
    }

    this->close();
}

///
/// @brief
///     清除位置信息
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月17日，新建函数
///
void DialogEdit::clearInfo()
{
    ui->pos->clear();
    ui->rack->clear();
    this->close();
}

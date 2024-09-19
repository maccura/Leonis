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
/// @file     BackupOrderAddAssayDlg.cpp
/// @brief    应用--检测--备用订单--增加项目
///
/// @author   7951/LuoXin
/// @date     2022年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "BackupOrderAddAssayDlg.h"
#include "ui_BackupOrderAddAssayDlg.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "src/common/Mlog/mlog.h"

BackupOrderAddAssayDlg::BackupOrderAddAssayDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::BackupOrderAddAssayDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

BackupOrderAddAssayDlg::~BackupOrderAddAssayDlg()
{

}

void BackupOrderAddAssayDlg::GetUserConfigData(std::vector<int32_t>& selectAssayCodes, std::vector<int64_t>& profiles)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 单项目
	std::vector<::tf::TestItem> testItems;
    ui->container->GetSampleTestItemInfo(testItems);
    for (auto& item : testItems)
    {
        selectAssayCodes.push_back(item.assayCode);
    }
     
    // 组合项目
    ui->container->GetProfiles(profiles);   
}

QPushButton* BackupOrderAddAssayDlg::GetSaveBtnPtr()
{
    return ui->save_btn;
}

void BackupOrderAddAssayDlg::ShowAddAssayDlg(const std::vector<int32_t>& assayCodes, std::vector<int64_t>& profiles, int sampleSource)
{


    // 单项目
    std::vector<std::shared_ptr<tf::TestItem>> testItems;
    for (int32_t code : assayCodes)
    {      
        auto spesTitem = std::make_shared<tf::TestItem>();
        spesTitem->__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
        spesTitem->__set_assayCode(code);
        testItems.push_back(std::move(spesTitem));
    }

    show();
	// modify bug0024651 by wuht
	ui->container->UpdateTestItemStatus(testItems);
	// 样本源
	ui->container->UpdateAssayProfileStatus(profiles);
	ui->container->SetCurrentSampleType(sampleSource);

}

void BackupOrderAddAssayDlg::InitCtrls()
{
	BaseDlg::SetTitleName(tr("项目添加"));

    // 设置类型
    KeyDataIn setData;
    setData.assayType = AssayTypeNew::ASSAYTYPE2;
    ui->container->SetKeyStatus(setData);

	// 项目选择变化
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, ui->container, OnUpdateView);
}

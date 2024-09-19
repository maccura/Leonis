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
/// @file     QCaliCurveCover.h
/// @brief    校准覆盖
///
/// @author   5774/WuHongTao
/// @date     2022年3月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliCurveCover.h"
#include "ui_QCaliCurveCover.h"
#include "shared/tipdlg.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"

QCaliCurveCover::QCaliCurveCover(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QCaliCurveCover();
    ui->setupUi(this);
    connect(ui->ok_Button, SIGNAL(clicked()), this, SLOT(OnCoverCurrentCurve()));
    // 对话框关闭
    connect(ui->Cancel_button, SIGNAL(clicked()), this, SLOT(close()));
}

QCaliCurveCover::~QCaliCurveCover()
{
}

///
/// @brief
///    设置曲线列表
///
/// @param[in]  curves  曲线列表（第一条表示选中曲线，第二条表示历史曲线） 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliCurveCover::SetCurveList(std::vector<ch::tf::CaliCurve>& curves, ch::tf::ReagentGroup& group)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_curveList.clear();
    m_curveList = curves;
    m_ReagentGroup = group;
    if (curves.empty())
    {
        ULOG(LOG_ERROR, "%s(), the curve is empty", __FUNCTION__);
        return;
    }

    m_caliCurve = nullptr;
    // 默认设置选中曲线
    ui->Select_Button->setChecked(true);
}

///
/// @brief
///     覆盖当前工作曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliCurveCover::OnCoverCurrentCurve()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int index = 0;
    // 选中曲线
    if (ui->Select_Button->isChecked())
    {
        index = 0;
    }

    // 历史曲线
    if (ui->His_Button->isChecked())
    {
        index = 1;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (index >= m_curveList.size())
    {
        close();
        return;
    }

    ch::tf::ReagentGroup group;
    group.__set_id(m_ReagentGroup.id);
    group.__set_caliCurveId(m_curveList[index].id);
    group.__set_caliCurveSource(tf::CurveSource::CURVE_SOURCE_BOTTLE);
    if (!ch::c1005::LogicControlProxy::ModifyReagentGroup(group))
    {
        ULOG(LOG_ERROR, "%s(%s)", __FUNCTION__, "Modify reagentGroup failed");
    }

    close();
}

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
/// @file     mctubeslot.cpp
/// @brief    试剂盘槽位
///
/// @author   4170/TangChuXian
/// @date     2020年4月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "immctubeslot.h"
#include <QPainter>
#include <QImage>

#define  REAGENT_TUBE_RECT       QRect(0, 0, 45, 118)           // 试管绘制区域
#define  REAGENT_MASK_RECT       QRect(17, 89, 13, 13)          // 试剂屏蔽标志绘制区域
#define  REAGENT_BACKUP_NUM_RECT QRect(16, 102, 13, 13)         // 试剂备份编号

ImMcTubeSlot::ImMcTubeSlot(qreal dAngle, QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_bIsChecked(false),
      m_bShowBackupNo(false),
      m_fAngle(dAngle)
{
}

ImMcTubeSlot::~ImMcTubeSlot()
{
}

///
/// @brief
///     设置是否选中
///
/// @param[in] bChecked 是否选中
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcTubeSlot::SetChecked(bool bChecked)
{
    m_bIsChecked = bChecked;
}

///
/// @brief
///     获取是否选中
///
/// @param
///
/// @return true表示选中，false表示未选中
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
bool ImMcTubeSlot::IsChecked()
{
    return m_bIsChecked;
}

///
/// @brief
///     设置是否显示备用瓶编号
///
/// @param[in]  bShow  是否显示
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月4日，新建函数
///
void ImMcTubeSlot::SetShowBackupNo(bool bShow)
{
    m_bShowBackupNo = bShow;
}

///
/// @brief
///     是否显示备用瓶编号
///
/// @return true表示显示
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月4日，新建函数
///
bool ImMcTubeSlot::IsShowBackupNo()
{
    return m_bShowBackupNo;
}

///
/// @brief
///     装载试剂
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcTubeSlot::LoadReagent(const ImReagentInfo& info)
{
    m_stuReagentInfo = info;
}

///
/// @brief
///     获取试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月30日，新建函数
///
const boost::optional<ImReagentInfo>& ImMcTubeSlot::GetReagentInfo()
{
    return m_stuReagentInfo;
}

///
/// @brief
///     卸载试剂
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月14日，新建函数
///
void ImMcTubeSlot::UnloadReagent()
{
    m_stuReagentInfo = ImReagentInfo();
}

///
/// @brief
///     边界矩形
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
QRectF ImMcTubeSlot::boundingRect() const
{
    return REAGENT_TUBE_RECT;
}

///
/// @brief
///     重绘函数
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月15日，新建函数
///
void ImMcTubeSlot::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= Q_NULLPTR*/)
{
    // 保存画家对象
    painter->save();

    // 反走样
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    // 画试剂
    PaintRgnt(painter);

    // 恢复画家对象
    painter->restore();
}

///
/// @brief
///     画试剂
///
/// @param[in]  painter  画家对象
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月24日，新建函数
///
void ImMcTubeSlot::PaintRgnt(QPainter *painter)
{
    // 没有试剂信息，是一个空位
    if (!m_stuReagentInfo.has_value())
    {
        // 只需要画空位（待完善 选中效果处理）
        painter->drawImage(REAGENT_TUBE_RECT, QImage(":/venus/resource/image/empty.png"));
        return;
    }

    // 有试剂信息
    QImage reagentImg;
    if (m_stuReagentInfo->bDillute)
    {
        if (m_stuReagentInfo->bMask)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/dlt_mask_sel.png") : QString(":/Leonis/resource/image/im/dlt_mask.png"));
        }
        else if (m_stuReagentInfo->enState == USING)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/dlt_using_sel.png") : QString(":/Leonis/resource/image/im/dlt_using.png"));
        }
        else if (m_stuReagentInfo->enState == EMPTY)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_empty_sel.png") : QString(":/Leonis/resource/image/im/rgnt_empty.png"));
        }
        else if (m_stuReagentInfo->enState == EMPTY_BOTTLE)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/empty-dlt-sel.png") : QString(":/Leonis/resource/image/im/empty-dlt.png"));
        }
        else if (m_stuReagentInfo->enState == NOTICE)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/dlt_warn_sel.png") : QString(":/Leonis/resource/image/im/dlt_warn.png"));
        }
        else if (m_stuReagentInfo->enState == BACKUP)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/dlt_backup_sel.png") : QString(":/Leonis/resource/image/im/dlt_backup.png"));
        }
        else if (m_stuReagentInfo->enState == WARNING)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/dlt_error_sel.png") : QString(":/Leonis/resource/image/im/dlt_error.png"));
        }
    }
    else
    {
        if (m_stuReagentInfo->bMask)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_mask_sel.png") : QString(":/Leonis/resource/image/im/rgnt_mask.png"));
        }
        else if (m_stuReagentInfo->enState == USING)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_using_sel.png") : QString(":/Leonis/resource/image/im/rgnt_using.png"));
        }
        else if (m_stuReagentInfo->enState == EMPTY)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_empty_sel.png") : QString(":/Leonis/resource/image/im/rgnt_empty.png"));
        }
        else if (m_stuReagentInfo->enState == EMPTY_BOTTLE)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/empty-rgnt-sel.png") : QString(":/Leonis/resource/image/im/empty-rgnt.png"));
        }
        else if (m_stuReagentInfo->enState == SCAN_FAILED)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/scan-failed-rgnt-sel.png") : QString(":/Leonis/resource/image/im/scan-failed-rgnt.png"));
        }
        else if (m_stuReagentInfo->enState == NOTICE)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_warn_sel.png") : QString(":/Leonis/resource/image/im/rgnt_warn.png"));
        }
        else if (m_stuReagentInfo->enState == BACKUP)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_backup_sel.png") : QString(":/Leonis/resource/image/im/rgnt_backup.png"));
        }
        else if (m_stuReagentInfo->enState == WARNING)
        {
            reagentImg.load(m_bIsChecked ? QString(":/Leonis/resource/image/im/rgnt_error_sel.png") : QString(":/Leonis/resource/image/im/rgnt_error.png"));
        }
    }

    // 画试剂
    painter->drawImage(REAGENT_TUBE_RECT, reagentImg);

    // 画试剂遮蔽
//     if (m_stuReagentInfo->bMask)
//     {
//         // 选中为正常视角
//         QPoint ptMaskCenter = REAGENT_MASK_RECT.center();
//         painter->translate(ptMaskCenter);
//         painter->rotate(-m_fAngle);
//         painter->translate(-ptMaskCenter);
//         painter->drawImage(REAGENT_MASK_RECT, QImage(":/Leonis/resource/image/rgnt_mask_icon.png"));
// 
//         // 恢复视角
//         painter->translate(ptMaskCenter);
//         painter->rotate(m_fAngle);
//         painter->translate(-ptMaskCenter);
//     }

    // 画备用试剂编号
    if (m_stuReagentInfo->iBackupNo > 0 && m_bShowBackupNo)
    {
        // 选中为正常视角
        QPoint ptBackupRgntCenter = REAGENT_BACKUP_NUM_RECT.center();
        painter->translate(ptBackupRgntCenter);
        painter->rotate(-m_fAngle);
        painter->translate(-ptBackupRgntCenter);

        // 设置字体
        QFont numFont = painter->font();
        QPen numPen = painter->pen();
        numFont.setPixelSize(10);
        numFont.setBold(true);
        QColor numColor(Qt::black);
        if (m_stuReagentInfo->enState == USING)
        {
            numColor.setRgb(0x45, 0x9f, 0xff);
        }
        else if (m_stuReagentInfo->enState == BACKUP)
        {
            numColor.setRgb(0x97, 0xa9, 0xc3);
        }
        else if (m_stuReagentInfo->enState == NOTICE)
        {
            numColor.setRgb(0xf2, 0xa2, 0x46);
        }
        else if (m_stuReagentInfo->enState == WARNING)
        {
            numColor.setRgb(0xf5, 0x7c, 0x7d);
        }
        numPen.setColor(numColor);

        // 设置颜色和字体
        painter->setFont(numFont);
        painter->setPen(numPen);

        // 画备用试剂编号
        painter->drawText(REAGENT_BACKUP_NUM_RECT, QString::number(m_stuReagentInfo->iBackupNo), Qt::AlignVCenter | Qt::AlignHCenter);
    }
}

///
/// @brief
///     获取item类型
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
int ImMcTubeSlot::type() const
{
    return ReagentSlotType;
}

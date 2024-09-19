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
/// @file     mctubeslot.h
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

#pragma once

#include <QGraphicsItem>
#include <QVariant>
#include <boost/optional.hpp>

// 试剂状态
enum ImReagentState
{
    USING = 0,          // 在用
    BACKUP,             // 备用
    NOTICE,             // 提醒
    WARNING,            // 警告
    EMPTY,              // 空位
    EMPTY_BOTTLE,       // 空瓶
    SCAN_FAILED         // 扫描失败
};

// 试剂信息
struct ImReagentInfo
{
    bool           bMask;        // 是否屏蔽
    bool           bDillute;     // 是否是稀释液
    int            iBackupNo;    // 备用试剂编号
    ImReagentState enState;      // 状态
    QVariant       userData;     // 用户自定义数据

    ImReagentInfo()
    {
        bMask = false;
        bDillute = false;
        iBackupNo = 0;
        enState = EMPTY;
    }
};

class ImMcTubeSlot : public QGraphicsItem
{
public:
    ImMcTubeSlot(qreal dAngle, QGraphicsItem *parent = Q_NULLPTR);
    ~ImMcTubeSlot();

    // item类型
    enum 
    {
        ReagentSlotType = QGraphicsItem::UserType + 1
    };

    ///
    /// @brief
    ///     设置是否选中
    ///
    /// @param[in] bChecked 是否选中
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void SetChecked(bool bChecked);

    ///
    /// @brief
    ///     获取是否选中
    ///
    /// @return true表示选中，false表示未选中
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    bool IsChecked();

    ///
    /// @brief
    ///     设置是否显示备用瓶编号
    ///
    /// @param[in]  bShow  是否显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月4日，新建函数
    ///
    void SetShowBackupNo(bool bShow);

    ///
    /// @brief
    ///     是否显示备用瓶编号
    ///
    /// @return true表示显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月4日，新建函数
    ///
    bool IsShowBackupNo();

    ///
    /// @brief
    ///     装载试剂
    ///
    /// @param[in] info    装载试剂的试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void LoadReagent(const ImReagentInfo& info);

    ///
    /// @brief
    ///     获取试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月30日，新建函数
    ///
    const boost::optional<ImReagentInfo>& GetReagentInfo();

    ///
    /// @brief
    ///     卸载试剂
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void UnloadReagent();

    ///
    /// @brief
    ///     边界矩形
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月15日，新建函数
    ///
    QRectF boundingRect() const;

    ///
    /// @brief
    ///     获取item类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    int type() const;

protected:
    ///
    /// @brief
    ///     重绘函数
    ///
    /// @param
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月15日，新建函数
    ///
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

    ///
    /// @brief
    ///     画试剂
    ///
    /// @param[in]  painter  画家对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月24日，新建函数
    ///
    void PaintRgnt(QPainter *painter);

private:
    bool                             m_bIsChecked;           // 是否被选中
    bool                             m_bShowBackupNo;        // 是否显示备用瓶编号
    qreal                            m_fAngle;               // 试剂偏移角度
    boost::optional<ImReagentInfo>   m_stuReagentInfo;       // 试剂信息
};

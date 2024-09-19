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
/// @file     immcreagentplate.h
/// @brief    试剂盘控件
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
#include <QWidget>
#include <QVector>
#include <QSet>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "boost/optional.hpp"

class ImMcReagentPlateItem;
class ImMcTubeSlot;
struct ImReagentInfo;

// 试剂盘场景
class ImMcReagentPlateScene : public QGraphicsScene
{
    Q_OBJECT

public:
    ImMcReagentPlateScene(QObject *parent = Q_NULLPTR){}
    ~ImMcReagentPlateScene(){}
};

// 试剂盘视图
class ImMcReagentPlate : public QGraphicsView
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in] iCount  试剂盘槽位总数
    /// @param[in] fDegree 试剂盘偏移角度
    /// @param[in] parent  父窗口指针
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    ImMcReagentPlate(int iCount, qreal fDegree, QWidget* parent = Q_NULLPTR);
    ~ImMcReagentPlate();

    ///
    /// @brief
    ///     获取试剂盘槽位总数
    ///
    /// @param
    ///
    /// @return 返回试剂盘可容纳试管总数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    int GetTubeSlotCount();

    ///
    /// @brief
    ///     设置选中项索引
    ///
    /// @param[in] iSelIndex 选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void SetSelIndex(int iSelIndex);

    ///
    /// @brief
    ///     获取选中项索引
    ///
    /// @param
    ///
    /// @return 返回试剂盘选中位置的索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    int GetSelIndex();

    ///
    /// @brief
    ///     设置选中项详情
    ///
    /// @param[in] strSelDetail 选中项详情
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void SetSelDetail(const QString& strSelDetail);

    ///
    /// @brief
    ///     获取选中项详情
    ///
    /// @param
    ///
    /// @return 返回选中位置的详细信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    QString GetSelDetail();

    ///
    /// @brief
    ///     装载试剂
    ///
    /// @param[in] iIndex  装载试剂的槽位索引
    /// @param[in] info    装载试剂的试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void LoadReagent(int iIndex, const ImReagentInfo& info);

    ///
    /// @brief
    ///     获取试剂信息
    ///
    /// @param[in] iIndex  装载试剂的槽位索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月30日，新建函数
    ///
    const boost::optional<ImReagentInfo> GetReagentInfo(int iIndex);

    ///
    /// @bref
    ///		设置指示器
    ///
    /// @param[in] selIndex 需要被指示的槽位索引
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月12日，新建函数
    ///
    void SetIndicator(const QSet<int>& selIndex);

    ///
    /// @brief
    ///     更新备用瓶编号显示
    ///
    /// @param[in]  selIndex  指示器索引列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月4日，新建函数
    ///
    void UpdateBackNoVisible(const QSet<int>& selIndex);

    ///
    /// @brief
    ///     卸载试剂
    ///
    /// @param[in] iIndex  卸载试剂的槽位索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void UnloadReagent(int iIndex);

    ///
    /// @brief
    ///     重置试剂
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月18日，新建函数
    ///
    void ReseReagent();

protected:
    ///
    /// @brief
    ///     重写大小改变事件
    ///
    /// @param[in] iIndex  卸载试剂的槽位索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    void resizeEvent(QResizeEvent *event);

    ///
    /// @brief
    ///     事件过滤器（主要用于监听场景事件）
    ///
    /// @param[in] obj    事件的目标对象
    /// @param[in] event  事件对象
    ///
    /// @return true表示事件已处理，false表示事件未处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月14日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
    ///
    /// @brief 选中索引发生改变
    ///     
    /// @param[in]  curItemIndex   当前选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void selectedItemChange(int curItemIndex);

private:
    int                     m_iTubeSlotCnt;       // 试管槽位总数
    int                     m_iSelIndex;          // 选中项索引
    QString                 m_strSelDetail;       // 选中项详情
    QVector<ImMcTubeSlot*>  m_vTubeSlot;          // 试管槽位
    ImMcReagentPlateItem*   m_pReagentPlateItem;  // 试剂盘底盘

    ImMcReagentPlateScene   m_scene;              // 内部场景
};

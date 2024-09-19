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
/// @file     imliquidbottle.h
/// @brief    储液瓶（主要用于显示耗材剩余量）
///
/// @author   4170/TangChuXian
/// @date     2022年8月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include <QMap>

class ImLiquidBottleWidget : public QWidget
{
    Q_OBJECT

public:
    // 储液瓶类型信息
    struct BottleTypeInfo
    {
        bool     bPlaced;                          // 是否放置
        int      iBottleType;                      // 储液瓶类型
        QPoint   ptFillLiquidTopLeft;              // 满液体时液体左上角坐标
        QPixmap  BottleImg;                        // 储液瓶图片
        QPixmap  LiquidImg;                        // 储液瓶液体图片

        BottleTypeInfo(int iType, QPoint ptTopLeft, QPixmap BottlePixmap, QPixmap LiquidPixmap, bool bIsPlaced = true)
        {
            bPlaced = bIsPlaced;
            iBottleType = iType;
            ptFillLiquidTopLeft = ptTopLeft;
            BottleImg = BottlePixmap;
            LiquidImg = LiquidPixmap;
        }

        bool IsValid() const
        {
            return ((iBottleType >= 0) && !BottleImg.isNull());
        }
    };

public:
    ImLiquidBottleWidget(QWidget *parent = Q_NULLPTR);
    ~ImLiquidBottleWidget();

    ///
    /// @brief
    ///     注册储液瓶类型信息
    ///
    /// @param[in]  stuBottleTypeInfo  储液瓶类型信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    static bool RegisterBottleType(const BottleTypeInfo& stuBottleTypeInfo);

    ///
    /// @brief
    ///     反注册储液瓶类型
    ///
    /// @param[in]  iBottleType  储液瓶类型
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    static bool UnRegisterBottleType(int iBottleType);

    ///
    /// @brief
    ///     设置储液瓶类型 
    ///
    /// @param[in]  iBottleType  储液瓶类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    bool SetBottleType(int iBottleType);

    ///
    /// @brief
    ///     获取储液瓶类型
    ///
    /// @return 储液瓶类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    int GetBottleType();

    ///
    /// @brief
    ///     设置液体余量
    ///
    /// @param[in]  dRatio  1为满，0为空
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    bool SetLiquidRemainRatio(double dRatio);

    ///
    /// @brief
    ///     获取液体剩余量
    ///
    /// @return 液体剩余量 1为满，0为空
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    double GetLiquidRemainRatio();

    ///
    /// @brief
    ///     设置余量文本
    ///
    /// @param[in]  strRemainVol  余量文本
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    bool SetRemainVolText(const QString& strRemainVol);

    ///
    /// @brief
    ///     获取余量文本
    ///
    /// @return 余量文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    const QString& GetRemainVolText();

protected:
    ///
    /// @brief
    ///     重写重绘事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月18日，新建函数
    ///
    virtual void paintEvent(QPaintEvent *event) override;

private:
    int                                 m_iBottleType;              // 储液瓶类型
    double                              m_dRemainRatio;             // 余量比例
    QString                             m_strRemainVolText;         // 余量文本

    static QMap<int, BottleTypeInfo>    sm_mapBottleTypeInfo;       // 储液瓶信息映射
};

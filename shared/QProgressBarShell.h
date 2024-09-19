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
/// @file     QProgressBarShell.h
/// @brief    酸性碱性清洗液的进度条
///
/// @author   5774/WuHongTao
/// @date     2022年1月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QPushButton>
#include <QWidget>
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"


namespace Ui {
    class QProgressSmpClnBar;
    class QProgressBarShell;
    class IseProgressBar;
};


class QSupplyProgress : public QWidget
{
    Q_OBJECT
public:

    struct SupplyStatus
    {
        QPixmap pixmap;
        QString shellPixmap;
        bool isShowBtn;
        int progress;
        int blance;

        SupplyStatus()
        {
            isShowBtn = true;
            shellPixmap = "";
            progress = 0;
            blance = 0;
        }
    };

    QSupplyProgress(QWidget *parent = Q_NULLPTR);
    ~QSupplyProgress() {};

    ///
    /// @brief 设置仓外耗材状态
    ///
    /// @param[in]  status  耗材状态
    ///
    /// @return true 返回成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月17日，新建函数
    ///
    virtual bool SetSupplyStatus(SupplyStatus status);

    ///
    /// @brief 获取当前耗材的状态
    ///
    ///
    /// @return 耗材状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月17日，新建函数
    ///
    SupplyStatus GetStatus() { return m_supplyStatus; };

signals:
    void focusOut();

private:
    ///
    /// @brief 显示耗材状态
    ///
    /// @param[in]  status  耗材状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月17日，新建函数
    ///
    virtual void ShowSupplyStatus(SupplyStatus status);

protected:
    SupplyStatus                    m_supplyStatus;             ///< 耗材状态
    Ui::QProgressSmpClnBar*         ui;
};


class QProgressSmpClnBarShell : public QSupplyProgress
{

public:
    QProgressSmpClnBarShell(QWidget *parent = Q_NULLPTR);
    ~QProgressSmpClnBarShell() {};
};

class QProgressBarShell : public QPushButton
{
    Q_OBJECT

public:
    // 仓外耗材类型
    enum SUPPLYTYPE {
        ACIDITY,
        ALKALINITY,
        SAMPLEWASH,
        WASTELIQUID,
        EMPTY
    };

    // 仓外耗材状态
    enum SUPPLSTATUS {
        CURRENT,
        BACKUP,
        SCANFAILED,
        EXPIRED,
        UNKONW
    };

    struct SupplyStatus
    {
        SUPPLYTYPE supplyType;
        SUPPLSTATUS status;
        ch::tf::SuppliesInfo supplyInfo;
        int progress;
        int blance;

        SupplyStatus()
        {
            supplyType = SUPPLYTYPE::EMPTY;
            status = SUPPLSTATUS::UNKONW;
            progress = 0;
            blance = 0;
        }
    };

    QProgressBarShell(QWidget *parent = Q_NULLPTR);
    ~QProgressBarShell();

    ///
    /// @brief 获取耗材状态
    ///
    ///
    /// @return 耗材状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月22日，新建函数
    ///
    SupplyStatus GetStatus();

    ///
    /// @brief 设置耗材状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void SetSupplyStatus(SupplyStatus status);

signals:
    void focusOut();
    void detectVolumn();

private:
    ///
    /// @brief 处理焦点失去的事件
    ///
    /// @param[in]  event  事件信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void focusOutEvent(QFocusEvent *event);

    ///
    /// @brief 酸性清洗液的状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月20日，新建函数
    ///
    void ComputAcidityStatusSupply(SupplyStatus status);

    ///
    /// @brief 碱性清洗液的状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月21日，新建函数
    ///
    void ComputAlkalinityStatusSupply(SupplyStatus status);

    ///
    /// @brief 样本针清洗液状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月21日，新建函数
    ///
    void ComputSampleWashAcidityStatusSupply(SupplyStatus status);

    ///
    /// @brief 废液桶状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月21日，新建函数
    ///
    void ComputWasteLiquidStatusSupply(SupplyStatus status);

private:
    Ui::QProgressBarShell*          ui;
    SupplyStatus                    m_supplyStatus;             ///< 耗材状态
    int                             m_bootleRemind;
    bool                            m_init;                     ///< 是否初始化
};

class QProgressIseBarShell : public QPushButton
{
    Q_OBJECT

public:
    // 仓外耗材类型
    enum SUPPLYTYPE {
        SAMPLEWASH,
        ISEIS,
        ISEBUF,
        EMPTY
    };

    // 仓外耗材状态
    enum SUPPLSTATUS {
        CURRENT,
        BACKUP,
        SCANFAILED,
        EXPIRED,
        UNKONW
    };

    struct SupplyStatus
    {
        SUPPLYTYPE supplyType;
        SUPPLSTATUS status;
        ch::tf::SuppliesInfo supplyInfo;
        ise::tf::SuppliesInfo supplyIseInfo;
        int progress;
        int blance;

        SupplyStatus()
        {
            supplyType = SUPPLYTYPE::EMPTY;
            status = SUPPLSTATUS::UNKONW;
            progress = 0;
            blance = 0;
        }
    };

    QProgressIseBarShell(QWidget *parent = Q_NULLPTR);
    ~QProgressIseBarShell() { ; };

    ///
    /// @brief 设置耗材状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void SetSupplyStatus(SupplyStatus status);

    ///
    /// @brief 设置提醒量(碱性)
    ///
    /// @param[in]  remind  提醒百分比
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月21日，新建函数
    ///
    void SetRemind(int remind);
signals:
    void focusOut();

private:
    ///
    /// @brief 处理焦点失去的事件
    ///
    /// @param[in]  event  事件信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月23日，新建函数
    ///
    void focusOutEvent(QFocusEvent *event);

    ///
    /// @brief 样本针清洗液状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月21日，新建函数
    ///
    void ComputSampleWashAcidityStatusSupply(SupplyStatus status);


    ///
    /// @brief ise的内部标准液的显示
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月8日，新建函数
    ///
    void IseISStatusSupply(SupplyStatus status);

    ///
    /// @brief ISE的缓释液的显示
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月8日，新建函数
    ///
    void IseBufStatusSupply(SupplyStatus status);

private:
    Ui::IseProgressBar*             ui;
    SupplyStatus                    m_supplyStatus;             ///< 耗材状态
    int                             m_bootleRemind;
};

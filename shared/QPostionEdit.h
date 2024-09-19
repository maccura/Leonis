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
#pragma once

#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QList>
#include <boost/optional.hpp>

class QPushButton;
namespace Ui {
    class QPostionEdit;
};


class QPostionEdit : public BaseDlg
{
    Q_OBJECT

public:
    // 编辑类型
    enum class MODEL
    {
        // 校准
        CALI = 0,
        // 质控
        QC = 1,
        // 免疫校准
        IMCALI = 2
    };

    QPostionEdit(QWidget *parent = Q_NULLPTR);
    ~QPostionEdit();

    ///
    /// @brief 设置校准品的列表
    ///
    /// @param[in]  docs   校准品列表
    /// @param[in]  level  水平
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    bool SetCaliDoc(const QList<ch::tf::CaliDoc>& docs, int level);

    ///
    /// @brief 获取校准文档列表
    ///
    ///
    /// @return 校准文档列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    boost::optional<QList<ch::tf::CaliDoc>> GetCaliDoc();


    ///
    /// @brief 设置质控品的列表和水平
    ///
    /// @param[in]  docs   质控品列表
    /// @param[in]  level  水平
    ///
    /// @return 质控品列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    bool SetQcDoc(const QList<::tf::QcDoc>& docs, int level);

    ///
    /// @brief 获取质控品的列表
    ///
    ///
    /// @return 质控品列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    boost::optional<QList<::tf::QcDoc>> GetQcDoc();

    ///
    /// @brief 设置免疫校准品的列表
    ///
    /// @param[in]  docs   校准品列表
    /// @param[in]  level  水平
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年12月13日，新建函数
    ///
    bool SetIMCaliDoc(const QList<im::tf::CaliDoc>& docs, int level);

    ///
    /// @brief 获取免疫校准文档列表
    ///
    /// @return 校准文档列表
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年12月13日，新建函数
    ///
    boost::optional<QList<im::tf::CaliDoc>> GetIMCaliDoc();

    ///
    /// @brief 初始化视图的控件
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年1月6日，新建函数
    ///
    void InitViewControls();

signals:
    ///
    /// @brief
    ///     成功修改位置类型信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void ModifyCaliPostion();

    ///
    /// @bref
    ///		质控编译框的消息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月6日，新建函数
    ///
    void ModifyQcPostion();

    ///
    /// @brief
    ///     成功修改免疫校准品位置类型信息
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年12月13日，新建函数
    ///
    void ModifyIMCaliPostion();

private:
    ///
    /// @brief 根据模式启动对应的对话框
    ///
    /// @param[in]  module  模式（校准或者质控）
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    bool StartInit(MODEL module);

    ///
    /// @brief 
    ///
    ///     初始化信号连接
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年8月30日，新建函数
    ///
    void InitSignalConnect();

    ///
    /// @brief  位置是否被占用
    ///
    /// @param[in]  strRack  架号
    /// @param[in]  iPos   位置号
    /// @param[in]  fl     是否是第一水平（直接报警）
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年12月13日，新建函数
    ///
    bool IsPositonBeUsing(const std::string& strRack, int iPos, bool fl = true);

    bool CheckPositionInvalid(tf::TubeType::type tubetype, const QString& rackNo, int pos);

    // 更新结果数据
    bool UpdateCaliCh(tf::TubeType::type tubetype, const QString& rackNo, int pos);
    bool UpdateCaliIm(tf::TubeType::type tubetype, const QString& rackNo, int pos);
    bool UpdateQc(tf::TubeType::type tubetype, const QString& rackNo, int pos);

    // 初始化界面
    void InitUiData(tf::TubeType::type tubetype, const std::string& rackNo, int pos);

    // 更新位置按钮可用状态
    void ResetPosAvable(const std::string& rack);

    private slots:
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
    void OnSavePostion();

    ///
    /// @brief 
    ///
    ///     点击清空按钮
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年8月30日，新建函数
    ///
    void OnClearPosition();

    ///
    /// @brief  架号变化
    ///
    /// @param[in]  rack  新架号
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年9月7日，新建函数
    ///
    void OnRackChanged(QString rack);

private:
    Ui::QPostionEdit				*ui;
    std::vector<QPushButton*>       m_pushButtons;              ///< 位置按钮

    QList<ch::tf::CaliDoc>          m_CaliDocs;                 ///< 校准品组
    QList<im::tf::CaliDoc>          m_IMCaliDocs;               ///< 校准品组
    QList<::tf::QcDoc>			    m_QcDocs;                   ///< 质控品组
    int                             m_Level;                    ///< 水平
    MODEL							m_module;					///< 校准模式，质控模式
    std::map<std::string, std::string>        m_mapPoses;                 ///< 当前设置的校准组或质控的位置
};

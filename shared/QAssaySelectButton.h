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
/// @file     QAssaySelectButton.h
/// @brief    项目选择按钮控件
///
/// @author   5774/WuHongTao
/// @date     2022年5月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/QAssayTabWidgets.h"
#include <QWidget>

class QMouseEvent;
namespace Ui {
    class QAssaySelectButton;
};


// 样本量
enum SampleSize
{
    STD_SIZE = 0,      // 标准
    DEC_SIZE,          // 减量
    INC_SIZE,          // 增量
    MANUAL_SIZE,       // 手工稀释
    USER_SIZE		   // 自动稀释
};

// 按钮类型
enum ButtonType
{
    NORAML_BUTTON = 0, // 普通按钮【普通项目的显示】
    SIMPLE_BUTTON,	   // 简单按钮 [组合项目和项目选择]
    EDIT_BUTTON		   // 编辑按钮 【应用界面点击按钮有弹框】
};

// 项目选择按钮属性
struct AssayButtonData
{
    bool              bIsProfile;             // 是否是组合项目
    bool              bIsRegentAbn;           // 是否试剂异常
    bool              bIsCaliAbn;             // 是否校准异常
    bool              bIsMask;                // 是否项目遮蔽
    bool              bIsAnalyzed;            // 是否已经分析完成
    bool              bIsRecheck;             // 是否需要复查
    bool              bIsRequest;             // 待测
    bool              bIsForbidden;           // 是否禁止
    bool			  bIsUse;				  // 是否使用中					
    int               iDilution;              // 稀释倍数
    int				  assayCode;			  // 项目的编号
    int				  testTimes;			  // 测试次数
    SampleSize        enSampleSize;           // 样本量
    SampleSize        preSampleSize;          // 样本量(上一次)
    int               preiDilution;           // 稀释倍数(上一次)
    ButtonType		  buttonType;			  // 控件类型
    AssayType		  assayType;              // 项目对话框类型
    QString           strAssayName;           // 项目名
    std::vector<int64_t> guidList;			  // 测试项目GUID表
    std::vector<int>  strSubAssayList;        // 子项目列表

    AssayButtonData();

    bool operator==(const AssayButtonData& rightData) const
    {
        return (bIsProfile == rightData.bIsProfile &&
            bIsRegentAbn == rightData.bIsRegentAbn &&
            bIsCaliAbn == rightData.bIsCaliAbn &&
            bIsMask == rightData.bIsMask &&
            bIsAnalyzed == rightData.bIsAnalyzed &&
            bIsRecheck == rightData.bIsRecheck &&
            bIsRequest == rightData.bIsRequest &&
            bIsUse == rightData.bIsUse &&
            buttonType == rightData.buttonType &&
            iDilution == rightData.iDilution &&
            assayCode == rightData.assayCode &&
            enSampleSize == rightData.enSampleSize &&
            buttonType == rightData.buttonType &&
            strAssayName == rightData.strAssayName &&
            guidList == rightData.guidList &&
            strSubAssayList == rightData.strSubAssayList);
    }

    bool operator!=(const AssayButtonData& rightData) const
    {
        return !(*this == rightData);
    }
};

using AssayButtons = std::map<int, AssayButtonData>;
using AssayButtonDatas = std::vector<AssayButtons>;


class QAssaySelectButton : public QWidget
{
    Q_OBJECT

public:
    QAssaySelectButton(QWidget *parent = Q_NULLPTR, int indexPos = -1);
    ~QAssaySelectButton();

    ///
    /// @brief 设置按钮属性
    ///
    /// @param[in]  attribute  属性参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void SetButtonAttribute(const AssayButtonData& attribute = AssayButtonData());

    ///
    /// @brief 更新按钮属性（为了减少get set的开销）
    ///
    /// @param[in]  isUse  是否使用中
    /// @param[in]  isProfile  是否组合项目
    ///
    /// @par History:
    /// @li 8276/huchunli，2022年10月8日，新建函数
    ///
    void SetButtonAttribute(bool isUse, bool isProfile);

    ///
    /// @brief 获取当前按钮属性
    ///
    ///
    /// @return 按钮属性
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    inline AssayButtonData GetButtonAttribute() { return m_attribute; };

    ///
    /// @bref
    ///		获取按钮的项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    inline int GetButtonAssayCode() { return m_attribute.assayCode; };

    ///
    /// @brief 设置项目的属性
    ///
    /// @param[in]  indexPos  项目位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    inline void SetPrivatePosIndex(int indexPos) { m_indexPos = indexPos; }

    ///
    /// @brief 获取位置信息
    ///
    ///
    /// @return 位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    inline int GetPrivatePostion() { return m_indexPos; };

    ///
    /// @brief 清除卡片当前信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void ClearStatus();

    ///
    /// @bref
    ///		置位非选中
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void ClearSelected();
    inline bool IsSelected() { return m_attribute.bIsRequest; };

    ///
    /// @bref
    ///		清除卡片内容
    ///
    /// @return 清除之前的assayCode
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月10日，新建函数
    ///
    int  ClearContent();
    void ResetText(const QString& strText);

signals:
    ///
    /// @brief 信号
    ///
    /// @param[in]	 项目位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void select(int);
	void select(int,int);
protected:
    ///
    /// @brief 更新按钮状态
    ///
    /// @param[in]  attribute  按钮状态参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void UpdateButtonStatus(const AssayButtonData& attribute);

    ///
    /// @brief 清除项目按钮的状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void ClearButtonStatus();

    ///
    /// @brief 重写鼠标按下事件
    ///
    /// @param[in]  event  鼠标事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::QAssaySelectButton*			ui;							///< 界面ui
    AssayButtonData					m_attribute;				///< 按钮属性
    int								m_indexPos;					///< 位置信息
};

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
/// @file     dbgparamfittingwidget.h
/// @brief    应用界面->工程师调试->参数（免疫）拟合
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include <functional>
#include <QWidget>
#include <QDialog>
#include <QListWidget>
#include "../../src/alg/im/IMM_ALG/imm_alg.h"

class QStandardItemModel;
class QButtonGroup;

namespace Ui
{
    class DbgParamFittingWidget;
};

namespace tf
{
    class GeneralAssayInfo;
};

namespace utilcomm {
    class CaliCurvParam;
};

///
/// @bref
///		列表弹出选择框
///
class SelectListDlg : public QDialog
{
    Q_OBJECT
public:
    SelectListDlg(QWidget* prant, const QStringList& contents);
    inline QString GetSelected() { return m_selectString; }

    protected Q_SLOTS:

    void ItemDoubleClick(QListWidgetItem *item);

private:
    QString m_selectString;
};


class DbgParamFittingWidget : public QWidget
{
    Q_OBJECT

public:
    DbgParamFittingWidget(QWidget *parent = Q_NULLPTR);
    ~DbgParamFittingWidget();

    ///
    /// @bref
    ///		加载校准拟合信息到界面
    ///
    /// @param[in] iAssayCode 项目ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void LoadAnalysisParam(const utilcomm::CaliCurvParam& curvParm, const std::string& curLot);

    ///
    /// @bref
    ///		设置当前项目对应的曲线的批号列表
    ///
    /// @param[in] vecLots 批号列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月8日，新建函数
    ///
    void SetCaliLotsList(const std::vector<std::string>& vecLots);

    ///
    /// @bref
    ///		获取界面的拟合信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    bool TakeCaliFittingInfo(utilcomm::CaliCurvParam& curvParam);

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

	protected Q_SLOTS:

    virtual bool eventFilter(QObject* target, QEvent* event) override;

private:

    ///
    /// @bref
    ///		对主标曲点列表赋值
    ///
    /// @param[in] lstPt 主标曲信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月14日，新建函数
    ///
    void AssignMasterCurvPointTb(const std::shared_ptr<im::CalibratePoints>& lstPt);

    ///
    /// @bref
    ///		对主标曲的分段信息列表赋值
    ///
    /// @param[in] lstPt 主标曲信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月14日，新建函数
    ///
    void AssignSegmentTb(const std::shared_ptr<im::Fit2Segment>& lstPt);

    ///
    /// @bref
    ///		清空界面的值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    void CleanUi();


    std::function<void(const QString& strVer)> m_funSetCaliLot;

public:
    ///
    /// @bref
    ///		设置校准批号改变后需要触发的回调
    ///
    /// @param[in] funSetCaliLot 回调函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月8日，新建函数
    ///
    void BindSetCaliLotCallBack(std::function<void(const QString& strVer)> funSetCaliLot)
    {
        m_funSetCaliLot = funSetCaliLot;
    }

private:
    Ui::DbgParamFittingWidget			*ui;                            // UI指针
    bool							    m_bInit;                        // 是否已经初始化
    QButtonGroup* m_btnGroup;

    std::map<im::FitType, QString> m_mapFitType; // <拟合类型, 类型字符串>
    QStandardItemModel* m_mainCurvPtModel;  // 主标曲点列表
    QStandardItemModel* m_sengModel; // 分段信息列表

    std::vector<std::string> m_vecLots; /// 同项目的曲线批号列表
};

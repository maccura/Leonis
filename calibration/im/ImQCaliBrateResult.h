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
/// @file     ImQCaliBrateResult.h
/// @brief    校准结果
///
/// @author   8090/YeHuaNing
/// @date     2022年11月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "boost/optional.hpp"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "curveDraw/common.h"
#include "PrintExportDefine.h"

class QStandardItemModel;
namespace Ui {
    class ImQCaliBrateResult;
};

class ImQCaliBrateResult : public BaseDlg
{
    Q_OBJECT

public:
    ImQCaliBrateResult(QWidget *parent = Q_NULLPTR);
    ~ImQCaliBrateResult();

    ///
    /// @brief 启动校准详情对话框
    ///
    /// @param[in]  reagent  实际详细信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void StartDialog(im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 获取单例
    ///
    ///
    /// @return 单例句柄
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    static ImQCaliBrateResult& GetInstance();

private:
    ///
    /// @brief 初始化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void Init();

    ///
    /// @brief 获取试剂的当前校准曲线
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @return 校准曲线
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void GetCurrentCurve(im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 获取试剂的历史校准曲线
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void GetHistoryCurves(im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 设置历史曲线
    ///
    /// @param[in]  caliCurves  历史曲线组
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void SetHistoryCurve(std::vector<std::shared_ptr<im::tf::CaliCurve>>& caliCurves);

    ///
    /// @brief 根据参数获取显示曲线
    ///
    /// @param[in]  curveData  曲线参数
    /// @param[in]  curveAlg   算法曲线相关参数
    /// @param[in]  curves     曲线可绘制参数
    ///
    /// @return		成功则true
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    bool ShowCurve(std::shared_ptr<im::tf::CaliCurve>& curveData, std::vector<curveSingleDataStore>& curves);

    ///
    /// @brief 设置校准状态
    ///
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void InitCaliStatus();

    ///
    /// @brief 根据试剂和校准曲线信息获取校准状态
    ///
    /// @param[in]  reagent  试剂信息
    /// @param[in]  curve    曲线状态
    ///
    /// @return 校准状态信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    boost::optional<QStringList> GetCaliStatusShow(im::tf::ReagentInfoTable& reagent, std::shared_ptr<im::tf::CaliCurve>& curve);

    ///
    /// @brief 显示当前曲线
    ///
    /// @param[in]  reagent		试剂信息
    /// @param[in]  curveStore  曲线的绘制信息
    ///
    /// @return 曲线的数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    std::shared_ptr<im::tf::CaliCurve> SetCurrentCurve(im::tf::ReagentInfoTable& reagent, std::vector<curveSingleDataStore>& curveStore);

    ///
    /// @brief  重新获取曲线进行显示
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年10月31日，新建函数
    ///
    void initShow(void);

    ///
    /// @brief 获取打印导出数据
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月19日，新建函数
    ///
    bool GetPrintExportInfo(CaliResultInfo& Info, bool bIsPrint);

    private slots:
    ///
    /// @brief 历史曲线的选择槽函数
    ///
    /// @param[in]    历史曲线index
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void OnHistoryCurve(int);

    ///
    /// @brief 更新工作曲线
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void OnUpdateWorkCurve();


    ///
    /// @brief 响应导出按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月30日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月30日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief  响应校准完成
    ///
    /// @param[in]  cv  曲线信息
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年10月31日，新建函数
    ///
    void OnCaliEnd(const im::tf::CaliCurve cv);

private:
    Ui::ImQCaliBrateResult*			ui;
    QStandardItemModel*             m_calibrate;				///< 校准品浓度mode
    std::vector<std::shared_ptr<im::tf::CaliCurve>> m_historyCurve;	///< 历史曲线列表
    std::shared_ptr<im::tf::CaliCurve> m_curCurve;                  ///< 当前校准曲线
    std::shared_ptr<im::tf::CaliCurve> m_masterCurve;                  ///< 主标曲
    std::vector<curveSingleDataStore> m_currentCurves;
    std::vector<curveSingleDataStore> m_masterCurves;
    std::vector<curveSingleDataStore> m_historyCurvesStore;
    std::map<int, QLabel*>			m_labelMap;					///< 显示校准详细信息的映射关系
    im::tf::ReagentInfoTable	    m_reagent;					///< 当前试剂
    int								m_currentIndex;				///< 历史曲线的当前选中号
    bool                            m_bDispMaster;              ///< 是否显示主曲线
};

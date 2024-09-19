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
/// @file     CaliBrateDetailShow.h
/// @brief    校准曲线参数显示页面
///
/// @author   5774/WuHongTao
/// @date     2021年11月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CaliBrateDetailShow.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父指针
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
CaliBrateDetailShow::CaliBrateDetailShow(QWidget *parent)
    : BaseDlg(parent)
    , m_CurveShow(nullptr)
    , m_curveObject(nullptr)
    , m_parent(qobject_cast<CaliBrationStatusPage*>(parent))
{
    ui.setupUi(this);
    connect(ui.ok, SIGNAL(clicked()), this, SLOT(OnCurveShow()));
    connect(ui.close, SIGNAL(clicked()), this, SLOT(OnClose()));
    connect(ui.Curve, SIGNAL(clicked()), this, SLOT(OnCurveShow()));
}

///
/// @brief
///     析构函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
CaliBrateDetailShow::~CaliBrateDetailShow()
{
}

///
/// @brief
///     显示试剂组对应的校准曲线的详细信息
///
/// @param[in]  db  试剂组的数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
void CaliBrateDetailShow::StartShowReagentCurve(int db)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ::ch::tf::ReagentGroupQueryResp qryResp;
    ::ch::tf::ReagentGroupQueryCond qryCond;

    // 设置查询条件，数据库主键
    qryCond.__set_id(db);

    // 查询成功
    if (!ch::c1005::LogicControlProxy::QueryReagentGroup(qryResp, qryCond))
    {
        ULOG(LOG_ERROR, "Can not find the reagent group id is %d", db);
        return;
    }

    // 没有查找到试剂数据，直接返回
    if (qryResp.lstReagentGroup.size() < 1)
    {
        return;
    }

    // 显示表头信息
    ShowCaliBrateStatus(qryResp.lstReagentGroup[0]);
    // 项目参数
    ShowCaliBrateParameter(CURVETYPE::PROJECTCALI, qryResp.lstReagentGroup[0]);
    // 批参数
    ShowCaliBrateParameter(CURVETYPE::LOTCALI, qryResp.lstReagentGroup[0]);
    // 瓶参数
    ShowCaliBrateParameter(CURVETYPE::CUURENTCALI, qryResp.lstReagentGroup[0]);
}

///
/// @brief
///     显示试剂状态信息（名称，位置等等）
///
/// @param[in]  group  试剂组信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
void CaliBrateDetailShow::ShowCaliBrateStatus(::ch::tf::ReagentGroup& group)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    // 防止错误，空指针
    if (m_parent == nullptr)
    {
        return;
    }

    // 显示试剂名称
    ui.name->setText(m_parent->GetAssayNameFromCode(group.assayCode));
    // 显示设备的名称
    ui.module->setText(m_parent->GetDeviceNameFromSn(group.deviceSN));
}

///
/// @brief
///     获取显示参数的控件列表
///
/// @param[in]  type  参数类型
///
/// @return 控件列表的组
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
std::vector<QLineEdit*> CaliBrateDetailShow::GetParameterContainer(CURVETYPE type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::vector<QLineEdit*> ShowParameters;

    // 瓶组曲线的参数
    if (type == CURVETYPE::CUURENTCALI)
    {
        ShowParameters.push_back(ui.s1abs);
        ShowParameters.push_back(ui.A);
        ShowParameters.push_back(ui.B);
        ShowParameters.push_back(ui.C);
        ShowParameters.push_back(ui.H);
        ShowParameters.push_back(ui.I);
        ShowParameters.push_back(ui.L);
        ShowParameters.push_back(ui.K);
    }
    // 批曲线的参数
    else if(type == CURVETYPE::LOTCALI)
    {
        ShowParameters.push_back(ui.S1abs_2);
        ShowParameters.push_back(ui.A_2);
        ShowParameters.push_back(ui.B_2);
        ShowParameters.push_back(ui.C_2);
        ShowParameters.push_back(ui.H_2);
        ShowParameters.push_back(ui.I_2);
        ShowParameters.push_back(ui.L_2);
        ShowParameters.push_back(ui.K_2);
    }
    // 项目曲线
    else if(type == CURVETYPE::PROJECTCALI)
    {
        ShowParameters.push_back(ui.S1ABS_3);
        ShowParameters.push_back(ui.A_3);
        ShowParameters.push_back(ui.B_3);
        ShowParameters.push_back(ui.C_3);
        ShowParameters.push_back(ui.H_3);
        ShowParameters.push_back(ui.I_3);
        ShowParameters.push_back(ui.L_3);
        ShowParameters.push_back(ui.K_3);
    }

    // 清空控件
    for (auto edit : ShowParameters)
    {
        edit->clear();
    }

    return ShowParameters;
}

///
/// @brief
///     显示各种类型的曲线
///
/// @param[in]  type  曲线类型
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
void CaliBrateDetailShow::ShowCaliBrateParameter(CURVETYPE type, ch::tf::ReagentGroup& group)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 1：查询瓶曲线的参数
    ::ch::tf::CaliCurveQueryResp _return;
    ::ch::tf::CaliCurveQueryCond ccqc;

    // 找到曲线信息的容器
    std::vector<QLineEdit*> ShowParameters = GetParameterContainer(type);

    switch (type)
    {
    // 当前校准曲线参数
    case CURVETYPE::CUURENTCALI:
    {
        ccqc.__set_id(group.caliCurveId);
    }
    break;
    // 批校准曲线参数
    case CURVETYPE::LOTCALI:
    {
        // 设备sn
        ccqc.__set_deviceSN(group.deviceSN);
        // 项目编号
        ccqc.__set_assayCode(group.assayCode);

        // 从试剂关键信息中获取批号（首先进行越界处理，避免）
        if (group.reagentKeyInfos.size() <= 0)
        {
            return;
        }

        // 试剂批号
        ccqc.__set_reagentLot(group.reagentKeyInfos[0].lot);
        // 是否批曲线
        ccqc.__set_lotCurve(true);
    }
    break;
    // 项目校准曲线参数
    case CURVETYPE::PROJECTCALI:
    {
        // 设备sn
        ccqc.__set_deviceSN(group.deviceSN);
        // 项目编号
        ccqc.__set_assayCode(group.assayCode);
        // 是否项目曲线
        ccqc.__set_assayCurve(true);
    }
    break;
    default:
        break;
    }

    // 执行查询
    ch::c1005::LogicControlProxy::QueryCaliCurve(_return, ccqc);
    if ((_return.result == ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS) && (_return.lstCaliCurves.size() > 0))
    {
        // 获取曲线信息
        std::shared_ptr<ch::ICalibrateCurve<>> curVeInfo;
        try
        {
            curVeInfo = ch::CreateCaliCurve(_return.lstCaliCurves[0].curveData);
        }
        catch (exception* e)
        {
            ULOG(LOG_ERROR, "exception :  %s", e->what());
            return;
        }
        catch (...)
        {
            ULOG(LOG_ERROR, "exception :  Can not creat curve");
            return;
        }

        if (curVeInfo != nullptr)
        {
            // 将当前曲线数据保存，以备显示使用
            m_curveObject = curVeInfo;
            // 循环填充曲线参数
            int index = 0;
            for (auto parameter : curVeInfo->GetModelParameters())
            {
                // 防止越界
                if (index < ShowParameters.size())
                {
                    ShowParameters[index++]->setText(QString::number(parameter.second));
                }
            }
        }
    }
}

///
/// @brief
///     点击按钮curve的反应
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
void CaliBrateDetailShow::OnCurveShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 根据判断生成对话框
    if (m_CurveShow == nullptr)
    {
        m_CurveShow = std::shared_ptr<CaliBrateCurveShow>(new CaliBrateCurveShow(this));
    }

    // 数据显示
    m_CurveShow->StartShowCurrentCurve(m_curveObject);
    m_CurveShow->show();
}

///
/// @brief
///     关闭对话框
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建函数
///
void CaliBrateDetailShow::OnClose()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_curveObject = nullptr;
    close();
}

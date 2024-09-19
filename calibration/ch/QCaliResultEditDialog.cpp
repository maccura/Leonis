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
/// @file     QCaliResultEditDialog.h
/// @brief    编辑校准结果
///
/// @author   5774/WuHongTao
/// @date     2022年3月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliResultEditDialog.h"
#include "ui_QCaliResultEditDialog.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "CaliBrateCommom.h"
#include "src/common/Mlog/mlog.h"

QCaliResultEditDialog::QCaliResultEditDialog(QWidget *parent)
    : BaseDlg(parent),
    m_currentIndex(-1)
{
    ui = new Ui::QCaliResultEditDialog();
    ui->setupUi(this);
    Init();
}

QCaliResultEditDialog::~QCaliResultEditDialog()
{
}

///
/// @brief
///    设置曲线列表
///
/// @param[in]  curves  曲线列表（第一条表示选中曲线，第二条表示历史曲线） 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::SetCurveList(std::vector<ch::tf::CaliCurve>& curves)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_curveList.clear();
    m_curveList = curves;
    if (curves.empty())
    {
        ULOG(LOG_ERROR, "%s(), the curve is empty", __FUNCTION__);
        return;
    }

    m_caliCurve = nullptr;
    // 默认设置选中曲线
    emit ui->Select_Button->clicked();
}

///
/// @brief
///     初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::Init()
{
    m_curveList.clear();
    // 设置为两点线性法
    ui->Methord_Edit->setText(tr("two_pointer"));
    // 固定不允许修改
    ui->Methord_Edit->setDisabled(true);
    // 公式
    ui->Function_Edit->setText(tr("A=K*C+B"));
    ui->Function_Edit->setDisabled(true);
    ui->Project_Edit->setDisabled(true);
    // 选中曲线
    connect(ui->Select_Button, &QPushButton::clicked, this, [=](){OnShowCurve(0);});
    // 历史曲线
    connect(ui->His_Button, &QPushButton::clicked, this, [=](){OnShowCurve(1);});
    // 曲线参数复位
    connect(ui->Reset_Button, SIGNAL(clicked()), this, SLOT(clearContent()));
    // 对话框关闭
    connect(ui->Close_Button, SIGNAL(clicked()), this, SLOT(close()));
    // 参数保存
    connect(ui->Save_Button, SIGNAL(clicked()), this, SLOT(OnSaveCurveParameter()));
}

///
/// @brief
///     清除对话框内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::clearContent()
{
    if (m_currentIndex >= m_curveList.size())
    {
        return;
    }

    ch::tf::CaliCurve curve = m_curveList[m_currentIndex];
    // 清空--保证使用原始值
//    curve.__set_curveDataEdit("");
    ShowCurveStatus(curve);
}

///
/// @brief
///     保存曲线参数
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::OnSaveCurveParameter()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 如果K值为空
    if (ui->K_Edit->text().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("please make the k has the right paramter!"), TipDlgType::SINGLE_BUTTON));
        if (QDialog::Rejected == pTipDlg->exec())
        {
            return;
        }
    }

    // 检查参数
    if (ui->R0_Edit->text().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("please make the R0 has the right paramter!"), TipDlgType::SINGLE_BUTTON));
        if (QDialog::Rejected == pTipDlg->exec())
        {
            return;
        }
    }

    if (m_caliCurve == nullptr)
    {
        return;
    }

    std::map<string, double> parameter = m_caliCurve->GetModelParameters();
    // 回写K值
    parameter["k"] = ui->K_Edit->text().toInt();
    // 回写b值
    parameter["b"] = ui->R0_Edit->text().toInt();
    // 曲线的参数字符串
    std::string curveData = m_caliCurve->SetModelParametersTemporary(parameter);


    if (m_currentIndex >= m_curveList.size())
    {
        return;
    }

    std::vector<ch::tf::CaliCurve> curveList;
    ch::tf::CaliCurve curve;
//    curve.__set_curveDataEdit(curveData);
    curve.__set_id(m_curveList[m_currentIndex].id);
    curveList.push_back(curve);

    if (!ch::c1005::LogicControlProxy::ModifyCaliCurves(curveList))
    {
        ULOG(LOG_INFO, "QueryCaliCurve failed");
    }

    close();
}

///
/// @brief
///     显示曲线状态
///
/// @param[in]  curve  曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::ShowCurveStatus(ch::tf::CaliCurve& curve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(curve.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", curve.assayCode);
        return;
    }

    // 设置名字
    ui->Project_Edit->setText(QString::fromStdString(spAssayInfo->assayName));
    m_caliCurve = nullptr;
    m_caliCurve = CaliBrateCommom::GetCurve(curve);

    if (m_caliCurve == nullptr)
    {
        return;
    }

    // 依次筛选出参数
    double k = 0;
    double b = 0;
    for (const auto& parameter : m_caliCurve->GetModelParameters())
    {
        // 参数
        if ("k" == parameter.first)
        {
            k = parameter.second;
        }

        // 截距
        if ("b" == parameter.first)
        {
            b = parameter.second;
        }
    }

    // K值
    ui->K_Edit->setText(QString::number(k, 'f', 2));
    // R0值
    ui->R0_Edit->setText(QString::number(b, 'f', 2));
}

///
/// @brief
///     点击响应
///
/// @param[in]  index  曲线序号
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建函数
///
void QCaliResultEditDialog::OnShowCurve(int index)
{
    m_currentIndex = index;
    if (index >= m_curveList.size())
    {
        return;
    }
    else
    { 
        // 显示对应的曲线
        ShowCurveStatus(m_curveList[index]);
    }
}

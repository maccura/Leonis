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

#include "QDeviceSupplyStatus.h"
#include "ui_QDeviceSupplyStatus.h"

QDeviceSupplyStatus::QDeviceSupplyStatus(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QDeviceSupplyStatus();
    ui->setupUi(this);
    Init();
}

QDeviceSupplyStatus::~QDeviceSupplyStatus()
{
}

///
/// @brief
///     设置设备类型
///
/// @param[in]  type  设备类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyStatus::SetDeviceType(DEVICETYPE& type)
{
    // 设置显示页面--根据类型
    ui->stackedWidget->setCurrentIndex(type);
}

///
/// @brief
///     设置设备名称
///
/// @param[in]  name  设备名称
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QDeviceSupplyStatus::SetDeviceName(QString name)
{
    ui->label->setText(name);
}

///
/// @brief
///     设置设备耗材数据，显示
///
/// @param[in]  supplyOverView  耗材数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyStatus::SetSupplyStatus(DEVICETYPE type, const std::vector<ch::tf::SuppliesOverview>& supplyOverView)
{
    // 设备类型决定了每次返回的耗材数据，若不匹配，则返回
    if (!((supplyOverView.size() == 9 && type == DEVICETYPE::CHE)
        || (supplyOverView.size() == 2 && type == DEVICETYPE::ISE)))
    {
        return;
    }

    // 设置显示页面--根据类型
    ui->stackedWidget->setCurrentIndex(type);

    switch (type)
    {
    case QDeviceSupplyStatus::CHE:

        // 依次变换数据
        for (const auto& overView : supplyOverView)
        {
            // 仓内耗材
            if (overView.posInfo.area == ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1)
            {
                SetUnderDeckSupplyStatus(overView);
            }
            // 仓外耗材
            else if (overView.posInfo.area == ch::tf::SuppliesArea::SUPPLIES_AREA_CABINET)
            {
                SetOutBoardSupplyStatus(overView);
            }
        }
        break;
    case QDeviceSupplyStatus::ISE:
        break;
    default:
        break;
    }

    ui->UnderDeck_Widget->SetSupplyStatus(m_UnderDeckSupplyList);
    ui->OutBoard_Widget->SetSupplyStatus(m_OutBoardSupplyList);
    ui->ISE->SetSupplyStatus(m_IseSupplyList);
}

///
/// @brief
///     初始化仓内仓外以及ISE初始内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyStatus::Init()
{
    QSpecialProgessBar::ProgressStatus status;
    status.progress = 0;
    status.title = "test";
    status.progressData = 0;
    // 仓内为4个
    InitVector(m_UnderDeckSupplyList, status,4);
    ui->UnderDeck_Widget->SetContainerType(QSupplyContainer::SUPPLYTYPE::UNDERDECK);
    ui->UnderDeck_Widget->show();
    // 仓外5个
    InitVector(m_OutBoardSupplyList, status, 5);
    ui->OutBoard_Widget->SetContainerType(QSupplyContainer::SUPPLYTYPE::OUTBOARD);
    ui->OutBoard_Widget->show();
    // ISE3个
    InitVector(m_IseSupplyList, status, 3);
    ui->ISE->SetContainerType(QSupplyContainer::SUPPLYTYPE::ISEUNDERDECK);
    ui->ISE->show();
}

///
/// @brief
///     根据耗材总览信息获取显示信息
///
/// @param[in]  overview  耗材总览信息
///
/// @return 显示信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
QSpecialProgessBar::ProgressStatus QDeviceSupplyStatus::GetStatusFromSupplyInfo(const ch::tf::SuppliesOverview& overView)
{
    QSpecialProgessBar::ProgressStatus status;
    // 余量
    status.progressData = overView.residual;
    // 状态
    overView.alarm ? (status.status = QSpecialProgessBar::USESTATE::WARING) : (status.status = QSpecialProgessBar::USESTATE::NORMAL);
    // 为0的时候错误状态
    if (status.progressData == 0)
    {
        status.status = QSpecialProgessBar::USESTATE::ERRORSTATE;
    }

    // 直接设置为0
    if (overView.capacity == 0)
    {
        status.progress = 0;
    }
    else
    {
        // 百分比
        status.progress = double(double(overView.residual) / double(overView.capacity)) * 100;
    }

    return status;
}

///
/// @brief
///     设置仓内清洗液
///
/// @param[in]  overview  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyStatus::SetUnderDeckSupplyStatus(const ch::tf::SuppliesOverview& overView)
{
    QSpecialProgessBar::ProgressStatus status;
    status = GetStatusFromSupplyInfo(overView);

    switch (overView.suppliesType)
    {
        // 样本针稀释液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_DILUENT_MIN:
        m_UnderDeckSupplyList[0] = status;
        // 标题
        m_UnderDeckSupplyList[0].title = tr("sample_wash").toStdString();
        break;
        // 抗菌无磷液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_HITERGENT:
        m_UnderDeckSupplyList[1] = status;
        // 标题
        m_UnderDeckSupplyList[1].title = tr("HITERGENT").toStdString();
        break;
        // 酸性清洗液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY:
        m_UnderDeckSupplyList[2] = status;
        // 标题
        m_UnderDeckSupplyList[2].title = tr("ACIDITY").toStdString();
        break;
        // 碱性清洗液
    case ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY:
        m_UnderDeckSupplyList[3] = status;
        // 标题
        m_UnderDeckSupplyList[3].title = tr("ALKALINITY").toStdString();
        break;
    default:
        break;
    }
}

///
/// @brief
///     设置仓外清洗液
///
/// @param[in]  overview  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyStatus::SetOutBoardSupplyStatus(const ch::tf::SuppliesOverview& overview)
{
    QSpecialProgessBar::ProgressStatus status;
    status = GetStatusFromSupplyInfo(overview);

    // 样本针清洗液
    if (overview.posInfo.pos == 5)
    {
        m_OutBoardSupplyList[4] = status;
        // 标题
        m_OutBoardSupplyList[4].title = tr("SAMPLE_WASH").toStdString();
        return;
    }

    switch (overview.suppliesType)
    {
        // 酸
    case ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY:

        // 酸1
        if (overview.posInfo.pos == 1)
        {
            m_OutBoardSupplyList[0] = status;
            // 标题
            m_OutBoardSupplyList[0].title = tr("ACIDITY1").toStdString();
        }
        // 酸2
        else if(overview.posInfo.pos == 2)
        {
            m_OutBoardSupplyList[1] = status;
            // 标题
            m_OutBoardSupplyList[1].title = tr("ACIDITY2").toStdString();
        }
        break;
        // 碱
    case ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY:

        // 碱1
        if (overview.posInfo.pos == 1)
        {
            m_OutBoardSupplyList[2] = status;
            // 标题
            m_OutBoardSupplyList[2].title = tr("ALKALINITY1").toStdString();
        }
        // 碱2
        else if (overview.posInfo.pos == 2)
        {
            m_OutBoardSupplyList[3] = status;
            // 标题
            m_OutBoardSupplyList[3].title = tr("ALKALINITY2").toStdString();
        }
        break;

    default:
        break;
    }
}

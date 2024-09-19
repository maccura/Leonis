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

#include "QAssayShowPostion.h"
#include "ui_QAssayShowPostion.h"
#include <QStandardItemModel>

#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#define CH_TQBWIDGET_INDEX         (0)             // 生化页面索引
#define IM_TQBWIDGET_INDEX         (1)             // 免疫页面索引

enum TableViewColunmIndex                          // 表格列的索引
{
    INDEX_UNM,                                     // 序号
    INDEX_NAME                                     // 项目名称
};

QAssayShowPostion::QAssayShowPostion(QWidget *parent)
    : BaseDlg(parent)
    , m_model(Q_NULLPTR)
{
    ui = new Ui::QAssayShowPostion();
    ui->setupUi(this);
    SetTitleName(tr("加样顺序"));
    Init();
}

QAssayShowPostion::~QAssayShowPostion()
{
}

void QAssayShowPostion::showEvent(QShowEvent *event)
{
    BaseDlg::showEvent(event);

    OnReFresh();
}

void QAssayShowPostion::OnReFresh()
{
    bool isCurrCh = (ui->tabWidget->currentWidget()->objectName() == "ch");

    OrderAssay showOrder;
    if (!DictionaryQueryManager::GetItemTestOrder(showOrder, isCurrCh))
    {
        ULOG(LOG_ERROR, "Failed to get itemtestorder config.");
        return;
    }

    // 设置勾选框
    if (isCurrCh)
    {
        ui->ch_add_sample_ckbox->setChecked(showOrder.enableCustom);
    }
    else
    {
        ui->im_add_sample_ckbox->setChecked(showOrder.enableCustom);
        ui->continuous_sample_ckbox->setChecked(showOrder.enableContinuousSample);
    }

    ui->sub_widget->setEnabled(showOrder.enableCustom);
    m_model->removeRows(0, m_model->rowCount());

    // 处理血清指数项目
    auto& tempVec = showOrder.orderVec;
    std::replace(tempVec.begin(), tempVec.end(), ch::tf::g_ch_constants.ASSAY_CODE_L, ch::tf::g_ch_constants.ASSAY_CODE_SIND);

    // 删除H、I
    auto iter = std::find(tempVec.begin(), tempVec.end(), ch::tf::g_ch_constants.ASSAY_CODE_H);
    if (iter != tempVec.end())
    {
        iter = tempVec.erase(iter);
        tempVec.erase(iter);
    }

    // 设置项目
    int row = 0;
    for (const auto& item : showOrder.orderVec)
    {
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(item);
        if (!spAssay)
        {
            continue;
        }

        AddTextToTableView(m_model, row, INDEX_UNM, QString::number(row + 1));

        auto nameitem = new QStandardItem(QString::fromStdString(spAssay->assayName));
        nameitem->setData(item);
        nameitem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        m_model->setItem(row, INDEX_NAME, nameitem);
        row++;
    }

    // 列宽设置
	ui->assay_tableView->setColumnWidth(0, 100);
}

void QAssayShowPostion::OnSaveData()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        TipDlg(tr("保存失败"), tr("仪器在运行中不能修改加样顺序")).exec();
        return;
    }

    OrderAssay saveOrder;
    for (int row = 0; row < m_model->rowCount(); row++)
    {
        auto item = m_model->item(row, 1);
        if (item == Q_NULLPTR)
        {
            continue;
        }

        int code = item->data(Qt::UserRole + 1).toInt();

        // 血清指数特殊处理
        if (code == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
        {
            saveOrder.orderVec.push_back(ch::tf::g_ch_constants.ASSAY_CODE_L);
            saveOrder.orderVec.push_back(ch::tf::g_ch_constants.ASSAY_CODE_H);
            saveOrder.orderVec.push_back(ch::tf::g_ch_constants.ASSAY_CODE_I);
        }
        else
        {
            saveOrder.orderVec.push_back(code);
        }        
    }

    bool isCurrCh = (ui->tabWidget->currentWidget()->objectName() == "ch");
    if (isCurrCh)
    {
        saveOrder.enableCustom = ui->ch_add_sample_ckbox->isChecked();
    }
    else
    {
        saveOrder.enableCustom = ui->im_add_sample_ckbox->isChecked();
        saveOrder.enableContinuousSample = ui->continuous_sample_ckbox->isChecked();
    }

    if (!DictionaryQueryManager::SaveItemTestOrder(saveOrder, isCurrCh))
    {
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

	// 修改显示顺序记录操作日志 add by chenjianlin 20231215
	AddOptLog(tf::OperationType::type::MOD, tr("修改加样顺序"));
    close();
}

void QAssayShowPostion::SortItemASC()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    std::vector<int> orderVec;
    for (int row = 0; row < m_model->rowCount(); row++)
    {
        auto item = m_model->item(row, 1);
        if (item == Q_NULLPTR)
        {
            continue;
        }

        orderVec.push_back(item->data(Qt::UserRole + 1).toInt());
    }

    // 项目编号从小到大排序
    std::sort(orderVec.begin(), orderVec.end());

    // 重新显示项目
    int row = 0;
    for (const auto& code : orderVec)
    {
        auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(code);
        if (!spAssay)
        {
            continue;
        }

        // 序号
        m_model->item(row, INDEX_UNM)->setText(QString::number(row+1));

        // 名称
        auto item = m_model->item(row, INDEX_NAME);
        item->setText(QString::fromStdString(spAssay->assayName));
        item->setData(code, Qt::UserRole + 1);

        row++;
    }
}

void QAssayShowPostion::UpdateCtrlsEnabled()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto& uim = UserInfoManager::GetInstance();

    // 是否允许修改加样顺序
    bool isOperOrder = uim->IsPermisson(PSM_IM_SUCKSAMPLE_ORDER_CUSTORM);
    bool isOperOrderShow = uim->IsPermissionShow(PSM_IM_SUCKSAMPLE_ORDER_CUSTORM);
    QString curtb = ui->tabWidget->currentWidget()->objectName();
    if (curtb == "ch")
    {
        ui->sub_widget->setEnabled(ui->ch_add_sample_ckbox->isChecked() && isOperOrder);
    }
    else if (curtb == "im")
    {
        ui->im_add_sample_ckbox->setEnabled(isOperOrder);
        isOperOrderShow ? ui->im_add_sample_ckbox->show() : ui->im_add_sample_ckbox->hide();

        ui->continuous_sample_ckbox->setEnabled(uim->IsPermisson(PSM_IM_SUCKSAMPLE_ORDER_CONTINUE));
        isOperOrderShow ? ui->continuous_sample_ckbox->show() : ui->continuous_sample_ckbox->hide();
    }

    // 顺序修改按钮
    ui->bottom_btn->setEnabled(isOperOrder);
    ui->down_btn->setEnabled(isOperOrder);
    ui->reset_btn->setEnabled(isOperOrder);
    ui->top_btn->setEnabled(isOperOrder);
    ui->up_btn->setEnabled(isOperOrder);

    if (isOperOrderShow)
    {
        ui->bottom_btn->show();
        ui->down_btn->show();
        ui->reset_btn->show();
        ui->top_btn->show();
        ui->up_btn->show();
    }
    else
    {
        ui->bottom_btn->hide();
        ui->down_btn->hide();
        ui->reset_btn->hide();
        ui->top_btn->hide();
        ui->up_btn->hide();
    }
}

void QAssayShowPostion::Init()
{
    // 设置标签鼠标事件穿透
    ui->label->setAttribute(Qt::WA_TransparentForMouseEvents);

    //初始化表格
    m_model = new QStandardItemModel(ui->assay_tableView);
    m_model->setHorizontalHeaderLabels({ tr("序号"), tr("项目名称") });
    ui->assay_tableView->setModel(m_model);
    ui->assay_tableView->setColumnWidth(0, 100);
 
	ui->assay_tableView->horizontalHeader()->setStretchLastSection(true);



    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type == CHEMISTRY)
    {
        ui->tabWidget->removeTab(1);
    }
    else if (type == IMMUNE)
    {
        ui->tabWidget->removeTab(0);
    }

    // 重新设置界面布局
    if (ui->tabWidget->count() < 2)
    {
        int hight = ui->tabWidget->tabBar()->height();
        ui->sub_widget->move(ui->sub_widget->x(), ui->sub_widget->y() - hight);
        ui->label->move(ui->label->x(), ui->label->y() - hight);
        ui->save_btn->move(ui->save_btn->x(), ui->save_btn->y() - hight);
        ui->cancel_btn->move(ui->cancel_btn->x(), ui->cancel_btn->y() - hight);
        this->setFixedHeight(this->height() - hight);
    }

    // 列宽自适应
    ResizeTblColToContent(ui->assay_tableView);

    // 向上
    connect(ui->up_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assay_tableView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        if (MoveItem(currentIndex.row(), true))
        {
            ui->assay_tableView->setCurrentIndex(m_model->index(currentIndex.row() - 1, 0));
        }

        UpdateBtnState();
    });

    // 向下
    connect(ui->down_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assay_tableView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        if (MoveItem(currentIndex.row(), false))
        {
            ui->assay_tableView->setCurrentIndex(m_model->index(currentIndex.row() + 1, 0));
        }

        UpdateBtnState();
    });

    // 置顶
    connect(ui->top_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assay_tableView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        // 多次移动
        for (int row = currentIndex.row(); row > 0; row--)
        {
            if (!MoveItem(row, true))
            {
                return;
            }
        }

        ui->assay_tableView->setCurrentIndex(m_model->index(0, 0));
        UpdateBtnState();
    });

    // 置底
    connect(ui->bottom_btn, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assay_tableView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        // 多次移动
        for (int row = currentIndex.row(); row < m_model->rowCount() - 1; row++)
        {
            if (!MoveItem(row, false))
            {
                return;
            }
        }

        ui->assay_tableView->setCurrentIndex(m_model->index(m_model->rowCount() - 1, 0));
        UpdateBtnState();
    });

    // 重置
    connect(ui->reset_btn, &QPushButton::clicked, this, &QAssayShowPostion::SortItemASC);
    // 保存
    connect(ui->save_btn, &QPushButton::clicked, this, &QAssayShowPostion::OnSaveData);
    // 点击更新当前按钮状态
    connect(ui->assay_tableView, &QTableView::clicked, this, &QAssayShowPostion::UpdateBtnState);

    // 勾选框
    connect(ui->ch_add_sample_ckbox, &QCheckBox::toggled, this, [&](bool checked)
    {
        if (ui->tabWidget->currentWidget()->objectName() == "ch")
        {            
            ui->sub_widget->setEnabled(checked 
            && UserInfoManager::GetInstance()->IsPermisson(PSM_IM_SUCKSAMPLE_ORDER_CUSTORM));
        }
    });

    connect(ui->im_add_sample_ckbox, &QCheckBox::toggled, this, [&](bool checked)
    {
        if (ui->tabWidget->currentWidget()->objectName() == "im")
        {
            ui->sub_widget->setEnabled(checked);
        }
    });

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &QAssayShowPostion::OnReFresh);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

bool QAssayShowPostion::MoveItem(int row, bool direction)
{
    if (m_model == Q_NULLPTR || m_model->rowCount() <= row || row < 0)
    {
        return false;
    }

    auto currentIndex = m_model->index(row, 1);

    QModelIndex destIndex = QModelIndex();

    // 往上
    if (direction)
    {
        // 第一行不能再往上移动了
        if (row == 0)
        {
            return false;
        }

        destIndex = m_model->index(row - 1, 1);
    }
    // 往下
    else
    {
        // 最后一行不能再往下了
        if (m_model->rowCount() == (row + 1))
        {
            return false;
        }

        destIndex = m_model->index(row + 1, 1);
    }

    // 获取位置
    auto currentItem = m_model->takeItem(currentIndex.row(), 1);
    auto destItem = m_model->takeItem(destIndex.row(), 1);

    // 交换位置
    m_model->setItem(destIndex.row(), 1, currentItem);
    m_model->setItem(currentIndex.row(), 1, destItem);
    ui->assay_tableView->update();

    return true;
}

void QAssayShowPostion::UpdateBtnState()
{
    ui->down_btn->setEnabled(false);
    ui->up_btn->setEnabled(false);
    ui->bottom_btn->setEnabled(false);
    ui->top_btn->setEnabled(false);

    int rowCount = m_model->rowCount();
    auto currentIndex = ui->assay_tableView->currentIndex();
    if (!currentIndex.isValid() || rowCount <= 1)
    {
        return;
    }

    int currentRow = currentIndex.row();

    // 第一行
    if (currentRow == 0)
    {
        ui->bottom_btn->setEnabled(true);
        ui->down_btn->setEnabled(true);
    }
    // 最后一行
    else if (currentRow == (rowCount - 1))
    {
        ui->up_btn->setEnabled(true);
        ui->top_btn->setEnabled(true);
    }
    else
    {
        ui->down_btn->setEnabled(true);
        ui->up_btn->setEnabled(true);
        ui->bottom_btn->setEnabled(true);
        ui->top_btn->setEnabled(true);
    }
}

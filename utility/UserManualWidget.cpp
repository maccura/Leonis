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
/// @file     UserManualWidget.cpp
/// @brief    用户手册界面
///
/// @author   6889/ChenWei
/// @date     2024年1月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "UserManualWidget.h"
#include "ui_UserManualWidget.h"
#include <QGraphicsScene>
#include <QScroller>
#include <QScrollBar>
#include <QDir>

#include "messagebus.h"
#include "msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"
#include "PdfPretreatment.h"
#include "PdfPage.h"
#include "BookmarkModel.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/CommonInformationManager.h"

#define  DELAYING_TIME          (800)               // 延迟时间
UserManualWidget::UserManualWidget(QWidget *parent)
    : QMainWindow(parent)
    , m_bInit(false)
    , m_dScalePercent(1.0)
    , m_iScrolValue(0)
{
    ui = new Ui::UserManualWidget();
    ui->setupUi(this);

    InitBeforeShow();
}

UserManualWidget::~UserManualWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}


///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::InitBeforeShow()
{
    setWindowFlags(Qt::FramelessWindowHint);
    ui->graphicsView->setBackgroundBrush(QBrush(QColor(0xed, 0xee, 0xf0)));
    ui->graphicsView->setFrameShape(QFrame::NoFrame);
    ui->graphicsView->centerOn(0, 0);
    m_resizeTimer.setSingleShot(true);
    ui->CurPageEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->treeView->setHeaderHidden(true);
    setCentralWidget(ui->graphicsView);

    QWidget* pDocTitle1 = ui->dockWidget->titleBarWidget();
    QWidget* pEmptyTitle = new QWidget();
    ui->dockWidget->setTitleBarWidget(pEmptyTitle);
    delete pDocTitle1;

    QWidget* pDocTitle2 = ui->dockWidget_2->titleBarWidget();
    QWidget* pEmptyTitle2 = new QWidget();
    ui->dockWidget_2->setTitleBarWidget(pEmptyTitle2);
    delete pDocTitle2;

}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::InitAfterShow()
{
    InitStrResource();
    InitSwitchoverBtn();
    SetScaleLab();
    //BuildBookmarkTree();
    InitConnect();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::InitStrResource()
{
    ui->ZoomInBtn->setText("");
    ui->ZoomOutBtn->setText("");
    ui->RecoverBtn->setText("");
    ui->PrePageBtn->setText("");
    ui->NextPageBtn->setText("");
    ui->FirstPageBtn->setText("");
    ui->LastPageBtn->setText("");
    ui->flat_return->setText(tr("返回"));
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::InitConnect()
{
    connect(ui->chBtn, SIGNAL(clicked()), this, SLOT(OnChClicked()));
    connect(ui->imBtn, SIGNAL(clicked()), this, SLOT(OnImClicked()));
    connect(ui->ZoomInBtn, SIGNAL(clicked()), this, SLOT(OnZoomInClicked()));
    connect(ui->ZoomOutBtn, SIGNAL(clicked()), this, SLOT(OnZoomOutClicked()));
    connect(ui->RecoverBtn, SIGNAL(clicked()), this, SLOT(OnRecoverClicked()));
    connect(ui->PrePageBtn, SIGNAL(clicked()), this, SLOT(OnPrePageClicked()));
    connect(ui->NextPageBtn, SIGNAL(clicked()), this, SLOT(OnNextPageClicked()));
    connect(ui->FirstPageBtn, SIGNAL(clicked()), this, SLOT(OnFirstPageClicked()));
    connect(ui->LastPageBtn, SIGNAL(clicked()), this, SLOT(OnLastPageClicked()));
    connect(ui->CurPageEdit, SIGNAL(returnPressed()), this, SLOT(JumpPage()));
    connect(ui->graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotSliderMoved(int)));
    connect(ui->treeView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(TreeItemClicked(const QModelIndex &)));
    connect(&m_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeDone()));
}

///
/// @brief
///     初始化说明书切换按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月25日，新建函数
///
void UserManualWidget::InitSwitchoverBtn()
{
    auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
    // 生免联机
    if (softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
    {
        ui->BtnWidget->setVisible(true);
        ui->treeView->setHeaderHidden(true);
        OnImClicked();
    }
    else if (softwareType == SOFTWARE_TYPE::IMMUNE)
    {
        // 免疫
        ui->BtnWidget->setVisible(false);
        ui->treeView->setHeaderHidden(false);
        OnImClicked();
    }
    else
    {
        // 生化
        ui->BtnWidget->setVisible(false);
        ui->treeView->setHeaderHidden(false);
        OnChClicked();
    }
}

///
/// @brief
///     获取用户手册名称
///
/// @par History:
/// @li 6889/ChenWei，2024年1月25日，新建函数
///
QString UserManualWidget::GetFileName(int iDocID)
{
    ULOG(LOG_INFO, "Printer:%s()", __FUNCTION__);

    QString strDirPath = QCoreApplication::applicationDirPath();
    QString strFilePath = strDirPath + "/ui_cfg/handbook";
    if (iDocID == 1)
    {
        bool hasC2000 = false;
        for (const auto& iter : CIM_INSTANCE->GetDeviceMaps())
        {
            // 只有C2000有组名
            if (!iter.second->groupName.empty())
            {
                hasC2000 = true;
                break;
            }
        }

        strFilePath += "/ch";
        if (hasC2000)
        {
            strFilePath += "/C2000";
        } 
        else
        {
            strFilePath += "/C1000";
        }
    }
    else if (iDocID == 2)
    {
        strFilePath += "/im";
    }

    QString strSuffix = tr("*.") + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + tr(".pdf");
    QDir dir(strFilePath);

    QStringList nameFilters;
    nameFilters << strSuffix;
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
    if (files.isEmpty())
    {
        return "";
    }

    QFileInfo info = files.first();
    return info.filePath();
}

///
/// @brief
///     加载说明书
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::LoadFile(QString strFileName, int iDocID)
{
    if (strFileName.isEmpty())
    {
        return;
    }

    PdfPretreatment::Error err = PdfPretreatment::GetInstance()->loadFile(strFileName, "", iDocID);
    ULOG(LOG_INFO, "PDF loadFile ：%d", int(err));

    int iCurDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto DocIt = m_Books.find(iCurDocID);
    if (DocIt == m_Books.end())
    {
        std::vector<std::shared_ptr<PdfPage>> Pages;
        m_Books[iCurDocID].m_PdfPages = Pages;
        m_Books[iCurDocID].m_pDocScene = new QGraphicsScene(this);
        ui->graphicsView->setScene(m_Books[iCurDocID].m_pDocScene);

        // 加载文档到界面
        int iCurHeight = 10;
        int iCurWidth = 10;
        int iPageCount = PdfPretreatment::GetInstance()->GetPageCount();
        for (int i = 0; i < iPageCount; i++)
        {
            QSizeF size = PdfPretreatment::GetInstance()->GetPageSize(i);
            std::shared_ptr<PdfPage> page = std::make_shared<PdfPage>(i, size);
            m_Books[iCurDocID].m_PdfPages.push_back(page);
            m_Books[iCurDocID].m_pDocScene->addItem(page.get());
            page->setPos(10, iCurHeight);
            iCurHeight += page->Height() + 20;
            if (page->Width() > iCurWidth)
            {
                iCurWidth = page->Width();
            }
        }

        m_Books[iCurDocID].m_pDocScene->setSceneRect(QRectF(0, 0, iCurWidth, iCurHeight));

        // 页面加载完，创建目录
        m_Books[iDocID].m_Bookmark = new BookmarkModel(this);
        m_Books[iDocID].m_Bookmark->BuildTree();
    }
    else
    {
        ui->graphicsView->setScene(m_Books[iCurDocID].m_pDocScene);
    }

    SetPageNumber();
    ui->PageCountLab->setText(tr("/") + QString::number(m_Books[iCurDocID].m_PdfPages.size()) + tr("页"));
    SetpageIsVisible(m_Books[iCurDocID].m_iCurrentPage, pageIsVisible(m_Books[iCurDocID].m_iCurrentPage));
    SetpageIsVisible(m_Books[iCurDocID].m_iCurrentPage + 1, pageIsVisible(m_Books[iCurDocID].m_iCurrentPage + 1));
    SetpageIsVisible(m_Books[iCurDocID].m_iCurrentPage - 1, pageIsVisible(m_Books[iCurDocID].m_iCurrentPage - 1));
}

///
/// @brief
///     构建目录
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::BuildBookmarkTree()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto ModelIter = m_Books.find(iDocID);
    if (ModelIter == m_Books.end())
    {
        return;
    }

    ui->treeView->setModel(m_Books[iDocID].m_Bookmark);
}

///
/// @brief
///     调整页面大于
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::resizeDone()
{
    SetScaleLab();

    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto Book = m_Books.find(iDocID);
    if (Book == m_Books.end())
    {
        return;
    }

    // 调整坐标
    int iCurHeight = 10;
    int iCurWidth = 10;
    for (auto page : m_Books[iDocID].m_PdfPages)
    {
        page->Scaled(m_dScalePercent);
        page->setPos(10, iCurHeight);
        iCurHeight += page->Height() + 20;
        iCurWidth = page->Width();
    }

    m_Books[iDocID].m_pDocScene->setSceneRect(QRectF(0, 0, iCurWidth, iCurHeight));
    ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    SetpageIsVisible(m_Books[iDocID].m_iCurrentPage, pageIsVisible(m_Books[iDocID].m_iCurrentPage));
    SetpageIsVisible(m_Books[iDocID].m_iCurrentPage + 1, pageIsVisible(m_Books[iDocID].m_iCurrentPage + 1));
    SetpageIsVisible(m_Books[iDocID].m_iCurrentPage - 1, pageIsVisible(m_Books[iDocID].m_iCurrentPage - 1));
}

///
/// @brief
///     响应放大按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnZoomInClicked()
{
    m_dScalePercent = m_dScalePercent + 0.1;
    m_resizeTimer.start(DELAYING_TIME);
    SetScaleLab();
    //resizeDone();
}

///
/// @brief
///     响应缩小按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnZoomOutClicked()
{
    double scalePercent = m_dScalePercent - 0.1;
    if (scalePercent < 0.1)
    {
        return;
    }

    m_dScalePercent = scalePercent;
    m_resizeTimer.start(DELAYING_TIME);
    SetScaleLab();
    //resizeDone();
}

///
/// @brief
///     显示缩放比例
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::SetScaleLab()
{
    QString strScale = QString::number(m_dScalePercent * 100, 'f', 0) + tr("%");
    ui->scaleLab->setText(strScale);
}

///
/// @brief
///     响应恢复按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnRecoverClicked()
{
    m_dScalePercent = 1.0;
    m_resizeTimer.start(DELAYING_TIME);
    SetScaleLab();
    //resizeDone();
}

///
/// @brief
///     响应上一页按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnPrePageClicked()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    if ((!m_Books[iDocID].m_PdfPages.empty()) && (m_Books[iDocID].m_iCurrentPage > 1))
    {
        m_Books[iDocID].m_iCurrentPage--;
        ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
        SetPageNumber();
    }
    else
    {
        m_Books[iDocID].m_iCurrentPage = 1;
    }
}

///
/// @brief
///     响应下一页按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnNextPageClicked()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    if ((!m_Books[iDocID].m_PdfPages.empty()) && (m_Books[iDocID].m_PdfPages.size() > m_Books[iDocID].m_iCurrentPage))
    {
        m_Books[iDocID].m_iCurrentPage++;
        ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
        SetPageNumber();
    }
    else
    {
        m_Books[iDocID].m_iCurrentPage = m_Books[iDocID].m_PdfPages.size();
    }
}

///
/// @brief
///     响应第一页按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnFirstPageClicked()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    if ((!m_Books[iDocID].m_PdfPages.empty()) && (m_Books[iDocID].m_iCurrentPage > 1))
    {
        m_Books[iDocID].m_iCurrentPage = 1;
        ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    }
}

///
/// @brief
///     响应最后一页按钮
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnLastPageClicked()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    if ((!m_Books[iDocID].m_PdfPages.empty()) && (m_Books[iDocID].m_PdfPages.size() > m_Books[iDocID].m_iCurrentPage))
    {
        m_Books[iDocID].m_iCurrentPage = m_Books[iDocID].m_PdfPages.size();
        ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    }
}

///
/// @brief
///     跳转页面到指定页
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::JumpPage()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end() || it->second.m_PdfPages.empty())
    {
        return;
    }

    int iPage = ui->CurPageEdit->text().toInt();
    if ((m_Books[iDocID].m_PdfPages.size() < iPage) || iPage <= 0)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("请输入合适的页数！")));
        pTipDlg->exec();
        SetPageNumber();
        return;
    }

    m_Books[iDocID].m_iCurrentPage = iPage;
    ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
}

///
/// @brief
///     目录被点击
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::TreeItemClicked(const QModelIndex& index)
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    int iPageIndex = m_Books[iDocID].m_Bookmark->GetPageIndex(index);
    if ((!m_Books[iDocID].m_PdfPages.empty()) && (m_Books[iDocID].m_PdfPages.size() > iPageIndex) && iPageIndex >= 0)
    {
        m_Books[iDocID].m_iCurrentPage = iPageIndex + 1;
        ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    }
}

///
/// @brief
///     显示当前页
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::SetPageNumber()
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
    {
        return;
    }

    ui->CurPageEdit->setText(QString::number(it->second.m_iCurrentPage));
}

///
/// @brief
///     响应页面滑动
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::slotSliderMoved(int iValue)
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    auto it = m_Books.find(iDocID);
    if (it == m_Books.end())
        return;

    if (ui->graphicsView->verticalScrollBar()->minimum() == iValue)
    {
        it->second.m_iCurrentPage = 1;
    }
    else if (ui->graphicsView->verticalScrollBar()->maximum() == iValue)
    {
        int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
        it->second.m_iCurrentPage = it->second.m_PdfPages.size();
    }

    // 更新当前页可见状态
    bool bPageV = pageIsVisible(it->second.m_iCurrentPage);
    SetpageIsVisible(it->second.m_iCurrentPage, bPageV);
    if (!bPageV)
    {
        if (iValue > m_iScrolValue)
        {
            it->second.m_iCurrentPage++;
        }
        else
        {
            it->second.m_iCurrentPage--;
            if (it->second.m_iCurrentPage <= 0)
                it->second.m_iCurrentPage = 1;
        }

        // 将新出现的一页置为可见
        SetpageIsVisible(it->second.m_iCurrentPage, true);
    }

    m_iScrolValue = iValue;
    SetPageNumber();
    SetpageIsVisible(it->second.m_iCurrentPage + 1, pageIsVisible(it->second.m_iCurrentPage + 1));
    SetpageIsVisible(it->second.m_iCurrentPage - 1, pageIsVisible(it->second.m_iCurrentPage - 1));
}

///
/// @brief
///     设置页面可见状态
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::SetpageIsVisible(int iPageIndex, bool bFlage)
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    if (iPageIndex - 1 >= 0 && iPageIndex - 1 < m_Books[iDocID].m_PdfPages.size())
    {
        m_Books[iDocID].m_PdfPages.at(iPageIndex - 1)->SetVisibleFlag(bFlage);
    }
}

///
/// @brief
///     判断页面是否可见
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
bool UserManualWidget::pageIsVisible(int iPageIndex)
{
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    QGraphicsView* view = ui->graphicsView;
    if (iPageIndex > m_Books[iDocID].m_PdfPages.size() || iPageIndex - 1 < 0)
        return false;

    auto page = m_Books[iDocID].m_PdfPages.at(iPageIndex - 1);
    return page->mapToScene(page->boundingRect()).boundingRect().intersects(
        view->mapToScene(view->viewport()->geometry()).boundingRect()
    );
}

///
/// @brief
///     计算页面跳转区域
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QRectF UserManualWidget::calcPageShift()
{
    // 越界时窗口不跳转
    int iDocID = PdfPretreatment::GetInstance()->GetCurDocID();
    if (m_Books[iDocID].m_iCurrentPage - 1 >= m_Books[iDocID].m_PdfPages.size())
    {
        return QRectF(0, 0, 0, 0);
    }

    QGraphicsView *view = ui->graphicsView;
    auto page = m_Books[iDocID].m_PdfPages.at(m_Books[iDocID].m_iCurrentPage - 1);
    QRectF pageRect = page->boundingRect();
    qreal pageHeight = page->mapToScene(pageRect).boundingRect().height();
    qreal viewHeight = view->mapToScene(0, view->viewport()->height()).y()
        - view->mapToScene(0, 0).y();

    viewHeight = (pageHeight < viewHeight) ? pageHeight : viewHeight;
    QRectF pageStartPos = page->mapRectToScene(pageRect);
    return QRectF(0, pageStartPos.y(), 0, viewHeight);
}

///
/// @brief
///     打开生化手册
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnChClicked()
{
    QString strFileName = GetFileName(1);
    if (strFileName.isEmpty())
    {
        return;
    }

    LoadFile(strFileName, 1);
    BuildBookmarkTree();
    ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    ui->chBtn->setChecked(true);
    ui->imBtn->setChecked(false);
}

///
/// @brief
///     打开免疫手册
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void UserManualWidget::OnImClicked()
{
    QString strFileName = GetFileName(2);
    if (strFileName.isEmpty())
    {
        return;
    }

    LoadFile(strFileName, 2);
    BuildBookmarkTree();
    ui->graphicsView->ensureVisible(calcPageShift(), 0, 0);
    ui->imBtn->setChecked(true);
    ui->chBtn->setChecked(false);
}


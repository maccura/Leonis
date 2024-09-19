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

#pragma once

#include <QWidget>
#include "QSortFilterProxyModel"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "PrintExportDefine.h"

// 前置声明
class QQcRegSample;
class QcDocRegDlg;
class QQcDownloadRegDlg;
class QPostionEdit;
class SortHeaderView;

namespace Ui {
    class QQcSettingPage;
};


class QcSortFilterProxyModel :public QSortFilterProxyModel
{
public:
    QcSortFilterProxyModel(QObject* obj)
        :QSortFilterProxyModel(obj)
    {}

    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
};


class QQcSettingPage : public QWidget
{
    Q_OBJECT

public:
    QQcSettingPage(QWidget *parent = Q_NULLPTR);
    ~QQcSettingPage();

    // 质控登记页面类型
    enum QcRegPageType
    {
        REG_PAGE_TYPE_SCAN = 0,     // 扫描登记
        REG_PAGE_TYPE_MANUAL,       // 手工登记
        REG_PAGE_TYPE_DOWNLOAD,     // 下载登记
        REG_PAGE_TYPE_MODIFY        // 修改登记
    };

protected:
    void Init();

    ///
    /// @brief
    ///     显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月7日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     切换质控登记页面类型
    ///
    /// @param[in]  enPageType  页面类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    bool SwitchQcRegPage(QcRegPageType enPageType);

    protected slots:

    ///
    /// @bref
    ///		表格行被点击（试剂位置设置）
    ///
    /// @param[in] index 点击的行位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月24日，新建函数
    ///
    void OnClickedRow(const QModelIndex& index);
    void OnSavePostion();

    void OnTabelSelectChanged(const QModelIndex& cur, const QModelIndex& pre);

    ///
    /// @brief
    ///     查询条件改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月10日，新建函数
    ///
    void OnQryCondChanged();

    ///
    /// @bref
    ///		处理扫描登记、手动登记、修改质控
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月25日，新建函数
    ///
    void OnClickOpRegDoc();

    ///
    /// @brief
    ///     显示数据改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月7日，新建函数
    ///
    void OnDisplayDataChanged();

    ///
    /// @brief 删除质控文档
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年7月29日，新建函数
    ///
    void OnDelete();

    ///
    /// @brief 获取界面显示的质控文档信息
    ///
    /// @param[in]  vecQcDocUpdate  
    /// @param[in]  beDeleteIndexes  
    /// @param[in]  vecQcDoc  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年7月29日，新建函数
    ///
    bool GetSelectionQqDoc(std::set<int>& beDeleteIndexes, std::vector<::tf::QcDoc>& vecQcDoc);

    virtual void showEvent(QShowEvent *event) override;

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月3日，新建函数
    ///
    void OnPermisionChanged();

    private slots:
    ///
    /// @brief 质控文档更新通知
    ///  
    /// @param[in]  infos  更新的质控文档信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年10月19日，新建函数
    ///
    void OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos);

    ///
    /// @brief
    ///     角落查询条件被重置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月16日，新建函数
    ///
    void OnTabCornerQryCondReset();

    ///
    /// @brief
    ///     更新条件显示字符串
    ///
    /// @param[in]  strCond  筛选条件字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月16日，新建函数
    ///
    void UpdateCondDisplayStr(QString strCond);

    void OnQcRuleSettings();

    void InitQcRegDlg(const QModelIndex& currSel);

    void OnClickedQcDocSearch();
    void OnClickedQcRest();

    ///
    /// @brief
    ///     质控登记对话框切换扫描登记页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void OnQcRegDlgSwitchScanRegPage();

    ///
    /// @brief
    ///     质控登记对话框切换手工登记页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void OnQcRegDlgSwitchManualRegPage();

    ///
    /// @brief
    ///     质控登记对话框切换质控下载页面
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void OnQcRegDlgSwitchQcDownloadPage();

    ///
    /// @brief
    ///     质控申请信息更新(临时)
    ///
    /// @param[in]  qa  质控申请信息
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年8月22日，新建函数
    ///
    void OnImQcApplyInfoUpdate(im::tf::QcApply qa);

    ///
    /// @bref
    ///		当项目信息中的单位变化时，需要更新质控成分中的单位信息，与相关换算
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月25日，新建函数
    ///
    void OnAssayInfoChanged();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月14日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月14日，新建函数
    ///
    void OnExportBtnClicked();

    void GetPrintExportInfo(QCItemRecord& info);
    ///
    /// @bref
    ///		响应提示信息信号
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年1月29日，新建函数
    ///
    void OnHint(QString strInfo);

signals:
    void hint(QString strInfo);         // 提示信息

private:
    bool GetCurrentOporationSel(QModelIndex& rowIdx);

    // 更新化质控品名称CombBox
    void ReloadQcDocCombData();

    void UpdateTableView(const std::string& qcDocNameFilter, tf::DocRegistType::type registType);

    void UpdateAssay(const QModelIndex& index);

    void UpdateDeleteButtonStatu();

    ///
    /// @bref
    ///		初始化注册类型过滤下拉框
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月27日，新建函数
    ///
    void InitRegisFilterCombox();

    ///
    /// @bref
    ///		获取质控品id对参与联合质控项目的对应
    ///
    /// @param[out] qcDocMap 对应关系
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月11日，新建函数
    ///
    void GetJionUnionQcAssay(std::map<int64_t, std::set<std::string>>& qcDocMap);

    ///
    /// @bref
    ///		检查质控品是否参与了联合质控，返回参与联合质控的项目
    ///
    /// @param[in] vecQcDoc 待检测的质控品
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月11日，新建函数
    ///
    QString GetJoinUnionQcAssayNames(std::vector<::tf::QcDoc>& vecQcDoc);

private:
    Ui::QQcSettingPage *ui;
    QQcRegSample*					m_settingDialog;
    QcDocRegDlg*					m_pQcRegDlg;                        // 质控登记对话框
    QQcDownloadRegDlg*              m_downloadRegDlg;					// 下载登记对话框
    QPostionEdit*					m_PostionEditDialog;

    QSortFilterProxyModel*          m_qSortModel;
    SortHeaderView*                 m_pSortHeader;                      // 表格的排序头

    bool                            m_bInit;                            // 是否初始化
};

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
/// @file     QAutoMaintainCfgDlg.h
/// @brief 	  自动维护设置
///
/// @author   7656/zhang.changjiang
/// @date      2023年2月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年2月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include <QSortFilterProxyModel>
#include "src/thrift/gen-cpp/defs_types.h"

class QStandardItemModel;
class MaintainGroupModel;
class QAutoMaintainFilterProxyModel;
class QCustomCard;
class QComboBox;
class QPushButton;
class QLineEdit;
class QTableView;

namespace Ui {
    class QAutoMaintainCfgDlg;
};

///
///  @brief 
///     用于缓存一行中的控件
///
class UiRowCtrl : public QObject
{
    Q_OBJECT

public:
    QComboBox* m_devComb;       // 设备下拉框
    QLineEdit* m_intervEdit;    // 间隔时间框
    QCustomCard* m_cardTime;    // 定时设置框
    QPushButton* m_clearBtn;    // 清空按钮

    int m_rowIdx; // 控件所在位置
    QTableView* m_parentTb;

    explicit UiRowCtrl(int rowIdx, QTableView *parent = nullptr);
    ~UiRowCtrl();

    ///
    /// @bref
    ///		值是否可用
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    inline bool IsValid()
    {
        return m_devComb != nullptr && m_intervEdit != nullptr && m_cardTime != nullptr && m_clearBtn != nullptr;
    }

    ///
    /// @bref
    ///		初始化信号槽连接
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年12月29日，新建函数
    ///
    void InitConnect();

    // 焦点
    bool eventFilter(QObject *obj, QEvent *event) override;

    public slots:

    ///
    /// @bref
    ///		初始化
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年12月28日，新建函数
    ///
    void ClearAll();

    ///
    /// @bref
    ///		清空按钮
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年12月28日，新建函数
    ///
    void BtnClear();

    ///
    /// @bref
    ///		初始化
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年12月28日，新建函数
    ///
    void ChengeClearBtnState();
};

class QAutoMaintainCfgDlg : public BaseDlg
{
    Q_OBJECT

public:
    enum AUTO_MAINTAIN_COL
    {
        DEVICE_COL,				// 仪器
        TIME_INTERVAL_COL,		// 间隔时间（小时）
        AUTO_TIME_COL,			// 自动维护时间
        CLEAR_COL,				// 清空
    };
    QAutoMaintainCfgDlg(QWidget *parent = Q_NULLPTR);
    ~QAutoMaintainCfgDlg();

    ///
    ///  @brief 初始化
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年2月27日，新建函数
    ///
    void Init();

    ///
    ///  @brief 连接信号槽
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年2月27日，新建函数
    ///
    void ConnectSlots();

protected:
    signals :
            void OnClearSignal();

            private slots:

            ///
            ///  @brief 确认按钮被点击
            ///
            void OnOkBtnClicked();

            ///
            ///  @brief 取消按钮被点击
            ///
            void OnCancerBtnClicked();

            ///
            /// @bref
            ///		左侧维护组名选择发生变化
            ///
            /// @param[in] current 当前选择
            /// @param[in] previous 之前选择
            ///
            /// @par History:
            /// @li 8276/huchunli, 2023年3月22日，新建函数
            ///
            void OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

            ///
            /// @bref
            ///		权限变化响应
            ///
            /// @par History:
            /// @li 8276/huchunli, 2024年4月10日，新建函数
            ///
            void OnPermisionChanged();

            virtual void showEvent(QShowEvent *event) override;

private:

    ///
    /// @bref
    ///		初始化界面右侧的时间设置控件
    ///
    /// @param[in] autoMaintItems 自动维护配置
    /// 
    /// @par History:
    /// @li 8276/huchunli, 2023年3月21日，新建函数
    ///
    void UpdateView(const tf::MaintainGroup& currentGroup, const std::vector<tf::AutoMaintainCfg>& autoMaintItems);

    ///
    /// @bref
    ///		获取UI上自动维护设置
    ///
    /// @param[out] maintCfgs 获取到的界面上的维护设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月29日，新建函数
    ///
    void GetMaintainUiCfg(std::vector<tf::AutoMaintainCfg>& maintCfgs);

    ///
    /// @bref
    ///		初始化设备下拉列框
    ///
    /// @param[in] comb 下拉框控件
    /// @param[in] groupId 维护组ID
    /// @param[in] crrentDevSn 当前设备序号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    void InitDeviceComb(QComboBox* comb, const tf::MaintainGroup& currentGroup, const std::string& crrentDevSn);

    ///
    /// @bref
    ///		初始化一行的周时间选择框
    ///
    /// @param[in] currRowIdx 当前行索引
    /// @param[out] rowCtrl 缓存的控件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    void InitRowContentUi(int currRowIdx, UiRowCtrl* rowCtrl);

    ///
    /// @bref
    ///		从界面获取设备SN设置
    ///
    /// @param[in] rowIdx 界面行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    QString GetDevSnFromUI(int rowIdx);

    ///
    /// @bref
    ///		从界面获取自动维护的间隔时间设置
    ///
    /// @param[in] rowIdx 界面行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    QString GetIntervlTimeFromUI(int rowIdx);

    ///
    /// @bref
    ///		从界面获取自动维护的定时维护设置
    ///
    /// @param[in] rowIdx 界面行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    void GetFixtimeFromUI(int rowIdx, QString& strTime, std::vector<int>& vecDays);

    ///
    /// @bref
    ///		缓存发生修改的配置
    ///
    /// @param[in] groupName 配置名
    /// @param[in] cfg 自动维护配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月5日，新建函数
    ///
    void CacheChangedCfg(const QString& groupName, const std::vector<tf::AutoMaintainCfg>& cfg);

    ///
    /// @bref
    ///		缓存界面修改
    ///
    /// @param[in] rowIdx 维护组名的目标行
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月6日，新建函数
    ///
    void CacheCfgFromUI(const QModelIndex& rowIdx);

    ///
    /// @bref
    ///		比较两个自动维护配置是否相等，相等返回真
    ///
    /// @param[in] newCfg 新的自动维护配置
    /// @param[in] oldCfg 旧的自动维护配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月6日，新建函数
    ///
    bool CompareAutoMaintenCfg(const std::vector<tf::AutoMaintainCfg>& newCfg, const std::vector<tf::AutoMaintainCfg>& oldCfg);

    ///
    /// @bref
    ///		检查是否存在只设置了时间，没有设置周的情况, 合法则返回真
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月6日，新建函数
    ///
    bool CheckWeekValid();
    void ClearRepeatMaintainCfg(const std::vector<tf::AutoMaintainCfg>& maintanCfg, std::vector<tf::AutoMaintainCfg>& noRepeatCfg);

private:
    Ui::QAutoMaintainCfgDlg *ui;
    QAutoMaintainFilterProxyModel*	m_pMaintainGroupModel;
    QStandardItemModel*				m_autoMaintModel;

    std::vector<UiRowCtrl*>          m_rowsCtrl; // 记录表格中所有控件
    std::map<QString, std::vector<tf::AutoMaintainCfg>> m_changedCfgs;  // 发生修改的维护配置

    std::vector<tf::MaintainGroup>  m_origData; // 缓存设置前的数据，用于取消设置时还原单例Model数据
};

///////////////////////////////////////////////////////////////////////////
/// @class     QAutoMaintainFilterProxyModel
/// @brief 	   维护组筛选排序模型代理
///////////////////////////////////////////////////////////////////////////
class QAutoMaintainFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    QAutoMaintainFilterProxyModel(QObject* parent = nullptr);
    ~QAutoMaintainFilterProxyModel();
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

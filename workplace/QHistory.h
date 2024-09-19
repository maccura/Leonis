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
/// @file     QHistory.h
/// @brief    数据浏览页面
///
/// @author   5774/WuHongTao
/// @date     2022年5月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/public/ConfigDefine.h"
#include "QWorkShellPage.h"
#include "QHistorySampleAssayModel.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "PrintExportDefine.h"
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <QComboBox>

class CommonInformationManager;
class QPushSampleToRemote;
class QReCheckDialog;
class QHistoryFilterSample;
class QAssayResultDetail;
class QStandardItemModel;
class QHistoryItemResultDetailDlg;
class QDialogAi;
// 前置声明
class QHistorySampleFilterDataModule;
class QTableView;
class QCheckBox;
class QPushButton;
class CustomButton;
class PrintSetDlg;
namespace Ui {
    class QHistory;
};


class QHistory : public QWorkShellPage
{
	Q_OBJECT

public:
	QHistory(QWidget *parent = Q_NULLPTR);
	~QHistory();

	///
	/// @brief 刷新右上坐标控件
	///
	/// @param[in]  index  页面序号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月25日，新建函数
	///
	void RefreshCornerWidgets(int index)override;

protected:
	///
	/// @brief 初始化页面
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月25日，新建函数
	///
	void Init();

	void UpdateCheckAllBoxStatus();

	///
	/// @brief 更新按钮状态
	///
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月18日，新建函数
	///
	void UpdateButtonStatus();

	///
	/// @bref
	///		对样本条码下拉框赋值
	///
	/// @par History:
	/// @li 8276/huchunli, 2023年9月21日，新建函数
	///
	void AssignSampleComb(QComboBox* pComb);

	///
	/// @brief 重置项目详情列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月25日，新建函数
	///
	virtual void ResetAssayModel();

	///
	/// @brief 初始化表头
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月4日，新建函数
	///
	void InitHeader(QTableView* tableView, QCheckBox*& selectCheckBtn, QPushButton*& statusBtn);

	///
	/// @brief 重置数据浏览样本筛选数据
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void ResetSampleSelectModel();

	///
	/// @brief 设置样本浏览模型
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void SetSampleBrowseModel();

	///
	/// @brief 删除选中的样本
	///
	///
	/// @return true删除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	bool DealSeriesSampIe(int method);

    ///
    /// @brief 时间过滤器
    ///
    /// @param[in]  target  目标对象
    /// @param[in]  event   时间对象
    ///
    /// @return 处理则返回true，否则返回false
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月12日，新建函数
    ///
    virtual bool eventFilter(QObject* target, QEvent* event) override;

    ///
    /// @brief 刷新结果详情页面
    ///
    /// @param[in]  dialog  结果详情的对话框
    ///
    /// @return true,刷新成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    bool ShowSampleDetail(QHistoryItemResultDetailDlg* dialog);

    ///
    /// @brief 页面允许功能
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月29日，新建函数
    ///
    virtual void PageAllowFun();

protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

    ///
    /// @brief 获取单样本的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintData(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds);

    ///
    /// @brief 获取按样本打印的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintDataBySample(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds);

    ///
    /// @brief 获取按项目打印的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
    /// @param[in]   assayCode        要打印项目的编号，后续可根据业务需求换成其他标识
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintItemData(ItemSampleInfoVector& vecItemDatas, EM_RESULT_MODE ResultMode);

private:
	
	///
	///  @brief 查询病人信息
	///
	///
	///  @param[in]   patientId  病人信息id
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月24日，新建函数
	///
	::tf::PatientInfo QueryPatientInfoById(int64_t patientId);
	
	///
	///  @brief 根据设备序列号查询设备类型
	///
	///
	///  @param[in]   deviceSN  设备序列号
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
	///
	boost::optional<::tf::AssayClassify::type> QueryAssayClassifyBySn(const std::string& deviceSN);
	

	///
	/// @brief  参考范围
	///
	/// @param[in]  resultid  结果id
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年8月9日，新建函数
	///
	std::string QueryRefRange(int64_t resultid);

	///
	/// @brief 检查行是否有效
	///
	/// @param[in]  row  行数
	///
	/// @return true:有效，false:无效
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月17日，新建函数
	///
	inline bool CheckIndexRange(int row) { return (row >= 0 && row < QHistorySampleAssayModel::Instance().rowCount(QModelIndex())); }

	///
	/// @brief 取消快捷筛选按钮的check状态
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月16日，新建函数
	///
	void UnCheckedFastFilterBtn();

	///
	/// @brief 根据筛选条件取消快捷筛选按钮的check状态
	///
	/// @param[in]  cond  筛选条件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月30日，新建函数
	///
	void UnCheckedFastFilterBtn(const ::tf::HistoryBaseDataQueryCond& cond);

	///
	/// @brief 查询条件转换为字符串
	///
	/// @param[in]    cond
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月23日，新建函数
	///
	QString QueryCondToStr(const ::tf::HistoryBaseDataQueryCond& cond);

protected:
    signals:
    // 显示之后发送
    void afterShow();

protected slots:

	void OnUpdateColumn();
    ///
    /// @bref
    ///		显示之后执行
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月27日，新建函数
    ///
    void OnAfterShow();

	///
	/// @brief 查询历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月29日，新建函数
	///
	void OnSearch();

	///
	/// @brief 模式切换
	///
	/// @param[in]  detectionSetting  切换参数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年3月24日，新建函数
	///
	void OnChangeMode(DetectionSetting detectionSetting);

	///
	/// @brief 向上查询历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月29日，新建函数
	///
	void OnSearchPre();

	///
	/// @brief 向下查询历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月29日，新建函数
	///
	void OnSearchNext();
	
	///
	/// @brief 显示样本的详细信息
	///
	/// @param[in]  index  样本的位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月25日，新建函数
	///
	void OnShowSampleDetail(const QModelIndex& index);

	///
	/// @brief 显示结果详情
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void OnShowResultDetail();

	///
	/// @brief 筛选对话框
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void OnShowFilterDialog();

	///
	/// @brief 筛选范围页面
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月1日，新建函数
	///
	void OnReturnPage();

	///
	/// @brief 刷新过滤样本
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月1日，新建函数
	///
	void OnShowFilterSampleInfo();

	///
	///
	/// @brief 快捷筛选
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月14日，新建函数
	///
	void OnFastFilterSampleInfo();
    void OnHightLightChanged(int index);
    void OnAIVisonResultIdentifyClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

	///
	/// @brief 初始化快捷筛选按钮
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月14日，新建函数
	///
	void OnfastFilterFinished(const ::tf::HistoryBaseDataQueryCond& cond);

    ///
    /// @brief  响应打印按钮
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月23日，新建函数
    ///
    void OnPrintBtnClicked();

	///
	/// @brief
	///     数据导出按钮被选中
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年4月6日，新建函数
	///
    void OnExportBtnClicked();

    ///
    /// @brief 查询测试结果
    ///
    /// @param[in]  itemId  testItem表主键
    ///
    /// @return 测试结果（可能不存在）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月24日，新建函数
    ///
    std::vector<ch::tf::AssayTestResult> GetChAssayTestResult(int64_t itemId);
    std::vector<ise::tf::AssayTestResult> GetIseAssayTestResult(int64_t itemId);

    ///
    /// @brief 获取病人信息
    ///
    /// @param[in]  id  主键
    ///
    /// @return 病人信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月24日，新建函数
    ///
    std::shared_ptr<tf::PatientInfo> GetPatientInfoById(int64_t id);

    ///
    /// @brief 将标准string导出为QString (导出数据不能为空，默认保留一个空格)
    ///
    /// @param[in]  src  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    QString ToExQString(std::string& src) {
        QString dest = " ";
        if (src.empty())
            return dest;

        return QString::fromStdString(src);
    }
	
	///
	///  @brief 按样本导出
	///
	///
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool ExportSample();
    bool ExportSampleData(const QString& path);
    bool ExportSample_ch(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& dynamicTitle, QVariantList& rowDatas);
    bool ExportSample_ch_cali(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& dynamicTitle, QVariantList& rowDatas, int times);
    bool ExportSample_ise(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas);
    bool ExportSample_ise_cali(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas, int times);
    bool ExportSample_calc(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas);
    bool ExportSample_im();
    bool ExportData_im(QString fileName, tf::UserType::type UType, std::vector<std::shared_ptr<HistoryBaseDataSample>>& vecSample);
    bool ExportItemData_im(QString fileName, const QModelIndexList &selectedIndexs);

	///
	///  @brief 按项目导出
	///
	///
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool ExPortItem();
    bool ExPortItemData(const QString& path);
    void MakeExportItemResult(bool isExportSample, SampleExportInfo& SampleInfo, const std::shared_ptr<HistoryBaseDataSample>& pSample, 
        const std::shared_ptr<HistoryBaseDataByItem>& pItem, const std::shared_ptr<im::tf::AssayTestResult>& pFirstTestResult, 
        const std::shared_ptr<im::tf::AssayTestResult>& pLastTestResult);

	void OnShowBySample();
	void OnShowByAssay();

    ///
    /// @brief 下一条
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    void OnMoveBtn(bool direction);

    ///
    /// @brief 数据改变的时候刷新页面数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月19日，新建函数
    ///
    void OnDataChanged();

    ///
    /// @bref
    ///		数据刷新
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月15日，新建函数
    ///
    void OnRefreshRow(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs);

    ///
    /// @brief 手工传输
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    void OnTransfer();

    ///
    /// @brief 复查样本
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月22日，新建函数
    ///
    void OnRecheckSample();

	///
	/// @brief 审核样本
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月5日，新建函数
	///
    void OnCheckSample();

    ///
    /// @brief 获取样本中项目测试次数最大值（可能样本中某个项目比其它项目测试次数更多）
    ///
    /// @param[in]  sampleId  
    ///
    /// @return 返回最多的次数
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年3月14日，新建函数
    ///
    int GetMaxTestCount(int64_t sampleId);
    
	///
	/// @brief 清除当前筛选
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月16日，新建函数
	///
	void OnCloseFilter();

	///
	/// @brief 处理样本审核状态改变消息
	///
	/// @param[in]  vecSampleID		发生改变的样本ID
	/// @param[in]  bStatus			发生改变的样本当前状态
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月29日，新建函数
	///
	void OnAuditChanged(class std::vector<__int64, class std::allocator<__int64> > vecSampleID, bool bStatus);

    ///
    /// @brief 更新工作页面的按钮状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月15日，新建函数
    ///
    void OnUpdateButtonStatus();

	///
	/// @brief 处理样本上传Lis状态改变消息
	///
	/// @param[in]  sampleID  样本ID
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月30日，新建函数
	///
	void OnLisChanged(int64_t sampleID);

	///
	/// @brief 处理复查结果更新消息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月14日，新建函数
	///
	void OnRetestResultChanged(::tf::HistoryBaseDataUserUseRetestUpdate ri);

	///
	/// @brief 处理复查结果更新消息
	///  
	/// @param[in]  vol  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月14日，新建函数
	///
	void OnUIRetestResultChanged(std::tuple<std::string, std::string, int64_t> vol);
    
    ///
    /// @bref
    ///		样本表格被点击
    ///
    /// @param[in] current 被点击的行
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月25日，新建函数（来自于重构提取
    ///
    void OnSampleListClick(const QModelIndex& current);
    void OnAssayListClick(const QModelIndex& current);

    ///
    /// @bref
    ///		样本列表选中行变化
    ///
    /// @param[in] current 当前选中
    /// @param[in] pre 上次选中
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月25日，新建函数（来自重构提取
    ///
    void OnSampleListSelChanged(const QModelIndex& current, const QModelIndex& pre);
    void OnAssayListSelChanged(const QModelIndex& current, const QModelIndex& pre);

    void ProcScrollBarValueChanged(QTableView* qTb, QCheckBox* qCheck, QPushButton* qBtn);

	void ProSelect(QTableView* tableView , const QModelIndex& current);

private:
        ///
        /// @bref
        ///		查找数据
        ///
        /// @param[in] direction 查找方向（1：向上， 2：向下）
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年12月6日，新建函数
        ///
        void CheckAndSearch(int direction);

        ///
        /// @bref
        ///		通过sampleId和itemId从i6000_assaytestresult表中获取测试结果信息
        ///
        /// @param[in] sampleId 样本id
        /// @param[in] itemId 测试项id
        /// @param[in] retime 该样本复查的最大结束时间（用于判断定位复查结果
        /// @param[out] pFirst 初测结果
        /// @param[out] pReTest 复查结果
        ///
        /// @par History:
        /// @li 8276/huchunli, 2024年7月12日，新建函数
        ///
        void GetImAssayTestResult(int64_t sampleId, int64_t itemId, const boost::posix_time::ptime& retime,
            std::shared_ptr<::im::tf::AssayTestResult>& pFirst, std::shared_ptr<::im::tf::AssayTestResult>& pReTest);

protected:
	Ui::QHistory*			ui;
	QStandardItemModel*				m_selectSampleModel;		///< 筛选模型
	QAssayResultDetail*				m_showResultDetail;
    boost::optional<tf::SampleInfo> m_sampleInfo;
	QWidget*						m_cornerItem;
	QHistoryFilterSample*			m_filterDialog;				///< 筛选对话框
    QHistoryItemResultDetailDlg*    m_sampleDetailDialog;       ///< 样本结果显示对话框
    QReCheckDialog*                 m_recheckDialog;            ///< 样本复查对话框
    QPushSampleToRemote*            m_transferDialog;           ///< 手工传输对话框
	CustomButton*					m_status;
	QObject *						m_lastFilterObj;			///< 上一次快捷筛选按钮指针
    PageSet                         m_workSet;                  ///< 工作页面设置
	QDialogAi*						m_dialogAi;
	QString							m_sCheckBtnText;			///< 审核
	QString							m_sUnCheckBtnText;			///< 取消审核
	int								m_preventRow;				///< 上次选中的行（-1代表未选中）
    int                             m_preventRowAssaylist;      ///< 上次选中的行（-1代表未选中）项目列表模式
    int                             m_currentHighLightRow;      ///< 缓存当前选中行
	bool                            m_controlModify;            ///< control被选中
	bool                            m_shiftKey;					///< shiftKey被选中
	bool							m_click;					///< 是否点击了鼠标

	QCheckBox*						m_selectCheckBtn;			///< 选取全部的按钮(样本)
    QPushButton*                    m_statusShow;               ///< 状态展示按钮(样本)
    QCheckBox*						m_selectAssayCheckBtn;		///< 选取全部的按钮(项目)
    QPushButton*                    m_assayButton;              ///< 项目选择状态显示按钮
    std::map<QHistorySampleAssayModel::VIEWMOUDLE, int> m_rowSelectedBeforeHide;    ///< 切换页面之前的选中行

    // 打印对象，移动到成员对象，是因为该对象中的打印使用了异步处理，
    // 局部对象在没有被其他引用时std::shared_ptr会将其析构，而线程中可能使用到未定义的回调变量
    std::shared_ptr<PrintSetDlg> m_pPrintSetDlg;

    // 全局样本Model实例
    QHistorySampleAssayModel &gHisSampleAssayModel = QHistorySampleAssayModel::Instance();
    std::shared_ptr<CommonInformationManager> m_pCommAssayMgr;
};

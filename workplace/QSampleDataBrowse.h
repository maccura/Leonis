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
/// @file     QSampleDataBrowse.h
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

#include <tuple>
#include <QMutex>
#include <QSortFilterProxyModel>
#include "boost/optional.hpp"
#include <boost/any.hpp>

#include "src/public/ConfigDefine.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/gen-cpp/defs_types.h"

#include "PrintExportDefine.h"
#include "QWorkShellPage.h"
#include "QFilterSample.h"

// 前置声明
class QPushSampleToRemote;
class QStandardItemModel;
class QAssayResultDetail;
class QPatientDialog;
class QReCheckDialog;
class ItemResultDetailDlg;
class QReCalcDialog;
class DetectionSetting;
class QSampleAssayModel;
class SortHeaderView;
class QCheckBox;
class QTableView;
class QRadioButton;
class QDialogAi;
class PrintSetDlg;

namespace Ui {
    class QSampleDataBrowse;
};


class QSampleFilterDataModule : public QSortFilterProxyModel
{
public:
    QSampleFilterDataModule(QObject* parent = nullptr);
    ~QSampleFilterDataModule();

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    ///
    /// @brief 设置过滤条件
    ///
    /// @param[in]  sampQryCond  过滤条件
    ///
    /// @return true表示设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月22日，新建函数
    ///
    inline void Setcond(FilterConDition& cond) { m_filterCondNew = cond; };
    inline void SetEnable(bool enable) { m_enableFilter = enable; };
	bool GetFilterEnable() { return m_enableFilter; };
    inline void Reset() { m_filterCondNew.Reset(); };
    bool IsRowCountChanged();
    inline void SetCountChangeFlag(bool flag) { m_countChanged = flag; };

    ///
    /// @brief 通过位置获取样本数据
    ///
    /// @param[in]  index  位置信息
    ///
    /// @return 样本数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    boost::optional<tf::SampleInfo> GetSampleByIndex(const QModelIndex &index);

    ///
    /// @brief 根据位置获取项目数据
    ///
    /// @param[in]  index  位置信息
    ///
    /// @return 项目数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    boost::optional<tf::TestItem> GetTestItemByIndex(const QModelIndex &index);
    ///
    ///  @brief 排序规则
    ///
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    ///
    ///  @brief 设置显示设置界面的配置
    ///
    ///
    ///  @param[in]   sampleShows  显示配置
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
    ///
    void SetSampleShows(const std::vector<std::pair<bool, int>> &sampleShows);

    ///
    ///  @brief 检查该类型的样本是否需要显示
    ///
    ///
    ///  @param[in]   spSampleInfo  样本信息指针
    ///
    ///  @return	true 显示 false 隐藏
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
    ///
    bool CheckIsSetToShow(const std::shared_ptr<tf::SampleInfo> spSampleInfo);

    ///
    /// @brief 全选
    ///
    /// @param[in]  mode  模式
    ///
    /// @return true:选中成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年9月14日，新建函数
    ///
    bool SetSelectedAll(int mode);
	
	///
	/// @brief  设置排序方式
	///
	/// @param[in]  order  排序方式
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月7日，新建函数
	///
	void SetSortOrder(const int order);
private:

    ///
    ///  @brief 检查字符串是否是纯数字
    ///
    ///
    ///  @param[in]    data  数据
    ///  @param[out]   value  是数字就输出转换后的数字
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
    ///
    bool IsNumber(const QVariant &data, double &value) const;

private:
    mutable FilterConDition						m_filterCondNew;
    bool										m_enableFilter;             // 是否开启过滤筛选
    std::map<::tf::SampleType::type, int>		m_samplePriotityMap;		// 样本类型排序优先级映射表
    std::map<::tf::SampleType::type, bool>		m_sampleFilterMap;		    // 样本类型筛选映射表
    mutable QMutex								m_priotityMapLock;			// 排序优先级映射表读写锁
    mutable QMutex								m_filterMapLock;			// 筛选映射表读写锁
    int                                         m_filterSamples;
    int											m_preRowCount;				///< 行数
    bool										m_countChanged;				///< 项目行数是否变化
	int                                         m_iOrder;					// 排序方式
};


class QSampleDataBrowse : public QWorkShellPage
{
    Q_OBJECT

public:
	enum DATATYPE
	{
		INTTYPE,
		DOUBLETYPE
	};

    QSampleDataBrowse(QWidget *parent = Q_NULLPTR);
    ~QSampleDataBrowse();

    ///
    /// @brief 刷新右上坐标控件
    ///
    /// @param[in]  index  页面序号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月25日，新建函数
    ///
    void RefreshCornerWidgets(int index)override;
    virtual void showEvent(QShowEvent *event) override;
protected:
    ///
    /// @brief 初始化页面
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月25日，新建函数
    ///
    void Init();

    ///
    /// @brief 重置项目详情列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月25日，新建函数
    ///
    virtual void ResetAssayModel();

    ///
    /// @brief 重置数据浏览样本筛选数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月31日，新建函数
    ///
    void ResetSampleSelectModel();

    ///
    /// @brief 检查全选checkBox的是状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年9月14日，新建函数
    ///
    void CheckSelectAllButtonStatus();

    ///
    /// @brief 设置样本浏览模型
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月31日，新建函数
    ///
    void SetSampleBrowseModel();
    void SetSampleColumnWidth();
    void SetAssayColumnWidth();

    ///
    /// @brief 根据数据库主键获取项目结果信息
    ///
    /// @param[in]  id  数据库主键
    ///
    /// @return 项目结果信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月16日，新建函数
    ///
    std::shared_ptr<ch::tf::AssayTestResult> GetAssayResultByid(int64_t id);

	///
	/// @brief 根据数据库主键获取项目结果信息(计算项目)
	///
	/// @param[in]  id  数据库主键
	///
	/// @return 项目结果信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月21日，新建函数
	///
	std::shared_ptr<tf::CalcAssayTestResult> GetCalcAssayResultByid(int64_t id);

    ///
    /// @brief
    ///     根据免疫测试项目结果ID获取测试项目结果信息
    ///
    /// @param[in]  id  测试项目结果ID
    ///
    /// @return 测试项目结果信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月19日，新建函数
    ///
    std::shared_ptr<im::tf::AssayTestResult> ImGetAssayResultByid(int64_t id);

    ///
    /// @brief 为某一行设置checkbox
    ///
    /// @param[in]  table  表格的view
    /// @param[in]  row    具体第几行
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月26日，新建函数
    ///
    virtual void SetSelectStatusBoxForRow(bool enable, QTableView *table, int row, int& column, bool isSelect, int assayCode);

    ///
    /// @brief 当前行是否选中
    ///
    /// @param[in]  current     当前行的Index
    /// @param[in]  isSample    是否按样本展示(true是按样本展示，否在是按项目展示)
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月17日，新建函数
    ///
    void DealCurrentIndex(const QModelIndex& current, bool isSample = true);

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
	bool CheckIndexRange(int row);

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
    /// @brief
    ///     根据结果关键信息获取项目结果(可考虑使用通用项目结果类型，使UI与设备类型隔离)
    ///
    /// @param[in]  stuResultKey  结果关键信息
    /// @param[out] pVarRlt       项目测试结果信息
    ///
    /// @return 项目结果
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月19日，新建函数
    ///
    QString GetItemResult(const tf::TestResultKeyInfo& stuResultKey, QVariant* pVarRlt = Q_NULLPTR);

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
    bool ShowSampleDetail(ItemResultDetailDlg* dialog);

    ///
    /// @brief 获取当前选中的项目的索引
    ///
    ///
    /// @return 返回索引和当前的view
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月28日，新建函数
    ///
    boost::optional<std::tuple<QModelIndex, QTableView*, QSampleFilterDataModule*>> GetCurrentIndex();

    ///
    /// @brief 更新按钮状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月5日，新建函数
    ///
    void UpdateButtonStatus();

    ///
    /// @brief 获取当前样本
    ///
    ///
    /// @return 当前样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月28日，新建函数
    ///
    boost::optional<tf::SampleInfo> GetCurrentSample();

    ///
    /// @brief 页面允许功能
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月29日，新建函数
    ///
    virtual void PageAllowFun();

    ///
    /// @brief 设置当前位置的信息
    ///
    /// @param[in]  model  model
    /// @param[in]  row    行
    /// @param[in]  column 列
    /// @param[in]  value  值
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月13日，新建函数
    ///
    bool SetModelData(QStandardItemModel* model, int row, int column, QString value);

	///
	/// @brief 获取用户选择的样本信息
	///
	/// @param[in]  samples  返回的样本信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年9月22日，新建函数
	///
	void GetSelectedSamples(std::vector<tf::SampleInfo>& samples);

	///
	/// @brief 根据病人key获取病人详细信息
	///
	/// @param[in]  db  数据库记录ID
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年9月22日，新建函数
	///
	std::shared_ptr<tf::PatientInfo> GetPatientInfo(const int64_t db);

public slots:
	void UpdateItemShowDatas(int assayCode);
    ///
    /// @brief 获取单样本的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
    /// @param[in]   selSamples		选择的样本信息
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintData(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds, const std::vector<tf::SampleInfo>& selSamples);

    ///
    /// @brief 获取按样本打印的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
	/// @param[in]   selSamples		选择的样本信息
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintDataBySample(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds, const std::vector<tf::SampleInfo>& selSamples);

    ///
    /// @brief 获取按项目打印的打印数据
    ///
    /// @param[out]  vecSampleDatas  组装好的打印数据
    /// @param[in]   QueryCondition   查询条件，类型待定
    ///
    /// @return true 设置正确
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月24日，新建函数
    ///
    bool GetPrintItemData(ItemSampleInfoVector& vecItemDatas, EM_RESULT_MODE ResultMode);

    protected slots:
    void OnAssayChanged(const QModelIndex& index, bool isupdate);

    void OnUpdateAssay();
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void OnUpdateColumn();

    void OnClickedSampleList(const QModelIndex& current);
    void OnClickedAssayList(const QModelIndex& current);

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
    /// @brief 查看病人信息按钮被按下
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void OnPatientInfoBtn();

    ///
    /// @brief 浏览上一条或者下一条病人信息
    ///
    /// @param[in]  orient  1:下一条，-1：上一条
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void MovePatientInfo(int orient);

    ///
    /// @brief 刷新过滤样本
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月1日，新建函数
    ///
    void OnShowFilterSampleInfo();

    ///
    /// @brief 删除对应的样本
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月20日，新建函数
    ///
    void OnDelBtnClicked();

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
    /// @brief 按照样本打印
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年9月6日，新建函数
    ///
    void PrintSample();

    ///
    /// @brief 按照项目打印
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年9月6日，新建函数
    ///
    void PrintItem();

    ///
    /// @brief
    ///     数据导出按钮被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年4月6日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief 查询测试结果
    ///
    /// @param[in]  id  测试结果表的主键
    ///
    /// @return 测试结果（可能不存在）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月24日，新建函数
    ///
    std::shared_ptr<ch::tf::AssayTestResult> GetChAssayTestResult(int64_t id);
    std::shared_ptr<ise::tf::AssayTestResult> GetIseAssayTestResult(int64_t id);

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
    bool ExportSampleData(const tf::UserType::type UType, const std::set<int64_t>& sampleIds, const QString& path);
	
	bool GenTitleList(const tf::UserType::type UType, QVariantList& updateTitle, const std::set<ExportInfoEn>& configData);
	bool GetSampleExport(const std::shared_ptr<tf::SampleInfo> spSample, SampleExportInfo& exportData);
	bool GetTestItemExport(const std::shared_ptr<tf::TestItem> spTestIetm, SampleExportInfo& exportData);
	bool GetResultExport(const tf::TestResultKeyInfo& resultKey, const int assayCode, SampleExportInfo& exportData);
	bool GetResultAbs(const std::shared_ptr<ch::tf::AssayTestResult> spResultInfo, SampleExportInfo& exportData, QVariantList& dynamicTitle);
	QVariantList GetExportData(bool isXlsx, const tf::UserType::type UType, const SampleExportInfo& srcData, const std::set<ExportInfoEn>& configData);
    bool ExportSample_im(tf::UserType::type UType);
    bool ExportData_im(QString fileName, tf::UserType::type UType, std::set<int64_t>& vecSample);
	bool ConvertData(const DATATYPE dstType, const QString& input, QVariant& dst);

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
    bool ExportItem(const tf::UserType::type UType, const std::set<int64_t>& itemIds, const QString& path);
    bool ExportItemData_im(QString fileName, tf::UserType::type UType, std::set<int64_t>& vecSample);
    void MakeExportItemResult(SampleExportInfo& SampleInfo, std::shared_ptr<tf::SampleInfo> pSample, std::shared_ptr<tf::TestItem> pItem, std::shared_ptr<std::vector<boost::optional<boost::any>>> pResults);

    ///
    ///  @brief 创建表头
    ///
    ///
    ///  @param[out]   strExportTextList  数据导出列表
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
    ///
    void CreateHeader(QStringList & strExportTextList);

    void OnShowBySample();
    void OnShowByAssay();

    ///
    /// @brief 设备状态更新
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月7日，新建函数
    ///
    void OnDeviceUpdate(class tf::DeviceInfo deviceInfo);

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
    void OnSampleListCurrentRowChanged(const QModelIndex& current, const QModelIndex& pre);

    ///
    /// @brief 保存修改
    ///
    /// @param[in]  type   1:修改审核 2：修改复查
    ///
    /// @return true:表示修改成功，反之修改失败
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月28日，新建函数
    ///
    bool OnSaveModify(tf::SampleInfo sampleInfo);

    ///
    /// @brief  重新计算
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    bool OnReCalcBtnClicked();

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
    /// @brief 获取样本中项目测试次数最大值（可能样本中某个项目比其它项目测试次数更多）
    ///
    /// @param[in]  sampleId  
    ///
    /// @return 返回最多的次数
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年3月9日，新建函数
    ///
    int GetMaxTestCount(int64_t sampleId);

    ///
    /// @brief 获取当前行的信息（1：总数量，2：当前数量，3：样本id）
    ///
    ///
    /// @return (1：总数量，2：当前数量，3：样本id）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月27日，新建函数
    ///
    bool GetCurrentPatientKey(std::tuple<int, int, int64_t>& data);

    ///
    /// @brief 查询模糊匹配的行号
    ///
    /// @param[in]  row      开始当前行
    /// @param[in]  keyWord  匹配关键字
    /// @param[in]  type     类型（0,：序号，1：条码）
    /// @param[in]  showTime 第几次
    ///
    /// @return 行号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月24日，新建函数
    ///
    boost::optional<int> GetIndexSearch(bool direction, int row, QString& keyWord, int type, int showTime);

    ///
    /// @brief 窗口位置变化的消息
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月20日，新建函数
    ///
    void OnChangedRow(int value);

    ///
    /// @brief 处理样本审核状态改变消息
    ///
    /// @param[in]  vecSampleID		发生改变的样本ID
    /// @param[in]  bStatus		发生改变的样本当前状态
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年5月29日，新建函数
    ///
    void OnAuditChanged(class std::vector<__int64, class std::allocator<__int64>> vecSampleID, bool bStatus);

    ///
    /// @brief 更新工作页面的按钮状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月15日，新建函数
    ///
    void OnUpdateButtonStatus();

    ///
    /// @brief 打印完毕更新状态
    ///
    /// @param[in]  bIsPrinted: true：表示打印完成；false：表示预览，不更新状态  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月21日，新建函数
    /// @li 6889/ChenWei，2023年9月8日，添加参数 bIsPrinted
    ///
    void OnPrintFinsihed(bool bIsPrinted = true);

    ///
    /// @brief 查找指定关键值的样本信息
    ///
    /// @param[in]  direction  查找方向，true:向下 false:向上
	/// @param[in]  isFirst    true:只查找一次
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年9月21日，添加注释
    ///
    void OnSeachSample(bool direction, bool isFirst = false);

    ///
    /// @brief 处理复查结果更新消息（历史数据）
    ///  
    /// @param[in]  ri  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年8月14日，新建函数
    ///
    void OnRetestResultChanged(std::string deviceSN, int64_t sampleID, ::tf::HistoryBaseDataUserUseRetestUpdate ri);

private:
    ///
    /// @bref
    ///		处理全选对话框
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    void ProcOnSelectAll(QCheckBox* pCheck, QTableView* tbView, int sFlag);

    ///
    /// @bref
    ///		处理排序的响应函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    void ProcOnSortBy(QTableView* tbView, SortHeaderView* pSortHeader, int logicIndex, int order);

    ///
    /// @bref
    ///		对样本条码下拉框赋值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    void AssignSampleComb(QComboBox* pComb);

    ///
    /// @bref
    ///		对病人信息进行上一条或下一条的更新
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    bool UpdatePatientInfoStep();

    ///
    /// @bref
    ///		从界面获取是否审核
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月21日，新建函数
    ///
    bool GetIsExamin();

    ///
    /// @brief 当前位置前后是否存在病人样本
    ///
    /// @param[in]  step  表示当前位置查询的步进数（负数向上，正数向下）
    ///
    /// @return 返回存在的modelIndex（步数为0时，返回自身）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月26日，新建函数
    ///
    QModelIndex IsExistPatientSampleByDirection(int step);
protected:
    Ui::QSampleDataBrowse*			ui;
    QStandardItemModel*				m_assayShowModel;			///< 项目状态列表的模式
    QStandardItemModel*				m_selectSampleModel;		///< 筛选模型
    QAssayResultDetail*				m_showResultDetail;
    boost::optional<tf::SampleInfo> m_sampleInfo;
    QWidget*						m_cornerItem;
    QFilterSample*					m_filterDialog;				///< 筛选对话框
    ItemResultDetailDlg*            m_sampleDetailDialog;       ///< 样本结果显示对话框
    QPatientDialog*                 m_paTientDialog;			///< 病人信息对话框
    QReCheckDialog*                 m_recheckDialog;            ///< 样本复查对话框
    QSampleFilterDataModule*		m_filterModule;				///< 过滤器模式
    QPushSampleToRemote*            m_transferDialog;           ///< 手工传输对话框
    QReCalcDialog*                  m_reCalcDialog;             ///< 重新计算对话框
	QDialogAi*						m_dialogAi;					///< AI识别的对话框
    std::vector<QPushButton*>       m_fastButton;               ///< 快捷按钮
    int                             m_searchTime;               ///< 查询次数
	tf::TestMode::type              m_sampleTestMode;           ///< 模式，参见 tf::TestMode::type
    bool                            m_lastDirection;            ///< 上次的查询方向
    bool                            m_controlModify;            ///< control被选中
    bool                            m_shiftKey;					///< shiftKey被选中
    int								m_preventRow;				///< 上次选中的行（-1代表未选中）
    bool                            m_checkBoxed;               ///< checkbox被选中
    bool                            m_pageChange;               ///< 页面是否更改
	bool							m_isAdjustWidth;			///< 是否调整了宽度
	bool							m_needUpdateDetail;			///< 是否需要更新结果详情
    QPushButton*                    m_assayButton;              ///< 项目选择状态显示按钮
    PageSet                         m_workSet;                  ///< 工作页面设置
    QString							m_strFilter;				///< 筛选条件字符串
    QCheckBox*						m_selectCheckBtn;			///< 选取全部的按钮(样本)
    QPushButton*                    m_statusShow;               ///< 状态展示按钮(样本)
    QCheckBox*						m_selectAssayCheckBtn;		///< 选取全部的按钮(项目)
	CommParamSet					m_cps;						///< 通信设置参数
	QRadioButton*					m_showBySampleRadioBtn;		///< 按样本显示
	QRadioButton*					m_showByAssayRadioBtn;		///< 按项目显示
	std::map<ExportInfoEn, DATATYPE>m_exportDataMap;			///< 需要导出特殊类型的列
    std::shared_ptr<PrintSetDlg>    m_pPrintSetDlg;             ///< 打印类，作为成员是为了避免被析构后其中的线程访问未定义变量
};

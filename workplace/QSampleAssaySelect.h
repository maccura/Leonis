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

#include <QSortFilterProxyModel>
#include "QWorkShellPage.h"
#include "src/public/ConfigSerialize.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <boost/optional.hpp>
#include <boost/multiprecision/cpp_int.hpp>

class QFrame;
class QSampleFilter;
class QDialogBatchRegister;
class QPatientDialog;
class QReCheckDialog;
class QStandardItemModel;
class QPushButton;

namespace Ui {
    class QSampleAssaySelect;
};

using namespace boost::multiprecision;


class QSampleAssaySelect : public QWorkShellPage
{
    Q_OBJECT

public:
    QSampleAssaySelect(QWidget *parent = Q_NULLPTR);
    ~QSampleAssaySelect();
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
protected:
    ///
    /// @brief 初始化样本列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月9日，新建函数
    ///
    void InitalSampleData();

    ///
    /// @brief 初始化样本属性选项
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月9日，新建函数
    ///
    void InitSampleAttribute();

    ///
    /// @brief 显示样本详细信息
    ///
    /// @param[in]  sampleInfo  样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void ShowSampleDetail(tf::SampleInfo& sampleInfo);

    ///
    /// @brief 设置空样本默认页面
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void SetDefaultSampleDetail(bool isDefault);

	///
	/// @brief 根据情况获取当前位置号
	///  
	/// @param[in]  samplePostion  位置信息
	/// @param[in]  isEmerge	   是否急诊
	/// @param[in]  isRetest	   是否复查
	///
	/// @return 修改后位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月18日，新建函数
	///
	std::pair<int, int> GetIncreasePostion(std::pair<int, int> samplePostion, bool isEmerge, bool isRetest);

    ///
    /// @brief 获取当前样本的信息
    ///
    /// @param[out]  sampleInfo  样本信息
    /// @param[in]   isWarning   需要告警否
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月13日，新建函数
    ///
    bool GetCurrentSampleInfo(tf::SampleInfo& sampleInfo, bool isWarning = true);

    ///
    /// @brief 删除选中的样本
    ///
    ///
    /// @return true删除成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    bool DelSelSampInfo();

    ///
    /// @brief 刷新右上坐标控件
    ///
    /// @param[in]  index  页面序号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月25日，新建函数
    ///
    void RefreshCornerWidgets(int index)override;

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
    /// @brief 获取下一个合适的空序号（序号模式）
    ///
    /// @param[in]  start  序号起点（终点是最大序号，在其中找空置的序号使用）
    ///
    /// @return 空置可用序号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月3日，新建函数
    ///
    cpp_int GetRightSeqNo(cpp_int start);

    ///
    /// @brief 鼠标点击事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    void mousePressEvent(QMouseEvent* event) override;

    ///
    /// @brief 安装（递归）
    ///
    /// @param[in]  object  对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    void InstallEventFilterRecursively(QObject *object);

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

    private slots:
    ///
    /// @brief 新增按钮槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnClickNewBtn();

    ///
    /// @brief 上一条记录被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void OnPreviousBtnClicked();

    ///
    /// @brief 下一条记录被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void OnNextBtnClicked();

    ///
    /// @brief 删除被选中的样本
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief 样本被选中的处理函数
    ///
    /// @param[in]  index  位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void OnSampleClicked(const QModelIndex& index);

    ///
    /// @brief 样本号回车键槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnSampleNoEditReturnPress();

    ///
    /// @brief 样本编码回车键槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnSampleBarEditReturnPress();

    ///
    /// @brief 保存样本信息的槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnSaveSampleInfo();

    ///
    /// @brief 序号模式下保存样本
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月3日，新建函数
    ///
    void SaveSampleInSeqMode();

    ///
    /// @brief 样本模式下保存样本
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月24日，新建函数
    ///
    void SaveSampleInBarcodeMode();

    ///
    /// @brief 批量输入按钮被按下
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnBatchInputBtnClicked();

    ///
    /// @brief 处理批量输入
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    void OnDealBatchInputSample();

    ///
    /// @brief 设置稀释方式
    ///
    /// @param[in]  index  0，标准，1减量，2增量，3自定义
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月14日，新建函数
    ///
    void OnDilution(int index);

    ///
    /// @brief 设置手动稀释倍数
    ///
    /// @param[in]  text  手动稀释倍数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月14日，新建函数
    ///
    void OnDilutionText(QString text);

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
    /// @brief 样本无条码的槽函数
    ///
    /// @param[in]  checked  是否被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月20日，新建函数
    ///
    void OnSampleNoBarCodeBtn(bool checked);

    void OnRefreshAssayCard();

    ///
    /// @brief 改变稀释倍数类型
    ///
    /// @param[in]  type  0：代表生化-1代表免疫
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月22日，新建函数
    ///
    void OnDilutionChanged(int type);

    ///
    /// @brief 复查按钮
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月23日，新建函数
    ///
    void OnRecheck();

	void OnAssaySelected(int postion, int assayCode);

    ///
    /// @brief 改变检测模式
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月24日，新建函数
    ///
    void OnChangeMode(DetectionSetting detectionSetting);

    void OnUpdateButtonStatus();

private:
	///
	/// @brief 是否需要响应当前项目信息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月22日，新建函数
	///
	bool IsUpdateData();

    ///
    /// @brief 根据位置信息获取样本
    ///
    /// @param[in]  rack    架号
    /// @param[in]  postion 位置号 
    /// @param[in]  testMode 样本所属测试模式 
    ///
    /// @return 样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月23日，新建函数
    ///
    boost::optional<tf::SampleInfo> GetSampleByPostion(const std::string& rack, int postion, tf::TestMode::type testMode);

    ///
    /// @brief 获取样本的加样量
    ///
    /// @param[in]  sampleInfo  样本信息
    ///
    /// @return 加样量大小
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月15日，新建函数
    ///
    int GetSampleVolumn(tf::SampleInfo& sampleInfo);

    ///
    /// @brief 根据稀释倍数获取加样量
    ///
    /// @param[in]  ratio  稀释倍数
    ///
    /// @return 加样量（若找不到，则返回0）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月16日，新建函数
    ///
    int GetSampleRatioVolumn(int ratio);

    ///
    /// @brief 获取项目的加样量
    ///
    /// @param[in]  testTtem  项目信息
    /// @param[in]  sampleType样本类型
    ///
    /// @return 项目加样量，注意：单位为 0.1ul
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月15日，新建函数
    ///
    int GetTestItemVolumn(tf::TestItem& testItem, int sampleType);

	///
	/// @brief 当前位置是否在范围内
	///
	/// @param[in]  isRerun		是否复查
	/// @param[in]  mode		模式
	/// @param[in]  isEmerge	是否急诊
	/// @param[in]  rack		架号
	/// @param[in]  postion		位置号
	///
	/// @return true:范围内
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月11日，新建函数
	///
	bool IsPostionProperty(bool isRerun, int mode, bool isEmerge, std::string rack, int postion);

private:
    Ui::QSampleAssaySelect*			ui;
    QStandardItemModel*				m_SampleViewModel;			///< 样本列表的model
	tf::TestMode::type				m_sampleTestMode;			///< 样本测试模式 
    QDialogBatchRegister*			m_batchDialog;				///< 批量输入对话框
    QPatientDialog*                 m_paTientDialog;			///< 病人信息对话框
    QSampleFilter*					m_filterModule;				///< 过滤器模式
    QReCheckDialog*                 m_recheckDialog;            ///< 复查对话框
    std::vector<QFrame*>            m_sampleListFrame;          ///< 样本列表需要的框
    std::vector<QFrame*>            m_sampleAppFrame;           ///< 样本申请需要的框
    QString                         m_sampleSeq;                ///< 样本序号
    bool                            m_isSeqChanged;             ///< 样本序号是否编辑过
	bool							m_assaySelected;			///< 项目是否被选中
	bool                            m_controlModify;            ///< control被选中
	bool                            m_shiftKey;					///< shiftKey被选中
    PageSet                         m_workSet;                  ///< 工作页面设置
    int                             m_currentSampleType;        ///< 当前样本类型
    QPushButton*					m_statusShow;				///< 状态按钮
};

class QSampleFilter : public QSortFilterProxyModel
{
public:
    QSampleFilter(QObject* parent = nullptr);
    ~QSampleFilter();
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    boost::optional<tf::SampleInfo> GetSampleByIndex(const QModelIndex &index);

    ///
    ///  @brief 排序规则
    ///
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
private:

    ///
    ///  @brief 检查字符串是否是纯数字
    ///
    bool IsNumber(const QVariant &data, double &value) const;
};

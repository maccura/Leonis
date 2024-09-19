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
#include <set>
#include <QWidget>
#include "src/thrift/gen-cpp/defs_types.h"

class QQcAssayEditModel;
class QTableView;
namespace Ui {
    class QQcRegSample;
};


// 用于记录一个质控品扫描的所有二维码
struct UiBarCodeInfo
{
    ///
    /// @bref
    ///		来自被修改的原质控品信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年4月12日，新建函数
    ///
    UiBarCodeInfo(const std::shared_ptr<tf::QcDoc>& curQcDoc);

    // 默认构造
    UiBarCodeInfo() {};

    ///
    /// @bref
    ///		添加一个二维码的信息
    ///
    /// @param[in] qcItem 一个条码的信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void AddItem(const tf::QcBarCodeInfo& qcItem);
  
    ///
    /// @bref
    ///		获取项目成分信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    std::vector<tf::QcComposition> GetCompositions();

    ///
    /// @bref
    ///		是否已经把所有的二维码扫描全了
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    bool FullScaned();

    std::string m_lot;
    std::string m_sn;
    int m_level;

    int m_totalPage;

    std::vector<int> m_notExistAssayCodes;  // 在二维码中存在，但在设置到界面时 没在界面中找到

    std::map<int, std::shared_ptr<tf::QcBarCodeInfo>> m_barCodes; // <currentPage, barCodeInfo>
};


class QQcRegSample : public QWidget
{
    Q_OBJECT

public:
    enum class	DIALOGMODULE
    {
        MANNUAL = 0,	///< 手动登记
        SCAN,           ///< 扫描登记
        MODIFY			///< 修改
    };

    QQcRegSample(QWidget *parent = Q_NULLPTR);
    ~QQcRegSample();

    ///
    /// @brief 设置质控数据
    ///
    /// @param[in]  data  质控数据
    ///
    /// @return true 成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月18日，新建函数
    ///
    bool StartModify(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc);

    ///
    /// @brief 手动登记质控文档
    ///
    /// @param[in]  index  显示序号
    /// @param[in]  curQcDoc  被修改行的原有质控品信息
    ///
    /// @return true 成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月22日，新建函数
    ///
    bool StartManual(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc);

    ///
    /// @brief 进入扫描模式
    ///
    /// @param[in]  index  显示序号
    /// @param[in]  curQcDoc  所在行的原有质控品信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月22日，新建函数
    ///
    bool StartScan(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc);

signals:
    ///
    /// @brief 更新数据的消息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月22日，新建函数
    ///
    void Update();

    ///
    /// @brief
    ///     关闭信号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void SigClosed();

    protected slots:
    ///
    /// @brief 保存质控品文档
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月22日，新建函数
    ///
    void OnSaveQcDoc();

    ///
    /// @bref
    ///		取消的按钮事件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void OnCancalButton();

    ///
    /// @bref
    ///		条码框出现变化的事件
    ///
    /// @param[in] barCode 扫描框中的字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void OnBarCodeEditChanged(const QString& barCode);

    ///
    /// @bref
    ///		二维码扫描的定时器事件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void OnBarCodeTimerEvent();

    ///
    /// @bref
    ///		成分表格退格键变化清空处理
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void OnTextChanged(const QString& ctrText);

    ///
    /// @bref
    ///		二维码扫描完成
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void OnBarFinished();

    ///
    /// @brief
    ///     质控文本被编辑
    ///
    /// @param[in]  strQcDoc  编辑后的质控文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月13日，新建函数
    ///
    void OnQcNameTextEditted(const QString& strQcDoc);

private:
    ///
    /// @bref
    ///		初始化成分表的模型
    ///
    /// @param[in] qcCompositions 成分列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void InitCompositionTableMode(const std::vector<tf::QcComposition>& qcCompositions);

    ///
    /// @bref
    ///		初始化扫描模式
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void InitScanEnvr();

    ///
    /// @bref
    ///		初始化表格
    ///
    /// @param[in] tbView 表格控件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void InitTableView(QTableView* tbView, DIALOGMODULE dlgType);

    ///
    /// @bref
    ///		获取生化和免疫项目ID集合
    ///
    /// @param[in] chimCode 生化的项目编号
    /// @param[in] immyCode 免疫的项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void TakeChimAndImmyAssayCode(std::vector<int32_t>& chimCode, std::vector<int32_t>& immyCode);

    ///
    /// @bref
    ///		清空质控基础信息界面
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void ClearQcbaseInfoUi();

    ///
    /// @bref
    ///		解析条码框中的条码，并填充对应的UI
    ///
    /// @param[in] barCode 条码
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void PaserScanBarCodeToUi(const QString& barCode);

    ///
    /// @bref
    ///		解析一个二维码到内存（m_uiBarInfo）中
    ///
    /// @param[in] strOneBar 二维码字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月9日，新建函数
    ///
    void PaserScanBarCodeOneBar(const QString& strOneBar);

    ///
    /// @bref
    ///		解析条码框中的条码
    ///
    /// @param[in] barCode 条码
    /// @param[out] qcDoc 解析结果
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    bool ParseScanBarCode(const QString& barCode, ::tf::QcBarCodeInfo &qcDoc);

    ///
    /// @bref
    ///		在扫描模式下，置灰Edit框
    ///
    /// @param[in] isSetDisable 是否是能
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void DisableNScanEdit(bool isSetDisable);

    ///
    /// @bref
    ///		进行新质控品的添加操作, 返回新增数据的主键，失败返回-1
    ///
    /// @param[in] newDoc 需要添加的质控品
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月25日，新建函数
    ///
    int64_t ProcessQcDocAdd(const tf::QcDoc& newDoc);

    ///
    /// @bref
    ///		显示与隐藏项目页签
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void ShowCurrentDeviceTab();

    ///
    /// @bref
    ///		更新成本表格数据
    ///
    /// @param[in] tb 表格控件
    /// @param[in] cModel 数据模型
    /// @param[in] assayCodes 涉及的项目
    /// @param[in] qcCompositions 成分
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    void UpdateComposition(QTableView* tb, std::shared_ptr<QQcAssayEditModel>& cModel, \
        const std::vector<int32_t>& assayCodes, const std::vector<tf::QcComposition>& qcCompositions);

    ///
    /// @bref
    ///		获取当前注册类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月23日，新建函数
    ///
    tf::DocRegistType::type GetCurrentRegistType();

    ///
    /// @bref
    ///		处理因设备不同产生的差异显示（隐藏成分页签、质控品类型）
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月11日，新建函数
    ///
    void InitByDiffDivece();

    ///
    /// @bref
    ///		检测是否在数据库中存在质控品编号批号水平重复的质控品
    ///
    /// @param[in] saveDoc 编辑后的质控品
    /// @return true 无重复可继续流程，false重复不可修改
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月17日，新建函数
    ///
    bool CheckRepeatQc(const ::tf::QcDoc& saveDoc);

    ///
    /// @bref
    ///		把项目编号vector组成成每十个编号一行的字符串
    ///
    /// @param[in] vecAssayCode 项目编号列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月12日，新建函数
    ///
    std::string CombineAssayCodes(const std::vector<int>& vecAssayCode);

    ///
    /// @bref
    ///		判断在运行或加样停时，成分信息不能被修改
    ///
    /// @param[in] newQcDoc 界面的成分信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月12日，新建函数
    ///
    bool CheckCompositionCondition(const ::tf::QcDoc& newQcDoc);

    ///
    /// @bref
    ///		从界面获取最新的质控品登记信息
    ///
    /// @param[out] saveQcDoc 质控品登记信息
    /// @param[out] notComplate 没有填写完整的成分信息(项目名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void TakeQcDocInfoFromUI(::tf::QcDoc& saveQcDoc, std::vector<std::string>& notComplate);

    ///
    /// @bref
    ///		通知消息组装质控品变化的通知结构
    ///
    /// @param[out] vecQcDocUpdate 通知信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void PostUpdateDocInfo(int64_t keyValue);

    ///
    /// @bref
    ///		检查二维码信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月16日，新建函数
    ///
    bool CheckBarcodeInfoValid();

	///
	/// @bref 保存前ISE特殊检测
	///		  ISE样本类型只支持“血清血浆”或“尿液”
	///
	/// @par History:
	/// @li 8580/GongZhiQiang, 2023年11月10日，新建函数
	///
	bool IseSpecialCheck(const ::tf::QcDoc& saveQcDoc);

    ///
    /// @bref
    ///		调整在扫描模式下的各个空间的焦点获取策略；以避免barCode失去焦点获取不到扫码枪的输入
    ///
    /// @param[in] focusPolicy 焦点策略
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月28日，新建函数
    ///
    void ChangeScanEffectedFocs(Qt::FocusPolicy focusPolicy);

    ///
    /// @brief
    ///     获取文本的字符数
    ///
    /// @param[in]  strText  文本
    ///
    /// @return 文本字符数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月31日，新建函数
    ///
    int GetCharCnt(const QString& strText);

private:
    Ui::QQcRegSample*	ui;
    int                                 m_rowIndex;             // 质控品在界面的行索引
    int                                 m_fixedBarcodeLength;   // 质控品二维码长度
    int                                 m_fixLengthSn;          // 质控编号标准长度
    int                                 m_fixLengthLot;         // 质控批号标准长度

    std::shared_ptr<tf::QcDoc>          m_qcDoc;
    DIALOGMODULE		m_module;

    std::shared_ptr<QTimer>             m_barCodeTimer;         // 用于侦测BarCodeEdit变化
    std::shared_ptr<UiBarCodeInfo>      m_uiBarInfo;            // 缓存扫描到的当前质控品条码页
    QString m_preBarcodeText;                                   // 防重复解析的判断

    std::shared_ptr<QQcAssayEditModel>  m_CompositionModeChim;
    std::shared_ptr<QQcAssayEditModel>  m_CompositionModeImmy;
};

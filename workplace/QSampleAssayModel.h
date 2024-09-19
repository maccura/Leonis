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
/// @file     QSampleAssayModel.h
/// @brief    样本模块module
///
/// @author   5774/WuHongTao
/// @date     2022年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <mutex>
#include <future>
#include <thread>
#include <QTimer>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <boost/optional.hpp>
#include <functional>
#include <QAbstractTableModel>
#include "boost/optional.hpp"
#include <boost/any.hpp>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigSerialize.h"
#include "WorkpageCommon.h"

class QSampleAssayModel;

using AssayResult = boost::optional<boost::any>;
using AssayVec = std::vector<AssayResult>;
using AssayResults = std::shared_ptr<AssayVec>;
using DataPara = std::pair<std::shared_ptr<tf::SampleInfo>, tf::TestItem>;

class MyCountDown : public QObject {
    Q_OBJECT
public:
    MyCountDown(QObject *parent = nullptr) : QObject(parent) {
        connect(timer, &QTimer::timeout, this, &MyCountDown::decrementCounter);
    }

    QMap<int64_t,int> GetRuntimeMap() 
    { 
        readWriteLock.lockForRead();
        auto keyMap = map; 
        readWriteLock.unlock();
        return keyMap;
    };

    // 添加函数对象参数，用于获取value
    void addElement(int64_t key, const tf::TestItem& testitem, std::function<int(const tf::TestItem& testItemInfo)> GetReactionTime) {
        // 在写入共享数据时加写锁
        readWriteLock.lockForWrite();

        // 添加元素到map
        if (!map.contains(key)) {
            map.insert(key, GetReactionTime(testitem));
        }

        // 如果map不为空并且计时器未启动，则启动计时器
        if (!timer->isActive() && !map.isEmpty()) {
            timer->start(1000);
            timer->setInterval(1000);
        }

        readWriteLock.unlock();
    }

    // 添加函数，用于获取Key对应的value
    boost::optional<int> getValue(int64_t key) {
        // 在读取共享数据时加读锁
        readWriteLock.lockForRead();

        boost::optional<int> result = boost::none;

        auto it = map.find(key);
        if (it != map.end() && it.value() > 0) {
            result = it.value();
        }

        readWriteLock.unlock();

        return result;
    }

    // 清除
    void clear()
    {
        readWriteLock.lockForWrite();
        map.clear();
        timer->stop();
        readWriteLock.unlock();
    }

public slots:
    void decrementCounter() {
        // 在写入共享数据时加写锁
        readWriteLock.lockForWrite();

        for (auto it = map.begin(); it != map.end();) {
            it.value()--;  // 计数器减1

            if (it.value() <= 0) {
                it = map.erase(it);  // 删除计数器为0的元素
            }
            else {
                ++it;
            }
        }

        if (map.isEmpty()) {
            timer->stop();
        }

        readWriteLock.unlock();

        emit counterDecremented();  // 发出消息
    }

    void removeElement(int64_t key) {
        // 在写入共享数据时加写锁
        readWriteLock.lockForWrite();

        // 从map中删除元素
        if (map.contains(key)) {
            map.remove(key);
        }

        // 如果map为空并且计时器已启动，则停止计时器
        if (map.isEmpty() && timer->isActive()) {
            timer->stop();
        }

        readWriteLock.unlock();
    }

signals:
    void counterDecremented();  // 定义信号

private:
    QMap<int64_t, int> map;
    QReadWriteLock readWriteLock;
    QTimer* timer = new QTimer(this);
};

class QDataItemInfo : public QObject
{
    Q_OBJECT

public:
    static QDataItemInfo& Instance();
    // 构造函数
    QDataItemInfo();

    ///
    /// @brief 获取当前行对应的数据（样本）
    ///
    /// @param[in]  index       位置信息
    /// @param[in]  sampleInfo  原始数据
    ///
    /// @return 返回数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月16日，新建函数
    ///
    QVariant GetSampleData(SampleColumn::SAMPLEMODE mode, const QModelIndex &index, const tf::SampleInfo& sampleInfo);

    ///
    /// @brief 获取项目信息
    ///
    /// @param[in]  index           位置信息
    /// @param[in]  testItemInfo    项目信息
    ///
    /// @return 项目信息（对应的）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月24日，新建函数
    ///
    QVariant GetOnlyAssay(const QModelIndex &index, const DataPara& dataPara);
    
    ///
    /// @bref
    ///		获取结果显示框的内容
    ///
    /// @param[in] reCheck 是否是复查结果
    /// @param[in] testItem 测试项目
    /// @param[in] data 样本
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月25日，新建函数
    ///
    static QString GetAssayResultWithCountdown(bool reCheck, const tf::TestItem& testItem, const std::shared_ptr<tf::SampleInfo>& pSample);

	///
	/// @brief 是否显示倒计时
	///
	/// @param[in]  reCheck		是否复查
	/// @param[in]  testItem	项目信息
	///
	/// @return true:显示倒计时
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月7日，新建函数
	///
	static bool IsShowCountDown(const bool reCheck, const tf::TestItem& testItem);

    ///
    /// @brief 获取当前行对应的数据
    ///
    /// @param[in]  index       位置信息
    /// @param[in]  dataPara    原始数据
    ///
    /// @return 返回数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月16日，新建函数
    ///
    QVariant GetAssayData(const QModelIndex &index, const DataPara& dataPara);

    ///
    /// @brief 获取试剂吸取类型
    ///
    /// @param[in]  itemData  数据
    ///
    /// @return 试剂吸取类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月6日，新建函数
    ///
    static QVariant GetSuckType(tf::TestItem& itemData);

	///
	/// @brief 获取当前的稀释类型
	///
	/// @param[in]  itemData  项目信息
	///
	/// @return 稀释类型
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年8月13日，新建函数
	///
	static QVariant GetDiluTionShow(tf::TestItem& itemData);

    ///
    /// @brief 根据秒数将倒计时转化为（00：00：00）
    ///
    /// @param[in]  second  秒数
    ///
    /// @return 倒计时
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月15日，新建函数
    ///
    static QString ConvertTime(int second);

    ///
    /// @brief 获取样本的测试状态
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @return 样本状态字符串（O,P,H）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月21日，新建函数
    ///
    static QString GetStatus(const tf::SampleInfo& sample);

    ///
    /// @brief 获取样本的测试状态
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @return 样本状态字符串（O,P,H）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月21日，新建函数
    ///
    static QString GetStatus(const tf::TestItem& item);

    ///
    /// @brief 获取样本号
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @return 样本号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月22日，新建函数
    ///
    static QString GetSeqNo(const tf::SampleInfo& sample);

    ///
    /// @brief 获取位置
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @return 位置信息（x-x）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月21日，新建函数
    ///
    static QString GetPos(const tf::SampleInfo& sample);

    ///
    /// @brief 获取测试结束时间
    ///
    /// @param[in]  assayAnyResult  测试结果
    ///
    /// @return 结束时间
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月24日，新建函数
    ///
    static QString GetAssayEndTime(const AssayResult& assayAnyResult);

    ///
    /// @brief 获取项目测试结果显示字符串
    ///
    /// @param[in]  assayAnyResult  项目结果
    ///
    /// @return 结果字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月3日，新建函数
    ///
    static QString GetAssayResult(const AssayResult& assayAnyResult);

    ///
    /// @brief 获取项目测试结果显示字符串
    ///
    /// @param[in]  reCheck  是否复查（true:代表复查,false:代表初次检查）
    /// @param[in]  testItem 测试项目
    /// @param[in]  bPrint   是否为打印，打印时不带修改标志
    ///
    /// @return 结果字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月19日，新建函数
    /// @li 6889/ChenWei，2024年4月7日，添加 bPrint 标志
    ///
    static QString GetAssayResult(bool reCheck, const tf::TestItem& testItem, std::shared_ptr<tf::SampleInfo> data, bool bPrint = false);

	///
	/// @brief 根据保留小数位数转换字符串
	///
	/// @param[in]  rawData  原始字符串
	/// @param[in]  key		 分隔符
	/// @param[in]  left	 保留小数位数
	///
	/// @return 转换后的字符串，当转化失败返回原始字符串
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月7日，新建函数
	///
	static QString GetValueFromLeft(const QString& rawData, const QString& key, int left);

    ///
    /// @brief 为给定的结果设置偏离标记
    ///
    /// @param[in]  dValue      给定的结果值（原始值或修改后的值）
    /// @param[in]  reCheck     是否复查（true:代表复查,false:代表初次检查）
    /// @param[in]  testItem    测试项目
    /// @param[in]  sampleType  样本类型
    ///
    /// @return 结果字符串
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年10月31日，新建函数
    ///
    static QString SetAssayResultFlag(double dValue, bool reCheck, bool isOriginal, const tf::TestItem& testItem, tf::SampleType::type sampleType, bool bPrint = false);

	///
	/// @brief 根据结果和设置显示结果范围
	///
	/// @param[in]  reCheck    true:复查，false:初测
	/// @param[in]  testItem   项目信息
	/// @param[in]  resultProm 结果提示
	///
	/// @return 超出上限显示：↑和H，超出下限显示↓和L
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月12日，新建函数
	///
	static QString ResultRangeFlag(bool reCheck, const tf::TestItem& testItem, const ResultPrompt& resultProm);

    ///
    /// @brief 获取结果状态
    ///
    /// @param[in]  assayResult  项目结果
    ///
    /// @return 结果状态字符串
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月4日，新建函数
    ///
    static QString GetAssayResultStatus(const AssayResult& assayAnyResult, const std::string& conc);

    ///
    /// @brief 获取项目当前单位的字符串
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 字符串单位
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月3日，新建函数
    ///
    static std::string GetAssayResultUnitName(int assayCode);

    ///
    /// @brief 获取设备名称
    ///
    /// @param[in]  assayAnyResult  测试结果
    ///
    /// @return 设备名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月28日，新建函数
    ///
    static QString GetDeivceName(const AssayResult& assayAnyResult);
	void SetWorkSet(PageSet& workSet) { m_workSet = workSet; };

    ///
    /// @brief 获取参考范围字符串
    ///
    /// @param[in]  assayAnyResult  测试结果
    ///
    /// @return 设备名称
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年8月8日，新建函数
    ///
    static QString GetRefStr(const AssayResult& assayAnyResult, int sourceType = -1, std::shared_ptr<tf::PatientInfo> pPatientInfo = nullptr);

	///
	/// @brief 根据项目信息获取结果范围状态
	///
	/// @param[in]  dataItem  项目信息
	/// @param[in]  column    当前列
	/// @param[in]  isSample  是否按样本展示（true:按样本，false；按项目）
	///
	/// @return 状态
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月12日，新建函数
	///
	QVariant ResultRangeStatus(tf::TestItem& dataItem, int column, bool isSample) const;

	///
	/// @brief 根据项目信息获取危机状态
	///
	/// @param[in]  dataItem  项目信息
	/// @param[in]  column    当前列
	/// @param[in]  isSample  是否按样本展示（true:按样本，false；按项目）
	///
	/// @return 状态是否超出upper
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月12日，新建函数
	///
	boost::optional<bool> ResultDangerStatus(tf::TestItem& dataItem, int column, bool isSample) const;

	///
	/// @brief 获取血清指数的错误码
	///
	/// @param[in]  dataItem  项目信息
	///
	/// @return 错误码信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月12日，新建函数
	///
	QString GetSINDStatusInfo(const int assayCode, const tf::QualJudge::type judgeType);

	///
	/// @brief 是否AI模式
	///
	///
	/// @return true:AI模式
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月2日，新建函数
	///
	bool IsAiRecognition();

	///
	/// @bref
	///		生成界面显示的测试状态码
	///
	/// @param[in] dataItem 测试项
	/// @param[in] strResult 结果串
    /// @param[in] isReTest 是否是复查
    /// @param[in] sampType 样本类型，用于结果状态码的处理
	///
	/// @par History:
	/// @li 8276/huchunli, 2023年8月9日，新建函数
	///
	QVariant GetDisplyResCodeVariant(const tf::TestItem& dataItem, const std::string& strResult, bool isReTest, int sampType);

protected:

    ///
    /// @brief 初始化列表映射选项
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月7日，新建函数
    ///
    void Init();

	///
	/// @brief 判断初测是否完成
	///
	/// @param[in]  testItem  项目信息
	///
	/// @return true:初测完成
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月1日，新建函数
	///
	bool IsFirstTested(const tf::TestItem& testItem);

Q_SIGNALS:
    void ColumnChanged();

protected slots:
    void UpdateResultShow();
private:
	PageSet                         m_workSet;                  ///< 工作页面设置
    SampleShowSet                   m_sampleResultShowSet;      ///< 校准/质控/样本结果的显示设置
};

class RangeRowManage
{
public:
    ///
    /// @brief 更新范围
    ///
    /// @param[in]  first  当前第一行
    /// @param[in]  last   当前最后一行
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月20日，新建函数
    ///
    void SetRange(int first, int last)
    {
        std::lock_guard<std::recursive_mutex> lock(m_rangelock);
        m_updateRange.first = first;
        m_updateRange.second = last;
    }

    std::pair<int, int> GetRange() { return m_updateRange; };

    ///
    /// @brief 当前行在范围之内
    ///
    /// @param[in]  row  当前行
    ///
    /// @return true代表在范围之内
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月20日，新建函数
    ///
    bool IsRangIn(int row)
    {
        std::lock_guard<std::recursive_mutex> lock(m_rangelock);
        if (row < m_updateRange.first || m_updateRange.second < row)
        {
            return false;
        }

        return true;
    }

    RangeRowManage()
    {
        // 初始化，不更新
        m_updateRange.first = 0;
        m_updateRange.second = 0;
    }

private:
    mutable std::recursive_mutex							m_rangelock;				// 刷新范围更新锁
    std::pair<int, int>                                     m_updateRange;              // 行刷新范围
};

class DataPrivate : public QObject
{
    Q_OBJECT

public:
    // 模式状态
    enum class MOUDLESTATUS
    {
        UNKONW = 0,            /// 未初始化
        PROCESS,			   /// 正在初始化
        FINISH,				   /// 初始化完成
    };

	enum class ACTIONTYPE
	{
		ADDSAMPLE = 0,          /// 添加样本
		MODIFYSAMPLE,			/// 修改样本
		DELETESAMPLE,			/// 删除样本
	};
	Q_ENUM(ACTIONTYPE);

    static DataPrivate& Instance();
    ~DataPrivate();

	///
	/// @brief 获取当前模式下的待测项目和测试次数

	///
	/// @param[in]  curTestCode    获取当前模式下的待测项目和测试次数

	/// @param[in]  result		   待测项目和测试次数
	///
	/// @return 待测项目和测试次数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月7日，新建函数
	///
	bool GetAssayAndTestTimes(tf::TestMode::type curTestCode, std::map<int, int>& result);

	QVariant GetMannualNumber(int64_t db);

    ///
    /// @brief 根据结果的关键信息获取结果
    ///
    /// @param[in]  testKey  结果关键信息
    ///
    /// @return 测试结果
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月24日，新建函数
    ///
    AssayResult GetAssayResultByTestKey(tf::TestResultKeyInfo& testKey) const;

    ///
    /// @brief 根据测试数据库主键获取测试结果
    ///
    /// @param[in]  assayId  项目主键
    ///
    /// @return 测试结果
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月10日，新建函数
    ///
    AssayResults GetAssayResultByTestItem(const tf::TestItem& testItem);
    void SetRange(int first, int last) { m_rangVisble.SetRange(first, last); };

	///
	/// @brief 是否打印样本信息
	///
	/// @param[in]  enable  true:使能
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月5日，新建函数
	///
	void SetPrintSampleStatus(bool enable) { m_needPrintSample = enable; };

    ///
    /// @brief 通过行号获取样本数据(按样本展示使用)
    ///
    /// @param[in]  row  行号
    ///
    /// @return 样本数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月22日，新建函数
    ///
    virtual boost::optional<tf::SampleInfo> GetSampleByRow(const int row) const;

    ///
    /// @brief 根据行号获取项目信息(按项目展示使用)
    ///
    /// @param[in]  row  行号
    ///
    /// @return 项目信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月5日，新建函数
    ///
    virtual std::shared_ptr<tf::TestItem> GetTestItemByRow(const int row) const;

    ///
    /// @brief 根据样本信息获取行号
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @return 行号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月5日，新建函数
    ///
    virtual boost::optional<int> GetRowBySample(tf::SampleInfo& sample);

    ///
    /// @brief 根据序号类型获取样本信息
    ///
    /// @param[in]  seqNo		序号
    /// @param[in]  type		类型
    /// @param[in]  IsEmerge	是否急诊样板
    ///
    /// @return 样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月10日，新建函数
    ///
    virtual boost::optional<tf::SampleInfo> GetSampleBySeq(tf::TestMode::type mode, const std::string& seqNo, tf::SampleType::type type, bool IsEmerge);

    ///
    /// @brief 通过样本条码获取样本
    ///
    /// @param[in]  barCode  样本条目
    /// @param[in]  type     类型
    /// @param[in]  mode	 样本所属模式
    ///
    /// @return 样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月12日，新建函数
    ///
    virtual boost::optional<tf::SampleInfo> GetSampleByBarCode(tf::TestMode::type mode, const std::string& barCode, tf::SampleType::type type);

    ///
    /// @brief 根据数据库主键获取对应的行号
    ///
    /// @param[in]  db  数据库主键
    ///
    /// @return 正确范围行号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    virtual boost::optional<int> GetSampleByDb(int64_t db);

	///
	///  @brief  通过数据库主键获取项目行号
	///
	///
	///  @param[in]   db  数据库主键
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月31日，新建函数
	///
	virtual boost::optional<int> GetTestItemRowByDb(const int64_t db) const;

    ///
    /// @brief 根据数据库主键获取样本信息
    ///
    /// @param[in]  db  数据库主键
    ///
    /// @return 样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月30日，新建函数
    ///
    virtual std::shared_ptr<tf::SampleInfo> GetSampleInfoByDb(int64_t db)
    {
        std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
        auto iter = m_sampInfoData.find(db);
        return (iter == m_sampInfoData.end()) ? Q_NULLPTR : iter->second;
    };

    virtual std::shared_ptr<tf::TestItem> GetTestItemInfoByDb(int64_t db)
    {
        std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
        auto iter = m_testItemData.find(db);
        return (iter == m_testItemData.end()) ? Q_NULLPTR : iter->second;
    };

    virtual boost::optional<AssayResults> GetAssayResultByDb(int64_t db)
    {
        std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
        if (m_testItemData.count(db) > 0)
        {
            return boost::make_optional(m_assayResults[db]);
        }

        return boost::none;
    };

    ///
    /// @brief 根据样本模式，样本架号，样本位置查询样本
    ///
    /// @param[in]  mode  样本模式
    /// @param[in]  rack  样本架号
    /// @param[in]  pos   样本位置号
    ///
    /// @return 样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月28日，新建函数
    ///
    virtual std::shared_ptr<tf::SampleInfo> GetSampleByRackAndPsotion(tf::TestMode::type mode, std::string rack, int pos)
    {
        for (const auto& sampleData :m_sampInfoData)
        {
            auto sampleInfo = sampleData.second;
            if (sampleInfo->testMode == mode
                && sampleInfo->rack == rack
                && sampleInfo->pos == pos
				// bug0013497
				&& (sampleInfo->status == tf::SampleStatus::type::SAMPLE_STATUS_PENDING
					|| sampleInfo->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING))
            {
                return sampleInfo;
            }
        }

        return Q_NULLPTR;
    }

    std::vector<std::shared_ptr<tf::TestItem>> GetSampleTestItems(int64_t db);

    ///
    /// @brief 设置某行是否被选中
    ///
    /// @param[in]  row  行坐标
    /// @param[in]  flag 选中与否
    ///
    /// @return true:表示是否设置选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月2日，新建函数
    ///
    bool SetSelectFlag(int mode, int row, bool flag);
    bool SetAllSelectedFlag(int mode);
    QVariant GetSelectFlag(int mode, int row);
    bool ClearSelected(int mode);

    inline std::set<int64_t> GetAllSelectFlag(int mode) const
    {
        return (0 == mode) ? m_selectedSampleMap : m_selectedItemMap;
    }

    // 获取从上往下的第一个被选中的行号, 没找到则返回-1
    int GetPrevSelectedRow(int mode);


    ///
    /// @brief 获取样本的项目的数据库主键集合
    ///
    /// @param[in]  db  样本数据库主键
    ///
    /// @return 项目数据库主键集合
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月26日，新建函数
    ///
    boost::optional<std::vector<int64_t>> GetSampleTestItemMap(int64_t db);

    std::map<int64_t, AssayResults> GetSampleTestResult()
    {
        std::lock_guard<std::recursive_mutex> lockRead(m_readWriteLock);
        return m_assayResults;
    }

    ///
    /// @brief 获取样本项目中的项目类型（主要用于界面的筛选）
    ///
    /// @param[in]  db  样本数据库主键
    ///
    /// @return 样本包含的项目类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月17日，新建函数
    ///
    std::set<int> GetSampleAssayCodes(int64_t db);
	
	///
	///  @brief 获取按样本显示的显示数据
	/// 
	///
	///  @param[in]   mode   模式
	///  @param[in]   index  索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
	///
	QVariant GetSampleDisplayData(SampleColumn::SAMPLEMODE mode, int row, int column);
	
	///
	///  @brief 更新倒计时
	///
	///
	///  @param[in]   itemId  项目id
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
	///
	void UpdateCountDown(int64_t itemId);

	///
	/// @brief 若项目正在运行，则开始推算项目测试完毕的倒计时
	///
	/// @param[in]  testItemInfo  项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年12月15日，新建函数
	///
	void StartTestTimeCountDown(const tf::TestItem& testItemInfo) const;

    ///
    /// @brief 根据id获取剩余运行时间
    ///
    /// @param[in]  id  项目的数据库主键
    ///
    /// @return 剩余执行时间
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月3日，新建函数
    ///
    boost::optional<int> GetRuntimesFromId(int64_t id) const; 

	///
	/// @brief 根据项目信息获取反应时间（单位秒）
	///
	/// @param[in]  testItemInfo 项目信息  
	///
	/// @return 反应时间秒
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年12月15日，新建函数
	///
	int GetReactionTime(const tf::TestItem& testItemInfo) const;

    bool ResetTestItemDulition(tf::TestItem& testItemInfo);

    ///
    /// @brief 清除计数链表
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月3日，新建函数
    ///
    void ClearRuntimeList() 
    {
        m_countDown.clear();
    };

	///
	///  @brief 项目设置界面修改项目后刷新按项目显示表格
	///
	///
	///  @param[in]   assayCode  项目通道号
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月30日，新建函数
	///
	void UpdateItemShowDatas(int32_t assayCode);

	///
	/// @brief 获取当前行对应的数据（样本）
	///
	/// @param[in]  column       位置信息
	/// @param[in]  sampleInfo  原始数据
	///
	/// @return 返回数据
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2022年12月15日，新建函数
	///
	QString GetItemShowData(const int column, const tf::TestItem & testItem) const;

protected:
    DataPrivate();
    ///
    /// @brief 初始化函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月19日，新建函数
    ///
    void Init();

	///
	/// @brief 打印样本信息
	///
	/// @param[in]  vSIs  样本信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月5日，新建函数
	///
	void PrintSampleInfo(std::vector<tf::SampleInfo>& vSIs);

	///
	/// @brief 打印项目信息
	///
	/// @param[in]  testItems  项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月22日，新建函数
	///
	void printTestItemInfo(const tf::TestItem& testItem);

    ///
    /// @brief 更新模式里面的数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    bool Update();

    ///
    /// @brief 更新项目信息
    ///
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月26日，新建函数
    ///
    bool UpdateTestItem();

    ///
    /// @brief 完成数据地图
    ///
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月4日，新建函数
    ///
    bool FinishMap();

	///
	///  @brief 创建样本显示数据（一行）
	///
	///
	///  @param[in]   sample  样本信息
	///
	///  @return	样本信息显示数据列表
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年1月4日，新建函数
	///
	QVector<QString> GetSampleShowDatas(const tf::SampleInfo& sample);

	///
	///  @brief 获取当前行对应的数据
	///
	///
	///  @param[in]   column    列位置信息
	///  @param[in]   dataPara  原始数据
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
	///
	QString GetItemData(const int column, const DataPara& dataPara) const;

	///
	///  @brief 创建样本显示数据（一行）
	///
	///
	///  @param[in]   sample  样本信息
	///
	///  @return	样本信息显示数据列表
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年1月4日，新建函数
	///
	QVector<QString> GetItemShowDatas(const tf::TestItem & testItem);


    ///
    /// @brief 添加样本信息
    ///
    /// @param[in]  samples  样本信息列表
    ///
    /// @return true表示添加成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    bool AddSamples(const std::vector<::tf::SampleInfo>& samples);

    ///
    /// @brief 删除样本信息
    ///
    /// @param[in]  samples  样本信息列表
    ///
    /// @return true表示删除成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    bool DelSamples(const std::vector<::tf::SampleInfo>& samples);

    ///
    /// @brief 修改样本信息
    ///
    /// @param[in]  samples  样本信息
    ///
    /// @return true表示修改成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    bool ModifySamples(const std::vector<::tf::SampleInfo>& samples);

	///
	/// @brief 修改项目信息
	///
	/// @param[in]  testItems  项目信息列表
	///
	/// @return true:修改成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月22日，新建函数
	///
	bool ModifyTestItem(const std::vector<::tf::TestItem>& testItems);

	///
	/// @brief 更新目标数据
	///
	/// @param[in]  srcData  源数据
	/// @param[in]  dstData  目标数据
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月22日，新建函数
	///
	bool UpdateTestItemData(const ::tf::TestItem& srcData , std::shared_ptr<::tf::TestItem> dstData);

Q_SIGNALS:
    void sampleChanged(ACTIONTYPE mode, int start, int end);
	void testItemChanged(std::set<int64_t> datadbs);
	void runAssayTimerChanged();
private slots:
    ///
    /// @brief 样本信息更新
    ///
    /// @param[in]  enUpdateType  更新类型
    /// @param[in]  vSIs		  更新的样本信息数组
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    void OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs);

	///
	/// @brief 项目 信息更新消息处理函数
	///
	/// @param[in]  enUpdateType  更新类型
	/// @param[in]  vSIs		  项目信息列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月22日，新建函数
	///
	void OnTestItemUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::TestItem, std::allocator<tf::TestItem>> vSIs);

	///
	/// @brief 当结果提示变化以后，需要同等更新按项目展示的结果信息的flag
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月15日，新建函数
	///
	void OnUpDateTestItemResultFlag();
private:
    std::map<int64_t, std::shared_ptr<tf::SampleInfo>>		m_sampInfoData;				// 样本数据
    std::vector<int64_t>					                m_samplePostionVec;			// 样本数据地图
    std::map<int64_t, std::shared_ptr<tf::TestItem>>		m_testItemData;				// 项目数据
    std::vector<int64_t>					                m_testitemPostionVec;		// 项目数据地图
    mutable std::map<int64_t, AssayResults>					m_assayResults;				// 项目结果
    mutable std::recursive_mutex							m_readWriteLock;			// 模式的核心数据的读写锁
    std::map <int64_t, std::vector<int64_t>>				m_sampleMap;				// 样本地图（样本与项目关系--样本db--项目dbs）
    std::set<int64_t>                                       m_selectedSampleMap;        // 数据浏览界面样本选中列表
    std::set<int64_t>                                       m_selectedItemMap;          // 数据浏览界面项目选中列表
																       
	mutable std::map<int64_t, QVector<QString>>				m_sampleShowMap;			// 按样本显示字符串地图（对应显示一行数据）
	mutable std::map<int64_t, QVector<QString>>				m_itemShowMap;				// 按项目显示字符串地图（对应显示一行数据）
    mutable MyCountDown                                     m_countDown;                // 倒计时处理类
    RangeRowManage                                          m_rangVisble;               // 需要显示的范围

    std::shared_future<bool>								m_sampleFuture;				// 查询样本数据线程的句柄
    std::shared_future<bool>								m_assayFuture;				// 查询项目数据线程的句柄
    int														m_sampleCount;              // 样本测试数目
    int														m_testItemCount;            // 项目测试数目
    MOUDLESTATUS											m_moduleStatus;				// 模式的状态
    bool													m_isAssayUpdate;            // 是否需要刷新项目信息
    int														m_modeType;
    friend class											QSampleAssayModel;
    friend class											QSampleDetailModel;
	bool													m_needPrintSample;			///< 是否打印样本信息
};

class QSampleDetailModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static QSampleDetailModel& Instance();
    virtual ~QSampleDetailModel() { ; };

    void SetRange(int first, int last);
    std::shared_ptr<tf::TestItem> GetCurrentItem(int row);
    void SetData(const boost::optional<tf::SampleInfo>& spSampleInfo, tf::TestMode::type sampleTestMode = tf::TestMode::type::INVALID_MODE);
    bool setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/);
	std::map<int64_t, std::shared_ptr<tf::TestItem>> GetTestSelectedItems();
    boost::optional<tf::SampleInfo> GetCurrentSample() { return m_currentSample; };

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool SortItem(std::vector<std::shared_ptr<tf::TestItem>>& m_testItems, std::vector<int64_t>& dbs, std::map<int, int>& showOrder);
    inline bool SortItem(std::vector<std::shared_ptr<tf::TestItem>>& m_testItems, std::vector<int64_t>& dbs){
        return SortItem(m_testItems, dbs, m_showOrder);
    };
protected:
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QSampleDetailModel();

	///
	/// @brief 当前项目是否使能复查功能
	///
	/// @param[in]  spSample  项目对应的样本信息
	/// @param[in]  testItem  项目信息
	///
	/// @return true:允许复查
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月23日，新建函数
	///
	bool IsEnableRecheck(boost::optional<tf::SampleInfo> sampleValue, const tf::TestItem& testItem) const;

	///
	/// @brief 样本中的当前项目是否有被选中复查的
	///
	/// @param[in]  assyaCode  项目编号
	/// @param[in]  testItems  所有项目列表
	///
	/// @return true:被选中
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月23日，新建函数
	///
	bool IsAssayCodeSetReChecked(const tf::TestItem& testItemData, std::vector<std::shared_ptr<tf::TestItem>> testItems) const;

	// 友元
	friend class QDataItemInfo;
protected slots: 
    ///
    /// @brief 初始化显示顺序表
    ///
    ///
    /// @return true初始化成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月1日，新建函数
    ///
    bool InitDisplayOrder();

Q_SIGNALS:
    void runAssayTimerChanged();
    void columnChanged();
	void selectStatusChanged();

private:
    std::vector<std::shared_ptr<tf::TestItem>> m_testItems;
	std::map<int64_t, int>			m_assayPostionMap;			///< 项目位置地图
	tf::TestMode::type				m_sampleTestMode;			///< 样本测试模式 
    QStringList                     m_hearData;
    boost::optional<tf::SampleInfo> m_currentSample;
    std::map<int, int>              m_showOrder;                ///< 显示顺序(项目编号--位置)
    mutable RangeRowManage          m_assayVisble;               // 需要显示的范围
};

class QSampleAssayModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	// 模式类型
	enum class VIEWMOUDLE
	{
		ASSAYSELECT = 0,       /// 项目选择
		DATABROWSE,			   /// 数据浏览-按样本展示
		ASSAYBROWSE,		   /// 按项目展示
	};

	virtual ~QSampleAssayModel();


    void SetFilterSamples(int number) {m_sampleFilter = number; };
    int GetFilterSamples() { return m_sampleFilter; };

	///
	/// @brief 获取单例对象
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	static QSampleAssayModel& Instance();

	///
	/// @brief 获取当前行
	///
	///
	/// @return 当前行
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月25日，新建函数
	///
    inline int GetCurrentRow() { return m_currentRow; };

    ///
    /// @brief 获取样本量和样本测试数目
    ///
    ///
    /// @return 样本量和样本测试数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月28日，新建函数
    ///
    std::pair<int, int> GetSampleTestInfo();

    ///
    /// @brief 获取私有数据
    ///
    ///
    /// @return 数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月19日，新建函数
    ///
    //DataPrivate& GetPrivateData() { return m_privateData; };

    ///
    /// @brief 获取样本地图
    ///
    ///
    /// @return 样本地图
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月19日，新建函数
    ///
    std::map <int64_t, std::vector<int64_t>>& GetSampleMap() { return DataPrivate::Instance().m_sampleMap; };

	///
	/// @brief 获取模式
	///
	///
	/// @return 获取当前模式
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月26日，新建函数
	///
    inline VIEWMOUDLE GetModule() { return m_moudleType; };

	///
	/// @brief 设置模式类型（项目选择，项目浏览-（按样本展示，按项目展示））
	///
	/// @param[in]  module  模式类型
	///
	/// @return 设置成功返回true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月21日，新建函数
	///
	bool SetSampleModuleType(VIEWMOUDLE module);

	///
	/// @brief 若项目正在运行，则开始推算项目测试完毕的倒计时
	///
	/// @param[in]  testItemInfo  项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年12月15日，新建函数
	///
	void StartTestTimeCountDown(const tf::TestItem& testItemInfo) const;

	///
	/// @brief
	///     获取单元格属性(可编辑、可选择)
	///
	/// @param[in]    index      当前单元格索引
	///
	/// @return       QVariant   包装的数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	///
	/// @brief 设置当前样本显示行
	///
	/// @param[in]  row  行号
	///
	/// @return true设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月10日，新建函数
	///
    inline void SetCurrentSampleRow(int64_t row) { m_currentRow = row; };

	///
	///  @brief 根据索引获取样本类型
	///
	///
	///  @param[in]   index  索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月9日，新建函数
	///
	boost::optional<::tf::SampleType::type> GetSampleTypeByIndex(const QModelIndex index);

	///
	///  @brief 根据索引获取主键
	///
	///
	///  @param[in]   index  索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月9日，新建函数
	///
	boost::optional<int64_t> GetIdByIndex(const QModelIndex index);

	///
	///  @brief 按样本显示
	///
	///
	///  @param[in]   mode   模式
	///  @param[in]   index  索引
	///  @param[in]   role   角色
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
	///
	QVariant SampleShow(const int mode, const QModelIndex &index, int role) const;
	
	///
	///  @brief 按项目显示
	///
	///
	///  @param[in]   mode   模式
	///  @param[in]   index  索引
	///  @param[in]   role   角色
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月20日，新建函数
	///
	QVariant ItemShow(const int mode, const QModelIndex &index, int role) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool ClearSelectedFlag(int mode);
    bool SetAllSelectedFlag(int mode);
    bool SetSelectFlag(int mode, int row, bool flag, bool isfromAll = false);
protected:
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QSampleAssayModel();
    // 友元
    friend class QDataItemInfo;
	friend class QSampleAssayDataManager;

Q_SIGNALS:
    void columnChanged();
	void selectItemChanged();

private:

	bool DoDataChanged(const QModelIndex& startIndex, const QModelIndex& endIndex);

	/// @brief 样本是否来着病人（其他则为校准品和质控品）
	///
	/// @param[in]  sample  样本信息
	///
	/// @return true表示来着病人
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月22日，新建函数
	///
    inline bool IsSamplePatient(const tf::SampleInfo& sample) 
    {
        return (sample.sampleType == ::tf::SampleType::SAMPLE_TYPE_PATIENT) ? true : false;
    };

	///
	/// @brief 获取样本的显示数据(按项目显示)
	///
	/// @param[in]  index  位置
	/// @param[in]  role   角色
	///
	/// @return 显示数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月3日，新建函数
	///
	QVariant DataAssay(const QModelIndex &index, int role) const;

private:
	QStringList                     m_assaySelectHeader;        ///< 项目选择的表头
	QStringList                     m_sampleBrowseHeader;       ///< 数据浏览的表头
	QStringList                     m_assayBrowseHeader;        ///< 数据浏览中按项目展示
	VIEWMOUDLE						m_moudleType;				///< 模式类型
	mutable int64_t					m_currentRow;				///< 当前样本选择行
    mutable int                     m_sampleFilter;             ///< 样本过滤数
    std::atomic_bool				m_isDataChanged;			///< 是否发出了datchanged的消息，用于控制消息回环
};

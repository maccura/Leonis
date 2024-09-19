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
/// @file     FileExporter.h
/// @brief    文件导出类, 负责导出execl、txt文件。
///
/// @author   6889/ChenWei
/// @date     2021年11月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2021年11月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QString>
#include <QPageSize>
#include <QVector>
#include <QMap>
#include <memory>
#include "PrintExportDefine.h"
#include "src/public/ConfigDefine.h"

//定义
typedef struct DataExportType
{
    QString                 sheetName;      // sheet页名称（excel导出适用）
    QVariantList            title;          // 数据表头
    QVariantList            contents;       // 数据（数据适用title.size()区分第几行）
}ExTaskType;

// 数据浏览样本导出信息映射
class SEMetaInfos
{
public:
    SEMetaInfos();

    // 获取单例
    static std::shared_ptr<SEMetaInfos> GetInstance();

    std::shared_ptr<SEMetaInfo> GetMetaInfo(ExportInfoEn key);

    void Init();

private:
    static std::shared_ptr<SEMetaInfos> s_Instance;
    QMap<ExportInfoEn, std::shared_ptr<SEMetaInfo>> m_mapSEInfos;
};

class FileExporter
{
public:
    FileExporter();
    ~FileExporter();

    ///
    ///  @brief 导出数据浏览/历史数据
    ///
    ///
    ///  @param[in]   vecSampleInfo  试剂信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年11月30日，新建函数
    ///
    bool ExportSampleInfo(ExpSampleInfoVector& vecSampleInfo, QString strFileName, tf::UserType::type UserType_);

    ///
    ///  @brief 导出数据浏览/历史数据为PDF格式
    ///
    ///
    ///  @param[in]   vecSampleInfo  试剂信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年11月30日，新建函数
    ///
    bool ExportSampleInfoToPdf(ExpSampleInfoVector& vecSampleInfo, SampleExportModule& PDFInfo);

    ///
    ///  @brief 导出试剂信息
    ///
    ///
    ///  @param[in]   vecLogDatas  试剂信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportReagentInfo(ReagentItemVector& vecLogDatas, QString strFileName);

    ///
    ///  @brief 导出耗材信息
    ///
    ///
    ///  @param[in]   vecLogDatas  耗材信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportSupplyInfo(SupplyItemVector& vecLogDatas, QString strFileName);

    ///
    ///  @brief 导出维护日志
    ///
    ///
    ///  @param[in]   vecLogDatas  维护日志数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportMaintainLog(MaintenanceLog& vecLogDatas, QString strFileName);

    ///
    ///  @brief 导出报警信息
    ///
    ///
    ///  @param[in]   vecLogDatas  报警信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportAlarmInfo(AlarmInfoLog& vecLogDatas, QString strFileName);

    ///
    ///  @brief 导出操作日志
    ///
    ///
    ///  @param[in]   vecOperationLog  操作日志数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportOperationLog(OperationLog& Log, QString strFileName);

    ///
    ///  @brief 导出耗材更换日志
    ///
    ///
    ///  @param[in]   vecReaChangeLog  耗材更换日志数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportChangeLog(ReagentChangeLog& Log, QString strFileName);

    ///
    ///  @brief 导出反应杯历史信息
    ///
    ///
    ///  @param[in]   info  反应杯历史信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 7951/LuoXin，2024年04月02日，新建函数
    ///
    bool ExportCupHistoryInfo(CupHistoryInfo& info, QString strFileName);

    ///
    ///  @brief 导出液路信息
    ///
    ///
    ///  @param[in]   info  液路信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 7951/LuoXin，2024年04月02日，新建函数
    ///
    bool ExportLiquidInfo(LiquidInfo& info, QString strFileName);

    ///
    ///  @brief 导出温度信息
    ///
    ///
    ///  @param[in]   vecTemInfo  温度信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportTemperatureInfo(TemperatureLog& Log, QString strFileName);

    ///
    ///  @brief 导出计数信息
    ///
    ///
    ///  @param[in]   Info         计数信息
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCountInfo(StatisticalInfo& Info, QString strFileName);

    ///
    ///  @brief 导出下位机单元日志
    ///
    ///
    ///  @param[in]   Log          下位机单元信息
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportUnitLog(LowerCpLog& Log, QString strFileName);

    ///
    ///  @brief 导出样本统计信息
    ///
    ///
    ///  @param[in]   Info          样本统计信息
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportSampleStatisticsInfo(SampleStatisticsInfo& Info, QString strFileName);

    ///
    ///  @brief 导出试剂统计信息
    ///
    ///
    ///  @param[in]   Info          试剂统计信息
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportReagentStatisticsInfo(ReagentStatisticsInfo& Info, QString strFileName);

    ///
    ///  @brief 导出校准统计信息
    ///
    ///
    ///  @param[in]   Info          校准统计信息
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCaliStatisticsInfo(CaliStatisticsInfo& Info, QString strFileName);

    ///
    ///  @brief 导出校准信息
    ///
    ///
    ///  @param[in]   strExportTextList  校准信息数据列表
    ///  @param[in]   strFileName        文件名
    ///  @param[in]   Curve              校准曲线
    ///
    ///  @return	
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCaliInfo(CaliResultInfo& CaliInfo, QString strFileName);

    ///
    ///  @brief 导出校准历史信息
    ///
    ///
    ///  @param[in]   strExportTextList  校准信息数据列表
    ///  @param[in]   strFileName        文件名
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCaliHistoryInfo(CaliHistoryInfo& CaliInfo, QString strFileName);

    ///
    ///  @brief 导出校准品记录
    ///
    ///
    ///  @param[in]   strExportTextList  校准信息数据列表
    ///  @param[in]   strFileName        文件名
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCaliRecordInfo(CaliRecordInfo& RecordInfo, QString strFileName);

    ///
    ///  @brief 导出校准架概况记录
    ///
    ///
    ///  @param[in]   RackInfo  校准信息数据列表
    ///  @param[in]   strFileName        文件名
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportCaliRackInfo(CaliRackInfo& RackInfo, QString strFileName);

    ///
    ///  @brief 导出LJ质控信息
    ///
    ///
    ///  @param[in]   QcLjInfo  LJ质控信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportLJInfo(QCLJInfo& QcLjInfo, QString strFileName);

    ///
    ///  @brief 导出Twin Plot 图信息
    ///
    ///
    ///  @param[in]   QcYDInfo  Twin Plot 图信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportTpInfo(QCYDInfo& info, QString strFileName);

    ///
    ///  @brief 导出质控品记录
    ///
    ///
    ///  @param[in]   QcYDInfo  Twin Plot 图信息数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool ExportQCItemRecord(QCItemRecord& info, QString strFileName);

	///
	///  @brief 导出数据浏览/历史数据到文件
	///
	///
	///  @param[in]   strExportTextList  数据浏览/历史数据数据列表
	///  @param[in]   strFileName  文件名
	///
	///  @return	true 导出成功 false 导出失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool ExportInfoToFile(QStringList& strExportTextList, QString strFileName);

    ///
    /// @brief 通过分类器进行数据导出
    ///
    /// @param[in]  strFileName  文件名
    ///
    /// @return true 导出成功 false 导出失败
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    bool ExportInfoToFileByClassify(QString strFileName, const QList<ExTaskType>& exTask, const ExpSampleInfoVector& pdfData);

    ///
    ///  @brief 将数据写入到文件
    ///
    ///
    ///  @param[in]   strExportTextList  数据列表
    ///  @param[in]   strFileName  文件名
    ///
    ///  @return	true 写入成功 false 写入失败
    ///
    ///  @par History: 
	///  @li 6889/ChenWei，2023年5月22日，新建函数
    ///
    bool SaveInfoToFile(QStringList& strExportTextList, QString strFileName);

    ///
    /// @brief 获取一个导出数据的分类器引用对象，可以在外部进行清空
    ///
    ///
    /// @return 分类器引用对象
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    QList<ExTaskType>& GetDataClassify() { return m_dataExTask; };

	///
	/// @brief 根据需求获取文件名
	///
	/// @param[in]  isCh		是否生化
	/// @param[in]  filePath	文件路径
	/// @param[in]  suffix		文件后缀
	///
	/// @return 文件名
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年6月27日，新建函数
	///
	static const QString GetFileName(bool isCh, const QString& filePath);

private:

	///
	///  @brief 导出为txt文本
	///
	///
	///  @param[in]   strExportTextList  数据列表
	///  @param[in]   strFileName        文件名
	///
	///  @return	true 导出成功 false 导出失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool SaveToTxt(QStringList& strExportTextList, QString strFileName);

	///
	///  @brief 导出为csv
	///
	///
	///  @param[in]   strExportTextList  数据列表
	///  @param[in]   strFileName        文件名
	///
	///  @return	true 导出成功 false 导出失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool SaveToCsv(QStringList& strExportTextList, QString strFileName);

	///
	///  @brief 导出为xlsx表格
	///
	///
	///  @param[in]   strExportTextList  数据列表
	///  @param[in]   strFileName        文件名
	///
	///  @return	true 导出成功 false 导出失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool SaveToXlsx(QStringList& strExportTextList, QString strFileName);
    bool SaveToXlsxByClassify(QString strFileName);

    ///
    ///  @brief 导出为xlsx表格
    ///
    ///
    ///  @param[in]   strExportTextList  数据列表
    ///  @param[in]   strFileName        文件名
    ///  @param[in]   Image              图片
    ///  @param[in]   iX.iY               插入位置
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年11月3日，新建函数
    ///
    bool SaveToXlsx(QStringList& strExportTextList, QImage& Image, int iX, int iY, QString strFileName);

	///
	///  @brief 导出为pdf文件
	///
	///
	///  @param[in]   strExportTextList  数据列表
	///  @param[in]   strFileName        文件名
	///  @param[in]   iColumn			 文本列数
	///  @param[in]   pageSize           纸张大小（A0、A1、A3、A4 ...）
	///
	///  @return	true 导出成功 false 导出失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
	///
	bool SaveToPdf(QStringList& strExportTextList, QString strFileName, int iColumn, QPageSize::PageSizeId pageSize);

    ///
    ///  @brief 导出为pdf文件
    ///
    ///
    ///  @param[in]   strExportTextList  数据列表
    ///  @param[in]   Image              图片
    ///  @param[in]   iX.iY              图片插入位置
    ///  @param[in]   strFileName        文件名
    ///  @param[in]   iColumn			 文本列数
    ///  @param[in]   pageSize           纸张大小（A0、A1、A3、A4 ...）
    ///
    ///  @return	true 导出成功 false 导出失败
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2023年11月3日，新建函数
    ///
    bool SaveToPdf(QStringList& strExportTextList, QImage& Image, int iX, int iY, QString strFileName, int iColumn, QPageSize::PageSizeId pageSize);

	///
	/// @brief
	///     获取对齐的表格文本
	///
	/// @param[in]  notAlignTblText  未对齐的表格文本
	///
	/// @return 对齐的表格文本
	///
	/// @par History:
	/// @li 4170/TangChuXian，2022年4月2日，新建函数
	///
    const QString GetAlignTableText(const QString& notAlignTblText);

	///
	/// @brief
	///     对齐表格文本
	///
	/// @param[in]  notAlignTblText  未对齐的表格文本
	///
	/// @return 对齐的表格文本
	///
	/// @par History:
	/// @li 4170/TangChuXian，2022年4月2日，新建函数
	///
    const QString GetAlignTableText(QStringList& notAlignTblText);

private:
    QList<ExTaskType>               m_dataExTask;           ///< 导出数据任务
};


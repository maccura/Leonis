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
/// @file     QCalibrateSet.h
/// @brief    校准设置
///
/// @author   5774/WuHongTao
/// @date     2022年2月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class SortHeaderView;

namespace Ui {
    class QCalibrateSet;
};
class QStandardItemModel;
class CaliBrateRecordDialog;
class QItemSelectionModel;
class QPostionEdit;
class ChCaliRecordInfo;

class RichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    RichTextDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class QCalibrateSet : public QWidget
{
    Q_OBJECT

    enum CaliGroupInfo{
        GROUPSN = 0,        // 序号
        GROUPNAME,          // 校准品组名称
        GROUPLOT,           // 校准品组批号
        DOCASSAY,           // 校准项目
        EXPIRYEDATE,        // 失效日期
        CALILEVEL1,         // 校准品1
        CALILEVEL2,         // 校准品2
        CALILEVEL3,         // 校准品3
        CALILEVEL4,         // 校准品4
        CALILEVEL5,         // 校准品5
        CALILEVEL6          // 校准品6
    };

public:
    QCalibrateSet(QWidget *parent = Q_NULLPTR);
    ~QCalibrateSet();

    ///
    /// @brief
    ///     初始化校准品组
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月24日，新建函数
    ///
    void ResetCaliBrateGroup();

    ///
    /// @brief刷新设备试剂列表
    ///     
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月24日，新建函数
    ///
    void RefreshPage(/*std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices*/);

	///
	/// @brief 更新界面，置空输入框
	///     
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月21日，新建函数
	///
	void ReInitSerachCombox();

    ///
    /// @brief
    ///     显示校准文档信息
    ///
    /// @param[in]  groupDb     校准品组的数据库主键
    /// @param[in]  docKeyInfo  校准文档的关键信息
    /// @param[out]  doc        校准文档的详细信息
    ///
    /// @return true表示查询成功
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月24日，新建函数
    ///
    bool ShowDocInfo(int64_t groupDb, ch::tf::CaliDocKeyInfo& docKeyInfo, ch::tf::CaliDoc& doc);

    ///
    /// @brief 获取所有的校准品数据
    ///
    ///
    /// @return 所有校准品组信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年4月11日，新建函数
    ///
    const std::map<int64_t, ch::tf::CaliDocGroup>& GetAllCaliGroups();

    ///
    /// @brief 获取校准品组对应的校准品文档
    ///
    /// @param[in]    校准品组id
    ///
    /// @return 校准文档列表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年4月11日，新建函数
    ///
    const std::vector<ch::tf::CaliDoc>& GetCaliDocsByGroupId(int64_t groupId);

private:
	///
	/// @brief
	///     初始化生化校准设置界面中的查询名称
	/// 
	/// 
	///
	/// @par History:
	/// @li 8580/GongzhiQiang，2022年12月19日，新建函数
	///
	void initCaliProjectNameBox();

	///
	/// @brief 根据项目名称，判断是否是目标校准组，也就是该项目组是否使用了该校准组
	///
	/// @param[in]  projectName  项目名称
	/// @param[in]  docGroup     待判断的校准组
	///
	/// @return  true:该项目使用了该校准组   false:该项目没有使用该校准组
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月21日，新建函数
	///
	bool isAimCaliDocGroup(const std::string& projectName, const ch::tf::CaliDocGroup& docGroup);

    ///
    /// @brief 按照第一列进行升序排序
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月30日，新建函数
    ///
    void SetAscSortByFirstColumn();

    ///
    /// @brief 获取校准品组信息
    ///
    /// @param[in]  index  选的单元格
    /// @param[out]  caliGroup  校准品组信息
    ///
    /// @return true:获取成功   false:获取失败
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月31日，新建函数
    ///
    bool GetCaliGroupInfo(const QModelIndex& index, ch::tf::CaliDocGroup& caliGroup);

    ///
    /// @brief 获取一个初始的列表(用于初始显示)
    ///
    ///
    /// @return 初始的每一列宽度
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月28日，新建函数
    ///
    QVector<int> GetInitColWidth();

    ///
    /// @brief 获取列表当前的宽度列表（用于宽度保持）
    ///
    ///
    /// @return 初始的每一列宽度
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月28日，新建函数
    ///
    QVector<int> GetCurColWidth();

    ///
    /// @brief 设置表格宽度
    ///
    /// @param[in]  vecWidth  宽度列表
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月28日，新建函数
    ///
    void SetColWidth(const QVector<int>& vecWidth);

    ///
    /// @brief 设置打印导出是否可用
    ///
    /// @param[in]  hasCurve  是否存在可用曲线
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月22日，新建函数
    ///
    void SetPrintAndExportEnabled(bool enabled);

    ///
    /// @brief 将缓存的数据显示到表格中
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月11日，新建函数
    ///
    void UpdateCacheCaliInfoToTable();

    ///
    /// @brief 根据表格行的校准品组信息
    ///
    /// @param[in]  row  表格行
    /// @param[in]  docGroup  校准品组数据
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月11日，新建函数
    ///
    bool SetTableRowData(int row, const ch::tf::CaliDocGroup& docGroup);

    ///
    /// @brief 删除指定校准品组主键对应的缓存数据
    ///
    /// @param[in]  groupId  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月11日，新建函数
    ///
    bool ClearCacheByGroupId(int64_t groupId);

    ///
    /// @brief 根据校准品组主键获取校准品组信息
    ///
    /// @param[in]  groupId  校准品组主键
    /// @param[in]  docGroup  校准品组信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月11日，新建函数
    ///
    bool GetSingleCaliGroupFromDb(int64_t groupId, ch::tf::CaliDocGroup& docGroup);

    ///
    /// @brief 获取导出数据
    ///
    /// @param[in]  strExportTextList  
    ///
    /// @return true 标识成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月5日，新建函数
    ///
    bool ExportCalibratorInfo(QStringList& strExportTextList);

    ///
    /// @brief 获取打印数据
    ///
    /// @param[in]  info  校准记录
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月12日，新建函数
    ///
    void GetPrintExportInfo(ChCaliRecordInfo& info);

    ///
    /// @brief 因用户操作导致校准品组信息被修改，刷新页面
    ///
    /// @param[in]  dbId  校准品组的主键
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月18日，新建函数
    ///
    void OnCaliInfoUpdatedByUser(int64_t dbId);

    ///
    /// @brief 新增了校准品组
    ///
    /// @param[in]  dbId  数据库主键
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月18日，新建函数
    ///
    void OnNewCaliGroupAdded(int64_t dbId);

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;



private slots:
    ///
    /// @brief
    ///     点击校准组名称
    ///
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月24日，新建函数
    ///
    void OnSelectGroup(QModelIndex index);

    ///
    /// @brief
    ///     修改或者增加校准品组(点击在空行为增加否则为修改)
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月25日，新建函数
    ///
	void OnEditCalibrateGroup();

	///
	/// @brief
	///     录入校准品组
	///
    /// @param[in]  InputType  录入方式
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年8月24日，新建函数
	///
	void OnAddNewCalibrateGroup();

    ///
    /// @brief
    ///     更新校准数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月25日，新建函数
    ///
    void OnUpdateCaliBrateData();

    ///
    /// @brief
    ///     删除校准品组
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月25日，新建函数
    ///
    void OnDeleteCaliBrateGroup();

	///
	/// @brief
	///     校准品文档表格双击信号响应
	///
	/// @param[in]  index  位置索引
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年8月29日，新建函数
	///
	void onCaliDocViewDoubleClicked(const QModelIndex& index);

	///
	/// @brief 
	///     当位置信息设置后
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年8月29日，新建函数
	///
	void OnRefreshPostion();

	///
	/// @brief 校准品组更新通知
	///  
	/// @param[in]  infos  更新的校准品组信息
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月19日，新建函数
	///
	void OnCaliGroupUpdate(std::vector<ch::tf::CaliGroupUpdate, std::allocator<ch::tf::CaliGroupUpdate>> infos);


	///
	/// @brief 校准项目查询
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
	///
	void OnCaliProjectNameSearch();

    ///
    /// @brief 项目配置发生变化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月9日，新建函数
    ///
    void OnAssayUpdated();

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
    /// @bref
    ///		权限变更响应
    ///
    /// @par History:
    /// @li 8090/YeHuaNing, 2024年01月15日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief 打印/导出
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月4日，新建函数
    ///
    void OnExport();
    void OnPrint();

private:
    Ui::QCalibrateSet*              ui;
    QStandardItemModel*             m_CaliBrateGroup;           // 校准品组
    CaliBrateRecordDialog*          m_addDocDialog;             // 新增校准组
    CaliBrateRecordDialog*          m_modifyDocDialog;          // 修改校准组
    SortHeaderView *                m_pCaliBrateGroupHeader;    // 排序表头
    QItemSelectionModel*            m_SelectModel;              // 选择模式
	std::map<std::string, std::vector<std::string>> m_caliDocGroupIndex;// 存储校准品组信息，用于生成不重复的序号
	QPostionEdit*					m_CaliPostionEditDialog;    // 校准位置编辑对话框

    std::map<int64_t, ch::tf::CaliDocGroup> m_CaliGroupSave;    // 保存校准组信息
    std::map<int64_t, QStringList>  m_caliAssayIndex;           // 校准项目索引 （key: 校准品组数据库主键， 校准项目名称列表）
    std::map<int64_t, std::vector<ch::tf::CaliDoc>>
        m_caliDocIndex;                                         // 校准项目索引 （key: 校准品组数据库主键， 校准品组对应的每个文档的缓存）
    QList<ch::tf::CaliDoc>          m_CurrentCalis;             // 当前的校准品组
    QVector<int>                    m_docAssayCodeVec;          // 选择某个校准品组以后，显示信息时的序号保持使用
    QString                         m_filterAssayName;          // 筛选项目名称
    int                             m_iSortOrder = 0;           // 当前排序规则
    int                             m_iSortColIdx = -1;         // 当前排序列
    bool                            m_isRunningStatus = false;  // 是否在运行状态
};

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
/// @file     MaintainGroupModel.h
/// @brief 	  维护项目组模型
///
/// @author   7656/zhang.changjiang
/// @date      2022年9月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QAbstractTableModel>
#include <src/thrift/gen-cpp/defs_types.h>

class MaintainGroupModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum MAINTAIN_GROUP_COL
    {
        INDEX = 0,                  // 序号
        NAME = 1					// 组合名称
    };

private:
    MaintainGroupModel(QObject *parent = Q_NULLPTR);

public:
    virtual ~MaintainGroupModel();

    ///
    ///  @brief 获取单例
    ///
    ///  @return	单例对象
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月16日，新建函数
    ///

    static MaintainGroupModel& Instance();
    ///
    ///  @brief	设置维护项目组数据
    ///
    ///  @param[in]   data  
    ///	
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    void SetData(std::vector<tf::MaintainGroup> & data);
    void UpdateAutoMantains(const std::string& groupName, const std::vector<tf::AutoMaintainCfg>& autoCfg);

    ///
    /// @bref
    ///		更新维护组的自动维护设置，但不改变原始设置时间，以及减少原始设置项
    ///
    /// @param[in/out] mtGroup 目标维护组
    /// @param[in] autoCfg 自动维护配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年8月16日，新建函数
    ///
    void UpdateGroupCfgWithNoSetTimeChanged(tf::MaintainGroup& mtGroup, const std::vector<tf::AutoMaintainCfg>& autoCfg);

    ///
    /// @bref
    ///		通过组合维护名获取组合维护项
    ///
    /// @param[in] groupName 组合维护名
    /// @param[out] mGroup 返回的组合维护项
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月22日，新建函数
    ///
    bool TakeMaintainGroup(const QString& groupName, tf::MaintainGroup& mGroup);

    ///
    ///  @brief 清空选中的维护组维护项列表
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年2月17日，新建函数
    ///
    ::tf::MaintainGroupType::type ClerCurRowItemData();

    ///
    ///  @brief 设置选中的维护组包含的维护项
    ///
    ///  @param[in]   items  维护项列表
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
    ///
    void SetSelectedGroup(const std::vector<tf::MaintainItem>& items);

    ///
    ///  @brief	添加维护组
    ///
    ///  @param[in]   data  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
    void AddGroupData(tf::MaintainGroup & data);
    bool IsExistEmptyNameGroup();

    ///
    ///  @brief	删除选中的维护组
    ///	
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
    void DeleteSelectGroupData();

	///
	///  @brief 删除指定的维护组
	///
	///
	///  @param[in]   lstMg  维护组列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年9月6日，新建函数
	///
	void DeleteGroups(const std::vector<::tf::MaintainGroup>& lstMg);

    ///
    ///  @brief	获取数据
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
    inline std::vector<tf::MaintainGroup> & GetData() { return m_data; };
	
	///
	///  @brief 设置选中的维护组名称
	///
	///
	///  @param[in]   text  维护组名称
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
	///
	void SetSelectedGroupName(const QString& text);

    ///
    ///  @brief	获取选中的数据
    ///
    ///  @param[out]   selectedData  被选中的数据
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
    bool GetSelectData(tf::MaintainGroup& selectedData);
    void CleanSelectedGroupItems();
    bool IsSelectedGroupSingleType();
	
	///
	///  @brief 获取指定索引值的维护组
	///
	///
	///  @param[out]   outGroup  输出维护组
	///  @param[in]    index     索引值
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
	///
	bool GetGroupByIndex(tf::MaintainGroup & outGroup,const QModelIndex& index);

	///
	///  @brief 获取对应id的维护组
	///
	///
	///  @param[out]   outGroup  输出维护组
	///  @param[in]    groupId   维护组id
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月16日，新建函数
	///
	bool GetGroupById(tf::MaintainGroup & outGroup, const int64_t groupId);

	///
	/// @brief  更新维护组配置
	///
	/// @param[in]  group  新的维护组
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月11日，新建函数
	///
	void UpdateGroup(const ::tf::MaintainGroup& group);

    ///
    ///  @brief 设置选中的行
    ///
    ///  @param[in]   iSelectedRow  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
    void SetSelectedRow(int iSelectedRow);

    ///
    ///  @brief 获取选中的行
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
    ///
    int GetSelectedRow();

    ///
    ///  @brief 选中的维护组在指定位置插入维护项类型
    ///
    ///  @param[in]   iPos  
    ///  @param[in]   item  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年3月1日，新建函数
    ///
    void InsertItem(int iPos, ::tf::MaintainItem item);

    ///
    ///  @brief 将from位置的元素移动到to位置
    ///
    ///  @param[in]   from	 移动的初始位置
    ///  @param[in]   to	 移动的目标位置
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
    ///
    void Move(int from, int to);

    ///
    ///  @brief 选中的维护组指定位置复制维护项
    ///
    ///  @param[in]   index  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
    ///
    void CopySelGroupItemByIndex(const QModelIndex &index);

    ///
    ///  @brief 删除选中的维护组指定位置维护项
    ///
    ///  @param[in]   index  
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
    ///
    void DelSelGroupItemByIndex(const QModelIndex &index);

	///
	///  @brief 是否是最后一行
	///
	///
	///  @param[in]   iRow  行数
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
	///
	bool IsLastRow(int iRow);

	///
	///  @brief 设置表头
	///
	///
	///  @param[in]   headerNames  表头字符列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月11日，新建函数
	///
	void SetmHeaderNames(const QStringList headerNames);

	///
	///  @brief 获取维护组名称
	///
	///
	///  @param[in]   group  维护组
	///
	///  @return	维护组名称
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	static QString GetGroupName(const tf::MaintainGroup& group);

    void BackupCurrentData();
    void RecoverBackupData();

signals:
	void CleanItemTableCheckBox(int item);

public slots:

    ///
    ///  @brief	配置数据
    ///
    ///  @param[in]   isChecked    选中状态
    ///  @param[in]   item		   配置的维护项
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
    ///
	void ConfigData(bool isChecked, const tf::MaintainItem &item);

public:
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
protected:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 用于备份实例数据
    struct BackupData 
    {
        QStringList bk_headerNames;
        std::vector<tf::MaintainGroup> bk_data;
        int bk_iSelectedRow;
    };

private:
    QStringList									m_headerNames;              // 表头名称集合
    std::vector<tf::MaintainGroup>				m_data;                     // 显示的数据
    int											m_iSelectedRow;				// 当前选中的行

    BackupData m_backupData;
};

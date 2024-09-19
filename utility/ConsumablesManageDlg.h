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
/// @file     ConsumablesManageDlg.h
/// @brief    耗材管理配置弹窗
///
/// @author   7915/LeiDingXiang
/// @date     2023年2月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include "shared/basedlg.h"
#include "src/public/im/ImConfigDefine.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QAbstractTableModel>
#include <QMouseEvent>
#include <QStyledItemDelegate>

namespace Ui
{
    class ConsumablesManageDlg;
};

class QSupplyManageModel;
class QImSupplyManageModel;


///////////////////////////////////////////////////////////////////////////
/// @class     ConsumablesItemTableDelegate
/// @brief 	   表格提示框代理
///////////////////////////////////////////////////////////////////////////
class ConsumablesItemTableDelegate : public QStyledItemDelegate
{
public:
    ConsumablesItemTableDelegate(QObject *parent = nullptr);
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class ConsumablesManageDlg : public BaseDlg
{
    Q_OBJECT

public:
    ConsumablesManageDlg(QWidget *parent = Q_NULLPTR);
    ~ConsumablesManageDlg();

    ///
    /// @brief  初始化窗体信息
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月2日，新建函数
    ///
    void InitBeforeShow();

    protected Q_SLOTS:
    ///
    /// @brief 权限变更
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月29日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::ConsumablesManageDlg*                           ui;
    QSupplyManageModel*                                 m_model;
    QImSupplyManageModel*                               m_ImModel;
};

class QSupplyManageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class COLSAMPLE
    {
        SEQ = 0,
        NAME,				                    ///< 项目名称
        SAMPLE_ACIDITY,	                        ///< 样本针酸性清洗液选择
        SAMPLE_ALKALINITY,	                    ///< 样本针碱性清洗液选择
        CUP_ACIDITY,	                        ///< 反应杯酸性清洗液选择
        CUP_ALKALINITY	                        ///< 反应杯碱性清洗液选择
    };

    struct ViewData
    {
        std::string     name;                   ///< 名称
        std::string     sn;                     ///< 序列号
        bool            acidityUp;              ///< 台面酸
        bool            alkalinityUp;           ///< 台面碱
        bool            acidityLower;           ///< 柜门酸
        bool            alkalinityLower;        ///< 柜门碱
        int             deviceType = 0;         ///< 设备类型
    };

    QSupplyManageModel();
    ~QSupplyManageModel() {};

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool SaveData();
    void UpdateDate();

protected:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QStringList						             m_heads;				    ///< 列表头
    std::vector<ViewData>                        m_data;                    ///< 数据
};


class QImSupplyManageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class COLSAMPLE
    {
        SEQ = 0,
        NAME,				    ///< 项目名称
        SUB1_SELECTED,	        ///< 底物液1选择
        SUB2_SELECTED,	        ///< 底物液2选择
        CONFECT1_SELECTED,	    ///< 清洗缓冲液1选择
        CONFECT2_SELECTED	    ///< 清洗缓冲液2选择
    };
    QImSupplyManageModel();
    ~QImSupplyManageModel() {};

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    ///
    /// @brief  保存数据
    ///
    ///
    /// @return 保存成功
    ///
    /// @par History:
	/// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    bool SaveData();

    ///
    /// @brief  更新耗材管理界面刷新数据
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    void UpdataShowData();

private:
	
	///
	/// @brief  是否执行底物填充
	///
	/// @param[in]  deviceSN  设备序列号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年7月8日，新建函数
	///
	bool IsNeedToFillPipe(const std::string& deviceSN);

	///
	/// @brief  获取指定维护项目类型的单项维护组id
	///
	/// @param[in]  itemType  维护项类型
	///
	/// @return 单项维护组id -1表示获取失败
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年2月22日，新建函数
	///
	int GetSingleMaintGroupId(const ::tf::MaintainItemType::type itemType);

    ///
    /// @brief  是否可以进行底物液启/停用
    ///
    /// @param[in]    DevSn:当前设备序列号 PreBtnStu:即将更改的状态
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年3月11日，新建函数
    ///
    bool IsChangeSub(const std::string& DevSn, const bool& PreBtnStu);

    ///
    /// @brief  获取当前设备状态
    ///
    /// @param[in]    m_DeviceSn DeviceSn设备号
    ///
    /// @return ::tf::DeviceWorkState::type 设备状态
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月6日，新建函数
    ///
    int32_t GetCurDeviceStat(std::string m_DeviceSn);

    ///
    /// @brief  是否可以进行清洗缓冲液启/停用
    ///
    /// @param[in]    DevSn:当前设备序列号 PreBtnStu:即将更改的状态 Btn:清洗缓冲液1/清洗缓冲液2
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月4日，新建函数
    ///
    bool IsChangeConfectLiquid(const std::string& DevSn, const bool& PreBtnStu,const int& Btn);

    ///
    /// @brief  查询当前清洗缓冲液是否为空
    ///
    /// @param[in]    int CurGroupNum 当前组号
    ///               bool bConfectIsEmpty  是否为空
    /// @return bool :true 查询成功 false 查询失败
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年6月14日，新建函数
    ///
    bool QueryCurConfectIsEmpty(const int& CurGroupNum, bool &bConfectIsEmpty);

    ///
    /// @brief  查询当前清洗缓冲液是否在配液
    ///
    /// @param[in]    
    ///               bool bConfectIsEmpty  是否为空
    /// @return bool :true 查询成功 false 查询失败
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年6月14日，新建函数
    ///
    bool QueryCurConfectIsMix(bool &m_bCurConfectIsMix);

    ///
    /// @brief  保存按钮被点击
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月2日，新建函数
    ///
    bool OnSaveBtnClicked(const std::shared_ptr<tf::DeviceInfo>& DevDate);

    ///
    /// @brief  传输选中的模块号和选项
    ///
    /// @param[in]    
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年2月3日，新建函数
    ///
    bool TransferChoseItem(const std::string& DeviceSn,const ImDeviceOtherInfo& imDevInfo);

protected:
    ///
    /// @brief  重写模型表格设置
    ///
    /// @param[in]    QModelIndex：模型索引
    ///
    /// @return int ： Qt::ItemFlags 单元格设置
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    ///
    /// @brief  重写模型数据行数
    ///
    /// @param[in]    QModelIndex：模型索引
    ///
    /// @return int ：行数
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    virtual int rowCount(const QModelIndex &parent) const override;

    ///
    /// @brief  重写模型数据列数
    ///
    /// @param[in]    QModelIndex：模型索引
    ///
    /// @return int ：列数
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    virtual int columnCount(const QModelIndex &parent) const override;
    ///
    /// @brief  重写模型数据显示
    ///
    /// @param[in]    index：当前行列 role：数据角色类型
    ///
    /// @return QVariant
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    QVariant data(const QModelIndex &index, int role) const override;

    ///
    /// @brief  重写模型表头内容
    ///
    /// @param[in]    section:部分  orientation:方向 role：角色
    ///
    /// @return QVariant
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年7月11日，新建函数
    ///
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QStringList						             m_heads;				     ///< 列表头
    std::vector<std::shared_ptr<tf::DeviceInfo>> m_data;                     ///< 数据
    bool                                         m_bCurConfectIsEmbty;       ///< 当前清洗缓冲液是否为空
    bool                                         m_bCurConfectIsMix;         ///< 当前清洗缓冲液是否在配液
    std::string                                  m_SupLot;                   ///< 当前清洗缓冲液批号
    std::string                                  m_SupSerial;                ///< 当前清洗缓冲液瓶号
    std::vector<ImDeviceOtherInfo>               m_VecConsumSet;             ///< 耗材管理设置缓存
};

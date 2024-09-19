/**************************************************************************
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
***************************************************************************
*   @file     main.cpp													  *
*   @brief    ui程序入口													  *
*																		  *
*   @author   3558/ZhouGuangMing										  *
*   @date     2021年3月18日												  *
*   @version  0.1														  *
*																		  *
*   @par Copyright(c):													  *
*       2015 迈克医疗电子有限公司，All rights reserved.					  *
*																		  *
*   @par History:														  *
*   @li 3558/ZhouGuangMing，2021年3月18日，新建文件						  *
*																		  *
*                         _oo0oo_										  *
*                        o8888888o										  *
*                        88" . "88										  *
*                        (| -_- |)										  *
*                        0\  =  /0										  *
*                      ___/`---'\___									  *
*                    .' \\|     |// '.									  *
*                   / \\|||  :  |||// \									  *
*                  / _||||| -:- |||||- \								  *
*                 |   | \\\  -  /// |   |								  *
*                 | \_|  ''\---/''  |_/ |								  *
*                 \  .-\__  '-'  ___/-. /								  *
*               ___'. .'  /--.--\  `. .'___								  *
*            ."" '<  `.___\_<|>_/___.' >' "".							  *
*           | | :  `- \`.;`\ _ /`;.`/ - ` : | |							  *
*           \  \ `_.   \_ __\ /__ _/   .-` /  /							  *
*       =====`-.____`.___ \_____/___.-`___.-'=====						  *
*                         `=---='										  *
*       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~						  *
*                 佛祖保佑         永无BUG								  *
*																		  *
***************************************************************************/
#include "Leonis.h"
#include "src/dcs/dcs.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/FileUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/Compress/QMiniZip.h"
#include "manager/SystemConfigManager.h"
#include "manager/SystemInitInfoManager.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "uidcsadapter/uidcsadapter.h"
#include "utility/maintaindatamng.h"
#include "thrift/UiService.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "mainwindow/loginwidget.h"
#include "mainwindow/mainwidget.h"
#include "mainwindow/LeonisSplashScreen.h"
#include "mainwindow/GlobalEventFilter.h"
#include "manager/WarnSetManager.h"
#include "manager/DictionaryQueryManager.h"
#include "printcom.hpp"
#include "regcom.hpp"
#include <string>
#include <memory>
#include <stdexcept>
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QFontDatabase>
#include <QFile>
#include <QResource>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <zip.h>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#include <DbgHelp.h>
using namespace std;

MainWidget* pMainWidget = nullptr;

///
/// @brief
///     未捕获异常的处理函数
///
/// @param[in]  lpExceptionInfo  异常信息
///
/// @return 是否已处理异常
///
/// @par History:
/// @li 3558/ZhouGuangMing，2019年11月11日，新建函数
///
LONG WINAPI VenusUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* lpExceptionInfo)
{
    // 合成dump文件名
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    TCHAR szFileName[64] = { 0 };
    _sntprintf(szFileName, sizeof(szFileName) / sizeof(szFileName[0]) - 1, _T("%s-%04d-%02d-%02d-%02d-%02d-%02d-%02d-%02d.dmp")
        , _T(APP_NAME), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, rand() % 100);

    // 创建dump文件
    HANDLE lhDumpFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != lhDumpFile)
    {
        // 设置异常信息
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ExceptionPointers = lpExceptionInfo;
        ExInfo.ThreadId = GetCurrentThreadId();
        ExInfo.ClientPointers = TRUE;

        // 导出异常信息到dump文件
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), lhDumpFile, MiniDumpWithFullMemory, &ExInfo, NULL, NULL);

        // 关闭dump文件
        CloseHandle(lhDumpFile);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif /* _WIN32 */

///
/// @brief  初始化代码对象的端口号
///
/// @param[in]  usDcsCtrlPort  端口号
///
/// @par History:
/// @li 3558/ZhouGuangMing，2023年2月19日，新建函数
///
void InitProxyPorts(unsigned short usDcsCtrlPort)
{
    ch::LogicControlProxy::m_usPort = usDcsCtrlPort;
    ch::c1005::LogicControlProxy::m_usPort = usDcsCtrlPort;
    im::LogicControlProxy::m_usPort = usDcsCtrlPort;
    im::i6000::LogicControlProxy::m_usPort = usDcsCtrlPort;
    ise::LogicControlProxy::m_usPort = usDcsCtrlPort;
    ise::ise1005::LogicControlProxy::m_usPort = usDcsCtrlPort;
}

void PrinterConnectionStatusUpdate(const bool connection_status)
{
    if (pMainWidget != nullptr)
    {
        pMainWidget->UpdatePrintDevStatus(connection_status);
    }
}

///
/// @brief 初始化打印库
///
/// @par History:
/// @li 6889/ChenWei，2023年12月29日，新建函数
///
void InitPrintcom()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString strFilePath = QCoreApplication::applicationDirPath();
    printcom::PRINTCOM_INIT_PARAM PrintParam;
    QString strModuleConfigFile = "/resource/printConfig/PrintConfiguration_" + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + ".json";
    strModuleConfigFile = strFilePath + strModuleConfigFile;
    PrintParam.template_config_files.push_back(strModuleConfigFile.toStdString());  // 加载公共模板

    // 加载免疫模板
    if (CommonInformationManager::GetInstance()->IsDeviceExistedByAssayClassify(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
    {
        QString strImModuleConfigFile = "/resource/printConfig/ImPrintConfiguration_" + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + ".json";
        strImModuleConfigFile = strFilePath + strImModuleConfigFile;
        PrintParam.template_config_files.push_back(strImModuleConfigFile.toStdString()); // 加载免疫模板
    }

    // 加载生化模板
    if (CommonInformationManager::GetInstance()->IsDeviceExistedByAssayClassify(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
    {
        QString strChModuleConfigFile = "/resource/printConfig/ChPrintConfiguration_" + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + ".json";
        strChModuleConfigFile = strFilePath + strChModuleConfigFile;
        PrintParam.template_config_files.push_back(strChModuleConfigFile.toStdString());
    }

    // 加载Ise模板
    if (CommonInformationManager::GetInstance()->IsDeviceExistedByAssayClassify(tf::AssayClassify::ASSAY_CLASSIFY_ISE))
    {
        QString strIseModuleConfigFile = "/resource/printConfig/IsePrintConfiguration_" + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + ".json";
        strIseModuleConfigFile = strFilePath + strIseModuleConfigFile;
        PrintParam.template_config_files.push_back(strIseModuleConfigFile.toStdString());
    }

    QString strTmplatePath = strFilePath + "/resource/system_templates";
    QString strUserTmplatePath = strFilePath + "/resource/user_templates";
    QString strConfigPath = strFilePath + "/resource/printConfig";
    QString strConfigFile = strConfigPath + "/printcom_config.json";
    QString strTransFile = "/printcom_";
    strTransFile += QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + ".qm";
    QString strTransFilePath = strConfigPath + strTransFile;

    QString strQss = strConfigPath + "/printcom.qss";

    PrintParam.system_template_dir = strTmplatePath.toStdString();
    PrintParam.user_template_dir = strUserTmplatePath.toStdString();
    PrintParam.config_file = strConfigFile.toStdString();
    PrintParam.trans_file = strTransFilePath.toStdString();
    PrintParam.qss_file = strQss.toStdString();
    PrintParam.printer_connection_status_handler = PrinterConnectionStatusUpdate;
    PrintParam.log_handler = [](const char *filename, const char* func_name, const int line, const int level, const char *format, ...) {
        ULOG(level, "Printcom::filename: %s; func_name: %s; info : %s", filename, func_name, format);
    };

    PrintParam.assign_export_result_handler = [](const std::string& unique_id, const int status){
        ULOG(LOG_INFO, "Printcom::assign_export_result unique_id: %s; status: %d", unique_id, status);
    };

    int iRect = printcom_init(PrintParam);
    ULOG(LOG_INFO, "printcom_init return:%d", iRect);
    ULOG(LOG_INFO, "printcom_init TmplatePath:%s", strTmplatePath.toStdString());    // 检查路径是否配置正确
    ULOG(LOG_INFO, "printcom_init ConfigPath:%s", strConfigPath.toStdString());
}

///
/// @bref 程序是否是开机自启动
///
/// @param[in] lstArgList 启动的参数列表，依据列表中是否存在自动启动参数（-autostart）来判断
///
/// @par History:
/// @li 8276/huchunli, 2024年9月10日，新建函数
///
bool IsAutoStartup(const QStringList& lstArgList)
{
    for (int i = 0; i < lstArgList.length(); ++i)
    {
        if (lstArgList[i].toLower().trimmed() == "-autostart")
        {
			return true;
        }
    }

	return false;
}

///
/// @brief 初始化所有管理器
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
bool LeonisInitManagers(QApplication* a)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判断软件是否注册(release版本)
#ifndef _DEBUG
    if (::reg::regcom::init_regcom(APP_NAME) != EXIT_SUCCESS
        || ::reg::regcom::verify_reg_code() != EXIT_SUCCESS)
    {
        ULOG(LOG_ERROR, "The software not register!");
        QMessageBox::critical(nullptr, u8"错误", u8"软件未注册，请使用配置工具进行注册！");
        return false;
    }
    else
    {
        SystemConfigManager::GetInstance()->SetNeedRegister(false);
    }
#endif

    // 加载系统配置文件
    std::shared_ptr<SystemConfigManager> spSCM = SystemConfigManager::GetInstance();
    if (!spSCM->LoadConfig(GetCurrentDir() + UI_SYS_CONFIG_FILE))
    {
        ULOG(LOG_ERROR, "Failed to execute SystemConfig::LoadConfig()");
        return false;
    }

    // 开启UI层的thrift服务
    std::shared_ptr<UiService> spUS = UiService::GetInstance();
    if (!spUS->Start(spSCM->GetUiReportPort()))
    {
        ULOG(LOG_ERROR, "Failed to execute Start()");
        return false;
    }

    // 检查是否是开机自启动，并开启DCS业务逻辑服务
	bool isAutoStart = IsAutoStartup(a->arguments());
    auto iRet = StartDcsService(isAutoStart);
    if (EXIT_SUCCESS != iRet)
    {
        // 如果是连接数据库失败
        if (EXIT_DB_CONN_FAILED == iRet)
        {
            QMessageBox::critical(nullptr, u8"错误", u8"连接数据库失败，请检查数据库连接配置");
        }
        
        ULOG(LOG_ERROR, "Failed to execute StartDcsService()");
        return false;
    }

    // 连接业务逻辑的thrift服务器
    std::shared_ptr<DcsControlProxy> spDCP = DcsControlProxy::GetInstance();
    if (!spDCP->Init("localhost", spSCM->GetDcsControlPort()))
    {
        ULOG(LOG_ERROR, "Failed to execute Init()");
        return false;
    }

    // 初始化代理对象的端口号
    InitProxyPorts(spSCM->GetDcsControlPort());

    QString strLan = QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType() + ".qm");

    // 加载翻译文件
    static QTranslator* lanuage = new QTranslator;
    bool ret = lanuage->load(qApp->applicationDirPath() + "/resource/lanuage/leonis_" + strLan);
    a->installTranslator(lanuage);

    // 加载Qt默认翻译文件
    static QTranslator* Qtlanuage = new QTranslator;
    bool bQtRet = Qtlanuage->load(qApp->applicationDirPath() + "/translations/qt_" + strLan);
    a->installTranslator(Qtlanuage);

    // 加载打印库翻译文件
    static QTranslator* printlibLanuage = new QTranslator;
    bool printlibRet = printlibLanuage->load(qApp->applicationDirPath() + "/resource/lanuage/mcprintlib_" + strLan);
    a->installTranslator(printlibLanuage);

    // 缓存字符串资源到dcs
    UiCommon::RecordResourceString();

    // 注册UIDCS适配器设备
    UiDcsAdapter::RegisterDevice();

    // 初始化UI项目编码管理器
    std::shared_ptr<CommonInformationManager> spAssayCodeManager = CommonInformationManager::GetInstance();
    if ((Q_NULLPTR == spAssayCodeManager) || !spAssayCodeManager->Init())
    {
        ULOG(LOG_ERROR, "Failed to execute AssayCodeManager::Init()");
        return false;
    }

    // 临时测试代码，用于质控申请信息查询性能测试，性能优化完成后删除-tcx
//     QList<QC_APPLY_INFO> stuQcApplyInfo;
//     if (!gUiAdapterPtr()->QueryDefaultQcTblInfo("", stuQcApplyInfo))
//     {
//         ULOG(LOG_ERROR, "%s(), QueryDefaultQcTblInfo() failed", __FUNCTION__);
//         return false;
//     }

	// 初始化报警设置管理器
	if (!WarnSetManager::GetInstance()->init())
	{
		ULOG(LOG_ERROR, "Failed to execute WarnSetManager::GetInstance()->init()");
	}

	// 初始化系统默认设置信息
	if (!SystemInitInfoManager::GetInstance()->SystemDefaultInfoInit())
	{
		ULOG(LOG_ERROR, "Failed to execute SystemInitInfoManager::GetInstance()->SystemDefaultInfoInit()");
	}

    // 初始化打印库
    InitPrintcom();

    //记录开机事件
    if (!spDCP->RecordStartEvent())
    {
        ULOG(LOG_ERROR, "Failed to execute RecordStartEvent()");
    }

    return true;
}

///
/// @brief
///     反初始化所有管理器
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
void LeonisUninitManagers()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 反初始化打印库
    int iRect = printcom::printcom_uninit();
    ULOG(LOG_INFO, "printcom_uninit return:%d", iRect);

    //记录关机事件
    if (!DcsControlProxy::GetInstance()->RecordExitEvent())
    {
        ULOG(LOG_ERROR, "Failed to execute RecordExitEvent()");
    }

    // 断开连接
    DcsControlProxy::GetInstance()->UnInit();

    // 停止DCS的thrift服务
    AbortDcsService();

    // 停止UI层的thrift服务
    std::shared_ptr<UiService> spUS = UiService::GetInstance();
    spUS->Stop();
}

///
/// @brief  备份上位机的日志
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2024年7月19日，新建函数
///
bool BackupLogs()
{
	// 检查源目录是否存在
	string srcDirPath = GetCurrentDir() + "/logs";
	if (!boost::filesystem::exists(srcDirPath) 
		|| !boost::filesystem::is_directory(srcDirPath))
	{
		qDebug() << "src directory not exist";
		return true;
	}

	// 检查目标目录是否存在
	string dstDirPath = GetCurrentDir() + "/../data/backup/upper_logs";
	if (!boost::filesystem::exists(dstDirPath))
	{
		// 创建完整目录
		if (!boost::filesystem::create_directories(dstDirPath))
		{
			qDebug() << "Failed to create dstDirPath";
			return false;
		}
	}

	// 创建zip文件对象
	auto formatTime = QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
	QString prefix = "leonis_logs_";
	QString filePath = QString::fromStdString(dstDirPath) + "/" + prefix + formatTime + ".zip";
	zipFile zf = zipOpen(filePath.toStdString().c_str(), APPEND_STATUS_CREATE);
	if (!zf)
	{
		qDebug() << "Failed to create zip file: " << filePath;
		return false;
	}

	// 自定义删除器，退出作用域时自动关闭文件
	std::shared_ptr<void> autoDeleter((void*)0, [&](void* p)
	{
		// 关闭zip文件
		zipClose(zf, NULL);
	});

	// 添加logs文件夹里的所有子文件、子文件夹到zip
	if (!QMiniZip::AddDirToZip(zf, srcDirPath))
	{
		qDebug() << "Failed to execute AddDirToZip()";
		return false;
	}

	try
	{
		// 如果目标文件夹中的文件数超限，那么尝试删除最老的文件
		FileUtil::DeleteOldestFile(dstDirPath);
		// 递归删除源文件夹及其内容
		boost::filesystem::remove_all(srcDirPath);
	}
	catch (std::exception& ex)
	{
		qDebug() << "Failed to execute remove_all()";
	}
	
	return true;
}

///
/// @brief
///     程序入口
///
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// 检查是否存在互斥运行的app
	vector<string> appNames{"configTool.exe", std::string(APP_NAME) + ".exe"};
	if (IsAnyProcessRunning(appNames))
	{
		ostringstream oss;
		oss << u8"本程序不可和以下程序同时运行：\n";
		for (auto& appName : appNames)
		{
			oss << "\n  " << appName;
		}

		QMessageBox::warning(nullptr, u8"运行互斥", oss.str().c_str());
		return EXIT_FAILURE;
	}

#ifdef _WIN32
	// 取消关机指令 qiuqf 20231223
	// bug: 0024325: [控制器] 管理员或普通用户关机后在1min内启动软件，软件启动成功但1min后关闭计算机 
	ExecuteCommand("shutdown /a");

    // 设置未捕获异常的处理函数
    ::SetUnhandledExceptionFilter(VenusUnhandledExceptionFilter);
#endif /* _WIN32 */

    // 初始化启动动画界面
    LeonisSplashScreen loader(qApp->applicationDirPath() + "/resource/video/Startup.wmv", LeonisSplashScreen::IMAGES_FORMAT::WMV);
    loader.showFullScreen();

	// 备份上位机的日志
	BackupLogs();

    // 初始化MLOG库，只有初始化成功后才能使用MLOG宏
    string strLogFile = GetCurrentDir() + "/log.ini";
    if (EXIT_SUCCESS != MLogInit(strLogFile.c_str()))
    {
        printf("[error] Failed to execute MLogInit()\n");
    }

    // 创建一个智能指针，退出作用域时会调用其自定义的删除器
    std::shared_ptr<void> spLog((void*)0, [](void* p)
    {
        MlogUninit();
    });

	ULOG(LOG_INFO, "########################################");
	ULOG(LOG_INFO, "# Ver: %s          #", GenerateBuildVersion());
	ULOG(LOG_INFO, "########################################");

	// 加载rcc文件
	QString rccFile = qApp->applicationDirPath() + "/leonis.rcc";
	if (!QResource::registerResource(rccFile))
	{
		ULOG(LOG_ERROR, "Failed to execute QResource::registerResource(%s)", rccFile.toStdString());
		return -1;
	}

	// 加载字体
	auto fontId = QFontDatabase::addApplicationFont(":/Leonis/resource/fonts/HarmonyOS_Sans_SC_Regular.ttf");
    QStringList strFamilyList = QFontDatabase::applicationFontFamilies(fontId);

    // 加载qss文件
    QFile styleFile(":/Leonis/resource/document/deviceView.qss");
    styleFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (styleFile.isOpen())
    {
        QString setstylesheet(styleFile.readAll());
        a.setStyleSheet(setstylesheet);
        styleFile.close();
    }

    // 初始化所有管理器对象
    if (!LeonisInitManagers(&a))
    {
        ULOG(LOG_ERROR, "Failed to execute LeonisInitManagers()");
        // 反初始化，防止业务逻辑线程崩溃
        LeonisUninitManagers();
        return 0;
    }

    // 创建主界面，从业务逻辑层加载数据
    MainWidget mainWidget(LeonisUninitManagers);
    pMainWidget = &mainWidget;

	// 创建全局事件过滤器
	GlobalEventFilter gEventFilter(&mainWidget);
	a.installEventFilter(&gEventFilter);

    // 系统加载完毕，关闭启动动画
    LoginWidget loginWidget;
    loginWidget.showFullScreen();

	// 延时2000ms关闭动画
    loader.Finish(2000);

    // 初始化打印机状态
    PrinterConnectionStatusUpdate(printcom::printcom_get_printer_connection_status());

    // 开机先清除界面设置的报警屏蔽信息（若当前已经清除过一次，则不重复清除）
    DcsControlProxy::GetInstance()->ClearAllAlarmShield();

    // 进入事件循环
    a.exec();
    return 0;
}

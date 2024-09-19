@echo off

for %%f in (*.ts) do (
    lrelease "%%~nf.ts"
) 

echo Compilation completed.

rem
rem 拷贝翻译文件
rem
xcopy ".\*.qm"  ..\..\..\..\build_windows\bin\x64\Debug\resource\lanuage /e /y
xcopy ".\*.qm"  ..\..\..\..\build_windows\bin\x64\Release\resource\lanuage /e /y
xcopy ".\*.qm"  ..\..\..\..\build_windows\bin\x86\Debug\resource\lanuage /e /y
xcopy ".\*.qm"  ..\..\..\..\build_windows\bin\x86\Release\resource\lanuage /e /y

echo Copy completed.

pause
@echo off

set ProjectDir=%1%
set OutDir=%2%
set Configuration=%3%

echo "ProjectDir=%ProjectDir%"
echo "OutDir=%OutDir%"
echo "Configuration=%Configuration%"
xcopy /D /Y /E %ProjectDir%..\..\%Configuration%\*.dll %OutDir%
xcopy /D /Y /E %ProjectDir%..\..\3rdparty\cef3\%Configuration%\*.dll %OutDir%
xcopy /D /Y /E %ProjectDir%..\..\3rdparty\cef3\%Configuration%\*.bin %OutDir%
xcopy /D /Y /E %ProjectDir%..\..\3rdparty\cef3\Resources\*.* %OutDir%
rem mkdir %OutDir%\www
xcopy /D /Y /E %ProjectDir%..\..\www\*.* %OutDir%\www\
xcopy /D /Y %ProjectDir%..\..\config.xml %OutDir%
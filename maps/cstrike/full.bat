echo Running full.bat on %1
@attrib -r %1.bsp
@%VPROJECT%\..\bin\vbsp %1
@%VPROJECT%\..\bin\vvis %2 %1
@%VPROJECT%\..\bin\vrad %1
echo Copying to %VPROJECT%\maps\%1.bsp
@attrib -r %VPROJECT%\maps\%1.bsp
copy %1.bsp %VPROJECT%\maps\%1.bsp

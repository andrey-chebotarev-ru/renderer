echo Running fast.bat on %1
@attrib -r %1.bsp
@%VPROJECT%\..\bin\vbsp %1
@%VPROJECT%\..\bin\vvis -mpi -fast %2 %1
@%VPROJECT%\..\bin\vrad -mpi -bounce 1 %1
echo Copying to %VPROJECT%\maps\%1.bsp
@attrib -r %VPROJECT%\maps\%1.bsp
copy %1.bsp %VPROJECT%\maps\%1.bsp

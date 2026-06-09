SET VPROJECT=U:\HL2\release\dev\hl2
u:\HL2\release\dev\bin\vbsp %1
u:\HL2\release\dev\bin\vvis.exe -threads 2 %1
u:\HL2\release\dev\bin\vrad.exe -extra -bounce 1 %1
copy %1.bsp u:\HL2\release\dev\hl2\maps\%1.bsp

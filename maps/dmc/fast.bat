U:\HL1ports\release\DEV\bin\vbsp %1
U:\HL1ports\release\DEV\bin\vvis.exe -threads 2 -fast %1
U:\HL1ports\release\DEV\bin\vrad.exe -bounce 1 %1

copy %1.bsp U:\HL1ports\release\DEV\hl1\maps\%1.bsp

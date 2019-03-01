@echo off

FOR /f %%f IN ('DIR /s/b "Vortex2D\*.h", "Vortex2D\*.cpp"') DO (
    ECHO Formatting %%f 
    clang-format.exe -i -style=file %%f   
)
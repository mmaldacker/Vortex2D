@echo off

FOR /f %%f IN ('DIR /s/b "Vortex\*.h", "Vortex\*.cpp", "Tests\*.h", "Tests\*.cpp", "Examples\*.h", "Examples\*.cpp",') DO (
    ECHO Formatting %%f 
    clang-format.exe -i -style=file %%f   
)
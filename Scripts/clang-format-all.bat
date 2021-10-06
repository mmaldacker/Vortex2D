@echo off

FOR /f %%f IN ('DIR /s/b "*.h", "*.cpp", "*.comp", "*.frag", "*.vert",') DO (
    ECHO Formatting %%f 
    clang-format.exe -i -style=file %%f   
)
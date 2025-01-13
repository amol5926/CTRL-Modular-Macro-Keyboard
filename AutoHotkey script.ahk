; Switch to Blender and minimize others
!d:: ; Alt + D
    ; Activate Blender
    IfWinExist, ahk_exe blender.exe
        WinActivate
    ; Minimize other windows
    IfWinExist, ahk_exe Designer.exe
        WinMinimize
    IfWinExist, ahk_exe Resolve.exe
        WinMinimize
    ; Minimize Windows Terminal if it's open
    IfWinExist, ahk_exe WindowsTerminal.exe
        WinMinimize
Return

; Switch to Affinity Designer and minimize others
^!r::
    ; Activate Designer
    IfWinExist, ahk_exe Designer.exe
        WinActivate
    ; Minimize other windows
    IfWinExist, ahk_exe blender.exe
        WinMinimize
    IfWinExist, ahk_exe Resolve.exe
        WinMinimize
    ; Minimize Windows Terminal if it's open
    IfWinExist, ahk_exe WindowsTerminal.exe
        WinMinimize
Return

; Switch to DaVinci Resolve and minimize others
^!b::
    ; Activate Resolve
    IfWinExist, ahk_exe Resolve.exe
        WinActivate
    ; Minimize other windows
    IfWinExist, ahk_exe blender.exe
        WinMinimize
    IfWinExist, ahk_exe Designer.exe
        WinMinimize
    ; Minimize Windows Terminal if it's open
    IfWinExist, ahk_exe WindowsTerminal.exe
        WinMinimize
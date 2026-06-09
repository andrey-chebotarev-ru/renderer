if "%1"=="fast" goto buildall_fast
goto buildall_full

:buildall_fast
set BUILDTYPE=fast
goto buildall_begin

:buildall_full
set BUILDTYPE=full
goto buildall_begin

:buildall_begin
call %BUILDTYPE%.bat c0a0
call %BUILDTYPE%.bat c0a0
call %BUILDTYPE%.bat c0a0a
call %BUILDTYPE%.bat c0a0b
call %BUILDTYPE%.bat c0a0c
call %BUILDTYPE%.bat c0a0d
call %BUILDTYPE%.bat c0a0e
call %BUILDTYPE%.bat c1a0
call %BUILDTYPE%.bat c1a0a
call %BUILDTYPE%.bat c1a0b
call %BUILDTYPE%.bat c1a0c
call %BUILDTYPE%.bat c1a0d
call %BUILDTYPE%.bat c1a0e
call %BUILDTYPE%.bat c1a1
call %BUILDTYPE%.bat c1a1a
call %BUILDTYPE%.bat c1a1b
call %BUILDTYPE%.bat c1a1c
call %BUILDTYPE%.bat c1a1d
call %BUILDTYPE%.bat c1a1f
call %BUILDTYPE%.bat c1a1g
call %BUILDTYPE%.bat c1a2
call %BUILDTYPE%.bat c1a2a
call %BUILDTYPE%.bat c1a2b
call %BUILDTYPE%.bat c1a2c
call %BUILDTYPE%.bat c1a2d
call %BUILDTYPE%.bat c1a3
call %BUILDTYPE%.bat c1a3a
call %BUILDTYPE%.bat c1a3b
call %BUILDTYPE%.bat c1a3c
call %BUILDTYPE%.bat c1a3d
call %BUILDTYPE%.bat c1a4
call %BUILDTYPE%.bat c1a4b
call %BUILDTYPE%.bat c1a4d
call %BUILDTYPE%.bat c1a4e
call %BUILDTYPE%.bat c1a4f
call %BUILDTYPE%.bat c1a4g
call %BUILDTYPE%.bat c1a4i
call %BUILDTYPE%.bat c1a4j
call %BUILDTYPE%.bat c1a4k
call %BUILDTYPE%.bat c2a1
call %BUILDTYPE%.bat c2a1a
call %BUILDTYPE%.bat c2a1b
call %BUILDTYPE%.bat c2a2
call %BUILDTYPE%.bat c2a2a
call %BUILDTYPE%.bat c2a2b1
call %BUILDTYPE%.bat c2a2b2
call %BUILDTYPE%.bat c2a2c
call %BUILDTYPE%.bat c2a2d
call %BUILDTYPE%.bat c2a2e
call %BUILDTYPE%.bat c2a2f
call %BUILDTYPE%.bat c2a2g
call %BUILDTYPE%.bat c2a2h
call %BUILDTYPE%.bat c2a3
call %BUILDTYPE%.bat c2a3a
call %BUILDTYPE%.bat c2a3b
call %BUILDTYPE%.bat c2a3c
call %BUILDTYPE%.bat c2a3d
call %BUILDTYPE%.bat c2a3e
call %BUILDTYPE%.bat c2a4
call %BUILDTYPE%.bat c2a4a
call %BUILDTYPE%.bat c2a4b
call %BUILDTYPE%.bat c2a4c
call %BUILDTYPE%.bat c2a4d
call %BUILDTYPE%.bat c2a4e
call %BUILDTYPE%.bat c2a4f
call %BUILDTYPE%.bat c2a4g
call %BUILDTYPE%.bat c2a5
call %BUILDTYPE%.bat c2a5a
call %BUILDTYPE%.bat c2a5b
call %BUILDTYPE%.bat c2a5c
call %BUILDTYPE%.bat c2a5d
call %BUILDTYPE%.bat c2a5e
call %BUILDTYPE%.bat c2a5f
call %BUILDTYPE%.bat c2a5g
call %BUILDTYPE%.bat c2a5w
call %BUILDTYPE%.bat c2a5x
call %BUILDTYPE%.bat c3a1
call %BUILDTYPE%.bat c3a1a
call %BUILDTYPE%.bat c3a1b
call %BUILDTYPE%.bat c3a2
call %BUILDTYPE%.bat c3a2a
call %BUILDTYPE%.bat c3a2b
call %BUILDTYPE%.bat c3a2c
call %BUILDTYPE%.bat c3a2d
call %BUILDTYPE%.bat c3a2e
call %BUILDTYPE%.bat c3a2f
call %BUILDTYPE%.bat c4a1a
call %BUILDTYPE%.bat c4a1b
call %BUILDTYPE%.bat c4a1c
call %BUILDTYPE%.bat c4a1f
call %BUILDTYPE%.bat c4a2
call %BUILDTYPE%.bat c4a2a
call %BUILDTYPE%.bat c4a2b
call %BUILDTYPE%.bat c5a1

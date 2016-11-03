rem: Prepare build solution envitorment for Windows
rem: important: Should be run with administrator privileges!
rem: The script installs the chocolatey tool and uses it to install cmake
rem: No need to run it if cmake is already installed

@powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" && SET PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin

choco install --yes cmake
# Rhodesia Language Installation Scripts

This directory contains installation scripts for the Rhodesia programming language on different operating systems.

## Available Scripts

### Fedora Installation
- **File**: `install_fedora.sh`
- **Description**: Installs Rhodesia on Fedora systems
- **Requirements**: Root/sudo access
- **Usage**:
  ```bash
  chmod +x install_fedora.sh
  sudo ./install_fedora.sh
  ```

### Linux Installation (Debian/Ubuntu)
- **File**: `install_linux.sh`
- **Description**: Installs Rhodesia on Debian/Ubuntu based systems
- **Requirements**: Root/sudo access
- **Usage**:
  ```bash
  chmod +x install_linux.sh
  sudo ./install_linux.sh
  ```

### Windows Installation
- **File**: `install_windows.bat`
- **Description**: Installs Rhodesia on Windows systems
- **Requirements**: Administrator privileges
- **Usage**: Double-click the file or run from Command Prompt as Administrator

## Installation Process

Both scripts perform the following steps:

1. **Check for administrator/root privileges**
2. **Install required dependencies**:
   - Build tools (CMake, Make, Git)
   - Compiler toolchain (GCC/Clang, LLVM)
   - Development libraries
3. **Clone the Rhodesia repository** (if not already present)
4. **Build the Rhodesia compiler**
5. **Add Rhodesia to system PATH**
6. **Create convenient access methods** (symlink on Linux, shortcut on Windows)

## Post-Installation

After successful installation:

- You can run `rhodesia` from any terminal/command prompt
- Try running: `rhodesia examples/basic/hello.rho`
- The compiler will be installed in:
  - Linux: `/opt/rhodesia-lang/build/`
  - Windows: `C:\rhodesia-lang\build\`

## Troubleshooting

### Fedora Issues
- If you get permission errors, ensure you're running with `sudo`
- If dependencies fail to install, check your DNF package manager and internet connection
- Ensure you have the RPM Fusion repositories enabled if needed

### Debian/Ubuntu Issues
- If you get permission errors, ensure you're running with `sudo`
- If dependencies fail to install, check your APT package manager and internet connection
- For older Ubuntu versions, you may need to adjust package names

### Windows Issues
- Ensure Chocolatey is installed or the script has internet access to install it
- If the PATH doesn't update immediately, you may need to restart your terminal
- For build issues, ensure Visual Studio build tools are installed

## Manual Installation

If you prefer manual installation:

1. Clone the repository: `git clone https://github.com/giulicrenna/rhodesia-lang.git`
2. Install dependencies manually
3. Build with: `mkdir build && cd build && cmake .. && make`
4. Add the build directory to your PATH

## Notes

- The scripts are designed for fresh installations
- Existing installations will be detected and reused
- Internet connection is required for dependency installation and repository cloning

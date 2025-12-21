#!/bin/bash

# Rhodesia Language Installer for Fedora
# This script installs the Rhodesia programming language and its dependencies on Fedora

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Please run this script as root or with sudo."
    exit 1
fi

# Update package lists
echo "Updating package lists..."
dnf update -q -y

# Install required dependencies for Fedora
echo "Installing required dependencies..."
dnf install -q -y \
    @development-tools \
    cmake \
    git \
    gcc-c++ \
    make \
    readline-devel \
    ncurses-devel \
    openssl-devel \
    bzip2-devel \
    sqlite-devel \
    llvm-toolset \
    clang \
    lldb \
    lld \
    libedit-devel \
    zlib-devel \
    libffi-devel \
    gdbm-devel \
    xz-devel \
    ncurses-compat-libs \
    sqlite-devel \
    tk-devel \
    libuuid-devel

# Clone Rhodesia repository if not already present
if [ ! -d "/opt/rhodesia-lang" ]; then
    echo "Cloning Rhodesia repository..."
    git clone https://github.com/giulicrenna/rhodesia-lang.git /opt/rhodesia-lang
else
    echo "Rhodesia repository already exists at /opt/rhodesia-lang"
fi

# Build the project
echo "Building Rhodesia..."
cd /opt/rhodesia-lang
mkdir -p build
cd build
cmake ..
make

# Add Rhodesia to PATH
if ! grep -q "/opt/rhodesia-lang/build" /etc/environment; then
    echo "Adding Rhodesia to system PATH..."
    echo 'PATH="/opt/rhodesia-lang/build:$PATH"' >> /etc/environment
    source /etc/environment
fi

# Create symlink for easy access
if [ ! -f "/usr/local/bin/rhodesia" ]; then
    echo "Creating symlink..."
    ln -s /opt/rhodesia-lang/build/rhodesia /usr/local/bin/rhodesia
fi

echo ""
echo "Rhodesia installation completed successfully!"
echo "You can now run 'rhodesia' from anywhere in your terminal."
echo "Try running: rhodesia examples/basic/hello.rho"

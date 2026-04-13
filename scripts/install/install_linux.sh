#!/bin/bash

# Rhodesia Language Installer for Linux
# This script installs the Rhodesia programming language and its dependencies

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Please run this script as root or with sudo."
    exit 1
fi

# Update package lists
echo "Updating package lists..."
apt-get update -qq

# Install required dependencies
echo "Installing required dependencies..."
apt-get install -y -qq \
    build-essential \
    cmake \
    git \
    g++ \
    make \
    libreadline-dev \
    libncurses-dev \
    libssl-dev \
    libbz2-dev \
    libsqlite3-dev \
    llvm \
    libllvm-ocaml-dev \
    llvm-runtime \
    llvm-dev \
    clang \
    lldb \
    lld \
    libclang-dev \
    libedit-dev \
    libz-dev \
    zlib1g-dev \
    libffi-dev \
    libgdbm-dev \
    liblzma-dev \
    libncursesw5-dev \
    libsqlite3-dev \
    tk-dev \
    uuid-dev

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

# Copy Rhodesia math libraries into installable libs directory
LIB_TARGET="/opt/rhodesia-lang/libs"
LIB_SOURCE="/opt/rhodesia-lang/examples/10_applications/math_library"
LIB_MODULES=(core linear_algebra numerical statistics)

echo "Provisioning Rhodesia math libraries..."
rm -rf "$LIB_TARGET"
mkdir -p "$LIB_TARGET"
for module in "${LIB_MODULES[@]}"; do
    if [ -d "$LIB_SOURCE/$module" ]; then
        cp -r "$LIB_SOURCE/$module" "$LIB_TARGET/$module"
    fi
done

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

LIB_ENV_LINE="RHODESIA_LIB_PATH=\"$LIB_TARGET\""
echo "Configuring RHODESIA_LIB_PATH..."
if grep -q "^RHODESIA_LIB_PATH=" /etc/environment; then
    sed -i "s|^RHODESIA_LIB_PATH=.*|$LIB_ENV_LINE|" /etc/environment
else
    echo "$LIB_ENV_LINE" >> /etc/environment
fi
export RHODESIA_LIB_PATH="$LIB_TARGET"
source /etc/environment

echo ""
echo "Rhodesia installation completed successfully!"
echo "You can now run 'rhodesia' from anywhere in your terminal."
echo "Try running: rhodesia examples/basic/hello.rho"

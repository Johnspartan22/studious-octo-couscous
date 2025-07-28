this is for me only 


sudo apt update
sudo apt upgrade -y



# Install GCC 11 (required for compatibility)
sudo apt install -y gcc-11 g++-11

# Install build tools
sudo apt install -y \
    build-essential \
    cmake \
    make \
    git \
    autoconf \
    pkg-config

# Install required libraries
sudo apt install -y \
    libboost-all-dev \
    libgmp-dev \
    libpugixml-dev \
    liblua5.1-0 \
    liblua5.1-0-dev \
    lua5.1 \
    libxml2-dev \
    libmariadb-dev \cd ~


git clone https://github.com/Johnspartan22/studious-octo-couscous.git
cd studious-octo-couscous
    libmariadb-dev-compat \
    libssl-dev \
    libcrypto++-dev \
    zlib1g-dev


mkdir build
cd build

CC=gcc-11 CXX=g++-11 cmake ..


# Compile using all CPU cores
make -j$(nproc)

# OR compile with single core if you have memory issues
# make -j1

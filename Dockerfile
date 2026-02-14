FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install Dependencies
RUN apt-get update && apt-get install -y build-essential gcc g++ clang cmake git curl zip tar gdb gdbserver && rm -rf /var/lib/apt/lists/*

# Install VCPKG
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && /opt/vcpkg/bootstrap-vcpkg.sh

# Set up the application directory
WORKDIR /app
COPY . .

# Arguments for matrix build
ARG CMAKE_BUILD_TYPE=Debug
ARG CC=gcc
ARG CXX=g++

# Persist ARGs as ENVs so they are available at runtime for CMD
ENV CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
ENV CC=${CC}
ENV CXX=${CXX}

# Configure + build
RUN mkdir -p build && \
    export CC=${CC} && \
    export CXX=${CXX} && \
    cmake -S . -B build \
      -DBUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
      -DCMAKE_C_FLAGS="-fsanitize=address,undefined,leak -fno-omit-frame-pointer -g" \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build build --parallel $(nproc)

# Enable ASAN_OPTIONS to get better stack traces in Docker logs
ENV ASAN_OPTIONS=symbolize=1:debug=true:check_initialization_order=true:detect_stack_use_after_return=true

# Run tests
CMD ["bash", "-c", "ctest --test-dir build -C ${CMAKE_BUILD_TYPE} --output-on-failure"]
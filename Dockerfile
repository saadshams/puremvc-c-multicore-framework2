FROM ubuntu:22.04

# Install Dependencies
RUN apt update && apt install -y build-essential gcc g++ cmake git curl zip

# Install VCPKG
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && /opt/vcpkg/bootstrap-vcpkg.sh

# Set up the application directory
WORKDIR /app
COPY . .

# Configure + build
RUN mkdir -p build-debug && \
    cmake -S . -B build-debug \
      -DBUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build build-debug --parallel

# Run tests
CMD ["ctest", "--test-dir", "build-debug", "-C", "Debug", "--output-on-failure"]

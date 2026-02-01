FROM ubuntu:22.04

# Install Dependencies
RUN apt update && apt install -y gcc g++ cmake git curl zip

# Install VCPKG
WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git
WORKDIR /opt/vcpkg
RUN ./bootstrap-vcpkg.sh

# Add VCPKG to PATH
ENV PATH="/opt/vcpkg:${PATH}"

WORKDIR /app

FROM debian:bookworm-slim

# Install build tools + MySQL connector (includes runtime .so)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libmysqlcppconn-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt .
COPY build/ build/
COPY include/ include/

# Build and install, then strip source from image
RUN mkdir cmake-build && cd cmake-build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . && \
    cmake --install . --prefix /usr/local && \
    cd /app && rm -rf cmake-build build include CMakeLists.txt

# Create config dir and non-root user
RUN useradd --system --no-create-home --shell /usr/sbin/nologin meter-reader && \
    mkdir -p /app/config && \
    chown -R meter-reader:meter-reader /app

USER meter-reader

ENTRYPOINT ["meter_reader"]
CMD ["/app/config/meter_config.json"]

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD pgrep -x meter_reader || exit 1
FROM debian:bookworm

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        inetutils-ping \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ft_ping

CMD ["bash"]

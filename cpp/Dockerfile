#
# dparillo/georectify
#
# A georectification service built on top of osgeo/gdal:alpine-normal

# or licensed under X/MIT (LICENSE.TXT) Copyright 2019 Dave Parillo

FROM osgeo/gdal:alpine-normal-latest
# FROM osgeo/gdal:alpine-normal-latest as builder


MAINTAINER Dave Parillo <dparillo@forwardslope.com>

RUN apk update && \
    apk add --no-cache \
    bash \
    curl \
    boost-dev \
    openssl \
    git \
    linux-headers \
    make cmake libtool autoconf automake pkgconfig g++ \
    && mkdir georectify

COPY CMakeLists.txt /georectify/
COPY src/ /georectify/src/

RUN echo "building georect . . . " \
    && mkdir -p /georectify/build \
    && cd /georectify/build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make 


##FROM osgeo/gdal:alpine-normal-latest as runner
#RUN apk update && \
#    apk add --no-cache \
#    bash \
#    curl \
#    \
#    && mkdir /data \
#    && addgroup -S appgroup \
#    && adduser --disabled-password --no-create-home --system --gecos "" appuser appgroup 

# COPY --from=builder  /build/src/georectify /usr/bin/


# RUN chmod 644 *.py

EXPOSE 8080

# USER appuser

# HEALTHCHECK --interval=1m --timeout=2s \
#     CMD curl -f http://localhost:8080/ || exit 1

# ENTRYPOINT ["python3", "service.py"]

CMD bash


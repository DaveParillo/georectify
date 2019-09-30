#
# dparillo/georect
#
# A georectification service built on top of osgeo/gdal:alpine-normal

# or licensed under X/MIT (LICENSE.TXT) Copyright 2019 Dave Parillo

FROM osgeo/gdal:alpine-normal-latest

MAINTAINER Dave Parillo <dparillo@forwardslope.com>

RUN apk update && \
    apk add --no-cache \
    bash \
    curl \
    \
    && if [ ! -e /usr/bin/python ]; then ln -sf python3 /usr/bin/python ; fi \
    && python3 -m ensurepip \
    && rm -r /usr/lib/python*/ensurepip \
    && pip3 install --no-cache --upgrade pip setuptools wheel \
    &&  if [ ! -e /usr/bin/pip ]; then ln -s pip3 /usr/bin/pip ; fi \
    && pip install cherrypy numpy \
    && mkdir /data \
    && addgroup -S appgroup \
    && adduser --disabled-password --no-create-home --system --gecos "" appuser appgroup 

COPY resources/service.py .
COPY resources/process.py .

RUN chmod 644 *.py

EXPOSE 8080

USER appuser

ENTRYPOINT ["python3", "service.py"]


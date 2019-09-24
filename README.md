# Georectification service demo
Demo service to georectify tiff images

Based on GDAL and CherryPy

The image builds on osgeo/gdal:alpine-normal-latest,
using GDAL 3.x and Python3.

Build with this command:    
```
docker build -t dparillo/georectify .
```

and run with:   
```
docker run --rm -it -p 8080:8080 -v $(pwd)/data:/data dparillo/georectify
```

The service runs on `http://localhost:8080/process`

The input to the API is a simple JSON object POSTed to the service endpoint:


```json
{
  "file": "sample.tif",
  "valid_time": "1997-01-23T14:51:08",
  "epsg": 26911,
  "transform": [530184.0, 1, 0, 3900640.0, 0, -1],
  "target_epsg": 4326
}
```

The input file is modified in place.

The transform inputs are the affine transform parameters in GDAL's expected order.
These are the same values you would get if you supplied a companion world file along side an ungeoreferenced image.

## Optional inputs

`valid_time` is optional.
If set, it will add the timestamp to the reprojected tif metadata.

`target_epsg` is optional.
If set it will create a new file with the georectified image
reprojected to the designated epsg code.


And when run, the following JSON object is returned:

```json
{
  "fileName": "sample.tif",
  "metadata": {
    "AREA_OR_POINT": "Area",
    "Time": "2019-09-23T16:40:00Z"
  },
  "proj": "NAD83 / UTM zone 11N",
  "bounds": {
    "upper_left": [ 530184, 3900640 ],
    "lower_right": [ 531148, 3899289 ]
  }
}
```


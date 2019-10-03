import cherrypy
from datetime import datetime, tzinfo, timezone
import json
import os.path

from osgeo import gdal
from osgeo import osr

gdal.UseExceptions()

#
# Class to reproject a file given its name, and transformation data
#
class Transform(object):
    fname = ""
    epsg = ""
    xform = []

    #
    # Entrance point for the service
    # :param inputs: A JSON object with POSTed inputs
    # :return: a JSON object with transformation metadata
    #
    def run(self, inputs):
        self.validate(inputs)
        self.process()
        ds = self.finished_dataset(inputs)
        # must be UTC
        if 'valid_time' in inputs:
            vt = datetime \
               .fromisoformat(inputs['valid_time']) \
               .replace(tzinfo=timezone.utc)
            ds.SetMetadata({'Time': vt.isoformat(timespec='seconds')})
        response = {'fileName': self.fname, \
                    'metadata': ds.GetMetadata(), \
                    'proj': self.proj(ds), \
                    'bounds': self.bounds(ds)}
        ds = None
        return response

    #
    # Verify required input parameters are set in JSON inputs
    #
    def validate(self, inputs):
        if 'file' not in inputs:
            msg = ('Required parameter \'file\' (name) not specified')
            raise cherrypy.HTTPError (400, message=msg)
        tmp = os.path.basename(inputs['file'])
        self.fname = tmp.replace('\\','')

        if 'epsg' not in inputs:
            msg = ('Required parameter \'epsg\' (EPSG code) not specified')
            raise cherrypy.HTTPError (400, message=msg)
        self.epsg = inputs['epsg']

        if 'transform' not in inputs:
            msg = ('Required parameter \'transform\' not specified')
            raise cherrypy.HTTPError (400, message=msg)
        self.xform = inputs['transform']

    #
    # In place file transformation
    #
    def process(self):
        try: 
            ds = gdal.Open(f'/data/{self.fname}', gdal.GA_Update)
            ds.SetGeoTransform(self.xform)
            srs = osr.SpatialReference()
            srs.ImportFromEPSG(self.epsg)
            ds.SetProjection(srs.ExportToWkt())
        except RuntimeError as e:
            msg = f'Error encountered when attempting to transform {self.fname}. {e}'
            raise cherrypy.HTTPError (520, message=msg)
        finally:
            ds = None



    #
    # Get projection metadata for the transformed file
    # :param dataset: the GDAL dataset opened on the transformed file
    #
    def proj(self, dataset):
        wkt = dataset.GetProjection()
        srs=osr.SpatialReference(wkt)
        # 80% solution
        proj = srs.GetAttrValue('projcs')
        # edge case
        if proj is None:
            proj = srs.GetAttrValue('geoccs')
        if proj is None:
            proj = srs.GetAttrValue('geogcs')
        # fall back to wkt if a Local CS or some other oddabll
        if proj is None:
            proj = wkt
        return proj

    #
    # Compute the projected bounds of the transformed file
    # :param dataset: the GDAL dataset opened on the transformed file
    #
    def bounds(self, dataset):
        ulx, xres, xskew, uly, yskew, yres  = dataset.GetGeoTransform()
        lrx = ulx + (dataset.RasterXSize * xres)
        lry = uly + (dataset.RasterYSize * yres)
        ul = [ulx, uly]
        lr = [lrx, lry]
        return {'upper_left': ul, 'lower_right': lr}

    #
    # Reproject the dataset from the affine coordinates
    # into another projected CS if 
    # 'target_epsg' is defined.
    # :param inputs: A JSON object with POSTed inputs
    # :return: either the dataset for the projected CS defined in the inputs
    #         or the reprojected (final destination) dataset.

    # This is here just for debugging.
    # Ultimately, self.fname should just get modified in place without
    # creating a second file.
    #
    def finished_dataset(self, inputs):
        ds = gdal.Open(f'/data/{self.fname}')
        dest = f'/data/reprojected_{self.fname}'
        if 'target_epsg' in inputs:
            gdal.Warp(dest, ds, dstSRS='EPSG:' + str(inputs['target_epsg']))

        if 'target_epsg' in inputs:
            ds = None
            ds = gdal.Open(dest)

        return ds


//----------------------------------------------------------------------------------------------------------------------
// File        :  src/gstgzdec.h
// Copyright   :  (c) Julian Bouzas 2017
// License     :  BSD3-style (see LICENSE)
// Maintainer  :  Julian Bouzas - nnoell3[at]gmail.com
//----------------------------------------------------------------------------------------------------------------------

#ifndef GST_GZ_GSTGZDEC_H_
#define GST_GZ_GSTGZDEC_H_

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

#include <zlib.h>
#include <bzlib.h>

G_BEGIN_DECLS

/**
 * @~english
 * The GLib type identification number of the GzDec GObject class
 */
#define GST_TYPE_GZDEC (gst_gzdec_get_type())

/**
 * @~english
 * Converts a GObject pointer into a GzDec object pointer
 * @param obj the pointer to convert
 */
#define GST_GZDEC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GZDEC, GstGzDec))

/**
 * @~english
 * Converts a GObject class into a GzDec class
 * @param klass the class to convert
 */
#define GST_GZDEC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_GZDEC, GstGzDecClass))

/**
 * @~english
 * Determines if an object is a GzDec object
 * @param obj the pointer to check
 */
#define GST_IS_GZDEC(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GZDEC))

/**
 * @~english
 * Determines if a GLib class is a GzDec class
 * @param klass the pointer to check
 */
#define GST_IS_GZDEC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_GZDEC))

/**
 * @~english
 * The GzDec decompress algorithm methods
 * GST_GZDEC_METHOD_GZIP: The GZip decoding method
 * GST_GZDEC_METHOD_BZIP: The BZip decoding method
 */
typedef enum {
  GST_GZDEC_METHOD_GZIP = 0,
  GST_GZDEC_METHOD_BZIP = 1
} GstGzDecMethod;

/**
 * @~english
 * The GzDec GStreamer plugin data
 */
struct _GstGzDec {
  /**
   * @~english
   * The base data
   */
  GstBaseTransform base_data;

  /**
   * @~english
   * Size in bytes to read when decoding
   */
  guint blocksize;

  /**
   * @~english
   * The decoding method to use
   */
  gint method;

  /**
   * @~english
   * The GZip stream
   */
  z_stream gzip_stream;

  /**
   * @~english
   * The BZip stream
   */
  bz_stream bzip_stream;
};

/**
 * @~english
 * A convenience type definition
 */
typedef struct _GstGzDec GstGzDec;

/**
 * @~english
 * The GzDec GStreamer plugin class
 */
struct _GstGzDecClass {
  /**
   * @~english
   * The parent class
   */
  GstBaseTransformClass base_data;
};

/**
 * @~english
 * A convenience type definition
 */
typedef struct _GstGzDecClass GstGzDecClass;

/**
 * @~english
 * Registers the plugin
 * @param plugin The plugin to register
 * @returns true if the plugin could be registered, false otherwise
 */
gboolean gst_gzdec_register(GstPlugin *plugin);

G_END_DECLS

#endif  // GST_GZ_GSTGZDEC_H_

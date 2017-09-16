//----------------------------------------------------------------------------------------------------------------------
// File        :  src/gstgzdec.c
// Copyright   :  (c) Julian Bouzas 2017
// License     :  BSD3-style (see LICENSE)
// Maintainer  :  Julian Bouzas - nnoell3[at]gmail.com
//----------------------------------------------------------------------------------------------------------------------

#include "gstgzdec.h"

/**
 * @~english
 * The properties enumerator
 * PROP_BLOCKSIZE: The size in bytes to read when decoding
 * PROP_METHOD: The decoding method to use: 0 = GZip, 1 = BZip
 */
enum {
  PROP_0,
  PROP_BLOCKSIZE,
  PROP_METHOD,
  PROP_LAST
};

/**
 * @~english
 * The GzDec method enumerator property
 */
static GType gst_gzdec_method_get_type(void) {
  static GType method_type = 0;
  static const GEnumValue method[] = {
    {GST_GZDEC_METHOD_GZIP, "GZip decoding algorithm", "gzip"},
    {GST_GZDEC_METHOD_BZIP, "BZip decoding algorithm", "bzip"},
    {0, NULL, NULL},
  };

  if (!method_type)
    method_type = g_enum_register_static("GstGzDecMethod", method);

  return method_type;
}
#define GST_TYPE_GZDEC_METHOD (gst_gzdec_method_get_type())

/**
 * @~english
 * The source pad template
 */
static GstStaticPadTemplate gst_gzdec_src_template =
    GST_STATIC_PAD_TEMPLATE("src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);

/**
 * @~english
 * The sink pad template
 */
static GstStaticPadTemplate gst_gzdec_sink_template =
    GST_STATIC_PAD_TEMPLATE("sink", GST_PAD_SINK, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);

/**
 * @~english
 * Defines the plugin
 */
G_DEFINE_TYPE(GstGzDec, gst_gzdec, GST_TYPE_BASE_TRANSFORM)

/**
 * @~english
 * Initializes the plugin
 * @param gzdec The gzdec plugin
 */
static void gst_gzdec_init(GstGzDec *gzdec) {
  // Init the properties
  gzdec->blocksize = 0;
  gzdec->method = 0;
}

/**
 * @~english
 * Finalizes the gobject
 * @param gobject The gobject
 */
static void gst_gzdec_finalize(GObject *gobject) {
  G_OBJECT_CLASS(gst_gzdec_parent_class)->finalize(gobject);
}

/**
 * @~english
 * Starts the plugin
 * @param trans The base transform instance
 * @returns True if the plugin could start, False otherwise
 */
static gboolean gst_gzdec_start(GstBaseTransform *trans) {
  GstGzDec *const gzdec = GST_GZDEC(trans);

  // Init the GZip stream
  gzdec->gzip_stream.zalloc = NULL;
  gzdec->gzip_stream.zfree = NULL;
  gzdec->gzip_stream.opaque = NULL;
  if (Z_OK != inflateInit2(&gzdec->gzip_stream, 32))
    return FALSE;

  // Init the BZip stream
  gzdec->bzip_stream.bzalloc = NULL;
  gzdec->bzip_stream.bzfree = NULL;
  gzdec->bzip_stream.opaque = NULL;
  if (BZ_OK != BZ2_bzDecompressInit(&gzdec->bzip_stream, 0, 0))
    return FALSE;

  return TRUE;
}

/**
 * @~english
 * Stops the plugin
 * @param trans The base transform instance
 * @returns True if the the plugin could stop, False otherwise
 */
static gboolean gst_gzdec_stop(GstBaseTransform *trans) {
  GstGzDec *const gzdec = GST_GZDEC(trans);

  // Deinit the GZip stream
  inflateEnd(&gzdec->gzip_stream);

  // Deinit the BZip stream
  BZ2_bzDecompressEnd(&gzdec->bzip_stream);

  return TRUE;
}

/**
 * @~english
 * Decodes a GZip stream
 * @param stream The gzip stream
 * @param indata The input data
 * @param insize The input size
 * @param outdata The output data
 * @param outsize The output size
 * @returns True if the the stream could be decoded, False otherwise
 */
static gboolean gst_gzdec_decode_gzip_stream(z_stream *stream, guint8 *indata, gsize insize, guint8 **outdata,
    gsize *outsize, guint blocksize) {
  // Allocate the output buffer
  guint8* out = (guint8 *)g_malloc_n(blocksize, sizeof(guint8));
  if (!out)
    return FALSE;

  // Inflate
  size_t out_size = 0;
  size_t offset = 0;
  stream->avail_in = insize;
  stream->next_in = indata;
  do {
    stream->avail_out = blocksize;
    stream->next_out = out + offset;
    const int res = inflate(stream, Z_NO_FLUSH);
    switch (res) {
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        g_free(out);
        return FALSE;
      default:
        break;
      }
    size_t have = blocksize - stream->avail_out;
    out_size += have;
    offset += blocksize;
    if (stream->avail_out == 0) {
      out = (guint8 *)g_realloc(out, (blocksize + offset) * sizeof(guint8));
      if (!out) {
        g_free(out);
        return FALSE;
      }
    }
  } while (stream->avail_out == 0);

  // Set outdata and outsize
  *outdata = out;
  *outsize = out_size;
  return TRUE;
}

/**
 * @~english
 * Decodes a BZip stream
 * @param stream The gzip stream
 * @param indata The input data
 * @param insize The input size
 * @param outdata The output data
 * @param outsize The output size
 * @returns True if the the stream could be decoded, False otherwise
 */
static gboolean gst_gzdec_decode_bzip_stream(bz_stream *stream, guint8 *indata, gsize insize, guint8 **outdata,
    gsize *outsize, guint blocksize) {
  // Allocate the output buffer
  guint8* out = (guint8 *)g_malloc_n(blocksize, sizeof(guint8));
  if (!out)
    return FALSE;

  // Decompress
  size_t out_size = 0;
  size_t offset = 0;
  stream->avail_in = insize;
  stream->next_in = (char *)indata;
  do {
    stream->avail_out = blocksize;
    stream->next_out = (char *)(out + offset);
    const int res = BZ2_bzDecompress(stream);
    switch (res) {
      case BZ_MEM_ERROR:
      case BZ_DATA_ERROR:
      case BZ_DATA_ERROR_MAGIC:
      case BZ_PARAM_ERROR:
        g_free(out);
        return FALSE;
      default:
        break;
    }
    size_t have = blocksize - stream->avail_out;
    out_size += have;
    offset += blocksize;
    if (stream->avail_out == 0) {
      out = (guint8 *)g_realloc(out, (blocksize + offset) * sizeof(guint8));
      if (!out) {
        g_free(out);
        return FALSE;
      }
    }
  } while (stream->avail_out == 0);

  // Set outdata and outsize
  *outdata = (guint8 *)out;
  *outsize = out_size;
  return TRUE;
}

#ifdef USE_GSTREAMER_010
/**
 * @~english
 * Transforms one incoming buffer to one outgoing buffer
 * @param trans The base transform instance
 * @param inbuf The input buffer
 * @param outbuf The output buffer
 * @returns a flow return value, indicating whether there was an error or not
 */
static GstFlowReturn gst_gzdec_transform(GstBaseTransform *trans, GstBuffer *inbuf, GstBuffer *outbuf) {
  GstGzDec *const gzdec = GST_GZDEC(trans);

  // Decode the data
  guint8 *outdata = NULL;
  gsize outsize = 0;
  const gsize blocksize = gzdec->blocksize <= 0 ? GST_BUFFER_SIZE(inbuf) : gzdec->blocksize;
  gboolean res = FALSE;
  switch (gzdec->method) {
    case GST_GZDEC_METHOD_GZIP:
      res = gst_gzdec_decode_gzip_stream(&gzdec->gzip_stream, GST_BUFFER_DATA(inbuf), GST_BUFFER_SIZE(inbuf), &outdata,
          &outsize, blocksize);
      break;

    case GST_GZDEC_METHOD_BZIP:
      res = gst_gzdec_decode_bzip_stream(&gzdec->bzip_stream, GST_BUFFER_DATA(inbuf), GST_BUFFER_SIZE(inbuf), &outdata,
          &outsize, blocksize);
      break;

    default:
      GST_ERROR("decoding method not implemented\n");
      return GST_FLOW_ERROR;
  }
  if (!res) {
    GST_ERROR("failed to decode stream\n");
    return GST_FLOW_ERROR;
  }

  // Set the buffer
  GST_BUFFER_SIZE(outbuf) = outsize;
  GST_BUFFER_DATA(outbuf) = outdata;

  return GST_FLOW_OK;
}
#else
/**
 * @~english
 * Prepares one outgoing buffer based on one incoming buffer
 * @param trans The base transform instance
 * @param inbuf The input buffer
 * @param outbuf The output buffer
 * @returns a flow return value, indicating whether there was an error or not
 */
static GstFlowReturn gst_gzdec_prepare_output_buffer(GstBaseTransform *trans, GstBuffer *inbuf, GstBuffer **outbuf) {
  GstGzDec *const gzdec = GST_GZDEC(trans);

  // Get the input buffer map
  GstMapInfo in_map;
  if (!gst_buffer_map(inbuf, &in_map, GST_MAP_READ)) {
    GST_ERROR("failed to map input buffer\n");
    return GST_FLOW_ERROR;
  }

  // Decode the data
  guint8 *outdata = NULL;
  gsize outsize = 0;
  const gsize blocksize = gzdec->blocksize <= 0 ? in_map.size : gzdec->blocksize;
  gboolean res = FALSE;
  switch (gzdec->method) {
    case GST_GZDEC_METHOD_GZIP:
      res = gst_gzdec_decode_gzip_stream(&gzdec->gzip_stream, in_map.data, in_map.size, &outdata, &outsize, blocksize);
      break;

    case GST_GZDEC_METHOD_BZIP:
      res = gst_gzdec_decode_bzip_stream(&gzdec->bzip_stream, in_map.data, in_map.size, &outdata, &outsize, blocksize);
      break;

    default:
      GST_ERROR("decoding method not implemented\n");
      gst_buffer_unmap(inbuf, &in_map);
      return GST_FLOW_ERROR;
  }
  if (!res) {
    GST_ERROR("failed to decode stream\n");
    gst_buffer_unmap(inbuf, &in_map);
    return GST_FLOW_ERROR;
  }

  // Allocate the output buffer
  *outbuf = gst_buffer_new_wrapped(outdata, outsize);
  if (!*outbuf) {
    GST_ERROR("failed to allocate output buffer\n");
    gst_buffer_unmap(inbuf, &in_map);
    return GST_FLOW_ERROR;
  }

  gst_buffer_unmap(inbuf, &in_map);
  return GST_FLOW_OK;
}
#endif

/**
 * @~english
 * Sets a property
 * @param object The base transform instance
 * @param prop_id The property ID
 * @param value The property value
 * @param pspec The property spec
 */
static void gst_gzdec_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
  GstGzDec *const gzdec = GST_GZDEC(object);
  GST_OBJECT_LOCK(gzdec);

  // Set the specific property
  switch (prop_id) {
    case PROP_BLOCKSIZE:
      gzdec->blocksize = g_value_get_uint(value);
      break;

    case PROP_METHOD:
      gzdec->method = g_value_get_enum(value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }

  GST_OBJECT_UNLOCK(gzdec);
}

/**
 * @~english
 * Gets a property
 * @param object The base transform instance
 * @param prop_id The property ID
 * @param value The property value
 * @param pspec The property spec
 */
static void gst_gzdec_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
  GstGzDec *const gzdec = GST_GZDEC(object);

  // Get the specific property
  switch (prop_id) {
    case PROP_BLOCKSIZE:
      g_value_set_uint(value, gzdec->blocksize);
      break;

    case PROP_METHOD:
      g_value_set_enum(value, gzdec->method);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

/**
 * @~english
 * Initializes the GzDec class
 * @param klass The class
 */
static void gst_gzdec_class_init(GstGzDecClass *klass) {
  GObjectClass *const gobject_class = G_OBJECT_CLASS(klass);
  GstElementClass *const element_class = GST_ELEMENT_CLASS(klass);
  GstBaseTransformClass *const base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

  // Set the metadata
  gst_element_class_set_details_simple(element_class, "Zip stream dececoder", "Codec/Decoder/Stream",
      "Decodes Zip streams", "Julian Bouzas <nnoell3@gmail.com>");

  // Add pad templates
  gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&gst_gzdec_sink_template));
  gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&gst_gzdec_src_template));

  // Set the vmethods
  gobject_class->set_property = gst_gzdec_set_property;
  gobject_class->get_property = gst_gzdec_get_property;
  gobject_class->finalize = gst_gzdec_finalize;
  base_transform_class->start = gst_gzdec_start;
  base_transform_class->stop = gst_gzdec_stop;
#ifdef USE_GSTREAMER_010
  base_transform_class->transform = gst_gzdec_transform;
#else
  base_transform_class->prepare_output_buffer = gst_gzdec_prepare_output_buffer;
#endif

  // Install the properties
  g_object_class_install_property(gobject_class, PROP_BLOCKSIZE, g_param_spec_uint("blocksize", "Block size",
      "Size in bytes to read when decoding (0 = input buffer's size)", 0, G_MAXUINT, 0,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property(gobject_class, PROP_METHOD, g_param_spec_enum("method", "Decoding method",
      "The decoding method to use", GST_TYPE_GZDEC_METHOD, GST_GZDEC_METHOD_GZIP,
      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

gboolean gst_gzdec_register(GstPlugin *plugin) {
  return gst_element_register(plugin, "gzdec", GST_RANK_NONE, GST_TYPE_GZDEC);
}

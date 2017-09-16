//----------------------------------------------------------------------------------------------------------------------
// File        :  src/gstgz.c
// Copyright   :  (c) Julian Bouzas 2017
// License     :  BSD3-style (see LICENSE)
// Maintainer  :  Julian Bouzas - nnoell3[at]gmail.com
//----------------------------------------------------------------------------------------------------------------------

#include "gstgzdec.h"

/**
 * @~english
 * The package macro
 */
#ifndef PACKAGE
#define PACKAGE "GZip plugins package"
#endif

/**
 * @~english
 * Registers the gz plugin
 * @param plugin The plugin to register
 * @returns true if the plugin could be registered, false otherwise
 */
gboolean gst_gz_register(GstPlugin *plugin) {
  return gst_gzdec_register(plugin);
}

#ifdef USE_GSTREAMER_010
/**
 * @~english
 * Defines the plugins for gstreamer 0.10
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, "gz", "GZip plugin library", gst_gz_register, "0.1.0",
    "Proprietary", "GStreamer GZip", "https://github.com/nnoell/gst-gz")
#else
/**
 * @~english
 * Defines the plugins for gstreamer 1.0
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, gz, "GZip plugin library", gst_gz_register, "0.1.0",
    "Proprietary", "GStreamer GZip", "https://github.com/nnoell/gst-gz")
#endif

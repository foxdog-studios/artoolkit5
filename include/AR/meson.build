cfg = configuration_data()

# Input modules
cfg.set('AR_INPUT_V4L',       false)
cfg.set('AR_INPUT_V4L2',      true)
cfg.set('AR_INPUT_DV',        false)
cfg.set('AR_INPUT_1394CAM',   false)
cfg.set('AR_INPUT_GSTREAMER', false)
cfg.set('AR_INPUT_IMAGE',     false)
cfg.set('AR_INPUT_DUMMY',     false)

# Default input module
cfg.set('AR_DEFAULT_INPUT_V4L',       false)
cfg.set('AR_DEFAULT_INPUT_V4L2',      true)
cfg.set('AR_DEFAULT_INPUT_DV',        false)
cfg.set('AR_DEFAULT_INPUT_1394CAM',   false)
cfg.set('AR_DEFAULT_INPUT_GSTREAMER', false)
cfg.set('AR_DEFAULT_INPUT_IMAGE',     false)
cfg.set('AR_DEFAULT_INPUT_DUMMY',     false)

# Default pixel formats
pix_fmt_bgr  = 'AR_PIXEL_FORMAT_BGR'
pix_fmt_bgra = 'AR_PIXEL_FORMAT_BGRA'
cfg.set('AR_INPUT_V4L2_DEFAULT_PIXEL_FORMAT', pix_fmt_bgr)

# Intel SIMD
cfg.set('HAVE_INTEL_SIMD', false)

configuration_file = configure_file(
  input : 'config.h.in',
  output : '@BASENAME@',
  configuration : cfg,
)
